#include "flight_plan_window.h"
#include "../widgets/label_widget.h"
#include "../widgets/input_field_widget.h"
#include "../widgets/combo_box_widget.h"
#include "../widgets/text_box_widget.h"
#include "../widgets/button_widget.h"
#include "../widgets/layout_strategy.h"
#include "../widgets/ui_manager.h" // For UIManager::Instance().RemoveWindow etc.
#include "../widgets/font_keys.h"   // For FONT_KEY_UI_LABEL

// Include necessary headers from the main application
#include "../../aircraft.h"      // For Aircraft, FlightPlan, PILOT_RATINGS
#include "../../usermanager.h"   // For USER, ASEL, opened_fp (though we'll try to avoid direct use of opened_fp)
#include "../../flightplan.h"  // For PullFPData, sendFlightPlanRequest, squawk_range
#include "../../packets.h"       // For sendFlightPlan
#include "../../constants.h"     // For FP_ constants (if used for IDs, but better to use member pointers)
#include "../../tools.h"         // For s2ws, capitalize, random, etc.
#include "../../interfaces.h"  

// Helper function from old flightplan.cpp (or similar logic)
std::string GenerateNewSquawk() {
    std::string pcode = "";
    while (pcode.empty()) {
        bool found = false;
        for (auto const& [key, val] : acf_map) { // Iterate over existing aircraft
            if (val && val->getFlightPlan() && val->getFlightPlan()->squawkCode == std::to_string(squawk_range + 1)) {
                found = true;
                ++squawk_range;
                break;
            }
        }
        if (!found) {
            pcode = std::to_string(++squawk_range);
            break;
        }
    }
    return pcode;
}


FlightPlanWindow::FlightPlanWindow(Aircraft* aircraft)
    : WindowWidget("FLIGHT PLAN"), targetAircraft(aircraft), initialCallsign(""), isNewPlan(false) {
    if (targetAircraft && targetAircraft->getIdentity()) {
        initialCallsign = targetAircraft->getIdentity()->callsign;
    }
    else if (targetAircraft) { // Aircraft exists but no identity? Fallback
        initialCallsign = targetAircraft->getCallsign(); // Assuming Aircraft has getCallsign
    }
    // If aircraft is null, this constructor shouldn't really be used, but handle gracefully
    if (initialCallsign.empty()) initialCallsign = "N/A";

    title = initialCallsign + " - FLIGHT PLAN"; // Set initial title
    SetupUI();
    LoadData();
}

FlightPlanWindow::FlightPlanWindow(const std::string& callsign_for_new_fp)
    : WindowWidget("FLIGHT PLAN"), targetAircraft(nullptr), initialCallsign(callsign_for_new_fp), isNewPlan(true) {
    title = initialCallsign + " - FLIGHT PLAN";
    SetupUI();
    LoadData();
}

FlightPlanWindow::~FlightPlanWindow() {
    // If this window was managing the global 'opened_fp', reset it
    if (opened_fp == targetAircraft || (isNewPlan && opened_fp && opened_fp->getCallsign() == initialCallsign)) {
        opened_fp = nullptr;
    }
}


