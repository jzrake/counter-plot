#include "MainComponent.hpp"
#include "MetalSurface.hpp"




//=============================================================================
class PatchesQuadMeshArtist : public PlotArtist
{
public:
    PatchesQuadMeshArtist (const patches2d::Database& model)
    {
        for (auto patch : model.all (patches2d::Field::vert_coords))
        {
            auto verts = patch.second;
            auto cells = model.at (patch.first, patches2d::Field::conserved);

            for (int i = 0; i < verts.shape(0) - 1; ++i)
            {
                for (int j = 0; j < verts.shape(1) - 1; ++j)
                {
                    const float r00 = verts (i + 0, j + 0, 0);
                    const float r01 = verts (i + 0, j + 1, 0);
                    const float r10 = verts (i + 1, j + 0, 0);
                    const float r11 = verts (i + 1, j + 1, 0);
                    const float q00 = verts (i + 0, j + 0, 1);
                    const float q01 = verts (i + 0, j + 1, 1);
                    const float q10 = verts (i + 1, j + 0, 1);
                    const float q11 = verts (i + 1, j + 1, 1);
                    const float x00 = r00 * std::sinf (q00);
                    const float x01 = r01 * std::sinf (q01);
                    const float x10 = r10 * std::sinf (q10);
                    const float x11 = r11 * std::sinf (q11);
                    const float y00 = r00 * std::cosf (q00);
                    const float y01 = r01 * std::cosf (q01);
                    const float y10 = r10 * std::cosf (q10);
                    const float y11 = r11 * std::cosf (q11);
                    const float c = std::log10f (cells (i, j, 0));

                    triangleVertices.push_back (simd::float2 {x00, y00});
                    triangleVertices.push_back (simd::float2 {x01, y01});
                    triangleVertices.push_back (simd::float2 {x10, y10});
                    triangleVertices.push_back (simd::float2 {x01, y01});
                    triangleVertices.push_back (simd::float2 {x10, y10});
                    triangleVertices.push_back (simd::float2 {x11, y11});

                    triangleColors.push_back (simd::float4 {c, c, c, 1.f});
                    triangleColors.push_back (simd::float4 {c, c, c, 1.f});
                    triangleColors.push_back (simd::float4 {c, c, c, 1.f});
                    triangleColors.push_back (simd::float4 {c, c, c, 1.f});
                    triangleColors.push_back (simd::float4 {c, c, c, 1.f});
                    triangleColors.push_back (simd::float4 {c, c, c, 1.f});

                    triangleScalars.push_back (simd::float1 (c));
                    triangleScalars.push_back (simd::float1 (c));
                    triangleScalars.push_back (simd::float1 (c));
                    triangleScalars.push_back (simd::float1 (c));
                    triangleScalars.push_back (simd::float1 (c));
                    triangleScalars.push_back (simd::float1 (c));
                }
            }
        }

        mapping.stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::magma_cmap);
        mapping.vmin = *std::min_element (triangleScalars.begin(), triangleScalars.end());
        mapping.vmax = *std::max_element (triangleScalars.begin(), triangleScalars.end());
    }

    void setColorMap (const Array<Colour>& stops)
    {
        mapping.stops = stops;
    }

    void render (RenderingSurface& surface) override { surface.renderTriangles (triangleVertices, triangleScalars, mapping); }
    bool isScalarMappable() const override { return true; }
    ScalarMapping getScalarMapping() const override { return mapping; }

private:
    ScalarMapping mapping;
    std::vector<simd::float2> triangleVertices;
    std::vector<simd::float4> triangleColors;
    std::vector<simd::float1> triangleScalars;
};




//=============================================================================
PatchesView::PatchesView()
{
    layout.templateRows    = { Grid::TrackInfo (1_fr) };
    layout.templateColumns = { Grid::TrackInfo (1_fr), Grid::TrackInfo (80_px) };

    figures.add (new FigureView);
    figures.add (new FigureView);

    FigureModel mainModel;
    mainModel.margin.setRight (20);
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
    colorbarModel.allowUserResize = false;
    figures[1]->setModel (colorbarModel);

    for (const auto& figure : figures)
    {
        figure->addListener (this);
        addAndMakeVisible (figure);
        layout.items.add (figure->getGridItem());
    }
    setWantsKeyboardFocus (true);
}

void PatchesView::setDocumentFile (File viewedDocument)
{
    auto ser = FileSystemSerializer (viewedDocument);
    auto db = patches2d::Database::load (ser);
    artist = std::make_shared<PatchesQuadMeshArtist> (db);
    reloadFigures();
}

