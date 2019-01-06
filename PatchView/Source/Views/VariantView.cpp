#include "VariantView.hpp"
#include "LookAndFeel.hpp"




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
    g.setColour (LookAndFeelHelpers::findTextColour (*getOwnerView(), depth()));
    g.drawText (key.toString() + " : " + data.toString(), 0, 0, width, height, Justification::centredLeft);
}

bool VariantView::Item::mightContainSubItems()
{
    return data.isArray() || data.getDynamicObject() != nullptr;
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
    tree.setColour (TreeView::backgroundColourId, Colours::darkgrey);
    tree.setDefaultOpenness (true);
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
