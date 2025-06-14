#define NOMINMAX
#include <windows.h>
#include <algorithm>

#include "scrollable_list_box_widget.h"
#include "ui_manager.h" // For UIManager if needed for focus (though ListBox handles its own)

ScrollableListBoxWidget::ScrollableListBoxWidget() : ContainerWidget() {
    auto hLayout = std::make_unique<HorizontalStackLayout>();
    hLayout->spacing = 0;
    hLayout->padding = { 0,0,0,0 };
    SetLayout(std::move(hLayout));

    listBox = new ListBoxWidget();
    ContainerWidget::AddChild(std::unique_ptr<ListBoxWidget>(listBox));

    scrollBar = new ScrollBarWidget(ScrollBarOrientation::Vertical);
    ContainerWidget::AddChild(std::unique_ptr<ScrollBarWidget>(scrollBar));

    scrollBar->OnScroll = std::bind(&ScrollableListBoxWidget::OnInternalScroll, this,
        std::placeholders::_1, std::placeholders::_2);

    listBox->OnItemSelected = [this](ListBoxWidget* lb, int index, const std::string& text) {
        // The ListBox itself handles selection. We just forward the event.
        if (this->OnItemSelected) {
            this->OnItemSelected(lb, index, text);
        }
        // Selection might have changed scroll due to ListBox ensuring visibility,
        // so update scrollbar.
        UpdateScrollBarParameters();
        };
}

void ScrollableListBoxWidget::AddItem(const std::string& item) {
    listBox->AddItem(item);
    //UpdateScrollBarParameters();
    MarkDirty(true);
}

void ScrollableListBoxWidget::ClearItems() {
    listBox->ClearItems();
    listBox->SetScrollOffset(0); // Reset scroll on clear
    //UpdateScrollBarParameters();
    MarkDirty(true);
}

int ScrollableListBoxWidget::GetSelectedIndex() const {
    if (!listBox) return -1;
    return listBox->selectedIndex; // Direct access to public member
}

std::string ScrollableListBoxWidget::GetSelectedText() const {
    if (!listBox || listBox->selectedIndex < 0 || listBox->selectedIndex >= (int)listBox->items.size()) {
        return "";
    }
    return listBox->items[listBox->selectedIndex]; // Direct access
}

void ScrollableListBoxWidget::SetSelectedIndex(int index) {
    if (!listBox) return;

    if (index < -1 || index >= (int)listBox->items.size()) { // Allow -1 for no selection
        if (index == -1) {
            listBox->selectedIndex = -1;
        }
        else if (!listBox->items.empty() && index >= (int)listBox->items.size()) {
            listBox->selectedIndex = (int)listBox->items.size() - 1;
        }
        else if (!listBox->items.empty() && index < -1) {
            listBox->selectedIndex = 0;
        }
        else { // index is out of bounds for non-empty list, or list is empty and index != -1
            listBox->selectedIndex = -1; // Default to no selection
        }
    }
    else {
        listBox->selectedIndex = index;
    }
    listBox->hoverIndex = listBox->selectedIndex; // Sync hover

    // Ensure selected item is visible by adjusting scroll
    if (listBox->selectedIndex != -1) {
        Rect clientBounds = listBox->GetClientBounds();
        if (clientBounds.height <= 0 && listBox->parent) { // If not yet arranged, try parent bounds
            clientBounds = listBox->parent->GetClientBounds();
        }


        int itemTopY = listBox->selectedIndex * listBox->itemHeight;
        int itemBottomY = itemTopY + listBox->itemHeight;

        if (clientBounds.height > 0) { // Only scroll if we have a valid client height
            if (itemTopY < listBox->scrollOffsetY) {
                listBox->SetScrollOffset(itemTopY);
            }
            else if (itemBottomY > listBox->scrollOffsetY + clientBounds.height) {
                listBox->SetScrollOffset(itemBottomY - clientBounds.height);
            }
        }
    }
    scrollBar->SetValue(listBox->scrollOffsetY, false); // Update scrollbar without firing event
    UpdateScrollBarParameters(); // Update visibility etc.
    listBox->MarkDirty(false);
    MarkDirty(false); // The container might need redraw due to scrollbar visibility change
}

void ScrollableListBoxWidget::UpdateScrollBarParameters() {
    if (!listBox || !scrollBar) return;

    Rect listBoxClientBounds = listBox->GetClientBounds();
    // If GetClientBounds returns 0 height before proper arrangement, use actual bounds
    // This can happen if called too early in the measure/arrange cycle.
    // The Arrange method should be the most reliable place for this.
    if (listBoxClientBounds.height <= 0 && listBox->bounds.height > 0) {
        listBoxClientBounds.height = listBox->bounds.height - listBox->style.padding.top - listBox->style.padding.bottom;
        if (listBoxClientBounds.height < 0) listBoxClientBounds.height = 0;
    }
    if (listBoxClientBounds.width <= 0 && listBox->bounds.width > 0) {
        listBoxClientBounds.width = listBox->bounds.width - listBox->style.padding.left - listBox->style.padding.right;
        if (listBoxClientBounds.width < 0) listBoxClientBounds.width = 0;
    }


    int viewportH = listBoxClientBounds.height;
    int contentH = listBox->GetContentHeight();
    int currentScrollY = listBox->scrollOffsetY;

    scrollBar->SetParameters(contentH, viewportH, currentScrollY);
    bool shouldBeVisible = contentH > viewportH;
    scrollBar->SetVisible(shouldBeVisible);
}

