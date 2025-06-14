#define NOMINMAX
#include <windows.h> // For OutputDebugString if used

#include "container_widget.h"
#include "ui_renderer.h"
#include "layout_strategy.h" // Ensure this is included

ContainerWidget::ContainerWidget() : Widget() {
    layout = std::make_unique<VerticalStackLayout>();
}

ContainerWidget::ContainerWidget(std::unique_ptr<LayoutStrategy> layoutStrategy)
    : Widget(), layout(std::move(layoutStrategy)) {}

void ContainerWidget::SetLayout(std::unique_ptr<LayoutStrategy> newLayout) {
    layout = std::move(newLayout);
    MarkDirty(true); // Layout change requires full re-measure/arrange/redraw
}

void ContainerWidget::AddChild(std::unique_ptr<Widget> child) {
    if (child) {
        child->parent = this;
        children.push_back(std::move(child));
        MarkDirty(true);
    }
}

// Override from Widget to handle children
bool ContainerWidget::DispatchEventToChildren(UIEvent& event_local_to_this_container) {
    // event_local_to_this_container.mouse.position is LOCAL to THIS ContainerWidget's
    // client area origin (i.e., if this container has padding, mouse_pos is already inside it).

    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        Widget* child = it->get();
        if (child && child->visible && child->enabled) {
            // child->bounds.x and child->bounds.y are relative to THIS container's client area origin.
            // So, we directly check if event_local_to_this_container.mouse.position is within child->bounds.
            if (child->bounds.Contains(event_local_to_this_container.mouse.position)) {
                UIEvent childEvent = event_local_to_this_container;
                childEvent.mouse.position.x -= child->bounds.x; // Make coordinates local to the CHILD's (0,0)
                childEvent.mouse.position.y -= child->bounds.y;
                childEvent.targetWidget = child;

                child->HandleEvent(childEvent);

                if (childEvent.handled) {
                    event_local_to_this_container.handled = true;
                    return true;
                }
                // If child was hit but didn't handle, we might allow event to "pass through"
                // to widgets visually behind it if they weren't obscured by this child's drawing.
                // For now, first hit that handles it wins for simplicity.
                // If you want click-through, remove the 'return true' and let loop continue,
                // but then the 'handled' flag logic becomes more complex.
                return false; // Child was hit but didn't handle it, stop further dispatch in this container for THIS event
                // to prevent multiple widgets from reacting to the same physical click
                // unless explicitly designed for pass-through.
                // Or, if you want it to pass through if unhandled: comment this return.
            }
        }
    }
    return false; // No child handled the event
}


void ContainerWidget::HandleEvent(UIEvent& event_from_parent) { // event_from_parent.mouse.position is LOCAL to THIS container
    if (!visible || !enabled) {
        event_from_parent.handled = false;
        return;
    }

    // 1. Try to dispatch to children first for mouse events.
    if (event_from_parent.eventType == UIEvent::Type::Mouse) {
        if (DispatchEventToChildren(event_from_parent)) {
            // event_from_parent.handled is set by DispatchEventToChildren
            return; // Child handled it.
        }
    }

    // 2. If no child handled it (or not a mouse event for children),
    //    let this container itself try to handle it via base Widget's callbacks.
    //    (e.g., for a click on the container's background, or keyboard events if container has focus).
    Widget::HandleEvent(event_from_parent); // This calls OnClick etc. for the Container itself
    // and sets event_from_parent.handled if consumed.
}


