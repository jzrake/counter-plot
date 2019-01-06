#include "LookAndFeel.hpp"




//=============================================================================
void LookAndFeelHelpers::setLookAndFeelDefaults (LookAndFeel& laf, TextColourScheme scheme)
{
    switch (scheme)
    {
        case TextColourScheme::pastels1:
            laf.setColour (propertyViewText0, Colour::fromRGB (255, 198, 252));
            laf.setColour (propertyViewText1, Colour::fromRGB (229, 181, 255));
            laf.setColour (propertyViewText2, Colour::fromRGB (255, 242, 214));
            laf.setColour (propertyViewText3, Colour::fromRGB (254, 240, 255));
            laf.setColour (propertyViewText4, Colour::fromRGB (255, 255, 255));
            laf.setColour (directoryTreeFile,         Colour::fromRGB (255, 198, 252));
            laf.setColour (directoryTreeDirectory,    Colour::fromRGB (229, 181, 255));
            laf.setColour (directoryTreeSymbolicLink, Colour::fromRGB (255, 242, 214));
            break;
        case TextColourScheme::pastels2:
            laf.setColour (propertyViewText0, Colour::fromRGB (162, 230, 244));
            laf.setColour (propertyViewText1, Colour::fromRGB (255, 203, 237));
            laf.setColour (propertyViewText2, Colour::fromRGB (243, 181, 255));
            laf.setColour (propertyViewText3, Colour::fromRGB (167, 234, 225));
            laf.setColour (propertyViewText4, Colour::fromRGB (251, 247, 156));
            laf.setColour (directoryTreeFile,         Colour::fromRGB (162, 230, 244));
            laf.setColour (directoryTreeDirectory,    Colour::fromRGB (255, 203, 237));
            laf.setColour (directoryTreeSymbolicLink, Colour::fromRGB (243, 181, 255));
            break;
    }
}

void LookAndFeelHelpers::setLookAndFeelDefaults (LookAndFeel& laf, BackgroundScheme scheme)
{
    switch (scheme)
    {
        case BackgroundScheme::dark:
            laf.setColour (directoryTreeBackground, Colours::darkgrey.darker (0.22f));
            laf.setColour (directoryTreeSelectedItem, Colours::darkgrey.darker (0.33f));
            laf.setColour (propertyViewBackground, Colours::darkgrey.darker (0.06));
            laf.setColour (propertyViewSelectedItem, Colours::darkgrey.darker (0.12f));
            break;
    }
}

Colour LookAndFeelHelpers::findColourForPropertyText (const Component& target, int index)
{
    switch (index % 5)
    {
        case 0: return target.findColour (propertyViewText0);
        case 1: return target.findColour (propertyViewText1);
        case 2: return target.findColour (propertyViewText2);
        case 3: return target.findColour (propertyViewText3);
        case 4: return target.findColour (propertyViewText4);
    }
    return Colours::black;
}

Colour LookAndFeelHelpers::colourFromVariant (const var& value)
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
        return Colour::fromString (value.toString());
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

int LookAndFeelHelpers::colourIdFromString (const String& name)
{
    if (name == "directory_tree.background")    return directoryTreeBackground;
    if (name == "directory_tree.selected_item") return directoryTreeSelectedItem;
    if (name == "directory_tree.file")          return directoryTreeFile;
    if (name == "directory_tree.directory")     return directoryTreeDirectory;
    if (name == "directory_tree.symbolic_link") return directoryTreeSymbolicLink;
    if (name == "property_view.background")     return propertyViewBackground;
    if (name == "property_view.selected_item")  return propertyViewSelectedItem;
    if (name == "property_view.text0")          return propertyViewText0;
    if (name == "property_view.text1")          return propertyViewText1;
    if (name == "property_view.text2")          return propertyViewText2;
    if (name == "property_view.text3")          return propertyViewText3;
    if (name == "property_view.text4")          return propertyViewText4;
    return -1;
}