void FlightPlanWindow::SetupUI() {
    minSize = { 500, 320 }; // Adjusted for potentially more content
    // preferredSize = {550, 380};

    ContainerWidget* ca = GetContentArea();
    auto contentLayout = std::make_unique<VerticalStackLayout>();
    contentLayout->spacing = 5;
    contentLayout->padding = { 5, 10, 10, 10 }; // top, right, bottom, left
    ca->SetLayout(std::move(contentLayout));

    // --- Helper for creating Label + InputField pairs ---
    auto createLabelInputPair = [&](const std::string& labelText, InputFieldWidget*& inputFieldPtr, bool readOnly = false, bool centeredInput = true, int inputWidth = 60, int labelWidth = 70) {
        auto pairContainer = std::make_unique<ContainerWidget>(std::make_unique<VerticalStackLayout>());
        static_cast<VerticalStackLayout*>(pairContainer->layout.get())->spacing = 1;

        auto label = std::make_unique<LabelWidget>(labelText);
        label->preferredSize.width = labelWidth;
        label->style.fontKey = FONT_KEY_UI_LABEL; // Ensure font is set
        pairContainer->AddChild(std::move(label));

        inputFieldPtr = new InputFieldWidget();
        inputFieldPtr->SetText(readOnly ? " " : ""); // Placeholder if readonly
        inputFieldPtr->preferredSize = { inputWidth, 20 };
        inputFieldPtr->minSize = { inputWidth, 20 };
        inputFieldPtr->style.fontKey = FONT_KEY_UI_LABEL;
        if (readOnly) {
            inputFieldPtr->SetEnabled(false); // Visually distinct and non-interactive
            inputFieldPtr->style.backgroundColor = { 0.6f, 0.6f, 0.6f, 0.5f };
        }
        // Centering text in InputFieldWidget is not directly supported, would need style or internal logic change.
        // For now, input fields are left-aligned.
        pairContainer->AddChild(std::unique_ptr<InputFieldWidget>(inputFieldPtr));
        return pairContainer;
        };

    // --- Line 1: Callsign, AC Type, Flight Rules | Amend Button ---
    auto line1 = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(line1->layout.get())->spacing = 10;
    {
        line1->AddChild(createLabelInputPair("Callsign:", callsignDisplayInput, true, false, 80));
        line1->AddChild(createLabelInputPair("A/C Type:", acTypeInput, false, true, 70));

        auto frPair = std::make_unique<ContainerWidget>(std::make_unique<VerticalStackLayout>());
        static_cast<VerticalStackLayout*>(frPair->layout.get())->spacing = 1;
        auto frLabel = std::make_unique<LabelWidget>("Flight Rules:");
        frLabel->preferredSize.width = 70; frLabel->style.fontKey = FONT_KEY_UI_LABEL;
        frPair->AddChild(std::move(frLabel));
        flightRulesCombo = new ComboBoxWidget();
        flightRulesCombo->AddItems({ "IFR", "VFR", "SVFR" });
        flightRulesCombo->preferredSize = { 80, 22 }; flightRulesCombo->minSize = { 80, 22 };
        flightRulesCombo->style.fontKey = FONT_KEY_UI_LABEL;
        frPair->AddChild(std::unique_ptr<ComboBoxWidget>(flightRulesCombo));
        line1->AddChild(std::move(frPair));

        amendButton = new ButtonWidget("AMEND PLAN");
        amendButton->preferredSize = { 100, 22 }; amendButton->minSize = { 100,22 };
        amendButton->OnClick = std::bind(&FlightPlanWindow::OnAmendClicked, this, std::placeholders::_1);
        line1->AddChild(std::unique_ptr<ButtonWidget>(amendButton));
    }
    contentArea->AddChild(std::move(line1));

    // --- Line 2: Depart, Arrive, Alternate | Refresh Button ---
    auto line2 = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(line2->layout.get())->spacing = 10;
    {
        line2->AddChild(createLabelInputPair("Depart:", departInput, false, true, 60));
        departInput->maxLength = 4; departInput->forceCaps = true;
        line2->AddChild(createLabelInputPair("Arrive:", arriveInput, false, true, 60));
        arriveInput->maxLength = 4; arriveInput->forceCaps = true;
        line2->AddChild(createLabelInputPair("Alternate:", alternateInput, false, true, 60));
        alternateInput->maxLength = 4; alternateInput->forceCaps = true;

        // Spacer to push button to the right
        auto spacer = std::make_unique<Widget>(); spacer->preferredSize = { 50,5 }; // Adjust width as needed
        line2->AddChild(std::move(spacer));


        refreshButton = new ButtonWidget("REFRESH PLAN");
        refreshButton->preferredSize = { 100, 22 }; refreshButton->minSize = { 100,22 };
        refreshButton->OnClick = std::bind(&FlightPlanWindow::OnRefreshClicked, this, std::placeholders::_1);
        line2->AddChild(std::unique_ptr<ButtonWidget>(refreshButton));
    }
    contentArea->AddChild(std::move(line2));

    // --- Line 3: Cruise, Scratch, Squawk | Assign Squawk Button ---
    auto line3 = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(line3->layout.get())->spacing = 10;
    {
        line3->AddChild(createLabelInputPair("Cruise Alt:", cruiseInput, false, true, 60));
        cruiseInput->maxLength = 5; cruiseInput->numbersOnly = true;
        line3->AddChild(createLabelInputPair("Scratchpad:", scratchpadInput, true, true, 60)); // Read-only
        line3->AddChild(createLabelInputPair("Squawk:", squawkInput, true, true, 60));         // Read-only

        auto spacer = std::make_unique<Widget>(); spacer->preferredSize = { 30,5 };
        line3->AddChild(std::move(spacer));


        assignSquawkButton = new ButtonWidget("ASSIGN SQUAWK");
        assignSquawkButton->preferredSize = { 120, 22 }; assignSquawkButton->minSize = { 120,22 };
        assignSquawkButton->OnClick = std::bind(&FlightPlanWindow::OnAssignSquawkClicked, this, std::placeholders::_1);
        line3->AddChild(std::unique_ptr<ButtonWidget>(assignSquawkButton));
    }
    contentArea->AddChild(std::move(line3));

    // --- Line 4: Route Label & TextBox ---
    auto routeContainer = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(routeContainer->layout.get())->spacing = 5;
    {
        auto routeLabel = std::make_unique<LabelWidget>("Route:");
        routeLabel->preferredSize = { 70, 20 }; routeLabel->style.fontKey = FONT_KEY_UI_LABEL;
        routeContainer->AddChild(std::move(routeLabel));
        routeBox = new TextBoxWidget();
        routeBox->minSize = { 300, 40 }; routeBox->preferredSize = { 0, 50 }; // Width will be stretched by layout
        routeBox->style.fontKey = FONT_KEY_UI_LABEL;
        routeBox->SetReadOnly(false); // For future editing, but interaction is via Amend button for now
        routeContainer->AddChild(std::unique_ptr<TextBoxWidget>(routeBox));
    }
    contentArea->AddChild(std::move(routeContainer));

    // --- Line 5: Remarks Label & TextBox ---
    auto remarksContainer = std::make_unique<ContainerWidget>(std::make_unique<HorizontalStackLayout>());
    static_cast<HorizontalStackLayout*>(remarksContainer->layout.get())->spacing = 5;
    {
        auto remarksLabel = std::make_unique<LabelWidget>("Remarks:");
        remarksLabel->preferredSize = { 70, 20 }; remarksLabel->style.fontKey = FONT_KEY_UI_LABEL;
        remarksContainer->AddChild(std::move(remarksLabel));
        remarksBox = new TextBoxWidget();
        remarksBox->minSize = { 300, 30 }; remarksBox->preferredSize = { 0, 40 }; // Width will be stretched
        remarksBox->style.fontKey = FONT_KEY_UI_LABEL;
        remarksBox->SetReadOnly(false);
        remarksContainer->AddChild(std::unique_ptr<TextBoxWidget>(remarksBox));
    }
    contentArea->AddChild(std::move(remarksContainer));
}

