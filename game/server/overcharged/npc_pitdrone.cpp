//========= Overcharged 2019. ============//
//
// Purpose: Pit Drone NPC
//	
//	Звуки адаптированы и полностью работают. Что осталось?
//
//	У меня MEAL функция в этом билде не заработала, я объявил Communicate звук в фрагменте твоего кода но код так и не прочитался
//	В прекеше уже всё есть, нужно активировать 1 звук в твоих функциях
//PrecacheScriptSound("NPC_Pitdrone.Communicate");	// Этот звук должен воспроизвестить 1 раз когда дрон нашёл добычу (регдолл)
// см if (!foundedMeal)
//
// В этой функции в условии смены приоритета на хедкраба игра вылетает если дрон был убит в этот момент, нужен фикс
//NPC_STATE CNPC_PitDrone::SelectIdealState(void)
//	(в других случаях вылет происходит в базовом классе ai_basenpc_schedule.cpp)

// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "game.h"
#include "AI_Default.h"
#include "AI_Schedule.h"
#include "AI_Hull.h"
#include "AI_Route.h"
#include "AI_Hint.h"
#include "AI_Navigator.h"
#include "AI_Senses.h"
#include "NPCEvent.h"
#include "animation.h"
#include "npc_pitdrone.h"
#include "gib.h"
#include "soundent.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "grenade_spit.h"
#include "util.h"
#include "shake.h"
#include "movevars_shared.h"
#include "decals.h"
#include "hl2_shareddefs.h"
#include "ammodef.h"
#include "particle_parse.h" // for particle attack and for oil sprays
#include "te_particlesystem.h" // for particle attack and for oil sprays
#include "te_effect_dispatch.h" // for particle attack and for oil sprays

#define		PDRONE_SPRINT_DIST	256 // how close the squid has to get before starting to sprint and refusing to swerve

ConVar sk_pitdrone_health("sk_pitdrone_health", "0");
ConVar sk_pitdrone_min_health("sk_pitdrone_min_health", "0");
ConVar sk_pitdrone_eat_health("sk_pitdrone_eat_health", "0");
ConVar sk_pitdrone_dmg_bite("sk_pitdrone_dmg_bite", "0");
ConVar sk_pitdrone_dmg_whip("sk_pitdrone_dmg_whip", "0");
ConVar sk_pitdrone_dmg_spike("sk_pitdrone_dmg_spike", "0");
ConVar sk_pitdrone_meal_search_radius("sk_pitdrone_meal_search_radius", "0");
//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_PDRONE_HURTHOP = LAST_SHARED_SCHEDULE,
	SCHED_PDRONE_SEECRAB,
	SCHED_PDRONE_EAT,
	SCHED_PDRONE_SNIFF_AND_EAT,
	SCHED_PDRONE_WALLOW,
	SCHED_PDRONE_CHASE_ENEMY,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_PDRONE_HOPTURN = LAST_SHARED_TASK,
	TASK_PDRONE_EAT,
};

//-----------------------------------------------------------------------------
// Squid Conditions
//-----------------------------------------------------------------------------
enum
{
	COND_PDRONE_SMELL_FOOD	= LAST_SHARED_CONDITION + 1,
};


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		BPDRONE_AE_SPIT		( 1 )
#define		BPDRONE_AE_BITE		( 2 )
#define		BPDRONE_AE_BLINK		( 3 )
#define		BPDRONE_AE_TAILWHIP	( 4 )
#define		BPDRONE_AE_HOP		( 5 )
#define		BPDRONE_AE_THROW		( 6 )
#define		BPDRONE_AE_RELOAD		( 7 )

LINK_ENTITY_TO_CLASS( npc_pitdrone, CNPC_PitDrone );

int ACT_PDRONE_EXCITED;
int ACT_PDRONE_EAT;
int ACT_PDRONE_DETECT_SCENT;
int ACT_PDRONE_INSPECT_FLOOR;

//=========================================================
// Bullsquid's spit projectile
//=========================================================
class CPitSpike : public CBaseCombatCharacter
{
	DECLARE_CLASS(CPitSpike, CBaseCombatCharacter);
public:
	void Spawn( void );
	void Precache( void );

	static void Shoot( CBaseEntity *pOwner, Vector vecStart, Vector vecVelocity );
	void Touch( CBaseEntity *pOther );
	void Animate( void );

	//int m_nSquidSpitSprite;

	DECLARE_DATADESC();

	void SetSprite( CBaseEntity *pSprite )
	{
		m_hSprite = pSprite;	
	}

	CBaseEntity *GetSprite( void )
	{
		return m_hSprite.Get();
	}

private:
	EHANDLE m_hSprite;


};

LINK_ENTITY_TO_CLASS( pitspike, CPitSpike );

BEGIN_DATADESC( CPitSpike )
	//DEFINE_FIELD( m_nSquidSpitSprite, FIELD_INTEGER ),
	DEFINE_FIELD( m_hSprite, FIELD_EHANDLE ),
END_DATADESC()


void CPitSpike::Precache( void )
{
	//m_nSquidSpitSprite = PrecacheModel("sprites/bigspit.vmt");// client side spittle.
	PrecacheModel("models/racex/pit_drone_spike.mdl");
	PrecacheScriptSound( "NPC_Pitdrone.SpikeTouch" );	//spittouch1
	PrecacheScriptSound( "NPC_Pitdrone.SpikeHit1" );		//spithit1
	PrecacheScriptSound( "NPC_Pitdrone.SpikeHit2" );		//spithit2
	PrecacheParticleSystem("Spore_launcher_muzzle_flash_main");
}

