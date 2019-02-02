#include "Program.hpp"
#include "../Views/Division.hpp"




//=============================================================================
static std::unique_ptr<cp::View> factory (const std::string& type_name)
{
    if (type_name == "Div")
    {
        return std::make_unique<Division>();
    }
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
    kernel.insert (action);
    resolve();
}




//=============================================================================
void cp::Program::resolve()
{
    for (const auto& rule : kernel.update_all (kernel.dirty_rules(), adapter))
    {
        if (rule == "views")
            changeToViewModelList();
        
        else if (rule == "layout")
            changeToLayout();
    }
}

void cp::Program::changeToViewModelList()
{
    int n = 0;
    bool childrenChanged = false;

    for (const auto& viewModel : kernel.at ("views"))
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
