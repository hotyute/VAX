#define NOMINMAX
#include <windows.h> // For OutputDebugString for debugging
#include <algorithm> // For std::min/max

#include "scrollbar_widget.h"
#include "ui_renderer.h"
#include "ui_manager.h" // For UIManager::Instance().SetCapture/ReleaseCapture

ScrollBarWidget::ScrollBarWidget(ScrollBarOrientation orient)
	: Widget(), orientation(orient) {
	style = Style::DefaultButton(); // Use button style as a base, can be customized
	style.padding = { 1, 1, 1, 1 };   // Minimal padding for the scrollbar itself
	focusable = true; // Can be focused to use arrow keys (if implemented)

	if (orientation == ScrollBarOrientation::Vertical) {
		minSize = { trackThickness, 50 }; // Default min size
		preferredSize = { trackThickness, 100 };
	}
	else {
		minSize = { 50, trackThickness };
		preferredSize = { 100, trackThickness };
	}
}

void DrawArrowGlyphInternal(UIRenderer& renderer, const Rect& buttonScreenRect, ScrollBarOrientation orientation, bool isUpOrLeft, const Color& color) {
	Point p[3];
	int arrowMargin = std::max(1, buttonScreenRect.width / 4);
	if (orientation == ScrollBarOrientation::Horizontal && buttonScreenRect.height > 0) { // Adjust margin for horizontal if based on height
		arrowMargin = std::max(1, buttonScreenRect.height / 4);
	}


	if (orientation == ScrollBarOrientation::Vertical) {
		int glyphContainerWidth = buttonScreenRect.width - 2 * arrowMargin;
		int glyphContainerHeight = buttonScreenRect.height - 2 * arrowMargin;
		if (glyphContainerWidth <= 0 || glyphContainerHeight <= 0) return;

		int arrowHeadWidth = glyphContainerWidth;
		int arrowHeadHeight = std::min(arrowHeadWidth / 2, glyphContainerHeight);
		arrowHeadHeight = std::max(2, arrowHeadHeight);

		int centerX = buttonScreenRect.x + buttonScreenRect.width / 2;

		if (isUpOrLeft) { // Up Arrow
			int topY = buttonScreenRect.y + arrowMargin;
			p[0] = { centerX - arrowHeadWidth / 2, topY + arrowHeadHeight };
			p[1] = { centerX + arrowHeadWidth / 2, topY + arrowHeadHeight };
			p[2] = { centerX, topY };
		}
		else { // Down Arrow
			int bottomY = buttonScreenRect.y + buttonScreenRect.height - arrowMargin;
			p[0] = { centerX - arrowHeadWidth / 2, bottomY - arrowHeadHeight };
			p[1] = { centerX + arrowHeadWidth / 2, bottomY - arrowHeadHeight };
			p[2] = { centerX, bottomY };
		}
	}
	else { // Horizontal
		int glyphContainerWidth = buttonScreenRect.width - 2 * arrowMargin;
		int glyphContainerHeight = buttonScreenRect.height - 2 * arrowMargin;
		if (glyphContainerWidth <= 0 || glyphContainerHeight <= 0) return;

		int arrowHeadHeight = glyphContainerHeight;
		int arrowHeadWidth = std::min(arrowHeadHeight / 2, glyphContainerWidth);
		arrowHeadWidth = std::max(2, arrowHeadWidth);

		int centerY = buttonScreenRect.y + buttonScreenRect.height / 2;

		if (isUpOrLeft) { // Left Arrow
			int leftX = buttonScreenRect.x + arrowMargin;
			p[0] = { leftX + arrowHeadWidth, centerY - arrowHeadHeight / 2 };
			p[1] = { leftX + arrowHeadWidth, centerY + arrowHeadHeight / 2 };
			p[2] = { leftX, centerY };
		}
		else { // Right Arrow
			int rightX = buttonScreenRect.x + buttonScreenRect.width - arrowMargin;
			p[0] = { rightX - arrowHeadWidth, centerY - arrowHeadHeight / 2 };
			p[1] = { rightX - arrowHeadWidth, centerY + arrowHeadHeight / 2 };
			p[2] = { rightX, centerY };
		}
	}
	renderer.DrawLine(p[0], p[1], color, 1.5f);
	renderer.DrawLine(p[1], p[2], color, 1.5f);
	renderer.DrawLine(p[2], p[0], color, 1.5f);
}

