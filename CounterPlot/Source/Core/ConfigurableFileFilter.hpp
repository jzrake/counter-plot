#pragma once
#include "JuceHeader.h"




//=============================================================================
class ConfigurableFileFilter : public FileFilter
{
public:

    //=========================================================================
    ConfigurableFileFilter();

    /**
     * Reset the filter to its initial state (all files are suitable).
     */
    void clear();

    /**
     * This function sets a list of file patterns to be considered suitable.
     * It may include wildcard characters, e.g. "*.png".
     */
    void setFilePatterns (StringArray patterns);

    /**
     * Adds a requirement that the file is HDF5, and contains a group at the
     * given location.
     */
    void requireHDF5Group (const String& groupThatMustExist);

    /**
     * Adds a requirement that the file is HDF5, and contains a dataset at the
     * given location with the given rank. If the rank is -1, then any rank
     * is considered suitable.
     */
    void requireHDF5Dataset (const String& datasetThatMustExist, int rank);

    //=========================================================================
    bool isFileSuitable (const File&) const override;
    bool isDirectorySuitable (const File&) const override;

private:

    //=========================================================================
    struct HDF5Requirements
    {
        String location;
        char type = 'g';
        int rank = -1;
    };

    WildcardFileFilter wildcardFilter;
    Array<HDF5Requirements> hdf5Requirements;
};
