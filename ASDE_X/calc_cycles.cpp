#include "calc_cycles.h"
#include "controller.h"
#include "flightplan.h"
#include "packets.h"
#include "usermanager.h"
#include "main.h"
#include "app_events.h"
#include "tools.h"        // For dist, getLocFromBearing, pnpoly
#include "constants.h"    // For AIRCRAFT_RADIUS
#include "collision.h"    // For runway_polygons, taxiway_polygons, logic (active runways)
#include "filereader.h"   // For filerdr.collisionPaths (if used as another taxiway source)

#include <set> // For unique active runway strings

// Forward declaration for helper
void RemoveCollisionBetween(Aircraft* acf1, Aircraft* acf2);

DWORD __stdcall CalcThread1(LPVOID)
{
    boost::posix_time::ptime start;
    boost::posix_time::ptime end;
    boost::posix_time::time_duration time;

    boost::posix_time::ptime curTime1 = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::ptime curTime2 = boost::posix_time::microsec_clock::local_time();

    while (true)
    {
        start = boost::posix_time::microsec_clock::local_time();

        if (boost::posix_time::time_duration(boost::posix_time::microsec_clock::local_time()
            - curTime1).total_milliseconds() >= 10000)
        {
            sendPingPacket();
            curTime1 = boost::posix_time::microsec_clock::local_time();
        }

        //code here
        update();
        CalculateCollisions();
        CalcDepartures();
        CalcControllerList();

        end = boost::posix_time::microsec_clock::local_time();

        time = (end - start);
        long long time1 = 100L;
        long long time2 = time1 - time.total_milliseconds();
        if (time2 < 1) {
            time2 = 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(time2));

    }
    return 0;
}

void update()
{
}


// --- Helper Functions for ASDE-X Logic ---

// Structure to hold aircraft's current ground context
struct AircraftGroundContext {
    Aircraft* aircraft = nullptr;
    Point2 currentPosition;
    Point2 futurePosition;
    double speed_kts = 0.0;
    double heading_deg = 0.0;
    bool isOnRunway = false;
    std::string currentRunwayId; // ID of the runway it's on (e.g., "09L")
    bool isOnTaxiway = false;
    int currentTaxiwayIndex = -1; // Index if taxiways are in a vector
    // Path currentTaxiwayPath; // If using filerdr.collisionPaths for taxiways
    bool isApproachingHoldShort = false;
    std::string approachingRunwayId;
};

// Function to determine which runway (if any) a point is on
std::string GetRunwayPointIsOn(const Point2& point, const std::vector<std::string>& activeRunways) {
    for (const auto& runway_id : activeRunways) {
        auto it = runway_polygons.find(runway_id);
        if (it != runway_polygons.end()) {
            const std::vector<Point2>& polygon = it->second;
            if (polygon.empty()) continue;

            // Convert Point2 polygon to double arrays for pnpoly
            std::vector<double> vertx(polygon.size());
            std::vector<double> verty(polygon.size());
            for (size_t i = 0; i < polygon.size(); ++i) {
                vertx[i] = polygon[i].x_;
                verty[i] = polygon[i].y_;
            }
            if (pnpoly(polygon.size(), vertx.data(), verty.data(), point.x_, point.y_)) {
                return runway_id;
            }
        }
    }
    return ""; // Not on any of the listed active runways
}

// Function to determine which taxiway (if any) a point is on
// This example uses taxiway_polygons. Adapt if using filerdr.collisionPaths
int GetTaxiwayPointIsOn(const Point2& point) {
    for (size_t i = 0; i < taxiway_polygons.size(); ++i) {
        const std::vector<Point2>& polygon = taxiway_polygons[i];
        if (polygon.empty()) continue;

        std::vector<double> vertx(polygon.size());
        std::vector<double> verty(polygon.size());
        for (size_t j = 0; j < polygon.size(); ++j) {
            vertx[j] = polygon[j].x_;
            verty[j] = polygon[j].y_;
        }
        if (pnpoly(polygon.size(), vertx.data(), verty.data(), point.x_, point.y_)) {
            return static_cast<int>(i);
        }
    }
    // Optionally check filerdr.collisionPaths if they represent different taxiways/aprons
    // for (size_t i = 0; i < filerdr.collisionPaths.size(); ++i) { ... }
    return -1; // Not on any known taxiway
}