void CPitSpike:: Spawn( void )
{
	Precache();

	SetMoveType ( MOVETYPE_FLY );
	//SetClassname( "squidspit" );
	
	SetSolid( SOLID_BBOX );

	//m_nRenderMode = kRenderTransAlpha;
	//SetRenderColorA( 255 );
	SetModel( "models/racex/pit_drone_spike.mdl" );
	
	//SetSprite( CSprite::SpriteCreate( "sprites/bigspit.vmt", GetAbsOrigin(), true ) );
	
	UTIL_SetSize( this, Vector( -5, -5, -5), Vector(5, 5, 5) );

	SetCollisionGroup( HL2COLLISION_GROUP_SPIT );
}

void CPitSpike::Shoot( CBaseEntity *pOwner, Vector vecStart, Vector vecVelocity )
{
	CPitSpike *pSpit = CREATE_ENTITY( CPitSpike, "squidspit" );
	pSpit->Spawn();
	
	UTIL_SetOrigin( pSpit, vecStart );
	pSpit->SetAbsVelocity( vecVelocity );
	pSpit->SetOwnerEntity( pOwner );

	/*CSprite *pSprite = (CSprite*)pSpit->GetSprite();

	if ( pSprite )
	{
		pSprite->SetAttachment( pSpit, 0 );
		pSprite->SetOwnerEntity( pSpit );

		pSprite->SetScale( 0.5 );
		pSprite->SetTransparency( pSpit->m_nRenderMode, pSpit->m_clrRender->r, pSpit->m_clrRender->g, pSpit->m_clrRender->b, pSpit->m_clrRender->a, pSpit->m_nRenderFX );
	}


	CPVSFilter filter( vecStart );

	VectorNormalize( vecVelocity );
	te->SpriteSpray( filter, 0.0, &vecStart , &vecVelocity, pSpit->m_nSquidSpitSprite, 210, 25, 15 );*/
}

void CPitSpike::Touch ( CBaseEntity *pOther )
{
	trace_t tr;
	int		iPitch;

	if ( pOther->GetSolidFlags() & FSOLID_TRIGGER )
		 return;

	if ( pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
	{
		return;
	}

	// splat sound
	iPitch = random->RandomFloat( 90, 110 );

	EmitSound( "NPC_Pitdrone.SpikeTouch" );

	switch ( random->RandomInt( 0, 1 ) )
	{
	case 0:
		EmitSound( "NPC_Pitdrone.SpikeHit1" );
		break;
	case 1:
		EmitSound( "NPC_Pitdrone.SpikeHit2" );
		break;
	}

	if ( !pOther->m_takedamage )
	{
		// make a splat on the wall
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 10, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
		UTIL_DecalTrace(&tr, "Impact.Concrete");
		//UTIL_DecalTrace(&tr, "BeerSplash" );

		// make some flecks
		//CPVSFilter filter( tr.endpos );

		//te->SpriteSpray( filter, 0.0,	&tr.endpos, &tr.plane.normal, m_nSquidSpitSprite, 30, 8, 5 );

	}
	else
	{
		CTakeDamageInfo info(this, this, sk_pitdrone_dmg_spike.GetFloat(), DMG_BULLET);
		CalculateBulletDamageForce( &info, GetAmmoDef()->Index("9mmRound"), GetAbsVelocity(), GetAbsOrigin() );
		pOther->TakeDamage( info );
	}

	UTIL_Remove( m_hSprite );
	UTIL_Remove( this );
}


BEGIN_DATADESC( CNPC_PitDrone )
	DEFINE_FIELD( m_fCanThreatDisplay, FIELD_BOOLEAN ),
	DEFINE_FIELD( foundedMeal, FIELD_BOOLEAN ),
	DEFINE_FIELD(waitforeating, FIELD_BOOLEAN),
	DEFINE_FIELD( stop, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLastHurtTime, FIELD_TIME ),
	DEFINE_FIELD( m_flNextSpitTime, FIELD_TIME ),
	DEFINE_FIELD(m_NumShotsFired, FIELD_INTEGER),
	DEFINE_FIELD( m_flHungryTime, FIELD_TIME ),
END_DATADESC()

//=========================================================
// Spawn
//=========================================================
void CNPC_PitDrone::Spawn()
{
	Precache( );

	SetModel("models/RaceX/pit_drone.mdl");
	SetHullType(HULL_WIDE_HUMAN);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	m_bloodColor		= BLOOD_COLOR_GREEN;
	
	SetRenderColor( 255, 255, 255, 255 );
	m_NumShotsFired = 6;
	m_iHealth = sk_pitdrone_health.GetFloat();
	m_flFieldOfView		= 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_NPCState			= NPC_STATE_NONE;
	
	CapabilitiesClear();
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_INNATE_MELEE_ATTACK2 );
	
	m_fCanThreatDisplay	= TRUE;
	m_flNextSpitTime = gpGlobals->curtime;

	NPCInit();
	SetActivity(ACT_IDLE);
	// OVERCHARGED: По умолчанию группа шипов дрона скрыта, активируем её при спавне.
	SetBodygroup( 1, 6);	// 1 - группа шипов // 2 - СЕДЬМАЯ ячейка с ШЕСТЬЮ шипами
	stop = false;
	foundedMeal = false;
	m_flDistTooFar		= 784;
	waitforeating = false;
	Q_snprintf(pMealName, sizeof(pMealName), "prop_ragdoll");
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_PitDrone::Precache()
{
	BaseClass::Precache();
	
	PrecacheModel("models/RaceX/pit_drone.mdl");
	
	//PrecacheModel("sprites/bigspit.vmt");	// cleanup

	PrecacheScriptSound( "NPC_Pitdrone.Idle" );
	PrecacheScriptSound( "NPC_Pitdrone.Pain" );
	PrecacheScriptSound( "NPC_Pitdrone.Alert" );
	PrecacheScriptSound( "NPC_Pitdrone.Die" );
	PrecacheScriptSound( "NPC_Pitdrone.MeleeAttackSingle" ); //growl
	PrecacheScriptSound( "NPC_Pitdrone.MeleeAttackDouble" );	//bite
	PrecacheScriptSound( "NPC_Pitdrone.SpikeAttack" );	//attack1

	// Custom from op4
	PrecacheScriptSound("NPC_Pitdrone.Communicate");	// звук нахождения добычи (регдолла)	// есть в коде, не активен см 	if (!foundedMeal)
	PrecacheScriptSound("NPC_Pitdrone.Hunt");	// звук при беге	// есть в QC, в код добавлять не нужно
	PrecacheScriptSound("NPC_Pitdrone.Eat");	// звук поедания	// есть в QC, в код добавлять не нужно
}


int CNPC_PitDrone::TranslateSchedule( int scheduleType )
{	
	switch	( scheduleType )
	{
		case SCHED_CHASE_ENEMY:
			return SCHED_PDRONE_CHASE_ENEMY;
			break;
	}

	return BaseClass::TranslateSchedule( scheduleType );
}

//-----------------------------------------------------------------------------
// Purpose: Indicates this monster's place in the relationship table.
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_PitDrone::Classify( void )
{
	return CLASS_RACEX;		// BriJee: Our own Race X class
}

//=========================================================
// IdleSound 
//=========================================================
#define PDRONE_ATTN_IDLE	(float)1.5
void CNPC_PitDrone::IdleSound( void )
{
	CPASAttenuationFilter filter( this, PDRONE_ATTN_IDLE );
	EmitSound( filter, entindex(), "NPC_Pitdrone.Idle" );	
}

//=========================================================
// PainSound 
//=========================================================
void CNPC_PitDrone::PainSound( const CTakeDamageInfo &info )
{
	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), "NPC_Pitdrone.Pain" );	
}

