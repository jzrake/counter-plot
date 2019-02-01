#include "MainComponent.hpp"
#include "../Core/LookAndFeel.hpp"
#include "../Core/Main.hpp"
#include "../Core/MovieWriter.hpp"
#include "../Core/Runtime.hpp"




//=========================================================================
EitherOrComponent::EitherOrComponent()
{
    addAndMakeVisible (button1);
    addAndMakeVisible (button2);
    button1.setToggleState (true, NotificationType::dontSendNotification);

    button1.onStateChange = [this]
    {
        if (! component1->isVisible() && button1.getToggleState())
        {
            component1->setVisible (true);
            grabKeyboardFocus();
        }
        else if (component1->isVisible() && ! button1.getToggleState())
        {
            component1->setVisible (false);
        }
    };
    button2.onStateChange = [this]
    {
        if (! component2->isVisible() && button2.getToggleState())
        {
            component2->setVisible (true);
            grabKeyboardFocus();
        }
        else if (component2->isVisible() && ! button2.getToggleState())
        {
            component2->setVisible (false);
        }
    };
}

void EitherOrComponent::setComponent1 (Component* componentToShow)
{
    addChildComponent (component1 = componentToShow);

    if (component1)
    {
        button1.setButtonText (component1->getName());
        component1->setVisible (button1.getToggleState());
    }
    button1.setEnabled (component1);
}

void EitherOrComponent::setComponent2 (Component* componentToShow)
{
    addChildComponent (component2 = componentToShow);

    if (component2)
    {
        button2.setButtonText (component2->getName());
        component2->setVisible (button2.getToggleState());
    }
    button2.setEnabled (component2);
}

void EitherOrComponent::showComponent1()
{
    if (component1)
    {
        button1.setToggleState (true, NotificationType::dontSendNotification);
        component1->setVisible (true);
    }
}

void EitherOrComponent::showComponent2()
{
    if (component2)
    {
        button2.setToggleState (true, NotificationType::dontSendNotification);
        component2->setVisible (true);
    }
}




//=========================================================================
SourceList::SourceList()
{
    list.setColour (ListBox::ColourIds::backgroundColourId, Colours::transparentBlack);
    list.setModel (this);
    list.setRowHeight (96);
    list.setMultipleSelectionEnabled (true);
    addAndMakeVisible (list);
}

void SourceList::addListener (Listener* listener)
{
    listeners.add (listener);
}

void SourceList::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void SourceList::clear()
{
    sources.clear();
    assets.clear();
    list.updateContent();
}

void SourceList::setSources (const Array<File>& sourcesToShow)
{
    sources = sourcesToShow;
    assets.clear();
    assets.resize (sources.size());
    list.updateContent();
}

void SourceList::addSource (File source)
{
    if (! sources.contains (source))
    {
        sources.addUsingDefaultSort (source);
        assets.insert (sources.indexOf (source), SourceAssets());
        list.updateContent();
    }
}

void SourceList::removeSource (File source)
{
    removeSourceAtRow (sources.indexOf (source));
}

void SourceList::removeSourceAtRow (int row)
{
    sources.remove (row);
    assets.remove (row);
    list.updateContent();
}

void SourceList::setCaptureForSource (File source, Image capturedImage)
{
    int n = sources.indexOf (source);
    assets.getReference(n).capture = capturedImage;
    assets.getReference(n).thumbnail = capturedImage.rescaled (capturedImage.getWidth() / 4,
                                                               capturedImage.getHeight() / 4);
    list.repaintRow(n);
}

Array<Image> SourceList::getAllImageAssets() const
{
    Array<Image> captures;

    for (const auto& item : assets)
        captures.add (item.capture);
    return captures;
}

Array<File> SourceList::getSelectedSources() const
{
    Array<File> selectedSources;
    int n = 0;

    for (const auto& source : sources)
        if (list.isRowSelected (n++))
            selectedSources.add (source);
    return selectedSources;
}




//=========================================================================
void SourceList::resized()
{
    list.setBounds (getLocalBounds());
}

void SourceList::paint (Graphics& g)
{
    g.fillAll (findColour (AppLookAndFeel::directoryTreeBackground));
}

bool SourceList::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::spaceKey)
    {
        listeners.call (&Listener::sourceListWantsCaptureOfCurrent, this);
        return true;
    }
    if (key == KeyPress ('a', ModifierKeys::commandModifier, 0))
    {
        list.selectRangeOfRows (0, getNumRows());
        return true;
    }
    return false;
}




//=========================================================================
int SourceList::getNumRows()
{
    return sources.size();
}

