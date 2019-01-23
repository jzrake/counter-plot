#include "Viewer.hpp"
#include "../Components/VariantTree.hpp"
#include "../Core/DataHelpers.hpp"
#include "yaml-cpp/yaml.h"




//=============================================================================
void Viewer::registerCommands (ApplicationCommandManager& manager)
{
    Array<CommandID> commands;
    getAllCommands (commands);

    for (auto command : commands)
    {
        ApplicationCommandInfo info (command);
        getCommandInfo (command, info);
        manager.registerCommand (info);
    }
}

void Viewer::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = {
        Commands::makeSnapshotAndOpen,
        Commands::saveSnapshotAs,
        Commands::nextColourMap,
        Commands::prevColourMap,
        Commands::resetScalarRange,
    };
    commands.addArray (ids, numElementsInArray (ids));
}

void Viewer::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case Commands::makeSnapshotAndOpen:
            result.setInfo ("Create Snapshot", "", "File", 0);
            result.defaultKeypresses.add ({'e', ModifierKeys::commandModifier, 0});
            break;
        case Commands::saveSnapshotAs:
            result.setInfo ("Save Snapshot As...", "", "File", 0);
            result.defaultKeypresses.add ({'e', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0});
            break;
        case Commands::nextColourMap:
            result.setInfo ("Next Color Map", "", "View", 0);
            result.defaultKeypresses.add (KeyPress (KeyPress::rightKey, 0, 0));
            break;
        case Commands::prevColourMap:
            result.setInfo ("Previous Color Map", "", "View", 0);
            result.defaultKeypresses.add (KeyPress (KeyPress::leftKey, 0, 0));
            break;
        case Commands::resetScalarRange:
            result.setInfo ("Reset Scalar Range", "", "View", 0);
            result.defaultKeypresses.add (KeyPress (KeyPress::upKey, 0, 0));
            break;
    }
}

void Viewer::sendAsyncTaskStarted() const
{
    if (auto sink = messageSink ? messageSink : findParentComponentOfClass<MessageSink>())
    {
        sink->viewerAsyncTaskStarted();
    }
}

void Viewer::sendAsyncTaskFinished() const
{
    if (auto sink = messageSink ? messageSink : findParentComponentOfClass<MessageSink>())
    {
        sink->viewerAsyncTaskFinished();
    }
}

void Viewer::sendErrorMessage (const String& what) const
{
    if (auto sink = messageSink ? messageSink : findParentComponentOfClass<MessageSink>())
    {
        sink->viewerLogErrorMessage (what);
    }
}

void Viewer::sendIndicateSuccess() const
{
    if (auto sink = messageSink ? messageSink : findParentComponentOfClass<MessageSink>())
    {
        sink->viewerIndicateSuccess();
    }
}

void Viewer::sendEnvironmentChanged() const
{
    if (auto sink = messageSink ? messageSink : findParentComponentOfClass<MessageSink>())
    {
        sink->viewerEnvironmentChanged();
    }
}




//=============================================================================
JsonFileViewer::JsonFileViewer()
{
    addAndMakeVisible (view);
}

bool JsonFileViewer::isInterestedInFile (File file) const
{
    return file.hasFileExtension (".json") || file.hasFileExtension (".yaml");
}

void JsonFileViewer::loadFile (File fileToDisplay)
{
    currentFile = fileToDisplay;
    reloadFile();
}

void JsonFileViewer::reloadFile()
{
    if (currentFile.hasFileExtension (".json"))
    {
        view.setData (JSON::parse (currentFile));
    }
    else if (currentFile.hasFileExtension (".yaml"))
    {
        auto yroot = YAML::LoadFile (currentFile.getFullPathName().toStdString());
        view.setData (DataHelpers::varFromYamlNode (yroot));
    }
}

void JsonFileViewer::resized()
{
    view.setBounds (getLocalBounds());
}




//=============================================================================
ImageFileViewer::ImageFileViewer()
{
    addAndMakeVisible (view);
}

bool ImageFileViewer::isInterestedInFile (File file) const
{
    return ImageFileFormat::findImageFormatForFileExtension (file);
}

void ImageFileViewer::loadFile (File file)
{
    currentFile = file;
    reloadFile();
}

void ImageFileViewer::reloadFile()
{
    if (auto format = ImageFileFormat::findImageFormatForFileExtension (currentFile))
    {
        view.setImage (format->loadFrom (currentFile));
    }
}

void ImageFileViewer::resized()
{
    view.setBounds (getLocalBounds());
}
