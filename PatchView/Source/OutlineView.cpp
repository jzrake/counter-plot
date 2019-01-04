#include "OutlineView.hpp"


static int rowHeight = 22;
static int indent = 12;




//==============================================================================
struct OutlineItem
{
    var data;
    int row = 0;
    int level = 0;
};




//==============================================================================
void generateOutline (const var& data, Array<OutlineItem>& items, int level)
{
    if (data.isArray())
    {
        for (const auto& child : *data.getArray())
        {
            generateOutline (child, items, level + 1);
        }
    }
    else if (data.isString())
    {
        OutlineItem item;
        item.data = data;
        item.row = items.size();
        item.level = level;
        items.add (item);
    }
    else
    {
        jassertfalse;
    }
}

Array<OutlineItem> generateOutline (const var& root)
{
    Array<OutlineItem> res;
    generateOutline (root, res, 0);
    return res;
}




//==============================================================================
OutlineView::OutlineView()
{
    for (int n = 0; n < 20; ++n)
    {
        if (n % 4 == 0)
        {
            var node;
            node.append ("Child 1");
            node.append ("Child 2");
            node.append ("Child 3");
            model.append (node);
        }
        else
        {
            model.append (String ("Object" + String (n)));
        }
    }
}

void OutlineView::paint (Graphics& g)
{
    g.fillAll (Colours::brown);

    int width = getWidth();
    int y = 0;

    for (auto item : generateOutline (model))
    {
        if (item.row == selectedRow)
        {
            g.setColour (Colours::brown.darker());
            g.fillRect (0, y, width, rowHeight);
        }

        if (item.row == mouseOverRow)
        {
            g.setColour (findColour (Label::textColourId).withBrightness (1.f));
        }
        else
        {
            g.setColour (findColour (Label::textColourId));
        }

        g.drawText (item.data.toString(), indent * item.level, y, width, rowHeight, Justification::centredLeft);
        y += rowHeight;
    }
}

void OutlineView::resized()
{
}

void OutlineView::mouseMove (const MouseEvent &event)
{
    mouseOverRow = event.position.y / rowHeight;
    repaint();
}

void OutlineView::mouseExit (const MouseEvent &event)
{
    mouseOverRow = -1;
    repaint();
}

void OutlineView::mouseDown (const MouseEvent &event)
{
    selectedRow = event.position.y / rowHeight;
    auto items = generateOutline (model);

    if (items[selectedRow].data.isArray())
    {
        selectedRow = -1;
    }
    repaint();
}
