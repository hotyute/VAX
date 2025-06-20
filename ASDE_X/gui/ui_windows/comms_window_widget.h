// --- START OF FILE comms_window_widget.h ---
#pragma once
#include "../widgets/window_widget.h" 
// #include "../../comms.h" // REMOVE THIS - NO LONGER NEEDED
#include <vector>
#include <string>

// Forward declarations for UI widgets
class LabelWidget;
class ButtonWidget;
class RadioButtonWidget;
class InputFieldWidget;
class ContainerWidget;
// class CommsLine; // REMOVE THIS - Old struct not used

// Internal data structure to hold data and UI pointers for each comms line
struct CommsLineElements {
    std::string id;
    std::string positionName;
    std::string frequency;
    bool isPrimary = false;
    bool tx_checked = false;
    bool rx_checked = false;
    bool hdst_checked = false;
    bool spkr_checked = false;

    RadioButtonWidget* primRadio = nullptr;
    ButtonWidget* nameButton = nullptr;
    RadioButtonWidget* txRadio = nullptr;
    RadioButtonWidget* rxRadio = nullptr;
    RadioButtonWidget* hdstRadio = nullptr;
    RadioButtonWidget* spkrRadio = nullptr;

    ContainerWidget* uiRowContainer = nullptr;
};

class CommsWindowWidget : public WindowWidget {
public:
    CommsWindowWidget();
    ~CommsWindowWidget();

    void LoadState() override;
    void SaveState() override;

    void HandleEvent(UIEvent& event) override;

    const std::vector<CommsLineElements>& GetCommsLinesElements() const { return commsLines; }

    // void set_element(CommsLine* line); // REMOVE
    // void save_element(CommsLine* line); // REMOVE

    void OnShowOrBringToFront() override {
        WindowWidget::OnShowOrBringToFront();
        LoadState(); // LoadState will now read from USER->userdata
        MarkDirty(true);
    }

private:
    void SetupUI();
    ContainerWidget* CreateHeaderRow();
    void CreateCommsLineUIRow(int lineIndex, ContainerWidget* parentLayout);
    void UpdateCommsLineUIRow(int lineIndex);

    void ToggleEditSection(int lineIndex = -1);

    void OnPrimRadioClicked(Widget* sender, int lineIndex);
    void OnNameButtonClicked(Widget* sender, int lineIndex);
    void OnRadioToggled(Widget* sender, int lineIndex, int radioType);

    void OnSaveEditClicked(Widget* sender);
    void OnClearEditClicked(Widget* sender);
    void OnCancelEditClicked(Widget* sender);

    enum CommRadioType { TX, RX, HDST, SPKR };

    std::vector<CommsLineElements> commsLines;
    // static const int NUM_ELEMENTS = 3; // Already defined in .cpp

    ContainerWidget* editSectionContainer = nullptr;
    LabelWidget* editPositionLabel = nullptr;
    InputFieldWidget* editPositionInput = nullptr;
    LabelWidget* editFrequencyLabel = nullptr;
    InputFieldWidget* editFrequencyInput = nullptr;
    ButtonWidget* editSaveButton = nullptr;
    ButtonWidget* editClearButton = nullptr;
    ButtonWidget* editCancelButton = nullptr;

    bool isEditSectionVisible = false;
    int currentEditingLineIndex = -1;
    int currentPrimaryLineIndex = -1;
};
// --- END OF FILE comms_window_widget.h ---