//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Combine Ion-Rifle from episode 2 remake
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "soundent.h"
#include "shake.h"
#include "in_buttons.h"
#include "ndebugoverlay.h"
#include "particle_parse.h"
#include "AI_BaseNPC.h"
#include "NPCEvent.h"

#include "weapon_ionrifle.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_dmg_ionrifle("sk_dmg_ionrifle", "0"); //200 default

IMPLEMENT_SERVERCLASS_ST( CTE_IonBang, DT_TEIonExplosion )
	SendPropVector( SENDINFO(m_vecNormal), -1, SPROP_COORD ),
	SendPropFloat( SENDINFO(m_flScale), 0, SPROP_NOSCALE ),
	SendPropInt( SENDINFO(m_nRadius), 32, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_nMagnitude), 32, SPROP_UNSIGNED ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTE_IonBang::CTE_IonBang( const char *name ) : BaseClass( name )
{
	m_nRadius		= 0;
	m_nMagnitude	= 0;
	m_flScale		= 0.0f;

	m_vecNormal.Init();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTE_IonBang::~CTE_IonBang( void )
{
}


void TE_IonExplosion(IRecipientFilter& filter, float delay,
	const Vector* pos, float scale, int radius, int magnitude, const Vector* normal)
{
	g_TEIonExplosion.m_vecOrigin = *pos;
	g_TEIonExplosion.m_flScale = scale;
	g_TEIonExplosion.m_nRadius = radius;
	g_TEIonExplosion.m_nMagnitude = magnitude;

	if (normal)
		g_TEIonExplosion.m_vecNormal = *normal;
	else
		g_TEIonExplosion.m_vecNormal = Vector(0, 0, 1);

	// Send it over the wire
	g_TEIonExplosion.Create(filter, delay);
}


LINK_ENTITY_TO_CLASS( ionblast, C_IonBlast );

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( C_IonBlast )

//	DEFINE_FIELD( m_spriteTexture,	FIELD_INTEGER ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Create a Ion blast entity and detonate it
//-----------------------------------------------------------------------------
void CreateIonBlast( const Vector &origin, const Vector &surfaceNormal, CBaseEntity *pOwner, float magnitude )
{
	QAngle angles;
	VectorAngles( surfaceNormal, angles );
	C_IonBlast *pBlast = (C_IonBlast *) CBaseEntity::Create( "ionblast", origin, angles, pOwner );
	//DispatchParticleEffect( "hunter_projectile_explosion_1", origin, surfaceNormal, angles, pOwner); //test
	DispatchParticleEffect( "hunter_projectile_explosion_1", origin, angles, NULL);

	if ( pBlast )
	{
		pBlast->Explode( magnitude );
	}
}
void C_IonBlast::Explode(float magnitude)
{
	//Create a Ion explosion
	CPASFilter filter(GetAbsOrigin());

	Vector vecForward;
	AngleVectors(GetAbsAngles(), &vecForward);
	TE_IonExplosion(filter, 0.0,
		&GetAbsOrigin(),//position
		1.0f,	//scale
		256 * magnitude,	//radius
		175 * magnitude,	//magnitude
		&vecForward);	//normal

	int	colorRamp = random->RandomInt(128, 255);

	//Shockring
	CBroadcastRecipientFilter filter2;
	te->BeamRingPoint(filter2, 0,
		GetAbsOrigin(),	//origin
		16,			//start radius
		300 * magnitude,		//end radius
		m_spriteTexture, //texture
		0,			//halo index
		0,			//start frame
		2,			//framerate
		0.3f,		//life
		128,		//width
		16,			//spread
		0,			//amplitude
		colorRamp,	//r
		colorRamp,	//g
		255,		//g
		24,			//a
		128			//speed
		);

	//Do the radius damage
	RadiusDamage(CTakeDamageInfo(this, GetOwnerEntity(), sk_dmg_ionrifle.GetFloat(), DMG_BLAST/*|DMG_DISSOLVE*/), GetAbsOrigin(), 128, CLASS_PLAYER_ALLY, NULL);

	UTIL_Remove(this);
}

//#define	ION_BEAM_SPRITE		"effects/laser1_noz.vmt"
//#define	ION_LASER_SPRITE	"effects/black2.vmt"//"sprites/redglow1.vmt"
// Light Kill : Declare weapon functions here...



IMPLEMENT_SERVERCLASS_ST(CWeaponIonRifle, DT_WeaponIonRifle)
//SendPropInt(SENDINFO(EnableLaser), 1, SPROP_UNSIGNED),
END_SEND_TABLE()



LINK_ENTITY_TO_CLASS( weapon_ionrifle, CWeaponIonRifle );
PRECACHE_WEAPON_REGISTER( weapon_ionrifle );


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CWeaponIonRifle )
//DEFINE_FIELD(EnableLaser, FIELD_BOOLEAN),
//DEFINE_FIELD(m_hLaserMuzzleSprite, FIELD_EHANDLE),
//DEFINE_FIELD(m_hLaserBeam, FIELD_EHANDLE),
//DEFINE_FIELD(pressed, FIELD_BOOLEAN),
	//DEFINE_FIELD( m_flChargeTime,	FIELD_TIME ),
	//DEFINE_FIELD( m_bFired,			FIELD_BOOLEAN ),
