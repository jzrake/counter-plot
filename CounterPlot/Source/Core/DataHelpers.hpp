#include "JuceHeader.h"

namespace YAML { class Node; }




//=============================================================================
class DataHelpers
{
public:
    static YAML::Node nodeFromVar (const var& value);
    static var varFromYamlScalar (const YAML::Node& scalar);
    static var varFromYamlNode (const YAML::Node& node);
    static Array<Grid::TrackInfo> gridTrackInfoArrayFromVar (const var& value);
    static Colour colourFromVar (const var&);
};
