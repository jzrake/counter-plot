#pragma once
#include "JuceHeader.h"
#include "Runtime.hpp"




//=============================================================================
namespace cp {
    class ActionSink;
    class Program;
    class View;
    class ViewHolder;
    class Div;
    class Text;
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
    virtual void load (const crt::expression& newValue) {}
    void setActionSink (ActionSink* sinkToUse);
    bool hasSameType (const crt::expression&) const;
    const crt::expression& getLastModel() const;

protected:
    void sink (const crt::expression& action);
    ActionSink* actionSink = nullptr;

private:
    crt::expression lastModel;
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
     * Return the root component, in which child views are mounted.
     */
    Component& getRootComponent();


    //=========================================================================
    void dispatch (const crt::expression& action) override;

private:

    //=========================================================================
    void resolve();

    //=========================================================================
    crt::kernel kernel;
    crt::call_adapter adapter;
    std::unique_ptr<Div> root;
};
