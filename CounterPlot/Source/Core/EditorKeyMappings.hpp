#pragma once
#include "JuceHeader.h"




// =============================================================================
class EditorKeyMappings : public KeyListener
{
public:
    bool keyPressed (const juce::KeyPress& key, juce::Component*) override;
    std::function<bool()> escapeKeyCallback = nullptr;
    std::function<bool()> returnKeyCallback = nullptr;
};