// --- Main Collision Calculation ---
void CalculateCollisions() {
    const double PREDICTION_TIME_SECONDS_GROUND = 8.0;
    const double MIN_SPEED_FOR_PREDICTION_KNOTS = 1.0;
    const double AIRCRAFT_EFFECTIVE_RADIUS_NM = 0.0216; // ~40m safety bubble
    const double HOLD_SHORT_PROXIMITY_NM = 0.05; // Approx 90m: distance to consider "approaching" hold short
    const double RUNWAY_EDGE_BUFFER_NM = 0.01; // Small buffer for runway edge checks

    if (acf_map.empty()) {
        return;
    }

    // Get a unique list of currently active runways from the global 'logic' variable
    // (Assuming 'logic' contains designators like "09L", "27R")
    std::set<std::string> uniqueActiveRunways(logic.begin(), logic.end());
    std::vector<std::string> activeRunways(uniqueActiveRunways.begin(), uniqueActiveRunways.end());


    std::vector<AircraftGroundContext> contexts;
    contexts.reserve(acf_map.size());

    // --- Step 1: Gather Context for All Aircraft ---
    for (auto const& [callsign, acf_ptr] : acf_map) {
        if (!acf_ptr) continue;
        AircraftGroundContext ctx;
        ctx.aircraft = acf_ptr;

        acf_ptr->lock();
        ctx.speed_kts = acf_ptr->getSpeed();
        double lat_deg = acf_ptr->getLatitude();
        double lon_deg = acf_ptr->getLongitude();
        ctx.heading_deg = acf_ptr->getHeading();
        acf_ptr->unlock();

        ctx.currentPosition = { lon_deg, lat_deg };
        double travel_dist_nm = (ctx.speed_kts / 3600.0) * PREDICTION_TIME_SECONDS_GROUND;
        ctx.futurePosition = getLocFromBearing(lat_deg, lon_deg, travel_dist_nm, ctx.heading_deg);

        ctx.currentRunwayId = GetRunwayPointIsOn(ctx.currentPosition, activeRunways);
        ctx.isOnRunway = !ctx.currentRunwayId.empty();

        if (!ctx.isOnRunway) {
            ctx.currentTaxiwayIndex = GetTaxiwayPointIsOn(ctx.currentPosition);
            ctx.isOnTaxiway = (ctx.currentTaxiwayIndex != -1);
        }
        contexts.push_back(ctx);
    }

    // --- Step 2: Evaluate Rules and Proximity ---
    for (size_t i = 0; i < contexts.size(); ++i) {
        AircraftGroundContext& ctx1 = contexts[i];
        Aircraft* acf1 = ctx1.aircraft;

        for (size_t j = i + 1; j < contexts.size(); ++j) {
            AircraftGroundContext& ctx2 = contexts[j];
            Aircraft* acf2 = ctx2.aircraft;

            bool collision_detected_this_pair = false;
            std::string collision_reason = ""; // For debugging or more detailed alerts

            // --- Rule A: Basic Proximity (Generic Collision) ---
            double future_separation_nm = dist(ctx1.futurePosition.y_, ctx1.futurePosition.x_,
                ctx2.futurePosition.y_, ctx2.futurePosition.x_);
            if (future_separation_nm < (2 * AIRCRAFT_EFFECTIVE_RADIUS_NM)) {
                // This is a general proximity alert, but we want more specific rules first.
                // We can treat this as a fallback or a lower-priority alert.
                // For now, let's consider this a potential collision.
                collision_detected_this_pair = true;
                collision_reason = "Proximity";
            }

            // --- Rule B: Runway Incursion / Conflict ---
            // B1: acf1 on active runway, acf2 predicted to enter OR on same runway
            if (ctx1.isOnRunway) {
                std::string futureRunwayForAcf2 = GetRunwayPointIsOn(ctx2.futurePosition, activeRunways);
                if (futureRunwayForAcf2 == ctx1.currentRunwayId) {
                    collision_detected_this_pair = true;
                    collision_reason = "Runway Conflict on " + ctx1.currentRunwayId;
                }
            }
            // B2: Symmetric check - acf2 on active runway, acf1 predicted to enter OR on same runway
            if (!collision_detected_this_pair && ctx2.isOnRunway) { // Avoid re-checking if already detected
                std::string futureRunwayForAcf1 = GetRunwayPointIsOn(ctx1.futurePosition, activeRunways);
                if (futureRunwayForAcf1 == ctx2.currentRunwayId) {
                    collision_detected_this_pair = true;
                    collision_reason = "Runway Conflict on " + ctx2.currentRunwayId;
                }
            }

            // --- Rule C: Converging on Taxiway (Simplified) ---
            // If both on *different* taxiways but future positions are close
            if (!collision_detected_this_pair && ctx1.isOnTaxiway && ctx2.isOnTaxiway &&
                ctx1.currentTaxiwayIndex != ctx2.currentTaxiwayIndex) {
                // Using the general proximity check for future positions here is a simplification
                // of complex taxiway intersection logic.
                if (future_separation_nm < (2 * AIRCRAFT_EFFECTIVE_RADIUS_NM)) {
                    collision_detected_this_pair = true;
                    collision_reason = "Taxiway Convergence";
                }
            }
            // If both on the *same* taxiway and predicted to be too close (head-on or overtaking)
            else if (!collision_detected_this_pair && ctx1.isOnTaxiway && ctx2.isOnTaxiway &&
                ctx1.currentTaxiwayIndex == ctx2.currentTaxiwayIndex) {
                if (future_separation_nm < (2 * AIRCRAFT_EFFECTIVE_RADIUS_NM)) {
                    collision_detected_this_pair = true;
                    collision_reason = "Same Taxiway Conflict";
                }
            }

            // --- Rule D: Approaching Active Runway Hold Short Too Fast (Rudimentary) ---
            // This is complex. A simple version: if on a taxiway, moving towards an active runway,
            // and predicted to cross into it.
            if (!collision_detected_this_pair) {
                if (ctx1.isOnTaxiway && ctx1.speed_kts > MIN_SPEED_FOR_PREDICTION_KNOTS) {
                    std::string futureRunwayForAcf1 = GetRunwayPointIsOn(ctx1.futurePosition, activeRunways);
                    if (!futureRunwayForAcf1.empty()) { // Predicted to be on an active runway
                        collision_detected_this_pair = true;
                        collision_reason = acf1->getCallsign() + " potential runway entry " + futureRunwayForAcf1;
                        // This rule is unary but becomes a collision if that runway is occupied by acf2.
                        // For now, simple alert if predicted to enter *any* active runway.
                        // A better check would be: is acf2 on futureRunwayForAcf1?
                        if (ctx2.isOnRunway && ctx2.currentRunwayId == futureRunwayForAcf1) {
                            // Already covered by Rule B
                        }
                        else if (GetRunwayPointIsOn(ctx2.currentPosition, { futureRunwayForAcf1 }) == futureRunwayForAcf1) {
                            // If acf2 is currently on the runway acf1 is about to enter
                            collision_reason += " with " + acf2->getCallsign();
                        }
                    }
                }
                if (ctx2.isOnTaxiway && ctx2.speed_kts > MIN_SPEED_FOR_PREDICTION_KNOTS) {
                    std::string futureRunwayForAcf2 = GetRunwayPointIsOn(ctx2.futurePosition, activeRunways);
                    if (!futureRunwayForAcf2.empty()) {
                        // Similar logic for acf2
                        if (!collision_detected_this_pair) { // only set if not already true
                            collision_detected_this_pair = true;
                            collision_reason = acf2->getCallsign() + " potential runway entry " + futureRunwayForAcf2;
                        }
                        if (ctx1.isOnRunway && ctx1.currentRunwayId == futureRunwayForAcf2) {
                            // Already covered
                        }
                        else if (GetRunwayPointIsOn(ctx1.currentPosition, { futureRunwayForAcf2 }) == futureRunwayForAcf2) {
                            collision_reason += " with " + acf1->getCallsign();
                        }
                    }
                }
            }


            // --- Manage Collision Object State ---
            bool currently_marked_as_colliding = acf1->collisions.count(acf2);

            if (collision_detected_this_pair && !currently_marked_as_colliding) {
                Collision* new_collision = new Collision(acf1, acf2);
                new_collision->setUpdateFlag(COL_COLLISION_LINE, true);
                // You might want to store 'collision_reason' in the Collision object or log it.
                // std::cout << "New Collision: " << acf1->getCallsign() << " and " << acf2->getCallsign() << " Reason: " << collision_reason << std::endl;

                acf1->lock();
                acf1->collisions.emplace(acf2, new_collision);
                if (!acf1->isCollision()) {
                    acf1->setUpdateFlag(ACF_COLLISION, true);
                    acf1->setCollision(true);
                }
                acf1->unlock();

                acf2->lock();
                acf2->collisions.emplace(acf1, new_collision);
                if (!acf2->isCollision()) {
                    acf2->setUpdateFlag(ACF_COLLISION, true);
                    acf2->setCollision(true);
                }
                acf2->unlock();

                addCollisionToMirrors(new_collision);
                Collision_Map.emplace(acf1, new_collision); // Or your canonical key
                Collision_Map.emplace(acf2, new_collision);

            }
            else if (!collision_detected_this_pair && currently_marked_as_colliding) {
                RemoveCollisionBetween(acf1, acf2);
                // std::cout << "Resolved Collision: " << acf1->getCallsign() << " and " << acf2->getCallsign() << std::endl;
            }
        } // end inner loop (acf2)
    } // end outer loop (acf1)
}


