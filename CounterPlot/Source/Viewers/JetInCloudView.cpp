#include "JetInCloudView.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Plotting/MetalSurface.hpp"




//=============================================================================
struct JetInCloud::TriangleVertexData
{
    std::array<float, 2> scalarExtent = {0.f, 1.f};
    std::shared_ptr<std::vector<simd::float2>> vertices;
    std::shared_ptr<std::vector<simd::float1>> scalars;
};

JetInCloud::TriangleVertexData JetInCloud::loadTriangleDataFromFile (File file, std::function<bool()> bailout)
{
    try {
        auto startTime = Time::getMillisecondCounterHiRes();

        auto ser = FileSystemSerializer (file);
        auto db  = patches2d::Database::load (ser, {patches2d::Field::conserved, patches2d::Field::vert_coords});
        auto scalars  = std::vector<simd::float1>();
        auto vertices = std::vector<simd::float2>();
        auto iter = 0;

        DBG("jet-in-cloud: serializer ran in " << (Time::getMillisecondCounterHiRes() - startTime) / 1e3 << "s");

        for (auto patch : db.all (patches2d::Field::vert_coords))
        {
            auto verts = patch.second;
            auto cells = db.at (patch.first, patches2d::Field::conserved);

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

                    vertices.push_back (simd::float2 {x00, y00});
                    vertices.push_back (simd::float2 {x01, y01});
                    vertices.push_back (simd::float2 {x10, y10});
                    vertices.push_back (simd::float2 {x01, y01});
                    vertices.push_back (simd::float2 {x10, y10});
                    vertices.push_back (simd::float2 {x11, y11});

                    scalars.push_back (simd::float1 (c));
                    scalars.push_back (simd::float1 (c));
                    scalars.push_back (simd::float1 (c));
                    scalars.push_back (simd::float1 (c));
                    scalars.push_back (simd::float1 (c));
                    scalars.push_back (simd::float1 (c));

                    if (++iter % 1000 == 0 && bailout && bailout())
                    {
                        return {};
                    }
                }
            }
        }

        auto lower = *std::min_element (scalars.begin(), scalars.end());
        auto upper = *std::max_element (scalars.begin(), scalars.end());

        DBG("jet-in-cloud: data loaded in " << (Time::getMillisecondCounterHiRes() - startTime) / 1e3 << "s");
        return {
            {lower, upper},
            std::make_shared<std::vector<simd::float2>> (vertices),
            std::make_shared<std::vector<simd::float1>> (scalars)
        };
    }
    catch (std::exception& e)
    {
        DBG("jet-in-cloud: failed to load: " << e.what());
        return {};
    }
}




//=============================================================================
class JetInCloud::QuadmeshArtist : public PlotArtist
{
public:
    void setVertices (TriangleVertexData dataToUse)
    {
        data = dataToUse;
    }

    void setMapping (ScalarMapping mappingToUse)
    {
        mapping = mappingToUse;
    }

    void render (RenderingSurface& surface) override
    {
        if (data.vertices && data.scalars)
        {
            surface.renderTriangles (*data.vertices, *data.scalars, mapping);
        }
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
        return data.scalarExtent;
    }

    ScalarMapping mapping;
    TriangleVertexData data;
};


namespace jic {

//=============================================================================
using GradientArtist = ColourGradientArtist;
using QuadmeshArtist = JetInCloud::QuadmeshArtist;




//=============================================================================
struct State
{
    std::shared_ptr<QuadmeshArtist>       quadmesh;
    std::shared_ptr<GradientArtist>       gradient;
    JetInCloud::TriangleVertexData triangles;
    ScalarMapping                         mapping;
    FigureModel                           cmapModel;
    FigureModel                           mainModel;
    File                                  file;

    State()
    {
        mapping.stops = ColourMapCollection().getCurrentStops();
        mapping.vmin = 0.f;
        mapping.vmax = 1.f;

        gradient = std::make_shared<GradientArtist>();
        quadmesh = std::make_shared<QuadmeshArtist>();
        gradient->setStops (mapping.stops);
        quadmesh->setMapping (mapping);

        mainModel.titleShowing = true;
        mainModel.xlabelShowing = false;
        mainModel.ylabelShowing = false;
        mainModel.canEditMargin = true;
        mainModel.canEditXlabel = false;
        mainModel.canEditYlabel = false;
        mainModel.canEditTitle = false;
        mainModel.canDeformDomain = false;
        mainModel.margin.setRight (20);
        mainModel.margin.setLeft (40);
        mainModel.xtickCount = 5;
        mainModel.ytickCount = 5;
        mainModel.xmin = -5;
        mainModel.xmax = +5;
        mainModel.ymin = -5;
        mainModel.ymax = +5;
        mainModel.content = {quadmesh};

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
        cmapModel.xmin = -0.f;
        cmapModel.xmax = +1.f;
        cmapModel.ymin = -0.f;
        cmapModel.ymax = +1.f;
        cmapModel.gridlinesColour = Colours::transparentBlack;
        cmapModel.content = {gradient};
    }
};




//=============================================================================
struct Action
{
    struct SetFile           { File file; };
    struct SetTriangleData   { JetInCloud::TriangleVertexData triangles; };
    struct SetColourMap      { Array<Colour> stops; };
    struct SetFigureMargin   { BorderSize<int> margin; };
    struct SetFigureDomain   { float xmin=0, xmax=1, ymin=0, ymax=1; };
    struct SetScalarDomain   { float vmin=0, vmax=1; };
    struct SetScalarDomainToExtent {};
};




//=============================================================================
class Store
{
public:

    //=========================================================================
    /**
     * The subscriber will likely be a component. It must implement the update
     * method, which will be invoke by the store when the state has changed.
     */
    class Subscriber
    {
    public:
        virtual ~Subscriber() {}
        virtual void update (const State&) = 0;
        virtual void asynchronousTaskStarted() {}
        virtual void asynchronousTaskFinished() {}
    };

    //=========================================================================
    template<typename JobReturnType>
    class Worker : public ThreadPoolJob
    {
    public:
        using Bailout = std::function<bool()>;
        using Job = std::function<JobReturnType(Bailout)>;

        Worker (Store* recipient, Subscriber* subscriber, Job job)
        : ThreadPoolJob ("worker")
        , recipient (recipient)
        , subscriber (subscriber)
        , job (job)
        {
            subscriber->asynchronousTaskStarted();
        }

    private:
        JobStatus runJob() override
        {
            auto result = job ([this] { return shouldExit(); });

            if (! shouldExit())
            {
                MessageManager::callAsync ([recipient=recipient, result] { recipient->dispatch (result); });
            }
            MessageManager::callAsync ([subscriber=subscriber] { subscriber->asynchronousTaskFinished(); });
            return jobHasFinished;
        }

        Store* recipient;
        Subscriber* subscriber;
        Job job;
    };


    //=========================================================================
    Store (Subscriber& subscriber) : subscriber (subscriber), pool (2)
    {
        subscriber.update (state);
    }

    template <typename Callable>
    void dispatch (Callable job)
    {
        pool.removeAllJobs (true, 0);
        pool.addJob (new Worker<decltype(job(nullptr))> (this, &subscriber, job), true);
    }

    void dispatch (Action::SetFile action)
    {
        if (state.file != action.file)
        {
            state.file = action.file;
            state.mainModel.title = action.file.getFileName();
            dispatch ([action] (auto bailout) { return Action::SetTriangleData { JetInCloud::loadTriangleDataFromFile (action.file, bailout)}; });
            subscriber.update (state);
        }
    }

    void dispatch (Action::SetTriangleData action)
    {
        bool wasEmpty = state.triangles.vertices == nullptr;

        state.triangles = action.triangles;
        state.quadmesh->setVertices (action.triangles);

        if (wasEmpty)
            dispatch (Action::SetScalarDomainToExtent());
        else
            subscriber.update (state);
    }

    void dispatch (Action::SetColourMap action)
    {
        state.mapping.stops = action.stops;
        state.gradient->setStops (state.mapping.stops);
        state.quadmesh->setMapping (state.mapping);
        subscriber.update (state);
    }

    void dispatch (Action::SetScalarDomain action)
    {
        state.mapping.vmin = action.vmin;
        state.mapping.vmax = action.vmax;
        state.cmapModel.ymin = action.vmin;
        state.cmapModel.ymax = action.vmax;
        state.gradient->setStops (state.mapping.stops);
        state.quadmesh->setMapping (state.mapping);
        subscriber.update (state);
    }

    void dispatch (Action::SetScalarDomainToExtent)
    {
        dispatch (Action::SetScalarDomain {state.triangles.scalarExtent[0], state.triangles.scalarExtent[1]});
    }

    void dispatch (Action::SetFigureDomain action)
    {
        state.mainModel.xmin = action.xmin;
        state.mainModel.xmax = action.xmax;
        state.mainModel.ymin = action.ymin;
        state.mainModel.ymax = action.ymax;
        subscriber.update (state);
    }

    void dispatch (Action::SetFigureMargin action)
    {
        state.mainModel.margin = action.margin;
        state.cmapModel.margin.setTop (action.margin.getTop());
        state.cmapModel.margin.setBottom (action.margin.getBottom());
        subscriber.update (state);
    }

    void dispatch (Action::SetFigureDomain action1, Action::SetFigureMargin action2)
    {
        state.mainModel.xmin = action1.xmin;
        state.mainModel.xmax = action1.xmax;
        state.mainModel.ymin = action1.ymin;
        state.mainModel.ymax = action1.ymax;
        state.mainModel.margin = action2.margin;
        state.cmapModel.margin.setTop (action2.margin.getTop());
        state.cmapModel.margin.setBottom (action2.margin.getBottom());
        subscriber.update (state);
    }

private:
    State state;
    Subscriber& subscriber;
    ThreadPool pool;
};

}

using namespace jic;