void FlightPlanWindow::LoadData() {
    // Set the global 'opened_fp' to link with legacy systems that might still use it.
    // Be cautious with this global; ideally, it should be phased out.
    if (targetAircraft) {
        opened_fp = targetAircraft; // This line directly interacts with the global.
    }
    else {
        // If it's a new plan, there's no existing aircraft object for opened_fp
        // unless one is created/found based on initialCallsign. For now, leave opened_fp.
    }


    if (targetAircraft && targetAircraft->getFlightPlan() && targetAircraft->getFlightPlan()->cycle > 0) {
        isNewPlan = false; // It has an existing flight plan data from server
        std::string pilotFullName = targetAircraft->getIdentity() ? targetAircraft->getIdentity()->login_name : "N/A";
        std::string pilotRatingStr = (targetAircraft->getIdentity() && targetAircraft->getIdentity()->pilot_rating >= 0 && targetAircraft->getIdentity()->pilot_rating < 3) ? PILOT_RATINGS[targetAircraft->getIdentity()->pilot_rating] : "N/A";

        title = initialCallsign + " - FLIGHT PLAN (" + pilotFullName + " " + pilotRatingStr + ")";
        WindowWidget::title = title; // Update window title bar text

        PopulateWidgetsFromFlightPlan(*targetAircraft->getFlightPlan(), initialCallsign, pilotFullName, pilotRatingStr);
    }
    else {
        // Either targetAircraft is null (new plan for callsign) or aircraft exists but no FP data (cycle == 0)
        isNewPlan = true;
        title = initialCallsign + " - FLIGHT PLAN (New)";
        WindowWidget::title = title;

        PopulateEmptyForCallsign(initialCallsign);
    }
    MarkDirty(true); // Redraw window and content
}

