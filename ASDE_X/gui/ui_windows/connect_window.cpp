#include "connect_window.h"
#include "../widgets/label_widget.h"
#include "../widgets/input_field_widget.h"
#include "../widgets/button_widget.h"
#include "../widgets/combo_box_widget.h"
#include "../widgets/layout_strategy.h" // For VerticalStackLayout, HorizontalStackLayout
#include "../widgets/ui_manager.h"     // For UIManager::Instance().RemoveWindow()

// For existing app logic/data (replace with proper dependencies/event system later)
#include "../../usermanager.h" // For USER
#include "../../main.h"        // For connect(), sendErrorMessage()
#include "../../constants.h"   // For CONTROLLER_RATINGS, POSITIONS enum to string
#include "../../tools.h"       // For string manipulation if needed
#include "../../interfaces.h"

// Helper to convert POSITIONS enum to string (or get from constants.h if you have arrays)
std::string PositionToString(POSITIONS p) {
    switch (p) {
        case POSITIONS::OBSERVER: return "Observer";
        case POSITIONS::DELIVERY: return "Delivery";
        case POSITIONS::GROUND: return "Ground";
        case POSITIONS::TOWER: return "Tower";
        case POSITIONS::DEPARTURE: return "Departure";
        case POSITIONS::APPROACH: return "Approach";
        case POSITIONS::CENTER: return "Center";
        case POSITIONS::OCEANIC: return "Oceanic";
        case POSITIONS::FSS: return "FSS";
        default: return "Unknown";
    }
}


// connect_window.cpp

ConnectWindow::ConnectWindow() : WindowWidget("CONNECT") {
    SetDraggable(true);
    SetClosable(true);
    // Increased minSize.width to give more space. We'll refine this if needed.
    // Based on:
    // ComboBox text like "Supervisor" + padding for arrow can lead to ~150px preferred width.
    // Two such combos: 150 + 10 (spacing) + 150 = 310px.
    // contentAreaLayout padding: 10 (left) + 10 (right) = 20px.
    // Total contentArea width needed: 310 + 20 = 330px.
    // Window border (1px+1px) = 2px.
    // Minimum window width: 330 + 2 = 332px. Let's use 340 for some margin.
    minSize = { 300, 230 }; // Adjusted width, height might also need adjustment

    // Get the contentArea created by WindowWidget's constructor
    ContainerWidget* ca = GetContentArea();
    if (ca) {
        auto contentAreaLayout = std::make_unique<VerticalStackLayout>();
        contentAreaLayout->spacing = 10;
        contentAreaLayout->padding = { 20, 40, 10, 40 };
        contentAreaLayout->childAlignment = HorizontalAlignment::Center;
        ca->SetLayout(std::move(contentAreaLayout));
    }
    // All children (rows) will be added to 'contentArea' (which is 'ca', and is equivalent to this->contentArea)

    // Row 1: Callsign Label & Input, FullName Label & Input
    auto row1 = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(row1->layout.get())->spacing = 5;
    {
        auto callsignLabelPair = std::make_unique<ContainerWidget>(std::make_unique<VerticalStackLayout>());
        static_cast<VerticalStackLayout*>(callsignLabelPair->layout.get())->spacing = 2;
        callsignLabelPair->AddChild(std::make_unique<LabelWidget>("Callsign:"));
        callsignInput = new InputFieldWidget();
        callsignInput->id = "connect_callsign";
        callsignInput->preferredSize = { 120, 20 }; // This acts like a min/default preferred
        callsignInput->minSize = { 120,20 };        // Explicitly set minSize too
        callsignLabelPair->AddChild(std::unique_ptr<InputFieldWidget>(callsignInput));
        row1->AddChild(std::move(callsignLabelPair));

        auto fullNameLabelPair = std::make_unique<ContainerWidget>(std::make_unique<VerticalStackLayout>());
        static_cast<VerticalStackLayout*>(fullNameLabelPair->layout.get())->spacing = 2;
        fullNameLabelPair->AddChild(std::make_unique<LabelWidget>("Full Name:"));
        fullNameInput = new InputFieldWidget();
        fullNameInput->id = "connect_fullname";
        fullNameInput->preferredSize = { 120, 20 };
        fullNameInput->minSize = { 120,20 };
        fullNameLabelPair->AddChild(std::unique_ptr<InputFieldWidget>(fullNameInput));
        row1->AddChild(std::move(fullNameLabelPair));
    }
    contentArea->AddChild(std::move(row1));

    // Row 2: Username Label & Input, Password Label & Input
    auto row2 = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(row2->layout.get())->spacing = 5;
    {
        auto userLabelPair = std::make_unique<ContainerWidget>(std::make_unique<VerticalStackLayout>());
        static_cast<VerticalStackLayout*>(userLabelPair->layout.get())->spacing = 2;
        userLabelPair->AddChild(std::make_unique<LabelWidget>("Username:"));
        usernameInput = new InputFieldWidget();
        usernameInput->id = "connect_username";
        usernameInput->preferredSize = { 120, 20 };
        usernameInput->minSize = { 120,20 };
        userLabelPair->AddChild(std::unique_ptr<InputFieldWidget>(usernameInput));
        row2->AddChild(std::move(userLabelPair));

        auto passLabelPair = std::make_unique<ContainerWidget>(std::make_unique<VerticalStackLayout>());
        static_cast<VerticalStackLayout*>(passLabelPair->layout.get())->spacing = 2;
        passLabelPair->AddChild(std::make_unique<LabelWidget>("Password:"));
        passwordInput = new InputFieldWidget();
        passwordInput->id = "connect_password";
        passwordInput->isPassword = true;
        passwordInput->preferredSize = { 120, 20 };
        passwordInput->minSize = { 120,20 };
        passLabelPair->AddChild(std::unique_ptr<InputFieldWidget>(passwordInput));
        row2->AddChild(std::move(passLabelPair));
    }
    contentArea->AddChild(std::move(row2));

    // Row 3: Rating ComboBox, Position ComboBox
    auto row3 = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(row3->layout.get())->spacing = 10; // Minimal spacing
    static_cast<HorizontalStackLayout*>(row3->layout.get())->alignment = HorizontalAlignment::Center; // For later
    static_cast<HorizontalStackLayout*>(row3->layout.get())->padding = { 2, 10, 5, 10 };
    {
        ratingCombo = new ComboBoxWidget();
        ratingCombo->id = "connect_rating";
        std::vector<std::string> ratings;
        for (int i = 0; i < 12; ++i) ratings.push_back(CONTROLLER_RATINGS[i]);
        ratingCombo->AddItems(ratings);
        ratingCombo->minSize = { 100, 22 };
        ratingCombo->preferredSize = { 100, 22 }; // Set preferred to match min for now
        row3->AddChild(std::unique_ptr<ComboBoxWidget>(ratingCombo));

        positionCombo = new ComboBoxWidget();
        positionCombo->id = "connect_position";
        std::vector<std::string> positions;
        for (int i = 0; i <= static_cast<int>(POSITIONS::FSS); ++i) {
            positions.push_back(PositionToString(static_cast<POSITIONS>(i)));
        }
        positionCombo->AddItems(positions);
        positionCombo->minSize = { 100, 22 };
        positionCombo->preferredSize = { 100, 22 }; // Set preferred to match min
        row3->AddChild(std::unique_ptr<ComboBoxWidget>(positionCombo));
    }
    contentArea->AddChild(std::move(row3));

    // Row 4: Buttons
    auto buttonRow = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    auto* btnLayout = static_cast<HorizontalStackLayout*>(buttonRow->layout.get());
    btnLayout->spacing = 10;
    btnLayout->alignment = HorizontalAlignment::Center; // Set alignment
    // btnLayout->alignment = HorizontalAlignment::Center; // Add this if/when HSL supports alignment
    {
        auto okButton = std::make_unique<ButtonWidget>("CONNECT");
        okButton->id = "connect_ok";
        okButton->preferredSize = { 100, 25 };
        okButton->minSize = { 80, 25 }; // Ensure a minimum
        okButton->OnClick = std::bind(&ConnectWindow::OnConnectClicked, this, std::placeholders::_1);
        buttonRow->AddChild(std::move(okButton));

        auto cancelButton = std::make_unique<ButtonWidget>("CANCEL");
        cancelButton->id = "connect_cancel";
        cancelButton->preferredSize = { 100, 25 };
        cancelButton->minSize = { 80, 25 }; // Ensure a minimum
        cancelButton->OnClick = std::bind(&ConnectWindow::OnCancelClicked, this, std::placeholders::_1);
        buttonRow->AddChild(std::move(cancelButton));
    }
    contentArea->AddChild(std::move(buttonRow));

    if (USER) {
        LoadDataFromIdentity(*USER->getIdentity());
    }
    // UIManager::AddWindow will call Measure and Arrange.
    // If creating this window outside UIManager initially, you'd call:
    // Measure();
    // Arrange(initial_bounds_or_current_bounds_with_new_preferred_size);
}

