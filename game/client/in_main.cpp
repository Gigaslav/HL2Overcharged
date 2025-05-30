//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: builds an intended movement command to send to the server
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "kbutton.h"
#include "usercmd.h"
#include "in_buttons.h"
#include "input.h"
#include "iviewrender.h"
#include "iclientmode.h"
#include "prediction.h"
#include "bitbuf.h"
#include "checksum_md5.h"
#include "hltvcamera.h"
#if defined( REPLAY_ENABLED )
#include "replay/replaycamera.h"
#endif
#include <ctype.h> // isalnum()
#include <voice_status.h>
#include "cam_thirdperson.h"

#ifdef SIXENSE
#include "sixense/in_sixense.h"
#endif

#include "client_virtualreality.h"
#include "sourcevr/isourcevirtualreality.h"

// NVNT Include
#include "haptics/haptic_utils.h"
#include <vgui/ISurface.h>

extern ConVar in_joystick;
extern ConVar cam_idealpitch;
extern ConVar cam_idealyaw;

// For showing/hiding the scoreboard
#include <game/client/iviewport.h>

#include "view.h"
#include "hud_macros.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// FIXME, tie to entity state parsing for player!!!
int g_iAlive = 1;

static int s_ClearInputState = 0;

// Defined in pm_math.c
float anglemod( float a );

// FIXME void V_Init( void );
static int in_impulse = 0;
static int in_cancel = 0;

ConVar oc_player_view_threshold_fix("oc_player_view_threshold_fix","0", FCVAR_REPLICATED, "Enable/Disable fix for player's view.x threshold");
ConVar oc_player_view_threshold_border("oc_player_view_threshold_border", "0", FCVAR_REPLICATED, "Enable/Disable fix for player's view.x border value");

