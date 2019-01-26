#pragma once
#include <vector>
#include <string>




// =============================================================================
class AsciiLoader
{
public:
    AsciiLoader (std::istream& stream);
    unsigned long getNumColumns() const;
    unsigned long getNumRows() const;
    std::vector<double> getColumnData (int index) const;
    std::vector<double> getRowData (int index) const;
    std::string getColumnName (int index) const;
    std::string getStatusMessage() const;

    template <class OutputIterator>
    void column (int j, OutputIterator iter)
    {
        for (int i = 0; i < numRows; ++i)
        {
            *iter++ = data[i * numColumns + j];
        }
    }
private:
    unsigned long numColumns = 0;
    unsigned long numRows = 0;
    std::vector<double> data;
    std::vector<std::string> names;
    std::string status;
};
