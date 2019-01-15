#pragma once
#include "JuceHeader.h"




//=============================================================================
class Viewer;
class UserExtensionView;




//=============================================================================
class ViewerCollection : public Timer
{
public:

    //=========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void extensionViewerReconfigured (UserExtensionView*) = 0;
    };

    //=========================================================================
    ViewerCollection();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    void add (std::unique_ptr<Viewer> viewerToAdd);
    void clear();
    void loadAllInDirectory (File directory);
    Viewer* findViewerForFile (File file) const;
    Array<Viewer*> getAllComponents() const;
    void setBounds (const Rectangle<int>& bounds) const;
    void showOnly (Viewer* componentThatShouldBeVisible) const;

private:

    //=========================================================================
    void timerCallback() override;

    //=========================================================================
    struct Item
    {
        bool isExtension = false;
        File source;
        Time lastLoaded;
        std::unique_ptr<Viewer> viewer;
    };
    Array<Item> items;
    ListenerList<Listener> listeners;
};