void SourceList::paintListBoxItem (int row, Graphics &g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
    {
        g.fillAll (findColour (AppLookAndFeel::directoryTreeSelectedItem));

        g.setColour (Colours::cornflowerblue);
        g.fillRect (getLocalBounds().removeFromLeft (4));
    }

    auto area = Rectangle<int> (0, 0, width, height);
    auto nameArea = area.removeFromTop (22);
    auto imageArea = area.reduced (8, 0).withTrimmedBottom(4);
    auto image = assets.getReference (row).thumbnail;

    g.setColour (findColour (AppLookAndFeel::directoryTreeFile));
    g.drawText (sources.getReference (row).getFileName(), nameArea.reduced (8, 0), Justification::centred);

    if (image == Image())
    {
        auto target = imageArea.withSizeKeepingCentre (imageArea.getHeight(),
                                                       imageArea.getHeight()).toFloat();
        g.setColour (findColour (AppLookAndFeel::directoryTreeSelectedItem).brighter());
        g.fillRect (target);
        g.setColour (findColour (AppLookAndFeel::directoryTreeBackground).darker());
        g.drawRect (target, 1.f);
    }
    else
    {
        auto aspect = image.getBounds().getAspectRatio();
        auto target = imageArea.withSizeKeepingCentre (imageArea.getHeight() * aspect,
                                                       imageArea.getHeight()).toFloat();
        g.drawImage (image, target);
    }
}

void SourceList::selectedRowsChanged (int lastRowSelected)
{
    if (list.getNumSelectedRows() == 1)
        listeners.call (&Listener::sourceListSelectedSourceChanged, this, sources[lastRowSelected]);
}

void SourceList::deleteKeyPressed (int row)
{
    removeSourceAtRow (row);

    if (row >= getNumRows())
        list.selectRow (row - 1);
}

void SourceList::listBoxItemDoubleClicked (int row, const MouseEvent& e)
{
    auto image = assets.getReference(row).capture;

    if (image != Image())
    {
        auto target = File::createTempFile (".png");

        if (auto stream = std::unique_ptr<FileOutputStream> (target.createOutputStream()))
        {
            auto fmt = PNGImageFormat();
            fmt.writeImageToStream (image, *stream);
            target.startAsProcess();
        }
    }
}

String SourceList::getTooltipForRow (int row)
{
    return String();
}




//=========================================================================
void SourceList::sendSelectedSourceChanged (int row)
{
    listeners.call (&Listener::sourceListSelectedSourceChanged, this, sources[row]);
}




//=========================================================================
void EitherOrComponent::paint (Graphics& g)
{
    auto area = getLocalBounds();
    auto buttonRow = area.removeFromTop (34);

    g.setColour (findColour (AppLookAndFeel::directoryTreeBackground));
    g.fillRect (buttonRow);
}

void EitherOrComponent::resized()
{
    auto area = getLocalBounds();
    auto buttonRow = area.removeFromTop (34).reduced (6);
    button1.setBounds (buttonRow.removeFromLeft (getWidth() / 2));
    button2.setBounds (buttonRow);

    if (component1)
        component1->setBounds (area);
    if (component2)
        component2->setBounds (area);
}




//=========================================================================
EitherOrComponent::TabButton::TabButton() : Button ("")
{
    setWantsKeyboardFocus (false);
    setClickingTogglesState (true);
    setRadioGroupId (1);
}

void EitherOrComponent::TabButton::paintButton (Graphics& g, bool, bool)
{
    auto c1 = findColour (AppLookAndFeel::directoryTreeBackground);
    auto c2 = findColour (AppLookAndFeel::directoryTreeBackground).darker();
    g.setFont (Font().withHeight (11));

    if (getToggleState())
    {
        g.setColour (c2);
        g.fillRect (getLocalBounds());
        g.setColour (c1.brighter (0.4f));
        g.drawText (getButtonText(), getLocalBounds(), Justification::centred);
    }
    else
    {
        g.setColour (c1);
        g.fillRect (getLocalBounds());
        g.setColour (c2.brighter (0.8f));
        g.drawText (getButtonText(), getLocalBounds(), Justification::centred);
    }

    g.setColour (c2);
    g.drawRect (getLocalBounds());
}




//=============================================================================
StatusBar::EnvironmentViewToggleButton::EnvironmentViewToggleButton() : Button ("")
{
    setWantsKeyboardFocus (false);
}

void StatusBar::EnvironmentViewToggleButton::paintButton (Graphics& g,
                                                          bool highlighted,
                                                          bool /*down*/)
{
    if (auto main = findParentComponentOfClass<MainComponent>())
    {
        auto text = String (main->isEnvironmentViewShowing() ? "Hide" : "Show") + " Viewer Environment";
        g.setFont (Font().withHeight (11));
        g.setColour (findColour (AppLookAndFeel::statusBarText).brighter (highlighted ? 0.2f : 0.0f));
        g.drawText (text, getLocalBounds().withTrimmedLeft(8), Justification::centredLeft);
    }
}




