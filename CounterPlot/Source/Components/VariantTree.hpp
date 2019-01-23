#pragma once
#include "JuceHeader.h"




//=============================================================================
class VariantTree : public Component
{
public:

    //=========================================================================
    class Item : public TreeViewItem
    {
    public:
        Item (const var& key, const var& data);
        void paintItem (Graphics& g, int width, int height) override;
        bool mightContainSubItems() override;
        bool canBeSelected() const override;
        int getItemHeight() const override;

    private:
        //=====================================================================
        int depth();
        var key, data;
    };

    //=========================================================================
    VariantTree();
    ~VariantTree();
    void setData (const var &data);
    void resized() override;
    void colourChanged() override;
    void lookAndFeelChanged() override;

private:
    //=========================================================================
    void setColours();
    std::unique_ptr<Item> root;
    TreeView tree;
};
