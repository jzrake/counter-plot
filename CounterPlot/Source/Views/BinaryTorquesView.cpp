#include "BinaryTorquesView.hpp"
#include "../MetalSurface.hpp"




//=============================================================================
class BinaryTorquesView::QuadmeshArtist : public PlotArtist
{
public:
    QuadmeshArtist (nd::array<double, 2> data, std::function<bool()> bailout)
    {
        auto scaledData  = MeshHelpers::scaleByLog10 (data, bailout);
        triangleVertices = MeshHelpers::triangulateUniformRectilinearMesh (data.shape(0), data.shape(1), {-8.f, 8.f, -8.f, 8.f}, bailout);
        triangleScalars  = MeshHelpers::makeRectilinearGridScalars (scaledData, bailout);
        scalarExtent     = MeshHelpers::findScalarExtent (scaledData, bailout);
        mapping.vmin     = scalarExtent[0];
        mapping.vmax     = scalarExtent[1];
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

    void resetScalarDomainToExtent()
    {
        mapping.vmin = scalarExtent[0];
        mapping.vmax = scalarExtent[1];
    }

    void render (RenderingSurface& surface) override
    {
        surface.renderTriangles (triangleVertices, triangleScalars, mapping);
    }

    bool isScalarMappable() const override
    {
        return true;
    }

    ScalarMapping getScalarMapping() const override
    {
        return mapping;
    }

    std::array<float, 2> getScalarExtent() const override
    {
        return scalarExtent;
    }

    std::array<float, 2> scalarExtent;
    ScalarMapping mapping;
    std::vector<simd::float2> triangleVertices;
    std::vector<simd::float1> triangleScalars;
};




//=============================================================================
BinaryTorquesView::BinaryTorquesView()
{
    layout.templateRows    = { Grid::TrackInfo (4_fr) };
    layout.templateColumns = { Grid::TrackInfo (1_fr), Grid::TrackInfo (100_px) };
    layout.items.add (mainFigure.getGridItem());
    layout.items.add (cmapFigure.getGridItem());

    mainModel.titleShowing = true;
    mainModel.xlabelShowing = false;
    mainModel.ylabelShowing = false;
    mainModel.canEditMargin = true;
    mainModel.canEditXlabel = false;
    mainModel.canEditYlabel = false;
    mainModel.canEditTitle = false;
    mainModel.margin.setRight (20);
    mainModel.margin.setLeft (40);
    mainModel.xtickCount = 10;
    mainModel.ytickCount = 5;
    mainModel.xmin = -0.1f;
    mainModel.xmax = +1.1f;
    mainModel.ymin = -0.1f;
    mainModel.ymax = +1.1f;

    cmapModel.titleShowing = false;
    cmapModel.xlabelShowing = false;
    cmapModel.ylabelShowing = false;
    cmapModel.canEditMargin = false;
    cmapModel.canEditXlabel = false;
    cmapModel.canEditYlabel = false;
    cmapModel.canEditTitle = false;
    cmapModel.margin.setLeft (40);
    cmapModel.margin.setRight (20);
    cmapModel.xtickCount = 0;
    cmapModel.ytickCount = 10;
    cmapModel.xmin = -0.1f;
    cmapModel.xmax = +1.1f;
    cmapModel.ymin = -0.1f;
    cmapModel.ymax = +1.1f;
    cmapModel.gridlinesColour = Colours::transparentBlack;

    mainFigure.setRenderingSurface (std::make_unique<MetalRenderingSurface>());
    mainFigure.addListener (this);
    cmapFigure.addListener (this);

    updateFigures();
    setWantsKeyboardFocus (true);
    addAndMakeVisible (mainFigure);
    addAndMakeVisible (cmapFigure);
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
    auto h5d = h5f.open_dataset ("primitive/sigma");
    auto res = h5d.read<nd::array<double, 2>>();
    quadmesh = std::make_shared<QuadmeshArtist> (res, bailout);

    if (! bailout())
    {
        currentFile = viewedDocument;

        MessageManager::callAsync ([viewedDocument, self = SafePointer<BinaryTorquesView> (this)]
        {
            if (self.getComponent())
                self.getComponent()->updateFigures();
        });
    }
}

bool BinaryTorquesView::isInterestedInFile (File file) const
{
    return file.hasFileExtension (".h5");
}

void BinaryTorquesView::updateFigures()
{
    if (quadmesh)
    {
        quadmesh->setColorMap (cmaps.getCurrentStops());
        gradient = std::make_shared<ColourGradientArtist> (quadmesh->mapping);

        mainModel.title = currentFile.getFileName();
        mainModel.content = { quadmesh };
        cmapModel.content = { gradient };
        cmapModel.ymin = quadmesh->getScalarMapping().vmin;
        cmapModel.ymax = quadmesh->getScalarMapping().vmax;

        mainFigure.setModel (mainModel);
        cmapFigure.setModel (cmapModel);
    }
}




//=============================================================================
void BinaryTorquesView::resized()
{
    layout.performLayout (getLocalBounds());
}




//=============================================================================
void BinaryTorquesView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& margin)
{
    if (figure == &mainFigure)
    {
        mainModel.margin = margin;
        cmapModel.margin.setTop (margin.getTop());
        cmapModel.margin.setBottom (margin.getBottom());
        updateFigures();
        return;
    }
}

void BinaryTorquesView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
{
    if (figure == &mainFigure)
    {
        mainModel.setDomain (domain);
    }
    if (figure == &cmapFigure)
    {
        cmapModel.setDomain (domain);

        if (quadmesh)
        {
            quadmesh->mapping.vmin = domain.getY();
            quadmesh->mapping.vmax = domain.getBottom();
        }
    }
    updateFigures();
}

void BinaryTorquesView::figureViewSetXlabel (FigureView* figure, const String& value)
{
}

void BinaryTorquesView::figureViewSetYlabel (FigureView* figure, const String& value)
{
}

void BinaryTorquesView::figureViewSetTitle (FigureView* figure, const String& value)
{
}




//=============================================================================
void BinaryTorquesView::getAllCommands (Array<CommandID>& commands)
{
    FileBasedView::getAllCommands (commands);
}

void BinaryTorquesView::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    FileBasedView::getCommandInfo (commandID, result);
}

bool BinaryTorquesView::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case Commands::makeSnapshotAndOpen: saveSnapshot (true); break;
        case Commands::saveSnapshotAs: saveSnapshot (false); break;
        case Commands::nextColourMap: cmaps.next(); updateFigures(); break;
        case Commands::prevColourMap: cmaps.prev(); updateFigures(); break;
        case Commands::resetScalarRange:
            if (quadmesh) quadmesh->resetScalarDomainToExtent();
            updateFigures();
            break;
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

    auto image = mainFigure.createSnapshot();
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
