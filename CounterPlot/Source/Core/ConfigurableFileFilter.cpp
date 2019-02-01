#include "ConfigurableFileFilter.hpp"
#include "Main.hpp"




//=============================================================================
ConfigurableFileFilter::ConfigurableFileFilter() : FileFilter(""), wildcardFilter ("", "", "")
{
}

void ConfigurableFileFilter::clear()
{
    rejectAllFiles = false;
    wildcardFilter = WildcardFileFilter ("", "", "");
    hdf5Requirements.clear();
    pathches2dFieldRequirements.clear();
}

void ConfigurableFileFilter::setRejectsAllFiles (bool shouldRejectAllFiles)
{
    rejectAllFiles = shouldRejectAllFiles;
}

void ConfigurableFileFilter::setFilePatterns (StringArray patterns)
{
    if (patterns.isEmpty())
    {
        patterns = {"*"};
    }

    filePatterns = patterns;
    wildcardFilter = WildcardFileFilter (filePatterns.joinIntoString (";"),
                                         filePatterns.joinIntoString (";"), "");
}

void ConfigurableFileFilter::addFilePattern (const String& pattern)
{
    filePatterns.add (pattern);
    wildcardFilter = WildcardFileFilter (filePatterns.joinIntoString (";"),
                                         filePatterns.joinIntoString (";"), "");
}

void ConfigurableFileFilter::requireHDF5Group (const String& groupThatMustExist)
{
    hdf5Requirements.add ({ groupThatMustExist, 'g', -1 });
}

void ConfigurableFileFilter::requireHDF5Dataset (const String& datasetThatMustExist, int rank)
{
    hdf5Requirements.add ({ datasetThatMustExist, 'd', rank });
}

void ConfigurableFileFilter::requirePatches2dField (const String &fieldThatMustExist)
{
    pathches2dFieldRequirements.add (fieldThatMustExist);
}




//=============================================================================
bool ConfigurableFileFilter::isFileSuitable (const File& file) const
{
    return isPathSuitable (file) && wildcardFilter.isFileSuitable (file);
}

bool ConfigurableFileFilter::isDirectorySuitable (const File& file) const
{
    return isPathSuitable (file) && wildcardFilter.isDirectorySuitable (file);
}

bool ConfigurableFileFilter::isPathSuitable (const File& file) const
{
    if (rejectAllFiles)
    {
        return false;
    }

    for (const auto& requirement : hdf5Requirements)
    {
        ScopedLock lock (PatchViewApplication::getApp().getCriticalSectionForHDF5());

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

    for (const auto& requiredField : pathches2dFieldRequirements)
    {
        FileSystemSerializer ser (file);

        try {
            auto field = patches2d::parse_field (requiredField.toStdString());
            auto header = ser.read_header();

            if (header.count (field) == 0)
            {
                return false;
            }
        }
        catch (...)
        {
            return false;
        }
    }
    return true;
}