//	DEFINE_FIELD( m_beamIndex,		FIELD_INTEGER ),
//	DEFINE_FIELD( m_haloIndex,		FIELD_INTEGER ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Maps base activities to weapons-specific ones so our characters do the right things.
//-----------------------------------------------------------------------------
acttable_t CWeaponIonRifle::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_SHOTGUN,                    false },
    { ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_SHOTGUN,                    false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_SHOTGUN,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_SHOTGUN,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,        false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_SHOTGUN,                    false },
    //{ ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SHOTGUN,                false },		// END

	//{	ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SNIPER_RIFLE, true }

	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },	// FIXME: hook to shotgun unique

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
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,			false },
};

IMPLEMENT_ACTTABLE( CWeaponIonRifle );




class CLaserPoint : public CWeaponIonRifle
{
	DECLARE_CLASS(CLaserPoint, CWeaponIonRifle);
	//Class_T Classify(void) { return CLASS_NONE; }
public:
	CLaserPoint() { };
	~CLaserPoint() { };
	void Precache(void);
	void Spawn(void);
	static CLaserPoint		*DispatchImpactSound(const Vector &end);

	/*protected:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();*/
};
LINK_ENTITY_TO_CLASS(end_point, CLaserPoint);


CLaserPoint *CLaserPoint::DispatchImpactSound(const Vector &end)
{
	CLaserPoint *pPoint = (CLaserPoint *)(CreateEntityByName("end_point"));
	UTIL_SetOrigin(pPoint, end);
	pPoint->SetAbsAngles(QAngle(0, 0, 0));
	pPoint->Spawn();
	pPoint->SetOwnerEntity(pPoint);
	return pPoint;
}
void CLaserPoint::Precache(void)
{
	PrecacheScriptSound("Weapon_RPG.LaserOn");
	PrecacheScriptSound("Weapon_RPG.LaserOff");
	PrecacheScriptSound("Weapon_Gluon.Hit");
}
void CLaserPoint::Spawn(void)
{
	Precache();

	SetMoveType(MOVETYPE_NONE);
	UTIL_SetSize(this, -Vector(10, 10, 10), Vector(10, 10, 10));
	SetSolid(SOLID_NONE);

	//trace_t tr;
	//DispatchParticleEffect("laser_pointer", GetAbsOrigin(), tr.endpos, GetAbsAngles(), this);

	EmitSound("Weapon_Gluon.Hit");

	//UTIL_Remove(this);

}
//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponIonRifle::CWeaponIonRifle( void )
{
	m_flNextPrimaryAttack	= gpGlobals->curtime;

	DoOnce = true;
	EnableLaser = false;

}

