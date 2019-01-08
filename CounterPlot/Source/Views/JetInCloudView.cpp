#include "JetInCloudView.hpp"
#include "../MetalSurface.hpp"




//=============================================================================
class JetInCloudView::QuadmeshArtist : public PlotArtist
{
public:
    QuadmeshArtist (const patches2d::Database& model)
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

        mapping.stops = ColourMapHelpers::coloursFromRGBTable (BinaryData::magma_cmap);
        mapping.vmin = *std::min_element (triangleScalars.begin(), triangleScalars.end());
        mapping.vmax = *std::max_element (triangleScalars.begin(), triangleScalars.end());
        scalarExtent = { mapping.vmin, mapping.vmax };
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
    std::vector<simd::float4> triangleColors;
    std::vector<simd::float1> triangleScalars;
};




//=============================================================================
JetInCloudView::JetInCloudView()
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

JetInCloudView::~JetInCloudView()
{
}

bool JetInCloudView::isInterestedInFile(File file) const
{
    return FileSystemSerializer::looksLikeDatabase (file);
}

bool JetInCloudView::loadFile (File viewedDocument)
{
    auto ser = FileSystemSerializer (viewedDocument);
    auto db = patches2d::Database::load (ser);
    artist = std::make_shared<QuadmeshArtist> (db);
    scalarExtent = artist->getScalarExtent();
    currentFile = viewedDocument;
    reloadFigures();
    return true;
}

void JetInCloudView::loadFileAsync (File fileToDisplay, std::function<bool()> bailout)
{
}

void JetInCloudView::reloadFigures()
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
void JetInCloudView::resized()
{
    layout.performLayout (getLocalBounds());
}

bool JetInCloudView::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::leftKey)
    {
        cmaps.prev();
        reloadFigures();
        return true;
    }
    if (key == KeyPress::rightKey)
    {
        cmaps.next();
        reloadFigures();
        return true;
    }
    if (key == KeyPress::spaceKey)
    {
        scalarExtent = artist->getScalarExtent();
        reloadFigures();
        return true;
    }
    return false;
}




//=============================================================================
void JetInCloudView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& value)
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

void JetInCloudView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& value)
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

void JetInCloudView::figureViewSetXlabel (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.xlabel = value;
    });
}

void JetInCloudView::figureViewSetYlabel (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.ylabel = value;
    });
}

void JetInCloudView::figureViewSetTitle (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.title = value;
    });
}

void JetInCloudView::mutateFigure (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
{
    auto m = eventFigure->getModel();
    mutation (m);
    eventFigure->setModel (m);
}

void JetInCloudView::mutateFiguresInRow (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
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

void JetInCloudView::mutateFiguresInCol (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
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
