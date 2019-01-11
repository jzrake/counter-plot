#include "UserExtensionView.hpp"
#include "yaml-cpp/yaml.h"




//=============================================================================
static var varFromYamlScalar (const YAML::Node& scalar)
{
    assert (scalar.IsScalar());
    auto value = String (scalar.Scalar());

    if (! JSON::fromString (value).isVoid())
        return JSON::fromString (value);

    return value;
}

static var varFromYamlNode (const YAML::Node& node)
{
    switch (node.Type())
    {
        case YAML::NodeType::value::Null: return var();
        case YAML::NodeType::value::Scalar: return varFromYamlScalar (node);
        case YAML::NodeType::value::Undefined: return var::undefined();
        case YAML::NodeType::value::Sequence:
        {
            var res;

            for (const auto& child : node)
                res.append (varFromYamlNode (child));
            return res;
        }
        case YAML::NodeType::value::Map:
        {
            auto obj = std::make_unique<DynamicObject>();

            for (const auto& item : node)
                obj->setProperty (String (item.first.Scalar()), varFromYamlNode (item.second));
            return obj.release();
        }
    }
    return var();
}

static juce::Array<Grid::TrackInfo> getTrackInfoArray (const var& value)
{
    juce::Array<Grid::TrackInfo> info;

    if (auto arr = value.getArray())
    {
        for (auto item : *arr)
        {
            info.add (Grid::TrackInfo (Grid::Fr (int (item))));
        }
    }
    return info;
}





//=============================================================================
UserExtensionView::UserExtensionView()
{
}

void UserExtensionView::configure (const String& name, const var& config)
{
	viewerName = name;

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
        layout.templateColumns = getTrackInfoArray (root->getProperty ("rows"));
        layout.templateRows    = getTrackInfoArray (root->getProperty ("columns"));
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
    try {
        auto yroot = YAML::LoadFile (file.getFullPathName().toStdString());
        auto jroot = varFromYamlNode (yroot);
        configure (file.getFileNameWithoutExtension(), jroot);
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
bool  UserExtensionView::isInterestedInFile (File file) const
{
	return true;
}

void  UserExtensionView::loadFile (File fileToDisplay)
{
}

String  UserExtensionView::getViewerName() const
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
