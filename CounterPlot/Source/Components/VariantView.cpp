#include "VariantView.hpp"
#include "../Core/LookAndFeel.hpp"




//=============================================================================
VariantView::Item::Item (const var& key, const var& data) : key (key), data (data)
{
    if (data.isArray())
    {
        int n = 0;

        for (const auto& child : *data.getArray())
        {
            addSubItem (new Item (n++, child));
        }
    }
    else if (auto obj = data.getDynamicObject())
    {
        for (const auto& item : obj->getProperties())
        {
            addSubItem (new Item (item.name.toString(), item.value));
        }
    }
    setDrawsInLeftMargin (true);
}

void VariantView::Item::paintItem (Graphics& g, int width, int height)
{
    g.setColour (LookAndFeelHelpers::findColourForPropertyText (*getOwnerView(), depth()));
    g.setFont (Font().withHeight (11));

    if (mightContainSubItems())
    {
        g.drawText (key.toString(), 8, 0, width, height, Justification::centredLeft);
    }
    else
    {
        g.drawText (key.toString() + " : " + data.toString(), 8, 0, width, height, Justification::centredLeft);
    }
}

bool VariantView::Item::mightContainSubItems()
{
    return data.isArray() || data.getDynamicObject() != nullptr;
}

bool VariantView::Item::canBeSelected() const
{
    return true;
}

int VariantView::Item::depth()
{
    int n = 0;
    TreeViewItem* item = this;
    while ((item = item->getParentItem())) ++n;
    return n;
}




//=========================================================================
VariantView::VariantView()
{
    tree.setIndentSize (12);
    tree.setColour (TreeView::backgroundColourId, Colours::darkgrey);
    tree.setDefaultOpenness (true);
    setColours();
    addAndMakeVisible (tree);
}

VariantView::~VariantView()
{
    tree.setRootItem (nullptr);
}

void VariantView::setData (const var &data)
{
    tree.setRootItem (nullptr);
    root = std::make_unique<Item> ("root", data);
    tree.setRootItem (root.get());
}

void VariantView::resized()
{
    tree.setBounds (getLocalBounds());
}

void VariantView::colourChanged()
{
    setColours();
    repaint();
}

void VariantView::lookAndFeelChanged()
{
    setColours();
    repaint();
}

void VariantView::setColours()
{
    tree.setColour (TreeView::backgroundColourId, findColour (LookAndFeelHelpers::propertyViewBackground));
    tree.setColour (TreeView::selectedItemBackgroundColourId, findColour (LookAndFeelHelpers::propertyViewSelectedItem));
    tree.setColour (TreeView::dragAndDropIndicatorColourId, Colours::green);
    tree.setColour (TreeView::evenItemsColourId, Colours::transparentBlack);
    tree.setColour (TreeView::oddItemsColourId, Colours::transparentBlack);
    tree.setColour (TreeView::linesColourId, Colours::red);
}
