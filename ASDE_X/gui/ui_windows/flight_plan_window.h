#pragma once
#include "../widgets/window_widget.h"
#include <string> // For std::string

// Forward declarations for widget types
class LabelWidget;
class InputFieldWidget;
class ComboBoxWidget;
class TextBoxWidget;
class ButtonWidget;

// Forward declaration for application data struct
class Aircraft; // from aircraft.h
struct FlightPlan; // from aircraft.h

class FlightPlanWindow : public WindowWidget {
public:
    // Constructor for an existing flight plan associated with an Aircraft object
    FlightPlanWindow(Aircraft* aircraft);
    // Constructor for a new/unknown flight plan, identified by callsign
    FlightPlanWindow(const std::string& callsign_for_new_fp);

    ~FlightPlanWindow();

    void UpdateData(Aircraft* aircraft); // To refresh if underlying data changes

private:
    void SetupUI();
    void LoadData(); // Loads data based on current aircraft or callsign
    void PopulateWidgetsFromFlightPlan(const FlightPlan& fp, const std::string& callsign, const std::string& pilotFullName, const std::string& pilotRatingStr);
    void PopulateEmptyForCallsign(const std::string& callsign_str);

    // Callbacks
    void OnAmendClicked(Widget* sender);
    void OnRefreshClicked(Widget* sender);
    void OnAssignSquawkClicked(Widget* sender);
    // OnCloseClicked is handled by WindowWidget's default closable behavior

    // Store the target aircraft or callsign
    Aircraft* targetAircraft = nullptr; // If editing an existing aircraft's FP
    std::string initialCallsign;        // Callsign (either from aircraft or for new FP)
    bool isNewPlan = false;             // True if opened for a callsign without existing FP data loaded initially

    // Pointers to UI elements we need to interact with
    // Line 1
    InputFieldWidget* callsignDisplayInput; // Display-only for callsign
    InputFieldWidget* acTypeInput;
    ComboBoxWidget* flightRulesCombo;
    ButtonWidget* amendButton;

    // Line 2
    InputFieldWidget* departInput;
    InputFieldWidget* arriveInput;
    InputFieldWidget* alternateInput;
    ButtonWidget* refreshButton;

    // Line 3
    InputFieldWidget* cruiseInput;
    InputFieldWidget* scratchpadInput; // Display-only
    InputFieldWidget* squawkInput;     // Display-only (updated by Assign Squawk)
    ButtonWidget* assignSquawkButton;

    // Line 4 (Route)
    TextBoxWidget* routeBox;

    // Line 5 (Remarks)
    TextBoxWidget* remarksBox;
};