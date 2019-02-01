#include "Runtime.hpp"
#include "yaml-cpp/yaml.h"




//=============================================================================
void crt::core::import(crt::kernel& k)
{
    k.define("apply",     apply);
    k.define("attr",      attr);
    k.define("concat",    concat);
    k.define("dict",      dict);
    k.define("eval",      eval);
    k.define("first",     first);
    k.define("item",      item);
    k.define("join",      join);
    k.define("last",      last);
    k.define("len",       len);
    k.define("list",      list);
    k.define("map",       map);
    k.define("nest",      nest);
    k.define("range",     range);
    k.define("rest",      rest);
    k.define("reverse",   reverse);
    k.define("second",    second);
    k.define("slice",     slice);
    k.define("sort",      sort);
    k.define("table",     table);
    k.define("type",      type);
    k.define("unparse",   unparse);
    k.define("zip",       zip);

    k.define ("grid", crt::init<Grid>());
}




//=============================================================================
crt::expression crt::core::table(const crt::expression& e)
{
    return e;
}

crt::expression crt::core::list(const crt::expression& e)
{
    return e.list();
}

crt::expression crt::core::dict(const crt::expression& e)
{
    return e.dict();
}

crt::expression crt::core::item(const crt::expression& e)
{
    auto arg = e.first();
    auto ind = e.second();

    if (e.second().has_type(crt::data_type::i32))
    {
        return arg.item(ind.get_i32());
    }
    if (ind.has_type(crt::data_type::table))
    {
        std::vector<crt::expression> result;
        
        for (const auto& i : ind)
        {
            result.push_back(arg.item(int(i)).keyed(i.key()));
        }
        return result;
    }
    return {};
}

crt::expression crt::core::attr(const crt::expression& e)
{
    return e.first().attr(e.second());
}

crt::expression crt::core::range(const crt::expression& e)
{
    int start = 0;
    int final = 0;
    int steps = 1;

    if (e.size() == 1)
    {
        final = e.item(0);
    }
    else if (e.size() == 2)
    {
        start = e.item(0);
        final = e.item(1);
    }
    else if (e.size() == 3)
    {
        start = e.item(0);
        final = e.item(1);
        steps = e.item(2);
    }

    std::vector<crt::expression> result;

    if (start < final && steps > 0)
        for (int n = start; n < final; n += steps)
            result.push_back(n);

    if (start > final && steps < 0)
        for (int n = start; n > final; n += steps)
            result.push_back(n);

    return result;
}

crt::expression crt::core::slice(const crt::expression& e)
{
    return item({e.first(), range(e.rest())});
}

crt::expression crt::core::concat(const crt::expression& e)
{
    std::vector<crt::expression> result;

    for (const auto& part : e)
    {
        result.insert(result.end(), part.begin(), part.end());
    }
    return result;
}

crt::expression crt::core::join(const crt::expression& e)
{
    std::string result;
    std::string sep = e.attr("sep");

    bool first = true;

    for (const auto& part : e.list())
    {
        result += (first ? "" : sep) + std::string(part);
        first = false;
    }
    return result;
}

crt::expression crt::core::apply(const crt::expression& e)
{
    return e.first().call(e.second());
}

crt::expression crt::core::zip(const crt::expression& e)
{
    return e.zip();
}

crt::expression crt::core::map(const crt::expression& e)
{
    std::vector<crt::expression> result;

    for (const auto& argset : e.rest().zip())
    {
        result.push_back(e.first().call(argset));
    }
    return result;
}

crt::expression crt::core::first(const crt::expression& e)
{
    return e.first().first();
}

crt::expression crt::core::second(const crt::expression& e)
{
    return e.first().second();
}

crt::expression crt::core::rest(const crt::expression& e)
{
    return e.first().rest();
}

crt::expression crt::core::last(const crt::expression& e)
{
    return e.first().last();
}

crt::expression crt::core::len(const crt::expression& e)
{
    return int(e.first().size());
}

crt::expression crt::core::sort(const crt::expression& e)
{
    return e.first().sort();
}