void FlightPlanWindow::PopulateWidgetsFromFlightPlan(const FlightPlan& fp, const std::string& callsign, const std::string& pilotFullName, const std::string& pilotRatingStr) {
    callsignDisplayInput->SetText(callsign);
    // Window title is already updated in LoadData
    acTypeInput->SetText(fp.acType);
    flightRulesCombo->SetSelectedIndex(fp.flightRules); // Assumes enum maps directly
    departInput->SetText(fp.departure);
    arriveInput->SetText(fp.arrival);
    alternateInput->SetText(fp.alternate);
    cruiseInput->SetText(fp.cruise);
    scratchpadInput->SetText(fp.scratchPad);
    squawkInput->SetText(fp.squawkCode);

    routeBox->Clear();
    if (!fp.route.empty()) routeBox->AddLine(fp.route, style.foregroundColor); else routeBox->AddLine(" ", style.foregroundColor);

    remarksBox->Clear();
    if (!fp.remarks.empty()) remarksBox->AddLine(fp.remarks, style.foregroundColor); else remarksBox->AddLine(" ", style.foregroundColor);
}

void FlightPlanWindow::PopulateEmptyForCallsign(const std::string& callsign_str) {
    callsignDisplayInput->SetText(callsign_str);
    acTypeInput->SetText("C172/L"); // Default
    flightRulesCombo->SetSelectedIndex(0); // IFR default
    departInput->SetText("");
    arriveInput->SetText("");
    alternateInput->SetText("");
    cruiseInput->SetText("10000"); // Default
    scratchpadInput->SetText("");
    squawkInput->SetText("0000"); // Default
    routeBox->Clear(); routeBox->AddLine(" ", style.foregroundColor);
    remarksBox->Clear(); remarksBox->AddLine(" ", style.foregroundColor);
}


void FlightPlanWindow::OnAmendClicked(Widget* sender) {
    if (!targetAircraft) {
        // This is a new plan. We need to find or create an Aircraft object.
        // For now, let's assume this button is less active for completely new plans
        // or that 'targetAircraft' would be created when the FP window is made for a "known unknown".
        // This part needs integration with how your application handles aircraft creation for FPs.
        auto it = acf_map.find(initialCallsign);
        if (it != acf_map.end()) {
            targetAircraft = it->second;
        }
        else {
            sendErrorMessage("Cannot amend: Aircraft " + initialCallsign + " not found.");
            return;
        }
    }

    FlightPlan* fp = targetAircraft->getFlightPlan();
    if (!fp) return; // Should not happen if targetAircraft is valid

    bool changed = false;

    // Helper to update string and track changes
    auto updateIfChanged = [&](std::string& fp_field, const std::string& widget_text) {
        if (fp_field != widget_text) {
            fp_field = widget_text;
            changed = true;
        }
        };
    auto updateIntIfChanged = [&](int& fp_field, int widget_val) {
        if (fp_field != widget_val) {
            fp_field = widget_val;
            changed = true;
        }
        };


    updateIfChanged(fp->acType, acTypeInput->GetText());
    updateIntIfChanged(fp->flightRules, flightRulesCombo->GetSelectedIndex());
    updateIfChanged(fp->departure, departInput->GetText());
    updateIfChanged(fp->arrival, arriveInput->GetText());
    updateIfChanged(fp->alternate, alternateInput->GetText());
    updateIfChanged(fp->cruise, cruiseInput->GetText());
    // scratchpadInput is read-only from server data
    // squawkInput is display-only, set by Assign Squawk or server

    std::vector<std::string> routeLines = routeBox->GetAllLogicalLinesText();
    std::string routeTextConcatenated;
    for (size_t i = 0; i < routeLines.size(); ++i) {
        routeTextConcatenated += routeLines[i];
        if (i < routeLines.size() - 1) {
            routeTextConcatenated += "\n"; // Or " " depending on how multi-line input was handled
        }
    }
    updateIfChanged(fp->route, routeTextConcatenated);

    std::vector<std::string> remarksLines = remarksBox->GetAllLogicalLinesText();
    std::string remarksTextConcatenated;
    for (size_t i = 0; i < remarksLines.size(); ++i) {
        remarksTextConcatenated += remarksLines[i];
        if (i < remarksLines.size() - 1) {
            remarksTextConcatenated += "\n"; // Or " " depending on how multi-line input was handled
        }
    }
    updateIfChanged(fp->remarks, remarksTextConcatenated);

    // The Squawk code is usually assigned by ATC or system, not directly amended by this generic button.
    // If 'squawkInput' were editable and meant to be saved here, you'd include:
    // updateIfChanged(fp->squawkCode, squawkInput->GetText());

    if (changed || isNewPlan) { // If it's a new plan, always increment cycle and send
        fp->cycle++;
        if (connected) {
            sendFlightPlan(*targetAircraft);
            sendSystemMessage("Flight plan for " + targetAircraft->getCallsign() + " amended and sent.");
        }
        else {
            sendErrorMessage("Not connected. Flight plan saved locally.");
        }
        isNewPlan = false; // After first amend, it's no longer "new" in terms of needing initial send
        // Optionally, update title if it was "(New)"
        title = initialCallsign + " - FLIGHT PLAN";
        if (targetAircraft && targetAircraft->getIdentity()) {
            title += " (" + targetAircraft->getIdentity()->login_name + " " +
                ((targetAircraft->getIdentity()->pilot_rating >= 0 && targetAircraft->getIdentity()->pilot_rating < 3) ? PILOT_RATINGS[targetAircraft->getIdentity()->pilot_rating] : "N/A") + ")";
        }
        WindowWidget::title = title;
        MarkDirty(true);
    }
    else {
        sendSystemMessage("No changes to amend for " + initialCallsign + ".");
    }
}

