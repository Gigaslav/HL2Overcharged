//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: Heavily armored combine infantry
//
//=============================================================================

#include "cbase.h"
#include "npc_combines.h"
#include "weapon_physcannon.h"
#include "hl2_gamerules.h"

#include "prop_combine_ball.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_combine_armored_health( "sk_combine_armored_health","0");
ConVar	sk_combine_armored_kick( "sk_combine_armored_kick","0");

//-----------------------------------------------------------------------------
// Purpose: Heavily armored combine infantry
//-----------------------------------------------------------------------------
class CArmorPiece : public CBaseAnimating
{
	DECLARE_CLASS( CArmorPiece, CBaseAnimating );
public: 
	void Spawn( void )
	{
		BaseClass::Spawn();
		Precache();

		SetModel( STRING(GetModelName()) );

		CreateVPhysics();
	}

	void Precache( void )
	{
		PrecacheModel( STRING(GetModelName()) );
	}

	bool CreateVPhysics( void )
	{
		SetSolid( SOLID_VPHYSICS );
		IPhysicsObject *pPhysicsObject = VPhysicsInitShadow( false, false );

		if ( !pPhysicsObject )
		{
			SetSolid( SOLID_NONE );
			SetMoveType( MOVETYPE_NONE );
			Warning("ERROR!: Can't create physics object for %s\n", STRING( GetModelName() ) );
		}

		return true;
	}
};

LINK_ENTITY_TO_CLASS( combine_armor_piece, CArmorPiece );

//-----------------------------------------------------------------------------
// Purpose: Heavily armored combine infantry
//-----------------------------------------------------------------------------
class CNPC_Combine_Armored : public CNPC_CombineS
{
	DECLARE_CLASS( CNPC_Combine_Armored, CNPC_CombineS );
public: 
	void		Spawn( void );
	void		Precache( void );

	void		SpawnArmorPieces( void );

	void Event_Killed( const CTakeDamageInfo &info );	// L1ght 15 : Update

	int	OnTakeDamage_Alive( const CTakeDamageInfo &info );
	int	TakeDamageFromCombineBall( const CTakeDamageInfo &info );
};

LINK_ENTITY_TO_CLASS( npc_combine_armored, CNPC_Combine_Armored );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Combine_Armored::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );

	BaseClass::Spawn();		// L1ght 15 : Base class first, override later (health fix)

	SetHealth( sk_combine_armored_health.GetFloat() );
	SetMaxHealth( sk_combine_armored_health.GetFloat() );
	SetKickDamage( sk_combine_armored_kick.GetFloat() );

	CapabilitiesAdd( bits_CAP_ANIMATEDFACE );
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
	CapabilitiesAdd( bits_CAP_DOORS_GROUP );

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL ); //| EFL_NO_PHYSCANNON_INTERACTION );	// L1gh7 15 : missing addition

	//SpawnArmorPieces();	// L1ght 15 : Disabled for now
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_Combine_Armored::Precache()
{
	if( !GetModelName() )
	{
		SetModelName( MAKE_STRING( "models/armored_soldier.mdl" ) );
	}

	PrecacheModel( STRING( GetModelName() ) );

	//UTIL_PrecacheOther( "combine_armor_piece" );	// L1ght 15 : Disabled for now

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Combine_Armored::SpawnArmorPieces( void )
{
	struct armorpiecepositions_t
	{
		char	*pszAttachment;
		char	*pszModel;
	};

	armorpiecepositions_t ArmorPiecesPositions[] =
	{
		{ "attach_L_Thigh_armor",		"models/combine_armor.mdl" },
		{ "attach_L_UpperArm_armor",	"models/combine_armor.mdl" },
		{ "attach_R_Thigh_armor",		"models/combine_armor.mdl" },  
		{ "attach_R_UpperArm_armor",	"models/combine_armor.mdl" },
		{ "attach_chest_armor",			"models/combine_armor_chest.mdl" },
	};

	for ( int i = 0; i < ARRAYSIZE(ArmorPiecesPositions); i++ )
	{
		CArmorPiece *pArmor = (CArmorPiece *)CBaseEntity::CreateNoSpawn( "combine_armor_piece", GetAbsOrigin(), GetAbsAngles(), this );
		pArmor->SetModelName( MAKE_STRING(ArmorPiecesPositions[i].pszModel) );
		pArmor->SetParent( this, LookupAttachment(ArmorPiecesPositions[i].pszAttachment) );
		pArmor->SetLocalOrigin( vec3_origin );
		pArmor->SetLocalAngles( vec3_angle );
		DispatchSpawn( pArmor );
		pArmor->Activate();
	}
}

// L1ght 15 : Updates
void CNPC_Combine_Armored::Event_Killed( const CTakeDamageInfo &info )
{

	//SpawnArmorPieces();	//removearmorpieces?

	//CArmorPiece *pArmor = (CArmorPiece *)CBaseEntity::CreateNoSpawn( "combine_armor_piece", GetAbsOrigin(), GetAbsAngles(), this );

	//pArmor->Remove();//test

	/*CArmorPiece *pArmor = (CArmorPiece *)CBaseEntity::GetBaseEntity();

	if ( pArmor != NULL )
	{
		pArmor->
		//UTIL_Remove( pArmor	);
	}*/

	//pArmor->DestroyAllDataObjects();
	//BaseClass::Event_Killed( info );

	CNPC_Combine::Event_Killed( info );
}

int CNPC_Combine_Armored::TakeDamageFromCombineBall( const CTakeDamageInfo &info )
{
	float damage = info.GetDamage();

	// If it's only an AR2 alt-fire, we don't take much damage
	if ( UTIL_IsAR2CombineBall( info.GetInflictor() ) )
	{
		damage = 60;	// dmg here!!!
	}
	/*else
	{
		// Always start smoking when we're struck by a normal combine ball
		StartSmoking();
	}*/

	if( info.GetAttacker() && info.GetAttacker()->IsPlayer() )
	{
		// Route combine ball damage through the regular skill level code.
		damage = g_pGameRules->AdjustPlayerDamageInflicted(damage);
	}

	/*AddFacingTarget( info.GetInflictor(), info.GetInflictor()->GetAbsOrigin(), 0.5, 2.0 );
	if ( !UTIL_IsAR2CombineBall( info.GetInflictor() ) )
		RestartGesture( ACT_GESTURE_BIG_FLINCH );
	else
		RestartGesture( ACT_GESTURE_SMALL_FLINCH );*/
	
	//PainSound( info );

	m_iHealth -= damage;

	return damage;
}

int CNPC_Combine_Armored::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	// don't take damage from my own weapons!!!
	if ( info.GetInflictor() && info.GetInflictor()->GetOwnerEntity() == this )
		return 0;

	// special interaction with combine balls
	if ( UTIL_IsCombineBall( info.GetInflictor() ) )
		return TakeDamageFromCombineBall( info );
	else
		return BaseClass::OnTakeDamage_Alive( info );
}