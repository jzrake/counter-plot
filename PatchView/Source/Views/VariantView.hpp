#pragma once
#include "JuceHeader.h"




//=============================================================================
class VariantView : public Component
{
public:

    //=========================================================================
    class Item : public TreeViewItem
    {
    public:
        Item (const var& key, const var& data);
        void paintItem (Graphics& g, int width, int height) override;
        bool mightContainSubItems() override;

    private:
        //=====================================================================
        int depth();
        var key, data;
    };

    //=========================================================================
    VariantView();
    ~VariantView();
    void setData (const var &data);
    void resized() override;

private:
    //=========================================================================
    std::unique_ptr<Item> root;
    TreeView tree;
};
