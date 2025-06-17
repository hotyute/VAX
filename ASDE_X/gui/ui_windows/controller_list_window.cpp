#define NOMINMAX
#include <windows.h>
#include <algorithm> 
#include <vector>
#include <string>
#include <utility> 
#include <map>     

#include "controller_list_window.h"
#include "../widgets/list_box_widget.h"
#include "../widgets/text_box_widget.h"
#include "../widgets/button_widget.h"
#include "../widgets/container_widget.h"
#include "../widgets/layout_strategy.h"
#include "../widgets/ui_manager.h"
#include "../widgets/ui_renderer.h"
#include "../widgets/font_keys.h"
#include "private_chat_window.h"

#include "../../usermanager.h" 
#include "../../main.h"        
#include "../../constants.h"   
#include "../../controller.h"  
#include "../../tools.h"       
#include "../../interfaces.h"

namespace {
    std::string PosToStringLocal(POSITIONS p) { /* ... same ... */
        switch (p) {
        case POSITIONS::OBSERVER:   return "Observer";
        case POSITIONS::DELIVERY:   return "Delivery";
        case POSITIONS::GROUND:     return "Ground";
        case POSITIONS::TOWER:      return "Tower";
        case POSITIONS::DEPARTURE:  return "Departure";
        case POSITIONS::APPROACH:   return "Approach";
        case POSITIONS::CENTER:     return "Center";
        case POSITIONS::OCEANIC:    return "Oceanic";
        case POSITIONS::FSS:        return "FSS";
        default: return "Unknown";
        }
    }
    CHAT_TYPE GetChatTypeForCtrlLine(POSITIONS pos, int rating) { /* ... same ... */
        if (rating >= 10) return CHAT_TYPE::SUP_POS;
        if (pos == POSITIONS::OBSERVER) return CHAT_TYPE::ATC;
        return CHAT_TYPE::MAIN;
    }
}

