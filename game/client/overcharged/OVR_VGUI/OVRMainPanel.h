#ifndef MAINPANEL_H
#define MAINPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/Frame.h"
#include "OverCharged/OVR_VGUI/OVROptions.h"
//-----------------------------------------------------------------------------
// Purpose: Benchmark launch dialog
//-----------------------------------------------------------------------------
class CMainPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CMainPanel, vgui::Frame);
	//MESSAGE_FUNC(OpenOVRPanel, "OpenOVRPanel");
public:
	CMainPanel(vgui::VPANEL parent, const char *name);

protected:
	//VGUI overrides:
	virtual void OnTick();
	virtual void OnCommand(const char* pcCommand);
};


#endif // BENCHMARKDIALOG_H