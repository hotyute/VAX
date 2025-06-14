#define NOMINMAX
#include <windows.h>
#include <algorithm> // For std::clamp

#include "slider_widget.h"
#include "ui_renderer.h"
#include "ui_manager.h" // For UIManager capture
#include "style.h"      // For default styles

SliderWidget::SliderWidget(float initialValue, float minVal, float maxVal)
    : Widget(), currentValue(initialValue), minValue(minVal), maxValue(maxVal) {
    style = Style::DefaultButton(); // Base style, can be customized
    style.padding = { 4, 4, 4, 4 };
    focusable = true;

    // Default preferred size (horizontal slider assumed for now)
    minSize = { 100, thumbHeight + style.padding.top + style.padding.bottom };
    preferredSize = minSize;
    currentValue = std::clamp(currentValue, minValue, maxValue);
}

void SliderWidget::SetValue(float newValue, bool fireEvent) {
    float oldVal = currentValue;
    currentValue = std::clamp(newValue, minValue, maxValue);

    if (currentValue != oldVal) {
        MarkDirty(false);
        if (OnValueChanged && fireEvent) {
            OnValueChanged(this, currentValue);
        }
    }
}

void SliderWidget::SetMinValue(float minVal) {
    if (minValue != minVal) {
        minValue = minVal;
        SetValue(currentValue); // Re-clamp and potentially fire event
        MarkDirty(false);
    }
}
void SliderWidget::SetMaxValue(float maxVal) {
    if (maxValue != maxVal) {
        maxValue = maxVal;
        SetValue(currentValue); // Re-clamp
        MarkDirty(false);
    }
}


Rect SliderWidget::GetThumbRect() const {
    Rect client = GetClientBounds(); // Relative to widget's (0,0) + padding
    if (client.width <= 0 || client.height <= 0 || maxValue <= minValue) {
        return { client.x + client.width / 2 - thumbWidth / 2, client.y, thumbWidth, client.height }; // Default centered
    }

    // Assuming horizontal slider for now
    int trackWidth = client.width; // Thumb slides along the client width
    float valueRatio = (currentValue - minValue) / (maxValue - minValue);

    // Thumb position is the center of the thumb
    int thumbCenterX = client.x + static_cast<int>(valueRatio * trackWidth);

    int thumbActualWidth = thumbWidth;
    int thumbActualHeight = client.height; // Thumb takes full client height

    return {
        thumbCenterX - thumbActualWidth / 2,
        client.y,
        thumbActualWidth,
        thumbActualHeight
    };
}


// slider_widget.cpp
void SliderWidget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Added inheritedAlpha
    if (!visible) return;

    // Base Widget::Draw handles the slider's overall background/border, modulated by inheritedAlpha
    // Widget::Draw(renderer, inheritedAlpha); // Call if slider has its own distinct background/border

    Rect screenBounds = GetScreenBounds();
    Rect clientScreenBounds = {
        screenBounds.x + style.padding.left,
        screenBounds.y + style.padding.top,
        bounds.width - (style.padding.left + style.padding.right),
        bounds.height - (style.padding.top + style.padding.bottom)
    };

    if (clientScreenBounds.width <= 0 || clientScreenBounds.height <= 0) {
        dirty = false;
        return;
    }

    // 2. Draw Track
    Color trackColor = style.borderColor;
    trackColor.a *= inheritedAlpha; // Modulate track color alpha

    if (trackColor.a > 0.001f) {
        int trackVisualCenterY = clientScreenBounds.y + clientScreenBounds.height / 2;
        Rect trackRectScreen = {
            clientScreenBounds.x,
            trackVisualCenterY - trackThickness / 2,
            clientScreenBounds.width,
            trackThickness
        };
        renderer.DrawRect(trackRectScreen, trackColor, true);
    }

    // 3. Draw Thumb
    Rect thumbRelativeRect = GetThumbRect();
    Rect thumbScreenRect = {
        clientScreenBounds.x + thumbRelativeRect.x,
        clientScreenBounds.y + thumbRelativeRect.y,
        thumbRelativeRect.width,
        thumbRelativeRect.height
    };

    Color finalThumbColor = style.foregroundColor;
    if (isDraggingThumb || hasFocus) {
        finalThumbColor.r = std::min(1.0f, finalThumbColor.r * 1.2f);
        finalThumbColor.g = std::min(1.0f, finalThumbColor.g * 1.2f);
        finalThumbColor.b = std::min(1.0f, finalThumbColor.b * 1.2f);
    }
    finalThumbColor.a *= inheritedAlpha; // Modulate thumb color alpha

    Color finalThumbBorderColor = style.borderColor;
    finalThumbBorderColor.a *= inheritedAlpha;

    if (finalThumbColor.a > 0.001f) {
        renderer.DrawRect(thumbScreenRect, finalThumbColor, true);
    }
    if (style.hasBorder && finalThumbBorderColor.a > 0.001f) {
        renderer.DrawRect(thumbScreenRect, finalThumbBorderColor, false, 1.0f);
    }

    dirty = false;
}

