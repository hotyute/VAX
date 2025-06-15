#define NOMINMAX
#include <windows.h>

#include "combo_box_widget.h"
#include "button_widget.h"   // Real ButtonWidget
#include "scrollable_list_box_widget.h"
#include "list_box_widget.h" // Real ListBoxWidget
#include "ui_manager.h"
#include "ui_renderer.h"
#include "style.h"
#include "window_widget.h" // For creating dropdown as a temporary window

ComboBoxWidget* ComboBoxWidget::activeDropdownOwner = nullptr;

ComboBoxWidget::ComboBoxWidget() : ContainerWidget() {
    style = Style::DefaultInputField();
    focusable = true;

    displayButton = new ButtonWidget();
    displayButton->style = style;
    displayButton->style.padding = { 2, 20, 2, 2 };
    displayButton->OnClick = [this](Widget*) { ToggleDropdown(); };
    AddChild(std::unique_ptr<ButtonWidget>(displayButton));

    // Initialize prototype style for the dropdown list
    dropdownListPrototypeStyle = Style::DefaultInputField(); // Or a more specific ListBox style
    dropdownListPrototypeStyle.backgroundColor = { 0.95f, 0.95f, 0.95f, 1.0f };
    dropdownListPrototypeStyle.borderColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    dropdownListPrototypeStyle.hasBorder = true; // ListBox usually has its own border
    dropdownListPrototypeStyle.padding = { 2,2,2,2 }; // Internal padding for ListBox items

    // Initialize the callback for items selected in the dropdown
    onItemSelectedCallbackForDropdown = [this](ListBoxWidget* lb, int index, const std::string& text) {
        this->SetSelectedIndex(index);
        // ToggleDropdown(); // This will be called by SetSelectedIndex if it effectively closes the dropdown
        };

    // currentOpenDropdownListRawPtr is already initialized to nullptr
}

void ComboBoxWidget::AddItem(const std::string& item) {
    items.push_back(item);
    if (selectedIndex == -1 && !items.empty()) {
        SetSelectedIndex(0);
    }
    MarkDirty();
}
void ComboBoxWidget::AddItems(const std::vector<std::string>& new_items) {
    for(const auto& item : new_items) AddItem(item);
}


void ComboBoxWidget::ClearItems() {
    items.clear();
    selectedIndex = -1;
    displayButton->SetText(" ");
    MarkDirty();
}

int ComboBoxWidget::GetSelectedIndex() const {
    return selectedIndex;
}

std::string ComboBoxWidget::GetSelectedText() const {
    if (selectedIndex >= 0 && selectedIndex < (int)items.size()) {
        return items[selectedIndex];
    }
    return "";
}

void ComboBoxWidget::SetSelectedIndex(int index) {
    // Make sure selectedIndex can be set to -1 correctly even if items is not empty
    if (index >= -1 && index < (int)items.size()) {
        if (selectedIndex != index) {
            selectedIndex = index;
            if (selectedIndex != -1) {
                displayButton->SetText(items[selectedIndex]);
            }
            else {
                displayButton->SetText(" "); // Or placeholder
            }
            MarkDirty();
            if (OnSelectionChanged) {
                OnSelectionChanged(this, selectedIndex, GetSelectedText());
            }
        }
    }
    else if (items.empty() && index == -1) {
        if (selectedIndex != -1) {
            selectedIndex = -1;
            displayButton->SetText(" ");
            MarkDirty();
            if (OnSelectionChanged) {
                OnSelectionChanged(this, selectedIndex, GetSelectedText());
            }
        }
    }
    else if (!items.empty() && index >= (int)items.size()) {
        SetSelectedIndex((int)items.size() - 1); // Recursive call to select last valid
        return; // Avoid duplicate logic
    }
    else if (!items.empty() && index < -1) { // index < -1, e.g. -2
        SetSelectedIndex(0); // Recursive call to select first valid
        return; // Avoid duplicate logic
    }
    // If dropdown is visible and an item was selected, close it.
    // The original OnItemSelected in constructor already called ToggleDropdown.
    // If SetSelectedIndex is called externally, we might also want to close.
    // For now, relying on OnItemSelected from ListBox to trigger close.
}

