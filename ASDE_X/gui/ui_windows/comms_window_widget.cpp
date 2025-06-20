// --- START OF FILE comms_window_widget.cpp ---
#define NOMINMAX
#include <windows.h>
#include <iomanip>   
#include <sstream>   

#include "comms_window_widget.h"
#include "../widgets/label_widget.h"
#include "../widgets/button_widget.h"
#include "../widgets/radio_button_widget.h"
#include "../widgets/input_field_widget.h"
#include "../widgets/layout_strategy.h"
#include "../widgets/ui_manager.h"
#include "../widgets/font_keys.h"

#include "../../main.h"        
#include "../../usermanager.h" 
#include "../../packets.h"     
#include "../../tools.h"       
#include "../../interfaces.h"  
#include "../../user.h" // For UserData::NUM_SAVED_COMMS_LINES and CommsLinePersistentData

// REMOVE IsOldCommsLineValid function

static const int NUM_ELEMENTS = UserData::NUM_SAVED_COMMS_LINES; // Use the definition from UserData

CommsWindowWidget::CommsWindowWidget() : WindowWidget("COMMUNICATIONS") {
    minSize = { 280, 240 };
    SetDraggable(true);
    SetClosable(true);

    commsLines.resize(NUM_ELEMENTS);
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        commsLines[i].id = "LINE_" + std::to_string(i);
    }
    // Initial default values are set by CommsLineElements default member initializers

    SetupUI();
    // LoadState(); // LoadState is called by OnShowOrBringToFront
    ToggleEditSection(-1);
}

CommsWindowWidget::~CommsWindowWidget() {
    // If SaveState is meant to be called on close, UIManager::RemoveWindow
    // should eventually trigger some kind of OnClosing/OnDestroyed event for the window
    // where it can call SaveState(). For now, we rely on explicit save or saving
    // when the whole application closes (via save_info in main.cpp).
    // A simple way is to call SaveState() here, but it might be called too often if the
    // window is frequently created/destroyed without app closure.
    // For now, let's assume save_info in main application will handle saving USER->userdata.
}

// CreateHeaderRow() - NO CHANGES
ContainerWidget* CommsWindowWidget::CreateHeaderRow() {
    auto headerRow = new ContainerWidget(std::make_unique<HorizontalStackLayout>());
    auto* headerLayout = static_cast<HorizontalStackLayout*>(headerRow->layout.get());
    headerLayout->spacing = 2;
    headerLayout->padding = { 2, 2, 5, 2 };

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

// CreateCommsLineUIRow() - NO CHANGES
void CommsWindowWidget::CreateCommsLineUIRow(int lineIndex, ContainerWidget* parentLayout) {
    CommsLineElements& line = commsLines[lineIndex];

    auto lineRow = new ContainerWidget(std::make_unique<HorizontalStackLayout>());
    line.uiRowContainer = lineRow;
    auto* hLayout = static_cast<HorizontalStackLayout*>(lineRow->layout.get());
    hLayout->spacing = 2;
    hLayout->padding = { 1,1,1,1 };

    line.primRadio = new RadioButtonWidget("");
    line.primRadio->preferredSize = { 38, 20 }; line.primRadio->minSize = line.primRadio->preferredSize;
    line.primRadio->OnClick = std::bind(&CommsWindowWidget::OnPrimRadioClicked, this, std::placeholders::_1, lineIndex);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.primRadio));

    line.nameButton = new ButtonWidget("----");
    line.nameButton->preferredSize = { 90, 22 }; line.nameButton->minSize = line.nameButton->preferredSize;
    line.nameButton->style.padding = { 2, 5, 2, 5 };
    line.nameButton->OnClick = std::bind(&CommsWindowWidget::OnNameButtonClicked, this, std::placeholders::_1, lineIndex);
    lineRow->AddChild(std::unique_ptr<ButtonWidget>(line.nameButton));

    line.txRadio = new RadioButtonWidget("");
    line.txRadio->preferredSize = { 28, 20 }; line.txRadio->minSize = line.txRadio->preferredSize;
    line.txRadio->OnClick = std::bind(&CommsWindowWidget::OnRadioToggled, this, std::placeholders::_1, lineIndex, CommRadioType::TX);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.txRadio));

    line.rxRadio = new RadioButtonWidget("");
    line.rxRadio->preferredSize = { 28, 20 }; line.rxRadio->minSize = line.rxRadio->preferredSize;
    line.rxRadio->OnClick = std::bind(&CommsWindowWidget::OnRadioToggled, this, std::placeholders::_1, lineIndex, CommRadioType::RX);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.rxRadio));

    line.hdstRadio = new RadioButtonWidget("");
    line.hdstRadio->preferredSize = { 38, 20 }; line.hdstRadio->minSize = line.hdstRadio->preferredSize;
    line.hdstRadio->OnClick = std::bind(&CommsWindowWidget::OnRadioToggled, this, std::placeholders::_1, lineIndex, CommRadioType::HDST);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.hdstRadio));

    line.spkrRadio = new RadioButtonWidget("");
    line.spkrRadio->preferredSize = { 38, 20 }; line.spkrRadio->minSize = line.spkrRadio->preferredSize;
    line.spkrRadio->OnClick = std::bind(&CommsWindowWidget::OnRadioToggled, this, std::placeholders::_1, lineIndex, CommRadioType::SPKR);
    lineRow->AddChild(std::unique_ptr<RadioButtonWidget>(line.spkrRadio));

    parentLayout->AddChild(std::unique_ptr<ContainerWidget>(lineRow));
}

