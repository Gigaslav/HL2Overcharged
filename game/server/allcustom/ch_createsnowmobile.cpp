//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Spawn custom vehicles here
//
//===========================================================================//

#include "cbase.h"
//#include "const.h"
#include "baseplayer_shared.h"

#include "basecombatweapon.h"

#include "iservervehicle.h"
//#include "movevars_shared.h"
#include "vcollide_parse.h"
#include "player_command.h"
#include "vehicle_base.h"


static void CreateSnowMob( CBasePlayer *pPlayer )
{
	Vector vecForward;
	AngleVectors( pPlayer->EyeAngles(), &vecForward );
	CBaseEntity *pSnowMob = (CBaseEntity *)CreateEntityByName( "prop_vehicle_snowmobile" );
	if ( pSnowMob )
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 256 + Vector(0,0,64);
		QAngle vecAngles( 0, pPlayer->GetAbsAngles().y - 90, 0 );
		pSnowMob->SetAbsOrigin( vecOrigin );
		pSnowMob->SetAbsAngles( vecAngles );
		pSnowMob->KeyValue( "model", "models/snowmobile_remaster.mdl" );
		pSnowMob->KeyValue( "solid", "6" );
		pSnowMob->KeyValue( "targetname", "veh_snow" );	//jeep
		pSnowMob->KeyValue( "vehiclescript", "scripts/vehicles/snowmobile.txt" );
		pSnowMob->KeyValue( "skin", RandomInt(0, 5));
		DispatchSpawn( pSnowMob );
		pSnowMob->Activate();
		pSnowMob->Teleport( &vecOrigin, &vecAngles, NULL );
	}
}

void CC_CH_CreateSnowMob( void )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( !pPlayer )
		return;
	CreateSnowMob( pPlayer );
}

static ConCommand ch_createsnowmobile("ch_createsnowmobile", CC_CH_CreateSnowMob, "Spawn a snowmobile vehicle in front of the player.", FCVAR_CHEAT);
