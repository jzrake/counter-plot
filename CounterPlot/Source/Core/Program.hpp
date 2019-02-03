#pragma once
#include "JuceHeader.h"
#include "Runtime.hpp"




//=============================================================================
namespace cp {
    class ActionSink;
    class Program;
    class View;
    class ViewHolder;
}




//=============================================================================
class cp::ActionSink
{
public:
    virtual ~ActionSink() {}
    virtual void dispatch (const crt::expression& action) = 0;
};




//=============================================================================
class cp::View : public Component
{
public:
    virtual ~View() {}
    virtual void load (const crt::expression& newValue) = 0;

    void setActionSink (ActionSink* sinkToUse);

protected:
    void sink (const crt::expression& action);
    ActionSink* actionSink = nullptr;
};




//=============================================================================
class cp::ViewHolder : public Component
{
public:
    ViewHolder (Program&);
    void setValue (const crt::expression& newValue);
    void setExpression (const crt::expression& newExpression);
    const crt::expression& getExpression() const;

    void resized() override;

private:
    crt::expression value;
    Program& program;
    std::unique_ptr<View> view;
};




//=============================================================================
class cp::Program : public ActionSink
{
public:


    //=========================================================================
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


    //=========================================================================
    void dispatch (const crt::expression& action) override;

private:

    //=========================================================================
    class RootComponent;
    void resolve();
    void changeToContent();
    void changeToLayout();

    //=========================================================================
    crt::kernel kernel;
    crt::call_adapter adapter;
    OwnedArray<ViewHolder> viewHolders;
    std::unique_ptr<RootComponent> root;
};
