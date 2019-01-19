#include "JuceHeader.h"

namespace YAML { class Node; }




//=============================================================================
class DataHelpers
{
public:
    static var varFromExpression (const crt::expression& expr);
    static var varFromYamlScalar (const YAML::Node& scalar);
    static var varFromYamlNode (const YAML::Node& node);
    static var varFromBorderSize (const BorderSize<int>&);
    static var varFromStringPairArray (const StringPairArray& value);

    static crt::expression expressionFromVar (const var& value);
    static YAML::Node yamlNodeFromVar (const var& value);
    static Array<Grid::TrackInfo> gridTrackInfoArrayFromVar (const var& value);
    static Colour colourFromVar (const var&);
    static StringArray stringArrayFromVar (const var&);
    static StringPairArray stringPairArrayFromVar (const var&);

    static CriticalSection& getCriticalSectionForHDF5();
};