Size SliderWidget::Measure() {
    // For a horizontal slider, height is determined by thumbHeight and padding.
    // Width is more flexible or set by minSize/preferredSize.
    int defaultHeight = thumbHeight + style.padding.top + style.padding.bottom + (style.hasBorder ? 2 * (int)style.borderWidth : 0);
    int defaultWidth = 100 + style.padding.left + style.padding.right + (style.hasBorder ? 2 * (int)style.borderWidth : 0);

    preferredSize = {
        std::max(minSize.width, defaultWidth),
        std::max(minSize.height, defaultHeight)
    };
    preferredSize.width = std::min(preferredSize.width, maxSize.width);
    preferredSize.height = std::min(preferredSize.height, maxSize.height);
    return preferredSize;
}

void SliderWidget::UpdateValueFromMousePosition(const Point& localMousePosInClient) {
    // localMousePosInClient is relative to the client area (inside padding)
    Rect client = GetClientBounds(); // Relative to widget's (0,0) + padding
    if (client.width <= 0 || maxValue <= minValue) return;

    // Assuming horizontal slider
    float clickRatio = static_cast<float>(localMousePosInClient.x) / client.width;
    clickRatio = std::clamp(clickRatio, 0.0f, 1.0f);

    float newValue = minValue + clickRatio * (maxValue - minValue);
    SetValue(newValue);
}


void SliderWidget::HandleEvent(UIEvent& event) {
    if (!visible || !enabled) { event.handled = false; return; }

    Point mousePosInWidget_LOCAL = event.mouse.position; // This will be local due to parent dispatch

    // Convert to client area local (inside padding)
    Point mousePosInClient = {
        mousePosInWidget_LOCAL.x - style.padding.left,
        mousePosInWidget_LOCAL.y - style.padding.top
    };

    Rect clientRectRelative = GetClientBounds(); // {0,0, client_w, client_h} within this widget's padding

    if (event.eventType == UIEvent::Type::Mouse) {
        if (event.mouse.type == MouseEventType::Press && event.mouse.button == MouseButton::Left) {
            // Check if click is within the client area (track area + thumb potential area)
            if (clientRectRelative.Contains(mousePosInClient)) {
                RequestFocus();
                isDraggingThumb = true;
                UIManager::Instance().SetCapture(this);
                UpdateValueFromMousePosition(mousePosInClient); // Update value based on click
                MarkDirty(false);
                event.handled = true;
            }
        }
        else if (event.mouse.type == MouseEventType::Move) {
            if (isDraggingThumb) {
                // If captured, UIManager sends SCREEN coords in event.mouse.position
                // We need to convert them to local to this widget, then to client area.
                Point effectiveMousePosLocalToWidget = event.mouse.position;
                if (UIManager::Instance().GetCapturedWidget() == this) {
                    Rect screenBounds = GetScreenBounds();
                    effectiveMousePosLocalToWidget.x -= screenBounds.x;
                    effectiveMousePosLocalToWidget.y -= screenBounds.y;
                }
                // Now effectiveMousePosLocalToWidget is local to the SliderWidget's (0,0)

                Point effectiveMousePosInClient = {
                    effectiveMousePosLocalToWidget.x - style.padding.left,
                    effectiveMousePosLocalToWidget.y - style.padding.top
                };
                UpdateValueFromMousePosition(effectiveMousePosInClient);
                event.handled = true;
            }
        }
        else if (event.mouse.type == MouseEventType::Release && event.mouse.button == MouseButton::Left) {
            if (isDraggingThumb) {
                isDraggingThumb = false;
                UIManager::Instance().ReleaseCapture();
                MarkDirty(false);
                event.handled = true;
            }
        }
    }
    else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
        if (event.keyboard.type == KeyEventType::Press) {
            float step = (maxValue - minValue) / 20.0f; // 5% step
            if (event.keyboard.keyCode == KeyCode::LeftArrow) {
                SetValue(currentValue - step);
                event.handled = true;
            }
            else if (event.keyboard.keyCode == KeyCode::RightArrow) {
                SetValue(currentValue + step);
                event.handled = true;
            }
        }
    }

    if (!event.handled) {
        Widget::HandleEvent(event);
    }
}