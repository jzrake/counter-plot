#include "UserExtensionView.hpp"
#include "../Core/DataHelpers.hpp"
#include "../Core/Runtime.hpp"
#include "yaml-cpp/yaml.h"




//=============================================================================
UserExtensionView::UserExtensionView() : taskPool (4)
{
    reset();
    taskPool.addListener (this);
    setWantsKeyboardFocus (true);
}

void UserExtensionView::reset()
{
    kernel.clear();
    Runtime::load_builtins (kernel);
    kernel.insert ("file", currentFile.getFullPathName());
    kernel.insert ("stops", Runtime::make_data (colourMaps.getCurrentStops()));

    taskPool.cancelAll();
    figures.clear();
    layout.items.clear();
}

void UserExtensionView::configure (const var& config)
{
    // Reset everything but the kernel
    // -----------------------------------------------------------------------
    taskPool.cancelAll();
    figures.clear();
    layout.items.clear();


    // Set the name of the viewer
    // -----------------------------------------------------------------------
    if (config.hasProperty ("name"))
        viewerName = config["name"];


    // Configure the file filter
    // -----------------------------------------------------------------------
    fileFilter.clear();

    if (config["file-patterns"].isVoid())
    {
        fileFilter.setRejectsAllFiles (true);
    }
    else
    {
        fileFilter.setFilePatterns (DataHelpers::stringArrayFromVar (config["file-patterns"]));

        if (auto arr = config["hdf5-required-groups"].getArray())
            for (auto item : *arr)
                fileFilter.requireHDF5Group (item.toString());

        if (auto arr = config["hdf5-required-datasets"].getArray())
            for (auto item : *arr)
                fileFilter.requireHDF5Dataset (item["name"], item.getProperty ("rank", -1));

        if (auto arr = config["patches2d-required-fields"].getArray())
            for (auto item : *arr)
                fileFilter.requirePatches2dField (item.toString());
    }


    // Record the rules that are expensive (aka heavyweight, aka async).
    // -----------------------------------------------------------------------
    asyncRules = DataHelpers::stringArrayFromVar (config["expensive"]);


    // Load commands
    // -----------------------------------------------------------------------
    extensionCommands = config["commands"];


    // Load viewer environment and figure defs into the kernel
    // -----------------------------------------------------------------------
    loadExpressionsFromDictIntoKernel (kernel, config["environment"]);
    loadExpressionsFromDictIntoKernel (kernel, DataHelpers::makeDictFromList (config["figures"], "figure-"));


    // Update kernel definitions (synchronously on config)
    // -----------------------------------------------------------------------
    kernel.update_all (kernel.dirty_rules());


    // Create the figure components
    // -----------------------------------------------------------------------
    for (int n = 0; n < config["figures"].size(); ++n)
    {
        auto figure = std::make_unique<FigureView>();
        auto id = "figure-" + std::to_string(n);

        figure->addListener (this);
        figure->setComponentID (id);
        figure->setModel (FigureModel::fromVar (kernel.at (id), FigureModel()));

        addAndMakeVisible (figure.get());
        layout.items.add (GridItem());
        figures.add (figure.release());
    }


    // Load layout specification
    // -----------------------------------------------------------------------
    layout.templateColumns = DataHelpers::gridTrackInfoArrayFromVar (config["cols"]);
    layout.templateRows    = DataHelpers::gridTrackInfoArrayFromVar (config["rows"]);
    applyLayout();

    sendIndicateSuccess();
    sendEnvironmentChanged();
}

void UserExtensionView::configure (File file)
{
    viewerName = file.getFileName(); // will get overwritten if name property is specified

    try {
        auto yroot = YAML::LoadFile (file.getFullPathName().toStdString());
        auto jroot = DataHelpers::varFromYamlNode (yroot);
        configure (jroot);
    }
    catch (const std::exception& e)
    {
        return sendErrorMessage (e.what());
    }
}




//=============================================================================
void UserExtensionView::resized()
{
    applyLayout();
}

void UserExtensionView::applyLayout()
{
    layout.performLayout (getLocalBounds());

    int n = 0;

    for (auto item : layout.items)
    {
        figures[n]->setBoundsAndSendDomainResizeIfNeeded (item.currentBounds.toNearestIntEdges());
        ++n;
    }
}




//=============================================================================
bool UserExtensionView::isInterestedInFile (File file) const
{
    if (file.existsAsFile())
        return fileFilter.isFileSuitable (file);
    if (file.isDirectory())
        return fileFilter.isDirectorySuitable (file);
    return false;
}