Rect ScrollBarWidget::GetUpArrowRect() const {
	if (!showArrowButtons) return { 0, 0, 0, 0 };
	Rect client = GetClientBounds();
	if (orientation == ScrollBarOrientation::Vertical) {
		// Ensure button size is not more than half the client height (if client height is very small)
		int actualButtonSize = std::min(arrowButtonSize, client.height > 0 ? client.height / 2 : arrowButtonSize);
		if (client.height < arrowButtonSize && client.height > 0) actualButtonSize = client.height; // if only space for one button effectively
		else if (client.height <= 0) actualButtonSize = 0;

		return { client.x, client.y, client.width, actualButtonSize };
	}
	return { 0,0,0,0 };
}

Rect ScrollBarWidget::GetDownArrowRect() const {
	if (!showArrowButtons) return { 0, 0, 0, 0 };
	Rect client = GetClientBounds();
	if (orientation == ScrollBarOrientation::Vertical) {
		int actualButtonSize = std::min(arrowButtonSize, client.height > 0 ? client.height / 2 : arrowButtonSize);
		if (client.height < arrowButtonSize && client.height > 0) actualButtonSize = client.height;
		else if (client.height <= 0) actualButtonSize = 0;

		return { client.x, client.y + client.height - actualButtonSize, client.width, actualButtonSize };
	}
	return { 0,0,0,0 };
}

Rect ScrollBarWidget::GetLeftArrowRect() const {
	if (!showArrowButtons) return { 0,0,0,0 };
	Rect client = GetClientBounds();
	if (orientation == ScrollBarOrientation::Horizontal) {
		int actualButtonSize = std::min(arrowButtonSize, client.width > 0 ? client.width / 2 : arrowButtonSize);
		if (client.width < arrowButtonSize && client.width > 0) actualButtonSize = client.width;
		else if (client.width <= 0) actualButtonSize = 0;

		return { client.x, client.y, actualButtonSize, client.height };
	}
	return { 0,0,0,0 };
}

Rect ScrollBarWidget::GetRightArrowRect() const {
	if (!showArrowButtons) return { 0,0,0,0 };
	Rect client = GetClientBounds();
	if (orientation == ScrollBarOrientation::Horizontal) {
		int actualButtonSize = std::min(arrowButtonSize, client.width > 0 ? client.width / 2 : arrowButtonSize);
		if (client.width < arrowButtonSize && client.width > 0) actualButtonSize = client.width;
		else if (client.width <= 0) actualButtonSize = 0;

		return { client.x + client.width - actualButtonSize, client.y, actualButtonSize, client.height };
	}
	return { 0,0,0,0 };
}

Rect ScrollBarWidget::GetTrackAreaRect() const {
	Rect client = GetClientBounds();
	if (!showArrowButtons) return client;

	int actualButtonSize = 0;
	if (orientation == ScrollBarOrientation::Vertical) {
		actualButtonSize = std::min(arrowButtonSize, client.height > 0 ? client.height / 2 : 0);
		if (client.height < 2 * actualButtonSize) return { client.x, client.y + client.height / 2, client.width, 0 };
		return {
			client.x,
			client.y + actualButtonSize,
			client.width,
			client.height - (2 * actualButtonSize)
		};
	}
	else { // Horizontal
		actualButtonSize = std::min(arrowButtonSize, client.width > 0 ? client.width / 2 : 0);
		if (client.width < 2 * actualButtonSize) return { client.x + client.width / 2, client.y, 0, client.height };
		return {
			client.x + actualButtonSize,
			client.y,
			client.width - (2 * actualButtonSize),
			client.height
		};
	}
}

