#include "UserExtensionView.hpp"
#include "yaml-cpp/yaml.h"
#include "../Core/DataHelpers.hpp"




//=============================================================================
UserExtensionView::UserExtensionView()
{
}

void UserExtensionView::configure (const var& config)
{
    models.clear();
    figures.clear();
    layout.items.clear();

	if (auto root = config.getDynamicObject())
	{
		if (auto figureItems = root->getProperty ("figures").getArray())
		{
			for (auto f : *figureItems)
			{
                models.add (FigureModel::fromVar (f));
			}
		}
        layout.templateColumns = DataHelpers::gridTrackInfoArrayFromVar (root->getProperty ("cols"));
        layout.templateRows    = DataHelpers::gridTrackInfoArrayFromVar (root->getProperty ("rows"));
        viewerName = root->getProperty ("name");
    }

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
        DBG(e.what());
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
