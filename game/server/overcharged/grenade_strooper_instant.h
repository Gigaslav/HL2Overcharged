//========= Overcharged 2019. ============//
//
// Purpose:		Shock trooper instant grenade
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	strooper_instantGRENADE_H
#define	strooper_instantGRENADE_H
#include "grenade_strooper_bounce.h"
#include "sprite.h"
#include "particle_system.h"
//#include "npc_playercompanion.h"
#define	MAX_instant_NO_COLLIDE_TIME 0.2

class SmokeTrail;
class CWeaponinstant;

class CSporeLauncherBouncedProjectile : public CBaseGrenade //CHL1BaseGrenade
{
	DECLARE_CLASS(CSporeLauncherBouncedProjectile, CBaseGrenade /*CHL1BaseGrenade*/);
public:

	float		m_fSpawnTime;

	void		Spawn( void );
	void		Precache( void );
	void 		grenade_strooper_instantTouch( CBaseEntity *pOther );
	void		Event_Killed( CBaseEntity *pInflictor, CBaseEntity *pAttacker, float flDamage, int bitsDamageType );
	void		Think(void);
public:
	Vector startVelocity;
	void EXPORT				Detonate(void);
	static CSporeLauncherBouncedProjectile *SporeCreate(const Vector &vecOrigin, const Vector &vecFwd, const QAngle &angAngles, CBaseEntity *pentOwner = NULL);
private:
	float lifetime;
	float velocity;
protected:
	bool	CreateSprites(void);
	CHandle< CParticleSystem >	m_hSpitEffectA;
	CHandle<CSprite>	m_hSpitEffectC;
	DECLARE_DATADESC();
};



//#ifndef CLIENT_DLL

void TE_StickyBolt(IRecipientFilter& filter, float delay, Vector vecDirection, const Vector *origin);

//-----------------------------------------------------------------------------
// ShockRifle
//-----------------------------------------------------------------------------
class CSporeLauncherProjectile : public CBaseCombatCharacter
{
	DECLARE_CLASS(CSporeLauncherProjectile, CBaseCombatCharacter);

public:
	CSporeLauncherProjectile() { };
	~CSporeLauncherProjectile();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);	// changed
	void BoltTouch(CBaseEntity *pOther);
	bool CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;
	static CSporeLauncherProjectile *BoltCreate(const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner = NULL);

protected:
	void Detonate(void);
	bool	CreateSprites(void);
	CHandle< CParticleSystem >	m_hSpitEffectD;
	CHandle<CSprite>	m_hSpitEffectB;
	//CHandle<CSprite>		m_pGlowSprite;

	int		m_iDamage;
	float   m_flDamage;
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
//#endif

#endif	//grenade_strooper_instant_H