// SetupUI() - NO CHANGES other than ensuring NUM_ELEMENTS is defined correctly (handled by class const)
void CommsWindowWidget::SetupUI() {
    ContainerWidget* ca = GetContentArea();
    auto mainLayout = std::make_unique<VerticalStackLayout>();
    mainLayout->spacing = 2;
    mainLayout->padding = { 5, 5, 5, 5 };
    mainLayout->childAlignment = HorizontalAlignment::Center;
    ca->SetLayout(std::move(mainLayout));

    ca->AddChild(std::unique_ptr<ContainerWidget>(CreateHeaderRow()));

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        CreateCommsLineUIRow(i, ca);
    }

    editSectionContainer = new ContainerWidget(std::make_unique<VerticalStackLayout>());
    auto* editLayout = static_cast<VerticalStackLayout*>(editSectionContainer->layout.get());
    editLayout->spacing = 8;
    editLayout->padding = { 10, 15, 10, 15 };
    editSectionContainer->style.backgroundColor = { 0.25f, 0.25f, 0.25f, 0.8f };
    editSectionContainer->style.hasBorder = true;
    editSectionContainer->style.borderColor = { 0.4f, 0.4f, 0.4f, 0.8f };
    editSectionContainer->SetVisible(false);

    auto posRow = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(posRow->layout.get())->spacing = 5;
    editPositionLabel = new LabelWidget("Position:"); editPositionLabel->preferredSize = { 60, 20 };
    posRow->AddChild(std::unique_ptr<LabelWidget>(editPositionLabel));
    editPositionInput = new InputFieldWidget(); editPositionInput->preferredSize = { 180, 20 }; editPositionInput->minSize = { 150, 20 };
    posRow->AddChild(std::unique_ptr<InputFieldWidget>(editPositionInput));
    editSectionContainer->AddChild(std::move(posRow));

    auto freqRow = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(freqRow->layout.get())->spacing = 5;
    editFrequencyLabel = new LabelWidget("Frequency:"); editFrequencyLabel->preferredSize = { 60, 20 };
    freqRow->AddChild(std::unique_ptr<LabelWidget>(editFrequencyLabel));
    editFrequencyInput = new InputFieldWidget(); editFrequencyInput->preferredSize = { 180, 20 }; editFrequencyInput->minSize = { 150, 20 };
    editFrequencyInput->placeholderText = "e.g. 123.450";
    freqRow->AddChild(std::unique_ptr<InputFieldWidget>(editFrequencyInput));
    editSectionContainer->AddChild(std::move(freqRow));

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
    ca->AddChild(std::unique_ptr<ContainerWidget>(editSectionContainer));
}


void CommsWindowWidget::LoadState() {
    if (!USER) {
        // Handle case where USER is not yet initialized (e.g., initial setup before login)
        // For now, commsLines will retain their default constructed values or previous state.
        // Could log a warning or set to defaults if this is unexpected.
        return;
    }

    currentPrimaryLineIndex = USER->userdata.primaryCommsLineIndex;

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        if (i < UserData::NUM_SAVED_COMMS_LINES) { // Ensure we don't read out of bounds
            commsLines[i].positionName = USER->userdata.commsConfig[i].pos;
            commsLines[i].frequency = USER->userdata.commsConfig[i].freq;
            commsLines[i].tx_checked = USER->userdata.commsConfig[i].tx_checked;
            commsLines[i].rx_checked = USER->userdata.commsConfig[i].rx_checked;
            commsLines[i].hdst_checked = USER->userdata.commsConfig[i].hdst_checked;
            commsLines[i].spkr_checked = USER->userdata.commsConfig[i].spkr_checked;
            commsLines[i].isPrimary = (i == currentPrimaryLineIndex);
        }
        else {
            // If CommsWindowWidget::NUM_ELEMENTS > UserData::NUM_SAVED_COMMS_LINES,
            // initialize remaining lines to default. (Already handled by commsLines resize and CommsLineElements defaults)
        }
        UpdateCommsLineUIRow(i);
    }
    MarkDirty(true);
}

void CommsWindowWidget::SaveState() {
    if (!USER) {
        // Cannot save state if USER is not available.
        return;
    }

    USER->userdata.primaryCommsLineIndex = currentPrimaryLineIndex;

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        if (i < UserData::NUM_SAVED_COMMS_LINES) { // Ensure we don't write out of bounds
            USER->userdata.commsConfig[i].pos = commsLines[i].positionName;
            USER->userdata.commsConfig[i].freq = commsLines[i].frequency;
            USER->userdata.commsConfig[i].tx_checked = commsLines[i].tx_checked;
            USER->userdata.commsConfig[i].rx_checked = commsLines[i].rx_checked;
            USER->userdata.commsConfig[i].hdst_checked = commsLines[i].hdst_checked;
            USER->userdata.commsConfig[i].spkr_checked = commsLines[i].spkr_checked;
        }
    }
    // Note: This SaveState() updates USER->userdata. The actual disk persistence
    // happens when save_info() in main.cpp (or similar) is called.
}

