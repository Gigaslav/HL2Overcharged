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

class CHudFireMode : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE(CHudFireMode, CHudNumericDisplay);

public:
	CHudFireMode(const char *pElementName);
	void OnThink(void);

	bool ShouldDraw();
private:
	const char *elementName;
};

DECLARE_HUDELEMENT(CHudFireMode);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudFireMode::CHudFireMode(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudFireMode")
{
	elementName = pElementName;
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudFireMode::OnThink(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

	if (pPlayer && pPlayer->GetActiveWeapon())
	{
		CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
		if (pWeapon)
		{
			const char *name = pWeapon->nameOfFireMode;
			if (LOCALIZED_TEXT(name))
			{
				SetLabelText(LOCALIZED_TEXT(name));
			}
		}
	}
}

bool CHudFireMode::ShouldDraw()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	if (CurrentViewID() == VIEW_INTRO_CAMERA)
		return false;

	if (!pPlayer->IsAlive() || !pPlayer->IsSuitEquipped())
		return false;

	return true;
}
