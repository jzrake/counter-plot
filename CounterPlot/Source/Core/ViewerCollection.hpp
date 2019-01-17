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


    /**
     * Add a new viewer to the collection.
     */
    void add (std::unique_ptr<Viewer> viewerToAdd);


    /**
     * Remove all viewers from the collection.
     */
    void clear();


    /**
     * Create a UserExtensionView for each of the .yaml files in the given
     * directory.
     */
    void loadAllInDirectory (File directory);


    /**
     * Return the first viewer that is interested in the given file, or nullptr
     * if none exists.
     */
    Viewer* findViewerForFile (File file) const;


    /**
     * Return the first viewer with the given name, or nullptr if none exists.
     */
    Viewer* findViewerWithName (const String& viewerName) const;


    /**
     * Return an array of the viewer components.
     */
    Array<Viewer*> getAllComponents() const;


    /**
     * Set the bounds of all viewers to the given rectangle.
     */
    void setBounds (const Rectangle<int>& bounds) const;


    /**
     * Set the visibility of all viewers to be false, except for the one
     * specified.
     */
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
