#ifndef COVRCommonButton_H
#define COVRCommonButton_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/Button.h>
#include "OVROptions.h"

class COVRCommonButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE(COVRCommonButton, vgui::Button);

public:
	COVRCommonButton(Panel *parent, const char *panelName, const char *text);

	virtual void OnCommand(const char* pcCommand);

	virtual void	SetSelected(bool state);
	virtual void	Paint();


	MESSAGE_FUNC(DoClick, "PressButton");
	MESSAGE_FUNC(OnHotkey, "Hotkey")
	{
		DoClick();
	}

	string_t RefPage;

private:


};

#endif