void ComboBoxWidget::ToggleDropdown() {
    UIManager& manager = UIManager::Instance();

    if (isDropdownVisible) { // Logic for CLOSING the dropdown
        if (activeDropdownOwner == this && this->currentOpenDropdownListRawPtr &&
            this->currentOpenDropdownListRawPtr->parent && // Parent is ScrollableListBox
            this->currentOpenDropdownListRawPtr->parent->parent && // Grandparent is ContentArea
            this->currentOpenDropdownListRawPtr->parent->parent->parent) { // Great-grandparent is WindowWidget

            WindowWidget* windowToRemove = dynamic_cast<WindowWidget*>(this->currentOpenDropdownListRawPtr->parent->parent->parent);

            if (windowToRemove) {
                manager.RemoveWindow(windowToRemove); // This will delete the window and its children
            }
            this->currentOpenDropdownListRawPtr = nullptr; // Important: clear the raw pointer
            activeDropdownOwner = nullptr;
        }
        isDropdownVisible = false;
        displayButton->MarkDirty(false);
        this->displayButton->RequestFocus(); // Focus back to the button
    }
    else { // Logic for OPENING the dropdown
        if (activeDropdownOwner && activeDropdownOwner != this) {
            activeDropdownOwner->ToggleDropdown(); // Close any other active dropdown
        }

        // 1. Create the WindowWidget for the dropdown *first*
        auto dropdownWindow = std::make_unique<WindowWidget>();
        dropdownWindow->id = this->id + "_dropdown_popup";
        dropdownWindow->hasTitleBar = false;
        dropdownWindow->style.hasBorder = true;
        dropdownWindow->style.borderWidth = 1;
        dropdownWindow->style.borderColor = { 0.3f, 0.3f, 0.3f, 1.0f };
        dropdownWindow->style.padding = { 0,0,0,0 }; // Window itself has no extra padding

        ContainerWidget* contentArea = dropdownWindow->GetContentArea();
        if (contentArea && contentArea->layout) { // Assuming contentArea has a layout (default is VerticalStackLayout)
            auto* layout = dynamic_cast<VerticalStackLayout*>(contentArea->layout.get());
            if (layout) layout->padding = { 1,1,1,1 }; // Minimal padding for contentArea, or {0,0,0,0}
        }

        // 2. Create the ScrollableListBoxWidget
        auto freshScrollableListUniquePtr = std::make_unique<ScrollableListBoxWidget>();
        freshScrollableListUniquePtr->id = this->id + "_scrollable_list";

        // Configure the internal ListBox via ScrollableListBoxWidget's methods/members
        freshScrollableListUniquePtr->GetListBox()->style = this->dropdownListPrototypeStyle;
        // Important: Make sure prototype style has a valid fontKey!
        // Or set it explicitly: freshScrollableListUniquePtr->GetListBox()->style.fontKey = FONT_KEY_UI_LABEL; (or similar)

        for (const auto& itemText : items) {
            freshScrollableListUniquePtr->AddItem(itemText);
        }
        freshScrollableListUniquePtr->OnItemSelected = this->onItemSelectedCallbackForDropdown;
        freshScrollableListUniquePtr->SetSelectedIndex(this->selectedIndex); // This also scrolls to make item visible

        // Get a raw pointer to the ScrollableListBoxWidget for convenience
        ScrollableListBoxWidget* slbwPtr = freshScrollableListUniquePtr.get();

        // Add the ScrollableListBoxWidget to the dropdown window's content area
        contentArea->AddChild(std::move(freshScrollableListUniquePtr));

        // Store raw ptr to the *internal ListBox*
        this->currentOpenDropdownListRawPtr = slbwPtr->GetListBox();
        if (this->currentOpenDropdownListRawPtr) {
            this->currentOpenDropdownListRawPtr->id = this->id + "_internal_listbox";
        }


        // 3. Determine the desired visible height for the ListBox's item display area
        ListBoxWidget* internalList = slbwPtr->GetListBox();
        internalList->Measure(); // Ensure ListBox's own preferredSize and itemHeight are up-to-date

        int allItemsHeight = internalList->GetContentHeight();
        int itemHeight = internalList->itemHeight > 0 ? internalList->itemHeight : 20;

        int maxVisibleItems = 7;
        int defaultMaxListPixelHeight = 150;

        int desiredListClientHeight = itemHeight * maxVisibleItems;
        desiredListClientHeight = std::min(allItemsHeight, desiredListClientHeight);
        desiredListClientHeight = std::min(desiredListClientHeight, defaultMaxListPixelHeight);
        if (internalList->items.empty()) {
            desiredListClientHeight = itemHeight;
        }
        else {
            desiredListClientHeight = std::max(desiredListClientHeight, itemHeight);
        }

        // Calculate ListBox's own chrome (padding + border around its item area)
        int lbChromeY = internalList->style.padding.top + internalList->style.padding.bottom +
            (internalList->style.hasBorder ? 2 * (int)internalList->style.borderWidth : 0);

        // Calculate ScrollableListBoxWidget's chrome (padding + border around LB & SB)
        int slbwChromeY = slbwPtr->style.padding.top + slbwPtr->style.padding.bottom +
            (slbwPtr->style.hasBorder ? 2 * (int)slbwPtr->style.borderWidth : 0);

        // The height SLBW's *bounds* needs to be to provide `desiredListClientHeight` for the ListBox.
        // This means `desiredListClientHeight` + LB's chrome + SLBW's chrome.
        int targetSLBWHeight = desiredListClientHeight + lbChromeY + slbwChromeY;

        slbwPtr->minSize.height = targetSLBWHeight;
        slbwPtr->maxSize.height = targetSLBWHeight;
        slbwPtr->minSize.width = std::max(displayButton->bounds.width, 120); // Min width for SLBW

        // 4. Now, let the dropdownWindow measure itself.
        Size windowPreferredSizeByMeasure = dropdownWindow->Measure();

        // 5. Set initial position and final size for the dropdown window
        Rect comboAbsoluteBounds = GetScreenBounds();
        dropdownWindow->bounds.x = comboAbsoluteBounds.x;
        dropdownWindow->bounds.y = comboAbsoluteBounds.y + comboAbsoluteBounds.height;
        dropdownWindow->bounds.width = std::max(displayButton->bounds.width, windowPreferredSizeByMeasure.width);
        dropdownWindow->bounds.height = windowPreferredSizeByMeasure.height;

        manager.AddWindow(std::move(dropdownWindow));

        if (this->currentOpenDropdownListRawPtr) {
            this->currentOpenDropdownListRawPtr->RequestFocus();
        }
        isDropdownVisible = true;
        displayButton->MarkDirty(false);
        activeDropdownOwner = this;
    }
}