void ScrollableListBoxWidget::OnInternalScroll(ScrollBarWidget* sb, int newValue) {
    if (listBox) {
        listBox->SetScrollOffset(newValue);
    }
}

void ScrollableListBoxWidget::Arrange(const Rect& finalRect) {
    Widget::Arrange(finalRect); // Sets this->bounds

    if (listBox && scrollBar && layout) {
        listBox->Measure();
        scrollBar->Measure();

        Size scrollBarPrefSize = scrollBar->preferredSize; // Scrollbar wants its preferred width

        int myClientWidth = bounds.width - style.padding.left - style.padding.right;
        int myClientHeight = bounds.height - style.padding.top - style.padding.bottom;
        if (myClientWidth < 0) myClientWidth = 0;
        if (myClientHeight < 0) myClientHeight = 0;

        HorizontalStackLayout* hLayout = dynamic_cast<HorizontalStackLayout*>(layout.get());
        int layoutSpacing = hLayout ? hLayout->spacing : 0;
        Padding layoutInternalPadding = hLayout ? hLayout->padding : Padding{ 0,0,0,0 };

        // Space for ListBox content (width)
        int availableWidthForListBoxContent = myClientWidth - scrollBarPrefSize.width - layoutSpacing - layoutInternalPadding.left - layoutInternalPadding.right;
        if (availableWidthForListBoxContent < 0) availableWidthForListBoxContent = 0;

        listBox->preferredSize.width = std::max(listBox->minSize.width,
            std::min(availableWidthForListBoxContent, listBox->maxSize.width));

        // Space for children content (height)
        int availableHeightForChildrenContent = myClientHeight - layoutInternalPadding.top - layoutInternalPadding.bottom;
        if (availableHeightForChildrenContent < 0) availableHeightForChildrenContent = 0;

        listBox->preferredSize.height = std::max(listBox->minSize.height,
            std::min(availableHeightForChildrenContent, listBox->maxSize.height));
        scrollBar->preferredSize.height = std::max(scrollBar->minSize.height,
            std::min(availableHeightForChildrenContent, scrollBar->maxSize.height));

    }

    if (this->layout) {
        this->layout->ArrangeChildren(this, this->GetClientBounds());
    } // ... else fallback ...

    UpdateScrollBarParameters(); // IMPORTANT: Call after layout has set final child bounds
}

Size ScrollableListBoxWidget::Measure() {
    // Let the base ContainerWidget::Measure do its job.
    // It will call layout->MeasureChildren(this, availableSize).
    // layout->MeasureChildren will iterate through children (listBox, scrollBar),
    // call child->Measure() on each, and then aggregate them based on the layout logic.

    // We don't need to manually sum up sizes here if the layout strategy does it.
    // The key is that listBox and scrollBar must correctly report their *own* preferred sizes
    // when their Measure() is called by the layout strategy.

    // If we wanted to enforce that the ScrollableListBoxWidget has a specific height
    // (e.g., "height of 5 items for the listbox" + scrollbar height),
    // we would set listBox->preferredSize.height accordingly before calling base Measure,
    // or adjust the result from base Measure.

    // For now, let's rely on the HorizontalStackLayout's default behavior:
    // It sums widths (+ spacing + its own padding) and takes max height (+ its own padding).

    // First ensure children's preferred sizes are up-to-date for the layout to use
    if (listBox) listBox->Measure();
    if (scrollBar) scrollBar->Measure();

    Size calculatedSizeByLayout = { 0,0 };
    if (layout) {
        // The availableSize passed to layout->MeasureChildren should be derived from this container's maxSize constraints
        Size availableForLayout = {
            maxSize.width - (this->style.padding.left + this->style.padding.right),
            maxSize.height - (this->style.padding.top + this->style.padding.bottom)
        };
        if (availableForLayout.width < 0) availableForLayout.width = 0;
        if (availableForLayout.height < 0) availableForLayout.height = 0;

        calculatedSizeByLayout = layout->MeasureChildren(this, availableForLayout);

        // The layout strategy's MeasureChildren usually returns the content size needed *within its own padding*.
        // The layout's own padding is handled by the layout strategy.
        // This container's (ScrollableListBoxWidget's) padding needs to be added on top.
        preferredSize.width = calculatedSizeByLayout.width + this->style.padding.left + this->style.padding.right;
        preferredSize.height = calculatedSizeByLayout.height + this->style.padding.top + this->style.padding.bottom;

    }
    else {
        // Fallback if no layout (shouldn't happen as constructor sets one)
        // But if it did, it would be similar to ContainerWidget::Measure's fallback
        int maxChildWidth = 0;
        int maxChildHeight = 0;
        if (listBox) {
            maxChildWidth = std::max(maxChildWidth, listBox->preferredSize.width);
            maxChildHeight = std::max(maxChildHeight, listBox->preferredSize.height);
        }
        if (scrollBar) {
            maxChildWidth = std::max(maxChildWidth, scrollBar->preferredSize.width); // This would be wrong for H-Stack
            maxChildHeight = std::max(maxChildHeight, scrollBar->preferredSize.height);
        }
        // This fallback logic needs to be specific to how non-layout children are handled
        preferredSize.width = maxChildWidth + this->style.padding.left + this->style.padding.right;
        preferredSize.height = maxChildHeight + this->style.padding.top + this->style.padding.bottom;

    }


    // Apply overall min/max constraints for this ScrollableListBoxWidget
    preferredSize.width = std::max(minSize.width, std::min(preferredSize.width, maxSize.width));
    preferredSize.height = std::max(minSize.height, std::min(preferredSize.height, maxSize.height));

    return preferredSize;
}

