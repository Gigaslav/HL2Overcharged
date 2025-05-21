#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "materialsystem/IMaterialSystem.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "rendertexture.h"
#include "view_scene.h"
#include "viewrender.h"

#include "dlight.h"
#include "r_efx.h"

#define NV_Material "HUD/NightVision"

#ifdef HL2_CLIENT_DLL
CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectNV)

CLIENTEFFECT_MATERIAL(NV_Material)
CLIENTEFFECT_REGISTER_END()
#endif

static void NightVision_f(void)
{
	//IMaterial *pMaterialNV = materials->FindMaterial(NV_Material, TEXTURE_GROUP_OTHER, true);

	/*C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
	{
		const Vector *vOrigin = &pPlayer->GetAbsOrigin(); //get the local players origin

		if (!pPlayer->pNVLight)
		{
			//C_Scope_light *pNV = new C_Scope_light();
			C_NVLight *pNV = new C_NVLight();
			pPlayer->pNVLight = pNV;

			if (!pPlayer->pNVLight)
				return;

			pNV->TurnOnNVLight();

			//view->SetScreenOverlayMaterial(pMaterialNV); //and draw it on the screen
			//cvar->FindVar("mat_fullbright")->SetValue(1);
			CLocalPlayerFilter filter;
			pPlayer->EmitSound(filter, 0, "Player.NightvisionOn", vOrigin);
		}
		else
		{
			if (pPlayer->pNVLight)
			{
				pPlayer->pNVLight = NULL;
				delete pPlayer->pNVLight;

				//view->SetScreenOverlayMaterial(NULL); //set screenoverlay to nothing
				//cvar->FindVar("mat_fullbright")->SetValue(0);
				CLocalPlayerFilter filter;
				pPlayer->EmitSound(filter, 0, "Player.NightvisionOff", vOrigin); //and play sound
			}
		}
	}*/

/*	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer(); //get the local player 

	if (!pPlayer)
		return;

	const Vector *vOrigin = &pPlayer->GetAbsOrigin(); //get the local players origin
	static bool bDisplayed;

	if (cvar->FindVar("aa_PlrIsDead")->GetInt() == 1)
		return;

	if (cvar->FindVar("aa_allow_nightvision")->GetInt() == 0)
		return;

	if (pPlayer->IsSuitEquipped())
	{
		if (bDisplayed)
		{
			view->SetScreenOverlayMaterial(NULL); //set screenoverlay to nothing
			//cvar->FindVar("mat_fullbright")->SetValue(0);
			CLocalPlayerFilter filter;
			pPlayer->EmitSound(filter, 0, "Player.NightvisionOff", vOrigin); //and play sound
			cvar->FindVar("aa_PlrLight")->SetValue(0);
			cvar->FindVar("aa_PlrCLNVisON")->SetValue(0);
		}
		else
		{
			view->SetScreenOverlayMaterial(pMaterialNV); //and draw it on the screen
			//cvar->FindVar("mat_fullbright")->SetValue(1);
			CLocalPlayerFilter filter;
			pPlayer->EmitSound(filter, 0, "Player.NightvisionOn", vOrigin);
			cvar->FindVar("aa_PlrLight")->SetValue(1);
			cvar->FindVar("aa_PlrCLNVisON")->SetValue(1);
		}
		bDisplayed = !bDisplayed;
	}*/
}

//static ConCommand oc_enable_nightvision("oc_enable_nightvision", NightVision_f);

//ConVar aa_nEXP( "aa_nEXP", "4", FCVAR_REPLICATED, "test" );
//ConVar aa_nDecay( "aa_nDecay", "0", FCVAR_REPLICATED, "test" );
//ConVar aa_nDie( "aa_nDie", "1", FCVAR_REPLICATED, "test" );

//ConVar aa_nX( "aa_nX", "0", FCVAR_REPLICATED, "test" );
//ConVar aa_nY( "aa_nY", "0", FCVAR_REPLICATED, "test" );
//ConVar aa_nZ( "aa_nZ", "0", FCVAR_REPLICATED, "test" );

static void NightVision_dLight(void)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return;

	Vector effect_origin;
	QAngle effect_angles;
	//int		m_iAttachment;

	pPlayer->GetAttachment(pPlayer->LookupAttachment("eyes"), effect_origin, effect_angles);

	dlight_t *dl = effects->CL_AllocDlight(pPlayer->index);
	dl->origin.x = effect_origin.x; //+ cvar->FindVar("aa_nX")->GetFloat();
	dl->origin.y = effect_origin.y; //+ cvar->FindVar("aa_nY")->GetFloat();

	if (pPlayer->GetFlags() & FL_DUCKING)
	{
		dl->origin.z = effect_origin.z - 5;
	}
	else
	{
		dl->origin.z = effect_origin.z + 50;
	}
	dl->color.r = 34;
	dl->color.g = 139;
	dl->color.b = 34;
	dl->color.exponent = 7; // + cvar->FindVar("aa_nEXP")->GetFloat();
	dl->radius = 160.0f;
	dl->die = gpGlobals->curtime + 0.2f;
	dl->decay = 0.0f; //+ cvar->FindVar("aa_nDecay")->GetFloat(); //512.0f;
}

static ConCommand oc_nvislight("oc_nvislight", NightVision_dLight, " Attach dlight to player model.", FCVAR_HIDDEN);	// Attach dlight to player model. developer command.