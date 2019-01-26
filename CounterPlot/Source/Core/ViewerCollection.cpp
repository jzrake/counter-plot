#include "ViewerCollection.hpp"
#include "DataHelpers.hpp"
#include "../Viewers/UserExtensionView.hpp"
#include "yaml-cpp/yaml.h"




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
    listeners.call (&Listener::viewerCollectionViewerAdded, viewerToAdd.get());
    items.add ({ false, File(), Time(), std::move (viewerToAdd) });
}

void ViewerCollection::clear()
{
    for (const auto& item : items)
        listeners.call (&Listener::viewerCollectionViewerRemoved, item.viewer.get());

    items.clear();
    extensionDirectories.clear();
}

Array<File> ViewerCollection::getWatchedDirectories() const
{
    auto result = Array<File>();

    for (auto ext : extensionDirectories)
        result.add (ext.directory);
    return result;
}

std::unique_ptr<XmlElement> ViewerCollection::getWatchedDirectoriesAsXml() const
{
    auto result = std::make_unique<XmlElement> ("FileList");

    for (auto file : getWatchedDirectories())
    {
        auto child = new XmlElement ("File");
        child->addTextElement (file.getFullPathName());
        result->addChildElement (child);
    }
    return result;
}

void ViewerCollection::setWatchedDirectories (const Array<File>& directoriesToWatch)
{
    Array<File> toStopWatching;

    for (auto ext : extensionDirectories)
        if (! directoriesToWatch.contains (ext.directory))
            toStopWatching.add (ext.directory);

    for (auto dir : toStopWatching)
        stopWatchingDirectory (dir);

    for (auto dir : directoriesToWatch)
        startWatchingDirectory (dir);
}

void ViewerCollection::setWatchedDirectories (const XmlElement &directoriesToWatch)
{
    Array<File> directories;

    for (int n = 0; n < directoriesToWatch.getNumChildElements(); ++n)
    {
        auto path = directoriesToWatch.getChildElement(n)->getAllSubText();

        if (File::isAbsolutePath (path))
            directories.add (path);
    }
    setWatchedDirectories (directories);
}

bool ViewerCollection::isExtensionViewerLoaded (File source) const
{
    for (const auto& item : items)
        if (item.source == source)
            return true;
    return false;
}

bool ViewerCollection::watchesDirectory (File directory) const
{
    for (const auto& ext : extensionDirectories)
        if (ext.directory == directory)
            return true;
    return false;
}

void ViewerCollection::startWatchingDirectory (File directory)
{
    if (! watchesDirectory (directory))
    {
        extensionDirectories.add ({directory, Time::getCurrentTime()});
        loadAllInDirectory (directory);
    }
}

void ViewerCollection::stopWatchingDirectory (File directory)
{
    if (watchesDirectory (directory))
    {
        int index = 0;

        for (const auto& ext : extensionDirectories)
        {
            if (ext.directory == directory)
            {
                unloadAllInDirectory (directory);
                break;
            }
            ++index;
        }
        extensionDirectories.remove (index);
    }
}

void ViewerCollection::loadFromYamlString (const String& source)
{
    auto viewer = std::make_unique<UserExtensionView>();
    auto yroot = YAML::Load (source.toStdString());
    auto jroot = DataHelpers::varFromYamlNode (yroot);
    viewer->configure (jroot);
    add (std::move (viewer));
}

Viewer* ViewerCollection::findViewerForFile (File file) const
{
    for (int n = items.size() - 1; n >= 0; --n)
        if (items.getReference(n).viewer->isInterestedInFile (file))
            return items.getReference(n).viewer.get();
    return nullptr;
}

Viewer* ViewerCollection::findViewerWithName (const String& viewerName) const
{
    for (int n = items.size() - 1; n >= 0; --n)
        if (items.getReference(n).viewer->getViewerName() == viewerName)
            return items.getReference(n).viewer.get();
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
                Desktop::getInstance().getAnimator().animateComponent (item.viewer.get(), bounds, 1.f, 200, false, 1.f, 1.f);
            else
                item.viewer->setBounds (bounds);
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
void ViewerCollection::loadAllInDirectory (File directory)
{
    for (auto child : directory.findChildFiles (File::findFiles, false))
    {
        if (child.hasFileExtension (".yaml") && ! isExtensionViewerLoaded (child))
        {
            auto viewer = std::make_unique<UserExtensionView>();
            auto v = viewer.get();
            viewer->configure (child);
            items.add ({ true, child, Time::getCurrentTime(), std::move (viewer) });
            listeners.call (&Listener::viewerCollectionViewerAdded, v);
            listeners.call (&Listener::viewerCollectionViewerReconfigured, v);
        }
    }
}

void ViewerCollection::unloadAllInDirectory (File directory)
{
    auto predicate = [directory] (const auto& item)
    {
        return item.source.isAChildOf (directory);
    };

    for (const auto& item : items)
        if (predicate (item))
            listeners.call (&Listener::viewerCollectionViewerRemoved, item.viewer.get());

    items.removeIf (predicate);
}

void ViewerCollection::unloadAllNonexistentInDirectory (File directory)
{
    auto predicate = [directory] (const auto& item)
    {
        return item.source.isAChildOf (directory) && ! item.source.existsAsFile();
    };

    for (const auto& item : items)
        if (predicate (item))
            listeners.call (&Listener::viewerCollectionViewerRemoved, item.viewer.get());

    items.removeIf (predicate);
}




//=========================================================================
void ViewerCollection::timerCallback()
{
    for (auto& item : items)
    {
        if (item.isExtension && item.lastLoaded < item.source.getLastModificationTime())
        {
            auto& viewer = dynamic_cast<UserExtensionView&> (*item.viewer);
            viewer.configure (item.source);
            item.lastLoaded = Time::getCurrentTime();
            listeners.call (&Listener::viewerCollectionViewerReconfigured, &viewer);
        }
    }

    for (auto& ext : extensionDirectories)
    {
        if (ext.lastLoaded < ext.directory.getLastModificationTime())
        {
            loadAllInDirectory (ext.directory);
            unloadAllNonexistentInDirectory (ext.directory);
        }
    }
}
