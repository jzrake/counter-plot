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
        Series (const String& name, const Array<double>& data);
        int size() const;
        const GlyphArrangement& getGlyphs (int i) const;

        String name;
        Array<int> integerData;
        Array<double> doubleData;
        Array<Time> timeData;
        StringArray stringData;
        bool selected = false;
        Type type;
        Array<GlyphArrangement> glyphsCache;
    };

    //=========================================================================
    int maxRows() const;

    //=========================================================================
    Array<Series> columns;
    int abscissa = 0;
    int columnWidth = 100;
    int rowHeight = 22;
    int headerHeight = 26;
    int gutterWidth = 32;
    Font headerFont = Font().withHeight (12);
    Font numberFont = Font ("Menlo", 11, 0);
    Point<float> scrollPosition;
};




//=============================================================================
class TableView : public Component
{
public:

    //=========================================================================
    class Controller
    {
    public:
        virtual ~Controller() {}
        virtual void tableViewMakeColumnAbscissa (TableView*, int column) = 0;
        virtual void tableViewSetColumnSelected (TableView*, int column, bool shouldBeSelected) = 0;
        virtual void tableViewSetScrollPosition (TableView*, Point<float> newScrollPosition) = 0;
    };

    //=========================================================================
    class DefaultController : public Controller
    {
    public:
        void tableViewMakeColumnAbscissa (TableView*, int column) override;
        void tableViewSetColumnSelected (TableView*, int column, bool shouldBeSelected) override;
        void tableViewSetScrollPosition (TableView*, Point<float> newScrollPosition) override;
    };

    //=========================================================================
    enum ColourIds
    {
        backgroundColourId   = 0x0761201,
        headerCellColourId   = 0x0761202,
        selectedCellColourId = 0x0761203,
        abscissaCellColourId = 0x0761204,
        gridlineColourId     = 0x0761205,
        textColourId         = 0x0761206,
    };

    //=========================================================================
    struct Geometry
    {
        Array<int> colEdges; // in table coordinates
        Array<int> rowEdges;
        Rectangle<int> getCellArea (int i, int j) const;
    };

    //=========================================================================
    struct Cell
    {
        int row, col;
    };

    //=========================================================================
    static void setLookAndFeelDefaults (LookAndFeel&);

    //=========================================================================
    TableView();
    void setModel (const TableModel& newModel);
    void setController (Controller* controllerToUse);
    const TableModel& getModel() const;

    //=========================================================================
    void paint (Graphics& g) override;
    void resized() override;
    void mouseDown (const MouseEvent&) override;
    void mouseMove (const MouseEvent&) override;
    void mouseExit (const MouseEvent&) override;
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;

private:

    //=========================================================================
    void paintGridlines (Graphics& g, const Geometry& geometry, char which);
    void paintHeaderShadow (Graphics &g, const Geometry &geometry);
    void paintHeader (Graphics& g, const Geometry& geometry);
    void paintColumn (Graphics& g, const Geometry& geometry, int j, const TableModel::Series& data);
    void paintGutter (Graphics& g, const Geometry& geometry);
    Geometry computeGeometry();
    Cell cellAtPosition (Point<float> pos);
    Point<float> tableToComponent (Point<float> tablePosition) const;
    Point<float> componentToTable (Point<float> componentPosition) const;
    bool isRowOnscreen (int row, const Geometry& geometry);
    bool isColOnscreen (int col, const Geometry& geometry);

    DefaultController defaultController;
    Controller* controller = &defaultController;
    TableModel model;
    Cell mouseOverCell;
//    Point<float> upperLeftOfTable;
};
