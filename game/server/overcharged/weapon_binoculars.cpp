//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: Implements binoculars.
//			
//			Primary attack button: zoom in.
//			Secondary attack button: zoom out.
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "NPCEvent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"
#include "player.h"
#include "gamerules.h"				// For g_pGameRules
#include "entitylist.h"
#include "soundenvelope.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"

#include "materialsystem/IMaterialSystem.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
//#ifdef CLIENT_DLL
//#include "viewrender.h"
//#endif

#define BINOCULARS_MAXSOURCES	50
#define BINOCULARS_CACHEFREQ	1.0
#define BINOCULARS_FAKEDIST		99999
#define BINOCULARS_VIEWCONE		0.6

#define BINOCULARS_ZOOM_RATE		0.15			// Interval between zoom levels in seconds.

#define WB_STATIC_CHANNEL	CHAN_VOICE
#define WB_SIGNAL_CHANNEL	CHAN_BODY

ConVar oc_weapon_binoculars_overlay( "oc_weapon_binoculars_overlay", "HUD/Binocular", FCVAR_REPLICATED|FCVAR_ARCHIVE, "Binoculars overlay." );
ConVar oc_weapon_binoculars_overlay_draw( "oc_weapon_binoculars_overlay_draw", "0", FCVAR_HIDDEN | FCVAR_REPLICATED, "Draw overlay?" );

enum ZoomMode_t
{
	Zoom_Exit = 0,
	Zoom_In,
	Zoom_Out
};


//-----------------------------------------------------------------------------
// Discrete zoom levels for the scope.
//-----------------------------------------------------------------------------
static int g_nZoomFOV[] =
{
	0,
	40,
	30,
	20,
	10,
	5
};


class CWeaponBinoculars : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponBinoculars, CBaseHLCombatWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	void Spawn( void );
	CWeaponBinoculars( void );
	~CWeaponBinoculars( void );
	void Precache( void );
	int CapabilitiesGet( void );
	bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	void ItemPostFrame( void );
	void Zoom( ZoomMode_t eMode );
	bool Deploy( void );
	//void CacheSoundSources( void );
	void	Drop( const Vector &vecVelocity );
	
	CBaseEntity *LocateBestSound( void );
	float GetSoundDist( CBaseEntity *pSound, const Vector &vecLOS );

protected:

	//CSoundPatch	*m_pSoundStatic;
	CSoundPatch	*m_pSoundSignal;

	float m_fNextZoom;
	int m_nZoomLevel;

	//static const char *pStaticSounds[];	// L1ght 15 : No more needed
	static const char *pSignalSounds[];

	float m_NextCacheTime;
	EHANDLE	m_hSources[ BINOCULARS_MAXSOURCES ];

	DECLARE_DATADESC();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponBinoculars, DT_WeaponBinoculars)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_binoculars, CWeaponBinoculars );
PRECACHE_WEAPON_REGISTER(weapon_binoculars);

//---------------------------------------------------------
//---------------------------------------------------------
BEGIN_DATADESC( CWeaponBinoculars )

	DEFINE_FIELD( m_NextCacheTime, FIELD_TIME ),
	DEFINE_SOUNDPATCH( m_pSoundSignal ),
	//DEFINE_SOUNDPATCH( m_pSoundStatic ),
	DEFINE_FIELD( m_fNextZoom, FIELD_FLOAT ),
	DEFINE_FIELD( m_nZoomLevel, FIELD_INTEGER ),
	DEFINE_ARRAY( m_hSources, FIELD_EHANDLE, BINOCULARS_MAXSOURCES ),

END_DATADESC()

acttable_t CWeaponBinoculars::m_acttable[] =
{
    { ACT_HL2MP_IDLE,                   ACT_IDLE_PACKAGE,						false },
    { ACT_HL2MP_RUN,                    ACT_RUN,							false },
    { ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_PHYSGUN,            false },
    { ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_PHYSGUN,            false },
    { ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,    false },
    { ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,        false },
    { ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_PHYSGUN,                    false },
    { ACT_RANGE_ATTACK1,                ACT_RANGE_ATTACK_SLAM,                false },

	{ ACT_IDLE_RELAXED,				ACT_IDLE_PACKAGE,		false },	// L1ght 15 : Anims update
	{ ACT_RUN_RELAXED,				ACT_RUN,				false },
};
IMPLEMENT_ACTTABLE( CWeaponBinoculars );

