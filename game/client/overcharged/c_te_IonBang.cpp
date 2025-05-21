//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "c_te_particlesystem.h"
#include "fx.h"
#include "ragdollexplosionenumerator.h"
#include "tier1/KeyValues.h"
#include "toolframework_client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Concussive explosion entity
//-----------------------------------------------------------------------------
class C_TEIonExplosion : public C_TEParticleSystem
{
public:
	DECLARE_CLASS(C_TEIonExplosion, C_TEParticleSystem);
	DECLARE_CLIENTCLASS();

	virtual void	PostDataUpdate(DataUpdateType_t updateType);

	void			AffectRagdolls(void);

	Vector	m_vecNormal;
	float	m_flScale;
	int		m_nRadius;
	int		m_nMagnitude;
};


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEIonExplosion, DT_TEIonExplosion, CTE_IonBang)
RecvPropVector(RECVINFO(m_vecNormal)),
RecvPropFloat(RECVINFO(m_flScale)),
RecvPropInt(RECVINFO(m_nRadius)),
RecvPropInt(RECVINFO(m_nMagnitude)),
END_RECV_TABLE()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEIonExplosion::AffectRagdolls(void)
{
	if ((m_nRadius == 0) || (m_nMagnitude == 0))
		return;

	CRagdollExplosionEnumerator	ragdollEnum(m_vecOrigin, m_nRadius, m_nMagnitude);
	partition->EnumerateElementsInSphere(PARTITION_CLIENT_RESPONSIVE_EDICTS, m_vecOrigin, m_nRadius, false, &ragdollEnum);
}


//-----------------------------------------------------------------------------
// Recording 
//-----------------------------------------------------------------------------
static inline void RecordIonExplosion(const Vector& start, const Vector &vecDirection)
{
	if (!ToolsEnabled())
		return;

	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("TempEntity");

		msg->SetInt("te", TE_CONCUSSIVE_EXPLOSION);
		msg->SetString("name", "TE_IonExplosion");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetFloat("originx", start.x);
		msg->SetFloat("originy", start.y);
		msg->SetFloat("originz", start.z);
		msg->SetFloat("directionx", vecDirection.x);
		msg->SetFloat("directiony", vecDirection.y);
		msg->SetFloat("directionz", vecDirection.z);

		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEIonExplosion::PostDataUpdate(DataUpdateType_t updateType)
{
	AffectRagdolls();

	FX_ConcussiveExplosion(m_vecOrigin, m_vecNormal);
	RecordIonExplosion(m_vecOrigin, m_vecNormal);
}

void TE_IonExplosion(IRecipientFilter& filter, float delay, KeyValues *pKeyValues)
{
	Vector vecOrigin, vecDirection;
	vecOrigin.x = pKeyValues->GetFloat("originx");
	vecOrigin.y = pKeyValues->GetFloat("originy");
	vecOrigin.z = pKeyValues->GetFloat("originz");
	vecDirection.x = pKeyValues->GetFloat("directionx");
	vecDirection.y = pKeyValues->GetFloat("directiony");
	vecDirection.z = pKeyValues->GetFloat("directionz");
	FX_ConcussiveExplosion(vecOrigin, vecDirection);
}