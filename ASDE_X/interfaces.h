#ifndef __INTERFACES_H
#define __INTERFACES_H

#include "main.h"
#include "packets.h"

extern std::vector<std::string> pm_callsigns;

void RenderControllerList(bool open, double x_, double y_);

void RenderConnect(double x, double y);

void LoadPrivateChat(double x_, double y_, std::string callsign, bool refresh, int id);

void LoadMainChatInterface(bool refresh);

void sendSystemMessage(std::string message);

void sendErrorMessage(std::string message);

void sendMainChatMessage(InputField* focusField);

void sendPrivateChatMessage(InterfaceFrame& frame, InputField* focusField);

void sendATCMessage(std::string message);

extern InterfaceFrame* controller_list, *main_chat;

#endif
