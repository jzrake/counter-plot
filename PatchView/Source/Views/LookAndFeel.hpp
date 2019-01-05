#pragma once
#include "JuceHeader.h"




//=============================================================================
class LookAndFeelHelpers
{
public:
    enum ColourIds
    {
        textColour0 = 0x0771600,
        textColour1 = 0x0771601,
        textColour2 = 0x0771602,
        textColour3 = 0x0771603,
        textColour4 = 0x0771604,
    };

    enum class TextColourScheme
    {
        pastels1,
        pastels2,
    };

    static void setLookAndFeelDefaults (LookAndFeel&, TextColourScheme scheme);
    static Colour findTextColour (const Component&, int index);
};
