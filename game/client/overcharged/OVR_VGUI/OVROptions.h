//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyDialog.h"
#include "vgui_controls/KeyRepeat.h"
#include "vgui_controls/ScrollBar.h"
#include "vgui_controls/RadioButton.h"
#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/Slider.h"
#include "OVRCommonPage.h"
#include "OVRCommonButton.h"
#include <vector>
//-----------------------------------------------------------------------------
// Purpose: Holds all the game option pages
//-----------------------------------------------------------------------------

//MikeD
using namespace std;

class COVRCommonPage;
class COVRCommonButton;

enum CvarValue
{
	_CBOX = 0,
	_INT = 1,
	_TEXT,
	_SLIDER
};

struct ConVarList
{
	CvarValue eType;
	int iRangeMin = 0.f;
	int iRangeMax = 1.f;
	//Vector2D vec2;
	float divider = 1.f;
	const char *cvarName;
	const char *resName;
	std::vector<const char *> pLinkedCommands;
	std::vector<bool> pLinkedCommandNegate;
};

class COVROptions : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE( COVROptions, vgui::PropertyDialog );

public:
	COVROptions(vgui::Panel *parent);
	~COVROptions();

	void Run();
	virtual void Activate();
	virtual void OnCancel();

	void InitialManifest();
	void InitialPage(/*COVRCommonPage *pPage, */const char *pPageMainName, const char *pPageLocName);

	virtual void OnSizeChanged(int newWide, int newTall);

	void FillGrid(vector<vgui::CheckButton*> &pButtons, vector<vgui::Slider*> &pSliders, ConVarList arr[], int arrSize);
	void ApplyGrid(vector<vgui::CheckButton*> &pButtons, vector<vgui::Slider*> &pSliders, ConVarList arr[], int arrSize);

	std::vector<ConVarList> cvarStructs;

	/*std::vector<std::vector<ConVarList>> cvarStructsArr;
	std::vector<string_t> pagesNames;
	std::vector<string_t> pagesLocNames;

	std::vector<COVRCommonButton*> pButtons;*/

	void SpawnPage(const char* pPageMainName);

	void UpdatePagesInfo();

private:
	void SavePagesInfo();

	//class COptionsSubAudio *m_pOptionsSubAudio;
	//class COptionsSubVideo *m_pOptionsSubVideo;
	virtual void OnCommand(const char* pcCommand);

	std::vector<COVRCommonPage*> pPages;

	COVRCommonButton *pDefButton;

	/*COVRGameplay *pGap1;
	COVRGameplay *pGap2;
	COVRGameplay *pGap3;
	COVRGameplay *pGap4;

	COVRGunplay *pGup1;
	COVRGunplay *pGup2;
	COVRGunplay *pGup3;

	COVRPostProcessing *pPP1;
	COVRPostProcessing *pPP2;
	COVRPostProcessing *pPP3;*/
};

#define OPTIONS_MAX_NUM_ITEMS 15

struct OptionData_t;

static const char* templatesNames[] =
{
	//{ "Label" },
	{ "Slider" },
	//{ "Button" },
	{ "Checkbutton" },
	/*{ "FloatSlider" },
	{ "RadioButton" },
	{ "ScrollBar" },*/
	{ "TextEntry" },
	/*{ "ToggleButton" },
	{ "ImagePanel" },
	{ "Divider" },
	{ "Combobox" },
	{ "URLLabel" },
	{ "RichText" },
	{ "ScrollBar_Horizontal" },
	{ "ScrollBar_Vertical" },
	{ "ContinuousProgressBar" },
	{ "MessageBox" },
	{ "ProgressBar" },
	{ "ScalableImagePanel" },
	{ "ExpandButton" },
	{ "BuildModeDialog" },
	{ "xdtsadty" },*/
};
#define TemplatesSize	(ARRAYSIZE(templatesNames))

//MikeD
static bool CheckIsTemplate(const char* pName)
{
	for (int i = 0; i < TemplatesSize; i++)
	{
		if (FStrEq(pName, templatesNames[i]))
		{
			return true;
		}
	}

	return false;
}

//MikeD
static bool CheckHasVar(const char *pResName, int size, ConVarList arr[])
{
	bool hasElem = false;

	for (int i = 0; i < size; i++)
	{
		hasElem = FStrEq(pResName, arr[i].cvarName);
		if (hasElem) break;
	}

	return hasElem;
};

