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
    if (i < 0 || i + 1 >= rowEdges.size() || j < 0 || j + 1 >= colEdges.size())
        return {};

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
    laf.setColour (selectedCellColourId, Colours::blue.withAlpha (0.1f));
    laf.setColour (abscissaCellColourId, Colours::green.withAlpha (0.1f));
    laf.setColour (gridlineColourId, Colours::lightgrey);
    laf.setColour (textColourId, Colours::black);
}




//=========================================================================
TableView::TableView()
{
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

    g.saveState();
    g.addTransform (AffineTransform::translation (upperLeftOfTable));

    for (int j = 0; j < model.columns.size(); ++j)
    {
        paintColumn (g, geometry, j, model.columns.getReference(j));
    }
    paintGutter (g, geometry);
    paintGridlines (g, geometry, 'h');

    g.restoreState();
    paintHeader (g, geometry);

    g.saveState();
    g.addTransform (AffineTransform::translation (upperLeftOfTable));
    paintGridlines (g, geometry, 'v');

    g.restoreState();
    paintHeaderShadow (g, geometry);
}

void TableView::resized()
{
}

void TableView::mouseDown (const MouseEvent& e)
{
    if (mouseOverCell.row == 0 && mouseOverCell.col != model.abscissa)
    {
        auto& column = model.columns.getReference (mouseOverCell.col - 1);

        if (e.mods.isPopupMenu())
        {
            auto menu = PopupMenu();

            menu.addItem (1, "Make Column Abscissa");
            menu.addItem (2, column.selected ? "Unselect Column" : "Select Column");

            switch (menu.show())
            {
                case 1: model.abscissa = mouseOverCell.col; column.selected = false; break;
                case 2: column.selected = ! column.selected; break;
            }
        }
        else
        {
            column.selected = ! column.selected;
        }
        repaint();
    }
}

void TableView::mouseMove (const MouseEvent& e)
{
    mouseOverCell = cellAtPosition (e.position);
    repaint();
}

void TableView::mouseExit (const MouseEvent& e)
{
    mouseOverCell = {-1, -1};
    repaint();
}

void TableView::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    float ymin = -model.maxRows() * model.rowHeight - model.headerHeight + getHeight();
    upperLeftOfTable.y += wheel.deltaY * getHeight();
    upperLeftOfTable.y = jlimit (ymin, 0.f, upperLeftOfTable.y);
    mouseOverCell = cellAtPosition (e.position);
    repaint();
}




//=========================================================================
void TableView::paintGridlines (Graphics& g, const Geometry& geometry, char which)
{
    g.setColour (findColour (ColourIds::gridlineColourId));

    if (which == 'v')
    {
        for (int j = 1; j < geometry.colEdges.size() - 1; ++j)
        {
            auto x = geometry.colEdges.getUnchecked(j);
            g.drawVerticalLine (x, 0, geometry.rowEdges.getLast());
        }
    }

    if (which == 'h')
    {
        for (int i = 1; i < geometry.rowEdges.size() - 1; ++i)
        {
            if (isRowOnscreen (i - 1, geometry) || isRowOnscreen (i, geometry))
            {
                auto y = geometry.rowEdges.getUnchecked(i);
                g.drawHorizontalLine (y, 0, geometry.colEdges.getLast());
            }
        }
    }
}

void TableView::paintHeaderShadow (Graphics &g, const Geometry &geometry)
{
    if (upperLeftOfTable.y < 0)
    {
        auto y1 = model.headerHeight;
        auto y2 = model.headerHeight + 16;
        auto rect = Rectangle<int>::leftTopRightBottom (geometry.colEdges[1], y1,
                                                        geometry.colEdges.getLast(), y2);
        ColourGradient grad (Colours::darkslateblue.withAlpha (0.2f), 0, y1,
                             Colours::darkslateblue.withAlpha (0.0f), 0, y2, false);
        g.setGradientFill (grad);
        g.fillRect (rect);
    }
}

