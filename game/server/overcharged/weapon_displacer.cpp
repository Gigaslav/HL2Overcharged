//========= OverCharged 2018 ==================================================//
//
// Purpose:		OP4 Displacer				20 ammo primary, 60 ammo secondary
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"
#include "weapon_displacer.h"
#include "particle_parse.h"
#include "beam_flags.h"
#include "te_effect_dispatch.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar aa_wpn_displacer_target( "aa_wpn_displacer_target", "disp_target", FCVAR_REPLICATED, "Teleport near the named entity. Usualy made to work with info_displacer_destination" );

//-----------------------------------------------------------------------------
// CWeaponGreenBall
//-----------------------------------------------------------------------------

#define BOLT_MODEL	/*aa_wpn_GreenBall_model.GetString()*/ "models/blackout.mdl" //"models/props_combine/headcrabcannister01a_skybox.mdl"	//models/blackout.mdl

#define BOLT_AIR_VELOCITY	1750
//#define BOLT_WATER_VELOCITY	1200
//#define	BOLT_SKIN_NORMAL	0
//#define BOLT_SKIN_GLOW		1

//extern ConVar sk_plr_dmg_displacer;
//extern ConVar sk_npc_dmg_displacer;
ConVar	sk_plr_dmg_displacer( "sk_plr_dmg_displacer", "130" );
ConVar	sk_npc_dmg_displacer( "sk_plr_dmg_displacer", "130" );

#ifndef CLIENT_DLL

void TE_StickyBolt( IRecipientFilter& filter, float delay,	Vector vecDirection, const Vector *origin );

//-----------------------------------------------------------------------------
// GreenBall
//-----------------------------------------------------------------------------
class CGreenBallProjectile : public CBaseCombatCharacter
{
	DECLARE_CLASS( CGreenBallProjectile, CBaseCombatCharacter );

public:
	CGreenBallProjectile() { };
	~CGreenBallProjectile();

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void BubbleThink( void );	// changed
	void BoltTouch(CBaseEntity *pOther);
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CGreenBallProjectile *BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner = NULL );

protected:
	bool	ShouldDrawWaterImpacts(const trace_t &shot_trace);
	int     m_nLightningSprite1;
	bool	CreateSprites( void );

	//CHandle<CSprite>		m_pGlowSprite;
	//CHandle<CSpriteTrail>	m_pGlowTrail;
	
	int		m_iDamage;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
LINK_ENTITY_TO_CLASS( GreenBall_projectile, CGreenBallProjectile );

BEGIN_DATADESC( CGreenBallProjectile )
	// Function Pointers
	DEFINE_FUNCTION( BubbleThink ),
	DEFINE_FUNCTION( BoltTouch ),

	// These are recreated on reload, they don't need storage
	//DEFINE_FIELD( m_pGlowSprite, FIELD_EHANDLE ),
	//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CGreenBallProjectile, DT_GreenBallProjectile )
END_SEND_TABLE()

