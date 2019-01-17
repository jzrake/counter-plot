#pragma once
#include "JuceHeader.h"




//=============================================================================
class ConfigurableFileFilter : public FileFilter
{
public:

    //=========================================================================
    ConfigurableFileFilter();
    void setFilePatterns (StringArray patterns);

    //=========================================================================
    bool isFileSuitable (const File&) const override;
    bool isDirectorySuitable (const File&) const override;

private:
    WildcardFileFilter wildcardFilter;
};
