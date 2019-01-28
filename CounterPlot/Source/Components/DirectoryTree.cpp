#include "DirectoryTree.hpp"
#include "../Core/LookAndFeel.hpp"




//=============================================================================
class DirectoryTree::Item : public TreeViewItem
{
public:


    //=========================================================================
    class ElementComparator
    {
    public:
        static int compareElements (File& f1, File& f2)
        {
            return DefaultElementComparator<String>::compareElements (f1.getFileName(), f2.getFileName());
        }
    };


    //=========================================================================
    Item (DirectoryTree& directory, File file) : directory (directory), file (file)
    {
        isDirectory = file.isDirectory();
        setDrawsInLeftMargin (true);
        refreshLook (false);
    }

    void refreshLook (bool recursively)
    {
        Font font;

        if (auto laf = dynamic_cast<AppLookAndFeel*> (&getOwnerView()->getLookAndFeel()))
            font = laf->getDefaultFont();

        auto text = file.getFileName();
        itemHeight = font.getHeight() * 11 / 5;

        glyphs.clear();
        glyphs.addLineOfText (font, text, 0, 0);
        glyphs.justifyGlyphs (0, text.length(), 0, 0, 1e10, itemHeight, Justification::centredLeft);

        if (recursively)
            for (int n = 0; n < getNumSubItems(); ++n)
                dynamic_cast<Item*> (getSubItem(n))->refreshLook (true);
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        Colour textColour;

        if (file.isDirectory())    textColour = getOwnerView()->findColour (AppLookAndFeel::directoryTreeDirectory);
        if (file.existsAsFile())   textColour = getOwnerView()->findColour (AppLookAndFeel::directoryTreeFile);
        if (file.isSymbolicLink()) textColour = getOwnerView()->findColour (AppLookAndFeel::directoryTreeSymbolicLink);

        g.setColour (isMouseOver() ? textColour.brighter (0.8f) : textColour);
        glyphs.draw (g);
    }

    bool mightContainSubItems() override
    {
        return isDirectory;
    }

    bool canBeSelected() const override
    {
        return true;
    }

    int getItemHeight() const override
    {
        return itemHeight;
    }

    String getUniqueName() const override
    {
        return file.getFullPathName();
    }

    void itemSelectionChanged (bool isNowSelected) override
    {
        if (isNowSelected)
        {
            directory.sendSelectedFilesChanged();
        }
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
        {
            auto dirs = file.findChildFiles (File::findDirectories, false);
            dirs.sort (comparator);

            auto files = file.findChildFiles (File::findFiles, false);
            files.sort (comparator);

            for (auto child : dirs)
                if (! child.isHidden())
                    addSubItem (new Item (directory, child));

            for (auto child : files)
                if (! child.isHidden())
                    addSubItem (new Item (directory, child));
        }
        else
        {
            directory.setMouseOverItem (nullptr);
            clearSubItems();
        }
    }

    void itemClicked (const MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            PopupMenu menu;
            menu.addItem (1, "Add as Source");
            switch (menu.show())
            {
                case 1: directory.sendSelectedFilesAsSources(); break;
            }
        }
    }

private:
    bool isMouseOver() const
    {
        return directory.mouseOverItem == this;
    }

    friend class DirectoryTree;
    int itemHeight = 24;
    GlyphArrangement glyphs;
    DirectoryTree& directory;
    File file;
    bool isDirectory = false; // cache for performance
    ElementComparator comparator;
};




//=============================================================================
DirectoryTree::DirectoryTree()
{
    tree.setIndentSize (20);
    tree.setRootItemVisible (true);
    tree.setMultiSelectEnabled (true);
    tree.addMouseListener (this, true);
    tree.getViewport()->setWantsKeyboardFocus (false);
    setColours();
    addAndMakeVisible (tree);
}

DirectoryTree::~DirectoryTree()
{
    tree.setRootItem (nullptr);
}

void DirectoryTree::addListener (Listener* listener)
{
    listeners.add (listener);
}

void DirectoryTree::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void DirectoryTree::setDirectoryToShow (File directoryToShow)
{
    if (currentDirectory != directoryToShow)
    {
        currentDirectory = directoryToShow;
        reloadAll();
    }
}