crt::expression crt::core::reverse(const crt::expression& e)
{
    auto arg = e.first();
    return crt::expression(arg.rbegin(), arg.rend());
}

crt::expression crt::core::nest(const crt::expression& e)
{
    return e.first().nest();
}

crt::expression crt::core::type(const crt::expression& e)
{
    return std::string(e.first().type_name());
}

crt::expression crt::core::eval(const crt::expression& e)
{
    return crt::parse(e.first());
}

crt::expression crt::core::unparse(const crt::expression& e)
{
    return e.first().unparse();
}




//=============================================================================
crt::expression crt::fromYamlFile (File source)
{
    return fromYamlNode (YAML::LoadFile (source.getFullPathName().toStdString()));
}

crt::expression crt::fromYamlNode (const YAML::Node &node)
{
    switch (node.Type())
    {
        case YAML::NodeType::value::Null: return crt::expression::none();
        case YAML::NodeType::value::Undefined: return crt::expression::none();
        case YAML::NodeType::value::Scalar: return fromYamlScalar (node.Scalar());
        case YAML::NodeType::value::Sequence:
        {
            std::vector<crt::expression> list;

            for (const auto& elem : node)
            {
                list.push_back (fromYamlNode (elem));
            }
            return list;
        }
        case YAML::NodeType::value::Map:
        {
            std::vector<crt::expression> list;
            std::vector<crt::expression> dict;

            for (const auto& item : node)
            {
                if (item.first.IsNull())
                    list.push_back (fromYamlNode (item.second));
                else
                    dict.push_back (fromYamlNode (item.second).keyed (item.first.Scalar()));
            }
            return crt::expression(list).concat (dict);
        }
    }
}

crt::expression crt::fromYamlScalar (const std::string& source)
{
    auto value = JSON::fromString (source);

    if (source.empty())
        return crt::expression::none();
    if (source[0] == '$')
        return crt::expression::symbol (source.substr(1));
    if (source[0] == '(')
        return crt::parse (source);
    if (value.isInt())
        return int (value);
    if (value.isDouble())
        return double (value);

    return source;
}




//=============================================================================
const char* crt::type_info<Colour>::name()
{
    return "Color";
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
    return "FileFilter";
}

crt::expression crt::type_info<ConfigurableFileFilter>::to_table (const ConfigurableFileFilter& filter)
{
    return crt::parse (filter.getSourceString().toStdString());
}

ConfigurableFileFilter crt::type_info<ConfigurableFileFilter>::from_expr (const expression& e)
{
    if (e.has_type (crt::data_type::data))
    {
        return e.check_data<ConfigurableFileFilter>();
    }

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




//=============================================================================
const char* crt::type_info<Grid>::name()
{
    return "Grid";
}

crt::expression crt::type_info<Grid>::to_table (const Grid&)
{
    return {};
}

Grid crt::type_info<Grid>::from_expr (const expression& e)
{
    if (e.has_type (crt::data_type::data))
    {
        return e.check_data<Grid>();
    }

    auto getTrackInfo = [] (auto e) -> Grid::TrackInfo
    {
        if (e.has_type (crt::data_type::str))
        {
            String str = e.get_str();

            if (str.endsWithIgnoreCase ("px"))
                return Grid::Px ((long double) str
                                 .upToLastOccurrenceOf ("px", false, true)
                                 .getDoubleValue());

            else if (str.endsWithIgnoreCase ("fr"))
                return Grid::Fr ((unsigned long long) str
                                 .upToLastOccurrenceOf ("fr", false, true)
                                 .getIntValue());
        }
        return Grid::Fr ((unsigned long long) int(e));
    };

    Grid grid;

    if (e.attr ("rows").empty())
        grid.templateRows = { 1_fr };
    else
        for (const auto& elem : e.attr ("rows"))
            grid.templateRows.add (getTrackInfo (elem));

    if (e.attr ("cols").empty())
        grid.templateColumns = { 1_fr };
    else
        for (const auto& elem : e.attr ("cols"))
            grid.templateColumns.add (getTrackInfo (elem));

    return grid;
}
