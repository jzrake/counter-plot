#include "MainComponent.hpp"
#include "../Core/LookAndFeel.hpp"
#include "../Core/Main.hpp"
#include "../Core/MovieWriter.hpp"
#include "../Viewers/Viewer.hpp"
#include "../Viewers/ColourMapViewer.hpp"
#include "../Viewers/UserExtensionView.hpp"




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
class MetaYamlViewer : public Viewer
{
public:

    //=========================================================================
    MetaYamlViewer (MessageSink* messageSink)
    {
        viewer.setMessageSink (messageSink);
        addAndMakeVisible (viewer);
    }

    //=========================================================================
    bool isInterestedInFile (File file) const override
    {
        return file.hasFileExtension (".yaml");
    }

    void loadFile (File file) override
    {
        if (file != currentFile)
        {
            currentFile = file;
            viewer.reset();
            reloadFile();
        }
    }

    void reloadFile() override
    {
        viewer.configure (currentFile);

        if (auto main = findParentComponentOfClass<MainComponent>())
        {
            main->refreshCurrentViewerName();
        }
    }

    String getViewerName() const override
    {
        return "Meta Viewer (" + viewer.getViewerName() + ")";
    }

    const Runtime::Kernel* getKernel() const override
    {
        return viewer.getKernel();
    }

    bool canReceiveMessages() const override
    {
        return viewer.canReceiveMessages();
    }

    bool receiveMessage (const String& message) override
    {
        return viewer.receiveMessage (message);
    }

    //=========================================================================
    void resized() override
    {
        viewer.setBounds (getLocalBounds());
    }

private:
    File currentFile;
    UserExtensionView viewer;
};




//=========================================================================
UserExtensionsDirectoryEditor::UserExtensionsDirectoryEditor()
{
    editor.addListener (this);
    editor.addKeyListener (&mappings);
    editor.setMultiLine (true);
    editor.setTabKeyUsedAsCharacter (false);
    editor.setFont (Font ("Menlo", 14, 0));
    mappings.returnKeyCallback = [this] () { return sendContentsToMainViewerCollection(); };
    setColours();
    addAndMakeVisible (editor);
}

void UserExtensionsDirectoryEditor::setDirectories (const Array<File>& directories)
{
    auto paths = StringArray();

    for (auto dir : directories)
        paths.add (toRelativePath (dir.getFullPathName()));
    editor.setText (paths.joinIntoString ("\n"));
}

Array<File> UserExtensionsDirectoryEditor::getDirectories() const
{
    auto directories = Array<File>();

    for (const auto& path : StringArray::fromLines (editor.getText()))
        if (path.isNotEmpty())
            directories.add (fromRelativePath (path));
    return directories;
}




//=========================================================================
void UserExtensionsDirectoryEditor::paint (Graphics& g)
{
    auto instructionsArea = getLocalBounds().withTop (editor.getBottom());
    auto press = KeyPress (KeyPress::returnKey, ModifierKeys::commandModifier, 0);

    g.setColour (findColour (AppLookAndFeel::statusBarBackground));
    g.fillRect (instructionsArea);

    g.setFont (Font().withHeight (11)); // TODO: add font preference to LAF
    g.setColour (findColour (AppLookAndFeel::statusBarText));
    g.drawText ("Directories to watch for .yaml extensions - one per line.", instructionsArea.withTrimmedLeft(6), Justification::centredLeft);
    g.drawText (press.getTextDescriptionWithIcons(), instructionsArea.withTrimmedRight(6), Justification::centredRight);
}

void UserExtensionsDirectoryEditor::resized()
{
    editor.setBounds (getLocalBounds().withTrimmedBottom (22));
}

void UserExtensionsDirectoryEditor::visibilityChanged()
{
    if (isVisible())
        if (auto main = findParentComponentOfClass<MainComponent>())
            setDirectories (main->getViewerCollection().getWatchedDirectories());
}

void UserExtensionsDirectoryEditor::colourChanged()
{
    setColours();
    repaint();
}

