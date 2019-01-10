#pragma once
#include "JuceHeader.h"
#include "VariantView.hpp"




//=============================================================================
class FileBasedView : public Component
{
public:

    /**
     * Common commands that views might typically respond to. Views that are
     * ApplicationCommandTarget's should return the subset of these which they
     * perform in the getAllCommands methods.
     */
    enum Commands
    {
        makeSnapshotAndOpen = 0x0213001,
        saveSnapshotAs      = 0x0213002,
        nextColourMap       = 0x0213003,
        prevColourMap       = 0x0213004,
        resetScalarRange    = 0x0213005,
    };

    /**
     * This returns the list of the above commands.
     */
    static void getAllCommands (Array<CommandID>& commands);

    /**
     * This registers all above the commands with the given command manager.
     */
    static void registerCommands (ApplicationCommandManager& manager);

    /**
     * Subclass views can defer to this method to get command descriptions.
     */
    static void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);

    /**
     * Destructor.
     */
    virtual ~FileBasedView() {}

    /**
     * This method should return true if it looks like the given file can be
     * displayed. This does not need to be a guarantee that loading it will
     * succeed, but only an indication if loading should be attempted.
     */
    virtual bool isInterestedInFile (File file) const = 0;

    /**
     * This method should display the given file.
     */
    virtual void loadFile (File fileToDisplay) = 0;

    /**
     * This method must return a name for this viewer. The name should be in the
     * following format:
     *
     * Name of This Viewer
     *
     * and it should be specific enough not to clash with the names of other possible
     * viewers.
     */
    virtual String getViewerName() const = 0;
};




//=============================================================================
class JsonFileViewer : public FileBasedView
{
public:

    //=========================================================================
    JsonFileViewer();
    bool isInterestedInFile (File file) const override;
    void loadFile (File fileToDisplay) override;
    String getViewerName() const override { return "JSON"; }

    //=========================================================================
    void resized() override;
private:
    VariantView view;
};




//=============================================================================
class ImageFileViewer : public FileBasedView
{
public:

    //=========================================================================
    ImageFileViewer();
    bool isInterestedInFile (File file) const override;
    void loadFile (File file) override;
    String getViewerName() const override { return "Image"; }

    //=========================================================================
    void resized() override;
private:
    ImageComponent view;
};
