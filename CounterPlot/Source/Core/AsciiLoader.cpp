#include <sstream>
#include "AsciiLoader.hpp"




// =============================================================================
AsciiLoader::AsciiLoader (std::istream& stream)
{
    int fileLine = 0;

    while (stream)
    {
        std::string line;
        std::getline (stream, line);
        std::istringstream iss (line);
        fileLine += 1;

        if (line[0] == '#' && data.empty())
        {
            /*
            * The first line is header information.
            */
            names = std::vector<std::string> {
                std::istream_iterator<std::string>(iss),
                std::istream_iterator<std::string>() };

            names.erase (names.begin());
            numColumns = names.size();
        }
        else if (line[0] != '#')
        {
            std::vector<double> row {
                std::istream_iterator<double>(iss),
                std::istream_iterator<double>() };

            if (names.empty() && data.empty())
            {
                numColumns = row.size();
            }

            if (names.empty())
            {
                for (int i = 0; i < numColumns; ++i)
                {
                    names.push_back ("Col " + std::to_string(i));
                }
            }

            if (row.size() == numColumns)
            {
                data.insert (data.end(), row.begin(), row.end());
                numRows += 1;
            }
            else if (! row.empty())
            {
                status = "Missing data on line " + std::to_string (fileLine);
            }
        }
    };
}

unsigned long AsciiLoader::getNumColumns() const
{
    return numColumns;
}

unsigned long AsciiLoader::getNumRows() const
{
    return numRows;
}

std::vector<double> AsciiLoader::getColumnData (int j) const
{
    std::vector<double> column;

    for (int i = 0; i < numRows; ++i)
    {
        column.push_back (data[i * numColumns + j]);
    }
    return column;
}

std::vector<double> AsciiLoader::getRowData (int i) const
{
    std::vector<double> row;

    for (int j = 0; j < numColumns; ++j)
    {
        row.push_back (data[i * numColumns + j]);
    }
    return row;
}

std::string AsciiLoader::getColumnName (int index) const
{
    return names.at (index);
}

std::string AsciiLoader::getStatusMessage() const
{
    return status;
}
