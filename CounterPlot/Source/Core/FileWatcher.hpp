#pragma once
#include "JuceHeader.h"



//=============================================================================
class FileWatcher : public Timer
{
public:

    //=========================================================================
    void setCallback (std::function<void(File)> callbackToInvoke)
    {
        callback = callbackToInvoke;
    }

    void setFileToWatch (File fileToPoll)
    {
        startTimer (100);
        file = fileToPoll;
        lastNotified = Time::getCurrentTime();
    }

    //=========================================================================
    void timerCallback() override
    {
        if (lastNotified < file.getLastModificationTime())
        {
            lastNotified = Time::getCurrentTime();

            if (callback)
                callback (file);
        }
    }

private:
    Time lastNotified;
    File file;
    std::function<void(File)> callback = nullptr;
};
