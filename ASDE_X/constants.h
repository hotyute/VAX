#ifndef CONSTANTS_H
#define CONSTANTS_H

//Children Interfaces
#define CONTENT_PANE 0
#define FRAME_BOUNDS 1

//frames
#define CONNECT_INTERFACE 0
#define MAIN_CHAT_INTERFACE 1
#define FP_INTERFACE 2
#define PRIVATE_MESSAGE_INTERFACE 3
#define PRIVATE_MESSAGE_INTERFACE2 4
#define PRIVATE_MESSAGE_INTERFACE3 5
#define PRIVATE_MESSAGE_INTERFACE4 6
#define PRIVATE_MESSAGE_INTERFACE5 7
#define PRIVATE_MESSAGE_INTERFACE6 8
#define PRIVATE_MESSAGE_INTERFACE7 9
#define PRIVATE_MESSAGE_INTERFACE8 10
#define PRIVATE_MESSAGE_INTERFACE9 11
#define PRIVATE_MESSAGE_INTERFACE10 12

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
#define FP_REMARKS_BOX 21

#define FP_AMMEND_PLAN 22
#define FP_REFRESH_PLAN 23
#define FP_ASSIGN_SQUAWK 24

#define FP_CLOSE_BUTTON 25

//PRIVATE_MESSAGE_INTERFACE CHILDREN
#define PRIVATE_MESSAGE_INPUT 0
#define PRIVATE_MESSAGE_BOX 1
#define PM_CLOSE_BOX 2

//GLOBAL FLAG COUNT
#define NUM_FLAGS 4

//GLOBAL FLAGS (UPDATES ALL)
#define GBL_COLLISION_LINE 0
#define GBL_CALLSIGN 1
#define GBL_COLLISION 2
#define GBL_COLLISION_TAG 3

//AIRCRAFT FLAGS COUNT
#define ACF_FLAG_COUNT 3

//AIRCRAFT FLAG CONSTANTS
#define ACF_CALLSIGN 0
#define ACF_COLLISION 1
#define ACF_COLLISION_TAG 2

//MIRROR FLAG COUNT
#define MIR_FLAG_COUNT 1

//MIRROR FLAGS
#define MIR_COLLISION_LINE 0

//AIRCRAFT FLAGS COUNT
#define COL_FLAG_COUNT 1

//AIRCRAFT FLAG CONSTANTS
#define COL_COLLISION_LINE 0

#endif