// (Keep other functions: add_to_ctrl_list, CalcDepartures, NotifyControllerListUIDirty, etc. as they were,
//  and the helper RemoveCollisionBetween from the previous response)

// ... (rest of your calc_cycles.cpp, including helper function implementations)


// Helper function to remove a collision between two aircraft
void RemoveCollisionBetween(Aircraft* acf1, Aircraft* acf2) {
    if (!acf1 || !acf2) return;

    Collision* collision_to_remove = nullptr;

    acf1->lock();
    auto it1 = acf1->collisions.find(acf2);
    if (it1 != acf1->collisions.end()) {
        collision_to_remove = it1->second;
        acf1->collisions.erase(it1);
        if (acf1->collisions.empty() && acf1->isCollision()) {
            acf1->setUpdateFlag(ACF_COLLISION, true);
            acf1->setCollision(false);
        }
    }
    acf1->unlock();

    acf2->lock();
    auto it2 = acf2->collisions.find(acf1);
    if (it2 != acf2->collisions.end()) {
        if (!collision_to_remove) collision_to_remove = it2->second; // Should be same
        acf2->collisions.erase(it2);
        if (acf2->collisions.empty() && acf2->isCollision()) {
            acf2->setUpdateFlag(ACF_COLLISION, true);
            acf2->setCollision(false);
        }
    }
    acf2->unlock();

    if (collision_to_remove) {
        for (auto it = Collision_Map.begin(); it != Collision_Map.end(); ) {
            if (it->second == collision_to_remove) {
                it = Collision_Map.erase(it);
            }
            else {
                ++it;
            }
        }

        removeCollisionFromMirrors(collision_to_remove);
        collision_to_remove->setUpdateFlag(COL_COLLISION_LINE, true);
        Collision_Del.push_back(collision_to_remove);
    }
}


