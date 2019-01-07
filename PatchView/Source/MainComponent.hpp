#pragma once
#include "JuceHeader.h"
#include "Views/DirectoryTree.hpp"
#include "Views/VariantView.hpp"
#include "Views/JetInCloudView.hpp"
#include "Views/BinaryTorquesView.hpp"




//=============================================================================
class MainComponent
: public Component
, public DirectoryTree::Listener
{
public:
    //=========================================================================
    MainComponent();
    ~MainComponent();
    void setCurrentDirectory (File newCurrentDirectory);

    //=========================================================================
    void paint (Graphics&) override;
    void resized() override;
    bool keyPressed (const KeyPress& key) override;

    //=========================================================================
    void selectedFileChanged (DirectoryTree*, File) override;

private:
    //=========================================================================
    ImageComponent imageView;
    VariantView variantView;
    JetInCloudView jetInCloudView;
    BinaryTorquesView binaryTorquesView;
    DirectoryTree directoryTree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
