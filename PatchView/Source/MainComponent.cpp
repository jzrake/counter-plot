#include "MainComponent.hpp"
#include "MetalSurface.hpp"




//=============================================================================
PageView::PageView()
{
    layout.templateRows    = { Grid::TrackInfo (1_fr) };
    layout.templateColumns = { Grid::TrackInfo (1_fr), Grid::TrackInfo (80_px) };

    figures.add (new FigureView);
    figures.add (new FigureView);

    FigureModel mainModel;
    mainModel.margin.setRight (20);
    figures[0]->setModel (mainModel);
    // figures[0]->setRenderingSurface (std::make_unique<MetalRenderingSurface>());

    FigureModel colorbarModel;
    colorbarModel.titleShowing = false;
    colorbarModel.xlabelShowing = false;
    colorbarModel.ylabelShowing = false;
    colorbarModel.margin.setLeft (40);
    colorbarModel.margin.setRight (20);
    colorbarModel.gridlinesColour = Colours::transparentBlack;
    colorbarModel.xtickCount = 0;
    colorbarModel.allowUserResize = false;
    colorbarModel.content.push_back (std::make_shared<ColourGradientArtist> (ScalarMapping { ColourmapHelpers::coloursFromRGBTable (BinaryData::cividis_cmap) }));
    figures[1]->setModel (colorbarModel);

    for (const auto& figure : figures)
    {
        figure->addListener (this);
        addAndMakeVisible (figure);
        layout.items.add (figure->getGridItem());
    }
}

void PageView::resized()
{
    layout.performLayout (getLocalBounds());
}




//=============================================================================
void PageView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& value)
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

void PageView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.xmin = value.getX();
        model.xmax = value.getRight();
        model.ymin = value.getY();
        model.ymax = value.getBottom();
    });
}

void PageView::figureViewSetXlabel (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.xlabel = value;
    });
}

void PageView::figureViewSetYlabel (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.ylabel = value;
    });
}

void PageView::figureViewSetTitle (FigureView* figure, const String& value)
{
    mutateFigure (figure, [value] (FigureModel& model)
    {
        model.title = value;
    });
}

void PageView::mutateFigure (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
{
    auto m = eventFigure->getModel();
    mutation (m);
    eventFigure->setModel (m);
}

void PageView::mutateFiguresInRow (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
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

void PageView::mutateFiguresInCol (FigureView* eventFigure, std::function<void(FigureModel&)> mutation)
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

        vmin = *std::min_element (triangleScalars.begin(), triangleScalars.end());
        vmax = *std::max_element (triangleScalars.begin(), triangleScalars.end());
    }

    void render (RenderingSurface& surface) override
    {
        surface.renderTriangles (triangleVertices, triangleScalars, vmin, vmax);
    }

    bool isScalarMappable() const override { return true; }

    ScalarMapping getScalarMapping() const override
    {
        return { ColourmapHelpers::coloursFromRGBTable (BinaryData::cividis_cmap) };
    }

private:
    float vmin = 0.f;
    float vmax = 1.f;
    std::vector<simd::float2> triangleVertices;
    std::vector<simd::float4> triangleColors;
    std::vector<simd::float1> triangleScalars;
};




//=============================================================================
MainComponent::MainComponent()
{
    directoryTree.setDirectoryToShow (File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory));
    figure.addListener (this);
    figure.setModel (model);

    // NOTE: disabling hardware rendering to investigate shutdown bug
    // figure.setRenderingSurface (std::make_unique<MetalRenderingSurface>());

    directoryTree.addListener (this);

    addAndMakeVisible (directoryTree);
    addChildComponent (figure);
    addChildComponent (imageView);
    addChildComponent (variantView);
    addAndMakeVisible (page);
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

    figure.setBounds (area);
    imageView.setBounds (area);
    variantView.setBounds (area);
    page.setBounds (area);
}

bool MainComponent::keyPressed (const juce::KeyPress &key)
{
    if (key == KeyPress::leftKey)
    {
        dynamic_cast<MetalRenderingSurface&>(*figure.getRenderingSurface()).prevColorMap();
        figure.setModel (model);
        return true;
    }
    if (key == KeyPress::rightKey)
    {
        dynamic_cast<MetalRenderingSurface&>(*figure.getRenderingSurface()).nextColorMap();
        figure.setModel (model);
        return true;
    }
    return false;
}




//=============================================================================
void MainComponent::figureViewSetMargin (FigureView*, const BorderSize<int>& value)
{
    model.margin = value;
    figure.setModel (model);
}

void MainComponent::figureViewSetDomain (FigureView*, const Rectangle<double>& value)
{
    model.xmin = value.getX();
    model.xmax = value.getRight();
    model.ymin = value.getY();
    model.ymax = value.getBottom();
    figure.setModel (model);
}

void MainComponent::figureViewSetXlabel (FigureView*, const String& value)
{
    model.xlabel = value;
    figure.setModel (model);
}

void MainComponent::figureViewSetYlabel (FigureView*, const String& value)
{
    model.ylabel = value;
    figure.setModel (model);
}

void MainComponent::figureViewSetTitle (FigureView*, const String& value)
{
    model.title = value;
    figure.setModel (model);
}




//=============================================================================
void MainComponent::selectedFileChanged (DirectoryTree*, File file)
{
    if (FileSystemSerializer::looksLikeDatabase (file))
    {
        FileSystemSerializer ser (file);
        auto db = patches2d::Database::load (ser);
        model.content.clear();
        model.content.push_back (std::make_shared<PatchesQuadMeshArtist> (db));
        figure.setModel (model);

        figure.setVisible (true);
        imageView.setVisible (false);
        variantView.setVisible (false);
        page.setVisible (false);
    }
    else if (ColourmapHelpers::looksLikeRGBTable (file))
    {
        auto cb = ScalarMapping();
        cb.stops = ColourmapHelpers::coloursFromRGBTable (file.loadFileAsString());
        model.content.clear();
        model.content.push_back (std::make_shared<ColourGradientArtist> (cb));
        figure.setModel (model);

        figure.setVisible (true);
        imageView.setVisible (false);
        variantView.setVisible (false);
        page.setVisible (false);
    }
    else if (auto format = ImageFileFormat::findImageFormatForFileExtension (file))
    {
        imageView.setImage (format->loadFrom (file));

        figure.setVisible (false);
        imageView.setVisible (true);
        variantView.setVisible (false);
        page.setVisible (false);
    }
    else if (file.hasFileExtension (".json"))
    {
        variantView.setData (JSON::parse (file));

        figure.setVisible (false);
        imageView.setVisible (false);
        variantView.setVisible (true);
        page.setVisible (false);
    }
}
