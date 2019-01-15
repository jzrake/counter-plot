#include "JuceHeader.h"

namespace YAML { class Node; }




//=============================================================================
class DataHelpers
{
public:
//    static crt::expression expressionFromYamlNode (const YAML::Node& node);
//    static crt::expression expressionFromYamlScalar (const YAML::Node& node);
    static crt::expression expressionFromVar (const var& value);

    static var varFromYamlScalar (const YAML::Node& scalar);
    static var varFromYamlNode (const YAML::Node& node);
    static var varFromBorderSize (const BorderSize<int>&);

    static YAML::Node yamlNodeFromVar (const var& value);
    static Array<Grid::TrackInfo> gridTrackInfoArrayFromVar (const var& value);
    static Colour colourFromVar (const var&);
};
