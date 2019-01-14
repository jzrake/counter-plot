#include "DataHelpers.hpp"
#include "yaml-cpp/yaml.h"




//=============================================================================
YAML::Node DataHelpers::nodeFromVar (const var& value)
{
    if (value.isVoid()) return YAML::Node();
    if (value.isString()) return YAML::Node (value.toString().toStdString());
    if (value.isBool()) return YAML::Node (bool (value));
    if (value.isInt()) return YAML::Node (int (value));
    if (value.isDouble()) return YAML::Node (double (value));
    if (value.isArray())
    {
        YAML::Node node;

        for (const auto& element : *value.getArray())
        {
            node.push_back (nodeFromVar (element));
        }
        return node;
    }
    if (auto obj = value.getDynamicObject())
    {
        YAML::Node node;

        for (const auto& item : obj->getProperties())
        {
            node[item.name.toString().toStdString()] = nodeFromVar (item.value);
        }
        return node;
    }
    return YAML::Node();
}

var DataHelpers::varFromYamlScalar (const YAML::Node& scalar)
{
    assert (scalar.IsScalar());
    auto value = String (scalar.Scalar());

    if (! JSON::fromString (value).isVoid())
        return JSON::fromString (value);

    return value;
}

var DataHelpers::varFromYamlNode (const YAML::Node& node)
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

Array<Grid::TrackInfo> DataHelpers::gridTrackInfoArrayFromVar (const var& value)
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

Colour DataHelpers::colourFromVar (const var& value)
{
    if (value.isVoid())
    {
        return Colours::transparentWhite;
    }
    if (value.isInt())
    {
        return Colour (int (value));
    }
    if (value.isString())
    {
        return Colour::fromString (value.toString());
    }
    if (value.size() == 3)
    {
        return Colour::fromRGB (int (value[0]), int (value[1]), int (value[2]));
    }
    if (value.size() == 4)
    {
        return Colour::fromRGBA (int (value[0]), int (value[1]), int (value[2]), int (value[3]));
    }
    return Colours::black;
}

