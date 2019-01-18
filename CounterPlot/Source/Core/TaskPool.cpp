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
    return taskPool.notify (this, task ([this] { return shouldExit(); }), ! shouldExit());
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
    for (auto job : jobs)
        if (job->getJobName() == name)
            threadPool.removeJob (job, true, 0);

    auto job = std::make_unique<Job> (*this, name, task);
    threadPool.addJob (job.get(), false);
    jobs.add (job.release());
}

ThreadPoolJob::JobStatus TaskPool::notify (Job* job, var result, bool completed)
{
    // 1.

    MessageManager::callAsync ([this, job, name=job->getJobName(), result, completed]
    {
        // 3.
        jobs.removeObject (job);

        if (completed)
            listeners.call (&Listener::taskCompleted, name, result);
        else
            listeners.call (&Listener::taskWasCancelled, name);
    });

    // 2.
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
        pool.enqueue ("Task 1", [] (auto bailout) {

            int n = 0;
            while (++n < 100000000 && ! bailout()) {}
            return var("Task 1 result");
        });
    };
    addTask2.onClick = [this]
    {
        resultLabel2.setText ("Task 2 Running", NotificationType::dontSendNotification);
        pool.enqueue ("Task 2", [] (auto bailout) {
            int n = 0;
            while (++n < 100000000 && ! bailout()) {}
            return var("Task 2 result");
        });
    };
    addTask3.onClick = [this]
    {
        resultLabel3.setText ("Task 3 Running", NotificationType::dontSendNotification);
        pool.enqueue ("Task 3", [] (auto bailout) {
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

void TaskPoolTestComponent::taskCompleted (const String& taskName, const var& result)
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