void TableView::paintHeader (Graphics& g, const Geometry& geometry)
{
    g.setFont (model.headerFont);

    for (int j = 0; j < model.columns.size(); ++j)
    {
        auto area = geometry.getCellArea (0, j + 1);
        auto& column = model.columns.getReference(j);

        if (   mouseOverCell.row == 0
            && mouseOverCell.col == j + 1
            && model.abscissa    != j + 1)
            g.setColour (findColour (ColourIds::headerCellColourId).darker (0.1f));
        else
            g.setColour (findColour (ColourIds::headerCellColourId));

        g.fillRect (area);
        g.setColour (findColour (ColourIds::textColourId));
        g.drawText (column.name, area, Justification::centred);
    }
}

void TableView::paintColumn (Graphics& g, const Geometry& geometry, int j, const TableModel::Series& data)
{
    auto& column = model.columns.getReference(j);

    for (int i = 0; i < data.size(); ++i)
    {
        if (isRowOnscreen (i, geometry))
        {
            if (column.selected)
                g.setColour (findColour (ColourIds::selectedCellColourId));
            else if (model.abscissa == j + 1)
                g.setColour (findColour (ColourIds::abscissaCellColourId));
            else
                g.setColour (Colours::transparentBlack);

            auto area = geometry.getCellArea (i + 1, j + 1);

            g.fillRect (area);
            g.setColour (findColour (ColourIds::textColourId));
            g.setFont (model.numberFont);
            g.drawText (String (data.doubleData.getUnchecked (i), 8), area, Justification::centred);
        }
    }
}

void TableView::paintGutter (Graphics& g, const Geometry& geometry)
{
    for (int i = 1; i < model.maxRows() + 1; ++i)
    {
        if (isRowOnscreen (i - 1, geometry))
        {
            auto area = geometry.getCellArea (i, 0);

            g.setColour (findColour (ColourIds::headerCellColourId));
            g.fillRect (area);

            g.setColour (findColour (ColourIds::textColourId));
            g.drawText (String (i), area, Justification::centred);
        }
    }
}

TableView::Geometry TableView::computeGeometry()
{
    Geometry g;
    g.colEdges.add(0);
    g.rowEdges.add(0);

    for (int j = 0; j < model.columns.size() + 1; ++j)
        g.colEdges.add (j * model.columnWidth + model.gutterWidth);

    for (int i = 0; i < model.maxRows() + 1; ++i)
        g.rowEdges.add (i * model.rowHeight + model.headerHeight);

    return g;
}

TableView::Cell TableView::cellAtPosition (Point<float> pos)
{
    // If the mouse is over a header cell, we return {0, col}.
    // ------------------------------------------------------------------------------
    int col = 1 + (pos.x - model.gutterWidth) / model.columnWidth;

    if (pos.y < model.headerHeight && 1 <= col && col <= model.columns.size())
    {
        return {0, col};
    }

    // Otherwise, we return the index of the cell the mouse is in (row/col starting
    // from 1).
    // ------------------------------------------------------------------------------
    int i = 1 + (-upperLeftOfTable.y + pos.y - model.headerHeight) / model.rowHeight;
    int j = 1 + (-upperLeftOfTable.x + pos.x - model.gutterWidth) / model.columnWidth;
    return {i, j};
}

Point<float> TableView::tableToComponent (Point<float> tablePosition) const
{
    return tablePosition + upperLeftOfTable;
}

Point<float> TableView::componentToTable (Point<float> componentPosition) const
{
    return componentPosition - upperLeftOfTable;
}

bool TableView::isRowOnscreen (int row, const Geometry& geometry)
{
    auto y0 = geometry.rowEdges[row + 1] + upperLeftOfTable.y;
    auto y1 = geometry.rowEdges[row + 2] + upperLeftOfTable.y;
    return y1 > 0 && y0 < getHeight();
}

bool TableView::isColOnscreen (int col, const Geometry& geometry)
{
    return true;
}
