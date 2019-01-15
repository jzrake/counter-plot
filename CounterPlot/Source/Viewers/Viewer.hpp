#pragma once
#include "JuceHeader.h"
#include "../Components/VariantView.hpp"
#include "../Core/Runtime.hpp"




//=============================================================================
class Viewer : public Component
{
public:

    //=========================================================================
    class MessageSink
    {
    public:
        virtual ~MessageSink() {}
        virtual void viewerAsyncTaskStarted() = 0;
        virtual void viewerAsyncTaskFinished() = 0;
        virtual void viewerLogErrorMessage (const String& what) = 0;
        virtual void viewerIndicateSuccess() = 0;
    };

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
     * Derived classes can call these method to find the nearest parent that is a message
     * sink and call the appropriate method.
     */
    void sendErrorMessage (const String& what) const;
    void sendIndicateSuccess() const;

    /**
     * Destructor.
     */
    virtual ~Viewer() {}

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

    /**
     * If this viewer uses a crt::kernel for state management, it should return it
     * in this method. The kernel might be queried by parent component to create
     * a view of its contents.
     */
    virtual const Runtime::Kernel* getKernel() const { return nullptr; }
};




//=============================================================================
class JsonFileViewer : public Viewer
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
class ImageFileViewer : public Viewer
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