void DirectoryTree::reloadAll()
{
    auto state = std::unique_ptr<XmlElement> (root ? root->getOpennessState() : nullptr);

    setMouseOverItem (nullptr);
    tree.setRootItem (nullptr);
    root = std::make_unique<Item> (*this, currentDirectory);
    tree.setRootItem (root.get());

    if (state)
        root->restoreOpennessState (*state);
    else
        root->setOpen (true);
}

File DirectoryTree::getCurrentDirectory() const
{
    return currentDirectory;
}

std::unique_ptr<XmlElement> DirectoryTree::getRootOpennessState() const
{
    return std::unique_ptr<XmlElement> (root ? root->getOpennessState() : nullptr);
}

void DirectoryTree::restoreRootOpenness (const XmlElement& state)
{
    root->restoreOpennessState (state);
}




//=============================================================================
void DirectoryTree::resized()
{
    tree.setBounds (getLocalBounds());
}

void DirectoryTree::mouseEnter (const MouseEvent& e)
{
    setMouseOverItem (tree.getItemAt (e.position.y - tree.getViewport()->getViewPositionY()));
}

void DirectoryTree::mouseExit (const MouseEvent& e)
{
    setMouseOverItem (nullptr);
}

void DirectoryTree::mouseMove (const MouseEvent& e)
{
    setMouseOverItem (tree.getItemAt (e.position.y - tree.getViewport()->getViewPositionY()));
}

bool DirectoryTree::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::spaceKey)
    {
        sendSelectedFilesAsSources();
        return true;
    }

    if (key == KeyPress (KeyPress::downKey, ModifierKeys::shiftModifier, 0))
        if (auto item = tree.getSelectedItem (tree.getNumSelectedItems() - 1))
            if (auto target = tree.getItemOnRow (item->getRowNumberInTree() + 1))
                return static_cast<void> (target->setSelected (true, false)), true;

    if (key == KeyPress (KeyPress::upKey, ModifierKeys::shiftModifier, 0))
        if (auto item = tree.getSelectedItem (0))
            if (auto target = tree.getItemOnRow (item->getRowNumberInTree() - 1))
                return static_cast<void> (target->setSelected (true, false)), true;

    return false;
}

void DirectoryTree::colourChanged()
{
    setColours();
    repaint();
}

void DirectoryTree::lookAndFeelChanged()
{
    root->refreshLook (true);
    root->treeHasChanged();
    setColours();
    repaint();
}




//=============================================================================
void DirectoryTree::handleAsyncUpdate()
{
    if (tree.getNumSelectedItems() == 1)
        listeners.call (&Listener::directoryTreeSelectedFileChanged, this, dynamic_cast<Item*> (tree.getSelectedItem(0))->file);
    else
        listeners.call (&Listener::directoryTreeSelectedFileChanged, this, File());
}




//=============================================================================
void DirectoryTree::sendSelectedFilesAsSources()
{
    for (int n = 0; n < tree.getNumSelectedItems(); ++n)
    {
        auto item = dynamic_cast<Item*> (tree.getSelectedItem(n));
        listeners.call (&Listener::directoryTreeWantsFileToBeSource, this, item->file);
    }
}

void DirectoryTree::sendSelectedFilesChanged()
{
    triggerAsyncUpdate();
}

void DirectoryTree::setMouseOverItem (TreeViewItem *newMouseOverItem)
{
    if (mouseOverItem) mouseOverItem->repaintItem();
    if (newMouseOverItem) newMouseOverItem->repaintItem();
    mouseOverItem = newMouseOverItem;
}

void DirectoryTree::setColours()
{
    tree.setColour (TreeView::backgroundColourId, findColour (AppLookAndFeel::directoryTreeBackground));
    tree.setColour (TreeView::selectedItemBackgroundColourId, findColour (AppLookAndFeel::directoryTreeSelectedItem));
    tree.setColour (TreeView::dragAndDropIndicatorColourId, Colours::green);
    tree.setColour (TreeView::evenItemsColourId, Colours::transparentBlack);
    tree.setColour (TreeView::oddItemsColourId, Colours::transparentBlack);
    tree.setColour (TreeView::linesColourId, Colours::red);
}
