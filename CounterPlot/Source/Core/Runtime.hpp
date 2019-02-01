#pragma once
#include "JuceHeader.h"
#include "ConfigurableFileFilter.hpp"
namespace YAML { class Node; }




//=============================================================================
namespace crt {


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
}
