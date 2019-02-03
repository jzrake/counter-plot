#pragma once
#include "JuceHeader.h"
#include "../Core/Program.hpp"




namespace cp {
    class Div;
    class Text;
};




//=============================================================================
class cp::Div : public cp::View
{
public:

    void load (const crt::expression& e) override
    {
        if (e != expr)
        {
            model = (expr = e).to<DivModel>();
            repaint();
        }
    }

    void paint (Graphics& g) override
    {
        auto area = getLocalBounds().toFloat();
        g.setColour (model.background);

        if (model.cornerRadius > 0.f)
        {
            g.fillRoundedRectangle (area.reduced (model.borderWidth / 2), model.cornerRadius);
        }
        else
        {
            g.fillRect (area);
        }

        if (! model.border.isTransparent() && model.borderWidth > 0.f)
        {
            g.setColour (model.border);

            if (model.cornerRadius > 0.f)
            {
                g.drawRoundedRectangle (area.reduced (model.borderWidth / 2), model.cornerRadius, model.borderWidth);
            }
            else
            {
                g.drawRect (area, model.borderWidth);
            }
        }
    }

    void mouseMove (const MouseEvent& e) override
    {
        if (model.onMove.isNotEmpty())
        {
            sink (crt::parse (model.onMove.toRawUTF8())
                  .replace ("x", e.position.x)
                  .replace ("y", e.position.y));
        }
    }

    void mouseDown (const MouseEvent& e) override
    {
        if (model.onDown.isNotEmpty())
        {
            sink (crt::parse (model.onDown.toRawUTF8())
                  .replace ("x", e.position.x)
                  .replace ("y", e.position.y));
        }
    }

private:
    DivModel model;
    crt::expression expr;
};




//=============================================================================
class cp::Text : public cp::View
{
public:

    void load (const crt::expression& e) override
    {
        if (e != expr)
        {
            model = (expr = e).to<TextModel>();
            repaint();
        }
    }

    void paint (Graphics& g) override
    {
        g.setFont (model.font);
        g.setColour (model.color);
        g.drawText (model.content, getLocalBounds(), model.justification);
    }

private:
    TextModel model;
    crt::expression expr;
};
