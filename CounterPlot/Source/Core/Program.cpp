#include "Program.hpp"
#include "../Views/Division.hpp"




//=============================================================================
static std::unique_ptr<cp::View> factory (const std::string& type_name)
{
    if (type_name == "Div")
        return std::make_unique<cp::Div>();
    if (type_name == "Text")
        return std::make_unique<cp::Text>();
    return nullptr;
}




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




//=============================================================================
cp::ViewHolder::ViewHolder (Program& program) : program (program)
{
}

void cp::ViewHolder::setValue (const crt::expression& newValue)
{
    if (value != newValue)
    {
        if (value.type_name() != newValue.type_name())
        {
            view = factory (newValue.type_name());

            if (view)
            {
                view->setBounds (getLocalBounds());
                view->setActionSink (&program);
                view->load (newValue);
                addAndMakeVisible (*view);
            }
        }
        else if (view)
        {
            view->load (newValue);
        }
        value = newValue;
    }
}

void cp::ViewHolder::resized()
{
    if (view)
    {
        view->setBounds (getLocalBounds());
    }
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
    for (const auto& expr : crt::fromYamlFile (file))
    {
        kernel.insert (expr);
    }
    resolve();
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
            changeToContent();

        else if (rule == "layout")
            changeToLayout();
    }
}

void cp::Program::changeToContent()
{
    int n = 0;
    bool childrenChanged = false;

    for (const auto& viewModel : kernel.at ("content"))
    {
        if (n < viewHolders.size())
        {
            viewHolders[n]->setValue (viewModel);
        }
        else
        {
            auto holder = std::make_unique<ViewHolder> (*this);
            holder->setValue (viewModel);
            root->addAndMakeVisible (*holder);
            viewHolders.add (holder.release());
            childrenChanged = true;
        }
        ++n;
    }

    if (viewHolders.size() > n)
    {
        viewHolders.removeLast (viewHolders.size() - n);
    }

    if (childrenChanged)
    {
        root->layout();
    }
}

void cp::Program::changeToLayout()
{
    root->grid = kernel.at ("layout").to<Grid>();
    root->layout();
}
