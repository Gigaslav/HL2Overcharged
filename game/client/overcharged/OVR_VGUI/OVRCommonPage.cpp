//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "OVRCommonPage.h"
#include "tier1/convar.h"
#include "tier1/KeyValues.h"
#include "overcharged/OVR_VGUI/DataParser.h"
#include "vgui_controls/RadioButton.h"

using namespace vgui;

static int GetElemIdx(std::vector<const char *> &collection, const char *pSearch)
{
	int i = 0;
	for (; i < (int)collection.size(); i++)
	{
		if (FStrEq(pSearch, collection.at(i))) return i;
	}

	return i;
}

void COVRCommonPage::UpdateGrid(ConVarList cvarStruct)
{
	auto cvarName = cvarStruct.cvarName;

	auto resName = cvarStruct.resName;

	if (!V_strstr(resName, "#"))
	{
		DevMsg("Error(FillGrid): Localization line %s does't exist\n", resName);
		return;
	}

	if (cvar->FindVar(cvarName) == NULL)
	{
		DevMsg("Error(FillGrid): Command %s does't exist\n", cvarName);
		return;
	}

	switch (cvarStruct.eType)
	{
	case _SLIDER:
	{
		for each (auto pSlider in pSliders)
		{
			if (FStrEq(pSlider->GetName(), cvarName))
			{
				float sVal = cvar->FindVar(cvarName)->GetFloat();
				pSlider->SetValue((int)(sVal / cvarStruct.divider));
			}
		}
	}
	break;
	case _CBOX:
	{
		for each (auto pButton in pButtons)
		{
			if (FStrEq(pButton->GetName(), cvarName))
			{
				bool nVal = cvar->FindVar(cvarName)->GetBool();
				pButton->SetSelected(nVal);
			}
		}
	}
	break;
	case _INT:
	{
		for each (auto pButton in pButtons)
		{
			if (FStrEq(pButton->GetName(), cvarName))
			{
				int iVal = cvar->FindVar(cvarName)->GetInt();
				pButton->SetSelected(iVal);
			}
		}
	}
	break;
	case _TEXT:
	{
		for each (auto pText in pTexts)
		{
			if (FStrEq(pText->GetName(), cvarName))
			{
				const char *pName = cvar->FindVar(cvarName)->GetString();
				pText->SetText(pName);
			}
		}
	}
	break;
	}
}

void COVRCommonPage::FillGrid(ConVarList cvarStruct)
{
	auto cvarName = cvarStruct.cvarName;

	auto resName = cvarStruct.resName;

	if (!V_strstr(resName, "#"))
	{
		DevMsg("Error(FillGrid): Localization line %s does't exist\n", resName);
		return;
	}

	if (cvar->FindVar(cvarName) == NULL)
	{
		DevMsg("Error(FillGrid): Command %s does't exist\n", cvarName);
		return;
	}

	if (!cvar->FindVar(cvarName)->IsFlagSet(FCVAR_ARCHIVE))
	{
		DevMsg("Error(FillGrid): Command %s does't set archive mode\n", cvarName);
		cvar->FindVar(cvarName)->AddFlags(FCVAR_ARCHIVE);
	}

	switch (cvarStruct.eType)
	{
	case _SLIDER:
	{
		pSliders.push_back(new vgui::Slider(this, cvarName));
		int lastIdx = pSliders.size() - 1;
		float sVal = cvar->FindVar(cvarName)->GetFloat();

		pSliders.at(lastIdx)->SetRange((float)cvarStruct.iRangeMin, (float)cvarStruct.iRangeMax);
		pSliders.at(lastIdx)->SetValue((int)(sVal / cvarStruct.divider));
	}
	break;
	case _CBOX:
	{
		pButtons.push_back(new vgui::CheckButton(this, cvarName, resName));
		int lastIdx = pButtons.size() - 1;
		bool nVal = cvar->FindVar(cvarName)->GetBool();
		pButtons.at(lastIdx)->SetSelected(nVal);
	}
	break;
	case _INT:
	{
		pButtons.push_back(new vgui::CheckButton(this, cvarName, resName));
		int lastIdx = pButtons.size() - 1;
		int iVal = cvar->FindVar(cvarName)->GetInt();
		pButtons.at(lastIdx)->SetSelected(iVal);
	}
	break;
	case _TEXT:
	{
		pTexts.push_back(new vgui::TextEntry(this, cvarName));
		int lastIdx = pTexts.size() - 1;
		const char *pName = cvar->FindVar(cvarName)->GetString();
		pTexts.at(lastIdx)->SetText(pName);
	}
	break;
	}

	SetLinkedCommands(cvarStruct);
}
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
COVRCommonPage::COVRCommonPage(vgui::Panel *parent, const char *schemaName, std::vector<ConVarList> cvarStructs/* ConVarList cvarStruct*/) : BaseClass(parent, NULL)
{
	//COVROptions *pOptions = (COVROptions*)parent;
	
	this->cvarStructs = cvarStructs;

	for each (auto cvarStruct in this->cvarStructs)
	{
		FillGrid(cvarStruct);
	}

	char schema[256];
	Q_snprintf(schema, sizeof(schema), "Resource/OVR_UI/%s.res", schemaName);

	LoadControlSettings(schema);
}

