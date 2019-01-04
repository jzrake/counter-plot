#pragma once
#include "JuceHeader.h"




//==============================================================================
class DirectoryTree : public Component
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void selectedFileChanged (DirectoryTree*, File) = 0;
    };

    //==========================================================================
    DirectoryTree();
    ~DirectoryTree();
    void addListener (Listener*);
    void removeListener (Listener*);
    void setDirectoryToShow (File directoryToShow);

    //==========================================================================
    void resized() override;
    void mouseEnter (const MouseEvent& e) override;
    void mouseExit (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;

private:
    //==========================================================================
    void setMouseOverItem (TreeViewItem*);
    void sendSelectedFileChanged (File);
    class Item;
    friend class Item;
    TreeView tree;
    std::unique_ptr<Item> root;
    TreeViewItem* mouseOverItem = nullptr;
    File currentDirectory;
    ListenerList<Listener> listeners;
};