//=========================================================
// AlertSound
//=========================================================
void CNPC_PitDrone::AlertSound( void )
{
	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), "NPC_Pitdrone.Alert" );
}

//=========================================================
// DeathSound
//=========================================================
void CNPC_PitDrone::DeathSound( const CTakeDamageInfo &info )
{
	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), "NPC_Pitdrone.Die" );	
}

//=========================================================
// AttackSound
//=========================================================
void CNPC_PitDrone::AttackSound( void ) // BriJee: Used for spike attack
{
	CPASAttenuationFilter filter( this );
	EmitSound( filter, entindex(), "NPC_Pitdrone.SpikeAttack" );	//attack1	
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
float CNPC_PitDrone::MaxYawSpeed( void )
{
	float flYS = 0;

	switch ( GetActivity() )
	{
	case	ACT_WALK:			flYS = 90;	break;
	case	ACT_RUN:			flYS = 90;	break;
	case	ACT_IDLE:			flYS = 90;	break;
	case	ACT_RANGE_ATTACK1:	flYS = 90;	break;
	default:
		flYS = 90;
		break;
	}

	return flYS;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CNPC_PitDrone::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
		case BPDRONE_AE_RELOAD:
		{
			//m_NumShotsFired = 6;
			DevMsg("BPDRONE_AE_RELOAD \n");
			SetBodygroup(1, m_NumShotsFired);
		}
		break;
		case BPDRONE_AE_SPIT:
		{

			// OVERCHARGED:
			// Тут объявлена атака плевка у булсквида, мы её адаптируем под наши нужды

			// ИВЕНТ ПЕРЕЗАРЯДКИ ШИПОВ
			// if SpikeCount > 5 (можно иначе поставить условие)	// если выстрелено шесть шипов то меняем условие
			//Проигрываем анимацию перезарядки, в QC есть анимация reload

			//{
			//				// будущий ивент для перезарядки шипов
			//				SpikeCount = 0	// обнуляем счётчик после перезарядки
			//				SetBodygroup( 1, 6 );	// 1 - бодигруппа шипов // 6 - полный комплект шипов ставим обратно	// СЕДЬМАЯ бодигруппа с ШЕСТЬЮ шипами так как нумерация с нуля идёт в QC и моделях на сорсе в целом
			//}

			//else	// иначе стандартный код снизу

			if (!waitforeating && GetEnemy())
			{
				Vector	vecSpitOffset;
				Vector	vecSpitDir;
				Vector  vRight, vUp, vForward;

				AngleVectors ( GetAbsAngles(), &vForward, &vRight, &vUp );

				// !!!HACKHACK - the spot at which the spit originates (in front of the mouth) was measured in 3ds and hardcoded here.
				// we should be able to read the position of bones at runtime for this info.
				vecSpitOffset = ( vRight * 8 + vForward * 60 + vUp * 50 );		
				vecSpitOffset = ( GetAbsOrigin() + vecSpitOffset );
				vecSpitDir = ( ( GetEnemy()->BodyTarget( GetAbsOrigin() ) ) - vecSpitOffset );

				VectorNormalize( vecSpitDir );

				vecSpitDir.x += random->RandomFloat( -0.05, 0.05 );
				vecSpitDir.y += random->RandomFloat( -0.05, 0.05 );
				vecSpitDir.z += random->RandomFloat( -0.05, 0 );

				// OVERCHARGED:
				// Лучше вместо координат взять новый аттачмент с именем 'shoot' чтобы шип вылетал строго откуда должен, координаты я сделал
				// Я так же потестил opposing force 2, у них шип вылетает криво над моделью
						
				AttackSound();

				// OVERCHARGED:
				// В стандартной функции повышаем счётчик пока не станет равен 6 и более (с каждым выстрелом +1)
				// так же с каждым выстрелом меняем бодигруппу модели чтобы шипов отображалось меньше с каждым разом

				//	if SpikeCount = 1	// считаем кол во выстрелов 
				//	SetBodygroup( 1, 5 );	// 1 - бодигруппа шипы // 5 - ШЕСТАЯ бодигруппа с ПЯТЬЮ шипами (так как изначально 6, вы убираем один по порядку)
				
				//	if SpikeCount = 2	// считаем кол во выстрелов 
				//	SetBodygroup( 1, 4 );	// 1 - бодигруппа шипы // 5 - ПЯТАЯ бодигруппа с 4 шипами
				
				//	if SpikeCount = 3	// считаем кол во выстрелов 
				//	SetBodygroup( 1, 3 );	// 1 - бодигруппа шипы // 5 - 4 бодигруппа с 3 шипами
				
				//	if SpikeCount = 4	// считаем кол во выстрелов 
				//	SetBodygroup( 1, 2 );	// 1 - бодигруппа шипы // 5 - 3 бодигруппа с 2 шипами
				
				//	if SpikeCount = 5	// считаем кол во выстрелов 
				//	SetBodygroup( 1, 1 );	// 1 - бодигруппа шипы // 5 - 2 бодигруппа с 1 шипом

				//	if SpikeCount = 6	// считаем кол во выстрелов, последний выстрел
				//	SetBodygroup( 1, 0 );	// 1 - бодигруппа шипы // 5 - 1 бодигруппа без модели шипов

				//SpikeCount +1
			
				//CPitSpike::Shoot( this, vecSpitOffset, vecSpitDir * 900 );
				DevMsg("GetBodygroup: %i \n", GetBodygroup(1));
				m_NumShotsFired--;
				SetBodygroup(1, m_NumShotsFired);


				if (m_NumShotsFired < 1)
				{
					m_NumShotsFired = 6;
					SetActivity(ACT_RELOAD);
				}

				Vector Shoot;
				GetAttachment("shoot", Shoot);
				CPitSpike *pGrenade = (CPitSpike*)CreateEntityByName("pitspike");
				pGrenade->SetAbsOrigin(Shoot);
				QAngle ang;
				VectorAngles(vecSpitDir, ang);
				pGrenade->SetAbsAngles(ang);
				//pGrenade->SetAbsAngles(vec3_angle);
				DispatchSpawn(pGrenade);
				//pGrenade->SetThrower(this);
				pGrenade->SetOwnerEntity(this);
				pGrenade->SetAbsVelocity(vecSpitDir * 900);
				// Tumble through the air
				//pGrenade->SetLocalAngularVelocity(QAngle(random->RandomFloat(-250, -500), random->RandomFloat(-250, -500), random->RandomFloat(-250, -500)));
				DispatchParticleEffect("Spore_launcher_muzzle_flash_main", PATTACH_POINT_FOLLOW, this, "shoot", false);

				//DispatchParticleEffect("Spore_launcher_muzzle_flash_main", vecSpitDir * 1.3, GetAbsAngles());


			}
		}
		break;

		case BPDRONE_AE_BITE:
		{
		// SOUND HERE!
			CBaseEntity *pHurt = CheckTraceHullAttack(70, Vector(-16, -16, -16), Vector(16, 16, 16), sk_pitdrone_dmg_bite.GetFloat(), DMG_SLASH);
			if ( pHurt )
			{
				Vector forward, up;
				AngleVectors( GetAbsAngles(), &forward, NULL, &up );
				pHurt->SetAbsVelocity( pHurt->GetAbsVelocity() + (forward * random->RandomInt(70,120)) );
				if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
					pHurt->ViewPunch(QAngle(random->RandomInt(10, 30), 0, 0));
				//pHurt->SetAbsVelocity( pHurt->GetAbsVelocity() + (up * 100) );
				pHurt->SetGroundEntity( NULL );
			}
		}
		break;

		case BPDRONE_AE_TAILWHIP:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack(70, Vector(-16, -16, -16), Vector(16, 16, 16), sk_pitdrone_dmg_whip.GetFloat(), DMG_SLASH | DMG_ALWAYSGIB);
			if ( pHurt ) 
			{
				Vector right, up;
				AngleVectors( GetAbsAngles(), NULL, &right, &up );

				if ( pHurt->GetFlags() & ( FL_NPC | FL_CLIENT ) )
					pHurt->ViewPunch(QAngle(random->RandomInt(-10, 10), random->RandomInt(-10, 10), random->RandomInt(10, -10)));
				int i = random->RandomInt(1,2);
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + (right * random->RandomInt(50, 70) * ((i == 1) ? 1 : -1)));
				//pHurt->SetAbsVelocity( pHurt->GetAbsVelocity() + (up * 100) );
			}
		}
		break;

		case BPDRONE_AE_BLINK:
		{
			// close eye. 
			m_nSkin = 1;
		}
		break;

		case BPDRONE_AE_HOP:
		{
			float flGravity = sv_gravity.GetFloat();

			// throw the squid up into the air on this frame.
			if ( GetFlags() & FL_ONGROUND )
			{
				SetGroundEntity( NULL );
			}

			// jump into air for 0.8 (24/30) seconds
			Vector vecVel = GetAbsVelocity();
			int i = random->RandomInt(1, 2);
			vecVel.z += (0.625 * flGravity) * 0.5 * ((i == 1) ? 1 : -1);
			SetAbsVelocity( vecVel );
		}
		break;

		case BPDRONE_AE_THROW:
			{
				// squid throws its prey IF the prey is a client. 
				CBaseEntity *pHurt = CheckTraceHullAttack( 70, Vector(-16,-16,-16), Vector(16,16,16), 0, 0 );


				if ( pHurt )
				{
					// croonchy bite sound
					CPASAttenuationFilter filter( this );
					EmitSound( filter, entindex(), "NPC_Pitdrone.MeleeAttackSingle" );	// bite
					//Msg("Drone bite \n");

					// screeshake transforms the viewmodel as well as the viewangle. No problems with seeing the ends of the viewmodels.
					UTIL_ScreenShake( pHurt->GetAbsOrigin(), 25.0, 1.5, 0.7, 2, SHAKE_START );

					if ( pHurt)//->IsPlayer() )
					{
						Vector forward, up;
						AngleVectors( GetAbsAngles(), &forward, NULL, &up );
				
						pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() + forward * random->RandomInt(50, 90)/* + up * 300 */);
					}
				}
			}
		break;

		default:
			BaseClass::HandleAnimEvent( pEvent );
	}
}