void CalcDepartures() {
    for (const auto& iter : acf_map)
    {
        Aircraft* acf1 = iter.second;
        if (acf1) {
            FlightPlan& fp = *acf1->getFlightPlan();
            std::string callsign = acf1->getCallsign();

            if (!departures.count(callsign)) {
                std::vector<std::string> points = split(fp.route, " .");

                //check if the aircraft is departing from this airport
                if (boost::iequals(fp.departure, icao) && !icao.empty()) {
                    std::vector<std::string> new_points;

                    int max_points = 2;
                    while (max_points > 0 && points.size() > 0) {
                        new_points.push_back(pop_front(points));
                        max_points--;
                    }
                    departures.emplace(callsign, new_points);
                    rendererFlags["renderDepartures"] = true;
                }
            }
            else
            {

                if (!boost::iequals(fp.departure, icao) || icao.empty())
                {
                }
                else
                {
                    //refresh
                    std::vector<std::string> _points = departures[callsign];

                    std::vector<std::string> points = split(fp.route, " .");

                    std::vector<std::string> new_points;

                    int max_points = 2;
                    while (max_points > 0 && points.size() > 0) {
                        new_points.push_back(pop_front(points));
                        max_points--;
                    }

                    if (new_points.size() > 0)
                    {
                        if (!boost::iequals(new_points[0], _points[0])
                            || (new_points.size() > 1 && !boost::iequals(new_points[1], _points[1])))
                        {
                            departures[callsign] = new_points;
                            rendererFlags["renderDepartures"] = true;
                        }
                    }
                }


                //check for removal
            }
        }
    }
}

