//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OVRGUNPLAY_H
#define OVRGUNPLAY_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/CheckButton.h"
#include "filesystem.h"
#include <vector>

class COVROptions;
//-----------------------------------------------------------------------------
// Purpose: Difficulty selection options
//-----------------------------------------------------------------------------
class COVRGunplay : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COVRGunplay, vgui::PropertyPage);

public:
	COVRGunplay(vgui::Panel *parent, int num);

	virtual void OnResetData();
	void OnApplyChanges(int num);

	MESSAGE_FUNC(OnRadioButtonChecked, "RadioButtonChecked");

private:

	std::vector<vgui::CheckButton*> pGPButtons;
	std::vector<vgui::TextEntry*> pTexts;
	std::vector<vgui::Slider*> pGPSliders;
};


#endif // OPTIONS_SUB_DIFFICULTY_H