void FlightPlanWindow::OnRefreshClicked(Widget* sender) {
    if (targetAircraft) {
        if (connected) {
            sendFlightPlanRequest(*targetAircraft);
            sendSystemMessage("Flight plan refresh requested for " + targetAircraft->getCallsign() + ".");
        }
        else {
            sendErrorMessage("Not connected. Cannot refresh flight plan.");
        }
    }
    else {
        sendErrorMessage("No aircraft selected to refresh flight plan.");
    }
    // The actual update of the UI will happen when the server responds with packet 17 (flight plan data)
    // This will require a mechanism for the packet handler to find this FlightPlanWindow instance
    // (if it's open) and call `UpdateData(aircraft)` or `LoadData()`.
}

void FlightPlanWindow::OnAssignSquawkClicked(Widget* sender) {
    if (!targetAircraft) {
        auto it = acf_map.find(initialCallsign);
        if (it != acf_map.end()) {
            targetAircraft = it->second;
        }
        else {
            sendErrorMessage("Cannot assign squawk: Aircraft " + initialCallsign + " not found for assignment.");
            return;
        }
    }

    FlightPlan* fp = targetAircraft->getFlightPlan();
    if (!fp) return;

    std::string newSquawk = GenerateNewSquawk();
    squawkInput->SetText(newSquawk);

    // Important: Also update the FlightPlan struct and send it if connected
    if (fp->squawkCode != newSquawk) {
        fp->squawkCode = newSquawk;
        fp->cycle++; // Increment cycle as data changed
        if (connected) {
            sendFlightPlan(*targetAircraft);
            sendSystemMessage("New squawk " + newSquawk + " assigned to " + targetAircraft->getCallsign() + " and plan sent.");
        }
        else {
            sendErrorMessage("Not connected. Squawk assigned locally.");
        }
        // If the aircraft is the globally selected one (ASEL), its callsign tag might need an update
        if (ASEL == targetAircraft) {
            targetAircraft->setUpdateFlag(ACF_CALLSIGN, true);
        }
    }
}

void FlightPlanWindow::UpdateData(Aircraft* aircraft) {
    if (this->targetAircraft == aircraft || (this->initialCallsign == aircraft->getCallsign())) {
        this->targetAircraft = aircraft; // Update pointer if it was previously null or different
        this->initialCallsign = aircraft->getCallsign(); // Ensure callsign is current
        isNewPlan = false; // Data is being updated from a known aircraft
        LoadData(); // Reload all data into widgets
    }
}