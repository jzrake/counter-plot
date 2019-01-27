#include "TableView.hpp"




//=========================================================================
int TableModel::Series::size() const
{
    switch (type)
    {
        case Type::Int: return integerData.size();
        case Type::Double: return doubleData.size();
        case Type::Time: return timeData.size();
        case Type::String: return stringData.size();
    }
}

int TableModel::maxRows() const
{
    int max = 0;

    for (const auto& series : columns)
        max = jmax (max, series.size());
    return max;
}




//=========================================================================
Rectangle<int> TableView::Geometry::getCellArea (int i, int j) const
{
    return {
        colEdges[j],
        rowEdges[i],
        colEdges[j + 1] - colEdges[j],
        rowEdges[i + 1] - rowEdges[i],
    };
}




//=========================================================================
void TableView::setLookAndFeelDefaults (LookAndFeel& laf)
{
    laf.setColour (backgroundColourId, Colours::white);
    laf.setColour (headerCellColourId, Colours::whitesmoke);
    laf.setColour (selectedCellColourId, Colours::lightgrey);
    laf.setColour (gridlineColourId, Colours::lightgrey);
    laf.setColour (textColourId, Colours::black);
}




//=========================================================================
TableView::TableView()
{
    model.columns.add ({"Column 1", Array<double> {1.2, 2.256, 3.122, 4.22211147, 0.5, 6.123}});
    model.columns.add ({"Column 2", Array<double> {4.2, 2.256, 2.122, 3.22211147, 0.9, 6.123}});
    model.columns.add ({"Column 3", Array<double> {1.2, 2.256, 2.122,-1231.30000, 0.9, 6.123}});
}

void TableView::setModel (const TableModel& newModel)
{
    model = newModel;
    repaint();
}




//=========================================================================
void TableView::paint (Graphics& g)
{
    auto geometry = computeGeometry();

    g.setColour (findColour (ColourIds::backgroundColourId));
    g.fillAll();

    paintHeader (g, geometry);

    for (int j = 0; j < model.columns.size(); ++j)
        paintColumn (g, geometry, j, model.columns.getReference(j));

    paintGridlines (g, geometry);
}

void TableView::resized()
{
}



//=========================================================================
TableView::Geometry TableView::computeGeometry()
{
    Geometry g;
    g.colEdges.add(0);
    g.rowEdges.add(0);

    for (int j = 0; j < model.columns.size() + 1; ++j)
        g.colEdges.add (j * model.columnWidth + model.leftMarginWidth);

    for (int i = 0; i < model.maxRows() + 1; ++i)
        g.rowEdges.add (i * model.rowHeight + model.headerHeight);

    return g;
}

Point<int> TableView::rowAndColumnAtPosition (Point<float> tablePosition)
{
    int i = (tablePosition.y - model.headerHeight) / model.rowHeight;
    int j = (tablePosition.x - model.leftMarginWidth) / model.columnWidth;
    return {i, j};
}

void TableView::paintGridlines (Graphics& g, const Geometry& geometry)
{
    g.setColour (findColour (ColourIds::gridlineColourId));

    for (int j = 1; j < geometry.colEdges.size() - 1; ++j)
    {
        auto x = geometry.colEdges.getUnchecked(j);
        g.drawVerticalLine (x, 0, geometry.rowEdges.getLast());
    }

    for (int i = 1; i < geometry.rowEdges.size() - 1; ++i)
    {
        auto y = geometry.rowEdges.getUnchecked(i);
        g.drawHorizontalLine (y, 0, geometry.colEdges.getLast());
    }
}

void TableView::paintHeader (Graphics& g, const Geometry& geometry)
{
    g.setFont (model.headerFont);

    for (int j = 0; j < model.columns.size(); ++j)
    {
        auto area = geometry.getCellArea (0, j + 1);

        g.setColour (findColour (ColourIds::headerCellColourId));
        g.fillRect (area);

        g.setColour (findColour (ColourIds::textColourId));
        g.drawText (model.columns.getUnchecked(j).name, area, Justification::centred);
    }
}

void TableView::paintColumn (Graphics& g, const Geometry& geometry, int column, const TableModel::Series& data)
{
    g.setColour (findColour (ColourIds::textColourId));
    g.setFont (model.numberFont);

    for (int i = 0; i < data.size(); ++i)
    {
        auto area = geometry.getCellArea (i + 1, column + 1);
        g.drawText (String (data.doubleData.getUnchecked (i), 8), area, Justification::centred);
    }
}