void UserExtensionView::loadFile (File fileToDisplay)
{
    if (currentFile != fileToDisplay)
    {
        currentFile = fileToDisplay;
        reloadFile();
    }
}

void UserExtensionView::reloadFile()
{
    kernel.insert ("file", currentFile.getFullPathName());
    resolveKernel();
}

String UserExtensionView::getViewerName() const
{
    return viewerName;
}

const Runtime::Kernel* UserExtensionView::getKernel() const
{
    return &kernel;
}

bool UserExtensionView::canReceiveMessages() const
{
    return true;
}

bool UserExtensionView::receiveMessage (const String& message)
{
    try {
        auto yroot = YAML::Load (message.toStdString());
        auto jroot = DataHelpers::varFromYamlNode (yroot);
        loadExpressionsFromDictIntoKernel (kernel, jroot, true);
        resolveKernel();
        return true;
    }
    catch (const std::exception& e)
    {
        sendErrorMessage (e.what());
        return false;
    }
}

bool UserExtensionView::isRenderingComplete() const
{
    return taskPool.getNumJobsRunningOrQueued() == 0 && kernel.dirty_rules().empty();
}

Image UserExtensionView::createViewerSnapshot()
{
    for (auto figure : figures)
        if (figure->isVisible())
            figure->captureRenderingSurfaceInNextPaint();

    return createComponentSnapshot (getLocalBounds());
}




//=========================================================================
void UserExtensionView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& margin)
{
    const auto& model = figure->getModel();
    const auto& capture = model.capture;

    if (capture.count ("margin"))
        kernel.insert (capture.at ("margin"), DataHelpers::varFromBorderSize (margin));

    if (! figure->sendDomainResizeForNewMargin (margin))
        resolveKernel();
}

void UserExtensionView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
{
    auto x0 = domain.getX();
    auto x1 = domain.getRight();
    auto y0 = domain.getY();
    auto y1 = domain.getBottom();

    const auto& capture = figure->getModel().capture;
    if (capture.count ("domain")) kernel.insert (capture.at ("domain"), Array<var>{x0, x1, y0, y1});
    if (capture.count ("xmin")) kernel.insert (capture.at ("xmin"), var (x0));
    if (capture.count ("xmax")) kernel.insert (capture.at ("xmax"), var (x1));
    if (capture.count ("ymin")) kernel.insert (capture.at ("ymin"), var (y0));
    if (capture.count ("ymax")) kernel.insert (capture.at ("ymax"), var (y1));
    resolveKernel();
}

void UserExtensionView::figureViewSetXlabel (FigureView* figure, const String& xlabel)
{
    const auto& capture = figure->getModel().capture;
    kernel.insert (capture.at ("xlabel"), xlabel);
    resolveKernel();
}

void UserExtensionView::figureViewSetYlabel (FigureView* figure, const String& ylabel)
{
    const auto& capture = figure->getModel().capture;
    kernel.insert (capture.at ("ylabel"), ylabel);
    resolveKernel();
}

void UserExtensionView::figureViewSetTitle (FigureView* figure, const String& title)
{
    const auto& capture = figure->getModel().capture;
    kernel.insert (capture.at ("title"), title);
    resolveKernel();
}




//=============================================================================
void UserExtensionView::getAllCommands (Array<CommandID>& commands)
{
    Viewer::getAllCommands (commands);
}

void UserExtensionView::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    Viewer::getCommandInfo (commandID, result);
}

bool UserExtensionView::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case Commands::makeSnapshotAndOpen: saveSnapshot (true); break;
        case Commands::saveSnapshotAs: saveSnapshot (false); break;
        case Commands::nextColourMap: kernel.insert ("stops", Runtime::make_data (colourMaps.next())); resolveKernel(); break;
        case Commands::prevColourMap: kernel.insert ("stops", Runtime::make_data (colourMaps.prev())); resolveKernel(); break;
        case Commands::resetScalarRange:
            loadExpressionsFromDictIntoKernel (kernel, extensionCommands["reset-scalar-range"]);
            resolveKernel();
            break;
    }
    return true;
}

ApplicationCommandTarget* UserExtensionView::getNextCommandTarget()
{
    return nullptr;
}




//=========================================================================
void UserExtensionView::taskStarted (const String& taskName)
{
    sendAsyncTaskStarted (taskName);
}

void UserExtensionView::taskCompleted (const String& taskName, const var& result, const std::string& error)
{
    auto key = taskName.toStdString();
    kernel.update_directly (key, result, error);
    kernel.mark (kernel.downstream (key));
    loadFromKernelIfFigure (key);
    resolveKernel();
    sendAsyncTaskCompleted (taskName);
}

