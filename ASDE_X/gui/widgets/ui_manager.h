#pragma once
#include "ui_common.h"
#include "../ui_windows/flight_plan_window.h"
#include <vector>
#include <memory>
#include <string> // For GetWindowById

// Forward declarations
class WindowWidget;
class Widget;
class UIRenderer;
class FlightPlanWindow;

class UIManager {
public:
    // Singleton access
    static UIManager& Instance();

    void Initialize(std::unique_ptr<UIRenderer> renderer, int screenWidth, int screenHeight);
    void Shutdown();

    void AddWindow(std::unique_ptr<WindowWidget> window);
    WindowWidget* GetWindowById(const std::string& id); // Find a top-level window by its ID
    void RemoveWindow(WindowWidget* window);
    void BringWindowToFront(WindowWidget* window);

    void ProcessSystemEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void Update(float deltaTime);
    void Render();

    void SetFocus(Widget* widget);
    Widget* GetFocusedWidget() const { return focusedWidget; }

    Widget* WidgetAtScreenPoint(const Point& screenPos, Widget* topLevelContext = nullptr);

    Widget* GetCapturedWidget() const { return capturedWidget; }
    void SetCapture(Widget* widget);
    void ReleaseCapture();

    UIRenderer* GetRenderer() const { return renderer.get(); }
    bool IsInitialized() const { return renderer != nullptr; } // Checks if renderer is set up
    bool LastEventHandled() const { return lastEventHandledByUI_INTERNAL; } // So external code can query

    void MarkAllWindowsDirty();

    FlightPlanWindow* GetFlightPlanWindowForAircraft(Aircraft* aircraft);

private:
    UIManager() = default;
    ~UIManager() = default;
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    std::vector<std::unique_ptr<WindowWidget>> windows;
    Widget* focusedWidget = nullptr;
    Widget* mouseHoverWidget = nullptr;
    Widget* capturedWidget = nullptr;

    std::unique_ptr<UIRenderer> renderer;
    int screenWidth = 0, screenHeight = 0;
    bool leftMouseDown = false;
    Point lastMousePosition;

    // Internal flag to track if the last system event was consumed by the UI.
    // This helps the main WindowProcedure decide if it needs to do further default processing.
    bool lastEventHandledByUI_INTERNAL = false;
    friend class WindowProcedure; // Or make a public getter if preferred over friend
};