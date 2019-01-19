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
        setDrawsInLeftMargin (true);
        glyphs.addLineOfText (Font().withHeight (11), file.getFileName(), 8, 16);
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        Colour textColour;

        if (file.isDirectory())    textColour = getOwnerView()->findColour (LookAndFeelHelpers::directoryTreeDirectory);
        if (file.existsAsFile())   textColour = getOwnerView()->findColour (LookAndFeelHelpers::directoryTreeFile);
        if (file.isSymbolicLink()) textColour = getOwnerView()->findColour (LookAndFeelHelpers::directoryTreeSymbolicLink);

        g.setColour (isMouseOver() ? textColour.brighter (0.8f) : textColour);
        glyphs.draw (g);
    }

    bool mightContainSubItems() override
    {
        return file.isDirectory();
    }

    bool canBeSelected() const override
    {
        return true;
    }

    int getItemHeight() const override { return 24; }

    void itemSelectionChanged (bool isNowSelected) override
    {
        if (isNowSelected)
        {
            directory.sendSelectedFileChanged (file);
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

private:
    bool isMouseOver() const
    {
        return directory.mouseOverItem == this;
    }

    GlyphArrangement glyphs;
    DirectoryTree& directory;
    File file;
    ElementComparator comparator;
};




//=============================================================================
DirectoryTree::DirectoryTree()
{
    tree.setIndentSize (12);
    tree.setRootItemVisible (true);
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
    setMouseOverItem (nullptr);
    tree.setRootItem (nullptr);
    root = std::make_unique<Item> (*this, currentDirectory);
    tree.setRootItem (root.get());
    root->setOpen (true);
}

File DirectoryTree::getCurrentDirectory() const
{
    return currentDirectory;
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

void DirectoryTree::colourChanged()
{
    setColours();
    repaint();
}

void DirectoryTree::lookAndFeelChanged()
{
    setColours();
    repaint();
}

void DirectoryTree::setMouseOverItem (TreeViewItem *newMouseOverItem)
{
    if (mouseOverItem) mouseOverItem->repaintItem();
    if (newMouseOverItem) newMouseOverItem->repaintItem();
    mouseOverItem = newMouseOverItem;
}

void DirectoryTree::sendSelectedFileChanged (juce::File file)
{
    listeners.call (&Listener::selectedFileChanged, this, file);
}

void DirectoryTree::setColours()
{
    tree.setColour (TreeView::backgroundColourId, findColour (LookAndFeelHelpers::directoryTreeBackground));
    tree.setColour (TreeView::selectedItemBackgroundColourId, findColour (LookAndFeelHelpers::directoryTreeSelectedItem));
    tree.setColour (TreeView::dragAndDropIndicatorColourId, Colours::green);
    tree.setColour (TreeView::evenItemsColourId, Colours::transparentBlack);
    tree.setColour (TreeView::oddItemsColourId, Colours::transparentBlack);
    tree.setColour (TreeView::linesColourId, Colours::red);
}
