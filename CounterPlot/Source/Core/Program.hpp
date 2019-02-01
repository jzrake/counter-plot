#pragma once
#include "JuceHeader.h"
#include "Runtime.hpp"




//=============================================================================
namespace cp {
    class Program;
    class View;
}




//=============================================================================
class cp::View : public Component
{
public:
    virtual ~View() {}
    virtual void load (const crt::expression&) = 0;

    void setExpression (const crt::expression& newExpression);
    const crt::expression& getExpression() const;

private:
    crt::expression expression;
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
    std::unique_ptr<View> createView (const crt::expression&);
    class RootComponent;
    crt::kernel kernel;
    crt::call_adapter adapter;
    OwnedArray<View> views;
    std::unique_ptr<RootComponent> root;
};
