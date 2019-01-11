#pragma once
#include "JuceHeader.h"
#include "FileBasedView.hpp"




//=============================================================================
class BinaryTorques
{
public:
    class QuadmeshArtist;
    class TriangleVertexData;
    static TriangleVertexData loadTriangleDataFromFile (File file, std::function<bool()> bailout);
    static FileBasedView* create();
};
