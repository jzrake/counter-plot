#pragma once
#include "JuceHeader.h"




//=============================================================================
struct TableModel
{

    //=========================================================================
    enum class Type
    {
        Int, Double, Time, String,
    };

    //=========================================================================
    struct Series
    {
        Series (const String& name, const Array<int>& data)    : name (name), integerData (data), type (Type::Int)    {}
        Series (const String& name, const Array<double>& data) : name (name), doubleData  (data), type (Type::Double) {}
        Series (const String& name, const Array<Time>& data)   : name (name), timeData    (data), type (Type::Time)   {}
        Series (const String& name, const StringArray& data)   : name (name), stringData  (data), type (Type::String) {}
        int size() const;

        String name;
        Array<int> integerData;
        Array<double> doubleData;
        Array<Time> timeData;
        StringArray stringData;
        Type type;
    };

    //=========================================================================
    int maxRows() const;

    //=========================================================================
    Array<Series> columns;
    int columnWidth = 100;
    int rowHeight = 22;
    int headerHeight = 26;
    int leftMarginWidth = 32;
    Font headerFont = Font().withHeight (12);
    Font numberFont = Font ("Menlo", 11, 0);
};




//=============================================================================
class TableView : public Component
{
public:

    enum ColourIds
    {
        backgroundColourId   = 0x0761201,
        headerCellColourId   = 0x0761202,
        selectedCellColourId = 0x0761203,
        gridlineColourId     = 0x0761204,
        textColourId         = 0x0761205,
    };

    //=========================================================================
    struct Geometry
    {
        Array<int> colEdges;
        Array<int> rowEdges;
        Rectangle<int> getCellArea (int i, int j) const;
    };

    //=========================================================================
    static void setLookAndFeelDefaults (LookAndFeel&);

    //=========================================================================
    TableView();
    void setModel (const TableModel& newModel);

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:

    //=========================================================================
    Geometry computeGeometry();
    Point<int> rowAndColumnAtPosition (Point<float> tablePosition);
    void paintGridlines (Graphics& g, const Geometry& geometry);
    void paintHeader (Graphics& g, const Geometry& geometry);
    void paintColumn (Graphics& g, const Geometry& geometry, int column, const TableModel::Series& data);

    TableModel model;
};
