#include "ConfigurableFileFilter.hpp"




//=============================================================================
ConfigurableFileFilter::ConfigurableFileFilter() : FileFilter(""), wildcardFilter ("", "", "")
{
}

void ConfigurableFileFilter::setFilePatterns (StringArray patterns)
{
    wildcardFilter = WildcardFileFilter (patterns.joinIntoString (";"),
                                         patterns.joinIntoString (";"), "");
}




//=============================================================================
bool ConfigurableFileFilter::isFileSuitable (const File& file) const
{
    return wildcardFilter.isFileSuitable (file);
}

bool ConfigurableFileFilter::isDirectorySuitable (const File& file) const
{
    return wildcardFilter.isDirectorySuitable (file);
}
