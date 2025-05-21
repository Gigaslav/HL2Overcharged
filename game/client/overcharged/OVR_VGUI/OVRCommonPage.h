//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OVRCommonPage_H
#define OVRCommonPage_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/CheckButton.h"
#include "filesystem.h"
#include "OVROptions.h"
#include <vector>

class COVROptions;
struct ConVarList;
//-----------------------------------------------------------------------------
// Purpose: Difficulty selection options
//-----------------------------------------------------------------------------
class COVRCommonPage : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COVRCommonPage, vgui::PropertyPage);

public:
	COVRCommonPage(vgui::Panel *parent, const char *schemaName, std::vector<ConVarList> cvarStructs/* ConVarList cvarStruct*/);

	virtual void OnResetData() {};
	void OnApplyChanges() {};

	void FillGrid(ConVarList cvarStruct);

	void UpdateGrid(ConVarList cvarStruct);

	void SavePageInfo(std::vector<ConVarList> cvarStructs);

	void UpdatePageInfo();

	MESSAGE_FUNC(OnRadioButtonChecked, "RadioButtonChecked");

private:
	void SaveGrid(ConVarList cvarStruct);
	void SetLinkedCommands(ConVarList cvarStruct);

	std::vector<ConVarList> cvarStructs;

	std::vector<vgui::CheckButton*> pButtons;
	std::vector<vgui::TextEntry*> pTexts;
	std::vector<vgui::Slider*> pSliders;
};


#endif // OPTIONS_SUB_DIFFICULTY_H