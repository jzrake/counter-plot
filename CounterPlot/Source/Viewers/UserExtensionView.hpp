#pragma once
#include "Viewer.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Core/Runtime.hpp"




//=============================================================================
class UserExtensionView
: public Viewer
, public FigureView::Listener
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
    String getViewerName() const override;
    const Runtime::Kernel* getKernel() const override;

    //=========================================================================
    void figureViewSetDomainAndMargin (FigureView*, const Rectangle<double>&, const BorderSize<int>&) override;
    void figureViewSetMargin (FigureView*, const BorderSize<int>&) override;
    void figureViewSetDomain (FigureView*, const Rectangle<double>&) override;
    void figureViewSetXlabel (FigureView*, const String&) override;
    void figureViewSetYlabel (FigureView*, const String&) override;
    void figureViewSetTitle (FigureView*, const String&) override;

private:

    //=========================================================================
    int resolveKernel();

    //=========================================================================
    String viewerName;
    Grid layout;
    Runtime::Kernel kernel;
    OwnedArray<FigureView> figures;
    File currentFile;
};
