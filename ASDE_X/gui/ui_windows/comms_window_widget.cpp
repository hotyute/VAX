#define NOMINMAX
#include <windows.h>
#include <iomanip>   // For std::fixed, std::setprecision
#include <sstream>   // For std::ostringstream

#include "comms_window_widget.h"
#include "../widgets/label_widget.h"
#include "../widgets/button_widget.h"
#include "../widgets/radio_button_widget.h"
#include "../widgets/input_field_widget.h"
#include "../widgets/layout_strategy.h"
#include "../widgets/ui_manager.h"
#include "../widgets/font_keys.h"

// Application-specific includes
#include "../../main.h"        // For CLIENT_WIDTH, CLIENT_HEIGHT, sendErrorMessage
#include "../../usermanager.h" // For USER
#include "../../packets.h"     // For sendPrimFreq
#include "../../tools.h"       // For string_to_frequency, frequency_to_string
#include "../../comms.h"       // For old globals like comms_line0, comms_line1 to load initial state
#include "../../interfaces.h"  // For sendSystemMessage

// Helper to check if old CommsLine pointers and their children are valid (simplified)
// In a real scenario, this might involve more robust checks or a different transition strategy.
bool IsOldCommsLineValid(const CommsLine* oldLine) {
    return oldLine && oldLine->prim && oldLine->btn && oldLine->tx && oldLine->rx && oldLine->hdst && oldLine->spkr;
}


CommsWindowWidget::CommsWindowWidget() : WindowWidget("COMMUNICATIONS") {
    minSize = { 280, 240 }; // Adjusted initial min size
    SetDraggable(true);
    SetClosable(true);

    commsLines.resize(NUM_DISPLAYED_LINES);
    commsLines[0].id = "LINE_0";
    commsLines[1].id = "LINE_1";

    SetupUI();
    LoadState(); // Load initial state from application
    ToggleEditSection(-1); // Ensure edit section is initially collapsed
}

CommsWindowWidget::~CommsWindowWidget() {
    SaveState(); // Persist changes if necessary
}

ContainerWidget* CommsWindowWidget::CreateHeaderRow() {
    auto headerRow = new ContainerWidget(std::make_unique<HorizontalStackLayout>());
    auto* headerLayout = static_cast<HorizontalStackLayout*>(headerRow->layout.get());
    headerLayout->spacing = 2; // Tight spacing for header
    headerLayout->padding = { 2, 2, 5, 2 }; // Padding below header

    auto primLabel = std::make_unique<LabelWidget>("PRIM");
    primLabel->preferredSize = { 38, 18 }; primLabel->minSize = primLabel->preferredSize;
    headerRow->AddChild(std::move(primLabel));

    auto nameLabel = std::make_unique<LabelWidget>("Name");
    nameLabel->preferredSize = { 90, 18 }; nameLabel->minSize = nameLabel->preferredSize;
    headerRow->AddChild(std::move(nameLabel));

    auto txLabel = std::make_unique<LabelWidget>("TX");
    txLabel->preferredSize = { 28, 18 }; txLabel->minSize = txLabel->preferredSize;
    headerRow->AddChild(std::move(txLabel));

    auto rxLabel = std::make_unique<LabelWidget>("RX");
    rxLabel->preferredSize = { 28, 18 }; rxLabel->minSize = rxLabel->preferredSize;
    headerRow->AddChild(std::move(rxLabel));

    auto hdstLabel = std::make_unique<LabelWidget>("HDST");
    hdstLabel->preferredSize = { 38, 18 }; hdstLabel->minSize = hdstLabel->preferredSize;
    headerRow->AddChild(std::move(hdstLabel));

    auto spkrLabel = std::make_unique<LabelWidget>("SPKR");
    spkrLabel->preferredSize = { 38, 18 }; spkrLabel->minSize = spkrLabel->preferredSize;
    headerRow->AddChild(std::move(spkrLabel));

    return headerRow;
}

