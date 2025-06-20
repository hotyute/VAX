#pragma once

#include "main.h"
#include "packets.h"
#include "inputfield.h"

extern std::vector<std::string> pm_callsigns;

void RenderControllerList(bool open, double x_, double y_);

void RenderConnect(double x, double y);

void LoadPrivateChat(double x_, double y_, std::string callsign, bool refresh, bool open, int id);

void RenderTerminalCommands(bool refresh);

void LoadMainChatInterface(bool refresh);

void sendSystemMessage(std::string message);

void sendErrorMessage(std::string message);

void sendMainChatMessage(InputField* focusField);

void sendPrivateChatMessage(InterfaceFrame& frame, InputField* focusField);

void sendATCMessage(std::string message);

extern InterfaceFrame* controller_list, *main_chat, *terminal_cmd;
