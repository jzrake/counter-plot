/******************************************************************************
BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_metal
  vendor:           Jonathan Zrake
  version:          0.0.1
  name:             Metal in JUCE
  description:      Provides a JUCE component to render content via Apple's Metal API
  website:          
  license:          GPL
  dependencies:     juce_gui_basics
  OSXFrameworks:    Metal.framework MetalKit.framework

END_JUCE_MODULE_DECLARATION
******************************************************************************/


#pragma once
#define JUCE_METAL_INCLUDED
#include <juce_metal/src/MetalComponent.hpp>