void UserExtensionView::taskCancelled (const String& taskName)
{
    sendAsyncTaskCancelled (taskName);
}




//=========================================================================
void UserExtensionView::resolveKernel()
{

    // Try to update all the rules that are dirty and not asynchronous.
    // We don't use kernel::update_all here, because that recurses and
    // can hit rules that become eligible, but are asynchronous. Not all
    // of the rules will be updated, since some will have asynchronous
    // depenencies. Below we record the number of rules that were updated
    // in each pass, and break if none had become eligible.
    // --------------------------------------------------------------
    auto synchronousDirtyRules = kernel.dirty_rules_excluding (Runtime::asynchronous);

    while (! synchronousDirtyRules.empty())
    {
        int numUpdatedRules = 0;

        for (const auto& rule : synchronousDirtyRules)
        {
            if (kernel.eligible (rule))
            {
                kernel.update (rule);
                loadFromKernelIfFigure (rule);
                ++numUpdatedRules;
            }
        }
        if (numUpdatedRules == 0)
        {
            break;
        }
        synchronousDirtyRules = kernel.dirty_rules_excluding (Runtime::asynchronous);
    }

    sendEnvironmentChanged();


    // After the synchronous updates are handled, we launch any
    // asynchronous ones. There's no need to recurse here, because new
    // rules will not become eligible until after the task is finished.
    // Each rule that is eligible for update gets enqueued, which
    // cancels any earlier tasks with the same name. Rules that are
    // asynchronous and dirty, but not eligible, are canceled.
    // --------------------------------------------------------------
    for (auto rule : kernel.dirty_rules_only (Runtime::asynchronous))
    {
        if (kernel.eligible (rule))
        {
            kernel.unmark (rule);

            taskPool.enqueue (rule, [kernel=kernel, rule] (auto bailout)
            {
                auto what = std::string();
                auto adapter = VarCallAdapter (bailout);
                auto result = kernel.resolve (rule, what, adapter);

                if (what.empty())
                {
                    return result;
                }
                throw std::runtime_error (what);
            });
        }
        else
        {
            taskPool.cancel (rule);
        }
    }
}

void UserExtensionView::loadFromKernelIfFigure (const std::string& id)
{
    if (auto figure = dynamic_cast<FigureView*> (findChildWithID (id)))
    {
        try {
            auto model = FigureModel::fromVar (kernel.at (id), figure->getModel().withoutContent());
            model.canEditTitle  = model.capture.count ("title");
            model.canEditXlabel = model.capture.count ("xlabel");
            model.canEditYlabel = model.capture.count ("ylabel");
            model.canEditMargin = model.capture.count ("margin");
            figure->setModel (model);
        }
        catch (const std::exception& e)
        {
            kernel.set_error (id, e.what());
        }
    }
}

void UserExtensionView::loadExpressionsFromDictIntoKernel (Runtime::Kernel& kernel, const var& dict, bool rethrowExceptions) const
{
    if (auto obj = dict.getDynamicObject())
    {
        for (const auto& item : obj->getProperties())
        {
            auto key = item.name.toString().toStdString();

            try {
                auto flag = asyncRules.contains (key.data()) ? Runtime::asynchronous : 0;
                auto expr = DataHelpers::expressionFromVar (item.value);

                if (     !  kernel.contains (key) ||
                    expr != kernel.expr_at (key) ||
                    flag != kernel.flags_at (key))
                    kernel.insert (key, expr, flag);
            }
            catch (const std::exception& e)
            {
                if (rethrowExceptions)
                {
                    throw std::runtime_error (e.what());
                }
                kernel.insert (key, var());
                kernel.set_error (key, e.what());
            }
        }
    }
}

void UserExtensionView::saveSnapshot (bool toTempDirectory)
{
    auto target = File();
    
    if (toTempDirectory)
    {
        target = File::createTempFile (".png");
    }
    else
    {
        FileChooser chooser ("Choose Save Location...", currentFile.getParentDirectory(), "", true, false, nullptr);
        
        if (chooser.browseForFileToSave (true))
            target = chooser.getResult();
        else
            return;
    }

    target.deleteFile();

    if (auto stream = std::unique_ptr<FileOutputStream> (target.createOutputStream()))
    {
        auto format = PNGImageFormat();
        auto image = createViewerSnapshot();
        format.writeImageToStream (image, *stream);
    }
    if (toTempDirectory)
    {
        target.startAsProcess();
    }
}