void UserExtensionsDirectoryEditor::lookAndFeelChanged()
{
    setColours();
    repaint();
}




//=========================================================================
void UserExtensionsDirectoryEditor::textEditorTextChanged (TextEditor&)
{
}

void UserExtensionsDirectoryEditor::textEditorReturnKeyPressed (TextEditor&)
{
    editor.insertTextAtCaret ("\n");
}

void UserExtensionsDirectoryEditor::textEditorEscapeKeyPressed (TextEditor&)
{
    setVisible (false);
}

void UserExtensionsDirectoryEditor::textEditorFocusLost (TextEditor&)
{
    setVisible (false);
}




//=========================================================================
void UserExtensionsDirectoryEditor::setColours()
{
    editor.setColour (TextEditor::textColourId, findColour (AppLookAndFeel::statusBarText).brighter());
    editor.setColour (TextEditor::backgroundColourId, findColour (AppLookAndFeel::statusBarBackground));
    editor.setColour (TextEditor::highlightColourId, findColour (AppLookAndFeel::statusBarBackground).brighter());
    editor.setColour (TextEditor::highlightedTextColourId, findColour (AppLookAndFeel::statusBarText).brighter());
    editor.setColour (TextEditor::focusedOutlineColourId, Colours::transparentBlack);
    editor.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
}

bool UserExtensionsDirectoryEditor::areContentsValid() const
{
    for (auto dir : getDirectories())
        if (dir == File())
            return false;
    return true;
}

bool UserExtensionsDirectoryEditor::sendContentsToMainViewerCollection()
{
    if (auto main = findParentComponentOfClass<MainComponent>())
    {
        if (areContentsValid())
        {
            main->getViewerCollection().setWatchedDirectories (getDirectories());
            main->indicateSuccess ("Extension directories updated");
            setVisible (false);
            return true;
        }
        main->logErrorMessage ("Invalid directories");
    }
    return false;
}

String UserExtensionsDirectoryEditor::toRelativePath (const File& file) const
{
    auto home = File::getSpecialLocation (File::userHomeDirectory).getFullPathName();

    if (file.getFullPathName().startsWith (home))
        return file.getFullPathName().replaceFirstOccurrenceOf (home, "~");
    return file.getFullPathName();
}

File UserExtensionsDirectoryEditor::fromRelativePath (const String& path) const
{
    auto home = File::getSpecialLocation (File::userHomeDirectory).getFullPathName();

    if (path.startsWith ("~"))
        return path.replaceFirstOccurrenceOf ("~", home);
    else if (File::isAbsolutePath (path))
        return path;
    return File();
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
    if (auto main = findParentComponentOfClass<MainComponent>())
    {
        PopupMenu menu;
        StringArray names;
        File currentFile = main->getCurrentFile();
        const Viewer* currentViewer = main->getCurrentViewer();

        for (auto viewer : main->getViewerCollection().getAllComponents())
        {
            auto name = viewer->getViewerName();
            names.add (name);
            menu.addItem (names.size(), name, viewer->isInterestedInFile (currentFile), viewer == currentViewer);
        }
        int result = menu.show();

        if (result != 0)
        {
            main->setCurrentViewer (names[result - 1]);
        }
    }
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
EnvironmentView::EnvironmentView()
{
    list.setModel (this);
    list.getViewport()->setWantsKeyboardFocus (false);
    setColours();
    addAndMakeVisible (list);
}

void EnvironmentView::setKernel (const Runtime::Kernel* kernelToView)
{
    kernel = kernelToView;
    refresh();
}

void EnvironmentView::refresh()
{
    keys.clear();

    if (kernel)
        for (const auto& item : *kernel)
            if ((item.second.flags & Runtime::builtin) == 0)
                keys.add (item.first);

    keys.sort();
    list.updateContent();
    repaint();
}




//=============================================================================
void EnvironmentView::resized()
{
    list.setBounds (getLocalBounds());
}

void EnvironmentView::colourChanged()
{
    setColours();
    repaint();
}

void EnvironmentView::lookAndFeelChanged()
{
    setColours();
    repaint();
}




//=============================================================================
void EnvironmentView::setColours()
{
    list.setColour (ListBox::backgroundColourId, findColour (AppLookAndFeel::environmentViewBackground));
}




//=============================================================================
int EnvironmentView::getNumRows()
{
    return keys.size();
}

void EnvironmentView::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    g.fillAll (rowIsSelected ? findColour (ListBox::backgroundColourId).darker() : Colours::transparentBlack);

    auto key = keys[rowNumber];
    auto err = kernel->error_at (key.toStdString());
    auto repr = Runtime::represent (kernel->at (keys[rowNumber].toStdString()));
    auto text1 = findColour (AppLookAndFeel::environmentViewText1);
    auto text2 = findColour (AppLookAndFeel::environmentViewText2);

    g.setFont (Font ("Menlo", 11, 0));
    g.setColour (text1);
    g.drawText (keys[rowNumber], 8, 0, width - 16, height, Justification::centredLeft);

    if (err.empty())
    {
        g.setFont (g.getCurrentFont().withStyle (Font::italic));
        g.setColour (text2);
        g.drawText (repr, 8, 0, width - 16, height, Justification::centredRight);
    }
    else
    {
        g.setFont (g.getCurrentFont().withStyle (Font::italic));
        g.setColour (Colours::orange);
        g.drawText (err, 8, 0, width - 16, height, Justification::centredRight);
    }
}