//---------------------------------------------------------
//---------------------------------------------------------
void CWeaponBinoculars::Spawn( void )
{
	BaseClass::Spawn();

	int i;

	for( i = 0 ; i < BINOCULARS_MAXSOURCES ; i++ )
	{
		m_hSources[ i ] = NULL;
	}
}

//-----------------------------------------------------------------------------			// L1ght 15 : No more needed
// Binocular radio sounds
//-----------------------------------------------------------------------------
/*const char *CWeaponBinoculars::pStaticSounds[] = 
{
	// These sounds are played constantly whilst the binoculars are
	// in use.
	"weapons/binoculars/buzz.wav",
};*/

//---------------------------------------------------------
//---------------------------------------------------------
const char *CWeaponBinoculars::pSignalSounds[] = 
{
	// These sounds play when the binoculars are nearly locked onto a signal
	//"weapons/binoculars/signal.wav",
	"weapons/airboat/airboat_gun_lastshot1.wav",
};

//-----------------------------------------------------------------------------
// Only need to call this every second or so WHEN the binoculars are zoomed in.
//-----------------------------------------------------------------------------
/*void CWeaponBinoculars::CacheSoundSources( void )		// BriJee - NOT work on sdk 2013, will be rewritten if needs
{
	EHANDLE			*pSources = m_hSources;
	CBaseEntity		*pEnt;
	
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	Assert( pPlayer != NULL );
	if (pPlayer == NULL)
	{
		return;
	}

	pEnt = UTIL_EntitiesInPVS( pPlayer, NULL );
	int count = 0;
	
	while( pEnt )
	{
		if( pEnt->IsSoundSource() )
		{
			*pSources = pEnt;
			pSources++;
			count++;
		}

		if( count == ( BINOCULARS_MAXSOURCES - 1 ) )
		{
			//Msg( "*** BINOCULARS: TOO MANY SOUND SOURCES!\n" );
			break;
		}

		pEnt = UTIL_EntitiesInPVS( pPlayer, pEnt );
	}

	pSources = NULL;
	m_NextCacheTime = gpGlobals->curtime + BINOCULARS_CACHEFREQ;
}*/


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponBinoculars::Deploy()
{
	// Ensure that we cache sounds as soon as the binoculars zoom in.
	m_NextCacheTime = gpGlobals->curtime;

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	// Create our sounds the first time we're used
	if ( /*!m_pSoundStatic ||*/ !m_pSoundSignal )
	{
		CPASAttenuationFilter filter( this, ATTN_NONE );
		//m_pSoundStatic = controller.SoundCreate( filter, entindex(), WB_STATIC_CHANNEL, pStaticSounds[ 0 ], ATTN_NONE );
		m_pSoundSignal = controller.SoundCreate( filter, entindex(), WB_SIGNAL_CHANNEL, pSignalSounds[ 0 ], ATTN_NONE );
	}

	//controller.CommandClear( m_pSoundStatic );
	controller.CommandClear( m_pSoundSignal );

	//controller.Play( m_pSoundStatic, 0.0, 100 );
	controller.Play( m_pSoundSignal, 0.0, 100 );

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CWeaponBinoculars::CWeaponBinoculars( void )
{
	m_fNextZoom = gpGlobals->curtime;
	m_nZoomLevel = 0;
	//m_pSoundStatic = NULL;
	m_pSoundSignal = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor - Free the sound resources we allocate in Spawn()
//-----------------------------------------------------------------------------
CWeaponBinoculars::~CWeaponBinoculars( void )
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	//controller.SoundDestroy( m_pSoundStatic );
	controller.SoundDestroy( m_pSoundSignal );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponBinoculars::CapabilitiesGet( void )
{
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Turns off the zoom when the binoculars are holstered.
//-----------------------------------------------------------------------------
bool CWeaponBinoculars::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	Zoom( Zoom_Exit );

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	//controller.SoundDestroy( m_pSoundStatic );
	controller.SoundDestroy( m_pSoundSignal );
	//m_pSoundStatic = NULL;
	m_pSoundSignal = NULL;

	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CWeaponBinoculars::GetSoundDist( CBaseEntity *pSound, const Vector &vecLOS )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return 0.0f;

	Vector	vecDirToSound;
	float	flDot;
	float	flDist;

	vecDirToSound = pSound->GetAbsOrigin() - pPlayer->GetAbsOrigin();
	VectorNormalize(vecDirToSound);

	flDot = DotProduct( vecDirToSound, vecLOS );
	
	if( flDot < BINOCULARS_VIEWCONE )
	{
		// Don't bother with sounds outside of a reasonable viewcone
		return BINOCULARS_FAKEDIST;
	}

	flDist = ( pSound->GetAbsOrigin() - pPlayer->GetAbsOrigin() ).Length();

	return flDist * (1.0 - flDot);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
CBaseEntity *CWeaponBinoculars::LocateBestSound( void )
{
	// go through the sound list and find the sound the player is probably trying to listen to.
	EHANDLE		*pIterate;
	CBaseEntity *pBestSound = NULL;
	float		flBestSoundDist;
	float		flTestDist;
	Vector		vecLOS;

	flBestSoundDist = BINOCULARS_FAKEDIST;
	pIterate = m_hSources;

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return NULL;

	pPlayer->EyeVectors( &vecLOS );

	while( *pIterate )
	{
		flTestDist = GetSoundDist( *pIterate, vecLOS );

		if( flTestDist < flBestSoundDist )
		{
			flBestSoundDist = flTestDist;
			pBestSound = *pIterate;
		}

		pIterate++;
	}

	return pBestSound;
}


//-----------------------------------------------------------------------------
// Purpose: Overloaded to handle the zoom functionality.
//-----------------------------------------------------------------------------
void CWeaponBinoculars::ItemPostFrame( void )
{
	if (cvar->FindVar("aa_cammode")->GetInt() == 1) //&& (m_bTpszoomRes))
	{
		if ( m_nZoomLevel != 0 )
		{
			Zoom( Zoom_Exit );
		}
		return;
	}

	/*if( gpGlobals->curtime >= m_NextCacheTime && m_nZoomLevel != 0 )
	{
		CacheSoundSources();
	}*/

	CBaseEntity *pSound;

	pSound = LocateBestSound();
	
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (pPlayer == NULL)
	{
		return;
	}

	float flStaticVolume;
	float flSignalVolume;
	float flDot;

	if( m_nZoomLevel != 0 )
	{
		Vector vecToSound;
		Vector vecFacing;

		pPlayer->EyeVectors( &vecFacing );

		if( pSound )
		{
			// Trying to tune a sound in.
			vecToSound = pSound->GetLocalOrigin() - pPlayer->GetLocalOrigin();
			VectorNormalize( vecToSound );

			flDot = DotProduct( vecToSound, vecFacing );

			/*
			// FIXME: Disabled this, the code for doing this must exist on the client
			if( flDot > 0.95 )
			{
				engine->EnableHOrigin( true );
				engine->SetHOrigin( pSound->GetLocalOrigin() );
			}
			else
			{
				engine->EnableHOrigin( false );
			}
			*/

			// static volume gets louder the farther you are from sound
			// signal volume gets louder as you near sound.
			flStaticVolume = ( 1.0 - flDot ) * 2.0;

			// signal volume gets louder as you near, quieter as you get VERY near.
			if( flDot < 0.90 || flDot > 0.9995 )
			{
				flSignalVolume = 0.1;
			}
			else
			{
				//flSignalVolume = (flDot - 0.6) * 2;
				flSignalVolume = flDot;
			}
		}
		else
		{
			// No sound. Static.
			flSignalVolume = 0.0;
			flStaticVolume = 1.0;
			
			//engine->EnableHOrigin( false );
		}
	}
	else
	{
		flStaticVolume = flSignalVolume = 0.0;

		cvar->FindVar("crosshair")->SetValue("1");
		cvar->FindVar("oc_weapon_binoculars_overlay_draw")->SetValue("0");
		//engine->EnableHOrigin( false );
	}

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	//controller.SoundChangeVolume( m_pSoundStatic, flStaticVolume, 0.1 );
	controller.SoundChangeVolume( m_pSoundSignal, flSignalVolume, 0.1 );

	if ( pPlayer->m_nButtons & IN_ATTACK )
	{
		if ( m_fNextZoom <= gpGlobals->curtime )
		{
			Zoom( Zoom_In );
			pPlayer->m_nButtons &= ~IN_ATTACK;
		}
	}
	else if ( pPlayer->m_nButtons & IN_ATTACK2 )
	{
		if ( m_fNextZoom <= gpGlobals->curtime )
		{
			Zoom( Zoom_Out );
			pPlayer->m_nButtons &= ~IN_ATTACK2;
		}
	}

	//
	// No buttons down.
	//
	if (!(( pPlayer->m_nButtons & IN_ATTACK ) || ( pPlayer->m_nButtons & IN_ATTACK2 )))
	{
		WeaponIdle();
		return;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBinoculars::Precache( void )
{
	//PrecacheSound( "weapons/binoculars/buzz.wav" );
	//PrecacheSound( "weapons/binoculars/signal.wav" );
	PrecacheSound( "weapons/airboat/airboat_gun_lastshot1.wav" );
	//PrecacheModel ("HUD/Binocular");//(cvar->FindVar("oc_weapon_binoculars_overlay")->GetString());

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Zooms in using the sniper rifle scope.
//-----------------------------------------------------------------------------
void CWeaponBinoculars::Zoom( ZoomMode_t eMode )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	//IMaterial *pMaterialZoom = materials->FindMaterial( ZOOM_Material, TEXTURE_GROUP_OTHER, true);
	//const Vector *vOrigin = &pPlayer->GetAbsOrigin(); 

	switch ( eMode )
	{
		//
		// Stop zooming with the binoculars.
		//
		case Zoom_Exit:
		{
			if ( m_nZoomLevel != 0 )
			{
				pPlayer->ShowViewModel(true);
				WeaponSound( SPECIAL2 );
				pPlayer->SetFOV( this, 0 );
				m_nZoomLevel = 0;

				cvar->FindVar("crosshair")->SetValue("1");
				cvar->FindVar("oc_weapon_binoculars_overlay_draw")->SetValue("0");
			}
			break;
		}

		//
		// Zoom in.
		//
		case Zoom_In:
		{
			if (( m_nZoomLevel + 1 ) < ( sizeof( g_nZoomFOV ) / sizeof( g_nZoomFOV[0] )))
			{
				m_nZoomLevel++;
				WeaponSound( SPECIAL1 );
				pPlayer->SetFOV( this, g_nZoomFOV[m_nZoomLevel] );

				if (g_nZoomFOV[m_nZoomLevel] != 0)
				{
					pPlayer->ShowViewModel(false);
				}
				cvar->FindVar("crosshair")->SetValue("0");
				cvar->FindVar("oc_weapon_binoculars_overlay_draw")->SetValue("1");
			}
			else
			{
				// Can't zoom in any further; play a special sound.
				WeaponSound( RELOAD );
			}

			m_fNextZoom = gpGlobals->curtime + BINOCULARS_ZOOM_RATE;
			break;
		}

		//
		// Zoom out.
		//
		case Zoom_Out:
		{
			if ( m_nZoomLevel > 0 )
			{
				m_nZoomLevel--;
				WeaponSound( SPECIAL2 );
				pPlayer->SetFOV( this, g_nZoomFOV[m_nZoomLevel] );

				if ( g_nZoomFOV[m_nZoomLevel] == 0 )
				{
					pPlayer->ShowViewModel(true);
				}
			}

			m_fNextZoom = gpGlobals->curtime + BINOCULARS_ZOOM_RATE;
			break;
		}

		default:
		{
			break;
		}
	}
}

void CWeaponBinoculars::Drop( const Vector &vecVelocity )
{
	Zoom( Zoom_Exit );
	BaseClass::Drop( vecVelocity );
}