void CalcControllerList() {
    if (!controller_map.empty())
    {
        for (int i = 0; i < 9; i++)
        {

        }
    }
}

// The function called by the calculation thread is now just a notification.
void NotifyControllerListUIDirty() {
    if (hWnd) { // Ensure main window handle is valid
        PostMessage(hWnd, WM_APP_REFRESH_CONTROLLER_LIST, 0, 0);
    }
}

void add_to_ctrl_list(std::string callsign, std::vector<std::string>& data,
    std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store)
{
    auto c = std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, controller_list_box);
    std::string controller_user;
    for (size_t i = 0; i < 7; i++)
    {
        if (i < data[1].length())
            controller_user += data[1][i];
        else
            controller_user += " ";
    }
    for (size_t i = 0; i < 16; i++)
    {
        if (i < callsign.length())
            controller_user += callsign[i];
        else
            controller_user += " ";
    }
    controller_user += data[2];
    c->setText(controller_user);

    controller_list_box->resetReaderIdx();

    if (atodd(data[3]) == 10 || atodd(data[3]) == 11)
        c->setType(CHAT_TYPE::SUP_POS);

    controller_list_box->addLineTop(c);
    controller_list_box->consolidate_lines();
    controller_list_box->gen_points();

    store.emplace(callsign, c);
}

void add_to_qlctrl_list(std::string callsign, std::vector<std::string>& data,
    std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store)
{
    auto c = std::make_shared<ChatLine>("", CHAT_TYPE::MAIN, qlc_list_box);
    std::string controller_user;
    for (size_t i = 0; i < 4; i++)
    {
        if (i < data[1].length())
            controller_user += data[1][i];
        else
            controller_user += " ";
    }
    int white_space = 10 - callsign.size();

    while (white_space > 0)
    {
        controller_user += " ";
        --white_space;
    }

    controller_user += callsign;

    c->setText(controller_user);

    qlc_list_box->resetReaderIdx();

    if (atodd(data[3]) == 10 || atodd(data[3]) == 11)
        c->setType(CHAT_TYPE::SUP_POS);

    qlc_list_box->addLineTop(c);
    qlc_list_box->consolidate_lines();
    qlc_list_box->gen_points();

    store.emplace(callsign, c);
}


void append_to_ctlr_List(std::string callsign, Controller& c, std::unordered_map<std::string, std::shared_ptr<ChatLine>>& list,
    std::unordered_map<std::string, std::shared_ptr<ChatLine>>& ql_list) {
    std::vector<std::string> data;

    data.push_back(std::to_string(static_cast<int>(c.getIdentity()->controller_position)));
    data.push_back("1A");
    data.push_back(frequency_to_string(c.userdata.frequency[0]));
    data.push_back(std::to_string(c.getIdentity()->controller_rating));

    add_to_ctrl_list(callsign, data, list);
    add_to_qlctrl_list(callsign, data, ql_list);
}

void clear_ctrl_list(std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store)
{
    auto it = store.begin();
    while (it != store.end())
    {
        remove_ctrl_list((*it).second);
        it = store.erase(it);
    }
}

void clear_qlctrl_list(std::unordered_map<std::string, std::shared_ptr<ChatLine>>& store)
{
    auto it = store.begin();
    while (it != store.end())
    {
        remove_qlctrl_list((*it).second);
        it = store.erase(it);
    }
}

void clear_ctrl_list()
{
    clear_ctrl_list(obs_list);
    clear_ctrl_list(del_list);
    clear_ctrl_list(gnd_list);
    clear_ctrl_list(twr_list);
    clear_ctrl_list(dep_list);
    clear_ctrl_list(app_list);
    clear_qlctrl_list(ql_obs_list);
    clear_qlctrl_list(ql_del_list);
    clear_qlctrl_list(ql_gnd_list);
    clear_qlctrl_list(ql_twr_list);
    clear_qlctrl_list(ql_dep_list);
    clear_qlctrl_list(ql_app_list);
}

void remove_ctrl_list(std::shared_ptr<ChatLine>& c)
{
    controller_list_box->removeLine(c);
    rendererFlags["drawings"] = true;
}

void remove_qlctrl_list(std::shared_ptr<ChatLine>& c)
{
    qlc_list_box->removeLine(c);
    rendererFlags["drawings"] = true;
}