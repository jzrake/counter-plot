#pragma once
#include "JuceHeader.h"
#include "FigureView.hpp"
#include "FileBasedView.hpp"




//=============================================================================
class BinaryTorquesViewFactory
{
public:
    class QuadmeshArtist;
    static FileBasedView* createNewVersion();
};
