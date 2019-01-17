#include "UserExtensionView.hpp"
#include "yaml-cpp/yaml.h"
#include "../Core/DataHelpers.hpp"
#include "../Core/Runtime.hpp"




//=============================================================================
UserExtensionView::UserExtensionView()
{

    auto data = Runtime::make_data (nd::linspace<double> (0.0, 1.0, 10));
    auto res = Runtime::check_data<nd::array<double, 1>>(data);

}

void UserExtensionView::configure (const var& config)
{
    int errors = 0;

    kernel.clear();
    figures.clear();
    layout.items.clear();


    // Set the name of the viewer
    // -----------------------------------------------------------------------
    if (config.hasProperty ("name"))
    {
        viewerName = config["name"];
    }


    // Load the viewer environment into the kernel
    // -----------------------------------------------------------------------
    Runtime::load_builtins (kernel);

    kernel.insert ("file", currentFile.getFullPathName());

    if (auto environment = config["environment"].getDynamicObject())
    {
        for (const auto& item : environment->getProperties())
        {
            auto key = item.name.toString().toStdString();

            if (item.value.isString())
            {
                try {
                    auto val = item.value.toString().toStdString();
                    kernel.insert (key, crt::parser::parse (val.data()));
                }
                catch (const std::exception& e)
                {
                    kernel.insert (key, var());
                    kernel.set_error (key, e.what());
                    ++errors;
                }
            }
            else
            {
                kernel.insert (key, item.value);
            }
        }
    }


    // Load figure definitions into the kernel
    // -----------------------------------------------------------------------
    if (auto items = config["figures"].getArray())
    {
        for (auto item : *items)
        {
            auto id = "figures:" + std::to_string (figures.size());

            try {
                kernel.insert (id, DataHelpers::expressionFromVar (item));
            }
            catch (const std::exception& e)
            {
                kernel.insert (id, FigureModel().toVar());
                kernel.set_error (id, e.what());
                // sendErrorMessage (e.what());
                ++errors;
            }

            figures.add (new FigureView);
            figures.getLast()->setComponentID (id);
        }
    }
    errors += resolveKernel();


    // Load layout specification
    // -----------------------------------------------------------------------
    layout.templateColumns = DataHelpers::gridTrackInfoArrayFromVar (config["cols"]);
    layout.templateRows    = DataHelpers::gridTrackInfoArrayFromVar (config["rows"]);


    for (auto figure : figures)
    {
        figure->addListener (this);
        addAndMakeVisible (*figure);
        layout.items.add (figure->getGridItem());
    }

    layout.performLayout (getLocalBounds());

    if (errors == 0)
    {
        sendIndicateSuccess();
    }
}

void UserExtensionView::configure (File file)
{
    viewerName = file.getFileName(); // will get overwritten if name property is specified

    try {
        auto yroot = YAML::LoadFile (file.getFullPathName().toStdString());
        auto jroot = DataHelpers::varFromYamlNode (yroot);
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
    return true;
}

void UserExtensionView::loadFile (File fileToDisplay)
{
    if (currentFile != fileToDisplay)
    {
        currentFile = fileToDisplay;
        kernel.insert ("file", fileToDisplay.getFullPathName());
        resolveKernel();
    }
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
    auto model = figure->getModel();
    model.xmin = domain.getX();
    model.ymin = domain.getY();
    model.xmax = domain.getRight();
    model.ymax = domain.getBottom();
    model.margin = margin;
    figure->setModel (model);
}

void UserExtensionView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& margin)
{
    auto model = figure->getModel();
    model.margin = margin;
    figure->setModel (model);
}

void UserExtensionView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
{
    auto model = figure->getModel();
    model.xmin = domain.getX();
    model.ymin = domain.getY();
    model.xmax = domain.getRight();
    model.ymax = domain.getBottom();

//    if (model.id.isNotEmpty())
//    {
//        kernel.insert (model.id.toStdString() + ".xmin", var (model.xmin));
//        kernel.insert (model.id.toStdString() + ".xmax", var (model.xmax));
//        kernel.insert (model.id.toStdString() + ".ymin", var (model.ymin));
//        kernel.insert (model.id.toStdString() + ".ymax", var (model.ymax));
//    }
    figure->setModel (model);
}

void UserExtensionView::figureViewSetXlabel (FigureView* figure, const String& xlabel)
{
    auto model = figure->getModel();
    model.xlabel = xlabel;
    figure->setModel (model);
}

void UserExtensionView::figureViewSetYlabel (FigureView* figure, const String& ylabel)
{
    auto model = figure->getModel();
    model.ylabel = ylabel;
    figure->setModel (model);
}

void UserExtensionView::figureViewSetTitle (FigureView* figure, const String& title)
{
    auto model = figure->getModel();
    model.title = title;
    figure->setModel (model);
}




//=========================================================================
int UserExtensionView::resolveKernel()
{
    int errors = 0;
    auto initiallyDirtyRules = kernel.dirty_rules();
    kernel.update_all (initiallyDirtyRules);

    for (auto figure : figures)
    {
        auto id = figure->getComponentID().toStdString();

        if (initiallyDirtyRules.count (id))
        {
            try {
                figure->setModel (FigureModel::fromVar (kernel.at (id)));
            }
            catch (const std::exception& e)
            {
                kernel.set_error (id, e.what());
                ++errors;
            }
        }
    }
    sendEnvironmentChanged();
    return errors;
}
