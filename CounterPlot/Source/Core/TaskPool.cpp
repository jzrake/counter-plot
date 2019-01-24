#include "TaskPool.hpp"




//=========================================================================
TaskPool::Job::Job (TaskPool& taskPool, const String& name, Task task)
: ThreadPoolJob (name)
, taskPool (taskPool)
, task (task)
{
}

ThreadPoolJob::JobStatus TaskPool::Job::runJob()
{
    try {
        auto result = task ([this] { return shouldExit(); });
        return taskPool.notify (getJobName(), result, std::string(), ! shouldExit());
    }
    catch (const std::exception& e)
    {
        return taskPool.notify (getJobName(), var(), e.what(), ! shouldExit());
    }
}




//=========================================================================
TaskPool::Selector::Selector (const String& name) : name (name)
{
}

bool TaskPool::Selector::isJobSuitable (ThreadPoolJob* job)
{
    return job->getJobName() == name;
}




//=========================================================================
TaskPool::TaskPool (int numThreads) : threadPool (numThreads) {}

TaskPool::~TaskPool()
{
    threadPool.removeAllJobs (true, 4000);
}

void TaskPool::addListener (Listener* listener)
{
    listeners.add (listener);
}

void TaskPool::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void TaskPool::enqueue (const String& name, Task task)
{
    listeners.call (&Listener::taskStarted, name);
    Selector jobsToRemove (name);
    threadPool.removeAllJobs (true, 0, &jobsToRemove);
    threadPool.addJob (new Job (*this, name, task), true);
}

void TaskPool::cancel (const String& name)
{
    Selector jobsToRemove (name);
    threadPool.removeAllJobs (true, 0, &jobsToRemove);
}

void TaskPool::cancelAll()
{
    threadPool.removeAllJobs (true, 0);
}

StringArray TaskPool::getActiveTaskNames() const
{
    return threadPool.getNamesOfAllJobs (false);
}




//=========================================================================
ThreadPoolJob::JobStatus TaskPool::notify (String name, var result, std::string error, bool completed)
{
    MessageManager::callAsync ([this, name, result, error, completed]
    {
        if (completed)
            listeners.call (&Listener::taskCompleted, name, result, error);
        else
            listeners.call (&Listener::taskWasCancelled, name);
    });
    return ThreadPoolJob::jobHasFinished;
}




//=========================================================================
TaskPoolTestComponent::TaskPoolTestComponent() : pool (4)
{
    pool.addListener (this);

    addTask1.setButtonText ("Trigger Task 1");
    addTask2.setButtonText ("Trigger Task 2");
    addTask3.setButtonText ("Trigger Task 3");

    addTask1.onClick = [this]
    {
        resultLabel1.setText ("Task 1 Running", NotificationType::dontSendNotification);
        pool.enqueue ("Task 1", [this] (auto bailout) {
            int n = 0;
            while (++n < 100000000 && ! bailout()) {}
            return var("Task 1 result");
        });
    };
    addTask2.onClick = [this]
    {
        resultLabel2.setText ("Task 2 Running", NotificationType::dontSendNotification);
        pool.enqueue ("Task 2", [this] (auto bailout) {
            int n = 0;
            while (++n < 100000000 && ! bailout()) {}
            return var("Task 2 result");
        });
    };
    addTask3.onClick = [this]
    {
        resultLabel3.setText ("Task 3 Running", NotificationType::dontSendNotification);
        pool.enqueue ("Task 3", [this] (auto bailout) {
            int n = 0;
            while (++n < 100000000 && ! bailout()) {}
            return var("Task 3 result");
        });
    };

    addAndMakeVisible (addTask1);
    addAndMakeVisible (addTask2);
    addAndMakeVisible (addTask3);
    addAndMakeVisible (resultLabel1);
    addAndMakeVisible (resultLabel2);
    addAndMakeVisible (resultLabel3);

    layout.templateRows = { 1_fr, 1_fr, 1_fr };
    layout.templateColumns = { 1_fr, 1_fr };
    layout.items.add (&addTask1);
    layout.items.add (&resultLabel1);
    layout.items.add (&addTask2);
    layout.items.add (&resultLabel2);
    layout.items.add (&addTask3);
    layout.items.add (&resultLabel3);
}

void TaskPoolTestComponent::taskCompleted (const String& taskName, const var& result, const std::string&)
{
    if (taskName == "Task 1")
        resultLabel1.setText (result.toString(), NotificationType::dontSendNotification);
    if (taskName == "Task 2")
        resultLabel2.setText (result.toString(), NotificationType::dontSendNotification);
    if (taskName == "Task 3")
        resultLabel3.setText (result.toString(), NotificationType::dontSendNotification);
}

void TaskPoolTestComponent::taskWasCancelled (const String& taskName)
{
    if (taskName == "Task 1")
        resultLabel1.setText ("cancelled", NotificationType::dontSendNotification);
    if (taskName == "Task 2")
        resultLabel2.setText ("cancelled", NotificationType::dontSendNotification);
    if (taskName == "Task 3")
        resultLabel3.setText ("cancelled", NotificationType::dontSendNotification);
}

void TaskPoolTestComponent::resized()
{
    layout.performLayout (getLocalBounds());
}
