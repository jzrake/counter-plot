#pragma once
#include "JuceHeader.h"
#include "DirectoryTree.hpp"
#include "../Plotting/FigureView.hpp"
#include "../Viewers/Viewer.hpp"
#include "../Viewers/UserExtensionView.hpp"




//=============================================================================
class ViewerCollection : public Timer
{
public:

    //=========================================================================
    class Listener
    {
    public:
        virtual ~Listener() {}
        virtual void extensionViewerReconfigured (UserExtensionView*) = 0;
    };

    //=========================================================================
    ViewerCollection()
    {
        startTimer (330);
    }

    void addListener (Listener* listener)
    {
        listeners.add (listener);
    }

    void removeListener (Listener* listener)
    {
        listeners.remove (listener);
    }

    void add (std::unique_ptr<Viewer> viewerToAdd)
    {
        items.add ({ false, File(), Time(), std::move (viewerToAdd) });
    }

    void clear()
    {
        items.clear();
    }

    void loadAllInDirectory (File directory)
    {
        for (auto child : directory.findChildFiles (File::findFiles, false))
        {
            if (child.hasFileExtension (".yaml"))
            {
                auto viewer = std::make_unique<UserExtensionView>();
                viewer->configure (child);
                listeners.call (&Listener::extensionViewerReconfigured, viewer.get());
                items.add ({ true, child, Time::getCurrentTime(), std::move (viewer) });
            }
        }
    }

    Viewer* findViewerForFile (File file) const
    {
        for (const auto& item : items)
            if (item.viewer->isInterestedInFile (file))
                return item.viewer.get();
        return nullptr;
    }

    Array<Viewer*> getAllComponents() const
    {
        Array<Viewer*> result;

        for (const auto& item : items)
            result.add (item.viewer.get());
        return result;
    }

    void setBounds (const Rectangle<int>& bounds) const
    {
        for (const auto& item : items)
            item.viewer->setBounds (bounds);
    }

    void showOnly (Viewer* componentThatShouldBeVisible) const
    {
        for (const auto& item : items)
            item.viewer->setVisible (item.viewer.get() == componentThatShouldBeVisible);
    }

private:

    //=========================================================================
    void timerCallback() override
    {
        for (auto& item : items)
        {
            if (item.isExtension && item.lastLoaded < item.source.getLastModificationTime())
            {
                DBG("reloading viewer " << item.source.getFileName());

                auto& viewer = dynamic_cast<UserExtensionView&> (*item.viewer);
                viewer.configure (item.source);
                item.lastLoaded = Time::getCurrentTime();

                listeners.call (&Listener::extensionViewerReconfigured, &viewer);
            }
        }
    }


    //=========================================================================
    struct Item
    {
        bool isExtension = false;
        File source;
        Time lastLoaded;
        std::unique_ptr<Viewer> viewer;
    };
    Array<Item> items;
    ListenerList<Listener> listeners;
};




//=============================================================================
class StatusBar : public Component
{
public:

    //=========================================================================
    void incrementAsyncTaskCount();
    void decrementAsyncTaskCount();
    void setMousePositionInFigure (Point<double> position);
    void setCurrentViewerName (const String& viewerName);

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    Point<double> mousePositionInFigure;
    String currentViewerName;
    int numberOfAsyncTasks = 0;
};




//=============================================================================
class MainComponent
: public Component
, public DirectoryTree::Listener
, public FigureView::MessageSink
, public Viewer::MessageSink
, public ViewerCollection::Listener
{
public:

    //=========================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);
    void reloadCurrentFile();
    void reloadDirectoryTree();
    void toggleDirectoryTreeShown();
    bool isDirectoryTreeShowing() const;
    File getCurrentDirectory() const;

    //=========================================================================
    void paint (Graphics&) override;
    void paintOverChildren (Graphics& g) override;
    void resized() override;
    bool keyPressed (const KeyPress& key) override;

    //=========================================================================
    void selectedFileChanged (DirectoryTree*, File) override;

    //=========================================================================
    void figureMousePosition (Point<double> position) override;

    //=========================================================================
    void ViewerAsyncTaskStarted() override;
    void ViewerAsyncTaskFinished() override;

    //=========================================================================
    void extensionViewerReconfigured (UserExtensionView*) override;

private:
    //=========================================================================
    void layout (bool animated);

    //=========================================================================
    File currentFile;
    bool directoryTreeShowing = true;

    //=========================================================================
    StatusBar statusBar;
    DirectoryTree directoryTree;
    // OwnedArray<Viewer> views;
    ViewerCollection viewers;
};
