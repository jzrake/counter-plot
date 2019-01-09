#include "FileBasedView.hpp"
#include "VariantView.hpp"




//=============================================================================
void FileBasedView::registerCommands (ApplicationCommandManager& manager)
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

void FileBasedView::getAllCommands (Array<CommandID>& commands)
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

void FileBasedView::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
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




//=============================================================================
JsonFileViewer::JsonFileViewer()
{
    addAndMakeVisible (view);
}

bool JsonFileViewer::isInterestedInFile (File file) const
{
    return file.hasFileExtension (".json");
}

bool JsonFileViewer::loadFile (File fileToDisplay)
{
    view.setData (JSON::parse (fileToDisplay));
    return true;
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

bool ImageFileViewer::loadFile (File file)
{
    if (auto format = ImageFileFormat::findImageFormatForFileExtension (file))
    {
        view.setImage (format->loadFrom (file));
    }
    return true;
}

void ImageFileViewer::resized()
{
    view.setBounds (getLocalBounds());
}