void EnvironmentView::selectedRowsChanged (int lastRowSelected)
{
    if (auto main = findParentComponentOfClass<MainComponent>())
    {
        main->showKernelRule (keys[lastRowSelected]);
    }
}

String EnvironmentView::getTooltipForRow (int row)
{
    auto key = keys[row];
    auto err = kernel->error_at (key.toStdString());
    return err;
}




//=========================================================================
KernelRuleEntry::KernelRuleEntry()
{
    editor.setTextToShowWhenEmpty ("key: value", Colours::grey);
    editor.setFont (Font ("Menlo", 15, 0));
    editor.addListener (this);
    editor.addKeyListener (&keyMappings);
    keyMappings.nextCallback = [this] () { return recallNext(); };
    keyMappings.prevCallback = [this] () { return recallPrev(); };
    setColours();
    addAndMakeVisible (editor);
}

void KernelRuleEntry::loadRule (const std::string& rule, const Runtime::Kernel& kernel)
{
    if (kernel.contains (rule))
    {
        if (! kernel.expr_at (rule).empty())
            editor.setText (rule + ": " + kernel.expr_at (rule).str());
        else
            editor.setText (rule + ": " + kernel.at (rule).toString().toStdString());

        loadedRule = rule;
        loadedText = editor.getText();
        editor.repaint();
    }
}

void KernelRuleEntry::refresh (const Runtime::Kernel* kernel)
{
    if (kernel && editor.getText() == loadedText)
    {
        loadRule (loadedRule.toStdString(), *kernel);
    }
}

bool KernelRuleEntry::recallNext()
{
    if (indexInHistory < history.size())
    {
        editor.setText (history[++indexInHistory]);
        return true;
    }
    return false;
}

bool KernelRuleEntry::recallPrev()
{
    if (indexInHistory > 0)
    {
        editor.setText (history[--indexInHistory]);
        return true;
    }
    return false;
}




//=========================================================================
void KernelRuleEntry::resized()
{
    auto indent = (getHeight() - editor.getFont().getHeight()) / 2;
    editor.setIndents (indent / 2, indent);
    editor.setBounds (getLocalBounds());
}

void KernelRuleEntry::colourChanged()
{
    setColours();
    repaint();
}

void KernelRuleEntry::lookAndFeelChanged()
{
    setColours();
    repaint();
}




//=========================================================================
void KernelRuleEntry::textEditorTextChanged (TextEditor&)
{
}

