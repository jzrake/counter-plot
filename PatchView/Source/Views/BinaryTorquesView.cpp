#include "BinaryTorquesView.hpp"
#include "../MetalSurface.hpp"




//=============================================================================
class BinaryTorquesView::QuadmeshArtist : public PlotArtist
{
public:
    QuadmeshArtist (nd::array<double, 2> data, std::function<bool()> bailout)
    {
        auto ni = data.shape(0);
        auto nj = data.shape(1);
        triangleVertices.reserve (data.size() * 6);
        triangleScalars .reserve (data.size() * 6);
        scalarExtent[0] = data (0, 0);
        scalarExtent[1] = data (0, 0);

        for (int i = 0; i < ni; ++i)
        {
            for (int j = 0; j < nj; ++j)
            {
                const float x0 = -8.f + (i + 0) * 16.f / ni;
                const float y0 = -8.f + (j + 0) * 16.f / nj;
                const float x1 = -8.f + (i + 1) * 16.f / ni;
                const float y1 = -8.f + (j + 1) * 16.f / nj;
                const float c = std::log10f (data (i, j));

                triangleVertices.push_back (simd::float2 {x0, y0});
                triangleVertices.push_back (simd::float2 {x0, y1});
                triangleVertices.push_back (simd::float2 {x1, y0});
                triangleVertices.push_back (simd::float2 {x0, y1});
                triangleVertices.push_back (simd::float2 {x1, y0});
                triangleVertices.push_back (simd::float2 {x1, y1});

                triangleScalars.push_back (simd::float1 (c));
                triangleScalars.push_back (simd::float1 (c));
                triangleScalars.push_back (simd::float1 (c));
                triangleScalars.push_back (simd::float1 (c));
                triangleScalars.push_back (simd::float1 (c));
                triangleScalars.push_back (simd::float1 (c));

                if (c < scalarExtent[0]) scalarExtent[0] = c;
                if (c > scalarExtent[1]) scalarExtent[1] = c;

                if (bailout())
                {
                    return;
                }
            }
        }

        mapping.stops = ColourMapHelpers::coloursFromRGBTable (BinaryData::magma_cmap);
        mapping.vmin = scalarExtent[0];
        mapping.vmax = scalarExtent[1];
    }

    void setColorMap (const Array<Colour>& stops)
    {
        mapping.stops = stops;
    }

    void setScalarDomain (float vmin, float vmax)
    {
        mapping.vmin = vmin;
        mapping.vmax = vmax;
    }

    void render (RenderingSurface& surface) override { surface.renderTriangles (triangleVertices, triangleScalars, mapping); }
    bool isScalarMappable() const override { return true; }
    ScalarMapping getScalarMapping() const override { return mapping; }
    std::array<float, 2> getScalarExtent() const override { return scalarExtent; }

private:
    std::array<float, 2> scalarExtent;
    ScalarMapping mapping;
    std::vector<simd::float2> triangleVertices;
    std::vector<simd::float1> triangleScalars;
};