void ScrollBarWidget::SetParameters(int newContentSize, int newViewportSize, int newValue) {
	bool changed = false;
	if (contentSize != newContentSize) {
		contentSize = std::max(0, newContentSize);
		changed = true;
	}
	if (viewportSize != newViewportSize) {
		viewportSize = std::max(0, newViewportSize);
		changed = true;
	}
	// Ensure value is valid after content/viewport changes
	currentValue = std::max(0, std::min(newValue, contentSize - viewportSize));
	if (currentValue < 0) currentValue = 0; // If contentSize < viewportSize

	if (changed || currentValue != newValue) { // currentValue might have been clamped
		MarkDirty(false);
		// if (OnScroll && currentValue != newValue) OnScroll(this, currentValue); // Decide if this should fire
	}
}

void ScrollBarWidget::SetValue(int newValue, bool fireEvent) {
	int oldVal = currentValue;
	currentValue = std::max(0, newValue);
	if (contentSize > viewportSize) {
		currentValue = std::min(currentValue, contentSize - viewportSize);
	}
	else {
		currentValue = 0; // Cannot scroll if content fits viewport
	}

	if (currentValue != oldVal) {
		MarkDirty(false);
		if (OnScroll && fireEvent) {
			OnScroll(this, currentValue);
		}
	}
}

Rect ScrollBarWidget::GetThumbRect() const {
	Rect trackAreaForThumb = GetTrackAreaRect();

	if (orientation == ScrollBarOrientation::Vertical) {
		if (trackAreaForThumb.height <= 0 || contentSize <= 0 || viewportSize <= 0 || contentSize <= viewportSize) {
			if (contentSize <= viewportSize && trackAreaForThumb.height > 0) {
				return { trackAreaForThumb.x, trackAreaForThumb.y, trackAreaForThumb.width, trackAreaForThumb.height };
			}
			return { trackAreaForThumb.x, trackAreaForThumb.y, trackAreaForThumb.width, 0 };
		}

		float thumbRatio = static_cast<float>(viewportSize) / contentSize;
		int thumbHeight = std::max(thumbMinSize, static_cast<int>(trackAreaForThumb.height * thumbRatio));
		thumbHeight = std::min(thumbHeight, trackAreaForThumb.height);

		int scrollableTrackHeight = trackAreaForThumb.height - thumbHeight;
		if (scrollableTrackHeight < 0) scrollableTrackHeight = 0;

		float valueRatio = (contentSize - viewportSize == 0) ? 0.0f : static_cast<float>(currentValue) / (contentSize - viewportSize);
		int thumbY = trackAreaForThumb.y + static_cast<int>(std::round(valueRatio * scrollableTrackHeight)); // Use round

		return { trackAreaForThumb.x, thumbY, trackAreaForThumb.width, thumbHeight };

	}
	else { // Horizontal
		if (trackAreaForThumb.width <= 0 || contentSize <= 0 || viewportSize <= 0 || contentSize <= viewportSize) {
			if (contentSize <= viewportSize && trackAreaForThumb.width > 0) {
				return { trackAreaForThumb.x, trackAreaForThumb.y, trackAreaForThumb.width, trackAreaForThumb.height };
			}
			return { trackAreaForThumb.x, trackAreaForThumb.y, 0, trackAreaForThumb.height };
		}

		float thumbRatio = static_cast<float>(viewportSize) / contentSize;
		int thumbWidth = std::max(thumbMinSize, static_cast<int>(trackAreaForThumb.width * thumbRatio));
		thumbWidth = std::min(thumbWidth, trackAreaForThumb.width);

		int scrollableTrackWidth = trackAreaForThumb.width - thumbWidth;
		if (scrollableTrackWidth < 0) scrollableTrackWidth = 0;

		float valueRatio = (contentSize - viewportSize == 0) ? 0.0f : static_cast<float>(currentValue) / (contentSize - viewportSize);
		int thumbX = trackAreaForThumb.x + static_cast<int>(std::round(valueRatio * scrollableTrackWidth)); // Use round

		return { thumbX, trackAreaForThumb.y, thumbWidth, trackAreaForThumb.height };
	}
}

