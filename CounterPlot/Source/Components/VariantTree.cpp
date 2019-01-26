#include "VariantTree.hpp"
#include "../Core/LookAndFeel.hpp"




//=============================================================================
VariantTree::Item::Item (const var& key, const var& data) : key (key), data (data)
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

void VariantTree::Item::paintItem (Graphics& g, int width, int height)
{
    Font font;

    if (auto laf = dynamic_cast<AppLookAndFeel*> (&getOwnerView()->getLookAndFeel()))
    {
        font = laf->getDefaultFont();
    }

    g.setColour (AppLookAndFeel::findColourForPropertyText (*getOwnerView(), depth()));
    g.setFont (font);

    if (mightContainSubItems())
    {
        g.drawText (key.toString(), 0, 0, width, height, Justification::centredLeft);
    }
    else
    {
        g.drawText (key.toString() + " : " + data.toString(), 0, 0, width, height, Justification::centredLeft);
    }
}

bool VariantTree::Item::mightContainSubItems()
{
    return data.isArray() || data.getDynamicObject() != nullptr;
}

bool VariantTree::Item::canBeSelected() const
{
    return true;
}

int VariantTree::Item::getItemHeight() const
{
    if (auto laf = dynamic_cast<AppLookAndFeel*> (&getOwnerView()->getLookAndFeel()))
    {
        return laf->getDefaultFont().getHeight() * 11 / 5;
    }
    return 24;
}

int VariantTree::Item::depth()
{
    int n = 0;
    TreeViewItem* item = this;
    while ((item = item->getParentItem())) ++n;
    return n;
}




//=========================================================================
VariantTree::VariantTree()
{
    tree.setIndentSize (20);
    tree.setColour (TreeView::backgroundColourId, Colours::darkgrey);
    tree.setDefaultOpenness (true);
    tree.getViewport()->setWantsKeyboardFocus (false);
    setColours();
    addAndMakeVisible (tree);
}

VariantTree::~VariantTree()
{
    tree.setRootItem (nullptr);
}

void VariantTree::setData (const var &data)
{
    tree.setRootItem (nullptr);
    root = std::make_unique<Item> ("root", data);
    tree.setRootItem (root.get());
}

void VariantTree::resized()
{
    tree.setBounds (getLocalBounds());
}

void VariantTree::colourChanged()
{
    setColours();
    repaint();
}

void VariantTree::lookAndFeelChanged()
{
    if (root)
        root->treeHasChanged();
    setColours();
    repaint();
}

void VariantTree::setColours()
{
    tree.setColour (TreeView::backgroundColourId, findColour (AppLookAndFeel::propertyViewBackground));
    tree.setColour (TreeView::selectedItemBackgroundColourId, findColour (AppLookAndFeel::propertyViewSelectedItem));
    tree.setColour (TreeView::dragAndDropIndicatorColourId, Colours::green);
    tree.setColour (TreeView::evenItemsColourId, Colours::transparentBlack);
    tree.setColour (TreeView::oddItemsColourId, Colours::transparentBlack);
    tree.setColour (TreeView::linesColourId, Colours::red);
}