//=============================================================================
BinaryTorquesView::BinaryTorquesView()
{
    cmaps.add ("cividis",  ColourMapHelpers::coloursFromRGBTable (BinaryData::cividis_cmap));
    cmaps.add ("dawn",     ColourMapHelpers::coloursFromRGBTable (BinaryData::dawn_cmap));
    cmaps.add ("fire",     ColourMapHelpers::coloursFromRGBTable (BinaryData::fire_cmap));
    cmaps.add ("inferno",  ColourMapHelpers::coloursFromRGBTable (BinaryData::inferno_cmap));
    cmaps.add ("magma",    ColourMapHelpers::coloursFromRGBTable (BinaryData::magma_cmap));
    cmaps.add ("plasma",   ColourMapHelpers::coloursFromRGBTable (BinaryData::plasma_cmap));
    cmaps.add ("seashore", ColourMapHelpers::coloursFromRGBTable (BinaryData::seashore_cmap));
    cmaps.add ("viridis",  ColourMapHelpers::coloursFromRGBTable (BinaryData::viridis_cmap));

    layout.templateRows    = { Grid::TrackInfo (1_fr) };
    layout.templateColumns = { Grid::TrackInfo (1_fr), Grid::TrackInfo (80_px) };

    figures.add (new FigureView);
    figures.add (new FigureView);

    FigureModel mainModel;
    mainModel.margin.setRight (20);
    mainModel.canEditTitle = false;
    mainModel.canEditXlabel = false;
    mainModel.canEditYlabel = false;
    mainModel.xlabel = "X";
    mainModel.ylabel = "Y";
    mainModel.xmin = -3;
    mainModel.xmax = +3;
    mainModel.ymin = -3;
    mainModel.ymax = +3;
    figures[0]->setModel (mainModel);
    figures[0]->setRenderingSurface (std::make_unique<MetalRenderingSurface>());

    FigureModel colorbarModel;
    colorbarModel.titleShowing = false;
    colorbarModel.xlabelShowing = false;
    colorbarModel.ylabelShowing = false;
    colorbarModel.margin.setLeft (40);
    colorbarModel.margin.setRight (20);
    colorbarModel.gridlinesColour = Colours::transparentBlack;
    colorbarModel.xtickCount = 0;
    colorbarModel.canEditMargin = false;
    figures[1]->setModel (colorbarModel);

    for (const auto& figure : figures)
    {
        figure->addListener (this);
        addAndMakeVisible (figure);
        layout.items.add (figure->getGridItem());
    }
    setWantsKeyboardFocus (true);
}

BinaryTorquesView::~BinaryTorquesView()
{
}

bool BinaryTorquesView::loadFile (File viewedDocument)
{
    return false;
}

void BinaryTorquesView::loadFileAsync (File viewedDocument, std::function<bool()> bailout)
{
    auto h5f = h5::File (viewedDocument.getFullPathName().toStdString());
    auto sig = h5f.open_dataset ("primitive/sigma");
    auto dat = sig.read<nd::array<double, 2>>();

    artist = std::make_shared<QuadmeshArtist> (dat, bailout);

    if (bailout())
    {
        return;
    }
    scalarExtent = artist->getScalarExtent();
    currentFile = viewedDocument;

    MessageManager::callAsync ([viewedDocument, self = SafePointer<BinaryTorquesView> (this)]
    {
        if (self.getComponent())
            self.getComponent()->reloadFigures();
    });
}

bool BinaryTorquesView::isInterestedInFile (File file) const
{
    return file.hasFileExtension (".h5");
}

void BinaryTorquesView::reloadFigures()
{
    auto mainModel     = figures[0]->getModel();
    auto colorbarModel = figures[1]->getModel();

    mainModel.title = currentFile.getFileName();
    colorbarModel.ymin = scalarExtent[0];
    colorbarModel.ymax = scalarExtent[1];

    if (artist)
    {
        artist->setColorMap (cmaps.getCurrentStops());
        artist->setScalarDomain (scalarExtent[0], scalarExtent[1]);
        mainModel.content = { artist };
        colorbarModel.content = { std::make_shared<ColourGradientArtist> (mainModel.content[0]->getScalarMapping()) };
    }

    figures[0]->setModel (mainModel);
    figures[1]->setModel (colorbarModel);
}




//=============================================================================
void BinaryTorquesView::resized()
{
    layout.performLayout (getLocalBounds());
}




//=============================================================================
void BinaryTorquesView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.margin = value;
    });

    mutateFiguresInRow (figure, [value] (FigureModel& model)
    {
        model.margin.setTop (value.getTop());
        model.margin.setBottom (value.getBottom());
    });

    mutateFiguresInCol (figure, [value] (FigureModel& model)
    {
        model.margin.setLeft (value.getLeft());
        model.margin.setRight (value.getRight());
    });
}

void BinaryTorquesView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.xmin = value.getX();
        model.xmax = value.getRight();
        model.ymin = value.getY();
        model.ymax = value.getBottom();
    });

    if (artist && figure == figures[1])
    {
        scalarExtent[0] = figure->getModel().ymin;
        scalarExtent[1] = figure->getModel().ymax;
        reloadFigures();
    }
}

