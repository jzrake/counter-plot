#pragma once
#include "JuceHeader.h"
#include "VariantView.hpp"




//=============================================================================
class FileBasedView : public Component
{
public:
    virtual ~FileBasedView() {}

    /**
     * This method should return true if it looks like the given file can be
     * displayed. This does not need to be a guarantee that loading it will
     * succeed, but only an indication if loading should be attempted.
     */
    virtual bool isInterestedInFile (File file) const = 0;

    /**
     * This method should display the given file and return true, unless (1)
     * it fails to load the file, in which case it can throw an exception or
     * (2) it thinks that loading it might take a long time in which case it
     * should do nothing and return false. In that case the caller will try
     * again with the loadFileAsync method.
     */
    virtual bool loadFile (File fileToDisplay) = 0;

    /**
     * This method should display the given file. It will be invoked on a
     * background thread, so it should check the bailout callback periodically
     * to see if it's being asked to stop, and it must defer any changes to the
     * component hierarchy to an async updater (or MessageManager::callAsync).
     * This method is allowed to throw an exception if something went wrong
     * loading the file.
     */
    virtual void loadFileAsync (File fileToDisplay, std::function<bool()> bailout) {}
};




//=============================================================================
class JsonFileViewer : public FileBasedView
{
public:
    JsonFileViewer();
    bool isInterestedInFile (File file) const override;
    bool loadFile (File fileToDisplay) override;
    void resized() override;
private:
    VariantView view;
};




//=============================================================================
class ImageFileViewer : public FileBasedView
{
public:
    ImageFileViewer();
    bool isInterestedInFile (File file) const override;
    bool loadFile (File file) override;
    void resized() override;
private:
    ImageComponent view;
};