//=============================================================================
class JetInCloudView
: public Viewer
, public Store::Subscriber
, public FigureView::Listener
, public ApplicationCommandTarget
{
public:
    class QuadmeshArtist;

    //=========================================================================
    JetInCloudView();
    ~JetInCloudView();

    //=========================================================================
    void update (const State&) override;
    void asynchronousTaskStarted() override;
    void asynchronousTaskFinished() override;

    //=========================================================================
    bool isInterestedInFile (File file) const override;
    void loadFile (File fileToDisplay) override;
    String getViewerName() const override { return "Jet In Cloud Problem"; }

    //=========================================================================
    void resized() override;

    //=========================================================================
    void figureViewSetDomainAndMargin (FigureView*, const Rectangle<double>&, const BorderSize<int>&) override;
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
    void saveSnapshot (bool toTempDirectory);

    //=========================================================================
    FigureView mainFigure;
    FigureView cmapFigure;
    Grid layout;
    ColourMapCollection cmaps;
    State state;
    Store store;
};




//=============================================================================
JetInCloudView::JetInCloudView() : store (*this)
{
    layout.templateRows    = { Grid::TrackInfo (1_fr) };
    layout.templateColumns = { Grid::TrackInfo (1_fr), Grid::TrackInfo (100_px) };
    layout.items.add (&mainFigure);
    layout.items.add (&cmapFigure);

    // mainFigure.setRenderingSurface (std::make_unique<MetalRenderingSurface>());
    mainFigure.addListener (this);
    cmapFigure.addListener (this);

    setWantsKeyboardFocus (true);
    addAndMakeVisible (mainFigure);
    addAndMakeVisible (cmapFigure);
}

JetInCloudView::~JetInCloudView()
{
}

void JetInCloudView::update (const State &newState)
{
    if (mainFigure.getRenderingSurface() == nullptr && state.file.exists()) // this is delayed to speed app initialization
        mainFigure.setRenderingSurface (std::make_unique<MetalRenderingSurface>());

    state = newState;
    mainFigure.setModel (state.mainModel);
    cmapFigure.setModel (state.cmapModel);
}

void JetInCloudView::asynchronousTaskStarted()
{
    if (auto sink = findParentComponentOfClass<MessageSink>())
    {
        sink->viewerAsyncTaskStarted();
    }
}

void JetInCloudView::asynchronousTaskFinished()
{
    if (auto sink = findParentComponentOfClass<MessageSink>())
    {
        sink->viewerAsyncTaskFinished();
    }
}

void JetInCloudView::loadFile (File viewedDocument)
{
    store.dispatch (Action::SetFile { viewedDocument });
}

bool JetInCloudView::isInterestedInFile (File file) const
{
    return FileSystemSerializer::looksLikeDatabase (file);
}




//=============================================================================
void JetInCloudView::resized()
{
    layout.performLayout (getLocalBounds());
}




//=============================================================================
void JetInCloudView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& margin)
{
    if (figure == &mainFigure)
    {
        store.dispatch (Action::SetFigureMargin { margin });
    }
}

void JetInCloudView::figureViewSetDomainAndMargin (FigureView* figure,
                                                      const Rectangle<double>& domain,
                                                      const BorderSize<int>& margin)
{
    if (figure == &mainFigure)
    {
        auto d = domain.toFloat();
        store.dispatch (Action::SetFigureDomain { d.getX(), d.getRight(), d.getY(), d.getBottom() },
                        Action::SetFigureMargin { margin });
    }
}

void JetInCloudView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
{
    auto d = domain.toFloat();

    if (figure == &mainFigure)
    {
        store.dispatch (Action::SetFigureDomain { d.getX(), d.getRight(), d.getY(), d.getBottom() });
    }
    else
    {
        store.dispatch (Action::SetScalarDomain { d.getY(), d.getBottom() });
    }
}

void JetInCloudView::figureViewSetXlabel (FigureView* figure, const String& value)
{
}

void JetInCloudView::figureViewSetYlabel (FigureView* figure, const String& value)
{
}

void JetInCloudView::figureViewSetTitle (FigureView* figure, const String& value)
{
}




//=============================================================================
void JetInCloudView::getAllCommands (Array<CommandID>& commands)
{
    Viewer::getAllCommands (commands);
}

void JetInCloudView::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    Viewer::getCommandInfo (commandID, result);
}

bool JetInCloudView::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case Commands::makeSnapshotAndOpen: saveSnapshot (true); break;
        case Commands::saveSnapshotAs: saveSnapshot (false); break;
        case Commands::nextColourMap: store.dispatch (Action::SetColourMap {cmaps.next()}); break;
        case Commands::prevColourMap: store.dispatch (Action::SetColourMap {cmaps.prev()}); break;
        case Commands::resetScalarRange: store.dispatch (Action::SetScalarDomainToExtent()); break;
    }
    return true;
}

ApplicationCommandTarget* JetInCloudView::getNextCommandTarget()
{
    return nullptr;
}




//=============================================================================
void JetInCloudView::saveSnapshot (bool toTempDirectory)
{
    auto target = File();

    if (toTempDirectory)
    {
        target = File::createTempFile (".png");
    }
    else
    {
        FileChooser chooser ("Open directory...", state.file.getParentDirectory(), "", true, false, nullptr);

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




//=============================================================================
Viewer* JetInCloud::create()
{
    return new JetInCloudView;
}