void BinaryTorquesView::figureViewSetXlabel (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.xlabel = value;
    });
}

void BinaryTorquesView::figureViewSetYlabel (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.ylabel = value;
    });
}

void BinaryTorquesView::figureViewSetTitle (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.title = value;
    });
}

void BinaryTorquesView::mutateFigure (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
{
    auto m = eventFigure->getModel();
    mutation (m);
    eventFigure->setModel (m);
}

void BinaryTorquesView::mutateFiguresInRow (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
{
    int sourceRow = figures.indexOf (eventFigure) / 2;
    int n = 0;

    for (const auto& f : figures)
    {
        if (n / 2 == sourceRow)
        {
            auto m = f->getModel();
            mutation (m);
            f->setModel (m);
        }
        ++n;
    }
}

void BinaryTorquesView::mutateFiguresInCol (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
{
    int sourceCol = figures.indexOf (eventFigure) % 2;
    int n = 0;

    for (const auto& f : figures)
    {
        if (n % 2 == sourceCol)
        {
            auto m = f->getModel();
            mutation (m);
            f->setModel (m);
        }
        ++n;
    }
}




//=============================================================================
void BinaryTorquesView::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = {
        Commands::makeSnapshotAndOpen,
        Commands::saveSnapshotAs,
        Commands::nextColourMap,
        Commands::prevColourMap,
        Commands::resetScalarRange,
    };
    commands.addArray (ids, numElementsInArray (ids));
}

void BinaryTorquesView::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case Commands::makeSnapshotAndOpen:
            result.setInfo ("Create Snapshot", "", "File", 0);
            result.defaultKeypresses.add ({'e', ModifierKeys::commandModifier, 0});
            break;
        case Commands::saveSnapshotAs:
            result.setInfo ("Save Snapshot As...", "", "File", 0);
            result.defaultKeypresses.add ({'e', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0});
            break;
        case Commands::nextColourMap:
            result.setInfo ("Next Color Map", "", "View", 0);
            result.defaultKeypresses.add (KeyPress (KeyPress::rightKey, 0, 0));
            break;
        case Commands::prevColourMap:
            result.setInfo ("Previous Color Map", "", "View", 0);
            result.defaultKeypresses.add (KeyPress (KeyPress::leftKey, 0, 0));
            break;
        case Commands::resetScalarRange:
            result.setInfo ("Reset Scalar Range", "", "View", 0);
            result.defaultKeypresses.add (KeyPress (KeyPress::upKey, 0, 0));
            break;
    }
}

bool BinaryTorquesView::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case Commands::makeSnapshotAndOpen: saveSnapshot (true); break;
        case Commands::saveSnapshotAs: saveSnapshot (false); break;
        case Commands::nextColourMap: cmaps.next(); reloadFigures(); break;
        case Commands::prevColourMap: cmaps.prev(); reloadFigures(); break;
        case Commands::resetScalarRange: scalarExtent = artist->getScalarExtent(); reloadFigures(); break;
    }
    return true;
}

ApplicationCommandTarget* BinaryTorquesView::getNextCommandTarget()
{
    return nullptr;
}




//=============================================================================
void BinaryTorquesView::saveSnapshot (bool toTempDirectory)
{
    auto target = File();

    if (toTempDirectory)
    {
        target = File::createTempFile (".png");
    }
    else
    {
        FileChooser chooser ("Open directory...", currentFile.getParentDirectory(), "", true, false, nullptr);

        if (chooser.browseForFileToSave (true))
            target = chooser.getResult();
        else
            return;
    }

    auto image = figures[0]->createSnapshot();
    target.deleteFile();

    if (auto stream = std::unique_ptr<FileOutputStream> (target.createOutputStream()))
    {
        auto fmt = PNGImageFormat();
        fmt.writeImageToStream (image, *stream);
    }
    if (toTempDirectory)
    {
        target.startAsProcess();
    }
}
