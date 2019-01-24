#include "EditorKeyMappings.hpp"




// =============================================================================
bool EditorKeyMappings::keyPressed (const KeyPress& key, Component* component)
{
    TextEditor& ed = *dynamic_cast<TextEditor*> (component);

    if (key == KeyPress ('K', ModifierKeys::ctrlModifier, 0))
    {
        ed.moveCaretToEndOfLine (true);

        if (ed.getCaretPosition() == 0)
        {
            ed.deleteForwards (false);
            return true;
        }
        ed.copyToClipboard();
        ed.insertTextAtCaret (String());
        return true;
    }
    if (key == KeyPress ('Y', ModifierKeys::ctrlModifier, 0))
    {
        ed.pasteFromClipboard();
        return true;
    }
    if (key == KeyPress ('A', ModifierKeys::ctrlModifier, 0))
    {
        ed.moveCaretToStartOfLine (false);
        return true;
    }
    if (key == KeyPress ('E', ModifierKeys::ctrlModifier, 0))
    {
        ed.moveCaretToEndOfLine (false);
        return true;
    }
    if (key == KeyPress ('D', ModifierKeys::ctrlModifier, 0))
    {
        ed.deleteForwards (false);
        return true;
    }
    if (key == KeyPress ('N', ModifierKeys::ctrlModifier, 0) && ed.isMultiLine())
    {
        ed.moveCaretDown (key.getModifiers().isShiftDown());
        return true;
    }
    if (key == KeyPress ('P', ModifierKeys::ctrlModifier, 0) && ed.isMultiLine())
    {
        ed.moveCaretUp (key.getModifiers().isShiftDown());
        return true;
    }
    if (key == KeyPress (KeyPress::escapeKey, 0, 0))
    {
        return escapeKeyCallback && escapeKeyCallback();
    }
    if (key == KeyPress (KeyPress::returnKey, 0, 0) && ! ed.isMultiLine())
    {
        return returnKeyCallback && returnKeyCallback();
    }
    if (key == KeyPress (KeyPress::returnKey, ModifierKeys::commandModifier, 0) && ed.isMultiLine())
    {
        return returnKeyCallback && returnKeyCallback();
    }
    if (key == KeyPress ('L', ModifierKeys::commandModifier, 0))
    {
        ed.moveCaretToStartOfLine (false);
        ed.moveCaretToEndOfLine (true);
        return true;
    }
    return false;
}