// container_widget.cpp
void ContainerWidget::Draw(UIRenderer& renderer, float inheritedAlpha) {
    if (!visible) return;

    // Draw this container's own background/border using the inheritedAlpha
    Widget::Draw(renderer, inheritedAlpha); // Base Widget::Draw now handles modulating its style by inheritedAlpha

    // Calculate this container's client area in screen coordinates
    Rect screenClientBounds = GetScreenBounds(); // Gets my absolute screen position
    screenClientBounds.x += style.padding.left;
    screenClientBounds.y += style.padding.top;
    screenClientBounds.width -= (style.padding.left + style.padding.right);
    screenClientBounds.height -= (style.padding.top + style.padding.bottom);

    if (screenClientBounds.width <= 0 || screenClientBounds.height <= 0) {
        // dirty = false; // This widget (frame) itself is drawn by Widget::Draw
        return;
    }

    renderer.PushClipRect(screenClientBounds);

    // Calculate the alpha to pass to children: this container's own style alpha * inherited alpha
    // This allows children to become more transparent if their parent container is also transparent.
    float alphaForChildren = this->style.backgroundColor.a * inheritedAlpha;
    // However, if the container itself has no background (alpha ~0), we don't want it to make all children invisible.
    // So, it might be better to just pass down the `inheritedAlpha` directly,
    // or the alpha of the *window* if this container is meant to be a "passthrough" visually.
    // For now, let's pass the effective alpha of THIS container.
    // If style.backgroundColor.a is already modulated by inheritedAlpha in Widget::Draw,
    // then we simply pass that result.
    // A simpler approach is just to pass inheritedAlpha through:
    // float childInheritedAlpha = inheritedAlpha;

    // Let's refine: The alpha passed to children should be the opacity of *this container's background*.
    // If this container's background itself is transparent (style.backgroundColor.a is low),
    // and it's inside an opaque window, children should still be fully visible *relative to this container*.
    // The `inheritedAlpha` is the opacity of the "backdrop" this container is drawn upon.
    // Children should be drawn with their own style's alpha, modulated by the `inheritedAlpha` received by *this* container.

    for (const auto& child : children) {
        if (child && child->visible) {
            // Pass the same inheritedAlpha this container received.
            // The child's own Widget::Draw will then modulate its style by this.
            child->Draw(renderer, inheritedAlpha);
        }
    }
    renderer.PopClipRect();
    dirty = false; // This should be set by the top-level WindowWidget after all its parts are drawn
}

Size ContainerWidget::Measure() {
    if (layout) {
        // GetClientBounds() used by ArrangeChildren is relative. Here we just need total size.
        // availableSize for children is this container's potential client width/height.
        // For initial measure, assume large available size, layout will constrain later.
        Size contentSize = layout->MeasureChildren(this, { maxSize.width - (style.padding.left + style.padding.right),
                                                            maxSize.height - (style.padding.top + style.padding.bottom) });
        preferredSize.width = contentSize.width + style.padding.left + style.padding.right;
        preferredSize.height = contentSize.height + style.padding.top + style.padding.bottom;
    }
    else {
        // No layout: default to max of children's preferred sizes + padding
        int maxChildWidth = 0;
        int maxChildHeight = 0;
        for (const auto& child : children) {
            if (child && child->visible) {
                Size childSize = child->Measure();
                if (childSize.width > maxChildWidth) maxChildWidth = childSize.width;
                if (childSize.height > maxChildHeight) maxChildHeight = childSize.height;
            }
        }
        preferredSize = {
            maxChildWidth + style.padding.left + style.padding.right,
            maxChildHeight + style.padding.top + style.padding.bottom
        };
    }
    preferredSize.width = std::max(minSize.width, std::min(preferredSize.width, maxSize.width));
    preferredSize.height = std::max(minSize.height, std::min(preferredSize.height, maxSize.height));
    return preferredSize;
}

void ContainerWidget::Arrange(const Rect& finalRect_relative_to_my_parent) {
    // 1. Set my own bounds relative to my parent's client area
    Widget::Arrange(finalRect_relative_to_my_parent);
    // Now this->bounds are {x_rel_to_parent, y_rel_to_parent, my_width, my_height}

    // 2. Arrange my children within my client area
    if (layout) {
        // GetClientBounds() returns a Rect like {my_padding_left, my_padding_top, my_client_width, my_client_height}
        // The coordinates in this Rect are relative to MY (0,0).
        // The layout strategy will place children starting from these offsets.
        layout->ArrangeChildren(this, GetClientBounds());
    }
    else {
        // Fallback if no layout
        for (const auto& child : children) {
            if (child && child->visible) {
                // Child's bounds.x/y will be relative to this container's client area (0,0 + padding)
                child->Arrange({ style.padding.left, style.padding.top, child->preferredSize.width, child->preferredSize.height });
            }
        }
    }
    MarkDirty(true); // Children arrangement might have changed
}