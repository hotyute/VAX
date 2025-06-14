#pragma once
#include "ui_common.h"
#include <vector>
#include <memory>

// Forward declaration
class ContainerWidget;
class Widget;

class LayoutStrategy {
public:
    virtual ~LayoutStrategy() = default;
    virtual Size MeasureChildren(ContainerWidget* container, const Size& availableSize) = 0;
    virtual void ArrangeChildren(ContainerWidget* container, const Rect& containerBounds) = 0;
};

enum class HorizontalAlignment { Left, Center, Right /*, Stretch (more complex) */ };

class VerticalStackLayout : public LayoutStrategy {
public:
    int spacing = 0;
    Padding padding; // Padding within the container before stacking children
    HorizontalAlignment childAlignment = HorizontalAlignment::Left; // Default

    Size MeasureChildren(ContainerWidget* container, const Size& availableSize) override;
    void ArrangeChildren(ContainerWidget* container, const Rect& containerClientBounds_relative_to_container) override;
};

class HorizontalStackLayout : public LayoutStrategy {
public:
    int spacing = 0;
    Padding padding;

    HorizontalAlignment alignment = HorizontalAlignment::Left; // Default to left

    Size MeasureChildren(ContainerWidget* container, const Size& availableSize) override;
    void ArrangeChildren(ContainerWidget* container, const Rect& containerBounds) override;
    void SetHorizontalAlignment(HorizontalAlignment align) { alignment = align; }
};