void COVRCommonPage::SavePageInfo(std::vector<ConVarList> cvarStructs)
{
	for each (auto cvarStruct in this->cvarStructs)
	{
		SaveGrid(cvarStruct);
	}
}

void COVRCommonPage::UpdatePageInfo()
{
	for each (auto cvarStruct in this->cvarStructs)
	{
		UpdateGrid(cvarStruct);
	}
}

static vgui::CheckButton *GetButton(ConVarList &cvarStruct, std::vector<vgui::CheckButton*> *pButtons)
{
	for (int i = 0; i < (int)pButtons->size(); i++)
	{
		if (FStrEq(pButtons->at(i)->GetName(), cvarStruct.cvarName))
			return pButtons->at(i);
	}

	return NULL;
}
static vgui::Slider *GetSlider(ConVarList &cvarStruct, std::vector<vgui::Slider*> *pSliders)
{
	for (int i = 0; i < (int)pSliders->size(); i++)
	{
		if (FStrEq(pSliders->at(i)->GetName(), cvarStruct.cvarName))
			return pSliders->at(i);
	}

	return NULL;
}
static vgui::TextEntry *GetTextEntry(ConVarList &cvarStruct, std::vector<vgui::TextEntry*> *pTexts)
{
	for (int i = 0; i < (int)pTexts->size(); i++)
	{
		if (FStrEq(pTexts->at(i)->GetName(), cvarStruct.cvarName))
			return pTexts->at(i);
	}

	return NULL;
}
void COVRCommonPage::SaveGrid(ConVarList cvarStruct)
{
	switch (cvarStruct.eType)
	{
		case _SLIDER:
		{
			auto slider = GetSlider(cvarStruct, &pSliders);
			if (slider)
			{
				auto pCvar = cvarStruct.cvarName;

				float iVal = (float)slider->GetValue();

				cvar->FindVar(pCvar)->SetValue((float)(iVal*cvarStruct.divider));
			}
		}
		break;
		case _CBOX:
		{
			auto button = GetButton(cvarStruct, &pButtons);
			if (button)
			{
				bool isSel = button->IsSelected();
				const char *pName = button->GetName();

				cvar->FindVar(pName)->SetValue(isSel);
			}
		}
		break;
		case _INT:
		{
			auto button = GetButton(cvarStruct, &pButtons);
			if (button)
			{
				int isSel = button->IsSelected();
				const char *pName = button->GetName();
				cvar->FindVar(pName)->SetValue(isSel);
			}
		}
		break;
		case _TEXT:
		{
			auto text = GetTextEntry(cvarStruct, &pTexts);
			if (text)
			{
				const int strLen = text->GetTextLength()+1;
				char fieldName[512];
				text->GetText(fieldName, strLen);
				const char *pName = text->GetName();
				cvar->FindVar(pName)->SetValue(fieldName);
			}
		}
		break;
	}

	SetLinkedCommands(cvarStruct);
}

void COVRCommonPage::SetLinkedCommands(ConVarList cvarStruct)
{
	auto cvarName = cvarStruct.cvarName;

	for each (auto lnkVarName in cvarStruct.pLinkedCommands)
	{
		if (cvar->FindVar(cvarName) == NULL)
		{
			DevMsg("Error(FillGrid: LinkedCommand): Command %s does't exist\n", cvarName);
			continue;
		}

		if (!cvar->FindVar(lnkVarName)->IsFlagSet(FCVAR_ARCHIVE))
		{
			DevMsg("Error(FillGrid: LinkedCommand): Command %s does't set archive mode\n", lnkVarName);
			cvar->FindVar(lnkVarName)->AddFlags(FCVAR_ARCHIVE);
		}

		switch (cvarStruct.eType)
		{
		case _SLIDER:
		{
			int idx = GetElemIdx(cvarStruct.pLinkedCommands, lnkVarName);

			bool bNegate = cvarStruct.pLinkedCommandNegate.at(idx);

			int iMult = bNegate ? -1 : 1;

			int iVal = cvar->FindVar(cvarName)->GetInt() * iMult;

			cvar->FindVar(lnkVarName)->SetValue(iVal);
		}
		break;
		case _CBOX:
		{
			int idx = GetElemIdx(cvarStruct.pLinkedCommands, lnkVarName);

			bool bNegate = cvarStruct.pLinkedCommandNegate.at(idx);

			bool bVal = bNegate ? !cvar->FindVar(cvarName)->GetBool() : cvar->FindVar(cvarName)->GetBool();

			cvar->FindVar(lnkVarName)->SetValue(bVal);
		}
		break;
		case _INT:
			cvar->FindVar(lnkVarName)->SetValue(cvar->FindVar(cvarName)->GetInt());
			break;
		case _TEXT:
			cvar->FindVar(lnkVarName)->SetValue(cvar->FindVar(cvarName)->GetString());
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: enables apply button on radio buttons being pressed
//-----------------------------------------------------------------------------
void COVRCommonPage::OnRadioButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}