void CommsWindowWidget::CreateCommsLineUIRow(int lineIndex, ContainerWidget* parentLayout) {
    CommsLineElements& line = commsLines[lineIndex];

    auto lineRow = new ContainerWidget(std::make_unique<HorizontalStackLayout>());
    line.uiRowContainer = lineRow;
    auto* hLayout = static_cast<HorizontalStackLayout*>(lineRow->layout.get());
    hLayout->spacing = 2;
    hLayout->padding = { 1,1,1,1 };

    // PRIM RadioButton
    line.primRadio = new RadioButtonWidget("");
    line.primRadio->preferredSize = { 38, 20 }; line.primRadio->minSize = line.primRadio->preferredSize;
    line.primRadio->OnClick = std::bind(&CommsWindowWidget::OnPrimRadioClicked, this, std::placeholders::_1, lineIndex);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.primRadio));

    // Name Button
    line.nameButton = new ButtonWidget("----");
    line.nameButton->preferredSize = { 90, 22 }; line.nameButton->minSize = line.nameButton->preferredSize;
    line.nameButton->style.padding = { 2, 5, 2, 5 }; // Less padding for denser look
    line.nameButton->OnClick = std::bind(&CommsWindowWidget::OnNameButtonClicked, this, std::placeholders::_1, lineIndex);
    lineRow->AddChild(std::unique_ptr<ButtonWidget>(line.nameButton));

    // TX RadioButton
    line.txRadio = new RadioButtonWidget("");
    line.txRadio->preferredSize = { 28, 20 }; line.txRadio->minSize = line.txRadio->preferredSize;
    line.txRadio->OnClick = std::bind(&CommsWindowWidget::OnRadioToggled, this, std::placeholders::_1, lineIndex, CommRadioType::TX);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.txRadio));

    // RX RadioButton
    line.rxRadio = new RadioButtonWidget("");
    line.rxRadio->preferredSize = { 28, 20 }; line.rxRadio->minSize = line.rxRadio->preferredSize;
    line.rxRadio->OnClick = std::bind(&CommsWindowWidget::OnRadioToggled, this, std::placeholders::_1, lineIndex, CommRadioType::RX);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.rxRadio));

    // HDST RadioButton
    line.hdstRadio = new RadioButtonWidget("");
    line.hdstRadio->preferredSize = { 38, 20 }; line.hdstRadio->minSize = line.hdstRadio->preferredSize;
    line.hdstRadio->OnClick = std::bind(&CommsWindowWidget::OnRadioToggled, this, std::placeholders::_1, lineIndex, CommRadioType::HDST);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.hdstRadio));

    // SPKR RadioButton
    line.spkrRadio = new RadioButtonWidget("");
    line.spkrRadio->preferredSize = { 38, 20 }; line.spkrRadio->minSize = line.spkrRadio->preferredSize;
    line.spkrRadio->OnClick = std::bind(&CommsWindowWidget::OnRadioToggled, this, std::placeholders::_1, lineIndex, CommRadioType::SPKR);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.spkrRadio));

    parentLayout->AddChild(std::unique_ptr<ContainerWidget>(lineRow));
}


