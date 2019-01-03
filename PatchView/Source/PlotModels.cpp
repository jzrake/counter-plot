#include "PlotModels.hpp"




//==============================================================================
Rectangle<int> FigureModel::getTopMargin (const Rectangle<int>& area) const
{
    return {
        margin.getLeft(),
        0,
        area.getRight() - margin.getLeftAndRight(),
        margin.getTop()
    };
}

Rectangle<int> FigureModel::getBottomMargin (const Rectangle<int>& area) const
{
    return {
        margin.getLeft(),
        area.getBottom() - margin.getBottom(),
        area.getRight() - margin.getLeftAndRight(),
        margin.getBottom()
    };
}

Rectangle<int> FigureModel::getLeftMargin (const Rectangle<int>& area) const
{
    return {
        0,
        margin.getTop(),
        margin.getLeft(),
        area.getBottom() - margin.getTopAndBottom()
    };
}

Rectangle<int> FigureModel::getRightMargin (const Rectangle<int>& area) const
{
    return {
        area.getRight() - margin.getRight(),
        margin.getTop(),
        margin.getRight(),
        area.getBottom() - margin.getTopAndBottom()
    };
}

Rectangle<double> FigureModel::getDomain() const
{
    return Rectangle<double>(xmin, ymin, xmax - xmin, ymax - ymin);
}