ControllerListWindow::ControllerListWindow() : WindowWidget("CONTROLLERS") {
    minSize = { 210, 300 }; // Increased height for two boxes + buttons
    SetDraggable(true);
    SetClosable(true);

    SetupUI();
    LoadState();

    controllerListBox->OnRightClickItem = std::bind(&ControllerListWindow::OnControllerRightClicked, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
}

ControllerListWindow::~ControllerListWindow() {}

void ControllerListWindow::SetupUI() {
    ContainerWidget* ca = GetContentArea();
    auto mainLayout = std::make_unique<VerticalStackLayout>();
    mainLayout->spacing = 8; // Increased spacing between main elements
    mainLayout->padding = { 5, 5, 5, 5 };
    mainLayout->childAlignment = HorizontalAlignment::Center;
    ca->SetLayout(std::move(mainLayout));

    // Button Row (same as before)
    buttonRowContainer = new ContainerWidget(std::make_unique<HorizontalStackLayout>());
    auto* buttonLayout = static_cast<HorizontalStackLayout*>(buttonRowContainer->layout.get());
    buttonLayout->spacing = 10;
    buttonLayout->padding = { 2, 0, 5, 0 };
    buttonLayout->alignment = HorizontalAlignment::Center;
    {
        alphaSortButton = new ButtonWidget("Alpha Sort");
        alphaSortButton->preferredSize = { 90, 22 }; alphaSortButton->minSize = { 80, 22 };
        alphaSortButton->OnClick = std::bind(&ControllerListWindow::OnAlphaSortClicked, this, std::placeholders::_1);
        buttonRowContainer->AddChild(std::unique_ptr<ButtonWidget>(alphaSortButton));

        pofOnlyButton = new ButtonWidget("POF Only");
        pofOnlyButton->preferredSize = { 90, 22 }; pofOnlyButton->minSize = { 80, 22 };
        pofOnlyButton->SetEnabled(false);
        pofOnlyButton->OnClick = std::bind(&ControllerListWindow::OnPOFOnlyClicked, this, std::placeholders::_1);
        buttonRowContainer->AddChild(std::unique_ptr<ButtonWidget>(pofOnlyButton));
    }
    ca->AddChild(std::unique_ptr<ContainerWidget>(buttonRowContainer));

    // Controller ListBox
    controllerListBox = new ListBoxWidget();
    controllerListBox->id = "controller_list_main";
    controllerListBox->minSize = { 200, 150 }; // Min height for the list itself
    controllerListBox->style.fontKey = FONT_KEY_CONFIG;
    controllerListBox->OnItemSelected = std::bind(&ControllerListWindow::OnControllerSelected, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    ca->AddChild(std::unique_ptr<ListBoxWidget>(controllerListBox));

    // Controller Info TextBox
    controllerInfoBox = new TextBoxWidget();
    controllerInfoBox->id = "controller_list_info";
    controllerInfoBox->minSize = { 200, 80 }; // Min height for info box
    controllerInfoBox->style.fontKey = FONT_KEY_CONFIG;
    controllerInfoBox->SetReadOnly(true);
    controllerInfoBox->SetMaxHistory(10); // Info box doesn't need long history
    ca->AddChild(std::unique_ptr<TextBoxWidget>(controllerInfoBox));
}

void ControllerListWindow::NotifyDataMightHaveChanged() {
    RefreshControllerList();
}

void ControllerListWindow::LoadState() {
    RefreshControllerList();
    UpdateControllerInfoBox(nullptr); // Clear info box initially
}

void ControllerListWindow::OnAlphaSortClicked(Widget* sender) {
    if (currentSortOrder == ControllerSortOrder::Alphabetical) {
        currentSortOrder = ControllerSortOrder::Default;
        alphaSortButton->SetText("Alpha Sort");
    }
    else {
        currentSortOrder = ControllerSortOrder::Alphabetical;
        alphaSortButton->SetText("Pos Sort");
    }
    RefreshControllerList();
}

void ControllerListWindow::OnPOFOnlyClicked(Widget* sender) {
    sendSystemMessage("POF Only filter not yet implemented.");
}

void ControllerListWindow::OnControllerSelected(ListBoxWidget* sender, int selectedIndex, const std::string& selectedText) {
    if (selectedIndex >= 0 && selectedIndex < (int)currentlyDisplayedControllers.size()) {
        currentlySelectedController = currentlyDisplayedControllers[selectedIndex];
        UpdateControllerInfoBox(currentlySelectedController);
    }
    else {
        currentlySelectedController = nullptr;
        UpdateControllerInfoBox(nullptr);
    }
}

void ControllerListWindow::RefreshControllerList() {
    std::string previouslySelectedCallsign;
    if (currentlySelectedController && currentlySelectedController->getIdentity()) {
        previouslySelectedCallsign = currentlySelectedController->getIdentity()->callsign;
    }

    currentlyDisplayedControllers.clear(); // Clear the old mapping
    currentlySelectedController = nullptr; // Reset selection before repopulating

    std::vector<Controller*> visibleControllersFiltered; // Temporary list for sorting

    if (USER) {
        for (auto const& [callsign, controller_ptr] : controller_map) {
            if (controller_ptr && controller_ptr->getIdentity()) {
                if (dist(USER->getLatitude(), USER->getLongitude(),
                    controller_ptr->getLatitude(), controller_ptr->getLongitude())
                    <= USER->getVisibility()) {
                    // if (pofOnlyFilterActive && !IsControllerPOF(controller_ptr)) continue; 
                    visibleControllersFiltered.push_back(controller_ptr);
                }
            }
        }
    }

    if (currentSortOrder == ControllerSortOrder::Alphabetical) {
        std::sort(visibleControllersFiltered.begin(), visibleControllersFiltered.end(),
            [](Controller* a, Controller* b) {
                if (!a || !a->getIdentity()) return false;
                if (!b || !b->getIdentity()) return true;
                return a->getIdentity()->callsign < b->getIdentity()->callsign;
            });
        // After sorting, directly use visibleControllersFiltered for currentlyDisplayedControllers
        currentlyDisplayedControllers = visibleControllersFiltered;
    }
    else { // Default (Positional Sort)
        std::map<POSITIONS, std::vector<Controller*>> controllersByPosition;
        for (Controller* c : visibleControllersFiltered) {
            if (c && c->getIdentity()) {
                controllersByPosition[c->getIdentity()->controller_position].push_back(c);
            }
        }
        // Build currentlyDisplayedControllers in position order, then alpha within position
        for (int i = 0; i <= static_cast<int>(POSITIONS::FSS); ++i) {
            POSITIONS currentPosEnum = static_cast<POSITIONS>(i);
            auto it = controllersByPosition.find(currentPosEnum);
            if (it != controllersByPosition.end() && !it->second.empty()) {
                std::vector<Controller*>& group = it->second;
                std::sort(group.begin(), group.end(), [](Controller* a, Controller* b) {
                    if (!a || !a->getIdentity()) return true;
                    if (!b || !b->getIdentity()) return false;
                    return a->getIdentity()->callsign < b->getIdentity()->callsign;
                    });
                for (Controller* controller : group) {
                    currentlyDisplayedControllers.push_back(controller);
                }
            }
        }
    }

    // Now build the lines for the ListBox
    std::vector<std::pair<std::string, CHAT_TYPE>> linesForListBox;
    bool controllersActuallyAddedToDisplay = false;

    if (currentSortOrder == ControllerSortOrder::Alphabetical) {
        if (!currentlyDisplayedControllers.empty()) controllersActuallyAddedToDisplay = true;
        for (Controller* controller : currentlyDisplayedControllers) {
            // Format line for ListBox (might be more concise than the TextBox version)
            Identity* id = controller->getIdentity();
            std::string lineText = id->callsign;
            // Optionally add rating or freq if space permits and desired in the list itself
            // lineText += " (" + std::string(CONTROLLER_RATINGS[id->controller_rating]) + ")";
            linesForListBox.push_back({ lineText, GetChatTypeForCtrlLine(id->controller_position, id->controller_rating) });
        }
    }
    else { // Default (Positional) - Rebuild with headers for ListBox
        std::map<POSITIONS, std::vector<Controller*>> controllersByPosition;
        for (Controller* c : currentlyDisplayedControllers) { // Use already sorted (by overall, then alpha) list
            if (c && c->getIdentity()) controllersByPosition[c->getIdentity()->controller_position].push_back(c);
        }

        // Temporarily rebuild currentlyDisplayedControllers to match the ListBox items including headers
        std::vector<Controller*> finalListBoxOrderControllers;
        linesForListBox.clear(); // Start fresh for listbox lines

        for (int i = 0; i <= static_cast<int>(POSITIONS::FSS); ++i) {
            POSITIONS currentPosEnum = static_cast<POSITIONS>(i);
            auto it = controllersByPosition.find(currentPosEnum);
            if (it != controllersByPosition.end() && !it->second.empty()) {
                if (!controllersActuallyAddedToDisplay) controllersActuallyAddedToDisplay = true;
                std::string positionNameStr = PosToStringLocal(currentPosEnum);
                std::transform(positionNameStr.begin(), positionNameStr.end(), positionNameStr.begin(), ::toupper);
                linesForListBox.push_back({ "---- " + positionNameStr + " ----", CHAT_TYPE::SYSTEM });
                finalListBoxOrderControllers.push_back(nullptr); // Placeholder for header

                for (Controller* controller : it->second) { // Already sorted alphabetically within group
                    Identity* id = controller->getIdentity();
                    std::string lineText = id->callsign;
                    linesForListBox.push_back({ lineText, GetChatTypeForCtrlLine(id->controller_position, id->controller_rating) });
                    finalListBoxOrderControllers.push_back(controller);
                }
            }
        }
        currentlyDisplayedControllers = finalListBoxOrderControllers; // Update with listbox structure
    }


    if (!controllersActuallyAddedToDisplay) {
        if (controller_map.empty()) {
            linesForListBox.push_back({ "No controllers online.", CHAT_TYPE::SYSTEM });
            currentlyDisplayedControllers.push_back(nullptr);
        }
        else {
            linesForListBox.push_back({ "No controllers in range.", CHAT_TYPE::SYSTEM });
            currentlyDisplayedControllers.push_back(nullptr);
        }
    }

    UpdateListBox(linesForListBox);

    // Try to reselect previously selected controller
    int newSelectedIndex = -1;
    if (!previouslySelectedCallsign.empty()) {
        for (size_t i = 0; i < currentlyDisplayedControllers.size(); ++i) {
            if (currentlyDisplayedControllers[i] && currentlyDisplayedControllers[i]->getIdentity() &&
                currentlyDisplayedControllers[i]->getIdentity()->callsign == previouslySelectedCallsign) {
                newSelectedIndex = i;
                break;
            }
        }
    }

    if (controllerListBox) {
        controllerListBox->selectedIndex = newSelectedIndex; // Directly set, ListBox won't auto-scroll here
        if (newSelectedIndex != -1) {
            controllerListBox->hoverIndex = newSelectedIndex;
            // Manually ensure visible if selection is restored
            Rect clientBounds = controllerListBox->GetClientBounds();
            int itemTopY = newSelectedIndex * controllerListBox->itemHeight;
            int itemBottomY = itemTopY + controllerListBox->itemHeight;
            if (clientBounds.height > 0) {
                if (itemTopY < controllerListBox->scrollOffsetY) {
                    controllerListBox->SetScrollOffset(itemTopY);
                }
                else if (itemBottomY > controllerListBox->scrollOffsetY + clientBounds.height) {
                    controllerListBox->SetScrollOffset(itemBottomY - clientBounds.height);
                }
            }

        }
        else {
            controllerListBox->hoverIndex = -1;
        }
    }

    if (newSelectedIndex != -1) {
        currentlySelectedController = currentlyDisplayedControllers[newSelectedIndex];
    }
    else {
        currentlySelectedController = nullptr;
    }
    UpdateControllerInfoBox(currentlySelectedController);
}


void ControllerListWindow::UpdateListBox(const std::vector<std::pair<std::string, CHAT_TYPE>>& formattedLines) {
    if (!controllerListBox) return;
    controllerListBox->ClearItems();

    for (const auto& linePair : formattedLines) {
        // ListBoxWidget doesn't directly support CHAT_TYPE for coloring its items.
        // If you need per-item coloring in ListBox, ListBoxWidget::Draw would need modification
        // to take color from an auxiliary structure or by parsing the text.
        // For now, all ListBox items will use the ListBox's default style.
        controllerListBox->AddItem(linePair.first);
    }
    MarkDirty(true);
}

void ControllerListWindow::UpdateControllerInfoBox(Controller* selectedController) {
    if (!controllerInfoBox) return;
    controllerInfoBox->Clear();

    if (selectedController && selectedController->getIdentity()) {
        Identity* id = selectedController->getIdentity();
        std::string ratingStr = (id->controller_rating >= 0 && id->controller_rating < NUM_CONTROLLER_RATINGS) ?
            CONTROLLER_RATINGS[id->controller_rating] : "N/A";

        Color defaultColor = controllerInfoBox->style.foregroundColor;

        controllerInfoBox->AddLine(id->callsign, defaultColor);
        controllerInfoBox->AddLine(id->login_name + " (" + ratingStr + ")", defaultColor);

        std::string freqText = "Freq: ";
        if (selectedController->userdata.frequency[0] != 99998 && selectedController->userdata.frequency[0] != 0) {
            freqText += frequency_to_string(selectedController->userdata.frequency[0]);
        }
        else {
            freqText += "---.---";
        }
        controllerInfoBox->AddLine(freqText, defaultColor);
        controllerInfoBox->AddLine("Vis Range: " + std::to_string(selectedController->getVisibility()), defaultColor);
        // Add more info as needed
    }
    else {
        controllerInfoBox->AddLine("No controller selected.", controllerInfoBox->style.foregroundColor);
    }
    MarkDirty(true);
}

void ControllerListWindow::OnControllerRightClicked(ListBoxWidget* sender, int itemIndex,
    const std::string& itemText, Point screenPos) {
    CloseContextMenu(); // Close any existing context menu first

    if (itemIndex >= 0 && itemIndex < (int)currentlyDisplayedControllers.size()) {
        Controller* target = currentlyDisplayedControllers[itemIndex];
        if (target && target->getIdentity()) { // Ensure it's not a header and is a valid controller
            OpenContextMenu(target, screenPos);
        }
    }
}

void ControllerListWindow::OpenContextMenu(Controller* targetController, Point screenPos) {
    if (!targetController || !targetController->getIdentity()) return;

    contextMenuTargetController = targetController; // Store for the callback

    // Create a simple popup window for the menu
    // A more robust solution would use a dedicated ContextMenuWidget class
    contextMenu = new WindowWidget("Actions"); // Raw new for now, owned by this class
    contextMenu->id = this->id + "_ctx_menu";
    contextMenu->hasTitleBar = false;
    contextMenu->style.backgroundColor = { 0.2f, 0.2f, 0.2f, 0.95f };
    contextMenu->style.borderColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    contextMenu->style.padding = { 2, 2, 2, 2 };

    ContainerWidget* content = contextMenu->GetContentArea();
    auto menuLayout = std::make_unique<VerticalStackLayout>();
    menuLayout->spacing = 1;
    menuLayout->padding = { 1,1,1,1 };
    content->SetLayout(std::move(menuLayout));

    auto openChatButton = std::make_unique<ButtonWidget>("Open Chat with " + targetController->getIdentity()->callsign);
    openChatButton->style = Style::DefaultButton(); // Use default button style
    openChatButton->style.padding = { 3, 5, 3, 5 };
    openChatButton->OnClick = std::bind(&ControllerListWindow::OnContextMenuOpenChatClicked, this, std::placeholders::_1);
    content->AddChild(std::move(openChatButton));

    // Add more menu items here if needed

    // Position and size the context menu
    contextMenu->Measure();
    Size menuSize = contextMenu->preferredSize;

    // Ensure menu stays on screen
    int finalX = screenPos.x;
    int finalY = screenPos.y;
    if (finalX + menuSize.width > UIManager::Instance().GetRenderer()->GetTextExtent("", "").width) { // HACK: Get screen width
        finalX = screenPos.x - menuSize.width;
    }
    if (finalY + menuSize.height > UIManager::Instance().GetRenderer()->GetTextExtent("", "").height) { // HACK: Get screen height
        finalY = screenPos.y - menuSize.height;
    }
    finalX = std::max(0, finalX);
    finalY = std::max(0, finalY);


    contextMenu->Arrange({ finalX, finalY, menuSize.width, menuSize.height });

    // Add it as a child of THIS window, or directly to UIManager
    // Adding to UIManager is usually better for popups.
    UIManager::Instance().AddWindow(std::unique_ptr<WindowWidget>(contextMenu)); // UIManager takes ownership
    contextMenu->RequestFocus(); // Focus the menu itself or its first item
}

void ControllerListWindow::CloseContextMenu() {
    if (contextMenu) {
        UIManager::Instance().RemoveWindow(contextMenu); // UIManager handles deletion
        contextMenu = nullptr;
        contextMenuTargetController = nullptr;
    }
}

void ControllerListWindow::OnContextMenuOpenChatClicked(Widget* sender) {
    if (contextMenuTargetController && contextMenuTargetController->getIdentity()) {
        std::string targetCallsign = contextMenuTargetController->getIdentity()->callsign;
        CloseContextMenu(); // Close menu before opening chat

        if (USER && USER->getIdentity() && USER->getIdentity()->callsign == targetCallsign) {
            sendErrorMessage("Cannot open private chat with yourself.");
            return;
        }

        std::string windowId = "PrivChat_" + targetCallsign;
        // Use the ShowOrCreateInstance logic (now assumed to be in main.cpp or UIManager)
        // This part needs to call the global/UIManager function for opening private chats.
        // For example, if you had a function in your main scope:
        // OpenPrivateChatWindow(targetCallsign); 
        // Or directly using UIManager if PrivateChatWindow has a simple constructor:

        PrivateChatWindow* pmWin = UIManager::ShowOrCreateInstance<PrivateChatWindow>(
            windowId,
            -1,
            targetCallsign
        );
        // If pmWin is null, ShowOrCreateInstance might have failed (e.g., UI not ready)
        // or it could mean the window already existed and was brought to front.
    }
    else {
        CloseContextMenu();
    }
}

void ControllerListWindow::HandleEvent(UIEvent& event) {
    // If context menu is open and the event is a click outside of it, close it.
    if (contextMenu && contextMenu->visible && event.eventType == UIEvent::Type::Mouse && event.mouse.type == MouseEventType::Press) {
        Rect menuScreenBounds = contextMenu->GetScreenBounds();
        if (!menuScreenBounds.Contains(event.mouse.raw_position)) { // Use screen_position from UIManager
            // Check if the click was on this ControllerListWindow itself (outside the context menu)
            // or any other window.
            bool clickOnSelfOrChild = false;
            Rect selfScreenBounds = this->GetScreenBounds();
            if (selfScreenBounds.Contains(event.mouse.raw_position)) {
                // Click might be on controller list box itself, let its event handling proceed
                // but we still want to close the context menu.
            }
            CloseContextMenu();
            // Don't mark event as handled here necessarily, let the click proceed to whatever was under it.
            // However, if the context menu was the only thing to react, then maybe.
            // For now, just close and let event propagate.
        }
    }

    WindowWidget::HandleEvent(event); // Standard handling for dragging, closing main window, child events
    // No specific Escape key handling here for context menu; UIManager/WindowWidget default close for menu works.
}