void ScrollableListBoxWidget::HandleEvent(UIEvent& event) {

    if (!visible || !enabled)
        return;

    // Store original mouse position (local to this ScrollableListBoxWidget)
    Point originalMousePos = event.mouse.position;
    bool eventWasHandledByChild = false;

    // 1. Dispatch to ScrollBar first if mouse is over it
    if (scrollBar && scrollBar->visible && scrollBar->enabled) {
        if (scrollBar->bounds.Contains(originalMousePos)) {
            UIEvent scrollBarEvent = event;
            scrollBarEvent.mouse.position.x = originalMousePos.x - scrollBar->bounds.x;
            scrollBarEvent.mouse.position.y = originalMousePos.y - scrollBar->bounds.y;
            scrollBar->HandleEvent(scrollBarEvent);
            if (scrollBarEvent.handled) {
                // OnInternalScroll is called by ScrollBarWidget if value changes
                event.handled = true;
                eventWasHandledByChild = true;
                // return; // Don't return yet, allow SLBW to handle wheel itself if scrollbar didn't
            }
        }
    }

    // 2. Dispatch to ListBox if mouse is over it AND event not yet handled
    if (!event.handled && listBox && listBox->visible && listBox->enabled) {
        if (listBox->bounds.Contains(originalMousePos)) {
            UIEvent listBoxEvent = event;
            listBoxEvent.mouse.position.x = originalMousePos.x - listBox->bounds.x;
            listBoxEvent.mouse.position.y = originalMousePos.y - listBox->bounds.y;
            listBox->HandleEvent(listBoxEvent);
            if (listBoxEvent.handled) {
                // If ListBox scrolled itself (e.g., keyboard)
                if (event.eventType == UIEvent::Type::Keyboard) {
                    scrollBar->SetValue(listBox->scrollOffsetY, false); // Update scrollbar visually
                    UpdateScrollBarParameters(); // Check if scrollbar visibility changed
                }
                // If ListBox handled a mouse wheel event itself (if ListBoxWidget implements it)
                // For now, ListBoxWidget doesn't directly handle mouse wheel.
                event.handled = true;
                eventWasHandledByChild = true;
                // return; // Don't return yet
            }
        }
    }

    // 3. Handle mouse wheel over the general ScrollableListBoxWidget area
    //    This will catch wheel events if the cursor is over the ListBox part
    //    (if ListBox itself doesn't handle wheel) or any padding of the SLBW.
    if (!event.handled && event.eventType == UIEvent::Type::Mouse && event.mouse.type == MouseEventType::Scroll) {
        // Check if the original mouse position (local to SLBW) is within SLBW's bounds.
        // This is important if the event was passed from a parent that already did this check,
        // but it's safer to re-verify for directness.
        Rect slbwLocalBounds = { 0, 0, bounds.width, bounds.height };
        if (slbwLocalBounds.Contains(originalMousePos)) {
            // Check if the ListBox content is actually scrollable
            if (listBox->GetContentHeight() > listBox->GetClientBounds().height) {
                int oldOffset = listBox->scrollOffsetY;

                // Determine scroll step. Use itemHeight for a line-by-line scroll feel.
                int scrollStep = listBox->itemHeight > 0 ? listBox->itemHeight : 20;
                int deltaLines = event.mouse.scrollDelta / WHEEL_DELTA; // WHEEL_DELTA is usually 120

                listBox->SetScrollOffset(listBox->scrollOffsetY - (deltaLines * scrollStep));

                if (listBox->scrollOffsetY != oldOffset) {
                    scrollBar->SetValue(listBox->scrollOffsetY, false); // Update scrollbar thumb position
                    UpdateScrollBarParameters(); // Update scrollbar visibility/params
                    MarkDirty(true); // SLBW and children need redraw
                }
                event.handled = true;
            }
        }
    }

    // 4. If not handled by children or specific SLBW logic (like wheel scroll),
    //    pass to base Widget::HandleEvent for general callbacks on SLBW itself.
    if (!event.handled) {
        Widget::HandleEvent(event); // For this container's own specific events (e.g. OnClick on SLBW bg)
    }
}