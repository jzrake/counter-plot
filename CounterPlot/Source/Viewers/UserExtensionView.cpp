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
    models.clear();
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
                    sendErrorMessage ("bad expression in environment block: " + std::string (e.what()));
                    ++errors;
                }
            }
            else
            {
                kernel.insert (key, item.value);
            }
        }
    }

    for (const auto& item : kernel)
    {
        if (kernel.dirty (item.first))
        {
            if (! kernel.update (item.first))
            {
                sendErrorMessage ("could not resolve kernel symbol: " + item.first);
                ++errors;
            }
        }
    }


    // Add figures
    // -----------------------------------------------------------------------
    if (auto figureItems = config["figures"].getArray())
    {
        for (auto f : *figureItems)
        {
            auto model = FigureModel::fromVar (f);

            if (auto content = f["content"].getArray())
            {
                for (const auto& element : *content)
                {
                    try {
                        auto artistExpression = crt::parser::parse (element.toString().toStdString().data());
                        auto artistAsVar = artistExpression.resolve<var, VarCallAdapter> (kernel);
                        auto artist = Runtime::check_data<std::shared_ptr<PlotArtist>> (artistAsVar);
                        model.content.push_back (artist);
                    }
                    catch (const crt::parser_error& e)
                    {
                        sendErrorMessage ("bad expression in figure content block: " + std::string (e.what()));
                        ++errors;
                    }
                    catch (const std::out_of_range& e)
                    {
                        sendErrorMessage (e.what());
                        ++errors;
                    }
                    catch (const std::exception& e)
                    {
                        sendErrorMessage (e.what());
                        ++errors;
                    }
                }
            }
            models.add (model);
        }
    }


    // Load layout specification
    // -----------------------------------------------------------------------
    layout.templateColumns = DataHelpers::gridTrackInfoArrayFromVar (config["cols"]);
    layout.templateRows    = DataHelpers::gridTrackInfoArrayFromVar (config["rows"]);


	for (const auto& model : models)
	{
        auto figure = std::make_unique<FigureView>();
        figure->setModel (model);
        figure->addListener (this);
        addAndMakeVisible (*figure);
		layout.items.add (figure->getGridItem());
        figures.add (figure.release());
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
}

String UserExtensionView::getViewerName() const
{
	return viewerName;
}




//=========================================================================
void UserExtensionView::figureViewSetDomainAndMargin (FigureView* figure, const Rectangle<double>& domain, const BorderSize<int>& margin)
{
    auto index = figures.indexOf (figure);
    auto& model = models.getReference (index);
    model.xmin = domain.getX();
    model.ymin = domain.getY();
    model.xmax = domain.getRight();
    model.ymax = domain.getBottom();
    model.margin = margin;
    figure->setModel (model);
}

void UserExtensionView::figureViewSetMargin (FigureView* figure, const BorderSize<int>& margin)
{
    auto index = figures.indexOf (figure);
    auto& model = models.getReference (index);
    model.margin = margin;
    figure->setModel (model);
}

void UserExtensionView::figureViewSetDomain (FigureView* figure, const Rectangle<double>& domain)
{
    auto index = figures.indexOf (figure);
    auto& model = models.getReference (index);
    model.xmin = domain.getX();
    model.ymin = domain.getY();
    model.xmax = domain.getRight();
    model.ymax = domain.getBottom();
    figure->setModel (model);
}

void UserExtensionView::figureViewSetXlabel (FigureView* figure, const String& xlabel)
{
    auto index = figures.indexOf (figure);
    auto& model = models.getReference (index);
    model.xlabel = xlabel;
    figure->setModel (model);
}

void UserExtensionView::figureViewSetYlabel (FigureView* figure, const String& ylabel)
{
    auto index = figures.indexOf (figure);
    auto& model = models.getReference (index);
    model.ylabel = ylabel;
    figure->setModel (model);
}

void UserExtensionView::figureViewSetTitle (FigureView* figure, const String& title)
{
    auto index = figures.indexOf (figure);
    auto& model = models.getReference (index);
    model.title = title;
    figure->setModel (model);
}
