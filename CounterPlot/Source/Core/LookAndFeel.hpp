#pragma once
#include "JuceHeader.h"




//=============================================================================
class LookAndFeelHelpers
{
public:
    enum ColourIds
    {
        propertyViewBackground      = 0x0771601,
        propertyViewSelectedItem    = 0x0771602,
        propertyViewText0           = 0x0771603,
        propertyViewText1           = 0x0771604,
        propertyViewText2           = 0x0771605,
        propertyViewText3           = 0x0771606,
        propertyViewText4           = 0x0771607,

        directoryTreeFile           = 0x0771608,
        directoryTreeDirectory      = 0x0771609,
        directoryTreeSymbolicLink   = 0x0771610,
        directoryTreeBackground     = 0x0771611,
        directoryTreeSelectedItem   = 0x0771612,

        statusBarBackground         = 0x0771613,
        statusBarText               = 0x0771614,
        statusBarErrorText          = 0x0771615,

        environmentViewBackground   = 0x0771616,
        environmentViewSelectedItem = 0x0771617,
        environmentViewText1        = 0x0771618,
        environmentViewText2        = 0x0771619,
    };

    enum class TextColourScheme
    {
        pastels1,
        pastels2,
    };

    enum class BackgroundScheme
    {
        dark,
    };

    static void setLookAndFeelDefaults (LookAndFeel&, TextColourScheme scheme);
    static void setLookAndFeelDefaults (LookAndFeel&, BackgroundScheme scheme);
    static Colour findColourForPropertyText (const Component&, int index);
    static int colourIdFromString (const String&);
};
