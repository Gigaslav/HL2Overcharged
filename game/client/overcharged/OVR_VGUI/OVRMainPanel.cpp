//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "overcharged/OVR_VGUI/OVRMainPanel.h"
#include "tier1/KeyValues.h"
#include "tier1/convar.h"
#include "FileSystem.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/CheckButton.h"


using namespace vgui;

bool enable = false;
COVROptions *pdialog = NULL;
//CMainPanel *pPanel = NULL;
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainPanel::CMainPanel(vgui::VPANEL parent, const char *name) : BaseClass(NULL, name)
{
	//Button *button = new Button(this, "RunButton", "RunButton");
	//button->SetCommand(new KeyValues("RunBenchmark"));
	//SetSizeable(false);
	//SetDeleteSelfOnClose(true);
	//SetVisible(false);
	//SetEnabled(false);
	//LoadControlSettings("Resource/BenchmarkDialog.res");

	//pPanel = this;
}

//-----------------------------------------------------------------------------
// Purpose: Launches the benchmark
//-----------------------------------------------------------------------------
/*void CMainPanel::OpenOVRPanel()
{
	delete this;
}*/

void CMainPanel::OnTick()
{
	/*if (enable)
	{
		engine->ClientCmd("ToggleAdvOptions");
		enable = false;

		pdialog = new COptionsDialog(this);
		pdialog->SetDeleteSelfOnClose(true);
		//SetSizeable(true);
		pdialog->Activate();
	}*/

	BaseClass::OnTick();
}
void CMainPanel::OnCommand(const char* pcCommand)
{
	//BaseClass::OnCommand(pcCommand);
	if (!Q_stricmp(pcCommand, "turnoff"))
	{
		//cl_showAdvOptions.SetValue(0);
		//AdvOptions->Destroy();
	}
	//if (!Q_stricmp(pcCommand, "GameUIShow"))
		//AdvOptions->Activate();
}

CON_COMMAND(ToggleAdvOptions, "Advanced options on")
{
	if (enable == false)
	{
		pdialog = new COVROptions(NULL);
		pdialog->SetDeleteSelfOnClose(true);
		pdialog->Activate();
		enable = true;
	}
};
CON_COMMAND(OffAdvOptions, "Advanced options off")
{
	if (enable == true)
	{
		delete pdialog;
		pdialog = NULL;
		enable = false;
	}
};