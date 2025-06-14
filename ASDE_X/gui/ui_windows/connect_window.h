#pragma once
#include "../widgets/window_widget.h" // Path to your new UI library

// Forward declare widgets if not including all headers
class LabelWidget;
class InputFieldWidget;
class ButtonWidget;
class ComboBoxWidget;
// Forward declare from your app
struct Identity; // From user.h
extern class Controller* USER; // From usermanager.h (if still global)

class ConnectWindow : public WindowWidget {
public:
    ConnectWindow();

private:
    void OnConnectClicked(Widget* sender);
    void OnCancelClicked(Widget* sender);
    void LoadDataFromIdentity(Identity& id); // Helper

    // Pointers to widgets we need to interact with
    InputFieldWidget* callsignInput;
    InputFieldWidget* fullNameInput;
    InputFieldWidget* usernameInput;
    InputFieldWidget* passwordInput;
    ComboBoxWidget* ratingCombo;
    ComboBoxWidget* positionCombo;
};