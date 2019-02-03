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
    k.define("switch",    switch_);
    k.define("table",     table);
    k.define("type",      type);
    k.define("unparse",   unparse);
    k.define("zip",       zip);

    k.define("div",       crt::init<DivModel>());
    k.define("text",      crt::init<TextModel>());
    k.define("flex",      crt::init<FlexBox>());
    k.define("grid",      crt::init<Grid>());
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

    if (ind.has_type(crt::data_type::i32))
    {
        return arg.item(int(ind));
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

crt::expression crt::core::switch_(const crt::expression& e)
{
    return e.first() ? e.second() : e.third();
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

crt::expression crt::fromYamlNode (const YAML::Node &value)
{
    std::vector<crt::expression> result;

    switch (value.Type())
    {
        case YAML::NodeType::value::Null: return crt::expression::none();
        case YAML::NodeType::value::Undefined: return crt::expression::none();
        case YAML::NodeType::value::Scalar: return fromYamlScalar (value.Scalar());
        case YAML::NodeType::value::Sequence:
        {
            for (const auto& elem : value)
                result.push_back (fromYamlNode (elem));
            break;
        }
        case YAML::NodeType::value::Map:
        {
            for (const auto& item : value)
                result.push_back (fromYamlNode (item.second).keyed (item.first.Scalar()));
            break;
        }
    }
    return result;
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
const char* crt::type_info<FlexBox>::name()
{
    return "FlexBox";
}

crt::expression crt::type_info<FlexBox>::to_table (const FlexBox&)
{
    return {};
}

FlexBox crt::type_info<FlexBox>::from_expr (const expression& e)
{
    if (e.has_type (crt::data_type::data))
    {
        return e.check_data<FlexBox>();
    }

    auto directionValues = std::unordered_map<std::string, FlexBox::Direction>
    {
        {"row",            FlexBox::Direction::row},
        {"row-reverse",    FlexBox::Direction::rowReverse},
        {"column",         FlexBox::Direction::column},
        {"column-reverse", FlexBox::Direction::columnReverse},
    };

    auto wrapValues = std::unordered_map<std::string, FlexBox::Wrap>
    {
        {"no-wrap",        FlexBox::Wrap::noWrap},
        {"wrap",           FlexBox::Wrap::wrap},
        {"wrap-reverse",   FlexBox::Wrap::wrapReverse},
    };

    auto alignContentValues = std::unordered_map<std::string, FlexBox::AlignContent>
    {
        {"stretch",        FlexBox::AlignContent::stretch},
        {"flex-start",     FlexBox::AlignContent::flexStart},
        {"flex-end",       FlexBox::AlignContent::flexEnd},
        {"center",         FlexBox::AlignContent::center},
        {"space-between",  FlexBox::AlignContent::spaceBetween},
        {"space-around",   FlexBox::AlignContent::spaceAround},
    };

    auto alignItemsValues = std::unordered_map<std::string, FlexBox::AlignItems>
    {
        {"stretch",        FlexBox::AlignItems::stretch},
        {"flex-start",     FlexBox::AlignItems::flexStart},
        {"flex-end",       FlexBox::AlignItems::flexEnd},
        {"center",         FlexBox::AlignItems::center},
    };

    auto justifyContentValues = std::unordered_map<std::string, FlexBox::JustifyContent>
    {
        {"flex-start",     FlexBox::JustifyContent::flexStart},
        {"flex-end",       FlexBox::JustifyContent::flexEnd},
        {"center",         FlexBox::JustifyContent::center},
        {"space-between",  FlexBox::JustifyContent::spaceBetween},
        {"space-around",   FlexBox::JustifyContent::spaceAround},
    };

    FlexBox flex;
    flex.flexDirection  = directionValues     [e.attr ("flex-direction")];
    flex.flexWrap       = wrapValues          [e.attr ("flex-wrap")];
    flex.alignContent   = alignContentValues  [e.attr ("align-content")];
    flex.alignItems     = alignItemsValues    [e.attr ("align-items")];
    flex.justifyContent = justifyContentValues[e.attr ("justify-content")];
    return flex;
}




//=============================================================================
const char* crt::type_info<FlexItem>::name()
{
    return "FlexItem";
}

crt::expression crt::type_info<FlexItem>::to_table (const FlexItem&)
{
    return {};
}

FlexItem crt::type_info<FlexItem>::from_expr (const expression& e)
{
    if (e.has_type (crt::data_type::data))
    {
        return e.check_data<FlexItem>();
    }

    auto alignSelfValues = std::unordered_map<std::string, FlexItem::AlignSelf>
    {
        {"auto-align", FlexItem::AlignSelf::autoAlign},
        {"flex-start", FlexItem::AlignSelf::flexStart},
        {"flex-end",   FlexItem::AlignSelf::flexEnd},
        {"center",     FlexItem::AlignSelf::center},
        {"stretch",    FlexItem::AlignSelf::stretch},
    };

    FlexItem item;
    item.alignSelf  = alignSelfValues[e.attr ("align-self")];
    item.width      = e.attr ("width")      .otherwise (item.width);
    item.height     = e.attr ("height")     .otherwise (item.height);
    item.minWidth   = e.attr ("min-width")  .otherwise (item.minWidth);
    item.maxWidth   = e.attr ("max-width")  .otherwise (item.maxWidth);
    item.minHeight  = e.attr ("min-height") .otherwise (item.minHeight);
    item.maxHeight  = e.attr ("max-height") .otherwise (item.maxHeight);
    item.flexGrow   = e.attr ("flex-grow")  .otherwise (item.flexGrow);
    item.flexShrink = e.attr ("flex-shrink").otherwise (item.flexShrink);
    item.flexBasis  = e.attr ("flex-basis") .otherwise (item.flexBasis);
    return item;
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




//=============================================================================
const char* crt::type_info<GridItem>::name()
{
    return "GridItem";
}

crt::expression crt::type_info<GridItem>::to_table (const GridItem&)
{
    return {};
}

GridItem crt::type_info<GridItem>::from_expr (const expression& e)
{
    if (e.has_type (crt::data_type::data))
    {
        return e.check_data<GridItem>();
    }
    return GridItem();
}




//=============================================================================
const char* crt::type_info<DivModel>::name()
{
    return "Div";
}

crt::expression crt::type_info<DivModel>::as_type (const DivModel& div, const char* type_name)
{
    if (type_name == crt::type_info<FlexItem>::name())
    {
        return crt::make_data (div.flexItem);
    }
    if (type_name == crt::type_info<GridItem>::name())
    {
        return crt::make_data (div.gridItem);
    }
    return {};
}

crt::expression crt::type_info<DivModel>::to_table (const DivModel& div)
{
    return crt::expression({
        crt::expression::from (div.background)     .keyed ("background"),
        crt::expression::from (div.border)         .keyed ("border"),
        crt::expression (div.borderWidth)          .keyed ("border-width"),
        crt::expression (div.cornerRadius)         .keyed ("corner-radius"),
        crt::expression (div.onDown.toStdString()) .keyed ("on-down"),
        crt::expression (div.onMove.toStdString()) .keyed ("on-move"),
    });
}

DivModel crt::type_info<DivModel>::from_expr (const crt::expression& e)
{
    if (e.has_type (crt::data_type::data))
    {
        return e.check_data<DivModel>();
    }

    DivModel div;
    div.background   = e.attr ("background")   .to<Colour>();
    div.border       = e.attr ("border")       .to<Colour>();
    div.borderWidth  = e.attr ("border-width") .as_f64();
    div.cornerRadius = e.attr ("corner-radius").as_f64();
    div.onDown       = e.attr ("on-down").as_str();
    div.onMove       = e.attr ("on-move").as_str();
    div.content      = e.attr ("content");
    div.layout       = e.attr ("layout");
    div.gridItem     = e.to<GridItem>();
    div.flexItem     = e.to<FlexItem>();
    return div;
}




//=============================================================================
const char* crt::type_info<TextModel>::name()
{
    return "Text";
}

crt::expression crt::type_info<TextModel>::to_table (const TextModel& div)
{
    return {};
}

TextModel crt::type_info<TextModel>::from_expr (const crt::expression& e)
{
    if (e.has_type (crt::data_type::data))
    {
        return e.check_data<TextModel>();
    }

    auto justificationValues = std::unordered_map<std::string, int>
    {
        {"left",                   Justification::left},
        {"right",                  Justification::right},
        {"horizontally-centered",  Justification::horizontallyCentred},
        {"top",                    Justification::top},
        {"bottom",                 Justification::bottom},
        {"vertically-centered",    Justification::verticallyCentred},
        {"horizontally-justified", Justification::horizontallyJustified},
        {"centered",               Justification::centred},
        {"centered-right",         Justification::centredRight},
        {"centered-top",           Justification::centredTop},
        {"centered-bottom",        Justification::centredBottom},
        {"top-left",               Justification::topLeft},
        {"top-right",              Justification::topRight},
        {"bottom-left",            Justification::bottomLeft},
        {"bottom-right",           Justification::bottomRight},
    };

    TextModel text;
    text.content       = e.attr ("content").otherwise (e.item(0)).as_str();
    text.color         = e.attr ("color").to<Colour>();
    text.font          = Font::fromString (e.attr ("font").as_str());
    text.justification = justificationValues[e.attr ("justification").as_str()];
    return text;
}
