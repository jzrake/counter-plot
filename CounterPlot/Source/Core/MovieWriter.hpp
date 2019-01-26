#pragma once
#include "JuceHeader.h"




//=============================================================================
class FFMpegMovieWriter
{
public:
    void setFFMpegExecutable (File pathToFFMpeg);
    void setFrameRate (int frameRateToUse);
    void writeImagesToFile (const Array<Image>& images, File outputMovieFile);

private:
    File ffmpeg = String ("/usr/local/bin/ffmpeg");
    int frameRate = 12;
};
