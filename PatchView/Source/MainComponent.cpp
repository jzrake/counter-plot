#include "MainComponent.hpp"
#include "MetalSurface.hpp"




//=============================================================================
MainComponent::MainComponent()
{
    directoryTree.setDirectoryToShow (File::getSpecialLocation(File::SpecialLocationType::userHomeDirectory));
    directoryTree.addListener (this);

    addAndMakeVisible (directoryTree);
    addChildComponent (imageView);
    addChildComponent (variantView);
    addAndMakeVisible (jetInCloudView);
    setSize (1024, 768 - 64);
}

MainComponent::~MainComponent()
{
}

void MainComponent::setCurrentDirectory (File newCurrentDirectory)
{
    directoryTree.setDirectoryToShow (newCurrentDirectory);
}

void MainComponent::paint (Graphics& g)
{
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    directoryTree.setBounds (area.removeFromLeft(300));

    imageView.setBounds (area);
    variantView.setBounds (area);
    jetInCloudView.setBounds (area);
}

bool MainComponent::keyPressed (const juce::KeyPress &key)
{
    return false;
}




//=============================================================================
void MainComponent::selectedFileChanged (DirectoryTree*, File file)
{
    if (FileSystemSerializer::looksLikeDatabase (file))
    {
        jetInCloudView.setDocumentFile (file);

        imageView.setVisible (false);
        variantView.setVisible (false);
        jetInCloudView.setVisible (true);
    }
    if (ColourMapHelpers::looksLikeRGBTable (file))
    {
//        auto cb = ScalarMapping();
//        cb.stops = ColourmapHelpers::coloursFromRGBTable (file.loadFileAsString());
//        model.content.clear();
//        model.content.push_back (std::make_shared<ColourGradientArtist> (cb));
//        figure.setModel (model);
//
//        figure.setVisible (true);
//        imageView.setVisible (false);
//        variantView.setVisible (false);
//        jetInCloudView.setVisible (false);
    }
    else if (auto format = ImageFileFormat::findImageFormatForFileExtension (file))
    {
        imageView.setImage (format->loadFrom (file));

        imageView.setVisible (true);
        variantView.setVisible (false);
        jetInCloudView.setVisible (false);
    }
    else if (file.hasFileExtension (".json"))
    {
        variantView.setData (JSON::parse (file));

        imageView.setVisible (false);
        variantView.setVisible (true);
        jetInCloudView.setVisible (false);
    }
}
