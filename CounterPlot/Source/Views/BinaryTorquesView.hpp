#pragma once
#include "JuceHeader.h"
#include "FigureView.hpp"
#include "FileBasedView.hpp"




//=============================================================================
class BinaryTorquesView
: public FileBasedView
, public FigureView::Listener
, public ApplicationCommandTarget
{
public:

    //=========================================================================
    BinaryTorquesView();
    ~BinaryTorquesView();

    //=========================================================================
    bool isInterestedInFile (File file) const override;
    bool loadFile (File fileToDisplay) override;
    void loadFileAsync (File fileToDisplay, std::function<bool()> bailout) override;

    //=========================================================================
    void resized() override;

    //=========================================================================
    void figureViewSetMargin (FigureView*, const BorderSize<int>&) override;
    void figureViewSetDomain (FigureView*, const Rectangle<double>&) override;
    void figureViewSetXlabel (FigureView*, const String&) override;
    void figureViewSetYlabel (FigureView*, const String&) override;
    void figureViewSetTitle (FigureView*, const String&) override;

    //=========================================================================
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

private:
    //=========================================================================
	class QuadmeshArtist;
    void updateFigures();
    void saveSnapshot (bool toTempDirectory);

    //=========================================================================
    std::shared_ptr<ColourGradientArtist> gradient;
    std::shared_ptr<QuadmeshArtist> quadmesh;
    FigureModel mainModel;
    FigureModel cmapModel;
    FigureView mainFigure;
    FigureView cmapFigure;
    Grid layout;
    File currentFile;
    ColourMapCollection cmaps;
};
