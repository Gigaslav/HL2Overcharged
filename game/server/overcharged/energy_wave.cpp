//=========== © Copyright 2000 Valve, L.L.C. All rights reserved. =========== 
 // 
 // $Revision: $ 
 // $NoKeywords: $ 
 //============================================================================= 

 #include "cbase.h" 
 #include "energy_wave.h" 
 #include "in_buttons.h" 
 #include "gamerules.h" 
 #include "particle_parse.h"

 LINK_ENTITY_TO_CLASS( energy_wave, CEnergyWave ); 

 EXTERN_SEND_TABLE(DT_BaseEntity) 

 IMPLEMENT_SERVERCLASS_ST(CEnergyWave, DT_EWaveEffect) 
 END_SEND_TABLE() 

 //--------------------------------------------------------- 
 // Save/Restore 
 //--------------------------------------------------------- 
 BEGIN_DATADESC( CEnergyWave ) 
 END_DATADESC() 

 //----------------------------------------------------------------------------- 
 // Purpose: 
 //----------------------------------------------------------------------------- 
 void CEnergyWave::Precache( void ) 
 { 
 SetClassname("energy_wave"); 
 PrecacheParticleSystem( "houndeye_wave_01" );
 } 

 //----------------------------------------------------------------------------- 
 // Purpose: 
 //----------------------------------------------------------------------------- 
 void CEnergyWave::Spawn( void ) 
 { 
 Precache(); 

 // Make it translucent 
 m_nRenderFX = kRenderTransAlpha; 
 SetRenderColorA( 255 ); 

 SetSolid( SOLID_BBOX ); 
 SetMoveType( MOVETYPE_NONE ); 
 UTIL_SetSize( this, vec3_origin, vec3_origin ); 
 
 DispatchParticleEffect( "houndeye_wave_01", GetAbsOrigin(), RandomAngle( 0, 360 ) );

 // Think function 
 SetNextThink( gpGlobals->curtime + 0.1f ); 
 } 

 //----------------------------------------------------------------------------- 
 // Purpose: Create an energy wave 
 //----------------------------------------------------------------------------- 

 CEnergyWave* CEnergyWave::Create( CBaseEntity *pentOwner ) 
 { 
 CEnergyWave *pEWave = (CEnergyWave*)CreateEntityByName("energy_wave"); 

 UTIL_SetOrigin( pEWave, pentOwner->GetLocalOrigin() ); 
 pEWave->SetOwnerEntity( pentOwner ); 
 pEWave->SetLocalAngles( pentOwner->GetLocalAngles() ); 
 pEWave->Spawn(); 

 return pEWave; 
 }