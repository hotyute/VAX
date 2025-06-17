#define NOMINMAX
#include <windows.h>
#include <chrono> // For timestamp
#include <iomanip> // For std::put_time
#include <sstream> // For std::ostringstream

#include "private_chat_window.h"
#include "../widgets/text_box_widget.h"
#include "../widgets/input_field_widget.h"
#include "../widgets/layout_strategy.h"
#include "../widgets/ui_manager.h"
#include "../widgets/font_keys.h"

// Application-specific includes
#include "../../usermanager.h" // For USER (local user's callsign)
#include "../../packets.h"     // For sendPrivateMessage
#include "../../main.h"        // For connected flag, sendSystemMessage
#include "../../tools.h"       // For s2ws if needed, currentDateTime if you want full date

PrivateChatWindow::PrivateChatWindow(const std::string& otherCallsign)
    : WindowWidget("PRIVATE CHAT: " + otherCallsign), otherParticipantCallsign(otherCallsign) {

    minSize = { 350, 200 };
    // preferredSize = {400, 250}; 
    SetDraggable(true);
    SetClosable(true); // Default close button will call UIManager::RemoveWindow

    SetupUI();
    // LoadMessageHistory(); // Call if you implement history loading
    messageInput->RequestFocus();
}

PrivateChatWindow::~PrivateChatWindow() {
    // UI elements managed by WindowWidget
}

void PrivateChatWindow::SetupUI() {
    ContainerWidget* ca = GetContentArea();
    auto mainLayout = std::make_unique<VerticalStackLayout>();
    mainLayout->spacing = 5;
    mainLayout->padding = { 5, 5, 5, 5 };
    ca->SetLayout(std::move(mainLayout));

    // Message Display Box
    messageDisplayBox = new TextBoxWidget();
    messageDisplayBox->id = "pm_display_" + otherParticipantCallsign;
    messageDisplayBox->minSize = { 300, 120 };
    messageDisplayBox->style.fontKey = FONT_KEY_UI_LABEL; // Or a dedicated chat font
    messageDisplayBox->SetReadOnly(true);
    messageDisplayBox->pruneTop = false; // Add new messages to bottom
    messageDisplayBox->SetMaxHistory(100);
    ca->AddChild(std::unique_ptr<TextBoxWidget>(messageDisplayBox));

    // Message Input Field
    messageInput = new InputFieldWidget();
    messageInput->id = "pm_input_" + otherParticipantCallsign;
    messageInput->minSize = { 300, 20 };
    messageInput->preferredSize = { 0, 22 }; // Width will be stretched by layout, height fixed
    messageInput->style.fontKey = FONT_KEY_UI_LABEL;
    messageInput->OnEnterPressed = std::bind(&PrivateChatWindow::OnInputEnterPressed, this, std::placeholders::_1);
    ca->AddChild(std::unique_ptr<InputFieldWidget>(messageInput));
}

void PrivateChatWindow::AddMessage(const std::string& sender, const std::string& message, bool isOutgoing) {
    if (!messageDisplayBox) return;

    // Get current time for timestamp
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm timeinfo;
    localtime_s(&timeinfo, &in_time_t); // Use localtime_s for safety

    std::ostringstream timeStream;
    timeStream << std::put_time(&timeinfo, "%H:%M:%S");

    std::string formattedMessage = "[" + timeStream.str() + "] " + sender + ": " + message;

    Color messageColor = messageDisplayBox->style.foregroundColor; // Default
    if (isOutgoing) {
        // Optionally, a different color for outgoing messages
        // messageColor = {0.2f, 0.6f, 0.2f, 1.0f}; // Darker green example
    }
    else {
        // Optionally, a different color for incoming messages
        // messageColor = {0.2f, 0.2f, 0.6f, 1.0f}; // Darker blue example
        // Play sound for incoming PM
        PlaySound(MAKEINTRESOURCE(IDW_SOUND4), NULL, SND_RESOURCE | SND_ASYNC | SND_NODEFAULT);

    }

    messageDisplayBox->AddLine(formattedMessage, messageColor);
    MarkDirty(true);
}

void PrivateChatWindow::OnInputEnterPressed(InputFieldWidget* sender) {
    if (!messageInput || !USER || !USER->getIdentity()) return;

    std::string textToSend = messageInput->GetText();
    if (textToSend.empty()) return;

    if (connected) {
        sendPrivateMessage(otherParticipantCallsign, textToSend); // Your existing network function
        AddMessage(USER->getIdentity()->callsign, textToSend, true); // Display sent message
    }
    else {
        AddMessage("SYSTEM", "Not connected. Message not sent.", false);
        // Or: sendErrorMessage("Not connected. Message not sent."); if it logs to a main console
    }
    messageInput->SetText(""); // Clear input field
    messageInput->RequestFocus(); // Keep focus on input
}

void PrivateChatWindow::LoadMessageHistory() {
    // Placeholder for loading message history if you implement it
    // For example, from USER->getPrivateMsgs() or a local log file.
    // This would involve iterating through historical messages and calling AddMessage().
    // If (USER) {
    //     auto& pmHistory = USER->getPrivateMsgs().message_history;
    //     User* otherUser = nullptr; // You'd need to find the User* for otherParticipantCallsign
    //     for (auto const& [user, chatLine] : pmHistory) {
    //         if (user && user->getCallsign() == otherParticipantCallsign) {
    //             otherUser = user;
    //             break;
    //         }
    //     }
    //     if (otherUser) {
    //         ChatLine* currentLine = pmHistory[otherUser];
    //         std::vector<std::pair<std::string, std::string>> historyLog; // sender, message
    //         while(currentLine) {
    //             // This assumes ChatLine stores sender and message separately, or parse from getText()
    //             // You'd need to adapt this based on how your old ChatLine worked.
    //             // For now, this part is highly conceptual.
    //             // historyLog.push_back({sender_from_chatline, message_from_chatline});
    //             // currentLine = currentLine->prev_in_history; // If you have such a link
    //         }
    //         std::reverse(historyLog.begin(), historyLog.end());
    //         for(const auto& entry : historyLog) {
    //             AddMessage(entry.first, entry.second, entry.first == USER->getCallsign());
    //         }
    //     }
    // }
}

void PrivateChatWindow::HandleEvent(UIEvent& event) {
    WindowWidget::HandleEvent(event);
    if (event.handled) return;

    if (event.eventType == UIEvent::Type::Keyboard &&
        event.keyboard.type == KeyEventType::Press &&
        event.keyboard.keyCode == KeyCode::Escape) {
        // Default close button behavior will remove the window
        // If you want custom behavior on Escape, handle it here.
        // For now, let UIManager handle close if the window is closable.
        // UIManager::Instance().RemoveWindow(this); // This would be handled by WindowWidget's default close
        event.handled = true;
    }
}