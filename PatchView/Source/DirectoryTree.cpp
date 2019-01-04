#include "DirectoryTree.hpp"




//=============================================================================
class DirectoryTree::Item : public TreeViewItem
{
public:
    Item (DirectoryTree& directory, File file) : directory (directory), file (file)
    {
        setDrawsInLeftMargin (true);
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        g.setColour (isMouseOver() ? Colours::white : Colours::lightgrey);
        g.drawText (file.getFileName(), 0, 0, width, height, Justification::centredLeft);
    }

    bool mightContainSubItems() override
    {
        return file.isDirectory();
    }

    bool canBeSelected() const override
    {
        return true;
    }

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
            for (auto child : file.findChildFiles (File::findFilesAndDirectories, false))
            {
                if (! child.isHidden())
                {
                    addSubItem (new Item (directory, child));
                }
            }
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

    DirectoryTree& directory;
    File file;
};




//=============================================================================
DirectoryTree::DirectoryTree()
{
    tree.setRootItemVisible (true);
    tree.addMouseListener (this, true);
    addAndMakeVisible (tree);
}

DirectoryTree::~DirectoryTree()
{
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
    setMouseOverItem (nullptr);
    tree.setRootItem (nullptr);
    root = std::make_unique<Item> (*this, currentDirectory = directoryToShow);
    tree.setRootItem (root.get());
}

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