//=============================================================================
StatusBar::ViewerNamePopupButton::ViewerNamePopupButton() : Button ("")
{
    setWantsKeyboardFocus (false);
}

void StatusBar::ViewerNamePopupButton::paintButton (Graphics& g,
                                                    bool highlighted,
                                                    bool /*down*/)
{
    g.setFont (Font().withHeight (11));
    g.setColour (findColour (AppLookAndFeel::statusBarText).brighter (highlighted ? 0.2f : 0.0f));
    g.drawText (currentViewerName, getLocalBounds().withTrimmedRight(8), Justification::centredRight);
}

void StatusBar::ViewerNamePopupButton::clicked()
{
}




//=============================================================================
StatusBar::StatusBar()
{
    PatchViewApplication::getApp().configureCommandButton (environmentViewToggleButton, PatchViewApplication::toggleEnvironmentView);
    addAndMakeVisible (environmentViewToggleButton);
    addAndMakeVisible (viewerNamePopupButton);
}

StatusBar::~StatusBar()
{
}

void StatusBar::incrementAsyncTaskCount()
{
    ++numberOfAsyncTasks;
    repaint();
}

void StatusBar::decrementAsyncTaskCount()
{
    --numberOfAsyncTasks;
    repaint();
}

void StatusBar::setMousePositionInFigure (Point<double> position)
{
    mousePositionInFigure = position;
    repaint();
}

void StatusBar::setCurrentViewerName (const String& viewerName)
{
    viewerNamePopupButton.currentViewerName = viewerName;
    viewerNamePopupButton.repaint();
}

void StatusBar::setCurrentErrorMessage (const String& what)
{
    currentErrorMessage = what;
    millisecondsToDisplayInfo = 4000;
    startTimer (40);
    repaint();
}

void StatusBar::setCurrentInfoMessage (const String& info, int milliseconds)
{
    if (milliseconds > 0)
    {
        millisecondsToDisplayInfo = milliseconds;
        startTimer (40);
    }
    currentInfoMessage = info;
    repaint();
}




//=============================================================================
void StatusBar::paint (Graphics& g)
{
    auto geom                = computeGeometry();
    auto backgroundColour    = findColour (AppLookAndFeel::statusBarBackground);
    auto fontColour          = findColour (AppLookAndFeel::statusBarText);
    auto errorColour         = findColour (AppLookAndFeel::statusBarErrorText);
    auto busyIndicatorColour = numberOfAsyncTasks ? Colours::yellow : Colours::transparentBlack;

    g.setColour (backgroundColour);
    g.fillAll();

    g.setColour (busyIndicatorColour);
    g.fillEllipse (geom.busyIndicatorArea.reduced (5).toFloat());

    g.setColour (fontColour);
    g.setFont (Font().withHeight (11));
    g.drawText (mousePositionInFigure.isOrigin() ? "" : mousePositionInFigure.toString(), geom.mousePositionArea, Justification::centredLeft);

    if (currentErrorMessage.isNotEmpty())
    {
        g.setColour (errorColour.withAlpha (jmin (1.f, millisecondsToDisplayInfo / 400.f)));
        g.drawText (currentErrorMessage, geom.messageArea, Justification::centredLeft);
    }
    else if (currentInfoMessage.isNotEmpty())
    {
        if (numberOfAsyncTasks > 0)
        {
            g.drawText (currentInfoMessage, geom.messageArea, Justification::centredLeft);
        }
        else if (millisecondsToDisplayInfo > 0)
        {
            g.setColour (fontColour.withAlpha (jmin (1.f, millisecondsToDisplayInfo / 400.f)));
            g.drawText (currentInfoMessage, geom.messageArea, Justification::centredLeft);
        }
    }
}

void StatusBar::resized()
{
    auto geom = computeGeometry();
    environmentViewToggleButton.setBounds (geom.environmentViewToggleArea);
    viewerNamePopupButton.setBounds (geom.viewerNamePopupArea);
}

void StatusBar::colourChanged()
{
    setColours();
    repaint();
}

void StatusBar::lookAndFeelChanged()
{
    setColours();
    repaint();
}




//=============================================================================
void StatusBar::timerCallback()
{
    millisecondsToDisplayInfo -= getTimerInterval();

    if (millisecondsToDisplayInfo <= 0)
    {
        millisecondsToDisplayInfo = 0;
        stopTimer();
    }
    repaint();
}