int CNPC_PitDrone::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( IsMoving() && flDist >= 512 )
	{
		// squid will far too far behind if he stops running to spit at this distance from the enemy.
		return ( COND_NONE );
	}

	if ( flDist > 85 && flDist <= 784 && flDot >= 0.5 && gpGlobals->curtime >= m_flNextSpitTime )
	{
		if (!waitforeating && GetEnemy() != NULL)
		{
			if ( fabs( GetAbsOrigin().z - GetEnemy()->GetAbsOrigin().z ) > 256 )
			{
				// don't try to spit at someone up really high or down really low.
				return( COND_NONE );
			}
		}

		if ( IsMoving() )
		{
			// don't spit again for a long time, resume chasing enemy.
			m_flNextSpitTime = gpGlobals->curtime + 5;
		}
		else
		{
			// not moving, so spit again pretty soon.
			m_flNextSpitTime = gpGlobals->curtime + 0.5;
		}

		return( COND_CAN_RANGE_ATTACK1 );
	}

	return( COND_NONE );
}

//=========================================================
// MeleeAttack2Conditions - bullsquid is a big guy, so has a longer
// melee range than most monsters. This is the tailwhip attack
//=========================================================
int CNPC_PitDrone::MeleeAttack1Conditions( float flDot, float flDist )
{
	if (!waitforeating && GetEnemy()->m_iHealth <= sk_pitdrone_dmg_whip.GetFloat() && flDist <= 65 && flDot >= 0.7)
	{
		return ( COND_CAN_MELEE_ATTACK1 );
	}
	
	return( COND_NONE );
}

