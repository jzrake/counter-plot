#include "UserExtensionView.hpp"
#include "yaml-cpp/yaml.h"
#include "../Core/DataHelpers.hpp"
#include "../Core/Runtime.hpp"




//=============================================================================
UserExtensionView::UserExtensionView() : taskPool (4)
{
    Runtime::load_builtins (kernel);
    kernel.insert ("file", currentFile.getFullPathName());
    kernel.insert ("stops", Runtime::make_data (colourMaps.getCurrentStops()));

    taskPool.addListener (this);
    setWantsKeyboardFocus (true);
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
    }


    // Record the rules that are expensive (aka heavyweight, aka async).
    // -----------------------------------------------------------------------
    asyncRules = DataHelpers::stringArrayFromVar (config["expensive"]);


    // Load viewer environment and figure defs into the kernel
    // -----------------------------------------------------------------------
    loadExpressionsFromDictIntoKernel (kernel, config["environment"]);
    loadExpressionsFromListIntoKernel (kernel, config["figures"], "figure-");


    // Update kernel definitions (done synchronous on config)
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

        layout.items.add (figure.get());
        figures.add (figure.release());
    }


    // Load layout specification
    // -----------------------------------------------------------------------
    layout.templateColumns = DataHelpers::gridTrackInfoArrayFromVar (config["cols"]);
    layout.templateRows    = DataHelpers::gridTrackInfoArrayFromVar (config["rows"]);
    layout.performLayout (getLocalBounds());

    sendIndicateSuccess();
    sendEnvironmentChanged();
}

void UserExtensionView::configure (File file)
{
    viewerName = file.getFileName(); // will get overwritten if name property is specified

    try {
        auto yroot = YAML::LoadFile (file.getFullPathName().toStdString());
        auto jroot = DataHelpers::varFromYamlNode (yroot);
        patchConfigVar (jroot, recordHotReloadableData());
        configure (jroot);
    }
    catch (YAML::ParserException& e)
    {
        return sendErrorMessage (e.what());
    }
}




//=============================================================================
void UserExtensionView::resized()
{
    layout.performLayout (getLocalBounds());
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




//=========================================================================
void UserExtensionView::figureViewSetDomainAndMargin (FigureView* figure, const Rectangle<double>& domain, const BorderSize<int>& margin)
{
    auto id = figure->getComponentID().toStdString();
    auto expr = kernel.expr_at (id);
    expr.set ("xmin", domain.getX());
    expr.set ("ymin", domain.getY());
    expr.set ("xmax", domain.getRight());
    expr.set ("ymax", domain.getBottom());
    expr.set ("margin", DataHelpers::expressionFromVar (DataHelpers::varFromBorderSize (margin)));
    kernel.insert (id, expr);
    captureInKernel (kernel, figure);
    resolveKernel();
}

void UserExtensionView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& margin)
{
    auto id = figure->getComponentID().toStdString();
    auto expr = kernel.expr_at (id);
    expr.set ("margin", DataHelpers::expressionFromVar (DataHelpers::varFromBorderSize (margin)));
    kernel.insert (id, expr);
    captureInKernel (kernel, figure);
    resolveKernel();
}

void UserExtensionView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
{
    auto id = figure->getComponentID().toStdString();
    auto expr = kernel.expr_at (id);
    expr.set ("xmin", domain.getX());
    expr.set ("ymin", domain.getY());
    expr.set ("xmax", domain.getRight());
    expr.set ("ymax", domain.getBottom());
    kernel.insert (id, expr);
    captureInKernel (kernel, figure);
    resolveKernel();
}

void UserExtensionView::figureViewSetXlabel (FigureView* figure, const String& xlabel)
{
    auto id = figure->getComponentID().toStdString();
    auto expr = kernel.expr_at (id);
    expr.set ("xlabel", xlabel.toStdString());
    captureInKernel (kernel, figure);
    resolveKernel();
}

void UserExtensionView::figureViewSetYlabel (FigureView* figure, const String& ylabel)
{
    auto id = figure->getComponentID().toStdString();
    auto expr = kernel.expr_at (id);
    expr.set ("ylabel", ylabel.toStdString());
    captureInKernel (kernel, figure);
    resolveKernel();
}

void UserExtensionView::figureViewSetTitle (FigureView* figure, const String& title)
{
    auto id = figure->getComponentID().toStdString();
    auto expr = kernel.expr_at (id);
    expr.set ("title", title.toStdString());
    captureInKernel (kernel, figure);
    resolveKernel();
}




//=========================================================================
void UserExtensionView::taskStarted (const String& taskName)
{
    sendAsyncTaskStarted();
}

void UserExtensionView::taskCompleted (const String& taskName, const var& result, const std::string& error)
{
    kernel.update_directly (taskName.toStdString(), result, error);
    loadFromKernelIfFigure (taskName.toStdString());
    resolveKernel();
    sendAsyncTaskFinished();
}

