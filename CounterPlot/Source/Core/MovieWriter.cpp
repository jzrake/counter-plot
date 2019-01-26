#include "MovieWriter.hpp"




//=============================================================================
void FFMpegMovieWriter::setFFMpegExecutable (File pathToFFMpeg)
{
	ffmpeg = pathToFFMpeg;
}

void FFMpegMovieWriter::setFrameRate (int frameRateToUse)
{
	frameRate = frameRateToUse;
}

void FFMpegMovieWriter::writeImagesToFile (const Array<Image>& images, File outputMovieFile)
{
	auto temp = File::getSpecialLocation (File::tempDirectory);
    auto n = 0;

    for (const auto& image : images)
    {
        char frameFileName[256];
        std::snprintf (frameFileName, 256, "frame.%08d.png", n);

        auto frameFile = temp.getChildFile (frameFileName);
        auto stream = std::unique_ptr<FileOutputStream> (frameFile.createOutputStream());
        auto format = PNGImageFormat();
        format.writeImageToStream (image, *stream);
        ++n;
    }

    StringArray args = {
        ffmpeg.getFullPathName(),
        "-r", String (frameRate),
        "-f", "image2",
        "-i", temp.getChildFile ("frame.%08d.png").getFullPathName(),
        "-vcodec", "libx264",
        "-pix_fmt", "yuv420p",
        "-crf", "25",
        "-y", outputMovieFile.getFullPathName(),
    };

    ChildProcess process;
    process.start (args);
    process.waitForProcessToFinish (10000);
}