//=============================================================================
StatusBar::Geometry StatusBar::computeGeometry() const
{
    auto geom = Geometry();
    auto area = getLocalBounds();
    geom.busyIndicatorArea         = area.removeFromRight (getHeight());
    geom.viewerNamePopupArea       = area.removeFromRight (200);
    geom.mousePositionArea         = area.removeFromRight (120);
    geom.environmentViewToggleArea = area.removeFromLeft  (150);
    geom.messageArea               = area;
    return geom;
}

void StatusBar::setColours()
{
}




//=============================================================================
MainComponent::MainComponent()
{
    directoryTree.addListener (this);
    directoryTree.getTreeView().setWantsKeyboardFocus (directoryTreeShowing);
    sourceList.addListener (this);
    statusBar.setCurrentViewerName ("Viewer List");

    directoryTree.setName ("Directory");
    sourceList.setName ("Sources");
    sidebar.setComponent1 (&directoryTree);
    sidebar.setComponent2 (&sourceList);

    addAndMakeVisible (sidebar);
    addAndMakeVisible (statusBar);

    setSize (1024, 768 - 64);
}

MainComponent::~MainComponent()
{
}

void MainComponent::setCurrentDirectory (File newCurrentDirectory)
{
    directoryTree.setDirectoryToShow (newCurrentDirectory);
}

void MainComponent::setCurrentFile (File newCurrentFile)
{
}

void MainComponent::reloadCurrentFile()
{
}

void MainComponent::reloadDirectoryTree()
{
    directoryTree.reloadAll();
}

void MainComponent::toggleDirectoryTreeShown (bool animated)
{
    directoryTreeShowing = ! directoryTreeShowing;
    directoryTree.getTreeView().setWantsKeyboardFocus (directoryTreeShowing);
    layout (animated);
}

void MainComponent::toggleEnvironmentViewShown (bool animated)
{
}

void MainComponent::toggleKernelRuleEntryShown()
{
}

void MainComponent::toggleUserExtensionsDirectoryEditor()
{
}

bool MainComponent::hideExtraComponents()
{
    if (kernelRuleEntryShowing)
    {
        toggleKernelRuleEntryShown();
        return true;
    }
    if (environmentViewShowing)
    {
        toggleEnvironmentViewShown (false);
        return true;
    }
    return false;
}

bool MainComponent::isDirectoryTreeShowing() const
{
    return directoryTreeShowing;
}

bool MainComponent::isEnvironmentViewShowing() const
{
    return environmentViewShowing;
}

bool MainComponent::isKernelRuleEntryShowing() const
{
    return kernelRuleEntryShowing;
}

bool MainComponent::isUserExtensionsDirectoryEditorShowing() const
{
    return false;
}

File MainComponent::getCurrentDirectory() const
{
    return directoryTree.getCurrentDirectory();
}

File MainComponent::getCurrentFile() const
{
    return currentFile;
}

DirectoryTree& MainComponent::getDirectoryTree()
{
    return directoryTree;
}

void MainComponent::showKernelRule (const String& rule)
{
}

void MainComponent::indicateSuccess (const String& info)
{
    statusBar.setCurrentErrorMessage (String());
    statusBar.setCurrentInfoMessage (info, 3000);
}

void MainComponent::logErrorMessage (const String& what)
{
    statusBar.setCurrentErrorMessage (what);
}

void MainComponent::createAnimation (bool toTempDirectory)
{
}




//=============================================================================
void MainComponent::resized()
{
    layout (false);
}




//=============================================================================
void MainComponent::directoryTreeSelectedFileChanged (DirectoryTree*, File file)
{
    setCurrentFile (file);
}

void MainComponent::directoryTreeWantsFileToBeSource (DirectoryTree*, File file)
{
    sourceList.addSource (file);
    sidebar.showComponent2();
}




//=============================================================================
void MainComponent::sourceListSelectedSourceChanged (SourceList*, File file)
{
}

void MainComponent::sourceListWantsCaptureOfCurrent (SourceList*)
{
}




//=============================================================================
void MainComponent::figureMousePosition (Point<double> position)
{
    statusBar.setMousePositionInFigure (position);
}




//=============================================================================
void MainComponent::layout (bool animated)
{
    auto setBounds = [animated] (Component& component, const Rectangle<int>& bounds)
    {
        if (animated && component.isVisible())
            Desktop::getInstance().getAnimator().animateComponent (&component, bounds, 1.f, 200, false, 1.f, 1.f);
        else
            component.setBounds (bounds);
    };

    auto area = getLocalBounds();
    auto statusBarArea = area.removeFromBottom (22);
    auto directoryTreeArea = directoryTreeShowing ? area.removeFromLeft (300) : area.withWidth (300).translated (-300, 0);

    setBounds (statusBar, statusBarArea);
    setBounds (sidebar, directoryTreeArea);
}
