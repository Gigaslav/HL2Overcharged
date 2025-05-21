//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The Half-Life 2 game rules, such as the relationship tables and ammo
//			damage cvars.
//
//=============================================================================

#include "cbase.h"
#include "hl2_gamerules.h"
#include "ammodef.h"
#include "hl2_shareddefs.h"

#ifdef CLIENT_DLL

#else
	#include "player.h"
	#include "game.h"
	#include "gamerules.h"
	#include "teamplay_gamerules.h"
	#include "hl2_player.h"
	#include "voice_gamemgr.h"
	#include "globalstate.h"
	#include "ai_basenpc.h"
	#include "weapon_physcannon.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


REGISTER_GAMERULES_CLASS( CHalfLife2 );

BEGIN_NETWORK_TABLE_NOBASE( CHalfLife2, DT_HL2GameRules )
	#ifdef CLIENT_DLL
		RecvPropBool( RECVINFO( m_bMegaPhysgun ) ),
	#else
		SendPropBool( SENDINFO( m_bMegaPhysgun ) ),
	#endif
END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( hl2_gamerules, CHalfLife2Proxy );
IMPLEMENT_NETWORKCLASS_ALIASED( HalfLife2Proxy, DT_HalfLife2Proxy )


#ifdef CLIENT_DLL
	void RecvProxy_HL2GameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CHalfLife2 *pRules = HL2GameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CHalfLife2Proxy, DT_HalfLife2Proxy )
		RecvPropDataTable( "hl2_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_HL2GameRules ), RecvProxy_HL2GameRules )
	END_RECV_TABLE()
#else
	void* SendProxy_HL2GameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
		CHalfLife2 *pRules = HL2GameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CHalfLife2Proxy, DT_HalfLife2Proxy )
		SendPropDataTable( "hl2_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_HL2GameRules ), SendProxy_HL2GameRules )
	END_SEND_TABLE()
#endif

ConVar  physcannon_mega_enabled("physcannon_mega_enabled", "0", FCVAR_CHEAT | FCVAR_REPLICATED );

// Controls the application of the robus radius damage model.
ConVar	sv_robust_explosions( "sv_robust_explosions","1", FCVAR_REPLICATED );

// Damage scale for damage inflicted by the player on each skill level.
ConVar	sk_dmg_inflict_scale1( "sk_dmg_inflict_scale1", "1.50", FCVAR_REPLICATED );
ConVar	sk_dmg_inflict_scale2( "sk_dmg_inflict_scale2", "1.00", FCVAR_REPLICATED );
ConVar	sk_dmg_inflict_scale3( "sk_dmg_inflict_scale3", "0.75", FCVAR_REPLICATED );

// Damage scale for damage taken by the player on each skill level.
ConVar	sk_dmg_take_scale1( "sk_dmg_take_scale1", "0.50", FCVAR_REPLICATED );
ConVar	sk_dmg_take_scale2( "sk_dmg_take_scale2", "1.00", FCVAR_REPLICATED );
#ifdef HL2_EPISODIC
	ConVar	sk_dmg_take_scale3( "sk_dmg_take_scale3", "2.0", FCVAR_REPLICATED );
#else
	ConVar	sk_dmg_take_scale3( "sk_dmg_take_scale3", "1.50", FCVAR_REPLICATED );
#endif//HL2_EPISODIC

ConVar	sk_allow_autoaim( "sk_allow_autoaim", "1", FCVAR_REPLICATED | FCVAR_ARCHIVE_XBOX );

// Autoaim scale
ConVar	sk_autoaim_scale1( "sk_autoaim_scale1", "1.0", FCVAR_REPLICATED );
ConVar	sk_autoaim_scale2( "sk_autoaim_scale2", "1.0", FCVAR_REPLICATED );
//ConVar	sk_autoaim_scale3( "sk_autoaim_scale3", "0.0", FCVAR_REPLICATED ); NOT CURRENTLY OFFERED ON SKILL 3

// Quantity scale for ammo received by the player.
ConVar	sk_ammo_qty_scale1 ( "sk_ammo_qty_scale1", "1.20", FCVAR_REPLICATED );
ConVar	sk_ammo_qty_scale2 ( "sk_ammo_qty_scale2", "1.00", FCVAR_REPLICATED );
ConVar	sk_ammo_qty_scale3 ( "sk_ammo_qty_scale3", "0.60", FCVAR_REPLICATED );

ConVar	sk_plr_health_drop_time		( "sk_plr_health_drop_time", "30", FCVAR_REPLICATED );
ConVar	sk_plr_grenade_drop_time	( "sk_plr_grenade_drop_time", "30", FCVAR_REPLICATED );

ConVar	sk_plr_dmg_ar2			( "sk_plr_dmg_ar2","0", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_ar2			( "sk_npc_dmg_ar2","0", FCVAR_REPLICATED);
ConVar	sk_max_ar2				( "sk_max_ar2","0", FCVAR_REPLICATED);
ConVar	sk_max_ar2_altfire		( "sk_max_ar2_altfire","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_alyxgun		( "sk_plr_dmg_alyxgun","0", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_alyxgun		( "sk_npc_dmg_alyxgun","0", FCVAR_REPLICATED);
ConVar	sk_max_alyxgun			( "sk_max_alyxgun","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_pistol		( "sk_plr_dmg_pistol","0", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_pistol		( "sk_npc_dmg_pistol","0", FCVAR_REPLICATED);
ConVar	sk_max_pistol			( "sk_max_pistol","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_smg1			( "sk_plr_dmg_smg1","0", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_smg1			( "sk_npc_dmg_smg1","0", FCVAR_REPLICATED);
ConVar	sk_max_smg1				( "sk_max_smg1","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_buckshot			( "sk_plr_dmg_buckshot","0", FCVAR_REPLICATED);	
ConVar	sk_npc_dmg_buckshot			( "sk_npc_dmg_buckshot","0", FCVAR_REPLICATED);
ConVar	sk_max_buckshot				( "sk_max_buckshot","0", FCVAR_REPLICATED);
ConVar	sk_plr_num_shotgun_pellets	( "sk_plr_num_shotgun_pellets","7", FCVAR_REPLICATED);
ConVar	sk_plr_num_shotgun_pellets2	("sk_plr_num_shotgun_pellets2", "14", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_rpg_round	( "sk_plr_dmg_rpg_round","0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_rpg_round	( "sk_npc_dmg_rpg_round","0", FCVAR_REPLICATED);
ConVar	sk_max_rpg_round		( "sk_max_rpg_round","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_sniper_round	( "sk_plr_dmg_sniper_round","0", FCVAR_REPLICATED);	
ConVar	sk_npc_dmg_sniper_round	( "sk_npc_dmg_sniper_round","0", FCVAR_REPLICATED);
ConVar	sk_max_sniper_round		( "sk_max_sniper_round","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_molotov		( "sk_plr_dmg_molotov","0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_molotov		( "sk_npc_dmg_molotov","0", FCVAR_REPLICATED);
ConVar	sk_max_molotov			( "sk_max_molotov","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_grenade		( "sk_plr_dmg_grenade","0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_grenade		( "sk_npc_dmg_grenade","0", FCVAR_REPLICATED);
ConVar	sk_max_grenade			( "sk_max_grenade","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_smg1_grenade	( "sk_plr_dmg_smg1_grenade","0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_smg1_grenade	( "sk_npc_dmg_smg1_grenade","0", FCVAR_REPLICATED);
ConVar	sk_max_smg1_grenade		( "sk_max_smg1_grenade","0", FCVAR_REPLICATED );

ConVar	sk_plr_dmg_357			( "sk_plr_dmg_357", "0", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_357			( "sk_npc_dmg_357", "0", FCVAR_REPLICATED );
ConVar	sk_max_357				( "sk_max_357", "0", FCVAR_REPLICATED );

ConVar	sk_plr_dmg_crossbow		( "sk_plr_dmg_crossbow", "0", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_crossbow		( "sk_npc_dmg_crossbow", "0", FCVAR_REPLICATED );
ConVar	sk_max_crossbow			( "sk_max_crossbow", "0", FCVAR_REPLICATED );

ConVar	sk_dmg_sniper_penetrate_plr( "sk_dmg_sniper_penetrate_plr","0", FCVAR_REPLICATED);
ConVar	sk_dmg_sniper_penetrate_npc( "sk_dmg_sniper_penetrate_npc","0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_airboat		( "sk_plr_dmg_airboat", "0", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_airboat		( "sk_npc_dmg_airboat", "0", FCVAR_REPLICATED );

ConVar	sk_max_gauss_round		( "sk_max_gauss_round", "0", FCVAR_REPLICATED );

// Gunship & Dropship cannons
ConVar	sk_npc_dmg_gunship			( "sk_npc_dmg_gunship", "0", FCVAR_REPLICATED );
ConVar	sk_npc_dmg_gunship_to_plr	( "sk_npc_dmg_gunship_to_plr", "0", FCVAR_REPLICATED );


//OverCharged
ConVar	sk_plr_dmg_m249					("sk_plr_dmg_m249", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_m249					("sk_npc_dmg_m249", "0", FCVAR_REPLICATED);
ConVar	sk_max_m249						("sk_max_m249", "0", FCVAR_REPLICATED);

// FIXME: remove these
ConVar	sk_plr_dmg_flare_round			("sk_plr_dmg_flare_round", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_flare_round			("sk_npc_dmg_flare_round", "0", FCVAR_REPLICATED);
ConVar	sk_max_flare_round				("sk_max_flare_round", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_combine_sniper_round ("sk_plr_dmg_combine_sniper_round", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_combine_sniper_round ("sk_npc_dmg_combine_sniper_round", "0", FCVAR_REPLICATED);
ConVar	sk_max_combine_sniper_round		("sk_max_combine_sniper_round", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_ar3					("sk_plr_dmg_ar3", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_ar3					("sk_npc_dmg_ar3", "0", FCVAR_REPLICATED);
ConVar	sk_max_ar3						("sk_max_ar3", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_abgun				("sk_plr_dmg_abgun", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_abgun				("sk_npc_dmg_abgun", "0", FCVAR_REPLICATED);
ConVar	sk_max_abgun					("sk_max_abgun", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_particlerifle			("sk_plr_dmg_particlerifle", "0", FCVAR_REPLICATED);
ConVar sk_npc_dmg_particlerifle			("sk_npc_dmg_particlerifle", "0", FCVAR_REPLICATED);
ConVar sk_max_particlerifle				("sk_max_particlerifle", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_uranium				("sk_plr_dmg_uranium", "0", FCVAR_REPLICATED);
ConVar sk_npc_dmg_uranium				("sk_npc_dmg_uranium", "0", FCVAR_REPLICATED);
ConVar sk_max_uranium					("sk_max_uranium", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_Ak47					("sk_plr_dmg_Ak47", "0", FCVAR_REPLICATED);
ConVar sk_npc_dmg_Ak47					("sk_npc_dmg_Ak47", "0", FCVAR_REPLICATED);
ConVar sk_max_Ak47						("sk_max_Ak47", "0", FCVAR_REPLICATED);

ConVar sk_plr_dmg_Laser					("sk_plr_dmg_Laser", "0", FCVAR_REPLICATED);
ConVar sk_npc_dmg_Laser					("sk_npc_dmg_Laser", "0", FCVAR_REPLICATED);
ConVar sk_max_Laser						("sk_max_Laser", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_oicw					("sk_plr_dmg_oicw", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_oicw					("sk_npc_dmg_oicw", "0", FCVAR_REPLICATED);
ConVar	sk_max_oicw						("sk_max_oicw", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_oicw_grenade			("sk_plr_dmg_oicw_grenade", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_oicw_grenade			("sk_npc_dmg_oicw_grenade", "0", FCVAR_REPLICATED);
ConVar	sk_max_oicw_grenade				("sk_max_oicw_grenade", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_flareround			("sk_plr_dmg_flareround", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_flareround			("sk_npc_dmg_flareround", "0", FCVAR_REPLICATED);
ConVar	sk_max_flareround				("sk_max_flareround", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_laserpistol			("sk_plr_dmg_laserpistol", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_laserpistol			("sk_npc_dmg_laserpistol", "0", FCVAR_REPLICATED);
ConVar	sk_max_laserpistol				("sk_max_laserpistol", "0", FCVAR_REPLICATED);

ConVar	sk_max_tripwire					("sk_max_tripwire", "3", FCVAR_REPLICATED);
//#ifdef HL2_EPISODIC
ConVar	sk_max_hopwire					("sk_max_hopwire", "3", FCVAR_REPLICATED);
ConVar	sk_max_striderbuster			("sk_max_striderbuster", "3", FCVAR_REPLICATED);
//#endif

ConVar	sk_max_slam						("sk_max_slam", "0", FCVAR_REPLICATED);
ConVar	sk_max_hopmine					("sk_max_hopmine", "0", FCVAR_REPLICATED);

ConVar  sk_spore_acid_radius			("sk_spore_acid_radius", "250", FCVAR_REPLICATED);
ConVar  sk_spore_acid_lifetime			("sk_spore_acid_lifetime", "350", FCVAR_REPLICATED);
ConVar  sk_spore_acid_maxdrips			("sk_spore_acid_maxdrips", "17", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_spore_acid			("sk_plr_dmg_spore_acid", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_spore_acid			("sk_npc_dmg_spore_acid", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_spore_acid_bounce	("sk_plr_dmg_spore_acid_bounce", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_spore_acid_bounce	("sk_npc_dmg_spore_acid_bounce", "0", FCVAR_REPLICATED);

ConVar	sk_max_spore_acid				("sk_max_spore_acid", "0", FCVAR_REPLICATED);
ConVar	sk_max_spore_acid_bounce		("sk_max_spore_acid_bounce", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_hornet("sk_plr_dmg_hornet", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_hornet("sk_npc_dmg_hornet", "0", FCVAR_REPLICATED);
ConVar	sk_max_hornet("sk_max_hornet", "0", FCVAR_REPLICATED);

ConVar	sk_plr_dmg_shock("sk_plr_dmg_shock", "0", FCVAR_REPLICATED);
ConVar	sk_npc_dmg_shock("sk_npc_dmg_shock", "0", FCVAR_REPLICATED);
ConVar	sk_max_shock("sk_max_shock", "0", FCVAR_REPLICATED);

//ConVar	sk_plr_dmg_molotov("sk_plr_dmg_molotov", "0", FCVAR_REPLICATED);
//ConVar	sk_npc_dmg_molotov("sk_npc_dmg_molotov", "0", FCVAR_REPLICATED);
//ConVar	sk_max_molotov("sk_max_molotov", "0", FCVAR_REPLICATED);

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
int CHalfLife2::Damage_GetTimeBased( void )
{
#ifdef HL2_EPISODIC
	int iDamage = ( DMG_PARALYZE | DMG_NERVEGAS | DMG_POISON | DMG_RADIATION | DMG_DROWNRECOVER | DMG_ACID | DMG_SLOWBURN );
	return iDamage;
#else
	return BaseClass::Damage_GetTimeBased();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iDmgType - 
// Output :		bool
//-----------------------------------------------------------------------------
bool CHalfLife2::Damage_IsTimeBased( int iDmgType )
{
	// Damage types that are time-based.
#ifdef HL2_EPISODIC
	// This makes me think EP2 should have its own rules, but they are #ifdef all over in here.
	return ((iDmgType & (DMG_PARALYZE | DMG_NERVEGAS | DMG_POISON | DMG_RADIATION | DMG_DROWNRECOVER | DMG_ACID | DMG_SLOWBURN)) != 0);//OverCharged //ADD DMG_ACID
#else
	return BaseClass::Damage_IsTimeBased( iDmgType );
#endif
}

#ifdef CLIENT_DLL
#else

#ifdef HL2_EPISODIC
ConVar  alyx_darkness_force( "alyx_darkness_force", "0", FCVAR_CHEAT | FCVAR_REPLICATED );
#endif // HL2_EPISODIC

#endif // CLIENT_DLL


#ifdef CLIENT_DLL //{


#else //}{

	extern bool		g_fGameOver;

#if !(defined( HL2MP ) || defined( PORTAL_MP ))
	class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
	{
	public:
		virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker, bool &bProximity )
		{
			return true;
		}
	};
	CVoiceGameMgrHelper g_VoiceGameMgrHelper;
	IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;
#endif
	
	//-----------------------------------------------------------------------------
	// Purpose:
	// Input  :
	// Output :
	//-----------------------------------------------------------------------------
	CHalfLife2::CHalfLife2()
	{
		m_bMegaPhysgun = false;
		
		m_flLastHealthDropTime = 0.0f;
		m_flLastGrenadeDropTime = 0.0f;
	}

	//-----------------------------------------------------------------------------
	// Purpose: called each time a player uses a "cmd" command
	// Input  : *pEdict - the player who issued the command
	//			Use engine.Cmd_Argv,  engine.Cmd_Argv, and engine.Cmd_Argc to get 
	//			pointers the character string command.
	//-----------------------------------------------------------------------------
	bool CHalfLife2::ClientCommand( CBaseEntity *pEdict, const CCommand &args )
	{
		if( BaseClass::ClientCommand( pEdict, args ) )
			return true;

		CHL2_Player *pPlayer = (CHL2_Player *) pEdict;

		if ( pPlayer->ClientCommand( args ) )
			return true;

		return false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Player has just spawned. Equip them.
	//-----------------------------------------------------------------------------
	void CHalfLife2::PlayerSpawn( CBasePlayer *pPlayer )
	{
	}

	//-----------------------------------------------------------------------------
	// Purpose: MULTIPLAYER BODY QUE HANDLING
	//-----------------------------------------------------------------------------
	class CCorpse : public CBaseAnimating
	{
	public:
		DECLARE_CLASS( CCorpse, CBaseAnimating );
		DECLARE_SERVERCLASS();

		virtual int ObjectCaps( void ) { return FCAP_DONT_SAVE; }	

	public:
		CNetworkVar( int, m_nReferencePlayer );
	};

	IMPLEMENT_SERVERCLASS_ST(CCorpse, DT_Corpse)
		SendPropInt( SENDINFO(m_nReferencePlayer), 10, SPROP_UNSIGNED )
	END_SEND_TABLE()

	LINK_ENTITY_TO_CLASS( bodyque, CCorpse );


	CCorpse		*g_pBodyQueueHead;

	void InitBodyQue(void)
	{
		CCorpse *pEntity = ( CCorpse * )CreateEntityByName( "bodyque" );
		pEntity->AddEFlags( EFL_KEEP_ON_RECREATE_ENTITIES );
		g_pBodyQueueHead = pEntity;
		CCorpse *p = g_pBodyQueueHead;
		
		// Reserve 3 more slots for dead bodies
		for ( int i = 0; i < 3; i++ )
		{
			CCorpse *next = ( CCorpse * )CreateEntityByName( "bodyque" );
			next->AddEFlags( EFL_KEEP_ON_RECREATE_ENTITIES );
			p->SetOwnerEntity( next );
			p = next;
		}
		
		p->SetOwnerEntity( g_pBodyQueueHead );
	}

	//-----------------------------------------------------------------------------
	// Purpose: make a body que entry for the given ent so the ent can be respawned elsewhere
	// GLOBALS ASSUMED SET:  g_eoBodyQueueHead
	//-----------------------------------------------------------------------------
	void CopyToBodyQue( CBaseAnimating *pCorpse ) 
	{
		if ( pCorpse->IsEffectActive( EF_NODRAW ) )
			return;

		CCorpse *pHead	= g_pBodyQueueHead;

		pHead->CopyAnimationDataFrom( pCorpse );

		pHead->SetMoveType( MOVETYPE_FLYGRAVITY );
		pHead->SetAbsVelocity( pCorpse->GetAbsVelocity() );
		pHead->ClearFlags();
		pHead->m_nReferencePlayer	= ENTINDEX( pCorpse );

		pHead->SetLocalAngles( pCorpse->GetAbsAngles() );
		UTIL_SetOrigin(pHead, pCorpse->GetAbsOrigin());

		UTIL_SetSize(pHead, pCorpse->WorldAlignMins(), pCorpse->WorldAlignMaxs());
		g_pBodyQueueHead = (CCorpse *)pHead->GetOwnerEntity();
	}

	//------------------------------------------------------------------------------
	// Purpose : Initialize all default class relationships
	// Input   :
	// Output  :
	//------------------------------------------------------------------------------
	void CHalfLife2::InitDefaultAIRelationships( void )
	{
		int i, j;

		//  Allocate memory for default relationships
		CBaseCombatCharacter::AllocateDefaultRelationships();

		// --------------------------------------------------------------
		// First initialize table so we can report missing relationships
		// --------------------------------------------------------------
		for (i=0;i<NUM_AI_CLASSES;i++)
		{
			for (j=0;j<NUM_AI_CLASSES;j++)
			{
				// By default all relationships are neutral of priority zero
				CBaseCombatCharacter::SetDefaultRelationship( (Class_T)i, (Class_T)j, D_NU, 0 );
			}
		}


		// ------------------------------------------------------------
		//	> CLASS_COMBINEGUARD
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_BARNACLE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_BULLSEYE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_BULLSQUID, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_CITIZEN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_CITIZEN_REBEL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_COMBINE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_COMBINE_GUNSHIP, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_COMBINE_HUNTER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_CONSCRIPT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_FLARE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_HEADCRAB, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_HOUNDEYE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_MANHACK, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_METROPOLICE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_MISSILE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_SCANNER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_STALKER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_VORTIGAUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_PROTOSNIPER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_EARTH_FAUNA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_HACKED_ROLLERMINE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_COMBINEGUARD, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_ALIENGRUNT, D_HT, 0);		// Overcharged: added
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINEGUARD, CLASS_SNARK, D_NU, 0);


		// ------------------------------------------------------------
		//	> CLASS_HYDRA
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_BARNACLE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_BULLSEYE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_BULLSQUID, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_CITIZEN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_CITIZEN_REBEL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_COMBINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_COMBINE_GUNSHIP, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_COMBINE_HUNTER, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_CONSCRIPT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_FLARE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_HEADCRAB, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_HOUNDEYE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_MANHACK, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_METROPOLICE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_MISSILE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_SCANNER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_STALKER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_VORTIGAUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_PROTOSNIPER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_EARTH_FAUNA, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_HACKED_ROLLERMINE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_CREMATOR, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_ALIENGRUNT, D_HT, 0);		// Overcharged: added
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_RACEX, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HYDRA, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_CREMATOR
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_BARNACLE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_BULLSEYE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_BULLSQUID, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_CITIZEN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_CITIZEN_REBEL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_COMBINE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_COMBINE_GUNSHIP, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_COMBINE_HUNTER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_CONSCRIPT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_FLARE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_HEADCRAB, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_HOUNDEYE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_MANHACK, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_METROPOLICE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_MISSILE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_SCANNER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_STALKER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_VORTIGAUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_PROTOSNIPER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_EARTH_FAUNA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_HACKED_ROLLERMINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_ALIENGRUNT, D_HT, 0);		// Overcharged: added
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_ALIENCONTROLLER, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_RACEX, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CREMATOR, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_ANTLION
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PROTOSNIPER,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_ANTLION,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION, CLASS_ALIENGRUNT, D_HT, 0);		// Overcharged: added
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION, CLASS_ALIENCONTROLLER, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION, CLASS_COMBINEGUARD, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ANTLION, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_BARNACLE
		//
		//  In this case, the relationship D_HT indicates which characters
		//  the barnacle will try to eat.
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BARNACLE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MANHACK,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_EARTH_FAUNA,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE, CLASS_ALIENGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE, CLASS_ALIENCONTROLLER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BARNACLE, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_BULLSEYE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_ANTLION,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE,			CLASS_HACKED_ROLLERMINE,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE, CLASS_ALIENGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE, CLASS_ALIENCONTROLLER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE, CLASS_HGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE, CLASS_RACEX, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSEYE, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_BULLSQUID
		// ------------------------------------------------------------
		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HEADCRAB,			D_HT, 1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HOUNDEYE,			D_HT, 1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MANHACK,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ALIENGRUNT,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_ALIENCONTROLLER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID,			CLASS_HGRUNT,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BULLSQUID, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_CITIZEN_PASSIVE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_BULLSQUID,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_COMBINE_HUNTER,	D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HEADCRAB,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HOUNDEYE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MANHACK,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_MISSILE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_VORTIGAUNT,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_ZOMBIE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE,	CLASS_HACKED_ROLLERMINE,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE, CLASS_ALIENGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE, CLASS_ALIENCONTROLLER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE, CLASS_HGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE, CLASS_RACEX, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_PASSIVE, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_CITIZEN_REBEL
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_BULLSQUID,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_MISSILE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL,		CLASS_HACKED_ROLLERMINE,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL, CLASS_ALIENGRUNT, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL, CLASS_ALIENCONTROLLER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CITIZEN_REBEL, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_COMBINE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE_GUNSHIP,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_COMBINE_HUNTER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE, CLASS_CREMATOR, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_COMBINE_GUNSHIP
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE_GUNSHIP,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_COMBINE_HUNTER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_MISSILE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP,		CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP, CLASS_CREMATOR, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_GUNSHIP, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_COMBINE_HUNTER
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,	CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE_GUNSHIP,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_COMBINE_HUNTER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,	CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER,		CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER, CLASS_HYDRA, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER, CLASS_CREMATOR, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_COMBINE_HUNTER, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_CONSCRIPT
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT,			CLASS_HACKED_ROLLERMINE,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT, CLASS_CREMATOR, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_CONSCRIPT, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_FLARE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_ANTLION,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE,			CLASS_HACKED_ROLLERMINE,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE, CLASS_ALIENGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE, CLASS_ALIENCONTROLLER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE, CLASS_HGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE, CLASS_RACEX, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_FLARE, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_HEADCRAB
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_BULLSQUID,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB,			CLASS_HACKED_ROLLERMINE,D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HEADCRAB, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_HOUNDEYE
		// ------------------------------------------------------------
		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_BULLSQUID,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE, CLASS_ALIENGRUNT, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE, CLASS_ALIENCONTROLLER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HOUNDEYE, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_MANHACK
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HEADCRAB,			D_HT,-1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HOUNDEYE,			D_HT,-1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MANHACK, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_METROPOLICE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE,		CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_METROPOLICE, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_MILITARY
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MILITARY, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_MISSILE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE, CLASS_HGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE, CLASS_COMBINEGUARD, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_MISSILE, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_NONE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_ANTLION,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BULLSEYE,			D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER_ALLY,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_PLAYER_ALLY_VITAL,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE,				CLASS_HACKED_ROLLERMINE,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_ALIENGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_ALIENCONTROLLER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_HGRUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_RACEX, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_NONE, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_PLAYER
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER,			D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BARNACLE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BULLSEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CITIZEN_PASSIVE,	D_LI, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CITIZEN_REBEL,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE_GUNSHIP,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PROTOSNIPER,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER,			CLASS_HACKED_ROLLERMINE,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_BEE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_BEE_PLR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_COMBINEGUARD, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_PLAYER_ALLY
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER,			D_LI, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BARNACLE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HEADCRAB,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_ZOMBIE,			D_FR, 1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PROTOSNIPER,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY,			CLASS_HACKED_ROLLERMINE,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_PLAYER_ALLY_VITAL
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER,			D_LI, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BARNACLE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_COMBINE_HUNTER,	D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PROTOSNIPER,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL,	CLASS_HACKED_ROLLERMINE,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PLAYER_ALLY_VITAL, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_SCANNER
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE_GUNSHIP,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_COMBINE_HUNTER,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MANHACK,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_METROPOLICE,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MILITARY,			D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_SCANNER,			D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_STALKER,			D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PROTOSNIPER,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER, CLASS_HYDRA, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER, CLASS_RACEX, D_FR, 0);	//fear
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SCANNER, CLASS_SNARK, D_NU, 0);

		// ------------------------------------------------------------
		//	> CLASS_STALKER
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_STALKER, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_VORTIGAUNT
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER,			D_LI, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BARNACLE,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CITIZEN_PASSIVE,	D_LI, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CITIZEN_REBEL,	D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_VORTIGAUNT,		D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT,		CLASS_HACKED_ROLLERMINE,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT, CLASS_HYDRA, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT, CLASS_ALIENGRUNT, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT, CLASS_ALIENCONTROLLER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_VORTIGAUNT, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_ZOMBIE
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HEADCRAB,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MANHACK,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MILITARY,			D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_ZOMBIE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE, CLASS_CREMATOR, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ZOMBIE, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_PROTOSNIPER
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_BULLSQUID,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HOUNDEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_METROPOLICE,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MILITARY,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_MISSILE,			D_NU, 5);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_STALKER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER, CLASS_COMBINEGUARD, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_PROTOSNIPER, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_EARTH_FAUNA
		//
		// Hates pretty much everything equally except other earth fauna.
		// This will make the critter choose the nearest thing as its enemy.
		// ------------------------------------------------------------	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_NONE,				D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE_GUNSHIP,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_FLARE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_MISSILE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_SCANNER,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_STALKER,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_VORTIGAUNT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PROTOSNIPER,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA,			CLASS_HACKED_ROLLERMINE,D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA, CLASS_CREMATOR, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_EARTH_FAUNA, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_HACKED_ROLLERMINE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER,			D_LI, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BARNACLE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_PASSIVE,	D_NU, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CITIZEN_REBEL,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_COMBINE_HUNTER,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_CONSCRIPT,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MANHACK,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_SCANNER,			D_NU, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_VORTIGAUNT,		D_LI, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_ZOMBIE,			D_HT, 1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_EARTH_FAUNA,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY,		D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_PLAYER_ALLY_VITAL,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE,			CLASS_HACKED_ROLLERMINE,D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE, CLASS_HYDRA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE, CLASS_CREMATOR, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE, CLASS_ALIENGRUNT, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE, CLASS_ALIENCONTROLLER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE, CLASS_COMBINEGUARD, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HACKED_ROLLERMINE, CLASS_SNARK, D_HT, 0);

		// BriJee: Overcharged ADDED
		// ------------------------------------------------------------
		//	> CLASS_ALIENGRUNT
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_BARNACLE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_BULLSEYE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_BULLSQUID, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_CITIZEN_PASSIVE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_CITIZEN_REBEL, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_COMBINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_COMBINE_GUNSHIP, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_COMBINE_HUNTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_CONSCRIPT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_FLARE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_HEADCRAB, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_HOUNDEYE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_MANHACK, D_HT, -1);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_METROPOLICE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_MISSILE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_SCANNER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_STALKER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_VORTIGAUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_PROTOSNIPER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_EARTH_FAUNA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_HACKED_ROLLERMINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_ALIENGRUNT, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_ALIENCONTROLLER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_COMBINEGUARD, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENGRUNT, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_ALIENCONTROLLER
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_BARNACLE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_BULLSEYE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_BULLSQUID, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_CITIZEN_PASSIVE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_CITIZEN_REBEL, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_COMBINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_COMBINE_HUNTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_COMBINE_GUNSHIP, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_CONSCRIPT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_FLARE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_HEADCRAB, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_HOUNDEYE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_MANHACK, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_METROPOLICE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_MISSILE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_SCANNER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_STALKER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_VORTIGAUNT, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_PROTOSNIPER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_EARTH_FAUNA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_HACKED_ROLLERMINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_ALIENGRUNT, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_ALIENCONTROLLER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_HYDRA, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_COMBINEGUARD, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_ALIENCONTROLLER, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_BEE
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_BEE_PLR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_BEE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_COMBINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_HEADCRAB, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_MANHACK, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_METROPOLICE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_COMBINE_HUNTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_SCANNER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_STALKER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_ALIENGRUNT, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_ALIENCONTROLLER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_CITIZEN_PASSIVE, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_CITIZEN_REBEL, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_HYDRA, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_COMBINEGUARD, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_BEE_PLR
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_BEE_PLR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_BEE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_PLAYER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_COMBINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_HEADCRAB, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_MANHACK, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_METROPOLICE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_COMBINE_HUNTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_SCANNER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_STALKER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_ALIENCONTROLLER, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_CITIZEN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_CITIZEN_REBEL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_HYDRA, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_COMBINEGUARD, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_BEE_PLR, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_HGRUNT
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_BARNACLE, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_BULLSEYE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_BULLSQUID, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_CITIZEN_PASSIVE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_CITIZEN_REBEL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_HGRUNT, D_LI, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_COMBINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_COMBINE_GUNSHIP, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_COMBINE_HUNTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_CONSCRIPT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_FLARE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_HEADCRAB, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_HOUNDEYE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_MANHACK, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_METROPOLICE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_MILITARY, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_MISSILE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_SCANNER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_STALKER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_VORTIGAUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_PROTOSNIPER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_EARTH_FAUNA, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_HACKED_ROLLERMINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_CREMATOR, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_HGRUNT, CLASS_SNARK, D_FR, 0);

		// ------------------------------------------------------------
		//	> CLASS_RACEX
		// ------------------------------------------------------------

		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_NONE,				D_NU, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_PLAYER,			D_HT, 0);			
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_ANTLION,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_BARNACLE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_BULLSEYE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_BULLSQUID,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_CITIZEN_PASSIVE,	D_HT, 0);	
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_CITIZEN_REBEL,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_COMBINE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_COMBINE_GUNSHIP,	D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_COMBINE_HUNTER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_CONSCRIPT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_FLARE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_HEADCRAB,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_HOUNDEYE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_MANHACK,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_METROPOLICE,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_MILITARY,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_MISSILE,			D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_SCANNER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_STALKER,			D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_VORTIGAUNT,		D_HT, 0);		
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_ZOMBIE,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_PROTOSNIPER,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_EARTH_FAUNA,		D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_PLAYER_ALLY,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_PLAYER_ALLY_VITAL,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_HACKED_ROLLERMINE,D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_CREMATOR,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_ALIENGRUNT,		D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_ALIENCONTROLLER,	D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_HGRUNT,			D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX,			CLASS_COMBINEGUARD,		D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_RACEX, CLASS_SNARK, D_HT, 0);

		// ------------------------------------------------------------
		//	> CLASS_SNARK
		// ------------------------------------------------------------
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_NONE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_PLAYER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_ANTLION, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_BARNACLE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_BULLSEYE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_BULLSQUID, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_CITIZEN_PASSIVE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_CITIZEN_REBEL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_COMBINE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_COMBINE_GUNSHIP, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_COMBINE_HUNTER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_CONSCRIPT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_FLARE, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_HEADCRAB, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_HOUNDEYE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_MANHACK, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_METROPOLICE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_MILITARY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_MISSILE, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_SCANNER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_STALKER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_VORTIGAUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_ZOMBIE, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_PROTOSNIPER, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_EARTH_FAUNA, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_PLAYER_ALLY, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_PLAYER_ALLY_VITAL, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_HACKED_ROLLERMINE, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_CREMATOR, D_NU, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_COMBINEGUARD, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_ALIENGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_ALIENCONTROLLER, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_HGRUNT, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_RACEX, D_HT, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_HYDRA, D_FR, 0);
		CBaseCombatCharacter::SetDefaultRelationship(CLASS_SNARK, CLASS_SNARK, D_NU, 0);
	}


	//------------------------------------------------------------------------------
	// Purpose : Return classify text for classify type
	// Input   :
	// Output  :
	//------------------------------------------------------------------------------
	const char* CHalfLife2::AIClassText(int classType)
	{
		switch (classType)
		{
			case CLASS_NONE:			return "CLASS_NONE";
			case CLASS_PLAYER:			return "CLASS_PLAYER";
			case CLASS_ANTLION:			return "CLASS_ANTLION";
			case CLASS_BARNACLE:		return "CLASS_BARNACLE";
			case CLASS_BULLSEYE:		return "CLASS_BULLSEYE";
			case CLASS_BULLSQUID:		return "CLASS_BULLSQUID";	
			case CLASS_CITIZEN_PASSIVE: return "CLASS_CITIZEN_PASSIVE";		
			case CLASS_CITIZEN_REBEL:	return "CLASS_CITIZEN_REBEL";
			case CLASS_COMBINE:			return "CLASS_COMBINE";
			case CLASS_COMBINE_GUNSHIP:	return "CLASS_COMBINE_GUNSHIP";
			case CLASS_COMBINE_HUNTER:	return "CLASS_COMBINE_HUNTER";
			case CLASS_CONSCRIPT:		return "CLASS_CONSCRIPT";
			case CLASS_HEADCRAB:		return "CLASS_HEADCRAB";
			case CLASS_SNARK:			return "CLASS_SNARK";
			case CLASS_HOUNDEYE:		return "CLASS_HOUNDEYE";
			case CLASS_MANHACK:			return "CLASS_MANHACK";
			case CLASS_METROPOLICE:		return "CLASS_METROPOLICE";
			case CLASS_MILITARY:		return "CLASS_MILITARY";	
			case CLASS_SCANNER:			return "CLASS_SCANNER";		
			case CLASS_STALKER:			return "CLASS_STALKER";		
			case CLASS_VORTIGAUNT:		return "CLASS_VORTIGAUNT";
			case CLASS_ZOMBIE:			return "CLASS_ZOMBIE";
			case CLASS_PROTOSNIPER:		return "CLASS_PROTOSNIPER";
			case CLASS_MISSILE:			return "CLASS_MISSILE";
			case CLASS_FLARE:			return "CLASS_FLARE";
			case CLASS_EARTH_FAUNA:		return "CLASS_EARTH_FAUNA";
			case CLASS_HYDRA:			return "CLASS_HYDRA";
			case CLASS_ALIENGRUNT:		return "CLASS_ALIENGRUNT";			// BriJee: Overcharged added
			case CLASS_ALIENCONTROLLER: return "CLASS_ALIENCONTROLLER";
			case CLASS_BEE:				return "CLASS_BEE";
			case CLASS_BEE_PLR:			return "CLASS_BEE_PLR";
			case CLASS_HGRUNT:			return "CLASS_HGRUNT";
			case CLASS_RACEX:			return "CLASS_RACEX";	//race x
			case CLASS_COMBINEGUARD:	return "CLASS_COMBINEGUARD";

			default:					return "MISSING CLASS in ClassifyText()";
		}
	}

	void CHalfLife2::PlayerThink( CBasePlayer *pPlayer )
	{
	}

	void CHalfLife2::Think( void )
	{
		BaseClass::Think();

		if( physcannon_mega_enabled.GetBool() == true )
		{
			m_bMegaPhysgun = true;
		}
		else
		{
			// FIXME: Is there a better place for this?
			m_bMegaPhysgun = ( GlobalEntity_GetState("super_phys_gun") == GLOBAL_ON );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Returns how much damage the given ammo type should do to the victim
	//			when fired by the attacker.
	// Input  : pAttacker - Dude what shot the gun.
	//			pVictim - Dude what done got shot.
	//			nAmmoType - What been shot out.
	// Output : How much hurt to put on dude what done got shot (pVictim).
	//-----------------------------------------------------------------------------
	float CHalfLife2::GetAmmoDamage( CBaseEntity *pAttacker, CBaseEntity *pVictim, int nAmmoType )
	{
		float flDamage = 0.0f;
		CAmmoDef *pAmmoDef = GetAmmoDef();

		if ( pAmmoDef->DamageType( nAmmoType ) & DMG_SNIPER )
		{
			// If this damage is from a SNIPER, we do damage based on what the bullet
			// HITS, not who fired it. All other bullets have their damage values
			// arranged according to the owner of the bullet, not the recipient.
			if ( pVictim->IsPlayer() )
			{
				// Player
				flDamage = pAmmoDef->PlrDamage( nAmmoType );
			}
			else
			{
				// NPC or breakable
				flDamage = pAmmoDef->NPCDamage( nAmmoType );
			}
		}
		else
		{
			flDamage = BaseClass::GetAmmoDamage( pAttacker, pVictim, nAmmoType );
		}

		if( pAttacker->IsPlayer() && pVictim->IsNPC() )
		{
			if( pVictim->MyCombatCharacterPointer() )
			{
				// Player is shooting an NPC. Adjust the damage! This protects breakables
				// and other 'non-living' entities from being easier/harder to break
				// in different skill levels.
				flDamage = pAmmoDef->PlrDamage( nAmmoType );
				flDamage = AdjustPlayerDamageInflicted( flDamage );
			}
		}

		return flDamage;
	}

	float CHalfLife2::GetAmmoDamageBullet(CBaseEntity *pAttacker, bool IsPlayer, int nAmmoType, CBaseEntity *pVictim)
	{
		float flDamage = 0.0f;
		CAmmoDef *pAmmoDef = GetAmmoDef();

		if (pAmmoDef->DamageType(nAmmoType) & DMG_SNIPER)
		{
			// If this damage is from a SNIPER, we do damage based on what the bullet
			// HITS, not who fired it. All other bullets have their damage values
			// arranged according to the owner of the bullet, not the recipient.
			if (IsPlayer)
			{
				// Player
				flDamage = pAmmoDef->PlrDamage(nAmmoType);
			}
			else
			{
				// NPC or breakable
				flDamage = pAmmoDef->NPCDamage(nAmmoType);
			}
		}
		else
		{
			flDamage = BaseClass::GetAmmoDamageBullet(pAttacker, IsPlayer, nAmmoType, pVictim);
		}

		/*if (pAttacker == NULL || IsPlayer == NULL)
		{
			if (pVictim->MyCombatCharacterPointer())
			{
				// Player is shooting an NPC. Adjust the damage! This protects breakables
				// and other 'non-living' entities from being easier/harder to break
				// in different skill levels.
				flDamage = pAmmoDef->PlrDamage(nAmmoType);
				flDamage = AdjustPlayerDamageInflicted(flDamage);
			}
			return flDamage;
		}*/


		if (pAttacker && pAttacker->IsPlayer() && !IsPlayer)
		{
			if (pVictim->MyCombatCharacterPointer())
			{
				// Player is shooting an NPC. Adjust the damage! This protects breakables
				// and other 'non-living' entities from being easier/harder to break
				// in different skill levels.
				flDamage = pAmmoDef->PlrDamage(nAmmoType);
				flDamage = AdjustPlayerDamageInflicted(flDamage);
			}
		}

		return flDamage;
	}
   	//-----------------------------------------------------------------------------
  	//-----------------------------------------------------------------------------
 	bool CHalfLife2::AllowDamage( CBaseEntity *pVictim, const CTakeDamageInfo &info )
  	{
#ifndef CLIENT_DLL
	if( (info.GetDamageType() & DMG_CRUSH) && info.GetInflictor() && pVictim->MyNPCPointer() )
	{
		if( pVictim->MyNPCPointer()->IsPlayerAlly() )
		{
			// A physics object has struck a player ally. Don't allow damage if it
			// came from the player's physcannon. 
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

			if( pPlayer )
			{
				CBaseEntity *pWeapon = pPlayer->HasNamedPlayerItem("weapon_physcannon");

				if( pWeapon )
				{
					CBaseCombatWeapon *pCannon = assert_cast <CBaseCombatWeapon*>(pWeapon);

					if( pCannon )
					{
						if( PhysCannonAccountableForObject(pCannon, info.GetInflictor() ) )
						{
							// Antlions can always be squashed!
							if ( pVictim->Classify() == CLASS_ANTLION )
								return true;

  							return false;
						}
					}
				}
			}
		}
	}
#endif
  		return true;
  	}
	//-----------------------------------------------------------------------------
	// Purpose: Whether or not the NPC should drop a health vial
	// Output : Returns true on success, false on failure.
	//-----------------------------------------------------------------------------
	bool CHalfLife2::NPC_ShouldDropHealth( CBasePlayer *pRecipient )
	{
		// Can only do this every so often
		if ( m_flLastHealthDropTime > gpGlobals->curtime )
			return false;

		//Try to throw dynamic health
		float healthPerc = ( (float) pRecipient->m_iHealth / (float) pRecipient->m_iMaxHealth );

		if ( random->RandomFloat( 0.0f, 1.0f ) > healthPerc*1.5f )
			return true;

		return false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Whether or not the NPC should drop a health vial
	// Output : Returns true on success, false on failure.
	//-----------------------------------------------------------------------------
	bool CHalfLife2::NPC_ShouldDropGrenade( CBasePlayer *pRecipient )
	{
		// Can only do this every so often
		if ( m_flLastGrenadeDropTime > gpGlobals->curtime )
			return false;
		
		int grenadeIndex = GetAmmoDef()->Index( "grenade" );
		int numGrenades = pRecipient->GetAmmoCount( grenadeIndex );

		// If we're not maxed out on grenades and we've randomly okay'd it
		if ( ( numGrenades < GetAmmoDef()->MaxCarry( grenadeIndex ) ) && ( random->RandomInt( 0, 2 ) == 0 ) )
			return true;

		return false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the drop counter for health
	//-----------------------------------------------------------------------------
	void CHalfLife2::NPC_DroppedHealth( void )
	{
		m_flLastHealthDropTime = gpGlobals->curtime + sk_plr_health_drop_time.GetFloat();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the drop counter for grenades
	//-----------------------------------------------------------------------------
	void CHalfLife2::NPC_DroppedGrenade( void )
	{
		m_flLastGrenadeDropTime = gpGlobals->curtime + sk_plr_grenade_drop_time.GetFloat();
	}

#endif //} !CLIENT_DLL


// ------------------------------------------------------------------------------------ //
// Shared CHalfLife2 implementation.
// ------------------------------------------------------------------------------------ //
bool CHalfLife2::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	// The smaller number is always first
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		int tmp = collisionGroup0;
		collisionGroup0 = collisionGroup1;
		collisionGroup1 = tmp;
	}
	
	// Prevent the player movement from colliding with spit globs (caused the player to jump on top of globs while in water)
	if ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT && collisionGroup1 == HL2COLLISION_GROUP_SPIT )
		return false;

	// HL2 treats movement and tracing against players the same, so just remap here
	if ( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT )
	{
		collisionGroup0 = COLLISION_GROUP_PLAYER;
	}

	if( collisionGroup1 == COLLISION_GROUP_PLAYER_MOVEMENT )
	{
		collisionGroup1 = COLLISION_GROUP_PLAYER;
	}

	//If collisionGroup0 is not a player then NPC_ACTOR behaves just like an NPC.
	if ( collisionGroup1 == COLLISION_GROUP_NPC_ACTOR && collisionGroup0 != COLLISION_GROUP_PLAYER )
	{
		collisionGroup1 = COLLISION_GROUP_NPC;
	}

	// This is only for the super physcannon
	if ( m_bMegaPhysgun )
	{
		if ( collisionGroup0 == COLLISION_GROUP_INTERACTIVE_DEBRIS && collisionGroup1 == COLLISION_GROUP_PLAYER )
			return false;
	}

	if ( collisionGroup0 == HL2COLLISION_GROUP_COMBINE_BALL )
	{
		if ( collisionGroup1 == HL2COLLISION_GROUP_COMBINE_BALL )
			return false;
	}

	if ( collisionGroup0 == HL2COLLISION_GROUP_COMBINE_BALL && collisionGroup1 == HL2COLLISION_GROUP_COMBINE_BALL_NPC )
		return false;

	if ( ( collisionGroup0 == COLLISION_GROUP_WEAPON ) ||
		( collisionGroup0 == COLLISION_GROUP_PLAYER ) ||
		( collisionGroup0 == COLLISION_GROUP_PROJECTILE ) )
	{
		if ( collisionGroup1 == HL2COLLISION_GROUP_COMBINE_BALL )
			return false;
	}

	if ( collisionGroup0 == COLLISION_GROUP_DEBRIS )
	{
		if ( collisionGroup1 == HL2COLLISION_GROUP_COMBINE_BALL )
			return true;
	}

	if (collisionGroup0 == HL2COLLISION_GROUP_HOUNDEYE && collisionGroup1 == HL2COLLISION_GROUP_HOUNDEYE )
		return false;

	if (collisionGroup0 == HL2COLLISION_GROUP_HOMING_MISSILE && collisionGroup1 == HL2COLLISION_GROUP_HOMING_MISSILE )
		return false;

	if ( collisionGroup1 == HL2COLLISION_GROUP_CROW )
	{
		if ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_NPC ||
			 collisionGroup0 == HL2COLLISION_GROUP_CROW )
			return false;
	}

	if ( ( collisionGroup0 == HL2COLLISION_GROUP_HEADCRAB ) && ( collisionGroup1 == HL2COLLISION_GROUP_HEADCRAB ) )
		return false;

	// striders don't collide with other striders
	if ( collisionGroup0 == HL2COLLISION_GROUP_STRIDER && collisionGroup1 == HL2COLLISION_GROUP_STRIDER )
		return false;

	// gunships don't collide with other gunships
	if ( collisionGroup0 == HL2COLLISION_GROUP_GUNSHIP && collisionGroup1 == HL2COLLISION_GROUP_GUNSHIP )
		return false;

	// weapons and NPCs don't collide
	if ( collisionGroup0 == COLLISION_GROUP_WEAPON && (collisionGroup1 >= HL2COLLISION_GROUP_FIRST_NPC && collisionGroup1 <= HL2COLLISION_GROUP_LAST_NPC ) )
		return false;

	//players don't collide against NPC Actors.
	//I could've done this up where I check if collisionGroup0 is NOT a player but I decided to just
	//do what the other checks are doing in this function for consistency sake.
	if ( collisionGroup1 == COLLISION_GROUP_NPC_ACTOR && collisionGroup0 == COLLISION_GROUP_PLAYER )
		return false;
		
	// In cases where NPCs are playing a script which causes them to interpenetrate while riding on another entity,
	// such as a train or elevator, you need to disable collisions between the actors so the mover can move them.
	if ( collisionGroup0 == COLLISION_GROUP_NPC_SCRIPTED && collisionGroup1 == COLLISION_GROUP_NPC_SCRIPTED )
		return false;

	// Spit doesn't touch other spit
	if ( collisionGroup0 == HL2COLLISION_GROUP_SPIT && collisionGroup1 == HL2COLLISION_GROUP_SPIT )
		return false;

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

#ifndef CLIENT_DLL
//---------------------------------------------------------
//---------------------------------------------------------
void CHalfLife2::AdjustPlayerDamageTaken( CTakeDamageInfo *pInfo )
{
	if( pInfo->GetDamageType() & (DMG_DROWN|DMG_CRUSH|DMG_FALL|DMG_POISON|DMG_SNIPER) )
	{
		// Skill level doesn't affect these types of damage.
		return;
	}

	switch( GetSkillLevel() )
	{
	case SKILL_EASY:
		pInfo->ScaleDamage( sk_dmg_take_scale1.GetFloat() );
		break;

	case SKILL_MEDIUM:
		pInfo->ScaleDamage( sk_dmg_take_scale2.GetFloat() );
		break;

	case SKILL_HARD:
		pInfo->ScaleDamage( sk_dmg_take_scale3.GetFloat() );
		break;
	}
}

//---------------------------------------------------------
//---------------------------------------------------------
float CHalfLife2::AdjustPlayerDamageInflicted( float damage )
{
	switch( GetSkillLevel() ) 
	{
	case SKILL_EASY:
		return damage * sk_dmg_inflict_scale1.GetFloat();
		break;

	case SKILL_MEDIUM:
		return damage * sk_dmg_inflict_scale2.GetFloat();
		break;

	case SKILL_HARD:
		return damage * sk_dmg_inflict_scale3.GetFloat();
		break;

	default:
		return damage;
		break;
	}
}
#endif//CLIENT_DLL

//---------------------------------------------------------
//---------------------------------------------------------
bool CHalfLife2::ShouldUseRobustRadiusDamage(CBaseEntity *pEntity)
{
#ifdef CLIENT_DLL
	return false;
#endif

	if( !sv_robust_explosions.GetBool() )
		return false;

	if( !pEntity->IsNPC() )
	{
		// Only NPC's
		return false;
	}

#ifndef CLIENT_DLL
	CAI_BaseNPC *pNPC = pEntity->MyNPCPointer();
	if( pNPC->CapabilitiesGet() & bits_CAP_SIMPLE_RADIUS_DAMAGE )
	{
		// This NPC only eligible for simple radius damage.
		return false;
	}
#endif//CLIENT_DLL

	return true;
}

#ifndef CLIENT_DLL
//---------------------------------------------------------
//---------------------------------------------------------
bool CHalfLife2::ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target )
{
	return false;// sk_allow_autoaim.GetBool() != 0;
}

//---------------------------------------------------------
//---------------------------------------------------------
float CHalfLife2::GetAutoAimScale( CBasePlayer *pPlayer )
{
#ifdef _X360
	return 1.0f;
#else
	switch( GetSkillLevel() )
	{
	case SKILL_EASY:
		return sk_autoaim_scale1.GetFloat();

	case SKILL_MEDIUM:
		return sk_autoaim_scale2.GetFloat();

	default:
		return 0.0f;
	}
#endif
}

//---------------------------------------------------------
//---------------------------------------------------------
float CHalfLife2::GetAmmoQuantityScale( int iAmmoIndex )
{
	switch( GetSkillLevel() )
	{
	case SKILL_EASY:
		return sk_ammo_qty_scale1.GetFloat();

	case SKILL_MEDIUM:
		return sk_ammo_qty_scale2.GetFloat();

	case SKILL_HARD:
		return sk_ammo_qty_scale3.GetFloat();

	default:
		return 0.0f;
	}
}

void CHalfLife2::LevelInitPreEntity()
{
	// Remove this if you fix the bug in ep1 where the striders need to touch
	// triggers using their absbox instead of their bbox
#ifdef HL2_EPISODIC
	if ( !Q_strnicmp( gpGlobals->mapname.ToCStr(), "ep1_", 4 ) )
	{
		// episode 1 maps use the surrounding box trigger behavior
		CBaseEntity::sm_bAccurateTriggerBboxChecks = false;
	}
#endif
	BaseClass::LevelInitPreEntity();
}

//-----------------------------------------------------------------------------
// Returns whether or not Alyx cares about light levels in order to see.
//-----------------------------------------------------------------------------
bool CHalfLife2::IsAlyxInDarknessMode()
{
#ifdef HL2_EPISODIC
	if ( alyx_darkness_force.GetBool() )
		return true;

	return ( GlobalEntity_GetState( "ep_alyx_darknessmode" ) == GLOBAL_ON );
#else
	return false;
#endif // HL2_EPISODIC
}


//-----------------------------------------------------------------------------
// This takes the long way around to see if a prop should emit a DLIGHT when it
// ignites, to avoid having Alyx-related code in props.cpp.
//-----------------------------------------------------------------------------
bool CHalfLife2::ShouldBurningPropsEmitLight()
{
#ifdef HL2_EPISODIC
	return IsAlyxInDarknessMode();
#else
	return false;
#endif // HL2_EPISODIC
}


#endif//CLIENT_DLL

// ------------------------------------------------------------------------------------ //
// Global functions.
// ------------------------------------------------------------------------------------ //

#ifndef HL2MP
#ifndef PORTAL

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			3.5
// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)

#define BULLET_SPEED(ftpersec) 0.12*ftpersec //inches per centisecond

CAmmoDef *GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;
	if (cvar->FindVar("oc_ragdoll_serverside")->GetBool())
	{ 
		if (!bInitted)
		{
			bInitted = true;
			
			def.AddAmmoType("AR2", DMG_BULLET, TRACER_AR2, "sk_plr_dmg_ar2", "sk_npc_dmg_ar2", "sk_max_ar2", BULLET_SPEED(680), BULLET_IMPULSE(1300, 2800), 0);//BULLET_IMPULSE(200, 1225)
			def.AddAmmoType("AlyxGun", DMG_BULLET, TRACER_PISTOL, "sk_plr_dmg_alyxgun", "sk_npc_dmg_alyxgun", "sk_max_alyxgun", BULLET_SPEED(680), BULLET_IMPULSE(1000, 2200), 0);
			def.AddAmmoType("Pistol", DMG_BULLET, TRACER_PISTOL, "sk_plr_dmg_pistol", "sk_npc_dmg_pistol", "sk_max_pistol", BULLET_SPEED(680), BULLET_IMPULSE(1100, 2400), 0);
			def.AddAmmoType("SMG1", DMG_BULLET, TRACER_SMG, "sk_plr_dmg_smg1", "sk_npc_dmg_smg1", "sk_max_smg1", BULLET_SPEED(680), BULLET_IMPULSE(1200, 2200), 0);
			def.AddAmmoType("M249", DMG_BULLET, TRACER_SMG, "sk_plr_dmg_m249", "sk_npc_dmg_m249", "sk_max_m249", BULLET_SPEED(680), BULLET_IMPULSE(1200, 2200), 0);
			def.AddAmmoType("FlareRound", DMG_BURN, TRACER_LINE, "sk_plr_dmg_flare_round", "sk_npc_dmg_flare_round", "sk_max_flare_round", 0, BULLET_IMPULSE(1500, 600), 0);	//added
			def.AddAmmoType("357", DMG_BULLET, TRACER_357, "sk_plr_dmg_357", "sk_npc_dmg_357", "sk_max_357", BULLET_SPEED(680), BULLET_IMPULSE(1800, 2900), 0);
			def.AddAmmoType("XBowBolt", DMG_BULLET, TRACER_LINE, "sk_plr_dmg_crossbow", "sk_npc_dmg_crossbow", "sk_max_crossbow", BULLET_SPEED(680), BULLET_IMPULSE(3800, 4000), 0);

			def.AddAmmoType("Buckshot", DMG_BULLET | DMG_BUCKSHOT, TRACER_SHOTGUN, "sk_plr_dmg_buckshot", "sk_npc_dmg_buckshot", "sk_max_buckshot", BULLET_SPEED(500), BULLET_IMPULSE(400, 800), 0);//BULLET_IMPULSE(400, 1200)
			def.AddAmmoType("RPG_Round", DMG_BURN, TRACER_NONE, "sk_plr_dmg_rpg_round", "sk_npc_dmg_rpg_round", "sk_max_rpg_round", 0, 0, 0);
			def.AddAmmoType("SMG1_Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_smg1_grenade", "sk_npc_dmg_smg1_grenade", "sk_max_smg1_grenade", 0, 0, 0);
			def.AddAmmoType("SniperRound", DMG_BULLET, TRACER_SMG, "sk_plr_dmg_sniper_round", "sk_npc_dmg_sniper_round", "sk_max_sniper_round", BULLET_SPEED(1000), BULLET_IMPULSE(2050, 3000), 0);
			def.AddAmmoType("CombineSniperRound", DMG_BULLET, TRACER_SNIPER, "sk_plr_dmg_combine_sniper_round", "sk_npc_dmg_combine_sniper_round", "sk_max_combine_sniper_round", BULLET_SPEED(1000), BULLET_IMPULSE(2050, 3000), 0);

			def.AddAmmoType("SniperPenetratedRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE, "sk_dmg_sniper_penetrate_plr", "sk_dmg_sniper_penetrate_npc", "sk_max_sniper_round", BULLET_SPEED(1000), BULLET_IMPULSE(150, 6000), 0);
			def.AddAmmoType("Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_grenade", 0, 0, 0);
			def.AddAmmoType("Thumper", DMG_SONIC, TRACER_NONE, 10, 10, 2, 0, 0, 0);
			def.AddAmmoType("Gravity", DMG_CLUB, TRACER_NONE, 0, 0, 10, 0, 0, 0);
			def.AddAmmoType("Extinguisher", DMG_BURN, TRACER_NONE, 0, 0, 100, 0, 0, 0);
			def.AddAmmoType("Battery", DMG_CLUB, TRACER_NONE, NULL, NULL, NULL, 0, 0, 0);
			def.AddAmmoType("GaussEnergy", DMG_GAUSS, TRACER_357, "sk_jeep_gauss_damage", "sk_jeep_gauss_damage", "sk_max_gauss_round", BULLET_SPEED(2000), BULLET_IMPULSE(1650, 2600), 0); // hit like a 10kg weight at 400 in/s
			def.AddAmmoType("CombineCannon", DMG_BULLET, TRACER_ABGUN, "sk_npc_dmg_gunship_to_plr", "sk_npc_dmg_gunship", NULL, BULLET_SPEED(2000), BULLET_IMPULSE(150, 800), 0); // hit like a 1.5kg weight at 750 ft/s
			def.AddAmmoType("AirboatGun", DMG_AIRBOAT, TRACER_ABGUN, "sk_plr_dmg_airboat", "sk_npc_dmg_airboat", NULL, BULLET_SPEED(650), BULLET_IMPULSE(140, 1000), 0);

			//=====================================================================
			// STRIDER MINIGUN DAMAGE - Pull up a chair and I'll tell you a tale.
			//
			// When we shipped Half-Life 2 in 2004, we were unaware of a bug in
			// CAmmoDef::NPCDamage() which was returning the MaxCarry field of
			// an ammotype as the amount of damage that should be done to a NPC
			// by that type of ammo. Thankfully, the bug only affected Ammo Types 
			// that DO NOT use ConVars to specify their parameters. As you can see,
			// all of the important ammotypes use ConVars, so the effect of the bug
			// was limited. The Strider Minigun was affected, though.
			//
			// According to my perforce Archeology, we intended to ship the Strider
			// Minigun ammo type to do 15 points of damage per shot, and we did. 
			// To achieve this we, unaware of the bug, set the Strider Minigun ammo 
			// type to have a maxcarry of 15, since our observation was that the 
			// number that was there before (8) was indeed the amount of damage being
			// done to NPC's at the time. So we changed the field that was incorrectly
			// being used as the NPC Damage field.
			//
			// The bug was fixed during Episode 1's development. The result of the 
			// bug fix was that the Strider was reduced to doing 5 points of damage
			// to NPC's, since 5 is the value that was being assigned as NPC damage
			// even though the code was returning 15 up to that point.
			//
			// Now as we go to ship Orange Box, we discover that the Striders in 
			// Half-Life 2 are hugely ineffective against citizens, causing big
			// problems in maps 12 and 13. 
			//
			// In order to restore balance to HL2 without upsetting the delicate 
			// balance of ep2_outland_12, I have chosen to build Episodic binaries
			// with 5 as the Strider->NPC damage, since that's the value that has
			// been in place for all of Episode 2's development. Half-Life 2 will
			// build with 15 as the Strider->NPC damage, which is how HL2 shipped
			// originally, only this time the 15 is located in the correct field
			// now that the AmmoDef code is behaving correctly.
			//
			//=====================================================================

#ifdef HL2_EPISODIC
			def.AddAmmoType("StriderMinigun", DMG_BULLET, TRACER_ABGUN, 5, 5, 15, BULLET_SPEED(650), 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 1.0kg weight at 750 ft/s
#else
			def.AddAmmoType("StriderMinigun", DMG_BULLET, TRACER_ABGUN, 5, 15, 15, BULLET_SPEED(1225), 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 1.0kg weight at 750 ft/s
#endif//HL2_EPISODIC

			def.AddAmmoType("StriderMinigunDirect", DMG_BULLET, TRACER_ABGUN, 2, 2, 15, BULLET_SPEED(650), 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 1.0kg weight at 750 ft/s
			def.AddAmmoType("HelicopterGun", DMG_BULLET, TRACER_ABGUN, "sk_npc_dmg_helicopter_to_plr", "sk_npc_dmg_helicopter", "sk_max_smg1", BULLET_SPEED(600), BULLET_IMPULSE(400, 1225), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER);
			def.AddAmmoType("AR2AltFire", DMG_DISSOLVE, TRACER_NONE, 0, 0, "sk_max_ar2_altfire", 0, 0, 0);
			def.AddAmmoType("Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_grenade", 0, 0, 0);
			def.AddAmmoType("Tripwire", DMG_BLAST, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_tripwire", 0, 0, 0);
			def.AddAmmoType("Hopwire", DMG_BLAST, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_hopwire", 0, 0, 0);
#ifdef HL2_EPISODIC//OverCharged

			def.AddAmmoType("CombineHeavyCannon", DMG_BULLET, TRACER_ABGUN, 40, 40, NULL, BULLET_SPEED(7225), 10 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 10 kg weight at 750 ft/s
			def.AddAmmoType("ammo_proto1", DMG_BULLET, TRACER_LINE, 0, 0, 10, 0, 0, 0);
#endif // HL2_EPISODIC//OverCharged

			//		def.AddAmmoType("Particle", DMG_SHOCK | DMG_ENERGYBEAM, TRACER_NONE, "sk_plr_dmg_particlerifle", "sk_npc_dmg_particlerifle", "sk_max_particlerifle", BULLET_IMPULSE(600, 1525), 0);
			//		def.AddAmmoType("Laser", DMG_DISSOLVE, TRACER_NONE, "sk_plr_dmg_Laser", "sk_npc_dmg_Laser", "sk_max_Laser", BULLET_IMPULSE(200, 1225), 0);
			def.AddAmmoType("Ak47", DMG_BULLET, TRACER_AK47, "sk_plr_dmg_Ak47", "sk_npc_dmg_Ak47", "sk_max_Ak47", BULLET_SPEED(680), BULLET_IMPULSE(1300, 2800), 0);
			def.AddAmmoType("AR3", DMG_BULLET, TRACER_AR2, "sk_plr_dmg_ar3", "sk_npc_dmg_ar3", "sk_max_ar3", BULLET_SPEED(680), BULLET_IMPULSE(1200, 2625), 0);
			def.AddAmmoType("ABGUN", DMG_AIRBOAT, TRACER_ABGUN, "sk_plr_dmg_abgun", "sk_npc_dmg_abgun", "sk_max_abgun", BULLET_SPEED(680), BULLET_IMPULSE(700, 1300), 0);
			def.AddAmmoType("Uranium", DMG_DISSOLVE, TRACER_NONE, "sk_plr_dmg_uranium", "sk_npc_dmg_uranium", "sk_max_uranium", 0, BULLET_IMPULSE(1000, 1500), 0);
			def.AddAmmoType("OICW", DMG_BULLET, TRACER_OICW, "sk_plr_dmg_oicw", "sk_npc_dmg_oicw", "sk_max_oicw", BULLET_SPEED(680), BULLET_IMPULSE(800, 2700), 0);
			def.AddAmmoType("FlareRound", DMG_BURN, TRACER_NONE, "sk_plr_dmg_flareround", "sk_npc_dmg_flareround", "sk_max_flareround", 0, BULLET_IMPULSE(620, 3700), 0);
			//		def.AddAmmoType("OICW_Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_oicw_grenade", "sk_npc_dmg_oicw_grenade", "sk_max_oicw_grenade", 0, 0);
			def.AddAmmoType("FlashGrenade", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_grenade", 0, 0, 0);	//23
			def.AddAmmoType("SmokeGrenade", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_grenade", 0, 0, 0);	//24
			def.AddAmmoType("Molotov", DMG_BURN, TRACER_NONE, "sk_plr_dmg_molotov", "sk_npc_dmg_molotov", "sk_max_molotov", 0, 0, 0);
			def.AddAmmoType("Slam", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_slam", 0, 0, 0);
			def.AddAmmoType("HopMine", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_hopmine", 0, 0, 0);
			def.AddAmmoType("SporeAcid", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_spore_acid", "sk_npc_dmg_spore_acid", "sk_max_spore_acid", BULLET_SPEED(680), BULLET_IMPULSE(1500, 2225), 0);
			def.AddAmmoType("SporeAcidBounce", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_spore_acid_bounce", "sk_npc_dmg_spore_acid_bounce", "sk_max_spore_acid_bounce", BULLET_SPEED(680), BULLET_IMPULSE(1500, 2225), 0);
			def.AddAmmoType("LaserPistol", DMG_DISSOLVE, TRACER_NONE, "sk_plr_dmg_laserpistol", "sk_npc_dmg_laserpistol", "sk_max_laserpistol", BULLET_SPEED(680), BULLET_IMPULSE(200, 1225), 0);
			def.AddAmmoType("Hornet", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_hornet", "sk_npc_dmg_hornet", "sk_max_hornet", BULLET_SPEED(680), BULLET_IMPULSE(1100, 2400), 0);
			def.AddAmmoType("Shock", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_shock", "sk_npc_dmg_shock", "sk_max_shock", BULLET_SPEED(680), BULLET_IMPULSE(1100, 2400), 0);

			//#endif // HL2_EPISODIC//OverCharged
		}
	}
	else
	{
		if (!bInitted)
		{
			bInitted = true;

			def.AddAmmoType("AR2", DMG_BULLET, TRACER_AR2, "sk_plr_dmg_ar2", "sk_npc_dmg_ar2", "sk_max_ar2", BULLET_SPEED(680), BULLET_IMPULSE(200, 1225), 0);//BULLET_IMPULSE(200, 1225)
			def.AddAmmoType("AlyxGun", DMG_BULLET, TRACER_PISTOL, "sk_plr_dmg_alyxgun", "sk_npc_dmg_alyxgun", "sk_max_alyxgun", BULLET_SPEED(680), BULLET_IMPULSE(200, 1225), 0);
			def.AddAmmoType("Pistol", DMG_BULLET, TRACER_PISTOL, "sk_plr_dmg_pistol", "sk_npc_dmg_pistol", "sk_max_pistol", BULLET_SPEED(680), BULLET_IMPULSE(200, 1225), 0);
			def.AddAmmoType("SMG1", DMG_BULLET, TRACER_SMG, "sk_plr_dmg_smg1", "sk_npc_dmg_smg1", "sk_max_smg1", BULLET_SPEED(680), BULLET_IMPULSE(200, 1225), 0);
			def.AddAmmoType("M249", DMG_BULLET, TRACER_SMG, "sk_plr_dmg_m249", "sk_npc_dmg_m249", "sk_max_m249", BULLET_SPEED(680), BULLET_IMPULSE(1200, 2200), 0);
			def.AddAmmoType("FlareRound", DMG_BURN, TRACER_LINE, "sk_plr_dmg_flare_round", "sk_npc_dmg_flare_round", "sk_max_flare_round", 0, BULLET_IMPULSE(1500, 600), 0);	//added
			def.AddAmmoType("357", DMG_BULLET, TRACER_357, "sk_plr_dmg_357", "sk_npc_dmg_357", "sk_max_357", BULLET_SPEED(680), BULLET_IMPULSE(800, 5000), 0);
			def.AddAmmoType("XBowBolt", DMG_BULLET, TRACER_LINE, "sk_plr_dmg_crossbow", "sk_npc_dmg_crossbow", "sk_max_crossbow", BULLET_SPEED(680), BULLET_IMPULSE(800, 8000), 0);

			def.AddAmmoType("Buckshot", DMG_BULLET | DMG_BUCKSHOT, TRACER_SHOTGUN, "sk_plr_dmg_buckshot", "sk_npc_dmg_buckshot", "sk_max_buckshot", BULLET_SPEED(500), BULLET_IMPULSE(400, 800), 0);//BULLET_IMPULSE(400, 1200)
			def.AddAmmoType("RPG_Round", DMG_BURN, TRACER_NONE, "sk_plr_dmg_rpg_round", "sk_npc_dmg_rpg_round", "sk_max_rpg_round", 0, 0, 0);
			def.AddAmmoType("SMG1_Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_smg1_grenade", "sk_npc_dmg_smg1_grenade", "sk_max_smg1_grenade", 0, 0, 0);
			def.AddAmmoType("SniperRound", DMG_BULLET, TRACER_SMG, "sk_plr_dmg_sniper_round", "sk_npc_dmg_sniper_round", "sk_max_sniper_round", BULLET_SPEED(1000), BULLET_IMPULSE(1350, 6000), 0);
			def.AddAmmoType("CombineSniperRound", DMG_BULLET, TRACER_SNIPER, "sk_plr_dmg_combine_sniper_round", "sk_npc_dmg_combine_sniper_round", "sk_max_combine_sniper_round", BULLET_SPEED(1000), BULLET_IMPULSE(1350, 6000), 0);

			def.AddAmmoType("SniperPenetratedRound", DMG_BULLET | DMG_SNIPER, TRACER_NONE, "sk_dmg_sniper_penetrate_plr", "sk_dmg_sniper_penetrate_npc", "sk_max_sniper_round", BULLET_SPEED(1000), BULLET_IMPULSE(150, 6000), 0);
			def.AddAmmoType("Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_grenade", 0, 0, 0);
			def.AddAmmoType("Thumper", DMG_SONIC, TRACER_NONE, 10, 10, 2, 0, 0, 0);
			def.AddAmmoType("Gravity", DMG_CLUB, TRACER_NONE, 0, 0, 10, 0, 0, 0);
			def.AddAmmoType("Extinguisher", DMG_BURN, TRACER_NONE, 0, 0, 100, 0, 0, 0);
			def.AddAmmoType("Battery", DMG_CLUB, TRACER_NONE, NULL, NULL, NULL, 0, 0, 0);
			def.AddAmmoType("GaussEnergy", DMG_GAUSS, TRACER_357, "sk_jeep_gauss_damage", "sk_jeep_gauss_damage", "sk_max_gauss_round", BULLET_SPEED(2000), BULLET_IMPULSE(650, 8000), 0); // hit like a 10kg weight at 400 in/s
			def.AddAmmoType("CombineCannon", DMG_BULLET, TRACER_ABGUN, "sk_npc_dmg_gunship_to_plr", "sk_npc_dmg_gunship", NULL, BULLET_SPEED(2000), BULLET_IMPULSE(150, 500), 0); // hit like a 1.5kg weight at 750 ft/s
			def.AddAmmoType("AirboatGun", DMG_AIRBOAT, TRACER_ABGUN, "sk_plr_dmg_airboat", "sk_npc_dmg_airboat", NULL, BULLET_SPEED(650), BULLET_IMPULSE(140, 1000), 0);

			//=====================================================================
			// STRIDER MINIGUN DAMAGE - Pull up a chair and I'll tell you a tale.
			//
			// When we shipped Half-Life 2 in 2004, we were unaware of a bug in
			// CAmmoDef::NPCDamage() which was returning the MaxCarry field of
			// an ammotype as the amount of damage that should be done to a NPC
			// by that type of ammo. Thankfully, the bug only affected Ammo Types 
			// that DO NOT use ConVars to specify their parameters. As you can see,
			// all of the important ammotypes use ConVars, so the effect of the bug
			// was limited. The Strider Minigun was affected, though.
			//
			// According to my perforce Archeology, we intended to ship the Strider
			// Minigun ammo type to do 15 points of damage per shot, and we did. 
			// To achieve this we, unaware of the bug, set the Strider Minigun ammo 
			// type to have a maxcarry of 15, since our observation was that the 
			// number that was there before (8) was indeed the amount of damage being
			// done to NPC's at the time. So we changed the field that was incorrectly
			// being used as the NPC Damage field.
			//
			// The bug was fixed during Episode 1's development. The result of the 
			// bug fix was that the Strider was reduced to doing 5 points of damage
			// to NPC's, since 5 is the value that was being assigned as NPC damage
			// even though the code was returning 15 up to that point.
			//
			// Now as we go to ship Orange Box, we discover that the Striders in 
			// Half-Life 2 are hugely ineffective against citizens, causing big
			// problems in maps 12 and 13. 
			//
			// In order to restore balance to HL2 without upsetting the delicate 
			// balance of ep2_outland_12, I have chosen to build Episodic binaries
			// with 5 as the Strider->NPC damage, since that's the value that has
			// been in place for all of Episode 2's development. Half-Life 2 will
			// build with 15 as the Strider->NPC damage, which is how HL2 shipped
			// originally, only this time the 15 is located in the correct field
			// now that the AmmoDef code is behaving correctly.
			//
			//=====================================================================

#ifdef HL2_EPISODIC
			def.AddAmmoType("StriderMinigun", DMG_BULLET, TRACER_ABGUN, 5, 5, 15, BULLET_SPEED(650), 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 1.0kg weight at 750 ft/s
#else
			def.AddAmmoType("StriderMinigun",	DMG_BULLET,					TRACER_ABGUN,			5, 15,15, BULLET_SPEED(1225),	1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED ); // hit like a 1.0kg weight at 750 ft/s
#endif//HL2_EPISODIC

			def.AddAmmoType("StriderMinigunDirect", DMG_BULLET, TRACER_ABGUN, 2, 2, 15, BULLET_SPEED(650), 1.0 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 1.0kg weight at 750 ft/s
			def.AddAmmoType("HelicopterGun", DMG_BULLET, TRACER_ABGUN, "sk_npc_dmg_helicopter_to_plr", "sk_npc_dmg_helicopter", "sk_max_smg1", BULLET_SPEED(600), BULLET_IMPULSE(400, 1225), AMMO_FORCE_DROP_IF_CARRIED | AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER);
			def.AddAmmoType("AR2AltFire", DMG_DISSOLVE, TRACER_NONE, 0, 0, "sk_max_ar2_altfire", 0, 0, 0);
			def.AddAmmoType("Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_grenade", 0, 0, 0);
			def.AddAmmoType("Tripwire", DMG_BLAST, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_tripwire", 0, 0, 0);
			def.AddAmmoType("Hopwire", DMG_BLAST, TRACER_NONE, "sk_plr_dmg_grenade", "sk_npc_dmg_grenade", "sk_max_hopwire", 0, 0, 0);
#ifdef HL2_EPISODIC//OverCharged

			def.AddAmmoType("CombineHeavyCannon", DMG_BULLET, TRACER_ABGUN, 40, 40, NULL, BULLET_SPEED(7225), 10 * 750 * 12, AMMO_FORCE_DROP_IF_CARRIED); // hit like a 10 kg weight at 750 ft/s
			def.AddAmmoType("ammo_proto1", DMG_BULLET, TRACER_LINE, 0, 0, 10, 0, 0, 0);
#endif // HL2_EPISODIC//OverCharged

			//		def.AddAmmoType("Particle", DMG_SHOCK | DMG_ENERGYBEAM, TRACER_NONE, "sk_plr_dmg_particlerifle", "sk_npc_dmg_particlerifle", "sk_max_particlerifle", BULLET_IMPULSE(600, 1525), 0);
			//		def.AddAmmoType("Laser", DMG_DISSOLVE, TRACER_NONE, "sk_plr_dmg_Laser", "sk_npc_dmg_Laser", "sk_max_Laser", BULLET_IMPULSE(200, 1225), 0);
			def.AddAmmoType("Ak47", DMG_BULLET, TRACER_AK47, "sk_plr_dmg_Ak47", "sk_npc_dmg_Ak47", "sk_max_Ak47", BULLET_SPEED(680), BULLET_IMPULSE(500, 3225), 0);
			def.AddAmmoType("AR3", DMG_BULLET, TRACER_AR2, "sk_plr_dmg_ar3", "sk_npc_dmg_ar3", "sk_max_ar3", BULLET_SPEED(680), BULLET_IMPULSE(400, 1225), 0);
			def.AddAmmoType("ABGUN", DMG_AIRBOAT, TRACER_ABGUN, "sk_plr_dmg_abgun", "sk_npc_dmg_abgun", "sk_max_abgun", BULLET_SPEED(680), BULLET_IMPULSE(700, 2000), 0);
			def.AddAmmoType("Uranium", DMG_DISSOLVE, TRACER_NONE, "sk_plr_dmg_uranium", "sk_npc_dmg_uranium", "sk_max_uranium", 0, BULLET_IMPULSE(1000, 1500), 0);
			def.AddAmmoType("OICW", DMG_BULLET, TRACER_OICW, "sk_plr_dmg_oicw", "sk_npc_dmg_oicw", "sk_max_oicw", BULLET_SPEED(680), BULLET_IMPULSE(620, 3700), 0);
			def.AddAmmoType("FlareRound", DMG_BURN, TRACER_NONE, "sk_plr_dmg_flareround", "sk_npc_dmg_flareround", "sk_max_flareround", 0, BULLET_IMPULSE(620, 3700), 0);
			//		def.AddAmmoType("OICW_Grenade", DMG_BURN, TRACER_NONE, "sk_plr_dmg_oicw_grenade", "sk_npc_dmg_oicw_grenade", "sk_max_oicw_grenade", 0, 0);
			def.AddAmmoType("FlashGrenade", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_grenade", 0, 0, 0);	//23
			def.AddAmmoType("SmokeGrenade", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_grenade", 0, 0, 0);	//24
			def.AddAmmoType("Molotov", DMG_BURN, TRACER_NONE, "sk_plr_dmg_molotov", "sk_npc_dmg_molotov", "sk_max_molotov", 0, 0, 0);
			def.AddAmmoType("Slam", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_slam", 0, 0, 0);
			def.AddAmmoType("HopMine", DMG_BURN, TRACER_NONE, NULL, NULL, "sk_max_hopmine", 0, 0, 0);
			def.AddAmmoType("SporeAcid", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_spore_acid", "sk_npc_dmg_spore_acid", "sk_max_spore_acid", BULLET_SPEED(680), BULLET_IMPULSE(500, 3225), 0);
			def.AddAmmoType("SporeAcidBounce", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_spore_acid_bounce", "sk_npc_dmg_spore_acid_bounce", "sk_max_spore_acid_bounce", BULLET_SPEED(680), BULLET_IMPULSE(500, 3225), 0);
			def.AddAmmoType("LaserPistol", DMG_DISSOLVE, TRACER_NONE, "sk_plr_dmg_laserpistol", "sk_npc_dmg_laserpistol", "sk_max_laserpistol", BULLET_SPEED(680), BULLET_IMPULSE(200, 1225), 0);
			def.AddAmmoType("Hornet", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_hornet", "sk_npc_dmg_hornet", "sk_max_hornet", BULLET_SPEED(680), BULLET_IMPULSE(1100, 2400), 0);
			def.AddAmmoType("Shock", DMG_BULLET, TRACER_NONE, "sk_plr_dmg_shock", "sk_npc_dmg_shock", "sk_max_shock", BULLET_SPEED(680), BULLET_IMPULSE(1100, 2400), 0);

			//#endif // HL2_EPISODIC//OverCharged
		}
	}
	
	return &def;
}

#endif
#endif