void KernelRuleEntry::textEditorReturnKeyPressed (TextEditor&)
{
    if (auto main = findParentComponentOfClass<MainComponent>())
    {
        if (! editor.isEmpty())
        {
            auto text = editor.getText();

            if (main->sendMessageToCurrentViewer (text))
            {
                editor.setText (String());
                history.add (text);
                indexInHistory = history.size();
            }
        }
    }
}

void KernelRuleEntry::textEditorEscapeKeyPressed (TextEditor&)
{
    if (auto main = findParentComponentOfClass<MainComponent>())
    {
        main->toggleKernelRuleEntryShown();
    }
}

void KernelRuleEntry::textEditorFocusLost (TextEditor&)
{
}




//=============================================================================
void KernelRuleEntry::setColours()
{
    editor.setColour (TextEditor::textColourId, findColour (AppLookAndFeel::statusBarText).brighter());
    editor.setColour (TextEditor::backgroundColourId, findColour (AppLookAndFeel::statusBarBackground));
    editor.setColour (TextEditor::highlightColourId, Colours::black.withAlpha (0.15f));
    editor.setColour (TextEditor::highlightedTextColourId, findColour (AppLookAndFeel::statusBarText).brighter());
    editor.setColour (TextEditor::focusedOutlineColourId, Colours::transparentBlack);
    editor.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
}




//=============================================================================
MainComponent::MainComponent()
{
    filePoller.setCallback ([this] (File) { reloadCurrentFile(); });
    directoryTree.addListener (this);
    directoryTree.getTreeView().setWantsKeyboardFocus (directoryTreeShowing);
    sourceList.addListener (this);
    environmentView.getListBox().setWantsKeyboardFocus (environmentViewShowing);
    statusBar.setCurrentViewerName ("Viewer List");

    viewers.addListener (this);
    viewers.add (std::make_unique<JsonFileViewer>());
    viewers.add (std::make_unique<ImageFileViewer>());
    viewers.add (std::make_unique<AsciiTableViewer>());
    viewers.add (std::make_unique<ColourMapViewer>());
    viewers.add (std::make_unique<PDFViewer>());
    viewers.add (std::make_unique<MetaYamlViewer> (this));

#if (JUCE_DEBUG == 0)
     viewers.loadFromYamlString (BinaryData::BinaryTorque_yaml);
     // viewers.loadFromYamlString (BinaryData::JetInCloud_yaml);
#warning("Loading hard-coded viewers")
#endif

    directoryTree.setName ("Directory");
    sourceList.setName ("Sources");
    sidebar.setComponent1 (&directoryTree);
    sidebar.setComponent2 (&sourceList);

    addAndMakeVisible (sidebar);
    addAndMakeVisible (environmentView);
    addAndMakeVisible (statusBar);
    addChildComponent (kernelRuleEntry);
    addChildComponent (userExtensionsDirectoryEditor);

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
    currentFile = newCurrentFile;
    filePoller.setFileToPoll (currentFile);

    if (currentViewer && currentViewer->isInterestedInFile (currentFile))
        currentViewer->loadFile (currentFile);
    else if (auto viewer = viewers.findViewerForFile (currentFile))
        makeViewerCurrent (viewer);
}

void MainComponent::reloadCurrentFile()
{
    if (currentViewer)
        currentViewer->reloadFile();
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
    environmentViewShowing = ! environmentViewShowing;
    environmentView.getListBox().setWantsKeyboardFocus (environmentViewShowing);
    layout (animated);
}

void MainComponent::toggleKernelRuleEntryShown()
{
    kernelRuleEntryShowing = ! kernelRuleEntryShowing;
    kernelRuleEntry.setVisible (kernelRuleEntryShowing);
    layout (false);

    if (isKernelRuleEntryShowing())
    {
        kernelRuleEntry.grabKeyboardFocus();
    }
}

