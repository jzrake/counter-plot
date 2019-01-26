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

    /**
     * Constructor.
     */
    ViewerCollection();


    /**
     * Add a listener. Listeners are notified whenever new extension views are
     * added and removed, or when their source code changes and their configure
     * method is called.
     */
    void addListener (Listener* listener);


    /**
     * Remove a listener.
     */
    void removeListener (Listener* listener);


    /**
     * Add a new viewer to the collection. Calls viewerCollectionViewerAdded
     * after the extension is actually loaded.
     */
    void add (std::unique_ptr<Viewer> viewerToAdd);


    /**
     * Remove all viewers from the collection. viewerCollectionViewerRemoved
     * is called for each extension before it is actually deleted.
     */
    void clear();


    /**
     * Return an array of the files which are being watched.
     */
    Array<File> getWatchedDirectories() const;


    /**
     * Return an XML element of the watched extension directories. Helpful for
     * persistence.
     */
    std::unique_ptr<XmlElement> getWatchedDirectoriesAsXml() const;


    /**
     * Set the directories that should be monitored for user extension views.
     * This method diffs the given list of directories with those already
     * watched, calling startWatchingDirectory and stopWatchingDirectory
     * appropriately.
     */
    void setWatchedDirectories (const Array<File>& directoriesToWatch);


    /**
     * Same as above, but can be called with an XML element like that returned
     * from getWatchedDirectoriesAsXml. 
     */
    void setWatchedDirectories (const XmlElement& directoriesToWatch);


    /**
     * Return true if the given file is currently loaded as an extension.
     */
    bool isExtensionViewerLoaded (File source) const;


    /**
     * Return true if the given directory is currently watched for extensions.
     */
    bool watchesDirectory (File directory) const;


    /**
     * Load all the extensions in the given directory, and keep them in sync.
     * Removing a file from that directory will trigger the extension to be
     * unloaded, and if a new one is added, it will be loaded.
     */
    void startWatchingDirectory (File directory);


    /**
     * Stop watching the given directory if it is currently watched, and
     * unload any extensions it contained.
     */
    void stopWatchingDirectory (File directory);


    /**
     * Load a viewer configuration from a valid YAML string. Exceptions are not caught
     * within the function, so be sure you are ready to catch them if the source isn't
     * something you've already verified.
     */
    void loadFromYamlString (const String& source);

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
    void loadAllInDirectory (File directory);
    void unloadAllInDirectory (File directory);
    void unloadAllNonexistentInDirectory (File directory);

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