CGreenBallProjectile *CGreenBallProjectile::BoltCreate( const Vector &vecOrigin, const QAngle &angAngles, int iDamage, CBasePlayer *pentOwner )
{
	// Create a new entity with CGreenBallProjectile private data
	CGreenBallProjectile *pGreenBallProjectile = (CGreenBallProjectile *)CreateEntityByName( "GreenBall_projectile" );
	UTIL_SetOrigin( pGreenBallProjectile, vecOrigin );
	pGreenBallProjectile->SetAbsAngles( angAngles );
	pGreenBallProjectile->Spawn();
	pGreenBallProjectile->SetOwnerEntity( pentOwner );

	pGreenBallProjectile->m_iDamage = iDamage;

	return pGreenBallProjectile;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGreenBallProjectile::~CGreenBallProjectile( void )
{
	/*if ( m_pGlowSprite )
	{
		UTIL_Remove( m_pGlowSprite );
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CGreenBallProjectile::CreateVPhysics( void )
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CGreenBallProjectile::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CGreenBallProjectile::CreateSprites( void )
{
	//DispatchParticleEffect( "hl1_tp", PATTACH_ABSORIGIN_FOLLOW, this );

	/*// Start up the eye glow
	m_pGlowSprite = CSprite::SpriteCreate( "sprites/light_glow02_noz.vmt", GetLocalOrigin(), false );

	if ( m_pGlowSprite != NULL )
	{
		m_pGlowSprite->FollowEntity( this );
		m_pGlowSprite->SetTransparency( kRenderGlow, 255, 255, 255, 128, kRenderFxNoDissipation );
		m_pGlowSprite->SetScale( 0.2f );
		m_pGlowSprite->TurnOff();
	}*/

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGreenBallProjectile::Spawn( void )
{
	Precache( );

	SetModel( BOLT_MODEL );
	SetMoveType( MOVETYPE_FLY, MOVECOLLIDE_FLY_CUSTOM );
	UTIL_SetSize( this, -Vector(10,10,10), Vector(10,10,10) );
	SetSolid( SOLID_BBOX );

	//AddEffects( EF_NODRAW );
	AddEffects( EF_NOSHADOW );	// BriJee : NO SHADOW ON PARTICLE ENTITY
	
	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch( &CGreenBallProjectile::BoltTouch );

	SetThink( &CGreenBallProjectile::BubbleThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
	
	CreateSprites();
	DispatchParticleEffect("DispBall_exhaust", PATTACH_ABSORIGIN_FOLLOW, this);	//Новый партиклевый эффект дыма
	DispatchParticleEffect("Xen_teleport_idle", PATTACH_ABSORIGIN_FOLLOW, this);	//Новый партиклевый эффект дыма
	DispatchParticleEffect("Xen_portal_big2", PATTACH_ABSORIGIN_FOLLOW, this);	//Новый партиклевый эффект дыма
	// Make us glow until we've hit the wall
	//m_nSkin = BOLT_SKIN_GLOW;
}


void CGreenBallProjectile::Precache( void )
{
	PrecacheModel( BOLT_MODEL );

	//PrecacheModel( "sprites/light_glow02_noz.vmt" );
	PrecacheScriptSound( "Weapon_Displacer.Explode" );	//ball explode
	//PrecacheParticleSystem( "hl1_tp" );
	//PrecacheParticleSystem("houndeye_wave_01");
	m_nLightningSprite1 = PrecacheModel("sprites/bluelaser1.vmt");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CGreenBallProjectile::BoltTouch(CBaseEntity *pOther)
{
	if (pOther)
	{

		if ((!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS)))
			return;

		/*if (pOther->IsBaseObject())//Убирает любую энтитю при столкновении
			UTIL_Remove(pOther);*/

		if (pOther->m_takedamage != DAMAGE_NO)
		{
			trace_t	tr, tr2;
			tr = BaseClass::GetTouchTrace();
			Vector	vecNormalizedVel = GetAbsVelocity();
			ShouldDrawWaterImpacts(tr);

			ClearMultiDamage();
			VectorNormalize(vecNormalizedVel);

			if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC())
			{
				//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_NEVERGIB );
				CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_displacer.GetFloat(), DMG_DISSOLVE); // BriJee : AR2 dmg effect//DMG_SHOCK
				dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
				CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
				dmgInfo.SetDamagePosition(tr.endpos);
				pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
				//MikeN this is funny
				//UTIL_Remove(pOther);
				RadiusDamage(CTakeDamageInfo(this, this, 125 * 10, DMG_DISSOLVE), GetAbsOrigin(), 256, CLASS_NONE, NULL);
				RadiusDamage(CTakeDamageInfo(this, this, 125 * 10, DMG_BLAST), GetAbsOrigin(), 256, CLASS_NONE, NULL);
			}
			else
			{
				//CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_iDamage, DMG_BULLET | DMG_NEVERGIB );
				CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), sk_plr_dmg_displacer.GetFloat(), DMG_DISSOLVE);//DMG_NEVERGIB | DMG_SHOCK
				CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
				dmgInfo.SetDamagePosition(tr.endpos);
				pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
				//MikeN this is funny
				//UTIL_Remove(pOther);
				RadiusDamage(CTakeDamageInfo(this, this, 125 * 10, DMG_DISSOLVE), GetAbsOrigin(), 256, CLASS_NONE, NULL);
				RadiusDamage(CTakeDamageInfo(this, this, 125 * 10, DMG_BLAST), GetAbsOrigin(), 256, CLASS_NONE, NULL);
			}
			//MikeN this is funny
			//UTIL_Remove(pOther);

			ApplyMultiDamage();

			//Adrian: keep going through the glass.
			if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS)
				return;

			SetAbsVelocity(Vector(0, 0, 0));

			// play body "thwack" sound
			//EmitSound( "Weapon_Crossbow.BoltHitBody" );



			/*Vector vForward;

			AngleVectors( GetAbsAngles(), &vForward );
			VectorNormalize ( vForward );

			UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );

			if ( tr2.fraction != 1.0f )
			{
			//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
			//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

			if ( tr2.m_pEnt == NULL || ( tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE ) )
			{
			CEffectData	data;

			data.m_vOrigin = tr2.endpos;
			data.m_vNormal = vForward;
			data.m_nEntIndex = tr2.fraction != 1.0f;

			DispatchEffect( "BoltImpact", data );
			}
			}*/

			SetTouch(NULL);
			SetThink(NULL);

			UTIL_Remove(this);
		}
		else
		{
			trace_t	tr;
			ShouldDrawWaterImpacts(tr);
			Vector vForward;
			AngleVectors(GetAbsAngles(), &vForward);
			VectorNormalize(vForward);
			UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr);

			RadiusDamage(CTakeDamageInfo(this, this, 125 * 10, DMG_DISSOLVE), GetAbsOrigin(), 256, CLASS_NONE, NULL);
			RadiusDamage(CTakeDamageInfo(this, this, 125 * 10, DMG_BLAST), GetAbsOrigin(), 256, CLASS_NONE, NULL);

			UTIL_Remove(this);
		}
	}
	else
	{
		trace_t	tr;
		ShouldDrawWaterImpacts(tr);
		Vector vForward;
		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, this, COLLISION_GROUP_NONE, &tr);

		RadiusDamage(CTakeDamageInfo(this, this, 125 * 10, DMG_DISSOLVE), GetAbsOrigin(), 256, CLASS_NONE, NULL);
		RadiusDamage(CTakeDamageInfo(this, this, 125 * 10, DMG_BLAST), GetAbsOrigin(), 256, CLASS_NONE, NULL);

		UTIL_Remove(this);
	}

	EmitSound("Weapon_Displacer.Explode");

	DispatchParticleEffect("houndeye_wave_01", GetAbsOrigin(), RandomAngle(0, 360));

	//Shockring
	CBroadcastRecipientFilter filter2;
	te->BeamRingPoint(filter2, 0, GetAbsOrigin(),	//origin
		64,			//start radius
		800,		//end radius
		m_nLightningSprite1, //texture
		0,			//halo index
		0,			//start frame
		2,			//framerate
		0.4f,		//life
		128,			//width
		0,			//spread
		0,			//amplitude
		255,	//r
		255,	//g
		225,	//b
		32,		//a
		0,		//speed
		FBEAM_FADEOUT
		);
	DispatchParticleEffect("Xen_portal_big1", GetAbsOrigin(), RandomAngle(0, 360));	//Новый партиклевый эффект дыма
}

