#ifndef __INTERFACES_H
#define __INTERFACES_H

#include "main.h"
#include "packets.h"

void RenderControllerList(double x_, double y_);

void RenderConnect(double x, double y);

void LoadPrivateChat(double x_, double y_, std::string callsign, bool refresh, int id);

void LoadMainChatInterface(bool refresh);

void sendErrorMessage(std::string message);

void sendMainChatMessage(InputField* focusField);

void sendATCMessage(std::string message);

extern InterfaceFrame* controller_list, *main_chat;

#endif
