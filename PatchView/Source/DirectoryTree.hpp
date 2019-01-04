#pragma once
#include "JuceHeader.h"




//==============================================================================
class DirectoryTree : public Component
{
public:

    //==========================================================================
    DirectoryTree();
    ~DirectoryTree();
    void setDirectoryToShow (File directoryToShow);

    //==========================================================================
    void resized() override;
    void mouseEnter (const MouseEvent& e) override;
    void mouseExit (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;

private:
    //==========================================================================
    void setMouseOverItem (TreeViewItem*);
    class Item;
    TreeView tree;
    std::unique_ptr<Item> root;
    TreeViewItem* mouseOverItem = nullptr;
    File currentDirectory;
};