// UpdateCommsLineUIRow() - NO CHANGES
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

// OnPrimRadioClicked() - NO CHANGES
void CommsWindowWidget::OnPrimRadioClicked(Widget* sender, int lineIndex) {
    int oldPrimaryIndex = currentPrimaryLineIndex;

    if (currentPrimaryLineIndex == lineIndex) {
        currentPrimaryLineIndex = -1;
        commsLines[lineIndex].isPrimary = false;
    }
    else {
        if (currentPrimaryLineIndex != -1 && currentPrimaryLineIndex < NUM_ELEMENTS) { // Bounds check
            commsLines[currentPrimaryLineIndex].isPrimary = false;
            if (commsLines[currentPrimaryLineIndex].primRadio) // Defensive check
                commsLines[currentPrimaryLineIndex].primRadio->SetChecked(false);
        }
        currentPrimaryLineIndex = lineIndex;
        commsLines[lineIndex].isPrimary = true;
    }

    // Ensure all radio buttons visually reflect the state
    for (int i = 0; i < NUM_ELEMENTS; ++i) {
        if (commsLines[i].primRadio) // Defensive check
            commsLines[i].primRadio->SetChecked(commsLines[i].isPrimary);
    }

    if (USER) {
        if (currentPrimaryLineIndex != -1) {
            USER->userdata.frequency[0] = string_to_frequency(commsLines[currentPrimaryLineIndex].frequency);
        }
        else {
            USER->userdata.frequency[0] = 99998;
        }
        sendPrimFreq();
    }
    SaveState(); // Save after primary change
    MarkDirty(true);
}

// OnNameButtonClicked() - NO CHANGES
void CommsWindowWidget::OnNameButtonClicked(Widget* sender, int lineIndex) {
    if (isEditSectionVisible && currentEditingLineIndex == lineIndex) {
        ToggleEditSection(-1);
    }
    else {
        ToggleEditSection(lineIndex);
    }
}

// OnRadioToggled() - NO CHANGES
void CommsWindowWidget::OnRadioToggled(Widget* sender, int lineIndex, int radioType) {
    RadioButtonWidget* rb = static_cast<RadioButtonWidget*>(sender);
    bool newState = rb->isChecked;

    switch (static_cast<CommRadioType>(radioType)) {
    case TX: commsLines[lineIndex].tx_checked = newState; break;
    case RX: commsLines[lineIndex].rx_checked = newState; break;
    case HDST: commsLines[lineIndex].hdst_checked = newState; break;
    case SPKR: commsLines[lineIndex].spkr_checked = newState; break;
    }
    SaveState();
}

// ToggleEditSection() - NO CHANGES
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
    Measure();
    Arrange(bounds);
    MarkDirty(true);
}

// OnSaveEditClicked() - NO CHANGES
void CommsWindowWidget::OnSaveEditClicked(Widget* sender) {
    if (currentEditingLineIndex == -1) return;

    std::string posStr = editPositionInput->GetText();
    std::string freqStr = editFrequencyInput->GetText();

    if (!freqStr.empty()) {
        if (freqStr.length() != 7 || freqStr.find('.') != 3) {
            if (string_to_frequency(freqStr) == 99998 && freqStr != "199.998") {
                sendErrorMessage("Invalid Frequency Format! Use XXX.XXX (e.g., 123.450)");
                editFrequencyInput->RequestFocus();
                return;
            }
        }
    }

    CommsLineElements& line = commsLines[currentEditingLineIndex];
    line.positionName = posStr;
    line.frequency = freqStr;

    UpdateCommsLineUIRow(currentEditingLineIndex);

    if (line.isPrimary && USER) {
        USER->userdata.frequency[0] = string_to_frequency(line.frequency);
        sendPrimFreq();
    }

    SaveState();
    ToggleEditSection(-1);
}

// OnClearEditClicked() - NO CHANGES
void CommsWindowWidget::OnClearEditClicked(Widget* sender) {
    editPositionInput->SetText("");
    editFrequencyInput->SetText("");
    editPositionInput->RequestFocus();
}

// OnCancelEditClicked() - NO CHANGES
void CommsWindowWidget::OnCancelEditClicked(Widget* sender) {
    ToggleEditSection(-1);
}

// HandleEvent() - NO CHANGES
void CommsWindowWidget::HandleEvent(UIEvent& event) {
    WindowWidget::HandleEvent(event);
    if (event.handled) return;

    if (isEditSectionVisible && event.eventType == UIEvent::Type::Keyboard &&
        event.keyboard.type == KeyEventType::Press && event.keyboard.keyCode == KeyCode::Escape) {
        OnCancelEditClicked(nullptr);
        event.handled = true;
    }
}