void CommsWindowWidget::SetupUI() {
    ContainerWidget* ca = GetContentArea();
    auto mainLayout = std::make_unique<VerticalStackLayout>();
    mainLayout->spacing = 2; // Minimal spacing between rows
    mainLayout->padding = { 5, 5, 5, 5 };
    ca->SetLayout(std::move(mainLayout));

    // Add Header Row
    ca->AddChild(std::unique_ptr<ContainerWidget>(CreateHeaderRow()));

    // Add Comms Line Rows
    for (int i = 0; i < NUM_DISPLAYED_LINES; ++i) {
        CreateCommsLineUIRow(i, ca);
    }

    // Edit Section Container (initially hidden)
    editSectionContainer = new ContainerWidget(std::make_unique<VerticalStackLayout>());
    auto* editLayout = static_cast<VerticalStackLayout*>(editSectionContainer->layout.get());
    editLayout->spacing = 8;
    editLayout->padding = { 10, 15, 10, 15 };
    editSectionContainer->style.backgroundColor = { 0.25f, 0.25f, 0.25f, 0.8f };
    editSectionContainer->style.hasBorder = true;
    editSectionContainer->style.borderColor = { 0.4f, 0.4f, 0.4f, 0.8f };
    editSectionContainer->SetVisible(false);

    // Edit Position
    auto posRow = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(posRow->layout.get())->spacing = 5;
    editPositionLabel = new LabelWidget("Position:"); editPositionLabel->preferredSize = { 60, 20 };
    posRow->AddChild(std::unique_ptr<LabelWidget>(editPositionLabel));
    editPositionInput = new InputFieldWidget(); editPositionInput->preferredSize = { 180, 20 }; editPositionInput->minSize = { 150, 20 };
    posRow->AddChild(std::unique_ptr<InputFieldWidget>(editPositionInput));
    editSectionContainer->AddChild(std::move(posRow));

    // Edit Frequency
    auto freqRow = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(freqRow->layout.get())->spacing = 5;
    editFrequencyLabel = new LabelWidget("Frequency:"); editFrequencyLabel->preferredSize = { 60, 20 };
    freqRow->AddChild(std::unique_ptr<LabelWidget>(editFrequencyLabel));
    editFrequencyInput = new InputFieldWidget(); editFrequencyInput->preferredSize = { 180, 20 }; editFrequencyInput->minSize = { 150, 20 };
    editFrequencyInput->placeholderText = "e.g. 123.450";
    freqRow->AddChild(std::unique_ptr<InputFieldWidget>(editFrequencyInput));
    editSectionContainer->AddChild(std::move(freqRow));

    // Edit Buttons
    auto btnRow = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    auto* editButtonsLayout = static_cast<HorizontalStackLayout*>(btnRow->layout.get());
    editButtonsLayout->spacing = 10;
    editButtonsLayout->alignment = HorizontalAlignment::Center;

    editSaveButton = new ButtonWidget("Save"); editSaveButton->preferredSize = { 65, 22 };
    editSaveButton->OnClick = std::bind(&CommsWindowWidget::OnSaveEditClicked, this, std::placeholders::_1);
    btnRow->AddChild(std::unique_ptr<ButtonWidget>(editSaveButton));

    editClearButton = new ButtonWidget("Clear"); editClearButton->preferredSize = { 65, 22 };
    editClearButton->OnClick = std::bind(&CommsWindowWidget::OnClearEditClicked, this, std::placeholders::_1);
    btnRow->AddChild(std::unique_ptr<ButtonWidget>(editClearButton));

    editCancelButton = new ButtonWidget("Cancel"); editCancelButton->preferredSize = { 65, 22 };
    editCancelButton->OnClick = std::bind(&CommsWindowWidget::OnCancelEditClicked, this, std::placeholders::_1);
    btnRow->AddChild(std::unique_ptr<ButtonWidget>(editCancelButton));

    editSectionContainer->AddChild(std::move(btnRow));
    ca->AddChild(std::unique_ptr<ContainerWidget>(editSectionContainer)); // Add to main layout
}

void CommsWindowWidget::LoadState() {
    // Load from old global comms_line0 and comms_line1
    // This is a placeholder for proper data loading from USER->userdata or a config file.
    if (IsOldCommsLineValid(comms_line0)) {
        CommsLineElements& line0 = commsLines[0];
        line0.positionName = comms_line0->pos;
        line0.frequency = comms_line0->freq;
        line0.isPrimary = (cur_prime == comms_line0);
        if (line0.isPrimary) currentPrimaryLineIndex = 0;
        line0.tx_checked = comms_line0->tx->checked;
        line0.rx_checked = comms_line0->rx->checked;
        line0.hdst_checked = comms_line0->hdst->checked;
        line0.spkr_checked = comms_line0->spkr->checked;
    }

    if (IsOldCommsLineValid(comms_line1)) {
        CommsLineElements& line1 = commsLines[1];
        line1.positionName = comms_line1->pos;
        line1.frequency = comms_line1->freq;
        line1.isPrimary = (cur_prime == comms_line1);
        if (line1.isPrimary) currentPrimaryLineIndex = 1;
        line1.tx_checked = comms_line1->tx->checked;
        line1.rx_checked = comms_line1->rx->checked;
        line1.hdst_checked = comms_line1->hdst->checked;
        line1.spkr_checked = comms_line1->spkr->checked;
    }
    if (currentPrimaryLineIndex == -1 && cur_prime != nullptr) { // If cur_prime was set but not to line0 or line1
        cur_prime = nullptr; // Ensure consistency
    }


    for (int i = 0; i < NUM_DISPLAYED_LINES; ++i) {
        UpdateCommsLineUIRow(i);
    }
    MarkDirty(true);
}