//MikeD
/*static void FillGrid(vgui::PropertyPage *Parent, vector<vgui::CheckButton*> &pButtons, vector<vgui::Slider*> &pSliders, vector<vgui::TextEntry*> &pText, ConVarList arr[], int arrSize)
{
	for (int i = 0; i < arrSize; i++)
	{
		const char *cvarName = arr[i].cvarName;

		const char *resName = arr[i].resName;

		if (!V_strstr(resName, "#"))
			continue;

		if (cvar->FindVar(cvarName) == NULL)
		{
			DevMsg("Error(FillGrid): Command %s does't exist\n", cvarName);
			continue;
		}

		switch (arr[i].bVal)
		{
		case _SLIDER:
		{
			pSliders.push_back(new vgui::Slider(Parent, cvarName));
			int lastIdx = pSliders.size() - 1;
			float sVal = cvar->FindVar(cvarName)->GetFloat();

			pSliders.at(lastIdx)->SetRange((int)arr[i].iRangeMin, (int)arr[i].iRangeMax);
			pSliders.at(lastIdx)->SetValue((int)(sVal / arr[i].divider));
		}
		break;
		case _CBOX:
		{

			pButtons.push_back(new vgui::CheckButton(Parent, cvarName, resName));
			int lastIdx = pButtons.size() - 1;
			bool nVal = cvar->FindVar(cvarName)->GetBool();
			pButtons.at(lastIdx)->SetSelected(nVal);
		}
		break;
		case _INT:
		{
			pButtons.push_back(new vgui::CheckButton(Parent, cvarName, resName));
			int lastIdx = pButtons.size() - 1;
			int iVal = cvar->FindVar(cvarName)->GetInt();
			pButtons.at(lastIdx)->SetSelected(iVal);
		}
		break;
		case _TEXT:
		{
			pText.push_back(new vgui::TextEntry(Parent, cvarName));
			int lastIdx = pText.size() - 1;
			const char *pName = cvar->FindVar(cvarName)->GetString();
			pText.at(lastIdx)->SetText(pName);
		}
		break;
		}

		if (!cvar->FindVar(cvarName)->IsFlagSet(FCVAR_ARCHIVE))
			cvar->FindVar(cvarName)->AddFlags(FCVAR_ARCHIVE);
	}

	for (int i = 0; i < arrSize; i++)
	{
		const char *cvarName = arr[i].cvarName;

		const char *resName = arr[i].resName;

		if (CheckHasVar(resName, arrSize, arr))
		{
			switch (arr[i].bVal)
			{
			case _SLIDER:
				cvar->FindVar(cvarName)->SetValue(cvar->FindVar(cvarName)->GetInt());
				break;
			case _CBOX:
				cvar->FindVar(cvarName)->SetValue(cvar->FindVar(resName)->GetBool());
				break;
			case _INT:
				cvar->FindVar(cvarName)->SetValue(cvar->FindVar(cvarName)->GetInt());
				break;
			case _TEXT:
				cvar->FindVar(cvarName)->SetValue(cvar->FindVar(cvarName)->GetString());
				break;
			}
		}
	}
}

//MikeD
static void ApplyGrid(vector<vgui::CheckButton*> &pButtons, vector<vgui::Slider*> &pSliders, vector<vgui::TextEntry*> &pText, ConVarList arr[], int arrSize)
{
	for (int i = 0; i < (int)pButtons.size(); i++)
	{
		bool isSel = pButtons.at(i)->IsSelected();
		const char *pName = pButtons.at(i)->GetName();
		cvar->FindVar(pName)->SetValue(isSel);
	}
	for (int i = 0; i < (int)pSliders.size(); i++)
	{
		float iVal = (float)pSliders.at(i)->GetValue();

		string_t pVar = STRING("");

		float divider = 1.f;

		for (int j = 0; j < arrSize; j++)
		{
			if (FStrEq(pSliders.at(i)->GetName(), arr[j].cvarName))
			{
				pVar = arr[j].cvarName;
				divider = arr[j].divider;
			}
		}

		cvar->FindVar(pVar)->SetValue((float)(iVal*divider));
	}
	for (int i = 0; i < (int)pText.size(); i++)
	{
		const int strLen = pText.at(i)->GetTextLength();
		char *fieldName;
		pText.at(i)->GetText(fieldName, strLen);
		const char *pName = pText.at(i)->GetName();
		cvar->FindVar(pName)->SetValue(fieldName);
	}
	for (int i = 0; i < arrSize; i++)
	{
		const char *cvarName = arr[i].cvarName;

		const char *resName = arr[i].resName;

		if (cvar->FindVar(cvarName) == NULL)
		{
			DevMsg("Error(ApplyGrid): Command %s does't exist\n", cvarName);
			continue;
		}

		if (CheckHasVar(resName, arrSize, arr))
		{
			switch (arr[i].bVal)
			{
			case _SLIDER:
			{
				bool nVal = cvar->FindVar(resName)->GetInt();
				cvar->FindVar(cvarName)->SetValue(nVal);
			}
			break;
			case _CBOX:
			{
				bool nVal = cvar->FindVar(resName)->GetBool();
				cvar->FindVar(cvarName)->SetValue(nVal);
			}
			break;
			case _INT:
			{
				int iVal = cvar->FindVar(cvarName)->GetInt();
				cvar->FindVar(cvarName)->SetValue(iVal);
			}
			break;
			case _TEXT:
			{
				const char* cVal = cvar->FindVar(cvarName)->GetString();
				cvar->FindVar(cvarName)->SetValue(cVal);
			}
			break;
			}
		}
	}
}*/

#endif // OPTIONSDIALOG_H
