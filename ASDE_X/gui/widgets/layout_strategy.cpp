#define NOMINMAX
#include <windows.h>

#include "layout_strategy.h"
#include "widget.h"         // For Widget
#include "container_widget.h" // For ContainerWidget (though typically accessed via Widget*)

// --- VerticalStackLayout ---
Size VerticalStackLayout::MeasureChildren(ContainerWidget* container, const Size& availableSize) {
    int totalHeight = padding.top + padding.bottom;
    int maxWidth = 0;
    bool first = true;

    for (const auto& child : container->children) {
        if (!child->visible) continue;

        Size childPreferredSize = child->Measure();
        if (!first) {
            totalHeight += spacing;
        }
        totalHeight += childPreferredSize.height;
        if (childPreferredSize.width > maxWidth) {
            maxWidth = childPreferredSize.width;
        }
        first = false;
    }
    maxWidth += padding.left + padding.right;
    return {maxWidth, totalHeight};
}

// layout_strategy.cpp
void VerticalStackLayout::ArrangeChildren(ContainerWidget* container, const Rect& containerClientBounds_relative_to_container) {
    int currentY = containerClientBounds_relative_to_container.y + padding.top;
    // Effective width for arranging children *within this layout's own padding*
    int effectiveLayoutWidth = containerClientBounds_relative_to_container.width - padding.left - padding.right;
    if (effectiveLayoutWidth < 0) effectiveLayoutWidth = 0;

    for (const auto& child : container->children) {
        if (!child || !child->visible) continue;

        Size childPreferredSize = child->preferredSize;
        Rect childRelativeRect;

        // Determine child's width: use its preferred, but clamp to what's available.
        childRelativeRect.width = std::min(childPreferredSize.width, effectiveLayoutWidth);
        childRelativeRect.width = std::max(child->minSize.width, childRelativeRect.width);

        // Calculate child's X position based on alignment
        int childX = containerClientBounds_relative_to_container.x + padding.left; // Base X for left alignment
        if (VerticalStackLayout::childAlignment == HorizontalAlignment::Center) {
            childX += std::max(0, (effectiveLayoutWidth - childRelativeRect.width) / 2);
        }
        else if (childAlignment == HorizontalAlignment::Right) {
            childX += std::max(0, effectiveLayoutWidth - childRelativeRect.width);
        }
        // For Left alignment, childX is already correct.

        childRelativeRect.x = childX;
        childRelativeRect.y = currentY;
        childRelativeRect.height = childPreferredSize.height;

        child->Arrange(childRelativeRect);
        currentY += childRelativeRect.height + spacing;
    }
}

// --- HorizontalStackLayout ---
Size HorizontalStackLayout::MeasureChildren(ContainerWidget* container, const Size& availableSize) {
    int totalWidth = padding.left + padding.right;
    int maxHeight = 0;
    bool first = true;

    for (const auto& child : container->children) {
        if (!child->visible) continue;
        Size childPreferredSize = child->Measure();
        if (!first) {
            totalWidth += spacing;
        }
        totalWidth += childPreferredSize.width;
        if (childPreferredSize.height > maxHeight) {
            maxHeight = childPreferredSize.height;
        }
        first = false;
    }
    maxHeight += padding.top + padding.bottom;
    return {totalWidth, maxHeight};
}

void HorizontalStackLayout::ArrangeChildren(ContainerWidget* container, const Rect& containerClientBounds_relative_to_container) {
    int childY = containerClientBounds_relative_to_container.y + padding.top;
    int availableWidthForLayout = containerClientBounds_relative_to_container.width - padding.left - padding.right;
    int availableHeightForLayout = containerClientBounds_relative_to_container.height - padding.top - padding.bottom;

    if (availableWidthForLayout < 0) availableWidthForLayout = 0;
    if (availableHeightForLayout < 0) availableHeightForLayout = 0;

    // Calculate total preferred width of content (children + spacing)
    int totalContentPreferredWidth = 0;
    int visibleChildrenCount = 0;
    for (const auto& child : container->children) {
        if (child && child->visible) {
            totalContentPreferredWidth += child->preferredSize.width;
            visibleChildrenCount++;
        }
    }
    if (visibleChildrenCount > 1) {
        totalContentPreferredWidth += (visibleChildrenCount - 1) * spacing;
    }

    // Calculate starting X based on alignment
    int currentX = containerClientBounds_relative_to_container.x + padding.left;
    if (alignment == HorizontalAlignment::Center) {
        currentX += std::max(0, (availableWidthForLayout - totalContentPreferredWidth) / 2);
    }
    else if (alignment == HorizontalAlignment::Right) {
        currentX += std::max(0, availableWidthForLayout - totalContentPreferredWidth);
    }
    // For HorizontalAlignment::Left, currentX is already correct.

    for (const auto& child : container->children) {
        if (!child || !child->visible) continue;

        Size childPreferredSize = child->preferredSize;
        Rect childRelativeRect;

        childRelativeRect.x = currentX;
        childRelativeRect.y = childY;

        childRelativeRect.width = childPreferredSize.width;
        childRelativeRect.height = std::min(childPreferredSize.height, availableHeightForLayout);
        childRelativeRect.height = std::max(child->minSize.height, childRelativeRect.height);

        child->Arrange(childRelativeRect);
        currentX += childRelativeRect.width + spacing;
    }
}