void ConnectWindow::LoadDataFromIdentity(Identity& id) {
    bool data_set = !boost::icontains(id.callsign, "NOT_LOGGED") && !id.callsign.empty();
    if (data_set) {
        callsignInput->SetText(id.callsign);
        fullNameInput->SetText(id.login_name);
        usernameInput->SetText(id.username);
        passwordInput->SetText(id.password); // Will be displayed as '***'
        ratingCombo->SetSelectedIndex(id.controller_rating);
        positionCombo->SetSelectedIndex(static_cast<int>(id.controller_position));
    } else {
        #ifdef _DEBUG // Pre-fill for debugging
        callsignInput->SetText("SM_SUP");
        fullNameInput->SetText("Samuel Mason");
        usernameInput->SetText("971202");
        passwordInput->SetText("583562");
        ratingCombo->SetSelectedIndex(10); // Supervisor
        positionCombo->SetSelectedIndex(static_cast<int>(POSITIONS::CENTER));
        #endif
    }
}

void ConnectWindow::OnConnectClicked(Widget* sender) {
    if (!USER) return; // Should not happen if initialized
    Identity& id = *USER->getIdentity();

    if (callsignInput->GetText().empty() || fullNameInput->GetText().empty() ||
        usernameInput->GetText().empty() || passwordInput->GetText().empty()) {
        sendErrorMessage("Missing Connection Data."); // Your existing global function
        return;
    }

    id.callsign = callsignInput->GetText();
    id.login_name = fullNameInput->GetText();
    id.username = usernameInput->GetText();
    id.password = passwordInput->GetText(); // Store actual password
    id.controller_rating = ratingCombo->GetSelectedIndex();
    id.controller_position = static_cast<POSITIONS>(positionCombo->GetSelectedIndex());

    connect(); // Your existing global function
    UIManager::Instance().RemoveWindow(this); // Close this window
}

void ConnectWindow::OnCancelClicked(Widget* sender) {
    // Optional: Clear fields if needed, or just close
    // callsignInput->SetText(""); // etc.
    UIManager::Instance().RemoveWindow(this);
}