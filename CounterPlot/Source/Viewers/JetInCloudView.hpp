#pragma once
#include "JuceHeader.h"
#include "Viewer.hpp"




//=============================================================================
class JetInCloud
{
public:
    class QuadmeshArtist;
    class TriangleVertexData;
    static TriangleVertexData loadTriangleDataFromFile (File file, std::function<bool()> bailout);
    static Viewer* create();
};
