#include "LookAndFeel.hpp"




//=============================================================================
void LookAndFeelHelpers::setLookAndFeelDefaults (LookAndFeel& laf, TextColourScheme scheme)
{
    switch (scheme)
    {
        case TextColourScheme::pastels1:
            laf.setColour (textColour0, Colour::fromRGB (255, 198, 252));
            laf.setColour (textColour1, Colour::fromRGB (229, 181, 255));
            laf.setColour (textColour2, Colour::fromRGB (255, 242, 214));
            laf.setColour (textColour3, Colour::fromRGB (254, 240, 255));
            laf.setColour (textColour4, Colour::fromRGB (255, 255, 255));
            break;
        case TextColourScheme::pastels2:
            laf.setColour (textColour0, Colour::fromRGB (162, 230, 244));
            laf.setColour (textColour1, Colour::fromRGB (255, 203, 237));
            laf.setColour (textColour2, Colour::fromRGB (243, 181, 255));
            laf.setColour (textColour3, Colour::fromRGB (167, 234, 225));
            laf.setColour (textColour4, Colour::fromRGB (251, 247, 156));
            break;
    }
}

Colour LookAndFeelHelpers::findTextColour (const Component& target, int index)
{
    switch (index % 5)
    {
        case 0: return target.findColour (textColour0);
        case 1: return target.findColour (textColour1);
        case 2: return target.findColour (textColour2);
        case 3: return target.findColour (textColour3);
        case 4: return target.findColour (textColour4);
    }
    jassertfalse;
    return Colours::black;
}
