#pragma once
#include "JuceHeader.h"
#include "FigureView.hpp"
#include "FileBasedView.hpp"




//=============================================================================
class BinaryTorquesView : public FileBasedView, public FigureView::Listener, public ApplicationCommandTarget
{
public:

    // TODO: move these commands to a more general command pool
    enum Commands {
        makeSnapshotAndOpen = 0x0213001,
        saveSnapshotAs      = 0x0213002,
        nextColourMap       = 0x0213003,
        prevColourMap       = 0x0213004,
        resetScalarRange    = 0x0213005,
    };

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

    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

private:
	class QuadmeshArtist;
    void mutateFigure (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void mutateFiguresInRow (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void mutateFiguresInCol (FigureView* eventFigure, std::function<void(FigureModel&)> mutation);
    void reloadFigures();
    void saveSnapshot (bool toTempDirectory);

    std::array<float, 2> scalarExtent;
    Grid layout;
    OwnedArray<FigureView> figures;
    std::shared_ptr<QuadmeshArtist> artist;
    ColourMapCollection cmaps;
    File currentFile;
};
