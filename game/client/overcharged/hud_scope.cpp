//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "c_basehlplayer.h" //alternative #include "c_baseplayer.h"
#include "vguimatsurface/IMatSystemSurface.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMesh.h"
#include "materialsystem/imaterialvar.h"
#include "viewrender.h"
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/**
* Simple HUD element for displaying a sniper scope on screen
*/
class CHudScope : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudScope, vgui::Panel);

public:
	CHudScope(const char *pElementName);

	void Init();
	void MsgFunc_ShowScope(bf_read &msg);
	bool ShouldDraw();
protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint(void);

private:
	int		    m_iRes;
	bool			m_bShow;
	CHudTexture*	m_pScope;
};

DECLARE_HUDELEMENT(CHudScope);
DECLARE_HUD_MESSAGE(CHudScope, ShowScope);

using namespace vgui;

/**
* Constructor - generic HUD element initialization stuff. Make sure our 2 member variables
* are instantiated.
*/
CHudScope::CHudScope(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudScope")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_bShow = false;
	m_pScope = 0;

	// Scope will not show when the player is dead
	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	SetZPos(-1);	// Light Kill : Align type FIX

	// fix for users with diffrent screen ratio (Lodle)
	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

}

/**
* Hook up our HUD message, and make sure we are not showing the scope
*/
void CHudScope::Init()
{
	HOOK_HUD_MESSAGE(CHudScope, ShowScope);

	m_bShow = false;
}

bool CHudScope::ShouldDraw()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	if (CurrentViewID() == VIEW_INTRO_CAMERA)
		return false;

	if (!pPlayer->IsAlive())
		return false;

	return true;
}

/**
* Load  in the scope material here
*/
void CHudScope::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
}

/**
* Simple - if we want to show the scope, draw it. Otherwise don't.
*/
void CHudScope::Paint(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

	if (pPlayer == NULL)
		return;

	if (m_bShow)
	{
		vgui::Panel *pParent = g_pClientMode->GetViewport();

		const char *nameS = GetActiveWeapon()->GetWpnData().twoDScopeTexture;

		m_pScope = gHUD.GetIcon(nameS);

		if (m_pScope)
		{
			//Performing depth hack to prevent clips by world
			m_pScope->DrawSelf(0, 0, pParent->GetWide(), pParent->GetTall(), Color(255, 255, 255, 255));

			int x1 = (GetWide() / 2) - (GetTall() / 2);
			int x2 = GetWide() - (x1 * 2);
			int x3 = GetWide() - x1;

			surface()->DrawSetColor(Color(0, 0, 0, 255));
			surface()->DrawFilledRect(0, 0, x1, GetTall()); //Fill in the left side

			surface()->DrawSetColor(Color(0, 0, 0, 255));
			surface()->DrawFilledRect(x3, 0, GetWide(), GetTall()); //Fill in the right side

			m_pScope->DrawSelf(x1, 0, x2, GetTall(), Color(255, 255, 255, 255)); //Draw the scope as a perfect square

			// Hide the crosshair
			pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
		}

	}
	else if ((pPlayer->m_Local.m_iHideHUD & HIDEHUD_CROSSHAIR) != 0)
	{
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
	}
}


/**
* Callback for our message - set the show variable to whatever
* boolean value is received in the message
*/
void CHudScope::MsgFunc_ShowScope(bf_read &msg)
{
	m_bShow = msg.ReadByte();
}