#pragma once
#include "JuceHeader.h"
#include "Views/DirectoryTree.hpp"
#include "Views/FigureView.hpp"
#include "Views/FileBasedView.hpp"
#include "Views/UserExtensionView.hpp"




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

    void add (std::unique_ptr<FileBasedView> viewToAdd)
    {
        viewers.add ({ false, File(), Time(), std::move (viewToAdd) });
    }

    void clear()
    {
        viewers.clear();
    }

    void loadAllInDirectory (File directory)
    {
        for (auto child : directory.findChildFiles (File::findFiles, false))
        {
            if (child.hasFileExtension (".yaml"))
            {
                auto view = std::make_unique<UserExtensionView>();
                view->configure (child);
                listeners.call (&Listener::extensionViewerReconfigured, view.get());
                viewers.add ({ true, child, Time::getCurrentTime(), std::move (view) });
            }
        }
    }

    FileBasedView* findViewerForFile (File file) const
    {
        for (const auto& viewer : viewers)
            if (viewer.component->isInterestedInFile (file))
                return viewer.component.get();
        return nullptr;
    }

    Array<FileBasedView*> getAllComponents() const
    {
        Array<FileBasedView*> result;

        for (const auto& viewer : viewers)
            result.add (viewer.component.get());
        return result;
    }

    void setBounds (const Rectangle<int>& bounds) const
    {
        for (const auto& viewer : viewers)
            viewer.component->setBounds (bounds);
    }

    void showOnly (FileBasedView* componentThatShouldBeVisible) const
    {
        for (const auto& viewer : viewers)
            viewer.component->setVisible (viewer.component.get() == componentThatShouldBeVisible);
    }

private:

    //=========================================================================
    void timerCallback() override
    {
        for (auto& viewer : viewers)
        {
            if (viewer.isExtension && viewer.lastLoaded < viewer.source.getLastModificationTime())
            {
                DBG("reloading viewer " << viewer.source.getFileName());

                auto& component = dynamic_cast<UserExtensionView&> (*viewer.component);
                component.configure (viewer.source);
                viewer.lastLoaded = Time::getCurrentTime();

                listeners.call (&Listener::extensionViewerReconfigured, &component);
            }
        }
    }


    //=========================================================================
    struct Viewer
    {
        bool isExtension = false;
        File source;
        Time lastLoaded;
        std::unique_ptr<FileBasedView> component;
    };
    Array<Viewer> viewers;
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
, public FileBasedView::MessageSink
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
    void fileBasedViewAsyncTaskStarted() override;
    void fileBasedViewAsyncTaskFinished() override;

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
    // OwnedArray<FileBasedView> views;
    ViewerCollection viewers;
};
