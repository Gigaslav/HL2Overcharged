//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	BEES BEEING UNKNOWN TO ME
//
//=============================================================================//

#ifndef WEAPON_BEE
#define WEAPON_BEE

#include "ai_navtype.h"
#include "ai_navgoaltype.h" 
#include "ai_navigator.h"

#include "ai_basenpc.h"

#include "SpriteTrail.h"

#define BEE_NAVITATION_DEBUG

#define BEE_USE_NOCLIP_ON_CHAINLINKS 

//#define BEE_USE_NPC_BASE

#define BEE_THINK_INTERVAL 0.08f

#ifdef BEE_USE_NPC_BASE

class CBee :  public  CAI_BaseNPC 
{
	DECLARE_CLASS( CBee,  CAI_BaseNPC ); 
#else

class CBee :  public  CBaseCombatCharacter
{
	DECLARE_CLASS( CBee,  CBaseCombatCharacter ); 
#endif


public:
	CBee();
	~CBee();

	Class_T					Classify( void ) { return CLASS_BEE; }
	
	void					Spawn( void );
	void					Precache( void );
	void					BeeTouch( CBaseEntity *pOther );
	void					Explode( void );
	void					IgniteThink( void );
	void					SeekThink( void );

	void					FlyToEnemy();

	void					LeaveTrail(); // float flTrailTime );

	void					SetGracePeriod( float flGracePeriod );


	void					Event_Killed( const CTakeDamageInfo &info );
	
	Vector					GetShootEnemyDir( const Vector &shootOrigin, bool bNoisy = true );

	unsigned int			PhysicsSolidMaskForEntity( void ) const;

	CHandle<CBaseEntity>	m_hOwner;

	static CBee				*Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner); //edict_t *pentOwner );

protected:
	float					m_flBuzzTime;

private:
	float					m_flGracePeriodEndsAt;
	float					m_flDelayThink;

#ifdef HLSS_BEE_OCCASIONAL_RANDOM_MOVEMENT
	float					m_flNextChangeDirection;
	bool					m_bDirectionToEnemy;
	QAngle					m_angRandomDir;
#endif

#ifndef BEE_USE_NPC_BASE
	Vector					m_vecLastTargetPosition;

public:

	void					SetEnemy( CBaseEntity *pEnemy);
	CBaseEntity*			GetEnemy( void ) { return m_hEnemy; }

private:

	EHANDLE					m_hEnemy;
#endif

	bool					m_bHasNoClip;

	int						m_iBeeFlyOffset;

	CSpriteTrail			*m_pTrail;

	DECLARE_DATADESC();
};


#endif //WEAPON_BEE