#include "Program.hpp"
#include "../Views/Division.hpp"




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




//=============================================================================
class cp::Div : public cp::View
{
public:

    void load (const crt::expression& e) override
    {
        if (getLastModel() != e)
        {
            model = e.to<DivModel>();
            content();
            layout();
            repaint();
        }
    }

    void paint (Graphics& g) override
    {
        auto area = getLocalBounds().toFloat();
        g.setColour (model.background);

        if (model.cornerRadius > 0.f)
        {
            g.fillRoundedRectangle (area.reduced (model.borderWidth / 2),
                                    model.cornerRadius);
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
                g.drawRoundedRectangle (area.reduced (model.borderWidth / 2),
                                        model.cornerRadius,
                                        model.borderWidth);
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

    void resized() override
    {
        layout();
    }

    void content (const crt::expression& e)
    {
        if (model.content != e)
        {
            model.content = e;
            content();
        }
    }

    void content()
    {
        // Step 1: ensure the same number of views as content items.
        if (views.size() > model.content.size())
        {
            views.removeLast (views.size() - int (model.content.size()));
        }
        while (views.size() < model.content.size())
        {
            views.add (new View);
        }

        // Step 2: ensure the views all have the correct types.
        int n = 0;

        for (const auto& item : model.content)
        {
            if (! views[n]->hasSameType (item))
            {
                views.set (n, factory (item.type_name()));
                views[n]->setActionSink (actionSink);
            }
            ++n;
        }

        // Step 3: pass values to views.
        n = 0;

        for (const auto& item : model.content)
        {
            views[n++]->load (item);
        }

        for (auto view : views)
        {
            addAndMakeVisible (view);
        }
        layout();
    }

    void layout (const crt::expression& e)
    {
        model.layout = e;
        layout();
    }

    void layout()
    {
        if (model.layout.has_type<Grid>())
        {
            auto grid = model.layout.to<Grid>();

            for (auto child : getChildren())
            {
                grid.items.add (child);
            }
            grid.performLayout (getLocalBounds());
        }
        else if (model.layout.has_type<FlexBox>())
        {
            FlexBox flex = model.layout.to<FlexBox>();
            int n = 0;

            for (const auto& viewModel : model.content)
            {
                auto item = crt::try_protocol<FlexItem> (viewModel);
                item.associatedComponent = getChildComponent(n);
                flex.items.add (item);
                ++n;
            }
            flex.performLayout (getLocalBounds());
        }
    }

    View* factory (const std::string& type_name)
    {
        if (type_name == crt::type_info<DivModel>::name())
            return new cp::Div;
        if (type_name == crt::type_info<TextModel>::name())
            return new cp::Text;
        return new cp::View;
    }

    DivModel model;
    OwnedArray<View> views;
};




//=============================================================================
void cp::View::setActionSink (ActionSink* sinkToUse)
{
    actionSink = sinkToUse;
}

void cp::View::sink (const crt::expression& action)
{
    if (actionSink)
        actionSink->dispatch (action);
}

bool cp::View::hasSameType (const crt::expression& e) const
{
    return lastModel.type_name() == e.type_name();
}

const crt::expression& cp::View::getLastModel() const
{
    return lastModel;
}




//=============================================================================
cp::Program::Program()
{
    root = std::make_unique<Div>();
    root->setActionSink (this);
    crt::core::import (kernel);
}

cp::Program::~Program()
{
}

void cp::Program::loadCommandsFromFile (File file)
{
    for (const auto& expr : crt::fromYamlFile (file))
    {
        kernel.insert (expr);
    }
    resolve();
}

Component& cp::Program::getRootComponent()
{
    return *root;
}




//=============================================================================
void cp::Program::dispatch (const crt::expression& action)
{
    auto which = action.first().as_str();
    auto rule = action.second().as_str();

    if (false) {}

    else if (which == "set")
        kernel.insert_literal (rule, action.item(2).resolve (kernel, adapter));
    else if (which == "inc")
        kernel.insert_literal (rule, kernel.attr (rule).inc());
    else if (which == "dec")
        kernel.insert_literal (rule, kernel.attr (rule).dec());
    else if (which == "toggle")
        kernel.insert_literal (rule, kernel.attr (rule).toggle());

    resolve();
}




//=============================================================================
void cp::Program::resolve()
{
    for (const auto& rule : kernel.update_all (kernel.dirty_rules(), adapter))
    {
        if (! kernel.error_at (rule).empty())
        {
            DBG(kernel.error_at(rule));
        }

        if (rule == "content")
        {
            root->content (kernel.at ("content"));
        }
        else if (rule == "layout")
        {
            root->layout (kernel.at ("layout"));
        }
    }
}
