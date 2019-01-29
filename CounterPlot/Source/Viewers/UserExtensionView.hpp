#pragma once
#include "Viewer.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Core/Runtime.hpp"
#include "../Core/ConfigurableFileFilter.hpp"
#include "../Core/TaskPool.hpp"




//=============================================================================
/**
 * A class that translates events coming from a component (e.g. a figure or
 * table) into actions that should be carried out on a kernel. Derived classes
 * own the component whose events they translate. Where that component should be
 * displayed is up to you.sub
 */
class KernelAgent
{
public:


    //=========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void kernelAgentInsert (const std::string&, const var&) = 0;
        virtual void kernelAgentInsert (const std::string&, const crt::expression&) = 0;
        virtual void kernelAgentSuggestResolve() = 0;
    };


    //=========================================================================
    virtual ~KernelAgent() {}


    /**
     * This method must return the component whose events this agent listens
     * to and interprets.
     */
    virtual Component& getComponent() = 0;


    /**
     * This method must be implemented to first decode the var object and turn
     * it into a data structure the view component understands, and then update
     * the view with that data structure.
     */
    virtual void setModel (const var& model) = 0;


    /**
     * Add a listener to be notified of intended actions to be applied to the 
     * kernel.
     */
    void addListener (Listener* listener) { listeners.add (listener); }


    /**
     * Remove a listener.
     */
    void removeListener (Listener* listener) { listeners.remove (listener); }


    /**
     * Set the id of this kernel agent. The owner of the kernel and its agents
     * should watch for changes to the rule whose key is this id, and call the
     * setModel method above when changes occur.
     */
    void setAgentID (const std::string& newId) { id = newId; }


    /**
     * Return the id of this agent. This id should be compared against kernel
     * rules when they are updated, and call the setModel method with updated
     * models for the view.
     */
    const std::string& getAgentID() const { return id; }


private:
    ListenerList<Listener> listeners;
    std::string id;
};




//=============================================================================
class UserExtensionView
: public Viewer
, public FigureView::Listener
, public KernelAgent::Listener
, public TaskPool::Listener
, public ApplicationCommandTarget
{
public:

    //=========================================================================
    UserExtensionView();
    void reset();
    void configure (const var& config);
    void configure (File file);

    //=========================================================================
    void resized() override;

    //=========================================================================
    bool isInterestedInFile (File file) const override;
    void loadFile (File fileToDisplay) override;
    void reloadFile() override;
    String getViewerName() const override;
    const Runtime::Kernel* getKernel() const override;
    bool canReceiveMessages() const override;
    bool receiveMessage (const String& message) override;
    bool isRenderingComplete() const override;
    Image createViewerSnapshot() override;
    Array<Component*> getControls() override;

    //=========================================================================
    void figureViewSetMargin (FigureView*, const BorderSize<int>&) override;
    void figureViewSetDomain (FigureView*, const Rectangle<double>&) override;
    void figureViewSetXlabel (FigureView*, const String&) override;
    void figureViewSetYlabel (FigureView*, const String&) override;
    void figureViewSetTitle (FigureView*, const String&) override;

    //=========================================================================
    void kernelAgentInsert (const std::string&, const var&) override;
    void kernelAgentInsert (const std::string&, const crt::expression&) override;
    void kernelAgentSuggestResolve() override;

    //=========================================================================
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    //=========================================================================
    void taskStarted (const String& taskName) override;
    void taskCompleted (const String& taskName, const var& result, const std::string& error) override;
    void taskCancelled (const String& taskName) override;

private:

    //=========================================================================
    void applyLayout();
    void resolveKernel();
    void loadFromKernelIfFigure (const std::string& id);
    void loadFromKernelIfControl (const std::string& id);
    void loadExpressionsFromDictIntoKernel (Runtime::Kernel& kernel, const var& dict, bool rethrowExceptions=false) const;
    void saveSnapshot (bool toTempDirectory);

    //=========================================================================
    String viewerName;
    Grid layout;
    ColourMapCollection colourMaps;
    ConfigurableFileFilter fileFilter;
    Runtime::Kernel kernel;
    OwnedArray<FigureView> figures;
    OwnedArray<KernelAgent> controls;
    File currentFile;
    TaskPool taskPool;
    StringArray asyncRules;
    var extensionCommands;
};