ConVar cl_anglespeedkey( "cl_anglespeedkey", "0.67", 0 );
ConVar cl_yawspeed( "cl_yawspeed", "210", FCVAR_NONE, "Client yaw speed.", true, -100000, true, 100000 );
ConVar cl_pitchspeed( "cl_pitchspeed", "225", FCVAR_NONE, "Client pitch speed.", true, -100000, true, 100000 );
ConVar cl_pitchdown( "cl_pitchdown", "89", FCVAR_CHEAT );
ConVar cl_pitchup( "cl_pitchup", "89", FCVAR_CHEAT );
#if defined( CSTRIKE_DLL )
ConVar cl_sidespeed( "cl_sidespeed", "400", FCVAR_CHEAT );
ConVar cl_upspeed( "cl_upspeed", "320", FCVAR_ARCHIVE|FCVAR_CHEAT );
ConVar cl_forwardspeed( "cl_forwardspeed", "400", FCVAR_ARCHIVE|FCVAR_CHEAT );
ConVar cl_backspeed( "cl_backspeed", "400", FCVAR_ARCHIVE|FCVAR_CHEAT );
#else
ConVar cl_sidespeed( "cl_sidespeed", "450", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar cl_upspeed( "cl_upspeed", "320", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar cl_forwardspeed( "cl_forwardspeed", "450", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar cl_backspeed( "cl_backspeed", "450", FCVAR_REPLICATED | FCVAR_CHEAT );
#endif // CSTRIKE_DLL
ConVar lookspring( "lookspring", "0", FCVAR_ARCHIVE );
ConVar lookstrafe( "lookstrafe", "0", FCVAR_ARCHIVE );
ConVar in_joystick( "joystick","0", FCVAR_ARCHIVE );

ConVar thirdperson_platformer( "thirdperson_platformer", "0", 0, "Player will aim in the direction they are moving." );
ConVar thirdperson_screenspace( "thirdperson_screenspace", "0", 0, "Movement will be relative to the camera, eg: left means screen-left" );

ConVar sv_noclipduringpause( "sv_noclipduringpause", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "If cheats are enabled, then you can noclip with the game paused (for doing screenshots, etc.)." );

//static ConVar oc_weapon_free_aim_koef("oc_weapon_free_aim_koef", "1", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_changetime("oc_weapon_free_aim_changetime", "1", FCVAR_ARCHIVE);
//static ConVar oc_weapon_free_aim("oc_free_aim_enable", "1", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_use_interval("oc_weapon_free_aim_disable_separate_maincamera", "0", FCVAR_ARCHIVE); // use an interval for view turning
ConVar oc_weapon_free_aim_movethreshold("oc_weapon_free_aim_movethreshold", "0.7", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_movemax("oc_weapon_free_aim_movemax", "0.3", FCVAR_ARCHIVE);

ConVar oc_weapon_free_aim_speedturn("oc_weapon_free_aim_speedturn", "1", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_speed_evenyaw("oc_weapon_free_aim_speed_evenyaw", "1", FCVAR_ARCHIVE);

ConVar oc_weapon_free_aim_autoturn_speed("oc_weapon_free_aim_autoturn_speed", "250", FCVAR_ARCHIVE);
ConVar oc_weapon_free_aim_disabled_on_SprintAndWallStanding("oc_weapon_free_aim_disabled_on_SprintAndWallStanding", "0", FCVAR_ARCHIVE);


extern void ScreenToWorld(int mousex, int mousey, float fov, const Vector& vecRenderOrigin, const QAngle& vecRenderAngles, Vector& vecPickingRay);


extern ConVar cl_mouselook;

#define UsingMouselook() cl_mouselook.GetBool()

/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition

===============================================================================
*/

kbutton_t	in_speed;
kbutton_t	in_walk;
kbutton_t	in_jlook;
kbutton_t	in_strafe;
kbutton_t	in_commandermousemove;
kbutton_t	in_forward;
kbutton_t	in_back;
kbutton_t	in_moveleft;
kbutton_t	in_moveright;
// Display the netgraph
kbutton_t	in_graph;  
kbutton_t	in_joyspeed;		// auto-speed key from the joystick (only works for player movement, not vehicles)


static	kbutton_t	in_klook;
kbutton_t	in_left;
kbutton_t	in_right;
static	kbutton_t	in_lookup;
static	kbutton_t	in_lookdown;
static	kbutton_t	in_use;
static	kbutton_t	in_jump;
static	kbutton_t	in_attack;
static	kbutton_t	in_attack2;
static	kbutton_t	in_up;
static	kbutton_t	in_down;
static	kbutton_t	in_duck;
static	kbutton_t	in_reload;
static	kbutton_t	in_kick;
static	kbutton_t	in_laserswitch;
static	kbutton_t	in_score;
static	kbutton_t	in_break;
static	kbutton_t	in_zoom;
static  kbutton_t   in_grenade1;
static  kbutton_t   in_grenade2;
static	kbutton_t	in_attack3;
// BriJee OVR: New button definitions
static  kbutton_t	in_leanleft;
static  kbutton_t   in_leanright;
static	kbutton_t	in_swing;
static  kbutton_t   in_throwgrenade;
kbutton_t	in_aimmode;
static  kbutton_t   in_firemode;
static  kbutton_t   in_switchmode;
static  kbutton_t   in_switchsilencer;
static  kbutton_t   in_grenadeswitch;
static  kbutton_t   in_drop;
//static  kbutton_t   in_scrollup;
//static  kbutton_t   in_scrolldown;
kbutton_t	in_ducktoggle;
//kbutton_t   in_ir_sight;
/*CON_COMMAND(cam_ots_TurnAuto, "")
{
	::input->CAM_UpdateAngleByFreeAiming(true);
}
CON_COMMAND(cam_ots_Turn180, "")
{
	::input->CAM_UpdateAngle180();
}*/
// Update the actual eyeangles of the player entity and translate the movement input
bool FirstAng = true;
int VehAng = 0;
int Ud = 0;
bool WasInVehicle = false;
void CInput::CalcPlayerAngle(CUserCmd *cmd)
{
	C_BasePlayer *pl = C_BasePlayer::GetLocalPlayer();
	if (!pl)
		return;
	if (pl->IsInAVehicle()) //&& VehAng < 1)
	{
		//m_angViewAngle = pl->EyeAngles();
		//VehAng++;
		Ud = 0;
		WasInVehicle = true;
		return;
	}

	if (FirstAng)
	{
		m_angViewAngle = pl->EyeAngles();
		FirstAng = false;
	}

	if ((!pl->IsInAVehicle()) && (Ud == 0) && WasInVehicle)
	{
		VehAng = 0;
		m_angViewAngle = pl->EyeAngles();
		if (Ud < 5)
			Ud++;
		WasInVehicle = false;
	}
	if 	(cvar->FindVar("oc_level_shutdown")->GetInt())
	{
		m_angViewAngle = pl->EyeAngles();
		cvar->FindVar("oc_level_shutdown")->SetValue(0);
	}

		trace_t tr;
		const Vector eyePos = pl->EyePosition();
		Vector vecForward = MainViewForward();

		//SetCamViewangles(m_angViewAngle);
		//m_angViewAngle = MainViewAngles();
		QAngle angCam = m_angViewAngle; //MainViewAngles();
		/*DevMsg("angles.x: %i \n", angCam.x);
		DevMsg("angles.y: %i \n", angCam.y);
		DevMsg("angles.z: %i \n", angCam.z);*/
		if (pl->IsAlive() && ::input->CAM_IsFreeAiming())
		{
			int screen_x, screen_y;
			engine->GetScreenSize(screen_x, screen_y);
			screen_x *= m_vecFreeAimPos.x * 0.5f + 0.5f;
			screen_y *= m_vecFreeAimPos.y * 0.5f + 0.5f;
			ScreenToWorld(screen_x, screen_y, pl->GetFOV(), MainViewOrigin(), angCam, vecForward);
		}
		UTIL_TraceLine(MainViewOrigin(), MainViewOrigin() + vecForward * MAX_TRACE_LENGTH, MASK_SHOT, pl, COLLISION_GROUP_NONE, &tr);

		// ensure that the player entity does not shoot towards the camera, get dist to plane where the player is on and add a constant
		float flMinForward = abs(DotProduct(MainViewForward(), eyePos - MainViewOrigin())) + 32.0f;
		Vector vecTrace = tr.endpos - tr.startpos;
		float flLenOld = vecTrace.NormalizeInPlace();
		float flLen = max(flMinForward, flLenOld);
		vecTrace *= flLen;

		Vector vecFinalDir = MainViewOrigin() + vecTrace - eyePos; //eyePos;

		QAngle playerangles;
		VectorAngles(vecForward, playerangles);
		//playerangles.z = pl->EyePosition().z;

		//playerangles2.z = playerangles.x;
		if (playerangles.x >= 85.0f && playerangles.x < 180.0f)
		{
			playerangles.x = 85.0f;
		}
		if (playerangles.x <= 360.0f && playerangles.x >= 270.0f)
		{
			playerangles.x -= 360.0f;
		}
		/*if (playerangles.x <= 275.0f && playerangles.x > 180.0f)
		{
			playerangles.x = 275.0f;
		}*/

		//DevMsg("playerangles.x: %.2f \n", playerangles.x);

		if (oc_player_view_threshold_fix.GetBool())
		{
			float border = oc_player_view_threshold_border.GetFloat() ? oc_player_view_threshold_border.GetFloat() : 89.f;
			playerangles.x = Clamp(playerangles.x, -border, border);
		}

		engine->SetViewAngles(playerangles);

	playerangles.z = angCam.z = 0;
	playerangles.x = angCam.x = 0;
	Vector cFwd, cRight, pFwd, pRight;
	AngleVectors(angCam, &cFwd, &cRight, NULL);
	AngleVectors(playerangles, &pFwd, &pRight, NULL);

	float flMove[2] = { cmd->forwardmove, cmd->sidemove };
	cmd->forwardmove = DotProduct(cFwd, pFwd) * flMove[0] + DotProduct(cRight, pFwd) * flMove[1];
	cmd->sidemove = DotProduct(cRight, pRight) * flMove[1] + DotProduct(cFwd, pRight) * flMove[0];
}
/*void CInput::SetCamViewangles(QAngle const &view)
{
	m_angViewAngle = view;

	if (m_angViewAngle.x > 180.0f)
		m_angViewAngle.x -= 360.0f;
	if (m_angViewAngle.x < -180.0f)
		m_angViewAngle.x += 360.0f;
}
void CInput::CAM_UpdateAngle180()
{
	m_angViewAngle_Delta.y = 180.0f;
	if (cvar->FindVar("oc_weapon_free_aim")->GetInt() == 1)
		m_angViewAngle_Delta.y *= -Sign(m_vecFreeAimPos.x);
}*/
bool CInput::CAM_IsFreeAiming()
{
	C_BasePlayer *pl = C_BasePlayer::GetLocalPlayer();
	if (!pl)
		return false;

	/*C_BaseCombatWeapon *pWeapon = pl->GetActiveWeapon();
	if (!pWeapon)
		return false;

	if (!pWeapon->GetWpnData().AllowFreeAim)
		return false;*/

	bool isFreeAim = cvar->FindVar("oc_weapon_free_aim")->GetInt();

	return isFreeAim;
}
Vector2D CInput::CAM_GetFreeAimCursor()
{
	return m_vecFreeAimPos;
}

// Allows code to manipulate the view angles
/*void CInput::CAM_UpdateAngleByFreeAiming(bool bUser)
{
	if (prediction->InPrediction() && !prediction->IsFirstTimePredicted())
		return;

	if (cvar->FindVar("oc_weapon_free_aim")->GetInt() == 0)
		return;

	C_BasePlayer *pl = C_BasePlayer::GetLocalPlayer();
	if (!pl)
		return;


	Vector vecForward, vecForwardCam;
	int screen_x, screen_y;
	engine->GetScreenSize(screen_x, screen_y);
	screen_x *= m_vecFreeAimPos.x * 0.5f + 0.5f;
	screen_y *= m_vecFreeAimPos.y * 0.5f + 0.5f;
	ScreenToWorld(screen_x, screen_y, pl->GetFOV(), MainViewOrigin(), MainViewAngles(), vecForward);

	trace_t tr;
	UTIL_TraceLine(MainViewOrigin(), MainViewOrigin() + vecForward * MAX_TRACE_LENGTH, MASK_SHOT, pl, COLLISION_GROUP_NONE, &tr);
	Vector vecWorldTarget = tr.endpos;
	Vector vecDir_PlayerViewTarget = vecWorldTarget - pl->EyePosition();

	AngleVectors(m_angViewAngle, &vecForwardCam, 0, 0);

	Vector vecDir_PlayerCamPlane = MainViewOrigin() - pl->EyePosition();
	float flPlaneFwdComponent = DotProduct(vecDir_PlayerCamPlane, vecForwardCam);
	vecDir_PlayerCamPlane -= flPlaneFwdComponent * vecForwardCam;

	const float lengthCamToImaginaryTarget = FastSqrt(vecDir_PlayerViewTarget.LengthSqr() - vecDir_PlayerCamPlane.LengthSqr());
	Vector vecDir_PlayerTmpViewTarget = vecDir_PlayerCamPlane + vecForwardCam * lengthCamToImaginaryTarget;

	Vector vecDir_PlayerCamPlaneDst;
	QAngle ang1, ang2, angDelta;
	VectorAngles(vecDir_PlayerTmpViewTarget, ang1);
	VectorAngles(vecDir_PlayerViewTarget, ang2);
	RotationDelta(ang1, ang2, &angDelta);
	VectorRotate(vecDir_PlayerCamPlane, angDelta, vecDir_PlayerCamPlaneDst);

	Vector vecDstDirImaginaryTarget_NoOffset = vecWorldTarget - vecDir_PlayerCamPlaneDst;

	vecDstDirImaginaryTarget_NoOffset -= pl->EyePosition();

	if (!bUser)
	{
		m_angViewAngle_Delta.Init();
		m_vecFreeAimPos_Delta.Init();

		QAngle tmp;
		VectorAngles(vecDstDirImaginaryTarget_NoOffset, tmp);
		if (tmp.IsValid())
		{
			m_angViewAngle = tmp;
			if (m_angViewAngle.x > 180.0f)
				m_angViewAngle.x -= 360.0f;
			else if (m_angViewAngle.x < -180.0f)
				m_angViewAngle.x += 360.0f;
			m_vecFreeAimPos.Init();
		}
	}
	else
	{
		QAngle angTarget;
		VectorAngles(vecDstDirImaginaryTarget_NoOffset, angTarget);
		for (int i = 0; i < 2; i++)
			m_angViewAngle_Delta[i] = AngleDiff(angTarget[i], m_angViewAngle[i]);
		m_angViewAngle_Delta.z = 0;

		if (m_angViewAngle_Delta.y > 180.0f)
			m_angViewAngle_Delta.y = 180.0f;
		else if (m_angViewAngle_Delta.y < -180.0f)
			m_angViewAngle_Delta.y = -180.0f;

		if (!m_angViewAngle_Delta.IsValid())
			m_angViewAngle_Delta.Init();
		else
			m_vecFreeAimPos_Delta = -m_vecFreeAimPos;
	}
}*/

// Defines crosshair position and rotates the view if appropriate
float InVehicle = 0.0f;
float InIron = 0.0f;
float Interpolation = 0.0f;
float Ref = oc_weapon_free_aim_movemax.GetFloat();
float Ref2 = oc_weapon_free_aim_movemax.GetFloat();
bool DoOnceRecoil = true;
void CInput::TryCursorMove(QAngle& viewangles, CUserCmd *cmd, float x, float y)
{
	static ConVarRef m_yaw("m_yaw");
	static ConVarRef m_pitch("m_pitch");
	float MoveMax = oc_weapon_free_aim_movemax.GetFloat();

	C_BasePlayer *pl = C_BasePlayer::GetLocalPlayer();
	if (!pl)
		return;

	C_BaseCombatWeapon *pWeapon = pl->GetActiveWeapon();

	bool hasWeapon = pWeapon != NULL;

	/*if (!pWeapon)
		return;*/

	bool disableMove = hasWeapon ? (pWeapon->IsIronSighted() || pWeapon->IsScopeSighted() || !pWeapon->GetWpnData().AllowFreeAim) : true;
	//DevMsg("irSight: %i \n", irSight);
	bool wallBump3 = cvar->FindVar("oc_weapons_allow_wall_bump")->GetInt() ? cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0 : true;

	bool plrInUse = cvar->FindVar("oc_using_entity")->GetInt();

	if (pl->IsInAVehicle())
	{
		Ref = 0.0f;
	}
	else if (!pl->IsInAVehicle())
	{
		Ref = MoveMax;
	}

	if (pl->IsInAVehicle() || disableMove || pl->IsRunning() || plrInUse)
	{
		//DevMsg("Ref2: %.2f \n", Ref2);
		if (Ref2 > 0.0f)
			Ref2 = Ref2 - gpGlobals->frametime * oc_weapon_free_aim_changetime.GetFloat();
		else if (Ref2 <= 0.0f)
			Ref2 = 0.0f;

		MoveMax = Ref2;
	}
	else if (!pl->IsInAVehicle() || !disableMove || pl->IsRunning() || !plrInUse)
	{
		MoveMax = oc_weapon_free_aim_movemax.GetFloat();
		Ref2 = MoveMax;
	}

	float flFOV = (pl ? pl->GetFOV() : 1.0f);
	float flScaleNormalizedRange_Yaw = m_yaw.GetFloat() / flFOV;
	float flScaleNormalizedRange_Pitch = m_pitch.GetFloat() / flFOV;
	float flScale_Pitch = 1;
	if (oc_weapon_free_aim_speed_evenyaw.GetInt())
		flScale_Pitch = engine->GetScreenAspectRatio();

	/*if (pl->Weapon_Switch(pl->GetActiveWeapon()))
	{
		cvar->FindVar("oc_recoil_x")->SetValue(0);
		cvar->FindVar("oc_recoil_y")->SetValue(0);
	}*/

	float Recoil_X = cvar->FindVar("oc_recoil_x")->GetFloat();
	float Recoil_Y = cvar->FindVar("oc_recoil_y")->GetFloat();

	m_vecFreeAimPos += Vector2D(x * flScaleNormalizedRange_Yaw + Recoil_X, y * flScale_Pitch * flScaleNormalizedRange_Pitch + Recoil_Y);
	float flLength = m_vecFreeAimPos.NormalizeInPlace();

	if (disableMove)
	{
		viewangles.x += Recoil_X * 70;
		viewangles.y += Recoil_Y * 70;

		//AppendXViewAnglesClamped(viewangles, Recoil_X * 70);
		//viewangles.y += Recoil_Y * 70;
	}

	cvar->FindVar("oc_recoil_x")->SetValue(Lerp(1, Recoil_X, 0.f));
	cvar->FindVar("oc_recoil_y")->SetValue(Lerp(1, Recoil_X, 0.f));

	Vector2D Buffer;
	Buffer += Vector2D(x * flScaleNormalizedRange_Yaw + Recoil_X, y * flScale_Pitch * flScaleNormalizedRange_Pitch + Recoil_Y);
	float flLengthBuffer = Buffer.NormalizeInPlace();
	float flMoveVarsBuffer = 0.001f;

	//flMoveVars[1] = irSight || cvar->FindVar("oc_state_InSecondFire")->GetInt() ? (flMoveVars[1] <= 0.0f ? 0.0f : flMoveVars[1] - gpGlobals->frametime * oc_weapon_free_aim_changetime.GetFloat()) : flMoveVars[1];

	//float flMoveVars[2] = { oc_weapon_free_aim_movethreshold.GetFloat(), (oc_weapon_free_aim_disabled_on_SprintAndWallStanding.GetInt() == 1) ? ((pl->IsInAVehicle()) ? Ref : (irSight || wallBump || (pl->IsRunning()) || (cvar->FindVar("oc_state_InSecondFire")->GetInt() == 1) || (cvar->FindVar("oc_weapon_free_aim_option")->GetInt() == 0) || (cvar->FindVar("oc_using_entity")->GetInt() == 1)) ? Ref2 : MoveMax) : ((pl->IsInAVehicle()) ? Ref : ((cvar->FindVar("oc_state_IRsight_on")->GetInt() == 1) || (cvar->FindVar("oc_state_InSecondFire")->GetInt() == 1) || (cvar->FindVar("oc_weapon_free_aim_option")->GetInt() == 0) || (cvar->FindVar("oc_using_entity")->GetInt() == 1)) ? Ref2 : MoveMax) };

	float flMoveVars[2] = { oc_weapon_free_aim_movethreshold.GetFloat(), MoveMax };

	if (!pl->IsSuitEquipped())
		flMoveVars[1] = 0.f;

	if ((!disableMove
		&& wallBump3
		&& !plrInUse)
		&& ((oc_weapon_free_aim_disabled_on_SprintAndWallStanding.GetInt() == 1 && pl->IsRunning()) 
			|| oc_weapon_free_aim_disabled_on_SprintAndWallStanding.GetInt() == 0))
	{
		if (viewangles.x > 60.f && viewangles.x < 80.f)
		{
			float coefficient = viewangles.x - 60;
			flMoveVars[1] -= coefficient * (MoveMax / 20);
			//DevMsg("flMoveVars1After: %2f \n", flMoveVars[1]);
		}
		else if (viewangles.x < -60.f && viewangles.x > -80.f)
		{
			float coefficient = abs(viewangles.x) - 60;
			flMoveVars[1] -= coefficient * (MoveMax / 20);
			//DevMsg("flMoveVars1After: %2f \n", flMoveVars[1]);
		}
	}

	float flTurnSpeed;

	Vector2D moveDir = m_vecFreeAimPos;
	moveDir.NormalizeInPlace();

	if (oc_weapon_free_aim_use_interval.GetInt())
	{
		/*flTurnSpeed = oc_weapon_free_aim_speedturn.GetFloat() * max(0, (
			(flLength - flMoveVars[0]) / (flMoveVars[1] - flMoveVars[0])
			));
		viewangles += QAngle(moveDir.y * flTurnSpeed, moveDir.x * -flTurnSpeed, 0);*/

		flTurnSpeed = max(0, (flLengthBuffer - flMoveVarsBuffer));
		//flTurnSpeed = abs(flTurnSpeed);
		//DevMsg("flTurnSpeed: %.2f \n", flTurnSpeed);
		//DevMsg("flLengthBuffer - flMoveVarsBuffer: %.2f \n", (flLengthBuffer - flMoveVarsBuffer));
		
		viewangles += QAngle(y * flScale_Pitch * flScaleNormalizedRange_Pitch * flFOV, x * -flScaleNormalizedRange_Yaw * flFOV, 0);
		//viewangles += QAngle(moveDir.y * flTurnSpeed * flFOV, moveDir.x * -flTurnSpeed * flFOV, 0);

		//AppendXViewAnglesClamped(viewangles, y * flScale_Pitch * flScaleNormalizedRange_Pitch * flFOV);
		//viewangles.y += x * -flScaleNormalizedRange_Yaw * flFOV;
		//viewangles.z = 0;
	}
	else
	{
		flTurnSpeed = max(0, (flLength - flMoveVars[1]));

		//float X = Clamp((float)(moveDir.y * flTurnSpeed * flFOV), -78.0f, 78.0f);

		//float Y = moveDir.x * -flTurnSpeed * flFOV;

		//viewangles += QAngle(X, Y, 0);

		viewangles += QAngle(moveDir.y * flTurnSpeed * flFOV, moveDir.x * -flTurnSpeed * flFOV, 0);

		//AppendXViewAnglesClamped(viewangles, moveDir.y * flTurnSpeed * flFOV);
		//viewangles.y += moveDir.x * -flTurnSpeed * flFOV;
		//viewangles.z = 0;
	}

	//DevMsg("viewangles.x: %2f \n", viewangles.x);
	//DevMsg("viewangles.y: %2f \n", viewangles.y);
	//DevMsg("viewangles.z: %2f \n", viewangles.z);

	if (viewangles.x > 78.0f)//78
	{
		viewangles.x = 78.0f;
		viewangles.z = 0.0f;
	}
	else if (viewangles.x < -78.0f)
	{
		viewangles.x = -78.0f;
		viewangles.z = 0.0f;
	}

	/*if (viewangles.y > 180.0f)
		viewangles.y -= 360.0f;
	else if (viewangles.y < -180.0f)
		viewangles.y += 360.0f;*/

	if (oc_weapon_free_aim_use_interval.GetInt())
	{
		flLengthBuffer = min(flMoveVarsBuffer, flLengthBuffer);
		Buffer *= flLengthBuffer;

		flLength = min(flMoveVars[1], flLength);
		m_vecFreeAimPos *= flLength;
	}
	else
	{
		flLengthBuffer = 0.0f;
		Buffer *= 0;;

		flLength = min(flMoveVars[1], flLength);
		m_vecFreeAimPos *= flLength;
	}
	//flLength = min(flMoveVars[1], flLength);
	//m_vecFreeAimPos *= flLength;

	cmd->mousedx = (int)x;
	cmd->mousedy = (int)y;
}


/*
===========
IN_CenterView_f
===========
*/
void IN_CenterView_f (void)
{
	QAngle viewangles;

	if ( UsingMouselook() == false )
	{
		if ( !::input->CAM_InterceptingMouse() )
		{
			engine->GetViewAngles( viewangles );
			viewangles[PITCH] = 0;
			engine->SetViewAngles( viewangles );
		}
	}
}

/*
===========
IN_Joystick_Advanced_f
===========
*/
void IN_Joystick_Advanced_f (void)
{
	::input->Joystick_Advanced();
}

/*
============
KB_ConvertString

Removes references to +use and replaces them with the keyname in the output string.  If
 a binding is unfound, then the original text is retained.
NOTE:  Only works for text with +word in it.
============
*/
int KB_ConvertString( char *in, char **ppout )
{
	char sz[ 4096 ];
	char binding[ 64 ];
	char *p;
	char *pOut;
	char *pEnd;
	const char *pBinding;

	if ( !ppout )
		return 0;

	*ppout = NULL;
	p = in;
	pOut = sz;
	while ( *p )
	{
		if ( *p == '+' )
		{
			pEnd = binding;
			while ( *p && ( V_isalnum( *p ) || ( pEnd == binding ) ) && ( ( pEnd - binding ) < 63 ) )
			{
				*pEnd++ = *p++;
			}

			*pEnd =  '\0';

			pBinding = NULL;
			if ( strlen( binding + 1 ) > 0 )
			{
				// See if there is a binding for binding?
				pBinding = engine->Key_LookupBinding( binding + 1 );
			}

			if ( pBinding )
			{
				*pOut++ = '[';
				pEnd = (char *)pBinding;
			}
			else
			{
				pEnd = binding;
			}

			while ( *pEnd )
			{
				*pOut++ = *pEnd++;
			}

			if ( pBinding )
			{
				*pOut++ = ']';
			}
		}
		else
		{
			*pOut++ = *p++;
		}
	}

	*pOut = '\0';

	int maxlen = strlen( sz ) + 1;
	pOut = ( char * )malloc( maxlen );
	Q_strncpy( pOut, sz, maxlen );
	*ppout = pOut;

	return 1;
}

/*
==============================
FindKey

Allows the engine to request a kbutton handler by name, if the key exists.
==============================
*/
kbutton_t *CInput::FindKey( const char *name )
{
	CKeyboardKey *p;
	p = m_pKeys;
	while ( p )
	{
		if ( !Q_stricmp( name, p->name ) )
		{
			return p->pkey;
		}

		p = p->next;
	}
	return NULL;
}

/*
============
AddKeyButton

Add a kbutton_t * to the list of pointers the engine can retrieve via KB_Find
============
*/
void CInput::AddKeyButton( const char *name, kbutton_t *pkb )
{
	CKeyboardKey *p;	
	kbutton_t *kb;

	kb = FindKey( name );
	
	if ( kb )
		return;

	p = new CKeyboardKey;

	Q_strncpy( p->name, name, sizeof( p->name ) );
	p->pkey = pkb;

	p->next = m_pKeys;
	m_pKeys = p;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CInput::CInput( void )
{
	m_pCommands = NULL;
	m_pCameraThirdData = NULL;
	m_pVerifiedCommands = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CInput::~CInput( void )
{
}

/*
============
Init_Keyboard

Add kbutton_t definitions that the engine can query if needed
============
*/
void CInput::Init_Keyboard( void )
{
	m_pKeys = NULL;

	AddKeyButton( "in_graph", &in_graph );
	AddKeyButton( "in_jlook", &in_jlook );
}

/*
============
Shutdown_Keyboard

Clear kblist
============
*/
void CInput::Shutdown_Keyboard( void )
{
	CKeyboardKey *p, *n;
	p = m_pKeys;
	while ( p )
	{
		n = p->next;
		delete p;
		p = n;
	}
	m_pKeys = NULL;
}

/*
============
KeyDown
============
*/
void KeyDown( kbutton_t *b, const char *c )
{
	int		k = -1;
	if ( c && c[0] )
	{
		k = atoi(c);
	}

	if (k == b->down[0] || k == b->down[1])
		return;		// repeating key
	
	if (!b->down[0])
		b->down[0] = k;
	else if (!b->down[1])
		b->down[1] = k;
	else
	{
		if ( c[0] )
		{
			DevMsg( 1,"Three keys down for a button '%c' '%c' '%c'!\n", b->down[0], b->down[1], c[0]);
		}
		return;
	}
	
	if (b->state & 1)
		return;		// still down
	b->state |= 1 + 2;	// down + impulse down
}

/*
============
KeyUp
============
*/
void KeyUp( kbutton_t *b, const char *c )
{	
	if ( !c || !c[0] )
	{
		b->down[0] = b->down[1] = 0;
		b->state = 4;	// impulse up
		return;
	}

	int k = atoi(c);

	if (b->down[0] == k)
		b->down[0] = 0;
	else if (b->down[1] == k)
		b->down[1] = 0;
	else
		return;		// key up without coresponding down (menu pass through)

	if (b->down[0] || b->down[1])
	{
		//Msg ("Keys down for button: '%c' '%c' '%c' (%d,%d,%d)!\n", b->down[0], b->down[1], c, b->down[0], b->down[1], c);
		return;		// some other key is still holding it down
	}

	if (!(b->state & 1))
		return;		// still up (this should not happen)

	b->state &= ~1;		// now up
	b->state |= 4; 		// impulse up
}

void IN_CommanderMouseMoveDown( const CCommand &args ) {KeyDown(&in_commandermousemove, args[1] );}
void IN_CommanderMouseMoveUp( const CCommand &args ) {KeyUp(&in_commandermousemove, args[1] );}
void IN_BreakDown( const CCommand &args ) { KeyDown( &in_break , args[1] );}
void IN_BreakUp( const CCommand &args )
{ 
	KeyUp( &in_break, args[1] ); 
#if defined( _DEBUG )
	DebuggerBreak();
#endif
};
void IN_KLookDown ( const CCommand &args ) {KeyDown(&in_klook, args[1] );}
void IN_KLookUp ( const CCommand &args ) {KeyUp(&in_klook, args[1] );}
void IN_JLookDown ( const CCommand &args ) {KeyDown(&in_jlook, args[1] );}
void IN_JLookUp ( const CCommand &args ) {KeyUp(&in_jlook, args[1] );}
void IN_UpDown( const CCommand &args ) {KeyDown(&in_up, args[1] );}
void IN_UpUp( const CCommand &args ) {KeyUp(&in_up, args[1] );}
void IN_DownDown( const CCommand &args ) {KeyDown(&in_down, args[1] );}
void IN_DownUp( const CCommand &args ) {KeyUp(&in_down, args[1] );}
void IN_LeftDown( const CCommand &args ) {KeyDown(&in_left, args[1] );}
void IN_LeftUp( const CCommand &args ) {KeyUp(&in_left, args[1] );}
void IN_RightDown( const CCommand &args ) {KeyDown(&in_right, args[1] );}
void IN_RightUp( const CCommand &args ) {KeyUp(&in_right, args[1] );}
void IN_ForwardDown( const CCommand &args ) {KeyDown(&in_forward, args[1] );}
void IN_ForwardUp( const CCommand &args ) {KeyUp(&in_forward, args[1] );}
void IN_BackDown( const CCommand &args ) {KeyDown(&in_back, args[1] );}
void IN_BackUp( const CCommand &args ) {KeyUp(&in_back, args[1] );}
void IN_LookupDown( const CCommand &args ) {KeyDown(&in_lookup, args[1] );}
void IN_LookupUp( const CCommand &args ) {KeyUp(&in_lookup, args[1] );}
void IN_LookdownDown( const CCommand &args ) {KeyDown(&in_lookdown, args[1] );}
void IN_LookdownUp( const CCommand &args ) {KeyUp(&in_lookdown, args[1] );}
void IN_MoveleftDown( const CCommand &args ) {KeyDown(&in_moveleft, args[1] );}
void IN_MoveleftUp( const CCommand &args ) {KeyUp(&in_moveleft, args[1] );}
void IN_MoverightDown( const CCommand &args ) {KeyDown(&in_moveright, args[1] );}
void IN_MoverightUp( const CCommand &args ) {KeyUp(&in_moveright, args[1] );}
void IN_WalkDown( const CCommand &args ) {KeyDown(&in_walk, args[1] );}
void IN_WalkUp( const CCommand &args ) {KeyUp(&in_walk, args[1] );}
void IN_SpeedDown( const CCommand &args ) {KeyDown(&in_speed, args[1] );}
void IN_SpeedUp( const CCommand &args ) {KeyUp(&in_speed, args[1] );}
void IN_StrafeDown( const CCommand &args ) {KeyDown(&in_strafe, args[1] );}
void IN_StrafeUp( const CCommand &args ) {KeyUp(&in_strafe, args[1] );}
void IN_Attack2Down( const CCommand &args ) { KeyDown(&in_attack2, args[1] );}
void IN_Attack2Up( const CCommand &args ) {KeyUp(&in_attack2, args[1] );}
void IN_UseDown ( const CCommand &args ) {KeyDown(&in_use, args[1] );}
void IN_UseUp ( const CCommand &args ) {KeyUp(&in_use, args[1] );}
void IN_JumpDown ( const CCommand &args ) {KeyDown(&in_jump, args[1] );}
void IN_JumpUp ( const CCommand &args ) {KeyUp(&in_jump, args[1] );}
void IN_DuckDown( const CCommand &args ) {KeyDown(&in_duck, args[1] );}
void IN_DuckUp( const CCommand &args ) {KeyUp(&in_duck, args[1] );}
void IN_ReloadDown( const CCommand &args ) {KeyDown(&in_reload, args[1] );}
void IN_ReloadUp( const CCommand &args ) {KeyUp(&in_reload, args[1] );}
void IN_KickDown( const CCommand &args ) {KeyDown(&in_kick, args[1] );}
void IN_KickUp(const CCommand &args) { KeyUp(&in_kick, args[1]); }
void IN_LaserSwitchDown( const CCommand &args ) {KeyDown(&in_laserswitch, args[1] );}
void IN_LaserSwitchUp(const CCommand &args) { KeyUp(&in_laserswitch, args[1]); }
void IN_GraphDown( const CCommand &args ) {KeyDown(&in_graph, args[1] );}
void IN_GraphUp( const CCommand &args ) {KeyUp(&in_graph, args[1] );}
void IN_ZoomDown( const CCommand &args ) {KeyDown(&in_zoom, args[1] );}
void IN_ZoomUp( const CCommand &args ) {KeyUp(&in_zoom, args[1] );}
void IN_Grenade1Up( const CCommand &args ) { KeyUp( &in_grenade1, args[1] ); }
void IN_Grenade1Down( const CCommand &args ) { KeyDown( &in_grenade1, args[1] ); }
void IN_Grenade2Up( const CCommand &args ) { KeyUp( &in_grenade2, args[1] ); }
void IN_Grenade2Down( const CCommand &args ) { KeyDown( &in_grenade2, args[1] ); }
void IN_XboxStub( const CCommand &args ) { /*do nothing*/ }
void IN_Attack3Down( const CCommand &args ) { KeyDown(&in_attack3, args[1] );}
void IN_Attack3Up( const CCommand &args ) { KeyUp(&in_attack3, args[1] );}

// BriJee OVR: New button definitions
void IN_LeanLeftDown(const CCommand &args) { KeyDown(&in_leanleft, args[1]); }
void IN_LeanLeftUp(const CCommand &args) { KeyUp(&in_leanleft, args[1]); }
void IN_LeanRightDown(const CCommand &args) { KeyDown(&in_leanright, args[1]); }
void IN_LeanRightUp(const CCommand &args) { KeyUp(&in_leanright, args[1]); }
void IN_SwingDown(const CCommand &args) { KeyDown(&in_swing, args[1]); }
void IN_SwingUp(const CCommand &args) { KeyUp(&in_swing, args[1]); }
void IN_ThrowGrenadeDown(const CCommand &args) { KeyDown(&in_throwgrenade, args[1]); }
void IN_ThrowGrenadeUp(const CCommand &args) { KeyUp(&in_throwgrenade, args[1]); }
void IN_AimmodeDown(const CCommand &args) { KeyDown(&in_aimmode, args[1]); }
void IN_AimmodeUp(const CCommand &args) { KeyUp(&in_aimmode, args[1]); }
void IN_FireModeDown(const CCommand &args) { KeyDown(&in_firemode, args[1]); }
void IN_FireModeUp(const CCommand &args) { KeyUp(&in_firemode, args[1]); }

void IN_DropDown(const CCommand &args) { KeyDown(&in_drop, args[1]); }
void IN_DropUp(const CCommand &args) { KeyUp(&in_drop, args[1]); }

void IN_SwitchModeDown(const CCommand &args) { KeyDown(&in_switchmode, args[1]); }
void IN_SwitchModeUp(const CCommand &args) { KeyUp(&in_switchmode, args[1]); }

void IN_SwitchSilencerDown(const CCommand &args) { KeyDown(&in_switchsilencer, args[1]); }
void IN_SwitchSilencerUp(const CCommand &args) { KeyUp(&in_switchsilencer, args[1]); }

void IN_GrenadeSwitchDown(const CCommand &args) { KeyDown(&in_grenadeswitch, args[1]); }
void IN_GrenadeSwitchUp(const CCommand &args) { KeyUp(&in_grenadeswitch, args[1]); }
/*void IN_ScrollUpDown(const CCommand &args) { KeyDown(&in_scrollup, args[1]); }
void IN_ScrollUpUp(const CCommand &args) { KeyUp(&in_scrollup, args[1]); }
void IN_ScrollDownDown(const CCommand &args) { KeyDown(&in_scrolldown, args[1]); }
void IN_ScrollDownUp(const CCommand &args) { KeyUp(&in_scrolldown, args[1]); }*/
//void IN_IrSightDown(const CCommand &args) { KeyDown(&in_ir_sight, args[1]); }
//void IN_IrSightUp(const CCommand &args) { KeyUp(&in_ir_sight, args[1]); }

void IN_DuckToggle( const CCommand &args ) 
{ 
	if ( ::input->KeyState(&in_ducktoggle) )
	{
		KeyUp( &in_ducktoggle, args[1] ); 
	}
	else
	{
		KeyDown( &in_ducktoggle, args[1] ); 
	}
}

void IN_AttackDown( const CCommand &args )
{
	KeyDown( &in_attack, args[1] );
}

void IN_AttackUp( const CCommand &args )
{
	KeyUp( &in_attack, args[1] );
	in_cancel = 0;
}

// Special handling
void IN_Cancel( const CCommand &args )
{
	in_cancel = 1;
}

void IN_Impulse( const CCommand &args )
{
	in_impulse = atoi( args[1] );
}

void IN_ScoreDown( const CCommand &args )
{
	KeyDown( &in_score, args[1] );
	if ( gViewPortInterface )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, true );
	}
}

void IN_ScoreUp( const CCommand &args )
{
	KeyUp( &in_score, args[1] );
	if ( gViewPortInterface )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, false );
		GetClientVoiceMgr()->StopSquelchMode();
	}
}


/*
============
KeyEvent

Return 1 to allow engine to process the key, otherwise, act on it as needed
============
*/
int CInput::KeyEvent( int down, ButtonCode_t code, const char *pszCurrentBinding )
{
	// Deal with camera intercepting the mouse
	if ( ( code == MOUSE_LEFT ) || ( code == MOUSE_RIGHT ) )
	{
		if ( m_fCameraInterceptingMouse )
			return 0;
	}

	if ( g_pClientMode )
		return g_pClientMode->KeyInput(down, code, pszCurrentBinding);

	return 1;
}



/*
===============
KeyState

Returns 0.25 if a key was pressed and released during the frame,
0.5 if it was pressed and held
0 if held then released, and
1.0 if held for the entire time
===============
*/
float CInput::KeyState ( kbutton_t *key )
{
	float		val = 0.0;
	int			impulsedown, impulseup, down;
	
	impulsedown = key->state & 2;
	impulseup	= key->state & 4;
	down		= key->state & 1;
	
	if ( impulsedown && !impulseup )
	{
		// pressed and held this frame?
		val = down ? 0.5 : 0.0;
	}

	if ( impulseup && !impulsedown )
	{
		// released this frame?
		val = down ? 0.0 : 0.0;
	}

	if ( !impulsedown && !impulseup )
	{
		// held the entire frame?
		val = down ? 1.0 : 0.0;
	}

	if ( impulsedown && impulseup )
	{
		if ( down )
		{
			// released and re-pressed this frame
			val = 0.75;	
		}
		else
		{
			// pressed and released this frame
			val = 0.25;	
		}
	}

	// clear impulses
	key->state &= 1;		
	return val;
}

void CInput::IN_SetSampleTime( float frametime )
{
	m_flKeyboardSampleTime = frametime;
}

/*
==============================
DetermineKeySpeed

==============================
*/
static ConVar in_usekeyboardsampletime( "in_usekeyboardsampletime", "1", 0, "Use keyboard sample time smoothing." );

float CInput::DetermineKeySpeed( float frametime )
{

	if ( in_usekeyboardsampletime.GetBool() )
	{
		if ( m_flKeyboardSampleTime <= 0 )
			return 0.0f;
	
		frametime = MIN( m_flKeyboardSampleTime, frametime );
		m_flKeyboardSampleTime -= frametime;
	}
	
	float speed;

	speed = frametime;

	if ( in_speed.state & 1 )
	{
		speed *= cl_anglespeedkey.GetFloat();
	}

	return speed;
}

/*
==============================
AdjustYaw

==============================
*/
void CInput::AdjustYaw( float speed, QAngle& viewangles )
{
	if ( !(in_strafe.state & 1) )
	{
		viewangles[YAW] -= speed*cl_yawspeed.GetFloat() * KeyState (&in_right);
		viewangles[YAW] += speed*cl_yawspeed.GetFloat() * KeyState (&in_left);
	}

	// thirdperson platformer mode
	// use movement keys to aim the player relative to the thirdperson camera
	if ( CAM_IsThirdPerson() && thirdperson_platformer.GetInt() )
	{
		float side = KeyState(&in_moveleft) - KeyState(&in_moveright);
		float forward = KeyState(&in_forward) - KeyState(&in_back);

		if ( side || forward )
		{
			viewangles[YAW] = RAD2DEG(atan2(side, forward)) + g_ThirdPersonManager.GetCameraOffsetAngles()[ YAW ];
		}
		if ( side || forward || KeyState (&in_right) || KeyState (&in_left) )
		{
			cam_idealyaw.SetValue( g_ThirdPersonManager.GetCameraOffsetAngles()[ YAW ] - viewangles[ YAW ] );
		}
	}
}

/*
==============================
AdjustPitch

==============================
*/
void CInput::AdjustPitch( float speed, QAngle& viewangles )
{
	// only allow keyboard looking if mouse look is disabled
	if ( UsingMouselook() == false )
	{
		float	up, down;

		if ( in_klook.state & 1 )
		{
			view->StopPitchDrift ();
			viewangles[PITCH] -= speed*cl_pitchspeed.GetFloat() * KeyState (&in_forward);
			viewangles[PITCH] += speed*cl_pitchspeed.GetFloat() * KeyState (&in_back);
		}

		up		= KeyState ( &in_lookup );
		down	= KeyState ( &in_lookdown );
		
		viewangles[PITCH] -= speed*cl_pitchspeed.GetFloat() * up;
		viewangles[PITCH] += speed*cl_pitchspeed.GetFloat() * down;

		if ( up || down )
		{
			view->StopPitchDrift ();
		}
	}	
}

/*
==============================
ClampAngles

==============================
*/
void CInput::ClampAngles( QAngle& viewangles )
{
	if ( viewangles[PITCH] > cl_pitchdown.GetFloat() )
	{
		viewangles[PITCH] = cl_pitchdown.GetFloat();
	}
	if ( viewangles[PITCH] < -cl_pitchup.GetFloat() )
	{
		viewangles[PITCH] = -cl_pitchup.GetFloat();
	}

#ifndef PORTAL	// Don't constrain Roll in Portal because the player can be upside down! -Jeep
	if ( viewangles[ROLL] > 50 )
	{
		viewangles[ROLL] = 50;
	}
	if ( viewangles[ROLL] < -50 )
	{
		viewangles[ROLL] = -50;
	}
#endif
}

/*
================
AdjustAngles

Moves the local angle positions
================
*/
void CInput::AdjustAngles ( float frametime )
{
	float	speed;
	QAngle viewangles;
	
	// Determine control scaling factor ( multiplies time )
	speed = DetermineKeySpeed( frametime );
	if ( speed <= 0.0f )
	{
		return;
	}

	// Retrieve latest view direction from engine
	engine->GetViewAngles( viewangles );

	// Adjust YAW
	AdjustYaw( speed, viewangles );

	// Adjust PITCH if keyboard looking
	AdjustPitch( speed, viewangles );
	
	// Make sure values are legitimate
	ClampAngles( viewangles );

	if (oc_player_view_threshold_fix.GetBool())
	{
		float border = oc_player_view_threshold_border.GetFloat() ? oc_player_view_threshold_border.GetFloat() : 89.f;
		viewangles.x = Clamp(viewangles.x, -border, border);
	}
	// Store new view angles into engine view direction
	engine->SetViewAngles( viewangles );
}

/*
==============================
ComputeSideMove

==============================
*/
void CInput::ComputeSideMove( CUserCmd *cmd )
{
	// thirdperson platformer movement
	if ( CAM_IsThirdPerson() && thirdperson_platformer.GetInt() )
	{
		// no sideways movement in this mode
		return;
	}

	// thirdperson screenspace movement
	if ( CAM_IsThirdPerson() && thirdperson_screenspace.GetInt() )
	{
		float ideal_yaw = cam_idealyaw.GetFloat();
		float ideal_sin = sin(DEG2RAD(ideal_yaw));
		float ideal_cos = cos(DEG2RAD(ideal_yaw));
		
		float movement = ideal_cos*KeyState(&in_moveright)
			+  ideal_sin*KeyState(&in_back)
			+ -ideal_cos*KeyState(&in_moveleft)
			+ -ideal_sin*KeyState(&in_forward);

		cmd->sidemove += cl_sidespeed.GetFloat() * movement;

		return;
	}

	// If strafing, check left and right keys and act like moveleft and moveright keys
	if ( in_strafe.state & 1 )
	{
		cmd->sidemove += cl_sidespeed.GetFloat() * KeyState (&in_right);
		cmd->sidemove -= cl_sidespeed.GetFloat() * KeyState (&in_left);
	}

	// Otherwise, check strafe keys
	cmd->sidemove += cl_sidespeed.GetFloat() * KeyState (&in_moveright);
	cmd->sidemove -= cl_sidespeed.GetFloat() * KeyState (&in_moveleft);
}

/*
==============================
ComputeUpwardMove

==============================
*/
void CInput::ComputeUpwardMove( CUserCmd *cmd )
{
	cmd->upmove += cl_upspeed.GetFloat() * KeyState (&in_up);
	cmd->upmove -= cl_upspeed.GetFloat() * KeyState (&in_down);
}

/*
==============================
ComputeForwardMove

==============================
*/
void CInput::ComputeForwardMove( CUserCmd *cmd )
{
	// thirdperson platformer movement
	if ( CAM_IsThirdPerson() && thirdperson_platformer.GetInt() )
	{
		// movement is always forward in this mode
		float movement = KeyState(&in_forward)
			|| KeyState(&in_moveright)
			|| KeyState(&in_back)
			|| KeyState(&in_moveleft);

		cmd->forwardmove += cl_forwardspeed.GetFloat() * movement;

		return;
	}

	// thirdperson screenspace movement
	if ( CAM_IsThirdPerson() && thirdperson_screenspace.GetInt() )
	{
		float ideal_yaw = cam_idealyaw.GetFloat();
		float ideal_sin = sin(DEG2RAD(ideal_yaw));
		float ideal_cos = cos(DEG2RAD(ideal_yaw));
		
		float movement = ideal_cos*KeyState(&in_forward)
			+  ideal_sin*KeyState(&in_moveright)
			+ -ideal_cos*KeyState(&in_back)
			+ -ideal_sin*KeyState(&in_moveleft);

		cmd->forwardmove += cl_forwardspeed.GetFloat() * movement;

		return;
	}

	if ( !(in_klook.state & 1 ) )
	{	
		cmd->forwardmove += cl_forwardspeed.GetFloat() * KeyState (&in_forward);
		cmd->forwardmove -= cl_backspeed.GetFloat() * KeyState (&in_back);
	}	
}

/*
==============================
ScaleMovements

==============================
*/
void CInput::ScaleMovements( CUserCmd *cmd )
{
	// float spd;

	// clip to maxspeed
	// FIXME FIXME:  This doesn't work
	return;

	/*
	spd = engine->GetClientMaxspeed();
	if ( spd == 0.0 )
		return;

	// Scale the speed so that the total velocity is not > spd
	float fmov = sqrt( (cmd->forwardmove*cmd->forwardmove) + (cmd->sidemove*cmd->sidemove) + (cmd->upmove*cmd->upmove) );

	if ( fmov > spd && fmov > 0.0 )
	{
		float fratio = spd / fmov;

		if ( !IsNoClipping() ) 
		{
			cmd->forwardmove	*= fratio;
			cmd->sidemove		*= fratio;
			cmd->upmove			*= fratio;
		}
	}
	*/
}


/*
===========
ControllerMove
===========
*/
void CInput::ControllerMove( float frametime, CUserCmd *cmd )
{
	if ( IsPC() )
	{
		if ( !m_fCameraInterceptingMouse && m_fMouseActive )
		{
			MouseMove( cmd);
		}
	}

	JoyStickMove( frametime, cmd);

	// NVNT if we have a haptic device..
	if(haptics && haptics->HasDevice())
	{
		if(engine->IsPaused() || engine->IsLevelMainMenuBackground() || vgui::surface()->IsCursorVisible() || !engine->IsInGame())
		{
			// NVNT send a menu process to the haptics system.
			haptics->MenuProcess();
			return;
		}
#ifdef CSTRIKE_DLL
		// NVNT cstrike fov grabing.
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		if(player){
			haptics->UpdatePlayerFOV(player->GetFOV());
		}
#endif
		// NVNT calculate move with the navigation on the haptics system.
		haptics->CalculateMove(cmd->forwardmove, cmd->sidemove, frametime);
		// NVNT send a game process to the haptics system.
		haptics->GameProcess();
#if defined( WIN32 ) && !defined( _X360 )
		// NVNT update our avatar effect.
		UpdateAvatarEffect();
#endif
	}


}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *weapon - 
//-----------------------------------------------------------------------------
void CInput::MakeWeaponSelection( C_BaseCombatWeapon *weapon )
{
	m_hSelectedWeapon = weapon;
}

/*
================
CreateMove

Send the intended movement message to the server
if active == 1 then we are 1) not playing back demos ( where our commands are ignored ) and
2 ) we have finished signing on to server
================
*/

void CInput::ExtraMouseSample( float frametime, bool active )
{
	CUserCmd dummy;
	CUserCmd *cmd = &dummy;

	cmd->Reset();


	QAngle viewangles;
	engine->GetViewAngles( viewangles );
	QAngle originalViewangles = viewangles;

	if ( active )
	{

		// Determine view angles
		AdjustAngles ( frametime );

		// Determine sideways movement
		ComputeSideMove( cmd );

		// Determine vertical movement
		ComputeUpwardMove( cmd );

		// Determine forward movement
		ComputeForwardMove( cmd );

		// Scale based on holding speed key or having too fast of a velocity based on client maximum
		//  speed.
		ScaleMovements( cmd );

		// Allow mice and other controllers to add their inputs
		ControllerMove( frametime, cmd );
#ifdef SIXENSE
		g_pSixenseInput->SixenseFrame( frametime, cmd ); 

		if( g_pSixenseInput->IsEnabled() )
		{
			g_pSixenseInput->SetView( frametime, cmd );
		}
#endif
	}

	if (::input->CAM_IsFreeAiming())
	{
		CalcPlayerAngle(cmd);
	}
	// Retreive view angles from engine ( could have been set in IN_AdjustAngles above )
	engine->GetViewAngles( viewangles );

	// Set button and flag bits, don't blow away state
#ifdef SIXENSE
	if( g_pSixenseInput->IsEnabled() )
	{
		// Some buttons were set in SixenseUpdateKeys, so or in any real keypresses
		cmd->buttons |= GetButtonBits( 0 );
	}
	else
	{
		cmd->buttons = GetButtonBits( 0 );
	}
#else
	cmd->buttons = GetButtonBits( 0 );
#endif

	// Use new view angles if alive, otherwise user last angles we stored off.
	if ( g_iAlive )
	{
		VectorCopy( viewangles, cmd->viewangles );
		VectorCopy( viewangles, m_angPreviousViewAngles );
	}
	else
	{
		VectorCopy( m_angPreviousViewAngles, cmd->viewangles );
	}

	// Let the move manager override anything it wants to.
	if ( g_pClientMode->CreateMove( frametime, cmd ) )
	{
		if (oc_player_view_threshold_fix.GetBool())
		{
			float border = oc_player_view_threshold_border.GetFloat() ? oc_player_view_threshold_border.GetFloat() : 89.f;
			cmd->viewangles.x = Clamp(cmd->viewangles.x, -border, border);
		}

		// Get current view angles after the client mode tweaks with it
		engine->SetViewAngles( cmd->viewangles );
		prediction->SetLocalViewAngles( cmd->viewangles );
	}

	// Let the headtracker override the view at the very end of the process so
	// that vehicles and other stuff in g_pClientMode->CreateMove can override 
	// first
	if ( active && UseVR() )
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
		if( pPlayer && !pPlayer->GetVehicle() )
		{
			QAngle curViewangles, newViewangles;
			Vector curMotion, newMotion;
			engine->GetViewAngles( curViewangles );
			curMotion.Init ( 
				cmd->forwardmove,
				cmd->sidemove,
				cmd->upmove );
			g_ClientVirtualReality.OverridePlayerMotion ( frametime, originalViewangles, curViewangles, curMotion, &newViewangles, &newMotion );

			if (oc_player_view_threshold_fix.GetBool())
			{
				float border = oc_player_view_threshold_border.GetFloat() ? oc_player_view_threshold_border.GetFloat() : 89.f;
				newViewangles.x = Clamp(newViewangles.x, -border, border);
			}

			engine->SetViewAngles( newViewangles );
			cmd->forwardmove = newMotion[0];
			cmd->sidemove = newMotion[1];
			cmd->upmove = newMotion[2];

			cmd->viewangles = newViewangles;
			prediction->SetLocalViewAngles( cmd->viewangles );
		}
	}

}

void CInput::CreateMove ( int sequence_number, float input_sample_frametime, bool active )
{	
	CUserCmd *cmd = &m_pCommands[ sequence_number % MULTIPLAYER_BACKUP ];
	CVerifiedUserCmd *pVerified = &m_pVerifiedCommands[ sequence_number % MULTIPLAYER_BACKUP ];

	cmd->Reset();

	cmd->command_number = sequence_number;
	cmd->tick_count = gpGlobals->tickcount;

	QAngle viewangles;
	engine->GetViewAngles( viewangles );
	QAngle originalViewangles = viewangles;

	if ( active || sv_noclipduringpause.GetInt() )
	{
		// Determine view angles
		AdjustAngles ( input_sample_frametime );

		// Determine sideways movement
		ComputeSideMove( cmd );

		// Determine vertical movement
		ComputeUpwardMove( cmd );

		// Determine forward movement
		ComputeForwardMove( cmd );

		// Scale based on holding speed key or having too fast of a velocity based on client maximum
		//  speed.
		ScaleMovements( cmd );

		// Allow mice and other controllers to add their inputs
		ControllerMove( input_sample_frametime, cmd );
#ifdef SIXENSE
		g_pSixenseInput->SixenseFrame( input_sample_frametime, cmd ); 

		if( g_pSixenseInput->IsEnabled() )
		{
			g_pSixenseInput->SetView( input_sample_frametime, cmd );
		}
#endif
	}
	else
	{
		// need to run and reset mouse input so that there is no view pop when unpausing
		if ( !m_fCameraInterceptingMouse && m_fMouseActive )
		{
			float mx, my;
			GetAccumulatedMouseDeltasAndResetAccumulators( &mx, &my );
			ResetMouse();
		}
	}

	if (::input->CAM_IsFreeAiming())
	{
		CalcPlayerAngle(cmd);
	}

	// Retreive view angles from engine ( could have been set in IN_AdjustAngles above )
	engine->GetViewAngles( viewangles );

	// Latch and clear impulse
	cmd->impulse = in_impulse;
	in_impulse = 0;

	// Latch and clear weapon selection
	if ( m_hSelectedWeapon != NULL )
	{
		C_BaseCombatWeapon *weapon = m_hSelectedWeapon;

		cmd->weaponselect = weapon->entindex();
		cmd->weaponsubtype = weapon->GetSubType();

		// Always clear weapon selection
		m_hSelectedWeapon = NULL;
	}

	// Set button and flag bits
#ifdef SIXENSE
	if( g_pSixenseInput->IsEnabled() )
	{
		// Some buttons were set in SixenseUpdateKeys, so or in any real keypresses
		cmd->buttons |= GetButtonBits( 1 );
	}
	else
	{
		cmd->buttons = GetButtonBits( 1 );
	}
#else
	// Set button and flag bits
	cmd->buttons = GetButtonBits( 1 );
#endif

	// Using joystick?
#ifdef SIXENSE
	if ( in_joystick.GetInt() || g_pSixenseInput->IsEnabled() )
#else
	if ( in_joystick.GetInt() )
#endif
	{
		if ( cmd->forwardmove > 0 )
		{
			cmd->buttons |= IN_FORWARD;
		}
		else if ( cmd->forwardmove < 0 )
		{
			cmd->buttons |= IN_BACK;
		}
	}

	// Use new view angles if alive, otherwise user last angles we stored off.
	if ( g_iAlive )
	{
		VectorCopy( viewangles, cmd->viewangles );
		VectorCopy( viewangles, m_angPreviousViewAngles );
	}
	else
	{
		VectorCopy( m_angPreviousViewAngles, cmd->viewangles );
	}

	// Let the move manager override anything it wants to.
	if ( g_pClientMode->CreateMove( input_sample_frametime, cmd ) )
	{
		// Get current view angles after the client mode tweaks with it
#ifdef SIXENSE
		// Only set the engine angles if sixense is not enabled. It is done in SixenseInput::SetView otherwise.
		if( !g_pSixenseInput->IsEnabled() )
		{
			engine->SetViewAngles( cmd->viewangles );
		}
#else

		if (oc_player_view_threshold_fix.GetBool())
		{
			float border = oc_player_view_threshold_border.GetFloat() ? oc_player_view_threshold_border.GetFloat() : 89.f;
			cmd->viewangles.x = Clamp(cmd->viewangles.x, -border, border);
		}
		engine->SetViewAngles( cmd->viewangles );

#endif

		if ( UseVR() )
		{
			C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
			if( pPlayer && !pPlayer->GetVehicle() )
			{
				QAngle curViewangles, newViewangles;
				Vector curMotion, newMotion;
				engine->GetViewAngles( curViewangles );
				curMotion.Init ( 
					cmd->forwardmove,
					cmd->sidemove,
					cmd->upmove );
				g_ClientVirtualReality.OverridePlayerMotion ( input_sample_frametime, originalViewangles, curViewangles, curMotion, &newViewangles, &newMotion );

				if (oc_player_view_threshold_fix.GetBool())
				{
					float border = oc_player_view_threshold_border.GetFloat() ? oc_player_view_threshold_border.GetFloat() : 89.f;
					newViewangles.x = Clamp(newViewangles.x, -border, border);
				}

				engine->SetViewAngles( newViewangles );
				cmd->forwardmove = newMotion[0];
				cmd->sidemove = newMotion[1];
				cmd->upmove = newMotion[2];
				cmd->viewangles = newViewangles;
			}
			else
			{
				Vector vPos;
				g_ClientVirtualReality.GetTorsoRelativeAim( &vPos, &cmd->viewangles );

				if (oc_player_view_threshold_fix.GetBool())
				{
					float border = oc_player_view_threshold_border.GetFloat() ? oc_player_view_threshold_border.GetFloat() : 89.f;
					cmd->viewangles.x = Clamp(cmd->viewangles.x, -border, border);
				}

				engine->SetViewAngles( cmd->viewangles );
			}
		}
	}

	m_flLastForwardMove = cmd->forwardmove;

	cmd->random_seed = MD5_PseudoRandom( sequence_number ) & 0x7fffffff;

	HLTVCamera()->CreateMove( cmd );
#if defined( REPLAY_ENABLED )
	ReplayCamera()->CreateMove( cmd );
#endif

#if defined( HL2_CLIENT_DLL )
	// copy backchannel data
	int i;
	for (i = 0; i < m_EntityGroundContact.Count(); i++)
	{
		cmd->entitygroundcontact.AddToTail( m_EntityGroundContact[i] );
	}
	m_EntityGroundContact.RemoveAll();
#endif

	pVerified->m_cmd = *cmd;
	pVerified->m_crc = cmd->GetChecksum();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//			buffersize - 
//			slot - 
//-----------------------------------------------------------------------------
void CInput::EncodeUserCmdToBuffer( bf_write& buf, int sequence_number )
{
	CUserCmd nullcmd;
	CUserCmd *cmd = GetUserCmd( sequence_number);

	WriteUsercmd( &buf, cmd, &nullcmd );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : buf - 
//			buffersize - 
//			slot - 
//-----------------------------------------------------------------------------
void CInput::DecodeUserCmdFromBuffer( bf_read& buf, int sequence_number )
{
	CUserCmd nullcmd;
	CUserCmd *cmd = &m_pCommands[ sequence_number % MULTIPLAYER_BACKUP];

	ReadUsercmd( &buf, cmd, &nullcmd );
}

void CInput::ValidateUserCmd( CUserCmd *usercmd, int sequence_number )
{
	// Validate that the usercmd hasn't been changed
	CRC32_t crc = usercmd->GetChecksum();
	if ( crc != m_pVerifiedCommands[ sequence_number % MULTIPLAYER_BACKUP ].m_crc )
	{
		*usercmd = m_pVerifiedCommands[ sequence_number % MULTIPLAYER_BACKUP ].m_cmd;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *buf - 
//			from - 
//			to - 
//-----------------------------------------------------------------------------
bool CInput::WriteUsercmdDeltaToBuffer( bf_write *buf, int from, int to, bool isnewcommand )
{
	Assert( m_pCommands );

	CUserCmd nullcmd;

	CUserCmd *f, *t;

	int startbit = buf->GetNumBitsWritten();

	if ( from == -1 )
	{
		f = &nullcmd;
	}
	else
	{
		f = GetUserCmd( from );

		if ( !f )
		{
			// DevMsg( "WARNING! User command delta too old (from %i, to %i)\n", from, to );
			f = &nullcmd;
		}
		else
		{
			ValidateUserCmd( f, from );
		}
	}

	t = GetUserCmd( to );

	if ( !t )
	{
		// DevMsg( "WARNING! User command too old (from %i, to %i)\n", from, to );
		t = &nullcmd;
	}
	else
	{
		ValidateUserCmd( t, to );
	}

	// Write it into the buffer
	WriteUsercmd( buf, t, f );

	if ( buf->IsOverflowed() )
	{
		int endbit = buf->GetNumBitsWritten();

		Msg( "WARNING! User command buffer overflow(%i %i), last cmd was %i bits long\n",
			from, to,  endbit - startbit );

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : slot - 
// Output : CUserCmd
//-----------------------------------------------------------------------------
CUserCmd *CInput::GetUserCmd( int sequence_number )
{
	Assert( m_pCommands );

	CUserCmd *usercmd = &m_pCommands[ sequence_number % MULTIPLAYER_BACKUP ];

	if ( usercmd->command_number != sequence_number )
	{
		return NULL;	// usercmd was overwritten by newer command
	}

	return usercmd;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bits - 
//			in_button - 
//			in_ignore - 
//			*button - 
//			reset - 
// Output : static void
//-----------------------------------------------------------------------------
static void CalcButtonBits( int& bits, int in_button, int in_ignore, kbutton_t *button, bool reset )
{
	// Down or still down?
	if ( button->state & 3 )
	{
		bits |= in_button;
	}

	int clearmask = ~2;
	if ( in_ignore & in_button )
	{
		// This gets taken care of below in the GetButtonBits code
		//bits &= ~in_button;
		// Remove "still down" as well as "just down"
		clearmask = ~3;
	}

	if ( reset )
	{
		button->state &= clearmask;
	}
}

/*
============
GetButtonBits

Returns appropriate button info for keyboard and mouse state
Set bResetState to 1 to clear old state info
============
*/
int CInput::GetButtonBits( int bResetState )
{
	int bits = 0;

	CalcButtonBits( bits, IN_SPEED, s_ClearInputState, &in_speed, bResetState );
	CalcButtonBits( bits, IN_WALK, s_ClearInputState, &in_walk, bResetState );
	CalcButtonBits( bits, IN_ATTACK, s_ClearInputState, &in_attack, bResetState );
	CalcButtonBits( bits, IN_DUCK, s_ClearInputState, &in_duck, bResetState );
	CalcButtonBits( bits, IN_JUMP, s_ClearInputState, &in_jump, bResetState );
	CalcButtonBits( bits, IN_FORWARD, s_ClearInputState, &in_forward, bResetState );
	CalcButtonBits( bits, IN_BACK, s_ClearInputState, &in_back, bResetState );
	CalcButtonBits( bits, IN_USE, s_ClearInputState, &in_use, bResetState );
	CalcButtonBits( bits, IN_LEFT, s_ClearInputState, &in_left, bResetState );
	CalcButtonBits( bits, IN_RIGHT, s_ClearInputState, &in_right, bResetState );
	CalcButtonBits( bits, IN_MOVELEFT, s_ClearInputState, &in_moveleft, bResetState );
	CalcButtonBits( bits, IN_MOVERIGHT, s_ClearInputState, &in_moveright, bResetState );
	CalcButtonBits( bits, IN_ATTACK2, s_ClearInputState, &in_attack2, bResetState );
	CalcButtonBits( bits, IN_RELOAD, s_ClearInputState, &in_reload, bResetState );
	CalcButtonBits( bits, IN_KICK, s_ClearInputState, &in_kick, bResetState );
	CalcButtonBits( bits, IN_LASERSWITCH, s_ClearInputState, &in_laserswitch, bResetState);
	CalcButtonBits( bits, IN_SCORE, s_ClearInputState, &in_score, bResetState );
	CalcButtonBits( bits, IN_ZOOM, s_ClearInputState, &in_zoom, bResetState );
	CalcButtonBits( bits, IN_GRENADESWITCH, s_ClearInputState, &in_grenadeswitch, bResetState);
	CalcButtonBits( bits, IN_ATTACK3, s_ClearInputState, &in_attack3, bResetState );
	CalcButtonBits( bits, IN_LEANLEFT, s_ClearInputState, &in_leanleft, bResetState);			//BriJee OVR: New button definitions
	CalcButtonBits( bits, IN_LEANRIGHT, s_ClearInputState, &in_leanright, bResetState);
	CalcButtonBits( bits, IN_SWING, s_ClearInputState, &in_swing, bResetState);
	CalcButtonBits( bits, IN_THROWGRENADE, s_ClearInputState, &in_throwgrenade, bResetState);
	CalcButtonBits( bits, IN_AIMMODE, s_ClearInputState, &in_aimmode, bResetState);
	CalcButtonBits( bits, IN_FIREMODE, s_ClearInputState, &in_firemode, bResetState);
	CalcButtonBits( bits, IN_SWITCHMODE, s_ClearInputState, &in_switchmode, bResetState);
	CalcButtonBits( bits, IN_SWITCHSILENCER, s_ClearInputState, &in_switchsilencer, bResetState);
	CalcButtonBits( bits, IN_DROP, s_ClearInputState, &in_drop, bResetState );
	//CalcButtonBits( bits, IN_SCROLLUP, s_ClearInputState, &in_scrollup, bResetState);
	//CalcButtonBits( bits, IN_SCROLLDOWN, s_ClearInputState, &in_scrolldown, bResetState);
	//CalcButtonBits( bits, IN_IRSIGHT, s_ClearInputState, &in_ir_sight, bResetState);

	if ( KeyState(&in_ducktoggle) )
	{
		bits |= IN_DUCK;
	}

	// Cancel is a special flag
	if (in_cancel)
	{
		bits |= IN_CANCEL;
	}

	if ( gHUD.m_iKeyBits & IN_WEAPON1 )
	{
		bits |= IN_WEAPON1;
	}

	if ( gHUD.m_iKeyBits & IN_WEAPON2 )
	{
		bits |= IN_WEAPON2;
	}

	// Clear out any residual
	bits &= ~s_ClearInputState;

	if ( bResetState )
	{
		s_ClearInputState = 0;
	}

	return bits;
}


//-----------------------------------------------------------------------------
// Causes an input to have to be re-pressed to become active
//-----------------------------------------------------------------------------
void CInput::ClearInputButton( int bits )
{
	s_ClearInputState |= bits;
}


/*
==============================
GetLookSpring

==============================
*/
float CInput::GetLookSpring( void )
{
	return lookspring.GetInt();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CInput::GetLastForwardMove( void )
{
	return m_flLastForwardMove;
}


#if defined( HL2_CLIENT_DLL )
//-----------------------------------------------------------------------------
// Purpose: back channel contact info for ground contact
// Output :
//-----------------------------------------------------------------------------

void CInput::AddIKGroundContactInfo( int entindex, float minheight, float maxheight )
{
	CEntityGroundContact data;
	data.entindex = entindex;
	data.minheight = minheight;
	data.maxheight = maxheight;

	if (m_EntityGroundContact.Count() >= MAX_EDICTS)
	{
		// some overflow here, probably bogus anyway
		Assert(0);
		m_EntityGroundContact.RemoveAll();
		return;
	}

	m_EntityGroundContact.AddToTail( data );
}
#endif


static ConCommand startcommandermousemove("+commandermousemove", IN_CommanderMouseMoveDown);
static ConCommand endcommandermousemove("-commandermousemove", IN_CommanderMouseMoveUp);
static ConCommand startmoveup("+moveup",IN_UpDown);
static ConCommand endmoveup("-moveup",IN_UpUp);
static ConCommand startmovedown("+movedown",IN_DownDown);
static ConCommand endmovedown("-movedown",IN_DownUp);
static ConCommand startleft("+left",IN_LeftDown);
static ConCommand endleft("-left",IN_LeftUp);
static ConCommand startright("+right",IN_RightDown);
static ConCommand endright("-right",IN_RightUp);
static ConCommand startforward("+forward",IN_ForwardDown);
static ConCommand endforward("-forward",IN_ForwardUp);
static ConCommand startback("+back",IN_BackDown);
static ConCommand endback("-back",IN_BackUp);
static ConCommand startlookup("+lookup", IN_LookupDown);
static ConCommand endlookup("-lookup", IN_LookupUp);
static ConCommand startlookdown("+lookdown", IN_LookdownDown);
static ConCommand lookdown("-lookdown", IN_LookdownUp);
static ConCommand startstrafe("+strafe", IN_StrafeDown);
static ConCommand endstrafe("-strafe", IN_StrafeUp);
static ConCommand startmoveleft("+moveleft", IN_MoveleftDown);
static ConCommand endmoveleft("-moveleft", IN_MoveleftUp);
static ConCommand startmoveright("+moveright", IN_MoverightDown);
static ConCommand endmoveright("-moveright", IN_MoverightUp);
static ConCommand startspeed("+speed", IN_SpeedDown);
static ConCommand endspeed("-speed", IN_SpeedUp);
static ConCommand startwalk("+walk", IN_WalkDown);
static ConCommand endwalk("-walk", IN_WalkUp);
static ConCommand startattack("+attack", IN_AttackDown);
static ConCommand endattack("-attack", IN_AttackUp);
static ConCommand startattack2("+attack2", IN_Attack2Down);
static ConCommand endattack2("-attack2", IN_Attack2Up);
static ConCommand startuse("+use", IN_UseDown);
static ConCommand enduse("-use", IN_UseUp);
static ConCommand startjump("+jump", IN_JumpDown);
static ConCommand endjump("-jump", IN_JumpUp);
static ConCommand impulse("impulse", IN_Impulse);
static ConCommand startklook("+klook", IN_KLookDown);
static ConCommand endklook("-klook", IN_KLookUp);
static ConCommand startjlook("+jlook", IN_JLookDown);
static ConCommand endjlook("-jlook", IN_JLookUp);
static ConCommand startduck("+duck", IN_DuckDown);
static ConCommand endduck("-duck", IN_DuckUp);
static ConCommand startreload("+reload", IN_ReloadDown);
static ConCommand endreload("-reload", IN_ReloadUp);
static ConCommand startkick("+kick", IN_KickDown);
static ConCommand endkick("-kick", IN_KickUp);
static ConCommand startlaserswitch("+laserswitch", IN_LaserSwitchDown);
static ConCommand endlaserswitch("-laserswitch", IN_LaserSwitchUp);
static ConCommand startscore("+score", IN_ScoreDown);
static ConCommand endscore("-score", IN_ScoreUp);
static ConCommand startshowscores("+showscores", IN_ScoreDown);
static ConCommand endshowscores("-showscores", IN_ScoreUp);
static ConCommand startgraph("+graph", IN_GraphDown);
static ConCommand endgraph("-graph", IN_GraphUp);
static ConCommand startbreak("+break",IN_BreakDown);
static ConCommand endbreak("-break",IN_BreakUp);
static ConCommand force_centerview("force_centerview", IN_CenterView_f);
static ConCommand joyadvancedupdate("joyadvancedupdate", IN_Joystick_Advanced_f, "", FCVAR_CLIENTCMD_CAN_EXECUTE);
static ConCommand startzoom("+zoom", IN_ZoomDown);
static ConCommand endzoom("-zoom", IN_ZoomUp);
static ConCommand endgrenade1( "-grenade1", IN_Grenade1Up );
static ConCommand startgrenade1( "+grenade1", IN_Grenade1Down );
static ConCommand endgrenade2( "-grenade2", IN_Grenade2Up );
static ConCommand startgrenade2( "+grenade2", IN_Grenade2Down );
static ConCommand startattack3("+attack3", IN_Attack3Down);
static ConCommand endattack3("-attack3", IN_Attack3Up);
// BriJee OVR: New button definitions
static ConCommand startleanleft( "+leanleft", IN_LeanLeftDown );
static ConCommand endleanleft( "-leanleft", IN_LeanLeftUp );
static ConCommand startleanright("+leanright", IN_LeanRightDown);
static ConCommand endleanright("-leanright", IN_LeanRightUp);
static ConCommand startswing("+swing", IN_SwingDown);
static ConCommand endswing("-swing", IN_SwingUp);
static ConCommand startgrenadethrow("+throwgrenade", IN_ThrowGrenadeDown);
static ConCommand endgrenadethrow("-throwgrenade", IN_ThrowGrenadeUp);
static ConCommand startgrenadeswitch("+switchrenade", IN_GrenadeSwitchDown);
static ConCommand endgrenadeswitch("-switchrenade", IN_GrenadeSwitchUp);

static ConCommand startaimmode("+aimmode", IN_AimmodeDown);
static ConCommand endaimmode("-aimmode", IN_AimmodeUp);
static ConCommand startfiremode("+firemode", IN_FireModeDown);
static ConCommand endfiremode("-firemode", IN_FireModeUp);

static ConCommand startDrop("+drop", IN_DropDown);
static ConCommand endDrop("-drop", IN_DropUp);

static ConCommand startswitchmode("+switchmode", IN_SwitchModeDown);
static ConCommand endswitchmode("-switchmode", IN_SwitchModeUp);

static ConCommand startswitchsilencer("+switchsilencer", IN_SwitchSilencerDown);
static ConCommand endswitchsilencer("-switchsilencer", IN_SwitchSilencerUp);
/*static ConCommand startscrollup("+scrollup", IN_ScrollUpDown);
static ConCommand endscrollup("-scrollup", IN_ScrollUpUp);
static ConCommand startscrolldown("+scrolldown", IN_ScrollDownDown);
static ConCommand endscrolldown("-scrolldown", IN_ScrollDownUp);*/
/*static ConCommand startirsight("+iron_sight", IN_IrSightDown);
static ConCommand endirsight("-iron_sight", IN_IrSightUp);
*/
#ifdef TF_CLIENT_DLL
static ConCommand toggle_duck( "toggle_duck", IN_DuckToggle );
#endif

// Xbox 360 stub commands
static ConCommand xboxmove("xmove", IN_XboxStub);
static ConCommand xboxlook("xlook", IN_XboxStub);

/*
============
Init_All
============
*/
void CInput::Init_All (void)
{
	Assert( !m_pCommands );
	m_pCommands = new CUserCmd[ MULTIPLAYER_BACKUP ];
	m_pVerifiedCommands = new CVerifiedUserCmd[ MULTIPLAYER_BACKUP ];

	m_fMouseInitialized	= false;
	m_fRestoreSPI		= false;
	m_fMouseActive		= false;
	Q_memset( m_rgOrigMouseParms, 0, sizeof( m_rgOrigMouseParms ) );
	Q_memset( m_rgNewMouseParms, 0, sizeof( m_rgNewMouseParms ) );
	Q_memset( m_rgCheckMouseParam, 0, sizeof( m_rgCheckMouseParam ) );

	m_rgNewMouseParms[ MOUSE_ACCEL_THRESHHOLD1 ] = 0;	// no 2x
	m_rgNewMouseParms[ MOUSE_ACCEL_THRESHHOLD2 ] = 0;	// no 4x
	m_rgNewMouseParms[ MOUSE_SPEED_FACTOR ] = 1;		// 0 = disabled, 1 = threshold 1 enabled, 2 = threshold 2 enabled

	m_fMouseParmsValid	= false;
	m_fJoystickAdvancedInit = false;
	m_fHadJoysticks = false;
	m_flLastForwardMove = 0.0;

	// Initialize inputs
	if ( IsPC() )
	{
		Init_Mouse ();
		Init_Keyboard();
	}
		
	// Initialize third person camera controls.
	Init_Camera();
}

/*
============
Shutdown_All
============
*/
void CInput::Shutdown_All(void)
{
	DeactivateMouse();
	Shutdown_Keyboard();

	delete[] m_pCommands;
	m_pCommands = NULL;

	delete[] m_pVerifiedCommands;
	m_pVerifiedCommands = NULL;
}

void CInput::LevelInit( void )
{
#if defined( HL2_CLIENT_DLL )
	// Remove any IK information
	m_EntityGroundContact.RemoveAll();
#endif
}

