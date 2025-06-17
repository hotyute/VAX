// --- START OF FILE controller_list_window.h ---
#pragma once
#include "../widgets/window_widget.h"
#include <vector>
#include <string>
#include <utility> // For std::pair

// Forward declarations for UI widgets
class ListBoxWidget;  // Changed from TextBoxWidget for the list
class TextBoxWidget;  // For the info display
class ButtonWidget;
class ContainerWidget;
class Controller;     // Forward declare from your application (controller.h)

#include "../../gui.h" // For CHAT_TYPE 

enum class ControllerSortOrder {
    Default,
    Alphabetical
};

class ControllerListWindow : public WindowWidget {
public:
    ControllerListWindow();
    ~ControllerListWindow();

    void NotifyDataMightHaveChanged();
    void LoadState();

    void HandleEvent(UIEvent& event) override;

private:
    void SetupUI();
    void OnAlphaSortClicked(Widget* sender);
    void OnPOFOnlyClicked(Widget* sender);
    void OnControllerSelected(ListBoxWidget* sender, int selectedIndex, const std::string& selectedText);

    void RefreshControllerList();  // Renamed from RefreshListDisplay for clarity
    void UpdateListBox(const std::vector<std::pair<std::string, CHAT_TYPE>>& formattedLines);
    void UpdateControllerInfoBox(Controller* selectedController);

    ListBoxWidget* controllerListBox;     // For the list of controllers
    TextBoxWidget* controllerInfoBox;     // For displaying selected controller's details

    ButtonWidget* alphaSortButton;
    ButtonWidget* pofOnlyButton;
    ContainerWidget* buttonRowContainer;

    ControllerSortOrder currentSortOrder = ControllerSortOrder::Default;
    bool pofOnlyFilterActive = false;

    void OnControllerRightClicked(ListBoxWidget* sender, int itemIndex, const std::string& itemText, Point screenPos);
    void OpenContextMenu(Controller* targetController, Point screenPos);
    void CloseContextMenu();
    void OnContextMenuOpenChatClicked(Widget* sender);

    // Context Menu related members
    WindowWidget* contextMenu = nullptr; // The popup window for the menu
    Controller* contextMenuTargetController = nullptr; // Controller the menu is for

    std::vector<Controller*> currentlyDisplayedControllers; // To map ListBox index to Controller*
    Controller* currentlySelectedController = nullptr;
};