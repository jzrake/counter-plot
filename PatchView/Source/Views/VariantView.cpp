#include "VariantView.hpp"




//=============================================================================
const static Colour palette1[5] =
{
    Colour::fromRGB (255, 198, 252),
    Colour::fromRGB (229, 181, 255),
    Colour::fromRGB (255, 242, 214),
    Colour::fromRGB (254, 240, 255),
    Colour::fromRGB (255, 255, 255),
};

const static Colour palette2[5] =
{
    Colour::fromRGB (162, 230, 244),
    Colour::fromRGB (255, 203, 237),
    Colour::fromRGB (243, 181, 255),
    Colour::fromRGB (167, 234, 225),
    Colour::fromRGB (251, 247, 156),
};




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
    g.setColour (palette2[depth() % 5]);
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