//=========================================================
// MeleeAttack2Conditions - bullsquid is a big guy, so has a longer
// melee range than most monsters. This is the bite attack.
// this attack will not be performed if the tailwhip attack
// is valid.
//=========================================================
int CNPC_PitDrone::MeleeAttack2Conditions( float flDot, float flDist )
{
	if ( flDist <= 85 && flDot >= 0.7 && !HasCondition( COND_CAN_MELEE_ATTACK1 ) )		// The player & bullsquid can be as much as their bboxes 
		 return ( COND_CAN_MELEE_ATTACK2 );
	
	return( COND_NONE );
}

bool CNPC_PitDrone::FValidateHintType ( CAI_Hint *pHint )
{
	if ( pHint->HintType() == HINT_HL1_WORLD_HUMAN_BLOOD )
		 return true;

	Msg ( "Couldn't validate hint type" );

	return false;
}

void CNPC_PitDrone::RemoveIgnoredConditions( void )
{
	if ( m_flHungryTime > gpGlobals->curtime )
		 ClearCondition( COND_PDRONE_SMELL_FOOD );

	if ( gpGlobals->curtime - m_flLastHurtTime <= 20 )
	{
		// haven't been hurt in 20 seconds, so let the squid care about stink. 
		ClearCondition( COND_SMELL );
	}

	if (!waitforeating && GetEnemy() != NULL)
	{
		// ( Unless after a tasty headcrab, yumm ^_^ )
		if ( FClassnameIs( GetEnemy(), "npc_headcrab" ) )
			 ClearCondition( COND_SMELL );
	}
}

Disposition_t CNPC_PitDrone::IRelationType( CBaseEntity *pTarget )
{
	if ( gpGlobals->curtime - m_flLastHurtTime < 5 && FClassnameIs ( pTarget, "npc_headcrab" ) )
	{
		// if squid has been hurt in the last 5 seconds, and is getting relationship for a headcrab, 
		// tell squid to disregard crab. 
		return D_NU;
	}

	return BaseClass::IRelationType ( pTarget );
}

//=========================================================
// TakeDamage - overridden for bullsquid so we can keep track
// of how much time has passed since it was last injured
//=========================================================
int CNPC_PitDrone::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{

#if 0 //Fix later.

	float flDist;
	Vector vecApex, vOffset;

	// if the squid is running, has an enemy, was hurt by the enemy, hasn't been hurt in the last 3 seconds, and isn't too close to the enemy,
	// it will swerve. (whew).
	if ( GetEnemy() != NULL && IsMoving() && pevAttacker == GetEnemy() && gpGlobals->curtime - m_flLastHurtTime > 3 )
	{
		flDist = ( GetAbsOrigin() - GetEnemy()->GetAbsOrigin() ).Length2D();
		
		if ( flDist > PDRONE_SPRINT_DIST )
		{
			AI_Waypoint_t*	pRoute = GetNavigator()->GetPath()->Route();

			if ( pRoute )
			{
				flDist = ( GetAbsOrigin() - pRoute[ pRoute->iNodeID ].vecLocation ).Length2D();// reusing flDist. 

				if ( GetNavigator()->GetPath()->BuildTriangulationRoute( GetAbsOrigin(), pRoute[ pRoute->iNodeID ].vecLocation, flDist * 0.5, GetEnemy(), &vecApex, &vOffset, NAV_GROUND ) )
				{
					GetNavigator()->PrependWaypoint( vecApex, bits_WP_TO_DETOUR | bits_WP_DONT_SIMPLIFY );
				}
			}
		}
	}
#endif

	if ( !FClassnameIs ( inputInfo.GetAttacker(), "npc_headcrab" ) )
	{
		// don't forget about headcrabs if it was a headcrab that hurt the squid.
		m_flLastHurtTime = gpGlobals->curtime;
	}

	return BaseClass::OnTakeDamage_Alive ( inputInfo );
}

