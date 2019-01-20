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
     * Whether the filter should just reject everything.
     */
    void setRejectsAllFiles (bool shouldRejectAllFiles);


    /**
     * Sets a list of file patterns to be considered suitable.
     */
    void setFilePatterns (StringArray patterns);


    /**
     * Add a requirement that the file is HDF5, and contains a group at the
     * given location.
     */
    void requireHDF5Group (const String& groupThatMustExist);


    /**
     * Add a requirement that the file is HDF5, and contains a dataset at the
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
    bool rejectAllFiles = false;
};
