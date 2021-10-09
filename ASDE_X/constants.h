#ifndef CONSTANTS_H
#define CONSTANTS_H

enum class CLIENT_TYPES { CONTROLLER_CLIENT = 0, PILOT_CLIENT = 1 };

extern const char* PILOT_RATINGS[], * CONTROLLER_RATINGS[];

#define MAX_PMCHATS 10

// if a display box is editable you need to leave the next slot free for the inputfield

//Children Interfaces
#define CONTENT_PANE 0
#define FRAME_BOUNDS 1

//frames
#define CONNECT_INTERFACE 0
#define MAIN_CHAT_INTERFACE 1
#define FP_INTERFACE 2
#define CONTROLLER_INTERFACE 3
#define PRIVATE_MESSAGE_INTERFACE 4
#define PRIVATE_MESSAGE_INTERFACE2 5
#define PRIVATE_MESSAGE_INTERFACE3 6
#define PRIVATE_MESSAGE_INTERFACE4 7
#define PRIVATE_MESSAGE_INTERFACE5 8
#define PRIVATE_MESSAGE_INTERFACE6 9
#define PRIVATE_MESSAGE_INTERFACE7 10
#define PRIVATE_MESSAGE_INTERFACE8 11
#define PRIVATE_MESSAGE_INTERFACE9 12
#define PRIVATE_MESSAGE_INTERFACE10 13
#define TERMINAL_COMMAND 14

//children frames

//CONNECT_INTERFACE CHILDREN
#define CONN_CALLSIGN_LABEL 0
#define FULLNAME_LABEL 1
#define USERNAME_LABEL 2
#define PASSWORD_LABEL 3

#define CALLSIGN_INPUT 4
#define FULLNAME_INPUT 5
#define USERNAME_INPUT 6
#define PASSWORD_INPUT 7

#define RATING_COMBO 8
#define CONN_POSITION_COMBO 9

#define CONN_CLOSE_BUTTON 10
#define CONN_OKAY_BUTTON 11
#define CONN_CANCEL_BUTTON 12


//MAIN_CHAT_INTERFACE CHILDREN
#define MAIN_CHAT_INPUT 0
#define MAIN_CONTROLLERS_BOX 1
#define MAIN_CHAT_MESSAGES 2

//FP_INTERFACE CHILDREN
#define FP_CALLSIGN_LABEL 0
#define FP_ACTYPE_LABEL 1
#define FP_FLIGHTRULES_LABEL 2
#define FP_DEPART_LABEL 3
#define FP_ARRIVE_LABEL 4
#define FP_ALTERNATE_LABEL 5
#define FP_CRUISE_LABEL 6
#define FP_SCRATCH_LABEL 7
#define FP_SQUAWK_LABEL 8
#define FP_ROUTE_LABEL 9
#define FP_REMARKS_LABEL 10

#define FP_CALLSIGN_INPUT 11
#define FP_ACTYPE_INPUT 12
#define FP_FLIGHTRULES_INPUT 13
#define FP_DEPART_INPUT 14
#define FP_ARRIVE_INPUT 15
#define FP_ALTERNATE_INPUT 16
#define FP_CRUISE_INPUT 17
#define FP_SCRATCH_INPUT 18
#define FP_SQUAWK_INPUT 19

#define FP_ROUTE_BOX 20
#define FP_ROUTE_EDIT 21// if a display box is editable you need to leave the next slot free for the inputfield
#define FP_REMARKS_BOX 22
#define FP_REMARKS_EDIT 23

#define FP_AMMEND_PLAN 24
#define FP_REFRESH_PLAN 25
#define FP_ASSIGN_SQUAWK 26

#define FP_CLOSE_BUTTON 27

//PRIVATE_MESSAGE_INTERFACE CHILDREN
#define PRIVATE_MESSAGE_INPUT 0
#define PRIVATE_MESSAGE_BOX 1
#define PM_CLOSE_BOX 2

//CONTROLLER_LIST CHILDREN
#define CONTROLLER_LIST_BOX 0
#define CONTROLLER_LIST_ALPHA 1
#define CONTROLLER_LIST_POF 2
#define CONTROLLER_LIST_INFOBOX 3
#define CONTROLLER_LIST_CLOSE 4

//TERMINAL CHILDREN
#define TERMINAL_INPUT 0

//GLOBAL FLAG COUNT
#define NUM_FLAGS 5

//GLOBAL FLAGS (UPDATES ALL)
#define GBL_COLLISION_LINE 0
#define GBL_CALLSIGN 1
#define GBL_COLLISION 2
#define GBL_COLLISION_TAG 3
#define GBL_VECTOR 4

//AIRCRAFT FLAGS COUNT
#define ACF_FLAG_COUNT 4

//AIRCRAFT FLAG CONSTANTS
#define ACF_CALLSIGN 0
#define ACF_COLLISION 1
#define ACF_COLLISION_TAG 2
#define ACF_VECTOR 3

//MIRROR FLAG COUNT
#define MIR_FLAG_COUNT 1

//MIRROR FLAGS
#define MIR_COLLISION_LINE 0

//AIRCRAFT FLAGS COUNT
#define COL_FLAG_COUNT 1

//AIRCRAFT FLAG CONSTANTS
#define COL_COLLISION_LINE 0

const double EARTH_RADIUS_NM = 3437.670013352;

//BEGIN LOCAL VARIABLES
#define _WINPOS_FLIGHTPLAN 0
#define _WINPOS_CTRLLIST 1
#define _WINPOS_CONNECT 2

#endif

