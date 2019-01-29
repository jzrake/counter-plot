#include "DataHelpers.hpp"
#include "yaml-cpp/yaml.h"




//=============================================================================
void DataHelpers::updateDict (var& dictToUpdate, const var& other)
{
    if (auto target = dictToUpdate.getDynamicObject())
    {
        if (auto source = other.getDynamicObject())
        {
            for (const auto& item : source->getProperties())
            {
                target->setProperty (item.name, item.value);
            }
        }
    }
}




//=============================================================================
var DataHelpers::makeDictFromList (const var& list, StringRef basename)
{
    if (auto obj = list.getDynamicObject())
    {
        return obj;
    }

    auto obj = std::make_unique<DynamicObject>();
    int n = 0;

    if (auto arr = list.getArray())
    {
        for (const auto& element : *arr)
        {
            obj->setProperty (basename + String(n), element);
            ++n;
        }
    }
    return obj.release();
}




//=============================================================================
var DataHelpers::varFromExpression (const crt::expression& expr)
{
    // This method is not tested yet... I think that turning an expression back into
    // a var might need input from the kernel... in particular in determining whether
    // a rule is concrete or abstract - if abstract (that is, the kernel has an
    // expression for it) then it should be decompiled and stored as a string. In other
    // words this function probably needs to be written over the whole kernel.
    jassertfalse;

    switch (expr.dtype())
    {
        case crt::data_type::composite:
        {
            var obj = new DynamicObject;

            for (const auto& part : expr)
            {
                // There's no way to turn an expression with
                // unnamed parts into a var...
                jassert (! part.key().empty());
                obj.getDynamicObject()->setProperty (Identifier (part.key()), varFromExpression (part));
            }
            return obj;
        }
        case crt::data_type::f64: return expr.get_f64();
        case crt::data_type::i32: return expr.get_i32();
        case crt::data_type::none: return var();
        case crt::data_type::str: return String (expr.repr());
        case crt::data_type::symbol: return "$" + String (expr.repr());
    }
}

var DataHelpers::varFromBorderSize (const BorderSize<int>& border)
{
    auto obj = std::make_unique<DynamicObject>();
    obj->setProperty ("top", border.getTop());
    obj->setProperty ("bottom", border.getBottom());
    obj->setProperty ("left", border.getLeft());
    obj->setProperty ("right", border.getRight());
    return obj.release();
}

var DataHelpers::varFromYamlScalar (const YAML::Node& scalar)
{
    assert (scalar.IsScalar());
    auto value = String (scalar.Scalar());

    if (! JSON::fromString (value).isVoid())
    {
        return JSON::fromString (value);
    }
    return value;
}

var DataHelpers::varFromStringPairArray (const StringPairArray &value)
{
    auto obj = std::make_unique<DynamicObject>();

    for (const auto& key : value.getAllKeys())
    {
        obj->setProperty (key, value.getValue (key, ""));
    }
    return obj.release();
}

var DataHelpers::varFromStringMap (const std::map<std::string, std::string>& value)
{
    auto obj = std::make_unique<DynamicObject>();

    for (const auto& item : value)
    {
        obj->setProperty (item.first.data(), item.second.data());
    }
    return obj.release();
}




//=============================================================================
YAML::Node DataHelpers::yamlNodeFromVar (const var& value)
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
            node.push_back (yamlNodeFromVar (element));
        }
        return node;
    }
    if (auto obj = value.getDynamicObject())
    {
        YAML::Node node;

        for (const auto& item : obj->getProperties())
        {
            node[item.name.toString().toStdString()] = yamlNodeFromVar (item.value);
        }
        return node;
    }
    return YAML::Node();
}

