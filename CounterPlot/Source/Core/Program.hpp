#pragma once
#include "JuceHeader.h"
#include "Runtime.hpp"




//=============================================================================
namespace cp {
    class Program;
    class View;
    class ViewHolder;
}




//=============================================================================
class cp::View : public Component
{
public:
    virtual ~View() {}
    virtual void load (const crt::expression& newValue) = 0;
};




//=============================================================================
class cp::ViewHolder : public Component
{
public:
    void setValue (const crt::expression& newValue);
    void setExpression (const crt::expression& newExpression);
    const crt::expression& getExpression() const;

    void resized() override;

private:
    crt::expression expr;
    crt::expression value;
    std::unique_ptr<View> view;
};




//=============================================================================
class cp::Program
{
public:

    Program();
    ~Program();


    /**
     * Load all commands from the given file, generating kernel inserts or
     * meta-data modifications appropriately.
     */
    void loadCommandsFromFile (File file);


    /**
     * Reset the kernel and delete all components.
     */
    void clear();


    /**
     * Return the root component, in which child views are mounted.
     */
    Component& getRootComponent();


private:
    class RootComponent;
    void loadEnvironmentEntry (const crt::expression&);
    void loadViewEntry (const crt::expression&);

    crt::kernel kernel;
    crt::call_adapter adapter;
    OwnedArray<ViewHolder> viewHolders;
    std::unique_ptr<RootComponent> root;
};
