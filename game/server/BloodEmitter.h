#ifndef	BLOODEMITTER_H
#define	BLOODEMITTER_H
//#include "items.h"
#include "sprite.h"
#include "particle_system.h"

#include "physobj.h"

class SmokeTrail;

#if 0
class CBloodEmitter : public CBaseEntity
{
	DECLARE_CLASS(CBloodEmitter, CBaseCombatCharacter);

public:
	CBloodEmitter() { };
	~CBloodEmitter();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);	// changed
	//void BoltTouch(CBaseEntity *pOther);
	void CreateChildDrips(void);

	//virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }
	//bool CreateVPhysics(void);
	//unsigned int PhysicsSolidMaskForEntity() const;
	CBloodEmitter *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner = NULL);

protected:
	//void Detonate(void);
	bool CreateSprites(void);
	CHandle< CParticleSystem >	m_hSpitEffectDD;

	bool touched;
	int  b_count;
	int		m_iDamage;
	float   m_flDamage;
	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();
};
//#endif
#endif

class CBloodEmitter : public CBaseEntity
{
	DECLARE_CLASS(CBloodEmitter, CBaseEntity);
	//Class_T Classify(void) { return CLASS_NONE; }
public:
	CBloodEmitter() { };
	~CBloodEmitter() { };
	void Precache(void);
	//bool CreateVPhysics();
	void Spawn(void);
	void BubbleThink(void);
	void CreateChildDrips(void);
	CBloodEmitter		*DispatchImpactSound(const Vector &end);
	bool touched;
	int  b_count;
	int  composeColor;
	float timeThink;
	QAngle angle;
	Vector spawn;
	Vector normal;
	Vector direction;
	IPhysicsObject *thisPhysics;
	/*protected:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();*/
};

#endif