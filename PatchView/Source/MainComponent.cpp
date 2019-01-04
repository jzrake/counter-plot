#include "MainComponent.hpp"




//==============================================================================
enum class LineStyle { none, solid, dash, dashdot };
enum class MarkerStyle { none, circle, square, diamond, plus, cross };




//==============================================================================
struct LinePlotModel
{
    nd::ndarray<double, 1> x;
    nd::ndarray<double, 1> y;
    float         lineWidth    = 1.f;
    float         markerSize   = 1.f;
    Colour        lineColour   = Colours::black;
    Colour        markerColour = Colours::black;
    LineStyle     lineStyle    = LineStyle::solid;
    MarkerStyle   markerStyle  = MarkerStyle::none;
};




//==============================================================================
class LinePlotArtist : public PlotArtist
{
public:
    LinePlotArtist (LinePlotModel model) : model (model)
    {
    }

    void paint (Graphics& g, const PlotTransformer& trans) override
    {
        jassert (model.x.size() == model.y.size());

        if (model.x.empty())
        {
            return;
        }

        Path p;
        p.startNewSubPath (trans.fromDomainX (model.x(0)),
                           trans.fromDomainY (model.y(0)));

        for (int n = 1; n < model.x.size(); ++n)
        {
            p.lineTo (trans.fromDomainX (model.x(n)),
                      trans.fromDomainY (model.y(n)));
        }
        g.setColour (model.lineColour);
        g.strokePath (p, PathStrokeType (model.lineWidth));
    }

private:
    LinePlotModel model;
};




//==============================================================================
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

    void render (RenderingSurface& surface)
    {
        surface.renderTriangles (triangleVertices, triangleScalars, vmin, vmax);
    }

private:
    float vmin = 0.f;
    float vmax = 1.f;
    std::vector<simd::float2> triangleVertices;
    std::vector<simd::float4> triangleColors;
    std::vector<simd::float1> triangleScalars;
};




//==============================================================================
class MetalRenderingSurface : public RenderingSurface
{
public:

    //==========================================================================
    MetalRenderingSurface()
    {
        setColorMap (0);
        setInterceptsMouseClicks (false, false);
        addAndMakeVisible (metal);
    }

    void setContent (std::vector<std::shared_ptr<PlotArtist>> artists, const PlotTransformer& trans) override
    {
        scene.clear();
        scene.setDomain (trans.getDomain());

        for (auto artist : artists)
        {
            artist->render (*this);
        }
        metal.setScene (scene);
    }

    void renderTriangles (const std::vector<simd::float2>& vertices,
                          const std::vector<simd::float4>& colors) override
    {
        assert(vertices.size() == colors.size());
        auto node = metal::Node();
        node.setVertexPositions (getOrCreateBuffer (vertices));
        node.setVertexColors (getOrCreateBuffer (colors));
        node.setVertexCount (vertices.size());
        scene.addNode (node);
    }

    void renderTriangles (const std::vector<simd::float2>& vertices,
                          const std::vector<simd::float1>& scalars,
                          float vmin, float vmax) override
    {
        assert(vertices.size() == scalars.size());
        auto node = metal::Node();
        node.setVertexPositions (getOrCreateBuffer (vertices));
        node.setVertexScalars (getOrCreateBuffer (scalars));
        node.setScalarMapping (colormap);
        node.setScalarDomain (vmin, vmax);
        node.setVertexCount (vertices.size());
        scene.addNode (node);
    }

    void resized() override
    {
        metal.setBounds (getLocalBounds());
    }

    void nextColorMap()
    {
        setColorMap ((colorMapIndex + 1) % 8);
    }

    void prevColorMap()
    {
        setColorMap ((colorMapIndex - 1 + 8) % 8);
    }

    void setColorMap (int index)
    {
        std::vector<uint32> data;

        switch (colorMapIndex = index)
        {
            case 0: data = ColormapHelpers::fromRGBTable (BinaryData::cividis_cmap); break;
            case 1: data = ColormapHelpers::fromRGBTable (BinaryData::dawn_cmap); break;
            case 2: data = ColormapHelpers::fromRGBTable (BinaryData::fire_cmap); break;
            case 3: data = ColormapHelpers::fromRGBTable (BinaryData::inferno_cmap); break;
            case 4: data = ColormapHelpers::fromRGBTable (BinaryData::magma_cmap); break;
            case 5: data = ColormapHelpers::fromRGBTable (BinaryData::plasma_cmap); break;
            case 6: data = ColormapHelpers::fromRGBTable (BinaryData::seashore_cmap); break;
            case 7: data = ColormapHelpers::fromRGBTable (BinaryData::viridis_cmap); break;
        }
        colormap = metal::Device::makeTexture1d (data.data(), data.size());
    }

private:
    //==========================================================================
    metal::Buffer getOrCreateBuffer (const std::vector<simd::float1>& data)
    {
        if (cachedBuffers1.count (&data))
        {
            return cachedBuffers1.at (&data);
        }
        auto newBuffer = metal::Device::makeBuffer (data.data(), data.size() * sizeof (simd::float1));
        cachedBuffers1[&data] = newBuffer;
        return newBuffer;
    }

    metal::Buffer getOrCreateBuffer (const std::vector<simd::float2>& data)
    {
        if (cachedBuffers2.count (&data))
        {
            return cachedBuffers2.at (&data);
        }
        auto newBuffer = metal::Device::makeBuffer (data.data(), data.size() * sizeof (simd::float2));
        cachedBuffers2[&data] = newBuffer;
        return newBuffer;
    }

    metal::Buffer getOrCreateBuffer (const std::vector<simd::float4>& data)
    {
        if (cachedBuffers4.count (&data))
        {
            return cachedBuffers4.at (&data);
        }
        auto newBuffer = metal::Device::makeBuffer (data.data(), data.size() * sizeof (simd::float4));
        cachedBuffers4[&data] = newBuffer;
        return newBuffer;
    }

    std::map<const std::vector<simd::float1>*, metal::Buffer> cachedBuffers1;
    std::map<const std::vector<simd::float2>*, metal::Buffer> cachedBuffers2;
    std::map<const std::vector<simd::float4>*, metal::Buffer> cachedBuffers4;

    metal::Texture colormap;
    metal::Scene scene;
    metal::MetalComponent metal;
    
    int colorMapIndex = 0;
};




//==============================================================================
MainComponent::MainComponent()
{
    model.backgroundColour = Colours::darkkhaki;
    model.marginColour = Colours::darkgrey;

    directoryTree.setDirectoryToShow (File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory));
    figure.addListener (this);
    figure.setModel (model);
    figure.setRenderingSurface (std::make_unique<MetalRenderingSurface>());
    
    directoryTree.addListener (this);

    addAndMakeVisible (directoryTree);
    addAndMakeVisible (figure);
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




//==============================================================================
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




//==============================================================================
void MainComponent::selectedFileChanged (DirectoryTree*, File file)
{
    if (FileSystemSerializer::looksLikeDatabase (file))
    {
        FileSystemSerializer ser (file);
        auto db = patches2d::Database::load (ser);
        model.content.clear();
        model.content.push_back (std::make_shared<PatchesQuadMeshArtist> (db));
        figure.setModel (model);
    }
}
