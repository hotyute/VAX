#ifndef INTERFACES_H
#define INTERFACES_H

#include "main.h"
#include "packets.h"

void RenderConnect(double x, double y);

void LoadPrivateChat(double x_, double y_, std::string callsign, bool refresh, int index);

void LoadMainChatInterface(bool refresh);

void sendErrorMessage(std::string message);

void sendMainChatMessage(InputField* focusField);

void sendATCMessage(std::string message);

#endif
