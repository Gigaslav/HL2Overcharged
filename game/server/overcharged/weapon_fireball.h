//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	BEES BEEING UNKNOWN TO ME
//
//=============================================================================//

#ifndef WEAPON_FIREBALL
#define WEAPON_FIREBALL

#include "ai_navtype.h"
#include "ai_navgoaltype.h" 
#include "ai_navigator.h"
#include "sprite.h"
#include "beam_shared.h"
#include "smoke_trail.h"

#define CONTROLLER_FIRE_SPRITE_TAIL "sprites/fireball/fireball_side.vmt"
#define CONTROLLER_FIRE_SPRITE		"sprites/fireball/fireball_front.vmt"

#define FIREBALL_USE_NOCLIP_ON_CHAINLINKS 

class CFireBall :  public CBaseCombatCharacter//, virtual public CBaseAnimating
{
	DECLARE_CLASS( CFireBall, CBaseCombatCharacter );

public:
	static const int EXPLOSION_RADIUS = 40;

	CFireBall();
	~CFireBall();

	Class_T						Classify( void ) { return CLASS_BEE; }
	
	void						Spawn( void );
	void						Precache( void );
	void						FireBallTouch( CBaseEntity *pOther );
	void						Explode( void );

	void						IgniteThink( void );
	void						FlyThink( void );
	void						FireThink( void );

	void						CreateFireTrail();

	void						SetGracePeriod( float flGracePeriod );

	//bool						CreateVPhysics( void );

	void						CreateFireBlast( void );
	
	//virtual float				GetDamage() { return m_flDamage; }
	//virtual void				SetDamage(float flDamage) { m_flDamage = flDamage; }
	Vector						GetShootEnemyDir( const Vector &shootOrigin, bool bNoisy = true );

	void						SteerToTarget(float flHomingSpeed);

	unsigned int				PhysicsSolidMaskForEntity( void ) const;

	CHandle<CBaseEntity>		m_hOwner;
	CHandle<CBaseEntity>		m_hTarget;

	static CFireBall			*Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner); //edict_t *pentOwner );

	int							CapabilitiesGet();

protected:

//	float						m_flDamage;

	CHandle<CFireTrail>			m_hRocketTrail;

private:
	float						m_flDelayThink;
	float						m_flGracePeriodEndsAt;

#ifdef FIREBALL_USE_NOCLIP_ON_CHAINLINKS 
	bool						m_bHasNoClip;
#endif
	
	//int						m_nHaloSprite;

	DECLARE_DATADESC();

	CFireBall( const CFireBall & ); // not defined, not accessible
};


#endif //WEAPON_FIREBALL