// ComboBoxWidget::Draw
// combo_box_widget.cpp
// In combo_box_widget.cpp

void ComboBoxWidget::Draw(UIRenderer& renderer, float inheritedAlpha) {
    if (!visible) return;

    // ContainerWidget::Draw will handle its own background (if any) and drawing children (displayButton)
    // It will pass down the inheritedAlpha to displayButton.
    ContainerWidget::Draw(renderer, inheritedAlpha); // This draws the displayButton

    // --- ARROW DRAWING LOGIC ---
    Rect buttonScreenBounds = displayButton->GetScreenBounds(); // Overall bounds of the button
    const Style& btnStyle = displayButton->style; // displayButton's own style

    // Arrow properties (from your original logic)
    // Base the arrow size on the button's total height, not just client area, for consistency
    int arrowBaseSizeCalculation = std::min(8, buttonScreenBounds.height / 2);
    arrowBaseSizeCalculation = std::max(4, arrowBaseSizeCalculation); // Ensure min size

    int arrowBoxWidth = arrowBaseSizeCalculation;       // Width of the arrow's horizontal base
    int arrowBoxHeight = arrowBaseSizeCalculation / 2;  // Height of the arrow from base to tip
    int arrowPaddingFromRightEdge = 5;                // Desired space from the absolute right edge of the button

    // Calculate position for the arrow's top-left point of its bounding box
    Point arrowBoundingBoxTopLeft;

    // X position: Start from the button's right edge and move left
    arrowBoundingBoxTopLeft.x = buttonScreenBounds.x + buttonScreenBounds.width - arrowPaddingFromRightEdge - arrowBoxWidth;

    // Y position: Vertically center the arrow's *bounding box* within the button's *overall height*.
    // The original arrow pointed down, so its bounding box top-left Y is (total height - arrow box height)/2
    arrowBoundingBoxTopLeft.y = buttonScreenBounds.y + (buttonScreenBounds.height - arrowBoxHeight) / 2;


    Color arrowColor = btnStyle.foregroundColor; // Get base arrow color from button's style
    arrowColor.a *= inheritedAlpha;             // Modulate by the alpha inherited by the ComboBoxWidget

    if (arrowColor.a > 0.001f) {
        Point arrow[3]; // Your original points for a downward-pointing triangle

        // These points are relative to arrowBoundingBoxTopLeft
        arrow[0] = { arrowBoundingBoxTopLeft.x, arrowBoundingBoxTopLeft.y };
        arrow[1] = { arrowBoundingBoxTopLeft.x + arrowBoxWidth, arrowBoundingBoxTopLeft.y };
        arrow[2] = { arrowBoundingBoxTopLeft.x + arrowBoxWidth / 2, arrowBoundingBoxTopLeft.y + arrowBoxHeight };

        renderer.DrawLine(arrow[0], arrow[1], arrowColor);
        renderer.DrawLine(arrow[1], arrow[2], arrowColor);
        renderer.DrawLine(arrow[2], arrow[0], arrowColor);
    }
    dirty = false;
}