void MainComponent::toggleUserExtensionsDirectoryEditor()
{
    if (userExtensionsDirectoryEditor.isVisible())
    {
        userExtensionsDirectoryEditor.setVisible (false);
    }
    else
    {
        userExtensionsDirectoryEditor.setVisible (true);
        userExtensionsDirectoryEditor.grabKeyboardFocus();
    }
}

bool MainComponent::hideExtraComponents()
{
    if (userExtensionsDirectoryEditor.isVisible())
    {
        userExtensionsDirectoryEditor.setVisible (false);
        return true;
    }
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
    return userExtensionsDirectoryEditor.isVisible();
}

File MainComponent::getCurrentDirectory() const
{
    return directoryTree.getCurrentDirectory();
}

File MainComponent::getCurrentFile() const
{
    return currentFile;
}

const Viewer* MainComponent::getCurrentViewer() const
{
    return currentViewer;
}

ViewerCollection& MainComponent::getViewerCollection()
{
    return viewers;
}

DirectoryTree& MainComponent::getDirectoryTree()
{
    return directoryTree;
}

void MainComponent::setCurrentViewer (const String& viewerName)
{
    if (auto viewer = viewers.findViewerWithName (viewerName))
    {
        makeViewerCurrent (viewer);
    }
}

bool MainComponent::isViewerSuitable (Viewer* viewer) const
{
    if (viewer == nullptr)
        return false;
    if (viewer->isInterestedInFile (currentFile))
        return true;
    return false;
}

void MainComponent::makeViewerCurrent (Viewer* viewer)
{
    if (viewer != currentViewer)
    {
        if (viewer)
        {
            viewer->loadFile (currentFile);
            statusBar.setCurrentViewerName (viewer->getViewerName());
            environmentView.setKernel (viewer->getKernel());
        }
        else
        {
            statusBar.setCurrentViewerName ("Viewer List");
            environmentView.setKernel (nullptr);
        }
        if (isKernelRuleEntryShowing() && (! viewer || ! viewer->canReceiveMessages()))
        {
            toggleKernelRuleEntryShown();
        }

        loadControlsForViewer (viewer);
        viewers.showOnly (viewer);
        currentViewer = viewer;
        PatchViewApplication::getApp().getCommandManager().commandStatusChanged();
    }
}

void MainComponent::loadControlsForViewer (Viewer* viewer)
{
    for (auto control : viewerControls)
    {
        removeChildComponent (control);
    }

    if (viewer)
    {
        viewerControls = viewer->getControls();

        for (auto control : viewerControls)
            addChildComponent (control);

        layout (false);
    }
}

void MainComponent::refreshCurrentViewerName()
{
    if (currentViewer)
        statusBar.setCurrentViewerName (currentViewer->getViewerName());
    else
        statusBar.setCurrentViewerName (String());
}

bool MainComponent::sendMessageToCurrentViewer (String& message)
{
    if (currentViewer)
    {
        return currentViewer->receiveMessage (message);
    }
    return false;
}

bool MainComponent::canSendMessagesToCurrentViewer() const
{
    if (currentViewer)
    {
        return currentViewer->canReceiveMessages();
    }
    return false;
}

void MainComponent::showKernelRule (const String& rule)
{
    if (currentViewer)
    {
        if (auto kernel = currentViewer->getKernel())
        {
            kernelRuleEntry.loadRule (rule.toStdString(), *kernel);
        }
    }
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
    auto target = File();

    if (toTempDirectory)
    {
        target = File::createTempFile (".mp4");
    }
    else
    {
        FileChooser chooser ("Choose Save Location...", currentFile.getParentDirectory(), "", true, false, nullptr);

        if (chooser.browseForFileToSave (true))
            target = chooser.getResult();
        else
            return;
    }

    FFMpegMovieWriter writer;
    writer.writeImagesToFile (sourceList.getAllImageAssets(), target);

    if (toTempDirectory)
    {
        target.startAsProcess();
    }
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
    currentFile = file;
    filePoller.setFileToPoll (currentFile);

    if (currentViewer && currentViewer->isInterestedInFile (currentFile))
        currentViewer->loadFile (currentFile);
    else if (auto viewer = viewers.findViewerForFile (file))
        makeViewerCurrent (viewer);
    else
        makeViewerCurrent (nullptr);
}

