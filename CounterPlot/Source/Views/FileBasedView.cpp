#include "FileBasedView.hpp"
#include "VariantView.hpp"




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
