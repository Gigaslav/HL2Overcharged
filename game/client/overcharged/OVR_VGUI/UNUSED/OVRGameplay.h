//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OVRGAMEPLAY_H
#define OVRGAMEPLAY_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/Slider.h"
#include "vgui_controls/TextEntry.h"
#include "filesystem.h"

#include <vector>

class COVROptions;
//-----------------------------------------------------------------------------
// Purpose: Difficulty selection options
//-----------------------------------------------------------------------------
class COVRGameplay : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( COVRGameplay, vgui::PropertyPage );

public:
	COVRGameplay(vgui::Panel *parent, int num);

	virtual void OnResetData();
	void OnApplyChanges(int num);

	MESSAGE_FUNC( OnRadioButtonChecked, "RadioButtonChecked" );

private:

	std::vector<vgui::CheckButton*> pGPButtons;
	std::vector<vgui::TextEntry*> pTexts;
	std::vector<vgui::Slider*> pGPSliders;

};


#endif // OPTIONS_SUB_DIFFICULTY_H