crt::expression DataHelpers::expressionFromVar (const var& value)
{
    if (value.isVoid()) return {};
    if (value.isBool()) return int (value);
    if (value.isInt()) return int (value);
    if (value.isDouble()) return double (value);
    if (value.isString())
    {
        auto str = value.toString();

        if (str.startsWithChar ('(') && str.endsWithChar (')'))
            return crt::parser::parse (str.getCharPointer());

        else if (str.startsWithChar ('$'))
            return crt::expression::symbol (str.substring (1).toStdString());

        return str.toStdString();
    }
    if (auto elements = value.getArray())
    {
        std::vector<crt::expression> expr = { crt::expression::symbol ("list") };

        for (const auto& element : *elements)
            expr.push_back (expressionFromVar (element));
        return expr;
    }
    if (auto obj = value.getDynamicObject())
    {
        std::vector<crt::expression> expr = { crt::expression::symbol ("dict") };

        for (const auto& item : obj->getProperties())
            expr.push_back (expressionFromVar (item.value).keyed (item.name.toString().toStdString()));
        return expr;
    }
    return {};
}

var DataHelpers::varFromYamlNode (const YAML::Node& node)
{
    switch (node.Type())
    {
        case YAML::NodeType::value::Null: return var();
        case YAML::NodeType::value::Undefined: return var::undefined();
        case YAML::NodeType::value::Scalar: return varFromYamlScalar (node);
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
            if (item.isString())
            {
                auto str = item.toString();

                if (str.endsWithIgnoreCase ("px"))
                {
                    info.add (Grid::Px ((long double) str.upToLastOccurrenceOf ("px", false, true).getDoubleValue()));
                }
                else if (str.endsWithIgnoreCase ("fr"))
                {
                    info.add (Grid::Fr ((unsigned long long) str.upToLastOccurrenceOf ("fr", false, true).getIntValue()));
                }
            }
            else
            {
                info.add (Grid::Fr (int (item)));
            }
        }
    }
    else if (value.isVoid())
    {
        info.add (1_fr);
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
        auto str = value.toString();

        if (str.startsWithChar ('#'))
            return Colour::fromString (value.toString());

        return Colours::findColourForName (str, Colours::black);
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

StringArray DataHelpers::stringArrayFromVar (const var& value)
{
    StringArray result;

    if (auto arr = value.getArray())
    {
        for (auto item : *arr)
        {
            result.add (item.toString());
        }
    }
    return result;
}

BorderSize<int> DataHelpers::borderSizeFromVar (const var& value)
{
    BorderSize<int> result;

    if (auto obj = value.getDynamicObject())
    {
        for (const auto& item : obj->getProperties())
        {
            if (item.name == Identifier ("top")) result.setTop (item.value);
            if (item.name == Identifier ("bottom")) result.setBottom (item.value);
            if (item.name == Identifier ("left")) result.setLeft (item.value);
            if (item.name == Identifier ("right")) result.setRight (item.value);
        }
    }
    else if (value.size() == 4)
    {
        result.setTop    (value[0]); // This ordering is the CSS convention
        result.setRight  (value[1]);
        result.setBottom (value[2]);
        result.setLeft   (value[3]);
    }
    return result;
}

StringPairArray DataHelpers::stringPairArrayFromVar (const var& value)
{
    StringPairArray result;

    if (auto obj = value.getDynamicObject())
    {
        for (auto item : obj->getProperties())
        {
            result.set (item.name.toString(), item.value.toString());
        }
    }
    return result;
}

std::map<std::string, std::string> DataHelpers::stringMapFromVar (const var& value)
{
    std::map<std::string, std::string> result;

    if (auto obj = value.getDynamicObject())
    {
        for (const auto& item : obj->getProperties())
        {
            result[item.name.toString().toStdString()] = item.value.toString().toStdString();
        }
    }
    return result;
}

nd::array<double, 1> DataHelpers::ndarrayDouble1FromVar(const var &value)
{
    if (auto arr = value.getArray())
    {
        nd::array<double, 1> result (arr->size());

        for (int n = 0; n < result.size(); ++n)
        {
            result(n) = arr->getUnchecked(n);
        }
        return result;
    }
    return var();
}




//=============================================================================
CriticalSection& DataHelpers::getCriticalSectionForHDF5()
{
    static CriticalSection section;
    return section;
}
