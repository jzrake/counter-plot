#include "TableView.hpp"
#include "../Core/DataHelpers.hpp"
#include "../Core/Runtime.hpp"
#define PROFILE_PAINT 1




//=========================================================================
static GlyphArrangement computeGlyphs (const String& text, int width)
{
    static std::map<std::pair<int, String>, GlyphArrangement> cache;
    auto font = Font ("Menlo", 11, 0);
    auto key  = std::make_pair (width, text);

    if (cache.count (key))
    {
        return cache.at (key);
    }

    auto glyphs = GlyphArrangement();
    glyphs.addLineOfText (font, text, 0, 0);
    glyphs.justifyGlyphs (0, text.length(), 0, 0, width, 22, Justification::centred);
    cache[key] = glyphs;

    if (cache.size() > 10000)
    {
        cache.clear();
    }
    return glyphs;

}




//=========================================================================
TableModel::Series::Series (const String& name, nd::array<double, 1> data)
: name (name)
, doubleData (data)
, type (Type::Double)
{
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
    auto model = TableModel();
    auto columns = DataHelpers::makeDictFromList (value["columns"], "Column ");

    for (const auto& item : columns.getDynamicObject()->getProperties())
        if (item.value.isArray())
            model.add (item.name.toString(), DataHelpers::ndarrayDouble1FromVar (item.value));
        else if (auto data = Runtime::opt_data<nd::array<double, 1>> (item.value))
            model.add (item.name.toString(), *data);

    model.scrollPosition.x = 0.f;
    model.scrollPosition.y = value["scroll-position"];
    model.abscissa = int (value["abscissa"]) + 1;

    for (int n = 0; n < value["selected"].size(); ++n)
    {
        int col = int (value["selected"][n]) + 1;

        if (1 <= col && col <= model.columns.size())
        {
            model.columns.getReference (col - 1).selected = true;
        }
    }
    return model;
}

GlyphArrangement TableModel::getGlyphs (int i, int j) const
{
    auto font = Font ("Menlo", 11, 0);

    if (i < columns.getReference(j).doubleData.size())
    {
        auto datum = columns.getReference(j).doubleData(i);
        auto text = String (datum, 8);
        return computeGlyphs (text, columnWidth);
    }
    return {};
}

Array<int> TableModel::getSelectedColumnIndexes() const
{
    Array<int> result;
    int j = 0;

    for (const auto& column : columns)
    {
        if (column.selected)
        {
            result.add (j);
        }
        ++j;
    }
    return result;
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
void TableView::setLookAndFeelDefaults (LookAndFeel& laf, ColourScheme scheme)
{
    switch (scheme)
    {
        case ColourScheme::dark:
            laf.setColour (backgroundColourId,   Colour::fromFloatRGBA (0.15f, 0.15f, 0.16f, 1.00f));
            laf.setColour (headerCellColourId,   Colour::fromFloatRGBA (0.10f, 0.10f, 0.12f, 1.00f));
            laf.setColour (selectedCellColourId, Colour::fromFloatRGBA (0.00f, 0.00f, 1.00f, 0.10f));
            laf.setColour (abscissaCellColourId, Colour::fromFloatRGBA (1.00f, 0.00f, 0.00f, 0.10f));
            laf.setColour (gridlineColourId,     Colour::fromFloatRGBA (0.20f, 0.20f, 0.25f, 1.00f));
            laf.setColour (textColourId,         Colour::fromFloatRGBA (0.65f, 0.60f, 0.60f, 1.00f));
            break;
        case ColourScheme::light:
            laf.setColour (backgroundColourId, Colours::white);
            laf.setColour (headerCellColourId, Colours::whitesmoke);
            laf.setColour (selectedCellColourId, Colours::blue.withAlpha (0.1f));
            laf.setColour (abscissaCellColourId, Colours::green.withAlpha (0.1f));
            laf.setColour (gridlineColourId, Colours::lightgrey);
            laf.setColour (textColourId, Colours::black);
            break;
    }
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
    auto start = Time::getMillisecondCounterHiRes();

    auto geometry = computeGeometry();
    g.setColour (findColour (ColourIds::backgroundColourId));
    g.fillAll();

    g.saveState();
    g.addTransform (AffineTransform::translation (model.scrollPosition));

    for (int j = 0; j < model.columns.size(); ++j)
    {
        paintColumn (g, geometry, j, model.columns.getReference(j));
    }
    paintGutter (g, geometry);
    paintGridlines (g, geometry, 'h');

    g.restoreState();
    paintHeader (g, geometry);

    g.saveState();
    g.addTransform (AffineTransform::translation (model.scrollPosition));
    paintGridlines (g, geometry, 'v');

    g.restoreState();
    paintHeaderShadow (g, geometry);

    if (PROFILE_PAINT)
    {
        auto paintTime = Time::getMillisecondCounterHiRes() - start;
        g.setColour (Colours::ghostwhite);
        g.setFont (model.numberFont);
        g.drawText (String (1e3 / paintTime, 4) + " fps",
                    getLocalBounds().removeFromTop (model.headerHeight).removeFromRight (100).toFloat(),
                    Justification::centredRight);
    }
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
        }
    }
    else
    {
        mouseDownScrollPosition = model.scrollPosition;
    }
}

void TableView::mouseDrag (const MouseEvent& e)
{
    if (mouseOverCell.col == 0)
    {
        auto targetY = mouseDownScrollPosition.y + e.getDistanceFromDragStartY();
        auto ymin = float (-model.maxRows() * model.rowHeight - model.headerHeight + getHeight());
        auto newy = jlimit (jmin (0.f, ymin), 0.f, targetY);
        controller->tableViewSetScrollPosition (this, Point<float> (0, newy));
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
    auto targetY = model.scrollPosition.y + wheel.deltaY * getHeight();
    auto ymin = float (-model.maxRows() * model.rowHeight - model.headerHeight + getHeight());
    auto newy = jlimit (jmin (0.f, ymin), 0.f, targetY);
    controller->tableViewSetScrollPosition (this, Point<float> (0, newy));
    mouseOverCell = cellAtPosition (e.position);
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
        ColourGradient grad (Colours::darkslateblue.withAlpha (0.3f), 0, y1,
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
            g.setColour (findColour (ColourIds::headerCellColourId).brighter (0.05f));
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

    for (int i = 0; i < model.maxRows(); ++i)
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

    for (int i = 0; i < model.maxRows(); ++i)
    {
        if (isRowOnscreen (i, geometry))
        {
            auto area = geometry.getCellArea (i + 1, 0);
            auto transform = AffineTransform::translation (area.getX(), area.getY());

            g.setColour (findColour (ColourIds::headerCellColourId));
            g.fillRect (area);

            g.setColour (findColour (ColourIds::textColourId));
            computeGlyphs (String (i + 1), model.gutterWidth).draw (g, transform);
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
