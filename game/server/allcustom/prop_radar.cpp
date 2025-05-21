#include "cbase.h"

#include "hl2_vehicle_radar.h"
#include "vehicle_jeep_episodic.h"
//#include "vehicle_jeep_episodic.cpp"
 
class CPropRadar : public CBaseAnimating
{
public:
	DECLARE_CLASS( CPropRadar, CBaseAnimating );
	DECLARE_DATADESC();
 
	CPropRadar()
	{
		m_bActive = false;
	}
 
	void Spawn( void );
	void Precache( void );
 
	void MoveThink( void );
 
	// Input function
	void InputToggle( inputdata_t &inputData );
 
private:
 
	bool	m_bActive;
	float	m_flNextChangeTime;

	// ADDED
	//void	InputEnableRadar( inputdata_t &data );
	//void	InputDisableRadar( inputdata_t &data );
};

LINK_ENTITY_TO_CLASS( prop_radar, CPropRadar );
 
// Start of our data description for the class
BEGIN_DATADESC( CPropRadar )
 
	// Save/restore our active state
	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flNextChangeTime, FIELD_TIME ),
 
	// Links our input name from Hammer to our input member function
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
 
	// Declare our think function
	DEFINE_THINKFUNC( MoveThink ),
 
END_DATADESC()

// Name of our entity's model
//#define	ENTITY_MODEL	"models/gibs/airboat_broken_engine.mdl"
//#define	ENTITY_MODEL	"models/vehicle_radar.mdl"
#define	ENTITY_MODEL	"models/props_junk/garbage_bag001a.mdl"

//-----------------------------------------------------------------------------
// Purpose: Precache assets used by the entity
//-----------------------------------------------------------------------------
void CPropRadar::Precache( void )
{
	PrecacheModel( ENTITY_MODEL );
 
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CPropRadar::Spawn( void )
{
	Precache();
 
	SetModel( ENTITY_MODEL );

	SetSolid( SOLID_VPHYSICS );
	//SetSolid( SOLID_BBOX );
	//UTIL_SetSize( this, -Vector(20,20,20), Vector(20,20,20) );

	/*// Select the scanner's idle sequence
	SetSequence( LookupSequence("idle") );	// L1ght 15:	default animation name on spawn
	// Set the animation speed to 100%
	SetPlaybackRate( 1.0f );
	// Tell the client to animate this model
	UseClientSideAnimation();*/

	//BaseClass::Spawn();
	//SetMoveType(MOVETYPE_VPHYSICS);
}

//-----------------------------------------------------------------------------
// Purpose: Think function to randomly move the entity
//-----------------------------------------------------------------------------
void CPropRadar::MoveThink( void )
{
	// See if we should change direction again
	if ( m_flNextChangeTime < gpGlobals->curtime )
	{
		// Randomly take a new direction and speed
		Vector vecNewVelocity = RandomVector( -64.0f, 64.0f );
		SetAbsVelocity( vecNewVelocity );
 
		// Randomly change it again within one to three seconds
		m_flNextChangeTime = gpGlobals->curtime + random->RandomFloat( 1.0f, 3.0f );
	}
 
	// Snap our facing to where we're heading
	Vector velFacing = GetAbsVelocity();
	QAngle angFacing;
	VectorAngles( velFacing, angFacing );
 	SetAbsAngles( angFacing );
 
	// Think at 20Hz
	SetNextThink( gpGlobals->curtime + 0.05f );
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the movement of the entity
//-----------------------------------------------------------------------------
void CPropRadar::InputToggle( inputdata_t &inputData )
{
	// Toggle our active state
	if ( !m_bActive )
	{
		// Start thinking
		SetThink( &CPropRadar::MoveThink );
 
		SetNextThink( gpGlobals->curtime + 0.05f );
 
		// Start moving
		SetMoveType( MOVETYPE_FLY );
 
		// Force MoveThink() to choose a new speed and direction immediately
		m_flNextChangeTime = gpGlobals->curtime;
 
		// Update m_bActive to reflect our new state
		m_bActive = true;
	}
	else
	{
		// Stop thinking
		SetThink( NULL );
 
		// Stop moving
		SetAbsVelocity( vec3_origin );
 		SetMoveType( MOVETYPE_NONE );
 
		m_bActive = false;
	}
}

//======================ADDED
/*
//-----------------------------------------------------------------------------
// Purpose: Turn on the Jalopy radar device
//-----------------------------------------------------------------------------
void CPropRadar::InputEnableRadar( inputdata_t &data )
{
	if( m_bRadarEnabled )
		return; // Already enabled

	//SetBodygroup( JEEP_RADAR_BODYGROUP, 1 );

	SpawnRadarPanel();
}

//-----------------------------------------------------------------------------
// Purpose: Turn off the Jalopy radar device
//-----------------------------------------------------------------------------
void CPropRadar::InputDisableRadar( inputdata_t &data )
{
	if( !m_bRadarEnabled )
		return; // Already disabled

	//SetBodygroup( JEEP_RADAR_BODYGROUP, 0 );

	DestroyRadarPanel();
}*/

// Console command
CON_COMMAND(create_prop_radar, "Create prop_radar in front of the player.")
{
	Vector vecForward;
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if (!pPlayer)
	{
		//Warning("Could not determine calling player!\n");
		return;
	}
 
	AngleVectors( pPlayer->EyeAngles(), &vecForward );
	CBaseEntity *pEnt = CreateEntityByName( "prop_radar" );
	if ( pEnt )
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 128 /*256*/ + Vector(0,0,64);
		QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 90, 0);
		pEnt->SetAbsOrigin(vecOrigin);
		pEnt->SetAbsAngles(vecAngles);
		DispatchSpawn(pEnt);
	}
}