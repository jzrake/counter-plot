#include "LookAndFeel.hpp"




//=============================================================================
void AppLookAndFeel::setLookAndFeelDefaults (LookAndFeel& laf, TextColourScheme scheme)
{
    switch (scheme)
    {
        case TextColourScheme::pastels1:
            laf.setColour (propertyViewText0, Colour::fromRGB (255, 198, 252));
            laf.setColour (propertyViewText1, Colour::fromRGB (229, 181, 255));
            laf.setColour (propertyViewText2, Colour::fromRGB (255, 242, 214));
            laf.setColour (propertyViewText3, Colour::fromRGB (254, 240, 255));
            laf.setColour (propertyViewText4, Colour::fromRGB (255, 255, 255));
            laf.setColour (directoryTreeFile,         Colours::grey.brighter());
            laf.setColour (directoryTreeDirectory,    Colours::grey.brighter());
            laf.setColour (directoryTreeSymbolicLink, Colours::grey.brighter());
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

void AppLookAndFeel::setLookAndFeelDefaults (LookAndFeel& laf, BackgroundScheme scheme)
{

    // These are the LookAndFeel_v4 DarkColourScheme colours:
    // ------------------------------------------------------
    // const static auto windowBackground  = 0xff323e44;
       const static auto widgetBackground  = 0xff263238;
    // const static auto menuBackground    = 0xff323e44;
    // const static auto outline           = 0xff8e989b;
       const static auto defaultText       = 0xffffffff;
    // const static auto defaultFill       = 0xff42a2c8;
    // const static auto highlightedText   = 0xffffffff;
    // const static auto highlightedFill   = 0xff181f22;
    // const static auto menuText          = 0xffffffff;
    // ------------------------------------------------------

    switch (scheme)
    {
        case BackgroundScheme::dark:
            laf.setColour (directoryTreeBackground, Colours::darkgrey.darker (0.22f));
            laf.setColour (directoryTreeSelectedItem, Colours::darkgrey.darker (0.33f));
            laf.setColour (propertyViewBackground, Colours::darkgrey.darker (0.06));
            laf.setColour (propertyViewSelectedItem, Colours::darkgrey.darker (0.12f));
            laf.setColour (statusBarBackground, Colours::darkgrey.darker (0.44f));
            laf.setColour (statusBarText, Colours::darkgrey.brighter (0.44f));
            laf.setColour (statusBarErrorText, Colours::darkorange);
            laf.setColour (environmentViewBackground, Colour (widgetBackground).withAlpha (0.95f));
            laf.setColour (environmentViewSelectedItem, Colour (widgetBackground).darker());
            laf.setColour (environmentViewText1, Colour (defaultText));
            laf.setColour (environmentViewText2, Colour (defaultText).darker());
            break;
    }
}

Colour AppLookAndFeel::findColourForPropertyText (const Component& target, int index)
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

int AppLookAndFeel::colourIdFromString (const String& name)
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
    if (name == "status_bar.background")        return statusBarBackground;
    return -1;
}




//=============================================================================
Font AppLookAndFeel::getDefaultFont() const
{
    return defaultFont;
}

void AppLookAndFeel::setDefaultFont (Font newDefaultFont)
{
    defaultFont = newDefaultFont;
}

void AppLookAndFeel::incrementFontSize (int amount)
{
    setDefaultFont (defaultFont.withHeight (defaultFont.getHeight() + amount));
}




//=============================================================================
void AppLookAndFeel::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area,
                                               Colour backgroundColour, bool isOpen, bool isMouseOver)
{
    Path p;
    p.addTriangle (0.0f, 0.0f, 1.0f, isOpen ? 0.0f : 0.5f, isOpen ? 0.5f : 0.0f, 1.0f);
    g.setColour (backgroundColour.contrasting().withAlpha (isMouseOver ? 0.4f : 0.2f));
    g.fillPath (p, p.getTransformToScaleToFit (area.reduced (6), true));
}

int AppLookAndFeel::getDefaultScrollbarWidth()
{
    return 6;
}

void AppLookAndFeel::drawTooltip (Graphics& g, const String& text, int width, int height)
{
    Rectangle<int> bounds (width, height);
    g.setColour (Colours::whitesmoke);
    g.fillRect (bounds);
    g.setColour (Colours::black);
    g.drawRect (bounds, 0.5);
    g.setFont (Font().withHeight (11));
    g.drawText (text, 4, 0, width, height, Justification::centredLeft);
}
