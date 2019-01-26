#pragma once
#include "JuceHeader.h"




//=============================================================================
/**
 * A TaskPool is a thin wrapper around a JUCE ThreadPool intended to
 * facilitate asynchronous tasks that return a value, and which may supercede
 * one another. Adding a task with a given name causes any queued or running
 * tasks with the same name to be cancelled. Tasks are lamba functions,
 * std::function<var(BailoutChecker)> where BailoutChecker is also a lambda,
 * std::function<bool()>. The bailout checker for a task will begin returning
 * false if that task has been cancelled. If it has, the var returned by the
 * lambda function is disregarded and the Listener method taskWasCancelled
 * gets invoked with the task name. When a task completes successfully,
 * Listener::taskCompleted gets invoked with the name of the task and the
 * resulting var.
 */
class TaskPool
{
public:


    //=========================================================================
    using BailoutChecker = std::function<bool()>;
    using Task = std::function<var(BailoutChecker)>;


    //=========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void taskStarted (const String& taskName) = 0;
        virtual void taskCompleted (const String& taskName, const var& result, const std::string& error) = 0;
        virtual void taskCancelled (const String& taskName) = 0;
    };


    //=========================================================================
    TaskPool (int numThreads=1);
    ~TaskPool();
    void addListener (Listener* listener);
    void removeListener (Listener* listener);


    /**
     * Add a task to the queue with the given name. Cancels any earlier tasks
     * with the same name.
     */
    void enqueue (const String& name, Task task);


    /**
     * Cancel any tasks with the given name. Returns immediately. taskWasCancelled
     * will be called when the thread actually exits.
     */
    void cancel (const String& name);


    /**
     * Cancel all tasks. Returns immediately. taskWasCancelled will be called when the
     * thread actually exits.
     */
    void cancelAll();


    /**
     * Return the number of running or queued jobs.
     */
    int getNumJobsRunningOrQueued() const;


private:


    //=========================================================================
    class Job : public ThreadPoolJob
    {
    public:
        Job (TaskPool& taskPool, const String& name, Task task);
        JobStatus runJob() override;
        TaskPool& taskPool;
        Task task;
    };


    //=========================================================================
    class Selector : public ThreadPool::JobSelector
    {
    public:
        Selector (const String& name);
        bool isJobSuitable (ThreadPoolJob*) override;
        String name;
    };


    //=========================================================================
    ThreadPoolJob::JobStatus notify (String name, var result, std::string error, bool completed);
    void indicateJobStarted (String name);

    ThreadPool threadPool;
    ListenerList<Listener> listeners;
};




//=============================================================================
class TaskPoolTestComponent : public Component, public TaskPool::Listener
{
public:


    //=========================================================================
    TaskPoolTestComponent();
    void taskStarted (const String& taskName) override {}
    void taskCompleted (const String& taskName, const var& result, const std::string& error) override;
    void taskCancelled (const String& taskName) override;


    //=========================================================================
    void resized() override;


private:


    //=========================================================================
    TaskPool pool;
    Grid layout;
    Label resultLabel1;
    Label resultLabel2;
    Label resultLabel3;
    TextButton addTask1;
    TextButton addTask2;
    TextButton addTask3;
};