// scrollbar_widget.cpp
void ScrollBarWidget::Draw(UIRenderer& renderer, float inheritedAlpha) { // Added inheritedAlpha
	if (!visible) return;

	Rect widgetScreenBounds = GetScreenBounds();

	Color finalTrackBgColor = style.backgroundColor;
	finalTrackBgColor.a *= inheritedAlpha;

	if (finalTrackBgColor.a > 0.001f) {
		renderer.DrawRect(widgetScreenBounds, finalTrackBgColor);
	}

	Point clientAreaScreenOrigin = { widgetScreenBounds.x + style.padding.left, widgetScreenBounds.y + style.padding.top };

	Color finalArrowButtonColor = style.borderColor;
	finalArrowButtonColor.a *= inheritedAlpha;

	Color finalArrowGlyphColor = style.foregroundColor;
	finalArrowGlyphColor.a *= inheritedAlpha;

	if (showArrowButtons && finalArrowButtonColor.a > 0.001f) { // Also check if glyph is visible
		if (orientation == ScrollBarOrientation::Vertical) {
			Rect upArrowRel = GetUpArrowRect();
			Rect upArrowScreen = { clientAreaScreenOrigin.x + upArrowRel.x, clientAreaScreenOrigin.y + upArrowRel.y, upArrowRel.width, upArrowRel.height };
			if (upArrowScreen.width > 0 && upArrowScreen.height > 0) {
				renderer.DrawRect(upArrowScreen, finalArrowButtonColor);
				if (finalArrowGlyphColor.a > 0.001f) DrawArrowGlyphInternal(renderer, upArrowScreen, orientation, true, finalArrowGlyphColor);
			}

			Rect downArrowRel = GetDownArrowRect();
			Rect downArrowScreen = { clientAreaScreenOrigin.x + downArrowRel.x, clientAreaScreenOrigin.y + downArrowRel.y, downArrowRel.width, downArrowRel.height };
			if (downArrowScreen.width > 0 && downArrowScreen.height > 0) {
				renderer.DrawRect(downArrowScreen, finalArrowButtonColor);
				if (finalArrowGlyphColor.a > 0.001f) DrawArrowGlyphInternal(renderer, downArrowScreen, orientation, false, finalArrowGlyphColor);
			}
		}
		else { /* Horizontal arrows with similar alpha checks */ }
	}

	if (contentSize > viewportSize && viewportSize > 0) {
		Rect thumbLocalRect = GetThumbRect();
		if (thumbLocalRect.width > 0 && thumbLocalRect.height > 0) {
			Rect thumbScreenRect = {
				clientAreaScreenOrigin.x + thumbLocalRect.x,
				clientAreaScreenOrigin.y + thumbLocalRect.y,
				thumbLocalRect.width,
				thumbLocalRect.height
			};

			Color finalThumbColor = style.foregroundColor;
			if (isDraggingThumb || hasFocus) {
				finalThumbColor.r = std::min(1.0f, finalThumbColor.r * 1.3f);
				finalThumbColor.g = std::min(1.0f, finalThumbColor.g * 1.3f);
				finalThumbColor.b = std::min(1.0f, finalThumbColor.b * 1.3f);
			}
			finalThumbColor.a *= inheritedAlpha;

			Color finalThumbBorderColor = style.borderColor;
			finalThumbBorderColor.a *= inheritedAlpha;

			if (finalThumbColor.a > 0.001f) {
				renderer.DrawRect(thumbScreenRect, finalThumbColor, true);
			}
			if (style.hasBorder && style.borderWidth > 0 && finalThumbBorderColor.a > 0.001f) {
				renderer.DrawRect(thumbScreenRect, finalThumbBorderColor, false, style.borderWidth);
			}
		}
	}

	Color finalWidgetBorderColor = style.borderColor;
	finalWidgetBorderColor.a *= inheritedAlpha;
	if (style.hasBorder && finalWidgetBorderColor.a > 0.001f) {
		renderer.DrawRect(widgetScreenBounds, finalWidgetBorderColor, false, style.borderWidth);
	}
	dirty = false;
}