Size ComboBoxWidget::Measure() {
    // Size is primarily determined by the displayButton
    Size btnSize = displayButton->Measure();
    preferredSize = btnSize;
    return preferredSize;
}

void ComboBoxWidget::HandleEvent(UIEvent& event) {
    if (!visible || !enabled) return;
    ContainerWidget::HandleEvent(event); // Let button handle its click

    if (event.handled) return;

    if (event.eventType == UIEvent::Type::Keyboard && hasFocus) { // Focus is on the button part
        if (event.keyboard.type == KeyEventType::Press &&
            (event.keyboard.keyCode == KeyCode::Enter || event.keyboard.keyCode == KeyCode::Space ||
             event.keyboard.keyCode == KeyCode::DownArrow || event.keyboard.keyCode == KeyCode::UpArrow)) {
            ToggleDropdown();
            event.handled = true;
        }
    }
}

void ComboBoxWidget::OnFocusLost() {
    Widget::OnFocusLost(); // Call base

    // If the dropdown is visible AND focus is NOT going to the dropdown list itself, close it.
    if (isDropdownVisible && activeDropdownOwner == this) {
        Widget* newlyFocusedWidget = UIManager::Instance().GetFocusedWidget();
        bool focusWentToDropdown = false;
        if (newlyFocusedWidget && currentOpenDropdownListRawPtr) {
            // Check if newlyFocusedWidget is the dropdown list or a child of its window
            Widget* p = newlyFocusedWidget;
            while (p) {
                if (p == currentOpenDropdownListRawPtr->parent->parent) { // parent->parent is the WindowWidget
                    focusWentToDropdown = true;
                    break;
                }
                p = p->parent;
            }
        }

        if (!focusWentToDropdown) {
            ToggleDropdown();
        }
    }
}