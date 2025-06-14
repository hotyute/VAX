#pragma once
#include "widget.h"
#include "container_widget.h"
#include <vector>
#include <string>
#include <functional> // For std::function

class ButtonWidget;
class ListBoxWidget;

class ComboBoxWidget : public ContainerWidget {
public:
    std::function<void(ComboBoxWidget*, int /*selectedIndex*/, const std::string& /*selectedText*/)> OnSelectionChanged;

    ComboBoxWidget();

    void AddItem(const std::string& item);
    void AddItems(const std::vector<std::string>& items);
    void ClearItems();

    int GetSelectedIndex() const;
    std::string GetSelectedText() const;
    void SetSelectedIndex(int index);

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f) override;
    Size Measure() override;
    void HandleEvent(UIEvent& event) override;
    void OnFocusLost() override; // Keep this, might be useful for auto-closing dropdown

    // Static methods for UIManager interaction
    static ComboBoxWidget* GetActiveDropdownOwner_Static() { return activeDropdownOwner; }
    static void CloseActiveDropdown_Static() { // Renamed for clarity
        if (activeDropdownOwner) {
            activeDropdownOwner->ToggleDropdown(); // Toggle will close it
        }
    }
    // Method to get the currently open listbox (if any) of this specific instance
    ListBoxWidget* GetCurrentOpenDropdownList() const { return currentOpenDropdownListRawPtr; }

private:
    void ToggleDropdown();
    // void SelectItem(int index); // This was declared but not defined, SetSelectedIndex covers it.

    ButtonWidget* displayButton;
    // ListBoxWidget* dropdownList; // Old raw pointer, replace with currentOpen...
    ListBoxWidget* currentOpenDropdownListRawPtr = nullptr; // Tracks the currently open ListBox

    std::vector<std::string> items;
    int selectedIndex = -1;
    bool isDropdownVisible = false;

    // Prototype style and callback for creating new dropdowns
    Style dropdownListPrototypeStyle;
    std::function<void(ListBoxWidget*, int, const std::string&)> onItemSelectedCallbackForDropdown;


    static ComboBoxWidget* activeDropdownOwner;
};