void CommsWindowWidget::SaveState() {
    // Save state back to old globals or USER->userdata
    if (IsOldCommsLineValid(comms_line0)) {
        CommsLineElements& line0 = commsLines[0];
        comms_line0->pos = line0.positionName;
        comms_line0->freq = line0.frequency;
        comms_line0->tx->checked = line0.tx_checked;
        comms_line0->rx->checked = line0.rx_checked;
        comms_line0->hdst->checked = line0.hdst_checked;
        comms_line0->spkr->checked = line0.spkr_checked;
    }
    if (IsOldCommsLineValid(comms_line1)) {
        CommsLineElements& line1 = commsLines[1];
        comms_line1->pos = line1.positionName;
        comms_line1->freq = line1.frequency;
        comms_line1->tx->checked = line1.tx_checked;
        comms_line1->rx->checked = line1.rx_checked;
        comms_line1->hdst->checked = line1.hdst_checked;
        comms_line1->spkr->checked = line1.spkr_checked;
    }

    if (currentPrimaryLineIndex == 0 && IsOldCommsLineValid(comms_line0)) cur_prime = comms_line0;
    else if (currentPrimaryLineIndex == 1 && IsOldCommsLineValid(comms_line1)) cur_prime = comms_line1;
    else cur_prime = nullptr;

    // Update USER data too
    if (USER && currentPrimaryLineIndex != -1 && currentPrimaryLineIndex < (int)commsLines.size()) {
        USER->userdata.frequency[0] = string_to_frequency(commsLines[currentPrimaryLineIndex].frequency);
    }
    else if (USER) {
        USER->userdata.frequency[0] = 99998; // Default if no primary selected
    }
}


void CommsWindowWidget::UpdateCommsLineUIRow(int lineIndex) {
    if (lineIndex < 0 || lineIndex >= (int)commsLines.size()) return;
    CommsLineElements& line = commsLines[lineIndex];

    std::string buttonText = line.positionName.empty() ? "----" : line.positionName;
    if (!line.frequency.empty()) {
        buttonText += " (" + line.frequency + ")";
    }
    line.nameButton->SetText(buttonText);

    line.primRadio->SetChecked(line.isPrimary);
    line.txRadio->SetChecked(line.tx_checked);
    line.rxRadio->SetChecked(line.rx_checked);
    line.hdstRadio->SetChecked(line.hdst_checked);
    line.spkrRadio->SetChecked(line.spkr_checked);
}

void CommsWindowWidget::OnPrimRadioClicked(Widget* sender, int lineIndex) {
    int oldPrimaryIndex = currentPrimaryLineIndex;

    if (currentPrimaryLineIndex == lineIndex) { // Clicking the already primary radio
        currentPrimaryLineIndex = -1; // Deselect
        commsLines[lineIndex].isPrimary = false;
    }
    else {
        if (currentPrimaryLineIndex != -1) {
            commsLines[currentPrimaryLineIndex].isPrimary = false;
            commsLines[currentPrimaryLineIndex].primRadio->SetChecked(false);
        }
        currentPrimaryLineIndex = lineIndex;
        commsLines[lineIndex].isPrimary = true;
    }
    // The clicked radio (sender) already toggled its state.
    // We just need to ensure others are correct.
    // UpdatePrimRadios method is not strictly needed if RadioButtonWidget correctly reflects its state.
    // But for safety:
    for (int i = 0; i < NUM_DISPLAYED_LINES; ++i) {
        commsLines[i].primRadio->SetChecked(commsLines[i].isPrimary);
    }


    if (USER) {
        if (currentPrimaryLineIndex != -1) {
            USER->userdata.frequency[0] = string_to_frequency(commsLines[currentPrimaryLineIndex].frequency);
        }
        else {
            USER->userdata.frequency[0] = 99998; // No primary
        }
        sendPrimFreq(); // Your existing global function
    }
    MarkDirty(true);
}

