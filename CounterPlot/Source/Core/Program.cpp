#include "Program.hpp"




//=============================================================================
class Division : public cp::View
{
public:

    void load (const crt::expression& e) override
    {
        model = e.to<DivModel>();
        repaint();
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
                g.drawRect (area.reduced (model.borderWidth / 2), model.borderWidth);
            }
        }
    }

private:
    DivModel model;
};




//=============================================================================
void cp::ViewHolder::setValue (const crt::expression& newValue)
{
    value = newValue;

    if (value.type_name() == std::string ("Div"))
    {
        view = std::make_unique<Division>();
        view->load (value);
        view->setBounds (getLocalBounds());
        addAndMakeVisible (*view);
    }
    else
    {
        view.reset();
    }
}

void cp::ViewHolder::setExpression (const crt::expression& newExpression)
{
    expr = newExpression;
}

const crt::expression& cp::ViewHolder::getExpression() const
{
    return expr;
}

void cp::ViewHolder::resized()
{
    if (view)
        view->setBounds (getLocalBounds());
}




//=============================================================================
class cp::Program::RootComponent : public Component
{
public:
    void layout()
    {
        grid.items.clear();

        for (auto child : getChildren())
        {
            grid.items.add (child);
        }
        grid.performLayout (getLocalBounds());
    }

    void resized() override
    {
        layout();
    }

    Grid grid;
};




//=============================================================================
cp::Program::Program()
{
    root = std::make_unique<RootComponent>();
    crt::core::import (kernel);
}

cp::Program::~Program()
{
}

void cp::Program::loadCommandsFromFile (File file)
{
    auto commands = crt::fromYamlFile (file);

    viewHolders.clear();

    for (const auto& expr : commands.attr ("views"))
    {
        loadViewEntry (expr);
    }
    for (const auto& expr : commands.attr ("environment"))
    {
        loadEnvironmentEntry (expr);
    }
    if (! commands.attr ("layout").empty())
    {
        kernel.insert ("layout", commands.attr ("layout"));
    }


    auto updated = kernel.dirty_rules();
    kernel.update_all (updated, adapter);


    for (auto holder : viewHolders)
    {
        const auto& expr = holder->getExpression();
        holder->setValue (expr.resolve (kernel, adapter));
    }

    root->grid = kernel.at ("layout").to<Grid>();
    root->layout();
}

void cp::Program::clear()
{
    kernel.clear();
    viewHolders.clear();
}

Component& cp::Program::getRootComponent()
{
    return *root;
}




//=============================================================================
void cp::Program::loadEnvironmentEntry (const crt::expression& e)
{
    if (e.empty())
    {
        kernel.erase (e.key());
    }
    else
    {
        kernel.insert (e.key(), e);
    }
}

void cp::Program::loadViewEntry (const crt::expression& e)
{
    auto holder = std::make_unique<ViewHolder>();
    holder->setExpression (e);
    root->addAndMakeVisible (holder.get());
    viewHolders.add (holder.release());
}