//----------------------------------------------------------------------------------
// Purpose: Check for water
//----------------------------------------------------------------------------------
#define FSetBit(iBitVector, bits)	((iBitVector) |= (bits))
#define FBitSet(iBitVector, bit)	((iBitVector) & (bit))
#define TraceContents( vec ) ( enginetrace->GetPointContents( vec ) )
#define WaterContents( vec ) ( FBitSet( TraceContents( vec ), CONTENTS_WATER|CONTENTS_SLIME ) )
bool CGreenBallProjectile::ShouldDrawWaterImpacts(const trace_t &shot_trace)
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if (WaterContents(shot_trace.startpos))
		return false;

	// We must end inside of water
	if (!WaterContents(shot_trace.endpos))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(shot_trace.startpos, shot_trace.endpos, (CONTENTS_WATER | CONTENTS_SLIME), UTIL_GetLocalPlayer(), COLLISION_GROUP_NONE, &waterTrace);


	if (waterTrace.fraction < 1.0f)
	{
		CEffectData	data;

		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = random->RandomFloat(2.0, 4.0f);	// Water effect scale

		// See if we hit slime
		if (FBitSet(waterTrace.contents, CONTENTS_SLIME))
		{
			FSetBit(data.m_fFlags, FX_WATER_IN_SLIME);
		}

		CPASFilter filter(data.m_vOrigin);
		te->DispatchEffect(filter, 0.0, data.m_vOrigin, "watersplash", data);
	}
	return true;
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGreenBallProjectile::BubbleThink( void )
{
	QAngle angNewAngles;

	VectorAngles( GetAbsVelocity()/2.0f, angNewAngles );
	SetAbsAngles( angNewAngles );

	SetNextThink( gpGlobals->curtime + 0.1f );

	if ( GetWaterLevel() || GetAbsVelocity().Length() <= 0)
		UTIL_Remove(this);
}

