#pragma once
#include "JuceHeader.h"
#include "ConfigurableFileFilter.hpp"
namespace YAML { class Node; }




//=============================================================================
struct DivModel
{
    float borderWidth;
    float cornerRadius = 0.f;
    Colour background;
    Colour border;
    String onMove;
    String onDown;

    //Grid grid;
    //FlexBox flex;
    crt::expression content;
    crt::expression layout;
};




//=============================================================================
struct TextModel
{
    String content;
    Colour color;
    Font font;
    Justification justification = Justification::centred;
};




//=============================================================================
namespace crt {




//=============================================================================
namespace core {

    expression table   (const expression& e);
    expression list    (const expression& e);
    expression dict    (const expression& e);
    expression switch_ (const expression& e);
    expression item    (const expression& e);
    expression attr    (const expression& e);
    expression range   (const expression& e);
    expression slice   (const expression& e);
    expression concat  (const expression& e);
    expression join    (const expression& e);
    expression apply   (const expression& e);
    expression zip     (const expression& e);
    expression map     (const expression& e);
    expression nest    (const expression& e);
    expression first   (const expression& e);
    expression second  (const expression& e);
    expression rest    (const expression& e);
    expression last    (const expression& e);
    expression len     (const expression& e);
    expression sort    (const expression& e);
    expression reverse (const expression& e);
    expression type    (const expression& e);
    expression eval    (const expression& e);
    expression unparse (const expression& e);

    void import(crt::kernel& k);
}



    /**
     * Return an expression from a YAML file.
     */
    crt::expression fromYamlFile (File source);


    /**
     * Return an expression from a YAML node.
     */
    crt::expression fromYamlNode (const YAML::Node& node);


    /**
     * Looks at the string content of a YAML scalar node, and turns it into an
     * expression according to these rules:
     *
     * - If the string starts with $, it's interpreted as a symbol.
     * - If it starts with an open parentheses, it's parsed as a table.
     * - Otherwise it's parsed as JSON and converted to int, double, or string
     */
    crt::expression fromYamlScalar (const std::string& source);


    //=========================================================================
    template<>
    struct crt::type_info<Colour>
    {
        static const char* name();
        static expression to_table (const Colour&);
        static Colour from_expr (const expression&);
    };


    //=========================================================================
    template<>
    struct crt::type_info<ConfigurableFileFilter>
    {
        static const char* name();
        static expression to_table (const ConfigurableFileFilter&);
        static ConfigurableFileFilter from_expr (const expression&);
    };


    //=========================================================================
    template<>
    struct crt::type_info<FlexBox>
    {
        static const char* name();
        static expression to_table (const FlexBox&);
        static FlexBox from_expr (const expression&);
    };
    

    //=========================================================================
    template<>
    struct crt::type_info<Grid>
    {
        static const char* name();
        static expression to_table (const Grid&);
        static Grid from_expr (const expression&);
    };


    //=========================================================================
    template<>
    struct crt::type_info<DivModel>
    {
        static const char* name();
        static expression to_table (const DivModel&);
        static DivModel from_expr (const expression&);
    };


    //=========================================================================
    template<>
    struct crt::type_info<TextModel>
    {
        static const char* name();
        static expression to_table (const TextModel&);
        static TextModel from_expr (const expression&);
    };
}