Size ScrollBarWidget::Measure() {
	int baseThickness = trackThickness;
	int minInteractiveLength = thumbMinSize; // Minimum length for the thumb itself
	int totalLength = minInteractiveLength;    // Start with this

	if (showArrowButtons) {
		totalLength += 2 * arrowButtonSize; // Add space for two arrow buttons
	}

	// Ensure overall minSize is respected if larger than calculated parts
	if (orientation == ScrollBarOrientation::Vertical) {
		totalLength = std::max(minSize.height > 0 ? minSize.height - (style.padding.top + style.padding.bottom + (style.hasBorder ? 2 * (int)style.borderWidth : 0)) : 0, totalLength);
		preferredSize.width = baseThickness + style.padding.left + style.padding.right + (style.hasBorder ? 2 * (int)style.borderWidth : 0);
		preferredSize.height = totalLength + style.padding.top + style.padding.bottom + (style.hasBorder ? 2 * (int)style.borderWidth : 0);
	}
	else { // Horizontal
		totalLength = std::max(minSize.width > 0 ? minSize.width - (style.padding.left + style.padding.right + (style.hasBorder ? 2 * (int)style.borderWidth : 0)) : 0, totalLength);
		preferredSize.height = baseThickness + style.padding.top + style.padding.bottom + (style.hasBorder ? 2 * (int)style.borderWidth : 0);
		preferredSize.width = totalLength + style.padding.left + style.padding.right + (style.hasBorder ? 2 * (int)style.borderWidth : 0);
	}
	return preferredSize;
}

void ScrollBarWidget::Arrange(const Rect& finalRect) {
	Widget::Arrange(finalRect);
	// No children to arrange for a simple scrollbar
	// Parameters might need re-evaluation if size changed significantly affecting thumb ratios
	SetParameters(contentSize, viewportSize, currentValue); // Re-clamp value and mark dirty
}


void ScrollBarWidget::UpdateValueFromThumbPosition(int thumbPixelPosInTrackArea) {
	// thumbPixelPosInTrackArea is the mouse's Y (for vertical) or X (for horizontal)
	// *relative to the start of the GetTrackAreaRect()*.
	int newValue = 0;
	Rect actualTrackArea = GetTrackAreaRect();
	Rect currentThumbDim = GetThumbRect();

	if (orientation == ScrollBarOrientation::Vertical) {
		if (actualTrackArea.height <= 0 || contentSize <= viewportSize) { SetValue(0); return; }
		int thumbHeight = currentThumbDim.height;
		int scrollableSpace = actualTrackArea.height - thumbHeight;
		if (scrollableSpace <= 0) { SetValue(0); return; }
		// Adjust click position to be the desired top of the thumb
		int desiredThumbTop = thumbPixelPosInTrackArea - thumbHeight / 2;
		desiredThumbTop = std::max(0, std::min(desiredThumbTop, scrollableSpace)); // Clamp
		float valueRatio = static_cast<float>(desiredThumbTop) / scrollableSpace;
		newValue = static_cast<int>(std::round(valueRatio * (contentSize - viewportSize)));
	}
	else { // Horizontal
		if (actualTrackArea.width <= 0 || contentSize <= viewportSize) { SetValue(0); return; }
		int thumbWidth = currentThumbDim.width;
		int scrollableSpace = actualTrackArea.width - thumbWidth;
		if (scrollableSpace <= 0) { SetValue(0); return; }
		int desiredThumbLeft = thumbPixelPosInTrackArea - thumbWidth / 2;
		desiredThumbLeft = std::max(0, std::min(desiredThumbLeft, scrollableSpace));
		float valueRatio = static_cast<float>(desiredThumbLeft) / scrollableSpace;
		newValue = static_cast<int>(std::round(valueRatio * (contentSize - viewportSize)));
	}
	SetValue(newValue);
}