void CommsWindowWidget::OnNameButtonClicked(Widget* sender, int lineIndex) {
    if (isEditSectionVisible && currentEditingLineIndex == lineIndex) {
        ToggleEditSection(-1); // Collapse if clicking the same name button again
    }
    else {
        ToggleEditSection(lineIndex);
    }
}

void CommsWindowWidget::OnRadioToggled(Widget* sender, int lineIndex, int radioType) {
    RadioButtonWidget* rb = static_cast<RadioButtonWidget*>(sender);
    bool newState = rb->isChecked; // RadioButton already handled its state change

    switch (static_cast<CommRadioType>(radioType)) {
    case TX: commsLines[lineIndex].tx_checked = newState; break;
    case RX: commsLines[lineIndex].rx_checked = newState; break;
    case HDST: commsLines[lineIndex].hdst_checked = newState; break;
    case SPKR: commsLines[lineIndex].spkr_checked = newState; break;
    }
}

void CommsWindowWidget::ToggleEditSection(int lineIndex) {
    currentEditingLineIndex = lineIndex;
    isEditSectionVisible = (lineIndex != -1);

    if (editSectionContainer) {
        editSectionContainer->SetVisible(isEditSectionVisible);
        if (isEditSectionVisible) {
            const auto& line = commsLines[lineIndex];
            editPositionInput->SetText(line.positionName);
            editFrequencyInput->SetText(line.frequency);
            editPositionInput->RequestFocus();
        }
    }
    // Request re-measure and re-arrange for the entire window, as content changed
    Measure();
    Arrange(bounds);
    MarkDirty(true);
}


void CommsWindowWidget::OnSaveEditClicked(Widget* sender) {
    if (currentEditingLineIndex == -1) return;

    std::string posStr = editPositionInput->GetText();
    std::string freqStr = editFrequencyInput->GetText();

    if (!freqStr.empty()) {
        if (freqStr.length() != 7 || freqStr.find('.') != 3) {
            // Check if it's parseable even if format is off for legacy reasons
            if (string_to_frequency(freqStr) == 99998 && freqStr != "199.998") { // 199.998 is often guard
                sendErrorMessage("Invalid Frequency Format! Use XXX.XXX (e.g., 123.450)");
                editFrequencyInput->RequestFocus();
                return;
            }
        }
    }
    // If freqStr is empty, it means clearing the frequency.

    CommsLineElements& line = commsLines[currentEditingLineIndex];
    line.positionName = posStr;
    line.frequency = freqStr;

    UpdateCommsLineUIRow(currentEditingLineIndex);

    if (line.isPrimary && USER) {
        USER->userdata.frequency[0] = string_to_frequency(line.frequency);
        sendPrimFreq();
    }

    ToggleEditSection(-1); // Collapse
}

void CommsWindowWidget::OnClearEditClicked(Widget* sender) {
    editPositionInput->SetText("");
    editFrequencyInput->SetText("");
    editPositionInput->RequestFocus();
}

void CommsWindowWidget::OnCancelEditClicked(Widget* sender) {
    ToggleEditSection(-1); // Collapse
}

void CommsWindowWidget::HandleEvent(UIEvent& event) {
    WindowWidget::HandleEvent(event); // Handle dragging, closing etc.
    if (event.handled) return;

    if (isEditSectionVisible && event.eventType == UIEvent::Type::Keyboard &&
        event.keyboard.type == KeyEventType::Press && event.keyboard.keyCode == KeyCode::Escape) {
        OnCancelEditClicked(nullptr);
        event.handled = true;
    }
}