void UserExtensionView::taskWasCancelled (const String& taskName)
{
    sendAsyncTaskFinished();
}




//=========================================================================
void UserExtensionView::resolveKernel()
{
    auto synchronousDirtyRules = kernel.dirty_rules_excluding (Runtime::asynchronous);
    kernel.update_all (synchronousDirtyRules);

    for (const auto& rule : synchronousDirtyRules)
        loadFromKernelIfFigure (rule);

    sendEnvironmentChanged();

    // After the synchronous updates are handled, we launch any
    // asynchronous ones. There's no need to recurse here, because new
    // rules will not become eligible until after the task is finished.
    // --------------------------------------------------------------
    for (auto rule : kernel.dirty_rules_only (Runtime::asynchronous))
    {
        if (kernel.current (kernel.incoming (rule)))
        {
            taskPool.enqueue (rule, [kernel=kernel, rule] (auto bailout)
            {
                auto what = std::string();
                auto adapter = VarCallAdapter (bailout);
                auto result = kernel.resolve (rule, what, adapter);

                if (what.empty())
                    return result;

                throw std::runtime_error(what);
            });
        }
    }
}

void UserExtensionView::loadFromKernelIfFigure (const std::string& id)
{
    if (auto figure = dynamic_cast<FigureView*> (findChildWithID (id)))
    {
        try {
            figure->setModel (FigureModel::fromVar (kernel.at (id), figure->getModel().withoutContent()));
        }
        catch (const std::exception& e)
        {
            kernel.set_error (id, e.what());
        }
    }
}

void UserExtensionView::loadExpressionsFromListIntoKernel (Runtime::Kernel& kernel, const var& list, const std::string& basename) const
{
    if (auto items = list.getArray())
    {
        int n = 0;

        for (auto item : *items)
        {
            auto id = basename + std::to_string(n);

            try {
                auto flag = asyncRules.contains (id.data()) ? Runtime::asynchronous : 0;
                auto expr = DataHelpers::expressionFromVar (item);

                if (! kernel.contains (id) || kernel.expr_at (id) != expr)
                {
                    kernel.insert (id, expr, flag);
                }
            }
            catch (const std::exception& e)
            {
                kernel.insert (id, var());
                kernel.set_error (id, e.what());
            }
            ++n;
        }
    }
}

void UserExtensionView::loadExpressionsFromDictIntoKernel (Runtime::Kernel& kernel, const var& dict) const
{
    if (auto obj = dict.getDynamicObject())
    {
        for (const auto& item : obj->getProperties())
        {
            auto key = item.name.toString().toStdString();

            try {
                auto flag = asyncRules.contains (key.data()) ? Runtime::asynchronous : 0;
                auto expr = DataHelpers::expressionFromVar (item.value);

                if (! kernel.contains (key) || expr != kernel.expr_at (key))
                {
                    kernel.insert (key, expr, flag);
                }
            }
            catch (const std::exception& e)
            {
                kernel.insert (key, var());
                kernel.set_error (key, e.what());
            }
        }
    }
}

void UserExtensionView::captureInKernel (Runtime::Kernel& kernel, const FigureView* figure) const
{
    const auto& model = figure->getModel();
    const auto& keys = model.capture.getAllKeys();
    const auto& vals = model.capture.getAllValues();

    for (int n = 0; n < keys.size(); ++n)
    {
        if (keys[n] == "xmin") kernel.insert (vals[n].toStdString(), var (model.xmin));
        if (keys[n] == "xmax") kernel.insert (vals[n].toStdString(), var (model.xmax));
        if (keys[n] == "ymin") kernel.insert (vals[n].toStdString(), var (model.ymin));
        if (keys[n] == "ymax") kernel.insert (vals[n].toStdString(), var (model.ymax));
    }
}

void UserExtensionView::patchConfigVar (var &root, const var &hotReloadData) const
{
    for (int n = 0; n < std::min (root["figures"].size(), hotReloadData.size()); ++n)
    {
        auto obj = root["figures"][n];
        DataHelpers::updateDict (obj, hotReloadData[n]);
    }
}

var UserExtensionView::recordHotReloadableData() const
{
    var result;

    for (auto figure : figures)
    {
        const auto& model = figure->getModel();
        auto obj = std::make_unique<DynamicObject>();

        obj->setProperty ("xmin", model.xmin);
        obj->setProperty ("xmax", model.xmax);
        obj->setProperty ("ymin", model.ymin);
        obj->setProperty ("ymax", model.ymax);
        obj->setProperty ("margin", DataHelpers::varFromBorderSize (model.margin));
        //        obj->setProperty ("xlabel", model.xlabel);
        //        obj->setProperty ("ylabel", model.ylabel);
        //        obj->setProperty ("title", model.title);

        result.append (obj.release());
    }
    return result;
}
