//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "OVRGunplay.h"
#include "tier1/convar.h"
#include "tier1/KeyValues.h"
#include "overcharged/OVR_VGUI/DataParser.h"
#include "vgui_controls/RadioButton.h"
#include "OVROptions.h"
using namespace vgui;

static ConVarList cvarGPList1[] =
{
	{ _CBOX, 0, 0, 1, "oc_weapons_allow_inspect_animations", "#OverUI_InspectAnims" },
	{ _CBOX, 0, 0, 1, "oc_particle_ejectbrass_smoke", "#OverUI_EjectBrassSmoke" },
	{ _CBOX, 0, 0, 1, "oc_weapon_megaphyscannon_use_battery", "#OverUI_GravityGunBattery" },
	{ _CBOX, 0, 0, 1, "oc_particle_other_weapons", "#OverUI_OtherWeapParticles" },
	{ _CBOX, 0, 0, 1, "oc_particle_muzzleflashes", "#GameUI_ParticleMuzzle" },
	{ _CBOX, 0, 0, 1, "oc_particle_tracers", "#GameUI_ParticleTracers" },
	{ _SLIDER, 5, 50, 0.01f, "oc_weapon_barnacle_material_width", "#OverUI_BarnacleTongueWidthDesc" },
	{ _SLIDER, 100, 1000, 0.01f, "oc_weapon_barnacle_speed", "#OverUI_barnaclespeed" },
	{ _SLIDER, 7, 25, 0.01f, "oc_weapon_cguard_charge_time", "#OverUI_cguard_charge_time" },
	{ _SLIDER, 25, 100, 0.01f, "oc_jeep_checking_sphere_radius", "#OverUI_jeep_radius" },
	{ _SLIDER, 25, 100, 0.01f, "oc_airboat_checking_sphere_radius", "#OverUI_airboat_radius" },
	{ _CBOX, 0, 0, 1, "oc_allow_pick_stunbaton", "#OverUI_StunstickPickup" },
	{ _CBOX, 0, 0, 1, "oc_allow_pick_dual_pistols", "#OverUI_DualPistolsPickup" },
	{ _CBOX, 0, 0, 1, "oc_wpn_flaregun_flare_stick", "#OverUI_FlareGunStick" },




	{ _INT, 0, 0, 1, "oc_weapon_free_aim", "#OverUI_FreeAim" },
	{ _SLIDER, 10, 27, 0.01f, "oc_weapon_free_aim_movemax", "#OverUI_FreeAimMaxMovement" },

};

#define GPListSize1	(ARRAYSIZE(cvarGPList1))

static ConVarList cvarGPList2[] =
{
	{ _CBOX, 0, 0, 1, "oc_particle_muzzleflashes", "#GameUI_ParticleMuzzle" },
	{ _CBOX, 0, 0, 1, "oc_particle_tracers", "#GameUI_ParticleTracers" },
	{ _CBOX, 0, 0, 1, "oc_particle_ejectbrass_smoke", "#OverUI_EjectBrassSmoke" },
	{ _CBOX, 0, 0, 1, "oc_weapon_megaphyscannon_use_battery", "#OverUI_GravityGunBattery" },
	{ _INT, 0, 0, 1, "oc_weapon_free_aim", "#OverUI_FreeAim" },
	{ _SLIDER, 10, 27, 0.01f, "oc_weapon_free_aim_movemax", "#OverUI_FreeAimMaxMovement" },
	{ _CBOX, 0, 0, 1, "oc_particle_other_weapons", "#OverUI_OtherWeapParticles" },
};

#define GPListSize2	(ARRAYSIZE(cvarGPList2))

static ConVarList cvarGPList3[] =
{
	{ _CBOX, 0, 0, 1, "oc_particle_muzzleflashes", "#GameUI_ParticleMuzzle" },
	{ _CBOX, 0, 0, 1, "oc_particle_tracers", "#GameUI_ParticleTracers" },
	{ _CBOX, 0, 0, 1, "oc_particle_ejectbrass_smoke", "#OverUI_EjectBrassSmoke" },
	{ _CBOX, 0, 0, 1, "oc_weapon_megaphyscannon_use_battery", "#OverUI_GravityGunBattery" },
	{ _INT, 0, 0, 1, "oc_weapon_free_aim", "#OverUI_FreeAim" },
	{ _SLIDER, 10, 27, 0.01f, "oc_weapon_free_aim_movemax", "#OverUI_FreeAimMaxMovement" },
	{ _CBOX, 0, 0, 1, "oc_particle_other_weapons", "#OverUI_OtherWeapParticles" },
};

#define GPListSize3	(ARRAYSIZE(cvarGPList3))
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
COVRGunplay::COVRGunplay(vgui::Panel *parent, int num) : BaseClass(parent, NULL)
{
	//COVROptions *pOptions = (COVROptions*)parent;

	switch (num)
	{
		case 0:
		{
			FillGrid(this, pGPButtons, pGPSliders, pTexts, cvarGPList1, GPListSize1);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsGunplay_01.res");
		}
		break;
		case 1:
		{
			FillGrid(this, pGPButtons, pGPSliders, pTexts, cvarGPList2, GPListSize2);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsGunplay_02.res");
		}
		break;
		case 2:
		{
			FillGrid(this, pGPButtons, pGPSliders, pTexts, cvarGPList3, GPListSize3);
			LoadControlSettings("Resource/OVR_UI/AdvOptionsGunplay_03.res");
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: resets controls
//-----------------------------------------------------------------------------
void COVRGunplay::OnResetData()
{

}

//-----------------------------------------------------------------------------
// Purpose: sets data based on control settings
//-----------------------------------------------------------------------------
void COVRGunplay::OnApplyChanges(int num)
{
	switch (num)
	{
	case 0:
		ApplyGrid(pGPButtons, pGPSliders, pTexts, cvarGPList1, GPListSize1);
		break;
	case 1:
		ApplyGrid(pGPButtons, pGPSliders, pTexts, cvarGPList2, GPListSize2);
		break;
	case 2:
		ApplyGrid(pGPButtons, pGPSliders, pTexts, cvarGPList3, GPListSize3);
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: enables apply button on radio buttons being pressed
//-----------------------------------------------------------------------------
void COVRGunplay::OnRadioButtonChecked()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}