//=========================================================
// GetSoundInterests - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
//=========================================================
int CNPC_PitDrone::GetSoundInterests ( void )
{
	return	SOUND_WORLD	|
			SOUND_COMBAT	|
		    SOUND_CARCASS	|
			SOUND_MEAT		|
			SOUND_GARBAGE	|
			SOUND_PLAYER;
}

//=========================================================
// OnListened - monsters dig through the active sound list for
// any sounds that may interest them. (smells, too!)
//=========================================================
void CNPC_PitDrone::OnListened( void )
{
	AISoundIter_t iter;
	
	CSound *pCurrentSound;

	static int conditionsToClear[] = 
	{
		COND_PDRONE_SMELL_FOOD,
	};

	ClearConditions( conditionsToClear, ARRAYSIZE( conditionsToClear ) );
	
	pCurrentSound = GetSenses()->GetFirstHeardSound( &iter );
	
	while ( pCurrentSound )
	{
		// the npc cares about this sound, and it's close enough to hear.
		int condition = COND_NONE;
		
		if ( !pCurrentSound->FIsSound() )
		{
			// if not a sound, must be a smell - determine if it's just a scent, or if it's a food scent
			if ( pCurrentSound->IsSoundType( SOUND_MEAT | SOUND_CARCASS ) )
			{
				// the detected scent is a food item
				condition = COND_PDRONE_SMELL_FOOD;
			}
		}
		
		if ( condition != COND_NONE )
			SetCondition( condition );

		pCurrentSound = GetSenses()->GetNextHeardSound( &iter );
	}

	BaseClass::OnListened();
}

CBaseEntity *pMeal;
//========================================================
// RunAI - overridden for bullsquid because there are things
// that need to be checked every think.
//========================================================
void CNPC_PitDrone::RunAI ( void )
{
	// first, do base class stuff
	BaseClass::RunAI();

	if (GetActivity() == ACT_RELOAD)
	{
		//Исправлен вылет, вызвращается исполнение метода из базового класса
		return BaseClass::RunAI();
	}
	if ( m_nSkin != 0 )
	{
		// close eye if it was open.
		m_nSkin = 0; 
	}

	if ( random->RandomInt( 0,39 ) == 0 )
	{
		m_nSkin = 1;
	}

	if (!waitforeating && GetEnemy() != NULL && GetActivity() == ACT_RUN)
	{
		// chasing enemy. Sprint for last bit
		if ( (GetAbsOrigin() - GetEnemy()->GetAbsOrigin()).Length2D() < PDRONE_SPRINT_DIST )
		{
			m_flPlaybackRate = 1.25;
		}
	}
	if (m_iHealth < sk_pitdrone_min_health.GetFloat() && m_iHealth < sk_pitdrone_health.GetFloat() && GetEnemy() == NULL && (GetActivity() != ACT_WALK && GetActivity() != ACT_DIE_BARNACLE_SWALLOW) && !stop)
	{
		/*if (GetActivity() != ACT_WALK || GetActivity() != ACT_DIE_BARNACLE_SWALLOW)
		{
			ResetActivity();
		}*/
		//SetEnemy(NULL);

		GetMotor()->SetIdealYaw(SearchForMeal());

		waitforeating = true;
		//DevMsg("GetActivity %i \n", GetActivity());
		if (GetAbsOrigin().DistTo(SearchForMeal()) > 38.f && SearchForMeal() != Vector(NULL, NULL, NULL))
		{
			GetNavigator()->SetGoal(SearchForMeal());
			if (GetActivity() != ACT_WALK)
				ResetIdealActivity(ACT_WALK);
			if (gpGlobals->curtime + SequenceDuration() <= gpGlobals->curtime)
			{
				//if (GetActivity() != ACT_WALK)
				ResetIdealActivity(ACT_WALK);
			}
		}
		else
		{
			if (GetAbsOrigin().DistTo(SearchForMeal()) > 38.f)
			{
				ResetIdealActivity(ACT_IDLE);
				//Q_snprintf(pMealName, sizeof(pMealName), "prop_ragdoll");
				stop = true;				
			}

			GetNavigator()->StopMoving();

			if (GetActivity() != ACT_DIE_BARNACLE_SWALLOW)
			{
				ResetIdealActivity(ACT_DIE_BARNACLE_SWALLOW);
				
				//DevMsg("m_iHealth++ \n");
				if (m_iHealth < sk_pitdrone_health.GetFloat())
					m_iHealth += sk_pitdrone_eat_health.GetInt();
				else if (m_iHealth >= sk_pitdrone_health.GetFloat())
				{
					SetActivity(ACT_IDLE);
					waitforeating = false;
					stop = true;
				}
			}
			if (gpGlobals->curtime + SequenceDuration() <= gpGlobals->curtime)
			{
				ResetIdealActivity(ACT_DIE_BARNACLE_SWALLOW);
			}

			/*if (pMeal != NULL)
			{
				/*char szSSName[50];
				Q_snprintf(szSSName, sizeof(szSSName), "prop_ragdoll_%s", "busy");
				pMealName = szSSName;*/

				//char tempName[50];
				/*Q_snprintf(pMealName, sizeof(pMealName), "prop_ragdoll%d", this->entindex());
				//pMealName = tempName;
				pMeal->SetClassname(pMealName);//This is for enother instance of this class
			}*/
			if (!foundedMeal)
				foundedMeal = true;
		}
	}

}

