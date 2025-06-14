#pragma once
#include "container_widget.h"

class WindowWidget : public ContainerWidget {
public:
    std::string title;
    bool draggable = true;
    bool closable = true;
    bool hasTitleBar = true;
    int titleBarHeight = 25; // Default, can be styled

    WindowWidget(const std::string& title = "Window");

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0f) override;
    void HandleEvent(UIEvent& event) override;
    Size Measure() override;
    void Arrange(const Rect& finalRect) override;

    void SetDraggable(bool canDrag) { draggable = canDrag; }
    void SetClosable(bool canClose) { closable = canClose; }

    ContainerWidget* GetContentArea() const { return contentArea; } // <<< ADDED THIS GETTER

    //protected: // contentArea was here - moved to private for better encapsulation with getter
    Rect GetTitleBarRect() const;
    Rect GetCloseButtonRect() const;

protected:
    ContainerWidget* contentArea;

private:

    bool isDragging = false;
    Point dragStartMousePos;
    Point dragStartWindowPos;
};