CWeaponIonRifle::~CWeaponIonRifle()
{

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponIonRifle::Precache( void )
{
	UTIL_PrecacheOther( "ionblast" );

	m_beamIndex = PrecacheModel( "sprites/bluelaser1.vmt" );
	m_haloIndex = PrecacheModel( "sprites/blueshaft1.vmt" );

	//PrecacheParticleSystem( "Weapon_Combine_Ion_Cannon_Backup" );
	//PrecacheParticleSystem( "electrical_arc_01_parent" );

	//PrecacheParticleSystem( "strider_headbeating_03" );
	PrecacheParticleSystem( "larvae_glow_extract_b" );
	PrecacheParticleSystem("ion_laser");
	PrecacheParticleSystem("ion_laser_dot");
	PrecacheParticleSystem("ion_laser_dot2");
	PrecacheParticleSystem( "Weapon_Combine_Ion_Cannon" );
	//PrecacheParticleSystem( "choreo_skyflower_01" );


	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponIonRifle::AlertTargets( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DispatchParticleEffect( "larvae_glow_extract_b", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", true); //blah

	// Fire the bullets
	Vector vecSrc	 = pPlayer->Weapon_ShootPosition( );
	//Vector vecAiming = pPlayer->GetRadialAutoVector( NEW_AUTOAIM_RADIUS, NEW_AUTOAIM_DIST ); //blah
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES );

	Vector	impactPoint	= vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	trace_t	tr;

	UTIL_TraceLine( vecSrc, impactPoint, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );
	
	if ( (vecSrc-tr.endpos).Length() > 1024 )
		return;

	CSoundEnt::InsertSound( SOUND_DANGER, tr.endpos, 128, 0.5f );
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponIonRifle::UpdateLasers( void )
{
	//Only update the lasers whilst charging
	if ( ( m_flChargeTime < gpGlobals->curtime ) || ( m_bFired ) )
		return;

	Vector	start, end, v_forward, v_right, v_up;

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	pPlayer->GetVectors( &v_forward, &v_right, &v_up );

	//Get the position of the laser
	start = pPlayer->Weapon_ShootPosition( );

	start += ( v_forward * 8.0f ) + ( v_right * 3.0f ) + ( v_up * -2.0f );

	end = start + ( v_forward * MAX_TRACE_LENGTH );

	float	angleOffset = ( 1.0f - ( m_flChargeTime - gpGlobals->curtime ) ) / 1.0f;
	Vector	offset[4];

	offset[0] = Vector( 0.0f,  0.5f, -0.5f );
	offset[1] = Vector( 0.0f,  0.5f,  0.5f );
	offset[2] = Vector( 0.0f, -0.5f, -0.5f );
	offset[3] = Vector( 0.0f, -0.5f,  0.5f );

	QAngle  v_ang;
	Vector	v_dir;

	angleOffset *= 2.0f;

	if ( angleOffset > 1.0f )
		angleOffset = 1.0f;

	for ( int i = 0; i < 4; i++ )
	{
		Vector	ofs = start + ( v_forward * offset[i][0] ) + ( v_right * offset[i][1] ) + ( v_up * offset[i][2] );

		float hScale = ( offset[i][1] <= 0.0f ) ? 1.0f : -1.0f;
		float vScale = ( offset[i][2] <= 0.0f ) ? 1.0f : -1.0f;

		VectorAngles( v_forward, v_ang );
		v_ang[PITCH] = UTIL_AngleMod( v_ang[PITCH] + ( (1.0f-angleOffset) * 15.0f * vScale ) );
		v_ang[YAW] = UTIL_AngleMod( v_ang[YAW] + ( (1.0f-angleOffset) * 15.0f * hScale ) );

		AngleVectors( v_ang, &v_dir );

		trace_t	tr;
		UTIL_TraceLine( ofs, ofs + ( v_dir * MAX_TRACE_LENGTH ), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

		UTIL_Beam( ofs, tr.endpos, m_beamIndex, 0, 0, 2.0f, 0.1f, 2, 0, 1, 0, 255, 255, 255, 32, 100 );
		
		UTIL_Beam( ofs, tr.endpos, m_haloIndex, 0, 0, 2.0f, 0.1f, 4, 0, 1, 16, 255, 255, 255, 8, 100 );
	}
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponIonRifle::PrimaryAttack( void )
{
	if ( m_flChargeTime >= gpGlobals->curtime )
		return;
		
	AlertTargets();

	WeaponSound( SPECIAL1 );

	//UTIL_ScreenShake( GetAbsOrigin(), 10.0f, 100.0f, 2.0f, 128, SHAKE_START, false );

	m_flChargeTime	= gpGlobals->curtime + 1.0f;
	m_bFired		= false;
}*/

void CWeaponIonRifle::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch ( pEvent->event )
	{
		case EVENT_WEAPON_SHOTGUN_FIRE:
		{
			Vector vecSrc, vecAiming,vecShootOrigin, vecTarget;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin ); //use attachment as start position

			GetVectors( &vecAiming, NULL, NULL );
			vecSrc = WorldSpaceCenter() + vecAiming * 64;
	
			Vector	impactPoint	= vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

			//===========

			//CAI_BaseNPC *npc = pOperator->MyNPCPointer();
			//ASSERT( npc != NULL );
			//vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			//vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			//AngleVectors( angShootDir, &vecShootDir );

			// Light Kill : Yeah, i am on right way now...
			CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
			vecTarget = pNPC->GetEnemy()->BodyTarget( vecSrc );

			//===========

			CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

			WeaponSound( SINGLE_NPC );
			pOperator->DoMuzzleFlash();
			m_iClip1 = m_iClip1 - 1;

			//===========

			trace_t	tr;
			Vector vecShootPos;
			//AI_TraceLine( vecSrc, impactPoint, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

			AI_TraceLine( vecShootOrigin, vecTarget, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

			//UTIL_Tracer( tr.startpos, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 6000 + random->RandomFloat( 0, 2000 ), true, "GunshipTracer" );
			//UTIL_Tracer( tr.startpos, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 6000 + random->RandomFloat( 0, 3000 ), true, "GunshipTracer" );
			//UTIL_Tracer( tr.startpos, tr.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 6000 + random->RandomFloat( 0, 4000 ), true, "GunshipTracer" );

			//DispatchParticleEffect( "Weapon_Combine_Ion_Cannon", vecSrc, tr.endpos, vec3_angle, NULL );//addition
			DispatchParticleEffect( "Weapon_Combine_Ion_Cannon", vecShootOrigin, tr.endpos, vec3_angle, NULL /*pOperator->GetEnemy()*/);

			CreateIonBlast( tr.endpos, tr.plane.normal, this, 1.0 );

			break;
		}
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}


/*bool CWeaponIonRifle::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	EnableLaser = false;
	StopParticleEffects(this);
	StopLaserEffects();
	if (m_hLaserMuzzleSprite)
	{
		UTIL_Remove(m_hLaserMuzzleSprite);
	}

	if (m_hLaserBeam)
	{
		UTIL_Remove(m_hLaserBeam);
	}

	return BaseClass::Holster(pSwitchingTo);
}*/

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponIonRifle::PrimaryAttack( void )
{
	//if ( m_flChargeTime >= gpGlobals->curtime )
		//return;

	//if ( m_bFired )
		//return;

	//m_bFired = true;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;
	
	if (IsNearWall() || GetOwnerIsRunning())
	{
		return;
	}

	//pPlayer->AddEffects(EF_M);

	// Abort here to handle burst and auto fire modes
	if ( (GetMaxClip1() != -1 && m_iClip1 == 0) || (GetMaxClip1() == -1 && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType) ) )
		return;

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);
	pPlayer->SetAnimation( PLAYER_ATTACK1 );				// Light Kill : Forgotten anim ?
	pPlayer->DoMuzzleFlash();

	//SendWeaponAnim( ACT_VM_PRIMARYATTACK ); // Light Kill : Overwrite shoot event.
	SendWeaponAnim(GetPrimaryAttackActivity());
	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	/*if ( GetSequence() != SelectWeightedSequence( ACT_VM_PRIMARYATTACK ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime;
	}*/
	
	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if ( UsesClipsForAmmo1() )
	{
		m_iClip1 = m_iClip1 - 1;
	}

	//PrepareHitmarker();

	// Fire the bullets
	Vector vecSrc	 = pPlayer->Weapon_ShootPosition( );
	//Vector vecAiming = pPlayer->GetRadialAutoVector( NEW_AUTOAIM_RADIUS, NEW_AUTOAIM_DIST ); // blah
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES );

	Vector	impactPoint	= vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	trace_t	tr;
	UTIL_TraceHull( vecSrc, impactPoint, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

	//=========

	Vector	vFirePos, vForward, vRight, vUp;

	pPlayer->GetVectors( &vForward, &vRight, &vUp );

	vFirePos = pPlayer->Weapon_ShootPosition( );

	// BriJee: Disabled for now
	//if (cvar->FindVar("aa_cammode")->GetInt() == 0)	// L1ght 15 : Viewmodel muzzle vectors
	//{
	vFirePos += ( vForward * 8.0f ) + ( vRight * 6.0f ) + ( vUp * -2.1f );
	/*}
	else	// Third-person worldmodel position
	{
		vFirePos += ( vForward * 8.0f ) + ( vRight * 10.0f ) + ( vUp * -17.0f );
	}*/

	//DispatchParticleEffect( "Weapon_Combine_Ion_Cannon", PATTACH_POINT_FOLLOW, pPlayer->GetViewModel(), "muzzle", true); //blah

	Vector Muzzle;
	Muzzle = GetClientMuzzleVector();
	//Muzzle.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();//pViewModel->tracerOrig_X;
	//Muzzle.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();//pViewModel->tracerOrig_Y;
	//Muzzle.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();//pViewModel->tracerOrig_Z;

	DispatchParticleEffect("Weapon_Combine_Ion_Cannon", Muzzle /*vFirePos*/, tr.endpos, vec3_angle, NULL);
	ShouldDrawWaterImpacts(tr);
	CreateIonBlast( tr.endpos, tr.plane.normal, this, 1.0 );
	UTIL_DecalTrace(&tr, "FadingScorch");

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate(); // Light Kill : Delay for avoid some mistakes.

	AddViewKick();
}
//----------------------------------------------------------------------------------
// Purpose: Check for water
//----------------------------------------------------------------------------------
#define FSetBit(iBitVector, bits)	((iBitVector) |= (bits))
#define FBitSet(iBitVector, bit)	((iBitVector) & (bit))
#define TraceContents( vec ) ( enginetrace->GetPointContents( vec ) )
#define WaterContents( vec ) ( FBitSet( TraceContents( vec ), CONTENTS_WATER|CONTENTS_SLIME ) )

bool CWeaponIonRifle::ShouldDrawWaterImpacts(const trace_t &shot_trace)
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
// Light Kill : addition from smg1
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CWeaponIonRifle::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSound ( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponIonRifle::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}*/

bool CWeaponIonRifle::Reload(void)
{
	//DevMsg("In Reload \n");
	//StopLaserEffects();
	return BaseClass::Reload();
}
#if 0
void CWeaponIonRifle::StartLaserEffects(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	//EnableLaser = true;

	if (m_hLaserMuzzleSprite == NULL)
	{
		m_hLaserMuzzleSprite = CSprite::SpriteCreate(ION_LASER_SPRITE, GetAbsOrigin(), false);

		if (m_hLaserMuzzleSprite == NULL)
		{
			// We were unable to create the sprite
			Assert(0);
			return;
		}

		m_hLaserMuzzleSprite->SetAttachment(pOwner->GetViewModel(), LookupAttachment("laser"));
		m_hLaserMuzzleSprite->SetTransparency(kRenderTransAdd, 255, 255, 180, 255, kRenderFxNoDissipation);
		m_hLaserMuzzleSprite->SetBrightness(255, 0.5f);
		m_hLaserMuzzleSprite->SetScale(0.15f, 0.2f);
		m_hLaserMuzzleSprite->TurnOn();
		//m_hLaserMuzzleSprite->SetAsTemporary();
		//m_hLaserMuzzleSprite->Expand(0, 3000);
		if (!(pOwner->m_nButtons & IN_FIREMODE))
			EmitSound("Weapon_RPG.LaserOn");
	}

	CEffectData	data;

	data.m_nEntIndex = pOwner->GetViewModel()->entindex();
	data.m_nAttachmentIndex = 2;
	DispatchEffect("IonMuzzleLight", data);

#if 0
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	//CBaseViewModel *pBeamEnt = static_cast<CBaseViewModel *>(pOwner->GetViewModel());
	//CBaseViewModel *pBeamEnt = pOwner->GetViewModel();
	//if (m_hLaserBeam == NULL)
	{
		//m_hLaserBeam = CBeam::BeamCreate(ION_BEAM_SPRITE, 1.0f);

		/*if (m_hLaserBeam == NULL)
		{
			// We were unable to create the beam
			Assert(0);
			return;
		}*/
		
		//DispatchParticleEffect("ion_laser", PATTACH_POINT_FOLLOW, pBeamEnt);

		//m_hLaserBeam->EntsInit(pBeamEnt, pBeamEnt);

		int	startAttachment = LookupAttachment("muzzle");
		
//		int endAttachment = LookupAttachment("laser_end");
		Vector Muzzle;
		Muzzle = GetClientMuzzleVector();


		Vector vecAiming = pOwner->GetAutoaimVector(AUTOAIM_2DEGREES);
		Vector	impactPoint = Muzzle + (vecAiming * MAX_TRACE_LENGTH);

		trace_t	tr;
		UTIL_TraceHull(Muzzle, impactPoint, Vector(-2, -2, -2), Vector(2, 2, 2), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

		QAngle End;
		/*End.x = cvar->FindVar("oc_muzzle_angle_x")->GetFloat();
		End.y = cvar->FindVar("oc_muzzle_angle_y")->GetFloat();
		End.z = cvar->FindVar("oc_muzzle_angle_z")->GetFloat();*/
		VectorAngles(Muzzle, End);
		int iEntIndex = pOwner->GetViewModel()->entindex();
		
		//if (!StartEffects)
		{
			UTIL_ParticleTracer("ion_laser", Muzzle, tr.endpos, iEntIndex, startAttachment, 1);//OverCharged
			UTIL_ParticleTracer("ion_laser_dot", Muzzle, tr.endpos, iEntIndex, startAttachment, 1);//OverCharged
			StartEffects = true;
		}


		/*DispatchParticleEffect("laser_pointer", GetAbsOrigin(), tr.endpos, End, pOwner->GetViewModel());
		DispatchParticleEffect("ion_laser", GetAbsOrigin(), tr.endpos, End, pOwner->GetViewModel());
		DispatchParticleEffect("ion_laser_dot", Muzzle, GetAbsAngles(), pOwner->GetViewModel());*/
	    //DispatchParticleEffect("ion_laser", Muzzle, End, NULL);
		//if (DoOnce)
		/*{		
			DispatchParticleEffect("ion_laser_dot2", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), startAttachment, false);
			//DoOnce = false;
		}*/
		//DispatchParticleEffect("ion_laser", Muzzle /*vFirePos*/, tr.endpos, vec3_angle, pBeamEnt);

		//this->FollowEntity

		//m_hLaserBeam->SetStartPos()
		//m_hLaserBeam->FollowEntity(pBeamEnt);
		/*m_hLaserBeam->SetStartPos(Muzzle);
		m_hLaserBeam->RelinkBeam();
		m_hLaserBeam->PointEntInit(tr.endpos, this);
		//m_hLaserBeam->SetEndPos(tr.endpos);
		//m_hLaserBeam->SetStartAttachment(startAttachment);
		//m_hLaserBeam->SetAbsEndPos(tr.endpos);
		//m_hLaserBeam->SetEndAttachment(endAttachment);
		m_hLaserBeam->SetNoise(0);
		m_hLaserBeam->SetColor(255, 0, 0);
		m_hLaserBeam->SetScrollRate(0);
		m_hLaserBeam->SetWidth(0.5f);
		m_hLaserBeam->SetEndWidth(0.5f);
		m_hLaserBeam->SetBrightness(128);
		m_hLaserBeam->SetBeamFlags(SF_BEAM_SHADEIN);*/
	}
/*	else
	{
		m_hLaserBeam->SetBrightness(128);
	}*/

	if (m_hLaserMuzzleSprite == NULL)
	{
		m_hLaserMuzzleSprite = CSprite::SpriteCreate(ION_LASER_SPRITE, GetAbsOrigin(), false);

		if (m_hLaserMuzzleSprite == NULL)
		{
			// We were unable to create the sprite
			Assert(0);
			return;
		}
		
		m_hLaserMuzzleSprite->SetAttachment(pOwner->GetViewModel(), LookupAttachment("muzzle"));
		m_hLaserMuzzleSprite->SetTransparency(kRenderTransAdd, 255, 255, 180, 255, kRenderFxNoDissipation);
		m_hLaserMuzzleSprite->SetBrightness(255, 0.5f);
		m_hLaserMuzzleSprite->SetScale(0.25f, 0.5f);
		m_hLaserMuzzleSprite->TurnOn();
	}
	/*else
	{
		m_hLaserMuzzleSprite->TurnOn();
		m_hLaserMuzzleSprite->SetScale(0.25f, 0.25f);
		m_hLaserMuzzleSprite->SetBrightness(255);
	}*/
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Stop the effects on the viewmodel of the Deagle
//-----------------------------------------------------------------------------
void CWeaponIonRifle::StopLaserEffects(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	//EnableLaser = false;
	/*if (m_hLaserBeam != NULL)
	{
		m_hLaserBeam->SetBrightness(0);

	}*/

	if (m_hLaserMuzzleSprite != NULL)
	{
		m_hLaserMuzzleSprite->SetScale(0.0f);
		m_hLaserMuzzleSprite->SetScale(0.001f);
		m_hLaserMuzzleSprite->SetBrightness(0, 0.5f);
		//m_hLaserMuzzleSprite->Expand(0, 3000);
		m_hLaserMuzzleSprite = NULL;
		if (m_hLaserMuzzleSprite != NULL)
			UTIL_Remove(m_hLaserMuzzleSprite);
		EmitSound("Weapon_RPG.LaserOff");
	}
#if 0
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	DispatchParticleEffect("ion_laser_stop", PATTACH_POINT_FOLLOW, pOwner->GetViewModel(), "muzzle", true);
	StartEffects = false;
	StopParticleEffects(pOwner->GetViewModel());
	StopParticleEffects(this);
	StopParticleEffects(pOwner);
	//DoOnce = true;

	if (m_hLaserBeam != NULL)
	{
		m_hLaserBeam->SetBrightness(0);
	}

	if (m_hLaserMuzzleSprite != NULL)
	{
		m_hLaserMuzzleSprite->SetScale(0.001f);
		m_hLaserMuzzleSprite->SetBrightness(0, 0.5f);
		m_hLaserMuzzleSprite->Expand(0, 3000);
		m_hLaserMuzzleSprite = NULL;
	}
#endif
}
#endif