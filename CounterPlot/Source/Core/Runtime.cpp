#include "Runtime.hpp"
#include "yaml-cpp/yaml.h"




//=============================================================================
crt::expression crt::fromYamlFile (File source)
{
    return fromYamlNode (YAML::LoadFile (source.getFullPathName().toStdString()));
}

crt::expression crt::fromYamlNode (const YAML::Node &node)
{
    std::vector<crt::expression> result;

    switch (node.Type())
    {
        case YAML::NodeType::value::Null: return crt::expression::none();
        case YAML::NodeType::value::Undefined: return crt::expression::none();
        case YAML::NodeType::value::Scalar: return fromYamlScalar (node.Scalar());
        case YAML::NodeType::value::Sequence:
            for (const auto& elem : node)
                result.push_back (fromYamlNode (elem));
            break;
        case YAML::NodeType::value::Map:
            for (const auto& item : node)
                result.push_back (fromYamlNode (item.second).keyed (item.first.Scalar()));
            break;
    }
    return result;
}

crt::expression crt::fromYamlScalar (const std::string& source)
{
    auto value = JSON::fromString (source);

    if (source.empty())
        return crt::expression::none();
    if (source[0] == '$')
        return crt::expression::symbol (source);
    if (source[0] == '(')
        return crt::parse (source);
    if (value.isInt())
        return int (value);
    if (value.isDouble())
        return double (value);
    if (value.isString())
        return source;

    return crt::expression::none();
}




//=============================================================================
const char* crt::type_info<Colour>::name()
{
    return "color";
}

crt::expression crt::type_info<Colour>::to_table (const Colour& val)
{
    return {
        expression (val.getRed())  .keyed ("r"),
        expression (val.getGreen()).keyed ("g"),
        expression (val.getBlue()) .keyed ("b"),
        expression (val.getAlpha()).keyed ("a"),
    };
}

Colour crt::type_info<Colour>::from_expr (const expression& e)
{
    if (e.has_type (crt::data_type::data))
    {
        return e.check_data<Colour>();
    }
    if (e.empty())
    {
        return Colours::transparentWhite;
    }
    if (e.has_type (crt::data_type::i32))
    {
        return Colour (e.get_i32());
    }
    if (e.has_type (crt::data_type::str))
    {
        String str = e.get_str();
        
        if (str.startsWithChar ('#'))
        {
            return Colour::fromString (str);
        }
        return Colours::findColourForName (str, Colours::black);
    }
    if (e.has_type (crt::data_type::table))
    {
        auto list = e.list();
        auto dict = e.dict();

        if (list.size() == 3)
        {
            return Colour::fromRGB (int (list[0]), int (list[1]), int (list[2]));
        }
        if (list.size() == 4)
        {
            return Colour::fromRGBA (int (list[0]), int (list[1]), int (list[2]), int (list[3]));
        }
        if (dict.size() == 3)
        {
            return Colour::fromRGB (int (e.attr ("r")),
                                    int (e.attr ("g")),
                                    int (e.attr ("b")));
        }
        if (dict.size() == 4)
        {
            return Colour::fromRGBA (int (e.attr ("r")),
                                     int (e.attr ("g")),
                                     int (e.attr ("b")),
                                     int (e.attr ("a")));
        }
    }
    return Colours::black;
}




//=============================================================================
const char* crt::type_info<ConfigurableFileFilter>::name()
{
    return "file-filter";
}

crt::expression crt::type_info<ConfigurableFileFilter>::to_table (const ConfigurableFileFilter& filter)
{
    return crt::parse (filter.getSourceString().toStdString());
}

ConfigurableFileFilter crt::type_info<ConfigurableFileFilter>::from_expr (const expression& e)
{
    ConfigurableFileFilter filter;

    filter.setSourceString (e.unparse());

    if (e.attr ("file-patterns").empty())
    {
        filter.setRejectsAllFiles (true);
    }
    else
    {
        for (auto elem : e.attr ("file-patterns"))
            filter.addFilePattern (elem.get_str());

        for (auto elem : e.attr ("hdf5-required-groups"))
            filter.requireHDF5Group (elem.get_str());

        for (auto elem : e.attr ("hdf5-required-datasets"))
            filter.requireHDF5Dataset (elem.attr ("name").get_str(), elem.attr ("rank").otherwise (-1));

        for (auto elem : e.attr ("patches2d-required-fields"))
            filter.requirePatches2dField (elem.get_str());
    }
    return filter;
}
