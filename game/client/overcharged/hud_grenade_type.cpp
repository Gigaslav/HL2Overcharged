//========= Copyright © 2013, BriJee, All rights reserved. ============//
//
// Purpose:	Player active grenade type.
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "fmtstr.h"
#include "vgui_controls/AnimationController.h"
#include "vgui/ILocalize.h"
#include "viewrender.h"
#include <vgui_controls/RichText.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define LOCALIZED_TEXT( name ) \
	( g_pVGuiLocalize->Find( CFmtStr( "#%s", name ) ) )

using namespace vgui;

class CHudGrenType : public CHudElement , public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudGrenType, CHudNumericDisplay );

public:
	CHudGrenType( const char *pElementName );
	void OnThink( void );

	bool ShouldDraw();
private:
	const char *elementName;
};

DECLARE_HUDELEMENT( CHudGrenType );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudGrenType::CHudGrenType( const char *pElementName ) : CHudElement( pElementName ), BaseClass(NULL, "HudGrenType")
{
	elementName = pElementName;
	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
}

void CHudGrenType::OnThink( void )
{
	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
		return;

	if (V_strlen(cvar->FindVar("oc_grenadetype")->GetString()) > 1)
	{

		auto name = MAKE_STRING(cvar->FindVar("oc_grenadetype")->GetString());
		//wchar_t GrenTypeString = (L("%s",));
		
		if (LOCALIZED_TEXT(name))
		{
			SetLabelText(LOCALIZED_TEXT(name));
		}
		else
		{
			wchar_t *GrenTypeString = g_pVGuiLocalize->Find("#GrenType_Hud_EMPTY");
			if (GrenTypeString)
				SetLabelText(GrenTypeString);
			else
				SetLabelText(L"EMPTY");

			//SetLabelText(L"NULL");
		}

		//if (cvar->FindVar("oc_gren1ammo")->GetInt())
		SetDisplayValue(cvar->FindVar("oc_gren1ammo")->GetInt());
	}
	else
	{
		wchar_t *GrenTypeString = g_pVGuiLocalize->Find("#GrenType_Hud_EMPTY");
		if (GrenTypeString)
			SetLabelText(GrenTypeString);
		else
			SetLabelText(L"EMPTY");
	}
}

bool CHudGrenType::ShouldDraw()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer )
		return false;

	if (CurrentViewID() == VIEW_INTRO_CAMERA)
		return false;

	if( !pPlayer->IsAlive() || !pPlayer->IsSuitEquipped() )
		return false;
	
	if (!(cvar->FindVar("oc_player_allow_fast_gren_throw")->GetInt()))
		return false;

	/*if (cvar->FindVar("oc_grenadetype")->GetInt() == 0)
		return false;
	else
		return true;*/
	//if (cvar->FindVar("oc_grenadetype")->GetString() == "")
	/*if (V_strcmp(cvar->FindVar("oc_grenadetype")->GetString(), "") == 0)
		return false;
	else*/
		return true;
}