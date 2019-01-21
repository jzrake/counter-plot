#pragma once
#include "Viewer.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Core/Runtime.hpp"
#include "../Core/ConfigurableFileFilter.hpp"
#include "../Core/TaskPool.hpp"




//=============================================================================
class UserExtensionView
: public Viewer
, public FigureView::Listener
, public TaskPool::Listener
{
public:

    //=========================================================================
    UserExtensionView();
    void configure (const var& config);
    void configure (File file);

    //=========================================================================
    void resized() override;

    //=========================================================================
    bool isInterestedInFile (File file) const override;
    void loadFile (File fileToDisplay) override;
    void reloadFile() override;
    String getViewerName() const override;
    const Runtime::Kernel* getKernel() const override;

    //=========================================================================
    void figureViewSetMargin (FigureView*, const BorderSize<int>&) override;
    void figureViewSetDomain (FigureView*, const Rectangle<double>&) override;
    void figureViewSetXlabel (FigureView*, const String&) override;
    void figureViewSetYlabel (FigureView*, const String&) override;
    void figureViewSetTitle (FigureView*, const String&) override;

    //=========================================================================
    void taskStarted (const String& taskName) override;
    void taskCompleted (const String& taskName, const var& result, const std::string& error) override;
    void taskWasCancelled (const String& taskName) override;

private:

    //=========================================================================
    void applyLayout();
    void resolveKernel();
    void loadFromKernelIfFigure (const std::string& id);
    void loadExpressionsFromListIntoKernel (Runtime::Kernel& kernel, const var& list, const std::string& basename) const;
    void loadExpressionsFromDictIntoKernel (Runtime::Kernel& kernel, const var& dict) const;

    //=========================================================================
    String viewerName;
    Grid layout;
    ColourMapCollection colourMaps;
    ConfigurableFileFilter fileFilter;
    Runtime::Kernel kernel;
    OwnedArray<FigureView> figures;
    File currentFile;
    TaskPool taskPool;
    StringArray asyncRules;
};
