#pragma once
#include "JuceHeader.h"
#include "../Components/VariantTree.hpp"
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
        virtual void viewerAsyncTaskStarted (const String& name) = 0;
        virtual void viewerAsyncTaskCompleted (const String& name) = 0;
        virtual void viewerAsyncTaskCancelled (const String& name) = 0;
        virtual void viewerLogErrorMessage (const String& what) = 0;
        virtual void viewerIndicateSuccess() = 0;
        virtual void viewerEnvironmentChanged() = 0;
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
     * This method should display the given file. It may do nothing if the file
     * name has not changed.
     */
    virtual void loadFile (File fileToDisplay) = 0;

    /**
     * This method should reload the current file.
     */
    virtual void reloadFile() {}

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

    virtual void setMessageSink (MessageSink* explicitMessageSink) { messageSink = explicitMessageSink; }

    /**
     * Derived classes can call these method to send a message to the sink. This
     * is either the private messageSink instance if it is not null, or the
     * nearest parent that is a message sink, if it exists.
     */
    void sendAsyncTaskStarted (const String& name) const;
    void sendAsyncTaskCompleted (const String& name) const;
    void sendAsyncTaskCancelled (const String& name) const;
    void sendErrorMessage (const String& what) const;
    void sendIndicateSuccess() const;
    void sendEnvironmentChanged() const;

private:
    MessageSink* messageSink = nullptr;
};




//=============================================================================
class JsonFileViewer : public Viewer
{
public:

    //=========================================================================
    JsonFileViewer();
    bool isInterestedInFile (File file) const override;
    void loadFile (File file) override;
    void reloadFile() override;
    String getViewerName() const override { return "JSON / YAML Viewer"; }

    //=========================================================================
    void resized() override;

private:
    File currentFile;
    VariantTree view;
};




//=============================================================================
class ImageFileViewer : public Viewer
{
public:

    //=========================================================================
    ImageFileViewer();
    bool isInterestedInFile (File file) const override;
    void loadFile (File file) override;
    void reloadFile() override;
    String getViewerName() const override { return "Image"; }

    //=========================================================================
    void resized() override;

private:
    File currentFile;
    ImageComponent view;
};




//=============================================================================
class PDFViewer : public Viewer
{
public:

    //=========================================================================
    PDFViewer();
    ~PDFViewer();
    bool isInterestedInFile (File file) const override;
    void loadFile (File file) override;
    void reloadFile() override;
    String getViewerName() const override { return "PDF Document"; }

    //=========================================================================
    void resized() override;

private:
    File currentFile;
    PDFViewComponent pdfView;
};
