#pragma once
#include "widget.h"
#include "layout_strategy.h"

class ContainerWidget : public Widget {
public:
    std::unique_ptr<LayoutStrategy> layout;

    ContainerWidget();
    explicit ContainerWidget(std::unique_ptr<LayoutStrategy> layoutStrategy);

    void SetLayout(std::unique_ptr<LayoutStrategy> newLayout);

    // Override AddChild to specifically manage children for layout
    void AddChild(std::unique_ptr<Widget> child) override;

    void HandleEvent(UIEvent& event) override;
    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f) override;
    Size Measure() override;
    void Arrange(const Rect& finalRect) override;
    bool DispatchEventToChildren(UIEvent& event) override;
};