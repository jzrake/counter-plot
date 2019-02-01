#include "Program.hpp"




//=============================================================================
void cp::View::setExpression (const crt::expression& newExpression)
{
    load (expression = newExpression);
}

const crt::expression& cp::View::getExpression() const
{
    return expression;
}




//=============================================================================
class RectangleView : public cp::View
{
public:

    void load (const crt::expression& e) override
    {
        bg = e.attr ("background").to<Colour>();
        repaint();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (bg);
    }

private:
    Colour bg;
};




//=============================================================================
class cp::Program::RootComponent : public Component
{
public:
    void resized() override
    {
        grid.performLayout (getLocalBounds());
    }
    Grid grid;
};




//=============================================================================
cp::Program::Program()
{
    root = std::make_unique<RootComponent>();
    crt::core::import(kernel);
}

cp::Program::~Program()
{
}

void cp::Program::loadCommandsFromFile (File file)
{
    auto base = crt::fromYamlFile (file);
    root->grid = base.attr ("layout").resolve (kernel, adapter).to<Grid>();

    views.clear();

    for (auto elem : base.attr ("views"))
    {
        views.add (createView (elem).release());
    }

    root->grid.items.clear();

    for (auto view : views)
    {
        root->addAndMakeVisible (view);
        root->grid.items.add (view);
    }
    root->resized();
}

void cp::Program::clear()
{
    kernel.clear();
    views.clear();
}

Component& cp::Program::getRootComponent()
{
    return *root;
}




//=============================================================================
std::unique_ptr<cp::View> cp::Program::createView (const crt::expression& e)
{
    if (e.attr ("type") == std::string ("rect"))
    {
        auto result = std::make_unique<RectangleView>();
        result->setExpression (e);
        return result;
    }
    return nullptr;
}
