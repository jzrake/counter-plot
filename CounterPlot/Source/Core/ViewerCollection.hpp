#pragma once
#include "JuceHeader.h"
#include "../Viewers/Viewer.hpp"




//=============================================================================
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
        virtual void viewerCollectionViewerReconfigured (Viewer*) = 0;
        virtual void viewerCollectionViewerAdded (Viewer*) = 0;
        virtual void viewerCollectionViewerRemoved (Viewer*) = 0;
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


    Array<File> getWatchedDirectories() const;


    void setWatchedDirectories (const Array<File>& directoriesToWatch);


    bool isExtensionViewerLoaded (File source) const;


    bool watchesDirectory (File directory) const;


    void startWatchingDirectory (File directory);


    void stopWatchingDirectory (File directory);


    /**
     * Create a UserExtensionView for each of the .yaml files in the given
     * directory.
     */
    void loadAllInDirectory (File directory, Viewer::MessageSink* messageSink=nullptr);


    void unloadAllInDirectory (File directory);


    void unloadAllNonexistentInDirectory (File directory);


    /**
     * Load a viewer configuration from a valid YAML string. Exceptions are not caught
     * within the function, so be sure you are ready to catch them if the source isn't
     * something you've already verified.
     */
    void loadFromYamlString (const String& source, Viewer::MessageSink* messageSink=nullptr);

    /**
     * Return the first viewer that is interested in the given file, or nullptr
     * if none exists. If multiple viewers are interested, the one added most
     * recently is returned.
     */
    Viewer* findViewerForFile (File file) const;


    /**
     * Return the most recently added viewer with the given name, or nullptr if
     * none exists.
     */
    Viewer* findViewerWithName (const String& viewerName) const;


    /**
     * Return an array of the viewer components.
     */
    Array<Viewer*> getAllComponents() const;


    /**
     * Set the bounds of any visible viewers to the given rectangle. This also
     * caches the bounds rectangle, and will apply it to the next viewer given
     * to the showOnly method.
     */
    void setBounds (const Rectangle<int>& newBounds, bool animated=false);


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

    struct ExtensionDirectory
    {
        File directory;
        Time lastLoaded;
    };

    Array<Item> items;
    Array<ExtensionDirectory> extensionDirectories;
    Rectangle<int> bounds;
    ListenerList<Listener> listeners;
};
