#pragma once
#include "../widgets/window_widget.h"
#include <string>

// Forward declarations
class TextBoxWidget;
class InputFieldWidget;

class PrivateChatWindow : public WindowWidget {
public:
    // Constructor takes the callsign of the other participant in the chat
    PrivateChatWindow(const std::string& otherParticipantCallsign);
    ~PrivateChatWindow();

    // Adds a message to the chat display
    // sender: Callsign of the message sender
    // message: The message content
    // isOutgoing: True if the message is from the local user, false if received
    void AddMessage(const std::string& sender, const std::string& message, bool isOutgoing);

    const std::string& GetOtherParticipantCallsign() const { return otherParticipantCallsign; }

    void HandleEvent(UIEvent& event) override; // For Escape key to close, etc.

private:
    void SetupUI();
    void OnInputEnterPressed(InputFieldWidget* sender);
    void LoadMessageHistory(); // Optional: If you implement message history loading

    std::string otherParticipantCallsign;

    TextBoxWidget* messageDisplayBox;
    InputFieldWidget* messageInput;
};