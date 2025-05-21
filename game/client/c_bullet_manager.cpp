#if 0
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Flare effects
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "iefx.h"
#include "dlight.h"
#include "view.h"
#include "fx.h"
#include "clientsideeffects.h"
#include "c_pixel_visibility.h"
#include "bullet_manager.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Precahce the effects
/*CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectFlares)
CLIENTEFFECT_MATERIAL("effects/redflare")
CLIENTEFFECT_MATERIAL("effects/yellowflare")
CLIENTEFFECT_MATERIAL("effects/yellowflare_noz")
CLIENTEFFECT_REGISTER_END()*/
extern C_BulletManager *g_pBulletManager;
class C_SimulatedBullet2 : public CBulletManager, CSimpleEmitter
{
public:
	DECLARE_CLASS(C_SimulatedBullet2, CBulletManager);
	DECLARE_CLIENTCLASS();

	C_SimulatedBullet2();

	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual void ClientThink(void);


	int index;
	Vector EndPoint;
private:

};

IMPLEMENT_CLIENTCLASS_DT(C_SimulatedBullet2, DT_SimulatedBullet2, CBulletManager)
//RecvPropInt(RECVINFO(index)),
RecvPropVector(RECVINFO(EndPoint)),
END_RECV_TABLE()

C_SimulatedBullet2::C_SimulatedBullet2() : CSimpleEmitter("C_SimulatedBullet2")
{
	DevMsg("Think\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
}
//-----------------------------------------------------------------------------
// Purpose: Starts the client-side version thinking
//-----------------------------------------------------------------------------
void C_SimulatedBullet2::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	/*if (iLaserAttachment == -1)
	{
		SetupAttachmentPoints();
	}*/
	DevMsg("Think\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

void C_SimulatedBullet2::ClientThink(void)
{
	//C_BulletManager *pBullet = g_pBulletManager;
	//pBullet->entindex;

	DevMsg("Think\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	dlight_t *el = effects->CL_AllocDlight(g_pBulletManager->index);//( index );
	el->origin = g_pBulletManager->EndPoint;

	el->color.r = 38;
	el->color.g = 151;
	el->color.b = 191;
	el->color.exponent = 5;

	el->radius = random->RandomInt(82, 118);
	el->decay = el->radius / 0.05f;
	el->die = gpGlobals->curtime + 0.07f;
}

#endif