#endif



IMPLEMENT_SERVERCLASS_ST(CWeapondisplacer, DT_Weapondisplacer)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_displacer, CWeapondisplacer );
PRECACHE_WEAPON_REGISTER( weapon_displacer );

BEGIN_DATADESC( CWeapondisplacer )
	DEFINE_KEYFIELD( m_iLandmark, FIELD_STRING, "landmark" ),
	DEFINE_FIELD(isFiring, FIELD_BOOLEAN),
END_DATADESC()

acttable_t CWeapondisplacer::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_PHYSGUN,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_PHYSGUN,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PHYSGUN,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PHYSGUN,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,        false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PHYSGUN,                    false },

	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },

	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,					false },
	{ ACT_WALK,						ACT_WALK_RIFLE,						true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,				true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
	{ ACT_RUN,						ACT_RUN_RIFLE,						true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },
};

IMPLEMENT_ACTTABLE(CWeapondisplacer);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapondisplacer::CWeapondisplacer( void )
{
	isFiring = false;
	m_bPrimary = false;
	m_bSecondary = false;
}

void CWeapondisplacer::Precache( void )
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapondisplacer::PrimaryAttack( void )
{

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if ( m_bShotDelayed )
		return;

	// Cannot fire underwater
	if ( GetOwner() && GetOwner()->GetWaterLevel() == 3 )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) < 20 )	// BriJee : Fix for 70 50 30 10 ammo count. Dont shoot if we have less than 20 ammo
	{
		WeaponSound( EMPTY );
		m_bPrimary = false;
		m_flNextPrimaryAttack = gpGlobals->curtime + 2.1f;	// BriJee : Loop primaryattack sound fix
		return;
	}

	m_bWeaponBlockWall = true;

	isFiring = true;
	m_bIsFiring = isFiring;
	SendWeaponAnim(ACT_GAUSS_SPINUP);	// spin anim start
	//pOwner->SetAnimation( PLAYER_ATTACK1 );

	WeaponSound( SPECIAL1 );
	//WeaponSound( SINGLE );

	m_bShotDelayed = true;
	m_flDelayedFire = gpGlobals->curtime + GetViewModelSequenceDuration(); //2.0f;

	m_bPrimary = true;
	m_BeamFireState = FIRE_STARTUP;
	m_flNextPrimaryAttack = gpGlobals->curtime + 3.5f; //GetViewModelSequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + 3.5f; //GetViewModelSequenceDuration();
	//Msg("Primary True \n");
	//DispatchParticleEffect("xelectrical_arc_01_parent", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);
	//DispatchParticleEffect("Disp_muzzle_in", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);
}

void CWeapondisplacer::SecondaryAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	if ( m_bShotDelayed )
		return;

	// Cannot fire underwater
	if ( GetOwner() && GetOwner()->GetWaterLevel() == 3 )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	//m_flAmmoRemoveDelay = -1;
	m_bWeaponBlockWall = true;
	isFiring = true;
	m_bIsFiring = isFiring;
	SendWeaponAnim(ACT_GAUSS_SPINUP);

	WeaponSound( SPECIAL2 );

	m_bShotDelayed = true;
	m_flDelayedFire = gpGlobals->curtime + GetViewModelSequenceDuration(); //2.5f;

	m_bSecondary = true;
	m_BeamFireState = FIRE_STARTUP;
	m_flNextPrimaryAttack = gpGlobals->curtime + 3.5f; //GetViewModelSequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + 3.5f; //GetViewModelSequenceDuration();

	SecondaryAttackEffects();
	//DispatchParticleEffect("Disp_muzzle_in", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);
	//Msg("Secondary True \n");
}

