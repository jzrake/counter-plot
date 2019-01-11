#include "BinaryTorquesView.hpp"
#include "../MetalSurface.hpp"




//=============================================================================
struct TriangleVertexData
{
    std::array<float, 2> scalarExtent = {0.f, 1.f};
    std::shared_ptr<std::vector<simd::float2>> vertices;
    std::shared_ptr<std::vector<simd::float1>> scalars;
};

static struct TriangleVertexData loadTriangleDataFromFile (File file, std::function<bool()> bailout)
{
    try {
        auto h5f  = h5::File (file.getFullPathName().toStdString());
        auto h5d  = h5f.open_dataset ("primitive/sigma");
        auto data = h5d.read<nd::array<double, 2>>();
        auto scaled = MeshHelpers::scaleByLog10 (data, bailout);
        auto V = MeshHelpers::triangulateUniformRectilinearMesh (data.shape(0), data.shape(1), {-8.f, 8.f, -8.f, 8.f}, bailout);
        auto S = MeshHelpers::makeRectilinearGridScalars (scaled, bailout);
        auto E = MeshHelpers::findScalarExtent (scaled, bailout);

        return {
            E,
            std::make_shared<std::vector<simd::float2>> (V),
            std::make_shared<std::vector<simd::float1>> (S)
        };
    }
    catch (std::exception& e)
    {
        DBG("failed to load: " << e.what());
        return {};
    }
}




//=============================================================================
class BinaryTorquesViewFactory::QuadmeshArtist : public PlotArtist
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




//=============================================================================
using GradientArtist = ColourGradientArtist;
using QuadmeshArtist = BinaryTorquesViewFactory::QuadmeshArtist;




//=============================================================================
struct State
{
    std::shared_ptr<QuadmeshArtist>       quadmesh;
    std::shared_ptr<GradientArtist>       gradient;
    TriangleVertexData                    triangles;
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
        gradient->setMapping (mapping);
        quadmesh->setMapping (mapping);

        mainModel.titleShowing = true;
        mainModel.xlabelShowing = false;
        mainModel.ylabelShowing = false;
        mainModel.canEditMargin = true;
        mainModel.canEditXlabel = false;
        mainModel.canEditYlabel = false;
        mainModel.canEditTitle = false;
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
    struct SetTriangleData   { TriangleVertexData triangles; };
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
        state.file = action.file;
        state.mainModel.title = action.file.getFileName();
        dispatch ([action] (auto bailout) { return Action::SetTriangleData { loadTriangleDataFromFile (action.file, bailout)}; });
        subscriber.update (state);
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
        state.gradient->setMapping (state.mapping);
        state.quadmesh->setMapping (state.mapping);
        subscriber.update (state);
    }

    void dispatch (Action::SetScalarDomain action)
    {
        state.mapping.vmin = action.vmin;
        state.mapping.vmax = action.vmax;
        state.cmapModel.ymin = action.vmin;
        state.cmapModel.ymax = action.vmax;
        state.gradient->setMapping (state.mapping);
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




//=============================================================================
class BinaryTorquesView
: public FileBasedView
, public Store::Subscriber
, public FigureView::Listener
, public ApplicationCommandTarget
{
public:
    class QuadmeshArtist;

    //=========================================================================
    BinaryTorquesView();
    ~BinaryTorquesView();

    //=========================================================================
    void update (const State&) override;
    void asynchronousTaskStarted() override;
    void asynchronousTaskFinished() override;

    //=========================================================================
    bool isInterestedInFile (File file) const override;
    void loadFile (File fileToDisplay) override;
    String getViewerName() const override { return "Binary Torque Problem"; }

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
BinaryTorquesView::BinaryTorquesView() : store (*this)
{
    layout.templateRows    = { Grid::TrackInfo (4_fr) };
    layout.templateColumns = { Grid::TrackInfo (1_fr), Grid::TrackInfo (100_px) };
    layout.items.add (mainFigure.getGridItem());
    layout.items.add (cmapFigure.getGridItem());

    mainFigure.setRenderingSurface (std::make_unique<MetalRenderingSurface>());
    mainFigure.addListener (this);
    cmapFigure.addListener (this);

    setWantsKeyboardFocus (true);
    addAndMakeVisible (mainFigure);
    addAndMakeVisible (cmapFigure);
}

BinaryTorquesView::~BinaryTorquesView()
{
}

void BinaryTorquesView::update (const State &newState)
{
    state = newState;
    mainFigure.setModel (state.mainModel);
    cmapFigure.setModel (state.cmapModel);
}

void BinaryTorquesView::asynchronousTaskStarted()
{
    if (auto sink = findParentComponentOfClass<MessageSink>())
    {
        sink->fileBasedViewAsyncTaskStarted();
    }
}

void BinaryTorquesView::asynchronousTaskFinished()
{
    if (auto sink = findParentComponentOfClass<MessageSink>())
    {
        sink->fileBasedViewAsyncTaskFinished();
    }
}

void BinaryTorquesView::loadFile (File viewedDocument)
{
    store.dispatch (Action::SetFile { viewedDocument });
}

bool BinaryTorquesView::isInterestedInFile (File file) const
{
    return file.hasFileExtension (".h5");
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
        store.dispatch (Action::SetFigureMargin { margin });
    }
}

void BinaryTorquesView::figureViewSetDomainAndMargin (FigureView* figure,
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

void BinaryTorquesView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
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
        case Commands::nextColourMap: store.dispatch (Action::SetColourMap {cmaps.next()}); break;
        case Commands::prevColourMap: store.dispatch (Action::SetColourMap {cmaps.prev()}); break;
        case Commands::resetScalarRange: store.dispatch (Action::SetScalarDomainToExtent()); break;
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
FileBasedView* BinaryTorquesViewFactory::createNewVersion()
{
    return new BinaryTorquesView;
}