Vector CNPC_PitDrone::SearchForMeal()
{
	/*if (foundedMeal)
	{
		return Vector(NULL, NULL, NULL);
	}*/

	pMeal = gEntList.FindEntityGenericWithin(this, pMealName, GetAbsOrigin(), sk_pitdrone_meal_search_radius.GetFloat());//prop_ragdoll
	pMeal = gEntList.FindEntityGenericNearest(pMealName, GetAbsOrigin(), sk_pitdrone_meal_search_radius.GetFloat());

	if (pMeal != NULL)
	{	
		Q_snprintf(pMealName, sizeof(pMealName), "prop_ragdoll%d", this->entindex());
		pMeal->SetClassname(pMealName);
		return pMeal->GetAbsOrigin();
	}
	return Vector(NULL,NULL,NULL);
}
//=========================================================
// GetSchedule 
//=========================================================
int CNPC_PitDrone::SelectSchedule( void )
{
	switch	( m_NPCState )
	{
	case NPC_STATE_ALERT:
		{
			if ( HasCondition( COND_LIGHT_DAMAGE ) || HasCondition( COND_HEAVY_DAMAGE ) )
			{
				return SCHED_PDRONE_HURTHOP;
			}

			if ( HasCondition( COND_PDRONE_SMELL_FOOD ) )
			{
				CSound		*pSound;

				pSound = GetBestScent();

				if ( pSound && (!FInViewCone ( pSound->GetSoundOrigin() ) || !FVisible ( pSound->GetSoundOrigin() )) )
				{
					// scent is behind or occluded
					return SCHED_PDRONE_SNIFF_AND_EAT;
				}

				// food is right out in the open. Just go get it.
				return SCHED_PDRONE_EAT;
			}

			if ( HasCondition( COND_SMELL ) )
			{
				// there's something stinky. 
				CSound		*pSound;

				pSound = GetBestScent();
				if ( pSound )
					return SCHED_PDRONE_WALLOW;
			}

			break;
		}
	case NPC_STATE_COMBAT:
		{
// dead enemy
			if ( HasCondition( COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return BaseClass::SelectSchedule();
			}

			if (HasCondition(COND_NEW_ENEMY) && !waitforeating)
			{
				if ( m_fCanThreatDisplay && IRelationType( GetEnemy() ) == D_HT && FClassnameIs( GetEnemy(), "npc_headcrab" ) )
				{
					// this means squid sees a headcrab!
					m_fCanThreatDisplay = FALSE;// only do the headcrab dance once per lifetime.
					return SCHED_PDRONE_SEECRAB;
				}
				else
				{
					return SCHED_WAKE_ANGRY;
				}
			}

			if ( HasCondition( COND_PDRONE_SMELL_FOOD ) )
			{
				CSound		*pSound;

				pSound = GetBestScent();
				
				if ( pSound && (!FInViewCone ( pSound->GetSoundOrigin() ) || !FVisible ( pSound->GetSoundOrigin() )) )
				{
					// scent is behind or occluded
					return SCHED_PDRONE_SNIFF_AND_EAT;
				}

				// food is right out in the open. Just go get it.
				return SCHED_PDRONE_EAT;
			}

			if ( HasCondition( COND_CAN_RANGE_ATTACK1 ) )
			{
				return SCHED_RANGE_ATTACK1;
			}

			if ( HasCondition( COND_CAN_MELEE_ATTACK1 ) )
			{
				return SCHED_MELEE_ATTACK1;
			}

			if ( HasCondition( COND_CAN_MELEE_ATTACK2 ) )
			{
				return SCHED_MELEE_ATTACK2;
			}
			
			return SCHED_CHASE_ENEMY;

			break;
		}
	}

	return BaseClass::SelectSchedule();
}

//=========================================================
// FInViewCone - returns true is the passed vector is in
// the caller's forward view cone. The dot product is performed
// in 2d, making the view cone infinitely tall. 
//=========================================================
bool CNPC_PitDrone::FInViewCone ( Vector pOrigin )
{
	Vector los = ( pOrigin - GetAbsOrigin() );

	// do this in 2D
	los.z = 0;
	VectorNormalize( los );

	Vector facingDir = EyeDirection2D( );

	float flDot = DotProduct( los, facingDir );

	if ( flDot > m_flFieldOfView )
		return true;

	return false;
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target vector
//=========================================================
bool CNPC_PitDrone::FVisible ( Vector vecOrigin )
{
	trace_t tr;
	Vector		vecLookerOrigin;
	
	vecLookerOrigin = EyePosition();//look through the caller's 'eyes'
	UTIL_TraceLine(vecLookerOrigin, vecOrigin, MASK_BLOCKLOS, this/*pentIgnore*/, COLLISION_GROUP_NONE, &tr);
	
	if ( tr.fraction != 1.0 )
		 return false; // Line of sight is not established
	else
		 return true;// line of sight is valid.
}

//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.  OVERRIDDEN for bullsquid because it needs to
// know explicitly when the last attempt to chase the enemy
// failed, since that impacts its attack choices.
//=========================================================
void CNPC_PitDrone::StartTask ( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MELEE_ATTACK2:
		{
			CPASAttenuationFilter filter( this );
			EmitSound( filter, entindex(), "NPC_Pitdrone.MeleeAttackDouble" );	//growl
			//Msg("Drone growl \n");
			BaseClass::StartTask ( pTask );
			break;
		}
	case TASK_PDRONE_HOPTURN:
		{
			SetActivity ( ACT_HOP );
			
			if (!waitforeating && GetEnemy())
			{
				Vector	vecFacing = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() );
				VectorNormalize( vecFacing );

				GetMotor()->SetIdealYaw( vecFacing );
			}

			break;
		}
	case TASK_PDRONE_EAT:
		{
			m_flHungryTime = gpGlobals->curtime + pTask->flTaskData;
			break;
		}

	default:
		{
			BaseClass::StartTask ( pTask );
			break;
		}
	}
}