void ScrollBarWidget::HandleEvent(UIEvent& event) {
	if (!visible || !enabled) {
		event.handled = false;
		return;
	}

	Point mousePosInWidget = event.mouse.position;

	if (UIManager::Instance().GetCapturedWidget() == this &&
		event.eventType == UIEvent::Type::Mouse &&
		event.mouse.type == MouseEventType::Move) {

		// Only localize if we are the captured widget AND it's a move event (implies drag update)
		Rect scrollbarScreenBounds = GetScreenBounds();
		mousePosInWidget.x -= scrollbarScreenBounds.x;
		mousePosInWidget.y -= scrollbarScreenBounds.y;
	}

	Point mousePosInClient = {
		mousePosInWidget.x - style.padding.left,
		mousePosInWidget.y - style.padding.top
	};
	Rect thumbLocalRect = GetThumbRect(); // Relative to client origin, within track area

	if (event.eventType == UIEvent::Type::Mouse) {
		if (event.mouse.type == MouseEventType::Press && event.mouse.button == MouseButton::Left) {
			RequestFocus();
			bool handledByPress = false;

			if (showArrowButtons) {
				if (orientation == ScrollBarOrientation::Vertical) {
					if (GetUpArrowRect().Contains(mousePosInClient)) { SetValue(currentValue - stepSize); handledByPress = true; }
					else if (GetDownArrowRect().Contains(mousePosInClient)) { SetValue(currentValue + stepSize); handledByPress = true; }
				}
				else {
					if (GetLeftArrowRect().Contains(mousePosInClient)) { SetValue(currentValue - stepSize); handledByPress = true; }
					else if (GetRightArrowRect().Contains(mousePosInClient)) { SetValue(currentValue + stepSize); handledByPress = true; }
				}
			}

			if (!handledByPress && contentSize > viewportSize) {
				if (thumbLocalRect.width > 0 && thumbLocalRect.height > 0 &&
					thumbLocalRect.Contains(mousePosInClient)) {
					isDraggingThumb = true;
					if (orientation == ScrollBarOrientation::Vertical) {
						dragThumbClickOffset = { 0, mousePosInClient.y - thumbLocalRect.y };
					}
					else {
						dragThumbClickOffset = { mousePosInClient.x - thumbLocalRect.x, 0 };
					}
					UIManager::Instance().SetCapture(this);
					MarkDirty(false);
					handledByPress = true;
				}
				else if (GetTrackAreaRect().Contains(mousePosInClient)) {
					Rect actualTrackArea = GetTrackAreaRect();
					int clickPosInTrackArea;
					if (orientation == ScrollBarOrientation::Vertical) {
						clickPosInTrackArea = mousePosInClient.y - actualTrackArea.y;
						// For paging, compare click relative to thumb's position *within the track area*
						if (clickPosInTrackArea < thumbLocalRect.y - actualTrackArea.y) {
							SetValue(currentValue - std::max(stepSize, viewportSize));
						}
						else if (clickPosInTrackArea > thumbLocalRect.y - actualTrackArea.y + thumbLocalRect.height) {
							SetValue(currentValue + std::max(stepSize, viewportSize));
						}
						// Optional: Jump-to-click behavior:
						// UpdateValueFromThumbPosition(clickPosInTrackArea);
					}
					else {
						clickPosInTrackArea = mousePosInClient.x - actualTrackArea.x;
						if (clickPosInTrackArea < thumbLocalRect.x - actualTrackArea.x) {
							SetValue(currentValue - std::max(stepSize, viewportSize));
						}
						else if (clickPosInTrackArea > thumbLocalRect.x - actualTrackArea.x + thumbLocalRect.width) {
							SetValue(currentValue + std::max(stepSize, viewportSize));
						}
						// Optional: Jump-to-click behavior:
						// UpdateValueFromThumbPosition(clickPosInTrackArea);
					}
					handledByPress = true;
				}
			}
			if (handledByPress) event.handled = true;

		}
		else if (event.mouse.type == MouseEventType::Move) {
			if (isDraggingThumb) { // This is true
				// mousePosInWidget = event.mouse.position, e.g., (4,42) - CORRECTLY LOCAL TO SCROLLBAR
				// mousePosInClient.x = mousePosInWidget.x - style.padding.left, e.g., 4-1 = 3
				// mousePosInClient.y = mousePosInWidget.y - style.padding.top, e.g., 42-1 = 41

				Rect actualTrackArea = GetTrackAreaRect(); // Relative to client origin.
				// E.g. {x:0, y:16, w:18, h:107} if padding=1, arrowSize=16, trackThickness=16+1+1=18
				Rect currentThumbDim = GetThumbRect();     // Relative to client origin, within actualTrackArea.
				// E.g. {x:0, y:16 + some_offset, w:18, h:64}

				int mousePosOnTrackAxis; // Mouse position along the scrollbar's axis, relative to start of actualTrackArea
				int thumbSizeOnAxis;
				int trackSizeOnAxis;

				if (orientation == ScrollBarOrientation::Vertical) {
					// mousePosInClient.y = 41 (example)
					// actualTrackArea.y = 16 (example, if arrowButtonSize=16 and client.y=0)
					mousePosOnTrackAxis = mousePosInClient.y - actualTrackArea.y; // e.g., 41 - 16 = 25. THIS SHOULD BE SMALL.
					thumbSizeOnAxis = currentThumbDim.height; // e.g., 64
					trackSizeOnAxis = actualTrackArea.height; // e.g., 107
				}
				else { // Horizontal
					mousePosOnTrackAxis = mousePosInClient.x - actualTrackArea.x;
					thumbSizeOnAxis = currentThumbDim.width;
					trackSizeOnAxis = actualTrackArea.width;
				}

				int scrollableSpace = trackSizeOnAxis - thumbSizeOnAxis;

				if (scrollableSpace > 0 && (contentSize - viewportSize) > 0) {
					int desiredThumbStart;
					if (orientation == ScrollBarOrientation::Vertical) {
						// dragThumbClickOffset.y was (mousePosInClient.y - thumbLocalRect.y) at press time.
						// mousePosOnTrackAxis is (mousePosInClient.y - actualTrackArea.y)
						// thumbLocalRect.y is (actualTrackArea.y + some_offset_within_track)
						// So, dragThumbClickOffset.y is (mouse_client_y - (actualTrackArea.y + offset_in_track))
						// This needs to be: (current_mouse_on_track_axis - offset_of_click_WITHIN_thumb_BODY)
						desiredThumbStart = mousePosOnTrackAxis - dragThumbClickOffset.y;
					}
					else {
						desiredThumbStart = mousePosOnTrackAxis - dragThumbClickOffset.x;
					}

					desiredThumbStart = std::max(0, std::min(desiredThumbStart, scrollableSpace));

					float positionRatio = 0.0f;
					if (scrollableSpace > 0) { // Avoid division by zero if scrollableSpace somehow became 0
						positionRatio = static_cast<float>(desiredThumbStart) / scrollableSpace;
					}

					int newValue = static_cast<int>(std::round(positionRatio * (contentSize - viewportSize)));
					SetValue(newValue);
				}
				else if (contentSize <= viewportSize) {
					SetValue(0); // Cannot scroll
				}
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
		else if (event.mouse.type == MouseEventType::Scroll) {
			Rect localWidgetBounds = { 0, 0, bounds.width, bounds.height };
			if (localWidgetBounds.Contains(mousePosInWidget)) {
				if (contentSize > viewportSize) {
					int scrollAmount = (event.mouse.scrollDelta / WHEEL_DELTA) * stepSize;
					SetValue(currentValue - scrollAmount);
					event.handled = true;
				}
			}
		}
	}
	else if (event.eventType == UIEvent::Type::Keyboard && hasFocus) {
		if (event.keyboard.type == KeyEventType::Press && contentSize > viewportSize) {
			int lineScrollAmount = stepSize;
			int pageScrollAmount = viewportSize > 0 ? viewportSize : stepSize * 5;
			if (orientation == ScrollBarOrientation::Vertical) {
				if (event.keyboard.keyCode == KeyCode::DownArrow) { SetValue(currentValue + lineScrollAmount); event.handled = true; }
				else if (event.keyboard.keyCode == KeyCode::UpArrow) { SetValue(currentValue - lineScrollAmount); event.handled = true; }
				else if (event.keyboard.keyCode == KeyCode::PageDown) { SetValue(currentValue + pageScrollAmount); event.handled = true; }
				else if (event.keyboard.keyCode == KeyCode::PageUp) { SetValue(currentValue - pageScrollAmount); event.handled = true; }
				else if (event.keyboard.keyCode == KeyCode::Home) { SetValue(0); event.handled = true; }
				else if (event.keyboard.keyCode == KeyCode::End) { SetValue(contentSize - viewportSize); event.handled = true; }
			}
			else {
				if (event.keyboard.keyCode == KeyCode::RightArrow) { SetValue(currentValue + lineScrollAmount); event.handled = true; }
				else if (event.keyboard.keyCode == KeyCode::LeftArrow) { SetValue(currentValue - lineScrollAmount); event.handled = true; }
			}
		}
	}

	if (!event.handled) {
		Widget::HandleEvent(event);
	}
}