void PatchesView::nextColorMap()
{
    setColorMap ((colorMapIndex + 1) % 8);
}

void PatchesView::prevColorMap()
{
    setColorMap ((colorMapIndex - 1 + 8) % 8);
}

void PatchesView::setColorMap (int index)
{
    colorMapIndex = index;
    reloadFigures();
}

Array<Colour> PatchesView::getColorMap() const
{
    Array<Colour> stops;

    switch (colorMapIndex)
    {
        case 0: stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::cividis_cmap); break;
        case 1: stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::dawn_cmap); break;
        case 2: stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::fire_cmap); break;
        case 3: stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::inferno_cmap); break;
        case 4: stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::magma_cmap); break;
        case 5: stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::plasma_cmap); break;
        case 6: stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::seashore_cmap); break;
        case 7: stops = ColourmapHelpers::coloursFromRGBTable (BinaryData::viridis_cmap); break;
    }
    return stops;
}

void PatchesView::reloadFigures()
{
    auto mainModel     = figures[0]->getModel();
    auto colorbarModel = figures[1]->getModel();

    if (artist)
    {
        artist->setColorMap (getColorMap());
        mainModel.content = { artist };
        colorbarModel.content = { std::make_shared<ColourGradientArtist> (mainModel.content[0]->getScalarMapping()) };
    }

    figures[0]->setModel (mainModel);
    figures[1]->setModel (colorbarModel);
}




//=============================================================================
void PatchesView::resized()
{
    layout.performLayout (getLocalBounds());
}

bool PatchesView::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::leftKey)
    {
        prevColorMap();
        return true;
    }
    if (key == KeyPress::rightKey)
    {
        nextColorMap();
        return true;
    }
    return false;
}




//=============================================================================
void PatchesView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& value)
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

void PatchesView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.xmin = value.getX();
        model.xmax = value.getRight();
        model.ymin = value.getY();
        model.ymax = value.getBottom();
    });
}

void PatchesView::figureViewSetXlabel (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.xlabel = value;
    });
}

void PatchesView::figureViewSetYlabel (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.ylabel = value;
    });
}

void PatchesView::figureViewSetTitle (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.title = value;
    });
}

void PatchesView::mutateFigure (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
{
    auto m = eventFigure->getModel();
    mutation (m);
    eventFigure->setModel (m);
}

void PatchesView::mutateFiguresInRow (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
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

void PatchesView::mutateFiguresInCol (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
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
MainComponent::MainComponent()
{
    directoryTree.setDirectoryToShow (File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory));
    directoryTree.addListener (this);

    addAndMakeVisible (directoryTree);
    addChildComponent (imageView);
    addChildComponent (variantView);
    addAndMakeVisible (patchesView);
    setSize (1024, 768 - 64);
}

MainComponent::~MainComponent()
{
}

void MainComponent::setCurrentDirectory (File newCurrentDirectory)
{
    directoryTree.setDirectoryToShow (newCurrentDirectory);
}

void MainComponent::paint (Graphics& g)
{
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    directoryTree.setBounds (area.removeFromLeft(300));

    imageView.setBounds (area);
    variantView.setBounds (area);
    patchesView.setBounds (area);
}

bool MainComponent::keyPressed (const juce::KeyPress &key)
{
    return false;
}




//=============================================================================
void MainComponent::selectedFileChanged (DirectoryTree*, File file)
{
    if (FileSystemSerializer::looksLikeDatabase (file))
    {
        patchesView.setDocumentFile (file);

        imageView.setVisible (false);
        variantView.setVisible (false);
        patchesView.setVisible (true);
    }
    else if (ColourmapHelpers::looksLikeRGBTable (file))
    {
//        auto cb = ScalarMapping();
//        cb.stops = ColourmapHelpers::coloursFromRGBTable (file.loadFileAsString());
//        model.content.clear();
//        model.content.push_back (std::make_shared<ColourGradientArtist> (cb));
//        figure.setModel (model);
//
//        figure.setVisible (true);
//        imageView.setVisible (false);
//        variantView.setVisible (false);
//        patchesView.setVisible (false);
    }
    else if (auto format = ImageFileFormat::findImageFormatForFileExtension (file))
    {
        imageView.setImage (format->loadFrom (file));

        imageView.setVisible (true);
        variantView.setVisible (false);
        patchesView.setVisible (false);
    }
    else if (file.hasFileExtension (".json"))
    {
        variantView.setData (JSON::parse (file));

        imageView.setVisible (false);
        variantView.setVisible (true);
        patchesView.setVisible (false);
    }
}
