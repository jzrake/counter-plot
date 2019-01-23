#!/bin/bash

rm -f Projucer.app
xcodebuild -project third_party/JUCE/extras/Projucer/Builds/MacOSX/Projucer.xcodeproj -configuration Release
ln -s third_party/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app .