//=========================================================
// RunTask
//=========================================================
void CNPC_PitDrone::RunTask ( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_PDRONE_HOPTURN:
		{
			if (!waitforeating && GetEnemy())
			{
				Vector	vecFacing = ( GetEnemy()->GetAbsOrigin() - GetAbsOrigin() );
				VectorNormalize( vecFacing );
				GetMotor()->SetIdealYaw( vecFacing );
			}

			if ( IsSequenceFinished() )
			{
				TaskComplete(); 
			}
			break;
		}
	default:
		{
			BaseClass::RunTask( pTask );
			break;
		}
	}
}

//=========================================================
// GetIdealState - Overridden for Bullsquid to deal with
// the feature that makes it lose interest in headcrabs for 
// a while if something injures it. 
//=========================================================
NPC_STATE CNPC_PitDrone::SelectIdealState ( void )
{
	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch ( m_NPCState )
	{
	case NPC_STATE_COMBAT:
		/*
		COMBAT goes to ALERT upon death of enemy
		*/
		{
			// BriJee OVR: В этом условии вылет если дрон целится на npc_headcrab и в этот момент его убивают.
			// Исправлено, добавил проверку на GetEnemy()
			if (GetEnemy() && !waitforeating != NULL && (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE)) && FClassnameIs(GetEnemy(), "npc_headcrab"))
			{
				// if the squid has a headcrab enemy and something hurts it, it's going to forget about the crab for a while.
				SetEnemy( NULL );
				return NPC_STATE_ALERT;
			}
			break;
		}
	}

	return BaseClass::SelectIdealState();
}


//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( monster_bullchicken, CNPC_PitDrone )

	DECLARE_TASK ( TASK_PDRONE_HOPTURN )
	DECLARE_TASK ( TASK_PDRONE_EAT )

	DECLARE_CONDITION( COND_PDRONE_SMELL_FOOD )

	DECLARE_ACTIVITY( ACT_PDRONE_EXCITED )
	DECLARE_ACTIVITY( ACT_PDRONE_EAT )
	DECLARE_ACTIVITY( ACT_PDRONE_DETECT_SCENT )
	DECLARE_ACTIVITY( ACT_PDRONE_INSPECT_FLOOR )

	//=========================================================
	// > SCHED_PDRONE_HURTHOP
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_PDRONE_HURTHOP,
	
		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_SOUND_WAKE				0"
		"		TASK_PDRONE_HOPTURN			0"
		"		TASK_FACE_ENEMY				0"
		"	"
		"	Interrupts"
	)
	
	//=========================================================
	// > SCHED_PDRONE_SEECRAB
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_PDRONE_SEECRAB,
	
		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_SOUND_WAKE				0"
		"		TASK_PLAY_SEQUENCE			ACTIVITY:ACT_PDRONE_EXCITED"
		"		TASK_FACE_ENEMY				0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
	)
	
	//=========================================================
	// > SCHED_PDRONE_EAT
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_PDRONE_EAT,
	
		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_PDRONE_EAT						10"
		"		TASK_STORE_LASTPOSITION				0"
		"		TASK_GET_PATH_TO_BESTSCENT			0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_PDRONE_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_PDRONE_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_PDRONE_EAT"
		"		TASK_PDRONE_EAT						50"
		"		TASK_GET_PATH_TO_LASTPOSITION		0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_CLEAR_LASTPOSITION				0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
		"		COND_SMELL"
	)
	
	//=========================================================
	// > SCHED_PDRONE_SNIFF_AND_EAT
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_PDRONE_SNIFF_AND_EAT,
	
		"	Tasks"
		"		TASK_STOP_MOVING					0"
		"		TASK_PDRONE_EAT						10"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_PDRONE_DETECT_SCENT"
		"		TASK_STORE_LASTPOSITION				0"
		"		TASK_GET_PATH_TO_BESTSCENT			0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_PDRONE_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_PDRONE_EAT"
		"		TASK_PLAY_SEQUENCE					ACTIVITY:ACT_PDRONE_EAT"
		"		TASK_PDRONE_EAT						50"
		"		TASK_GET_PATH_TO_LASTPOSITION		0"
		"		TASK_WALK_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_CLEAR_LASTPOSITION				0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
		"		COND_SMELL"
	)
	
	//=========================================================
	// > SCHED_PDRONE_WALLOW
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_PDRONE_WALLOW,
	
		"	Tasks"
		"		TASK_STOP_MOVING				0"
		"		TASK_PDRONE_EAT					10"
		"		TASK_STORE_LASTPOSITION			0"
		"		TASK_GET_PATH_TO_BESTSCENT		0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_PLAY_SEQUENCE				ACTIVITY:ACT_PDRONE_INSPECT_FLOOR"
		"		TASK_PDRONE_EAT					50"
		"		TASK_GET_PATH_TO_LASTPOSITION	0"
		"		TASK_WALK_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"		TASK_CLEAR_LASTPOSITION			0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
	)
	
	//=========================================================
	// > SCHED_PDRONE_CHASE_ENEMY
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_PDRONE_CHASE_ENEMY,
	
		"	Tasks"
		"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_RANGE_ATTACK1"
		"		TASK_GET_PATH_TO_ENEMY			0"
		"		TASK_RUN_PATH					0"
		"		TASK_WAIT_FOR_MOVEMENT			0"
		"	"
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_SMELL"
		"		COND_CAN_RANGE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK1"
		"		COND_CAN_MELEE_ATTACK2"
		"		COND_TASK_FAILED"
	)

AI_END_CUSTOM_NPC()
