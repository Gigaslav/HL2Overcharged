//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "OVRCommonButton.h"
#include "tier1/convar.h"
#include "tier1/KeyValues.h"
#include "vgui_controls/RadioButton.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
COVRCommonButton::COVRCommonButton(Panel *parent, const char *panelName, const char *text) : BaseClass(parent, panelName, text)
{

}

void COVRCommonButton::DoClick()
{
	/*COVROptions *parent = (COVROptions*)GetParent();
	parent->SpawnPage(RefPage);*/

	char filename[FILENAME_MAX];

	Q_snprintf(filename, sizeof(filename), "exec %s", cvar->FindVar("oc_config_default_preset")->GetString());

	engine->ClientCmd(filename);

	COVROptions *parent = (COVROptions*)GetParent();
	if (parent)
		parent->UpdatePagesInfo();

	SetSelected(false);
}

void COVRCommonButton::OnCommand(const char* pcCommand)
{
	if (!Q_stricmp(pcCommand, "PressButton"))
	{
		DevMsg("OnCommand()\n");
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COVRCommonButton::Paint()
{
	BaseClass::Paint();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *panel - 
//-----------------------------------------------------------------------------
void COVRCommonButton::SetSelected(bool state)
{
	BaseClass::SetSelected(state);
}

