#include "TableView.hpp"
#include "../Core/DataHelpers.hpp"




//=========================================================================
TableModel::Series::Series (const String& name, nd::array<double, 1> data)
: name (name)
, doubleData (data)
, type (Type::Double)
{
    Font font ("Menlo", 11, 0);

    for (auto datum : data)
    {
        auto glyphs = GlyphArrangement();
        auto text = String (datum, 8);

        glyphs.addLineOfText (font, text, 0, 0);
        glyphs.justifyGlyphs (0, text.length(), 0, 0, 100, 22, Justification::centred);
        glyphsCache.add (glyphs);
    }
}

int TableModel::Series::size() const
{
    switch (type)
    {
        case Type::Double: return int (doubleData.size());
    }
}

void TableModel::add (const String& name, nd::array<double, 1> data)
{
    columns.add ({name, data});
}




//=========================================================================
TableModel TableModel::fromVar (const var& value)
{
    TableModel model;

    if (auto columns = value["columns"].getArray())
    {
        int n = 0;

        for (auto column : *columns)
        {
            auto data = DataHelpers::ndarrayDouble1FromVar (column);
            model.add ("Column " + String (n++), data);
        }
    }
    else if (auto columns = value["columns"].getDynamicObject())
    {
        for (const auto& item : columns->getProperties())
        {
            auto data = DataHelpers::ndarrayDouble1FromVar (item.value);
            model.add (item.name.toString(), data);
        }
    }
    return model;
}

const GlyphArrangement& TableModel::getGlyphs (int i, int j) const
{
    return columns.getReference(j).glyphsCache.getReference(i);
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
void TableView::DefaultController::tableViewMakeColumnAbscissa (TableView* table, int column)
{
    auto model = table->getModel();
    model.abscissa = column;

    if (column != 0)
        model.columns.getReference (column - 1).selected = false;

    table->setModel (model);
}

void TableView::DefaultController::tableViewSetColumnSelected (TableView* table, int column, bool shouldBeSelected)
{
    auto model = table->getModel();

    if (model.abscissa == column)
        model.abscissa = 0;

    model.columns.getReference (column - 1).selected = shouldBeSelected;
    table->setModel (model);
}

void TableView::DefaultController::tableViewSetScrollPosition (TableView* table, Point<float> newScrollPosition)
{
    auto model = table->getModel();
    model.scrollPosition = newScrollPosition;
    table->setModel (model);
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

void TableView::setController (Controller* controllerToUse)
{
    controller = controllerToUse ? controllerToUse : &defaultController;
}

const TableModel& TableView::getModel() const
{
    return model;
}




//=========================================================================
void TableView::paint (Graphics& g)
{
    auto geometry = computeGeometry();
    g.setColour (findColour (ColourIds::backgroundColourId));
    g.fillAll();

    g.saveState();
    g.addTransform (AffineTransform::translation (model.scrollPosition));

    for (int j = 0; j < model.columns.size(); ++j)
    {
        paintColumn (g, geometry, j, model.columns.getReference(j));
    }
    paintGutter (g, geometry); // NOTE: cache glyphs for perf
    paintGridlines (g, geometry, 'h');

    g.restoreState();
    paintHeader (g, geometry);

    g.saveState();
    g.addTransform (AffineTransform::translation (model.scrollPosition));
    paintGridlines (g, geometry, 'v');

    g.restoreState();
    paintHeaderShadow (g, geometry);
}

void TableView::resized()
{
}

void TableView::mouseDown (const MouseEvent& e)
{
    if (mouseOverCell.col != 0)
    {
        auto c = mouseOverCell.col;
        auto& column = model.columns.getReference (c - 1);

        if (mouseOverCell.row == 0)
        {
            if (e.mods.isPopupMenu())
            {
                auto menu = PopupMenu();
                menu.addItem (c == model.abscissa ? 1 : 2, "Abscissa", true, c == model.abscissa);
                menu.addItem (column.selected     ? 3 : 4, "Selected", c != model.abscissa, column.selected);

                switch (menu.show())
                {
                    case 1: controller->tableViewMakeColumnAbscissa (this, 0); break;
                    case 2: controller->tableViewMakeColumnAbscissa (this, c); break;
                    case 3: controller->tableViewSetColumnSelected (this, c, false); break;
                    case 4: controller->tableViewSetColumnSelected (this, c, true); break;
                }
            }
            else if (c != model.abscissa)
            {
                controller->tableViewSetColumnSelected (this, c, ! column.selected);
            }
            repaint();
        }
    }
}

void TableView::mouseMove (const MouseEvent& e)
{
    mouseOverCell = cellAtPosition (e.position);
    repaint();
}

void TableView::mouseExit (const MouseEvent& e)
{
    mouseOverCell = {0, 0};
    repaint();
}

void TableView::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    auto dy = wheel.deltaY * getHeight();
    auto ymin = float (-model.maxRows() * model.rowHeight - model.headerHeight + getHeight());
    Point<float> newScrollPosition = {0.f, jlimit (jmin (0.f, ymin), 0.f, model.scrollPosition.y + dy)};
    controller->tableViewSetScrollPosition (this, newScrollPosition);
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
    if (model.scrollPosition.y < 0)
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
            && mouseOverCell.col == j + 1)
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
            auto area = geometry.getCellArea (i + 1, j + 1);
            auto transform = AffineTransform::translation (area.getX(), area.getY());

            if (column.selected)
            {
                g.setColour (findColour (ColourIds::selectedCellColourId));
                g.fillRect (area);
            }
            else if (model.abscissa == j + 1)
            {
                g.setColour (findColour (ColourIds::abscissaCellColourId));
                g.fillRect (area);
            }

            g.setColour (findColour (ColourIds::textColourId));
            model.getGlyphs (i, j).draw (g, transform);
        }
    }
}

void TableView::paintGutter (Graphics& g, const Geometry& geometry)
{
    g.setFont (model.numberFont);

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
    // -----------------------------------------------------------
    {
        int col = 1 + (pos.x - model.gutterWidth) / model.columnWidth;

        if (pos.y < model.headerHeight && 1 <= col && col <= model.columns.size())
        {
            return {0, col};
        }
    }

    // Otherwise, we return the index of the cell the mouse is
    // in (row/col starting from 1).
    // -----------------------------------------------------------
    {
        int row = 1 + (-model.scrollPosition.y + pos.y - model.headerHeight) / model.rowHeight;
        int col = 1 + (-model.scrollPosition.x + pos.x - model.gutterWidth) / model.columnWidth;
        return col <= model.columns.size() ? Cell {row, col} : Cell {0, 0};
    }
}

Point<float> TableView::tableToComponent (Point<float> tablePosition) const
{
    return tablePosition + model.scrollPosition;
}

Point<float> TableView::componentToTable (Point<float> componentPosition) const
{
    return componentPosition - model.scrollPosition;
}

bool TableView::isRowOnscreen (int row, const Geometry& geometry)
{
    auto y0 = geometry.rowEdges[row + 1] + model.scrollPosition.y;
    auto y1 = geometry.rowEdges[row + 2] + model.scrollPosition.y;
    return y1 > 0 && y0 < getHeight();
}

bool TableView::isColOnscreen (int col, const Geometry& geometry)
{
    return true;
}
