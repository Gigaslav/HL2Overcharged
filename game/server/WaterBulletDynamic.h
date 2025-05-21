#ifndef	WATERBULLETDYNAMIC_H
#define	WATERBULLETDYNAMIC_H

#include "basegrenade_shared.h"
#include "sprite.h"
class CParticleSystem;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifndef CLIENT_DLL

//void TE_StickyBolt(IRecipientFilter& filter, float delay, Vector vecDirection, const Vector *origin);

//-----------------------------------------------------------------------------
// ShockRifle
//-----------------------------------------------------------------------------
class CWaterBulletDynamic : public CBaseCombatCharacter
{
	DECLARE_CLASS(CWaterBulletDynamic, CBaseCombatCharacter);

public:
	CWaterBulletDynamic() { };
	~CWaterBulletDynamic();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);	// changed
	void BoltTouch(CBaseEntity *pOther);
	//bool CreateVPhysics(void);
	//unsigned int PhysicsSolidMaskForEntity() const;
	static CWaterBulletDynamic *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBaseEntity *pentOwner = NULL);
	float flActualDamage;
	float fldot;
	float GetBulletSpeedRatio;
	float flActualForce;
	CTakeDamageInfo info;
	int AmmoType;
	int GetDamageType;
	CNetworkVar(bool, underWater);
	Vector m_vecDirShooting;
	trace_t tr0;
	CTraceFilterSimpleList *m_pIgnoreList; //already hit
protected:
	QAngle AbsAngles;

	bool	CreateSprites(void);
	CHandle< CParticleSystem >	m_Spite;
	CHandle<CSprite>		m_Sprite;

	int		m_iDamage;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
#endif

#endif