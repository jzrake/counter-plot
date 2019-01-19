#include "ConfigurableFileFilter.hpp"
#include "../Core/DataHelpers.hpp"




//=============================================================================
ConfigurableFileFilter::ConfigurableFileFilter() : FileFilter(""), wildcardFilter ("", "", "")
{
}

void ConfigurableFileFilter::clear()
{
    wildcardFilter = WildcardFileFilter ("", "", "");
    hdf5Requirements.clear();
}

void ConfigurableFileFilter::setFilePatterns (StringArray patterns)
{
    if (patterns.isEmpty())
    {
        patterns = {"*"};
    }
    wildcardFilter = WildcardFileFilter (patterns.joinIntoString (";"),
                                         patterns.joinIntoString (";"), "");
}

void ConfigurableFileFilter::requireHDF5Group (const String& groupThatMustExist)
{
    hdf5Requirements.add ({ groupThatMustExist, 'g', -1 });
}

void ConfigurableFileFilter::requireHDF5Dataset (const String& datasetThatMustExist, int rank)
{
    hdf5Requirements.add ({ datasetThatMustExist, 'd', rank });
}




//=============================================================================
bool ConfigurableFileFilter::isFileSuitable (const File& file) const
{
    ScopedLock lock (DataHelpers::getCriticalSectionForHDF5());

    for (auto requirement : hdf5Requirements)
    {
        if (! h5::File::exists (file.getFullPathName().toStdString()))
        {
            return false;
        }
        auto h5f = h5::File (file.getFullPathName().toStdString(), "r");

        if (requirement.type == 'g')
        {
            try {
                h5f.open_group (requirement.location.toStdString());
            }
            catch (const std::exception& e)
            {
                return false;
            }
        }
        else if (requirement.type == 'd')
        {
            try {
                auto h5d = h5f.open_dataset (requirement.location.toStdString());

                if (requirement.rank != -1 && requirement.rank != h5d.get_space().rank())
                {
                    return false;
                }
            }
            catch (const std::exception& e)
            {
                return false;
            }
        }
    }
    return wildcardFilter.isFileSuitable (file);
}

bool ConfigurableFileFilter::isDirectorySuitable (const File& file) const
{
    return wildcardFilter.isDirectorySuitable (file);
}