void CWeapondisplacer::ItemPostFrame( void )
{
	//if (!isFiring)
	if (m_flNextPrimaryAttack < gpGlobals->curtime && m_flNextSecondaryAttack < gpGlobals->curtime)
		m_bIsFiring = false;

	if ( gpGlobals->curtime < m_flDelayedFire )
	{
		if (isFiring && GetActivity() != ACT_GAUSS_SPINUP && GetActivity() != ACT_GAUSS_SPINCYCLE)
			SendWeaponAnim(ACT_GAUSS_SPINCYCLE);

		DelayedAttack();
	}
	else
	{
		if (m_bShotDelayed)
		{
			m_bShotDelayed = false;
			DelayedAttack();
		}
	}

	BaseClass::ItemPostFrame();
}

void CWeapondisplacer::DelayedAttack( void )
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	CEffectData	fireData;
	fireData.m_nEntIndex = pOwner->GetViewModel()->entindex();
	fireData.m_vOrigin = pOwner->GetViewModel()->GetLocalOrigin();
	fireData.m_vAngles = pOwner->GetViewModel()->GetLocalAngles();
	fireData.m_nAttachmentIndex = 21;
	DispatchEffect("FireEvent", fireData);


	//DispatchParticleEffect("Disp_muzzle_root", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", false);
	DispatchParticleEffect(STRING(GetWpnData().iMuzzleFlashDelayed), PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), GetWpnData().iAttachment, false);

	if (!m_bShotDelayed && m_bPrimary)
	{
		FireTheBall();
		m_bWeaponBlockWall = false;
		m_bPrimary = false;
		m_BeamFireState = FIRE_NONE;
		//m_bIsFiring = false;
	}

	if (!m_bShotDelayed && m_bSecondary)
	{

		m_bSecondary = false;

		if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) < 60 )		// BriJee: Less than 60 ammo? Disable teleportation
		{
			WeaponSound( EMPTY );
			m_bWeaponBlockWall = false;
			//EmitSound(denied)
			isFiring = false;
			m_bIsFiring = false;
			m_BeamFireState = FIRE_NONE;
			return;
		}

		//===============

		CBaseEntity	*pentTarget = NULL;
		// The activator and caller are the same
		pentTarget = gEntList.FindEntityByName( pentTarget, /*m_target*/ /*"tp_dark_room"*/ aa_wpn_displacer_target.GetString(), NULL, pOwner, pOwner );
		if (!pentTarget)
		{
			//ClientPrint( pOwner, HUD_PRINTCENTER, "No displacers found" );
			WeaponSound(EMPTY);
			m_bWeaponBlockWall = false;
			m_bSecondary = false;
			m_bIsFiring = false;
			m_BeamFireState = FIRE_NONE;
			return;
		}

		//ClientPrint( pOwner, HUD_PRINTCENTER, "displacer Found" );

		CBaseEntity	*pentLandmark = NULL;
		Vector vecLandmarkOffset(0, 0, 0);
		if (m_iLandmark != NULL_STRING)
		{
			// The activator and caller are the same
			pentLandmark = gEntList.FindEntityByName(pentLandmark, m_iLandmark, NULL, pOwner, pOwner );
			if (pentLandmark)
			{
				vecLandmarkOffset = pOwner->GetAbsOrigin() - pentLandmark->GetAbsOrigin();
			}
		}

		pOwner->SetGroundEntity( NULL );
	
		Vector tmp = pentTarget->GetAbsOrigin();

		if (!pentLandmark && pOwner->IsPlayer())
		{
			// make origin adjustments in case the teleport is a player. (origin in center, not at feet)
			tmp.z -= pOwner->WorldAlignMins().z;
		}

		//
		// Only modify the toucher's angles and zero their velocity if no landmark was specified.
		//
		const QAngle *pAngles = NULL;
		Vector *pVelocity = NULL;

		if (!pentLandmark && !HasSpawnFlags(0x20) )
		{
			pAngles = &pentTarget->GetAbsAngles();

			pVelocity = NULL;	//BUGBUG - This does not set the player's velocity to zero!!!
		}

		tmp += vecLandmarkOffset;
		pOwner->Teleport( &tmp, pAngles, pVelocity );

		WeaponSound( SPECIAL3 );

		//pOwner->RemoveAmmo( 60, m_iPrimaryAmmoType );
		RemoveAmmo(GetPrimaryAmmoType(), 60);

		color32 TeleEffect = { 0,255,0,255 };
		UTIL_ScreenFade( pOwner, TeleEffect, 0.2f, 0.1f, FFADE_IN );
		m_bSecondary = false;
		m_bIsFiring = false;
		m_bWeaponBlockWall = false;
		m_BeamFireState = FIRE_NONE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeapondisplacer::FireTheBall( void )
{
	if ( m_iPrimaryAmmoType <= 0 )
	{
		WeaponSound( EMPTY );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

#ifndef CLIENT_DLL

	Vector vecAiming = pOwner->GetAutoaimVector(0);
	Vector vecSrc = pOwner->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
	VectorScale(vecThrow, 1000.0f, vecThrow);
	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;
	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	vThrowPos = GetClientTracerVector();
	trace_t	tr;
	UTIL_TraceLine(pOwner->EyePosition(), vThrowPos, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

	/*vThrowPos = pOwner->Weapon_ShootPosition();
	if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1)
	{
		vThrowPos += vForward * 1.0f;
		vThrowPos += vRight * 0.05f;// *1.0f;
		vThrowPos += vUp * -4.4f;
		//DevMsg("ISight enabled \n");
	}
	else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() == 0)
	{
		vThrowPos += vForward * 1.0f;
		vThrowPos += vRight * 5.4f;
		vThrowPos += vUp * -4.4f;
		//DevMsg("ISight disabled \n");*/
	//}*/

#if defined(HL2_EPISODIC)
	// !!!HACK - the other piece of the Alyx crossbow bolt hack for Outland_10 (see ::BoltTouch() for more detail)
	if (FStrEq(STRING(gpGlobals->mapname), "ep2_outland_10"))
	{
		trace_t tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 24.0f, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

		if (tr.m_pEnt != NULL && tr.m_pEnt->Classify() == CLASS_PLAYER_ALLY_VITAL)
		{
			// If Alyx is right in front of the player, make sure the bolt starts outside of the player's BBOX, or the bolt
			// will instantly collide with the player after the owner of the bolt is switched to Alyx in ::BoltTouch(). We 
			// avoid this altogether by making it impossible for the bolt to collide with the player.
			vecSrc += vecAiming * 24.0f;
		}
	}
#endif
	//CShockRifleProjectile *pShockRifleProjectile = CShockRifleProjectile::BoltCreate( vecSrc, angAiming, GetSDKWpnData().m_iDamage, pOwner );
	CGreenBallProjectile *pGreenBallProjectile = CGreenBallProjectile::BoltCreate(tr.endpos, angAiming, 0, pOwner); // Light Kill : Test
	pGreenBallProjectile->SetAbsVelocity(tr.endpos);

	if ( pOwner->GetWaterLevel() == 3 )		// BriJee: HURT player if we shoot in water
	{
		//pShockRifleProjectile->Remove();	//->SetAbsVelocity( vecAiming * BOLT_WATER_VELOCITY );
		CTakeDamageInfo hitself(this, this, 210, DMG_SHOCK );
		pOwner->TakeDamage(hitself);
	}
	else
	{
		pGreenBallProjectile->SetAbsVelocity(vecAiming * BOLT_AIR_VELOCITY);
	}

#endif

	//m_iClip1--;
	//pOwner->RemoveAmmo( 20, m_iPrimaryAmmoType );
	RemoveAmmo(GetPrimaryAmmoType(), 20);

	float X = -2.f;
	if (abs(pPlayer->LocalEyeAngles().x) + abs(X) >= 89.0f)
	{
		X = X < 0 ? (89.0f - abs(pPlayer->LocalEyeAngles().x)) * -1.f : 89.0f - abs(pPlayer->LocalEyeAngles().x);
	}
	pOwner->ViewPunch( QAngle( X, 1, -7 ) );

	WeaponSound( SINGLE );
	//WeaponSound( SPECIAL2 );
	isFiring = false;

	SendWeaponAnim(GetPrimaryAttackActivity());
	pOwner->SetAnimation( PLAYER_ATTACK1 );

	/*if ( !m_iClip1 && pOwner->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}*/
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextSecondaryAttack = m_flNextPrimaryAttack;
}



