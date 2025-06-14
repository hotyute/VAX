#pragma once
#include "container_widget.h"
#include "list_box_widget.h"
#include "scrollbar_widget.h"
#include "layout_strategy.h"

class ScrollableListBoxWidget : public ContainerWidget {
public:
    ScrollableListBoxWidget();

    void AddItem(const std::string& item);
    void ClearItems();
    int GetSelectedIndex() const;       // Exposes ListBox's selected index
    std::string GetSelectedText() const; // Exposes ListBox's selected text
    void SetSelectedIndex(int index);   // Sets ListBox's index and ensures visibility

    std::function<void(ListBoxWidget*, int, const std::string&)> OnItemSelected;

    void Arrange(const Rect& finalRect) override;
    void HandleEvent(UIEvent& event) override;
    Size Measure() override;

    ListBoxWidget* GetListBox() const { return listBox; } // For ComboBox to access the internal list

private:
    void UpdateScrollBarParameters();
    void OnInternalScroll(ScrollBarWidget* sb, int newValue);

    ListBoxWidget* listBox;
    ScrollBarWidget* scrollBar;
};