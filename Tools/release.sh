#!/bin/bash

xcodebuild -project CounterPlot/Builds/MacOSX/Counter\ Plot.xcodeproj -configuration Release
rm -rf Counter\ Plot.app
cp -r CounterPlot/Builds/MacOSX/build/Release/Counter\ Plot.app /Applications
cp -r CounterPlot/Builds/MacOSX/build/Release/Counter\ Plot.app ~/Dropbox/Work/BinaryTorques