void MainComponent::sourceListWantsCaptureOfCurrent (SourceList*)
{
    for (auto source : sourceList.getSelectedSources())
    {
        setCurrentFile (source);

        if (currentViewer && currentViewer->isRenderingComplete())
        {
            auto capture = currentViewer->createViewerSnapshot();
            sourceList.setCaptureForSource (currentFile, capture);
            indicateSuccess ("Recorded snapshot for " + currentFile.getFileName());
        }
        else
        {
            logErrorMessage ("Cannot do a snapshot because viewer is still rendering...");
        }
    }
}




//=============================================================================
void MainComponent::figureMousePosition (Point<double> position)
{
    statusBar.setMousePositionInFigure (position);
}




//=============================================================================
void MainComponent::viewerAsyncTaskStarted (const String& name)
{
    statusBar.incrementAsyncTaskCount();
    statusBar.setCurrentInfoMessage ("Started " + name);
}

void MainComponent::viewerAsyncTaskCompleted (const String& name)
{
    statusBar.decrementAsyncTaskCount();
    statusBar.setCurrentInfoMessage ("Completed " + name);
}

void MainComponent::viewerAsyncTaskCancelled (const String& name)
{
    statusBar.decrementAsyncTaskCount();
}

void MainComponent::viewerLogErrorMessage (const String& what)
{
    statusBar.setCurrentErrorMessage (what);
}

void MainComponent::viewerIndicateSuccess()
{
    statusBar.setCurrentErrorMessage (String());
}

void MainComponent::viewerEnvironmentChanged()
{
    if (currentViewer)
    {
        environmentView.refresh();
        kernelRuleEntry.refresh (currentViewer->getKernel());
    }
}




//=============================================================================
void MainComponent::viewerCollectionViewerReconfigured (Viewer *viewer)
{
    statusBar.setCurrentInfoMessage ("Configure viewer " + viewer->getViewerName(), 3000);

    if (viewer == currentViewer)
        if (! isViewerSuitable (viewer))
            makeViewerCurrent (viewers.findViewerForFile (currentFile));

    if (viewer == currentViewer)
        loadControlsForViewer (viewer);
}

void MainComponent::viewerCollectionViewerAdded (Viewer *viewer)
{
    statusBar.setCurrentInfoMessage ("Load viewer: " + viewer->getViewerName(), 3000);
    addChildComponent (viewer, 0);
    layout (false);

    if (currentViewer == nullptr && isViewerSuitable (viewer))
        makeViewerCurrent (viewer);
}

void MainComponent::viewerCollectionViewerRemoved (Viewer *viewer)
{
    statusBar.setCurrentInfoMessage ("Unload viewer " + viewer->getViewerName(), 3000);

    if (viewer == currentViewer)
        makeViewerCurrent (nullptr);
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
    auto kernelRuleEntryArea = area.removeFromBottom (kernelRuleEntryShowing ? 32 : 0);
    auto controlsArea = area.removeFromBottom (viewerControls.isEmpty() ? 0 : 200);
    auto environmentViewArea = Rectangle<int> (0, 0, 300, 330)
    .withBottomY (statusBarArea.getY())
    .translated (0, environmentViewShowing ? 0 : 330 + 22); // the 22 is to ensure it's offscreen, so not painted

    setBounds (statusBar, statusBarArea);
    setBounds (sidebar, directoryTreeArea);
    setBounds (environmentView, environmentViewArea);
    setBounds (kernelRuleEntry, kernelRuleEntryArea);

    viewers.setBounds (area, animated);

    for (auto control : viewerControls)
    {
        setBounds (*control, controlsArea);
        control->setVisible (control == viewerControls.getFirst());
    }
    userExtensionsDirectoryEditor.setBounds (getLocalBounds().withSizeKeepingCentre (400, 300));
}
