#include "ViewerCollection.hpp"
#include "../Viewers/UserExtensionView.hpp"




//=============================================================================
ViewerCollection::ViewerCollection()
{
    startTimer (330);
}

void ViewerCollection::addListener (Listener* listener)
{
    listeners.add (listener);
}

void ViewerCollection::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void ViewerCollection::add (std::unique_ptr<Viewer> viewerToAdd)
{
    items.add ({ false, File(), Time(), std::move (viewerToAdd) });
}

void ViewerCollection::clear()
{
    items.clear();
}

void ViewerCollection::loadAllInDirectory (File directory, Viewer::MessageSink* messageSink)
{
    for (auto child : directory.findChildFiles (File::findFiles, false))
    {
        if (child.hasFileExtension (".yaml"))
        {
            auto viewer = std::make_unique<UserExtensionView>();
            viewer->setMessageSink (messageSink);
            viewer->configure (child);
            listeners.call (&Listener::extensionViewerReconfigured, viewer.get());
            items.add ({ true, child, Time::getCurrentTime(), std::move (viewer) });
        }
    }
}

Viewer* ViewerCollection::findViewerForFile (File file) const
{
    for (const auto& item : items)
        if (item.viewer->isInterestedInFile (file))
            return item.viewer.get();
    return nullptr;
}

Viewer* ViewerCollection::findViewerWithName (const String& viewerName) const
{
    for (const auto& item : items)
        if (item.viewer->getViewerName() == viewerName)
            return item.viewer.get();
    return nullptr;
}

Array<Viewer*> ViewerCollection::getAllComponents() const
{
    Array<Viewer*> result;

    for (const auto& item : items)
        result.add (item.viewer.get());
    return result;
}

void ViewerCollection::setBounds (const Rectangle<int>& newBounds, bool animated)
{
    bounds = newBounds;

    for (const auto& item : items)
    {
        if (item.viewer->isVisible())
        {
            if (animated)
            {
                Desktop::getInstance().getAnimator().animateComponent (item.viewer.get(), bounds, 1.f, 200, false, 1.f, 1.f);
            }
            else
            {
                item.viewer->setBounds (bounds);
            }
        }
    }
}

void ViewerCollection::showOnly (Viewer* componentThatShouldBeVisible) const
{
    for (const auto& item : items)
    {
        item.viewer->setVisible (item.viewer.get() == componentThatShouldBeVisible);
        item.viewer->setBounds (bounds);
    }
}




//=========================================================================
void ViewerCollection::timerCallback()
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
