//====== Copyright © 2008-2018 OVERCHARGED =======
//
// Purpose: 10 Years and still on the road...
//
//=============================================================================

#include <cbase.h>
#include <convar.h>

#define oc_CUSTOMTITLE

//Global
ConVar oc_mapadd_preset("oc_mapadd_preset", "mapadd_overcharged", FCVAR_ARCHIVE, "mapadd preset cfg");
ConVar oc_weapons_scripts_preset("oc_weapons_scripts_preset", "weapons", FCVAR_ARCHIVE, "weapons scripts preset cfg");
ConVar oc_config_default_preset("oc_config_default_preset", "OverCharged_default", FCVAR_ARCHIVE, "config_default preset cfg");
ConVar oc_mapadd_lua_enabled("oc_mapadd_lua_enabled", "1", FCVAR_REPLICATED);


//UNUSED
ConVar oc_weapon_holster("oc_weapon_holster", "0", FCVAR_HIDDEN, "Enable/Disable weapon holster animation.");
//ConVar oc_hud_name("oc_hud_name", "ClientSchemeGreen");
//////////


//Test commands below
ConVar oc_player_true_firstperson_vm("oc_player_true_firstperson_vm", "0", FCVAR_ARCHIVE);//For test
ConVar oc_test_weapon_firerate("oc_test_weapon_firerate", "0", FCVAR_CHEAT);//For test
ConVar oc_particle_impacts("oc_particle_impacts", "0", FCVAR_ARCHIVE);//For test


//ALLY
ConVar oc_ally_allow_friendlyfire("oc_ally_allow_friendlyfire", "0", FCVAR_ARCHIVE, "Friendly fire state (Need to reload map, or respawn all friendly npcs)");
ConVar oc_ally_canbecome_enemy_in_radius("oc_ally_canbecome_enemy_in_radius", "0", FCVAR_ARCHIVE, "Radius for search nearest friendly entity");
ConVar oc_ally_canbecome_enemy_by_class("oc_ally_canbecome_enemy_by_class", "0", FCVAR_ARCHIVE, "1 - means search by same classname; 0 - means search by same relationships");


//WEAPONS
ConVar oc_weapons_allow_rt_blend("oc_weapons_allow_rt_blend", "0", FCVAR_ARCHIVE, "RT Blending from black to normal and conversely");
ConVar oc_weapons_allow_wall_bump("oc_weapons_allow_wall_bump", "0", FCVAR_ARCHIVE, "Wall bumping");
ConVar oc_weapons_infinite_ammo("oc_weapons_infinite_ammo", "0", FCVAR_CHEAT, "Infinity ammo");
ConVar oc_weapons_allow_inspect_animations("oc_weapons_allow_inspect_animations", "0", FCVAR_ARCHIVE, "Allow inspect animations");
ConVar oc_weapons_allow_pickup_animations("oc_weapons_allow_pickup_animations", "0", FCVAR_ARCHIVE, "Allow pickup animations");
ConVar oc_weapons_allow_sprint_lower("oc_weapons_allow_sprint_lower", "0", FCVAR_ARCHIVE, "Lower weapon on sprint. 0 - disabled, 1 - enabled.");
ConVar oc_weapon_disable_dissolve("oc_weapon_disable_dissolve", "0", FCVAR_ARCHIVE);
ConVar oc_weapon_holster_code("oc_weapon_holster_code", "0", FCVAR_ARCHIVE, "Enable/Disable weapon holster animation.");
ConVar oc_weapon_deploy_sound("oc_weapon_deploy_sound", "0", FCVAR_ARCHIVE, "Enable/Disable weapon deploy sound.");
ConVar oc_single_primary_weapon_mode("oc_single_primary_weapon_mode", "0", FCVAR_ARCHIVE, "1 weapon per slot or all weapons can be picked and used.");
ConVar oc_allow_weapon_autopickup("oc_allow_weapon_autopickup", "1", FCVAR_ARCHIVE, "Allow auto pick up weapons or not.");
ConVar oc_is_weapon_dual_pistols("oc_is_weapon_dual_pistols", "0", FCVAR_HIDDEN, "");
ConVar oc_allow_pick_stunbaton("oc_allow_pick_stunbaton", "0", FCVAR_ARCHIVE, "Allow pick up stunstick or not.");
ConVar oc_allow_pick_dual_pistols("oc_allow_pick_dual_pistols", "0", FCVAR_ARCHIVE, "Allow pick up dual pistols or not.");
ConVar oc_weapon_cguard_charge_time("oc_weapon_cguard_charge_time", "0", FCVAR_ARCHIVE, "");	// simple weapon lower on sprint
ConVar oc_weapon_laser_charge_time("oc_weapon_laser_charge_time", "0", FCVAR_ARCHIVE, "");	// simple weapon lower on sprint
ConVar oc_weapon_immolator_charge_time("oc_weapon_immolator_charge_time", "0", FCVAR_ARCHIVE, "");	// simple weapon lower on sprint

void	ChangeFlameThrowerRange();
ConVar oc_weapon_flamethrower_range("oc_weapon_flamethrower_range", "200", FCVAR_ARCHIVE, "", (FnChangeCallback_t)ChangeFlameThrowerRange);	// simple weapon lower on sprint
void	ChangeFlameThrowerRange()
{
	int iVal = oc_weapon_flamethrower_range.GetInt();
	iVal = Clamp(iVal, 10, 1000);
	oc_weapon_flamethrower_range.SetValue(iVal);
}
//ConVar oc_weapon_disable_autoswitch("oc_weapon_disable_autoswitch", "0", FCVAR_ARCHIVE, "Enable/Disable weapon auto switch on low ammo.");

//GRENADES
ConVar oc_grenadetype("oc_grenadetype", "", /*FCVAR_HIDDEN |*/ FCVAR_HIDDEN, "Current grenade type. 0 - Never draw, 1 - Frag, 2 - Flash, 3 - Smoke.");
ConVar oc_gren1ammo("oc_gren1ammo", "0", FCVAR_HIDDEN | FCVAR_REPLICATED, "Hud grenade display");



//NPCs
ConVar oc_npc_allow_jumping("oc_npc_allow_jumping", "0", FCVAR_ARCHIVE, "NPC allows jumping");
ConVar oc_npc_combines_allow_underbarrel_shot("oc_npc_combines_allow_underbarrel_shot", "0", FCVAR_ARCHIVE, "NPC allows underbarrel grenades shot");
ConVar oc_npc_zombies_dont_less_headcrab("oc_npc_zombies_dont_less_headcrab", "0", FCVAR_ARCHIVE);


//VEHICLES
//ConVar oc_vehicle_jeep_episodic_allow_gun("oc_vehicle_jeep_episodic_allow_gun", "0", FCVAR_ARCHIVE, "Vehicle jeep episodic allow gauss gun");


//ENV
ConVar oc_env_fire_light_radius("oc_env_fire_light_radius", "120", FCVAR_ARCHIVE, "Allow remove medkits cvar or not.");


// FX individual settings
ConVar oc_fx_particle_main_explosion_effect1_name("oc_fx_particle_main_explosion_effect1_name", "Explosion_01", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the 1 particle effect of base explosion.");
ConVar oc_fx_particle_main_explosion_effect2_name("oc_fx_particle_main_explosion_effect2_name", "Explosion_02", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the 2 particle effect of base explosion.");
ConVar oc_fx_particle_main_explosion_effect3_name("oc_fx_particle_main_explosion_effect3_name", "Explosion_03", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the 3 particle effect of base explosion.");
ConVar oc_fx_particle_main_explosion_effect4_name("oc_fx_particle_main_explosion_effect4_name", "Explosion_04", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the 4 particle effect of base explosion.");
ConVar oc_fx_particle_main_explosion_effect5_name("oc_fx_particle_main_explosion_effect5_name", "Explosion_05", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the 5 particle effect of base explosion.");

//ConVar oc_fx_particle_explosion_underwater_effect1_name("oc_fx_particle_explosion_underwater_effect1_name", "water_explosion", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the particle effect name of base explosion in water.");
//ConVar oc_fx_particle_explosion_underwater_effect2_name("oc_fx_particle_explosion_underwater_effect2_name", "water_explosion", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the particle effect name of base explosion in water.");
//ConVar oc_fx_particle_explosion_underwater_effect3_name("oc_fx_particle_explosion_underwater_effect3_name", "water_explosion", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the particle effect name of base explosion in water.");
//ConVar oc_fx_particle_explosion_underwater_effect4_name("oc_fx_particle_explosion_underwater_effect4_name", "water_explosion", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the particle effect name of base explosion in water.");
//ConVar oc_fx_particle_explosion_underwater_effect5_name("oc_fx_particle_explosion_underwater_effect5_name", "water_explosion", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the particle effect name of base explosion in water.");

ConVar oc_fx_particle_helibomb_explosion_effect_name("oc_fx_particle_helibomb_explosion_effect_name", "Heli_Bomb", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the particle effect for helibomb explosion.");
ConVar oc_fx_particle_crossbowbolt_explosion_effect_name("oc_fx_particle_crossbowbolt_explosion_effect_name", "XBowBolt_Explosion", FCVAR_REPLICATED | FCVAR_ARCHIVE, "Sets the particle effect for crossbow bolt explosion.");

//PLAYER
ConVar oc_player_bloodoverlay("oc_player_bloodoverlay", "1", FCVAR_ARCHIVE, "Enable blood decal on viewmodels");
ConVar oc_player_bloodoverlay_yellow_count("oc_player_bloodoverlay_yellow_count", "15", FCVAR_ARCHIVE | FCVAR_HIDDEN, "The count of green blood overlays(frames)");
ConVar oc_player_bloodoverlay_red_count("oc_player_bloodoverlay_red_count", "15", FCVAR_ARCHIVE | FCVAR_HIDDEN, "The count of red blood overlays(frames)");
ConVar oc_player_bloodoverlay_green_count("oc_player_bloodoverlay_green_count", "15", FCVAR_ARCHIVE | FCVAR_HIDDEN, "The count of green blood overlays(frames)");
ConVar oc_player_bloodoverlay_blue_count("oc_player_bloodoverlay_blue_count", "15", FCVAR_ARCHIVE | FCVAR_HIDDEN, "The count of acid blood overlays(frames)");
ConVar oc_player_bloodoverlay_lifetime("oc_player_bloodoverlay_lifetime", "35.0", FCVAR_ARCHIVE, "The life time of blood overlays");
ConVar oc_playermodel_suit("oc_playermodel_suit", "models/player/gordon_hev.mdl", FCVAR_ARCHIVE, "Set suit player model for first person.");
ConVar oc_playermodel_nosuit("oc_playermodel_nosuit", "models/player/gordon_citizen.mdl", FCVAR_ARCHIVE, "Set no suit player model for first person.");
ConVar oc_player_weapons_limit("oc_player_weapons_limit", "0", FCVAR_ARCHIVE, "Player max weapons carrying. 0 - unlimited, 1,2,3... - limit");
ConVar oc_player_allow_drop_used_perks("oc_player_allow_drop_used_perks", "0", FCVAR_ARCHIVE, "Player drops empty perks");
ConVar oc_player_allow_fast_gren_throw("oc_player_allow_fast_gren_throw", "0", FCVAR_ARCHIVE, "Player fast grenade throwing");
ConVar oc_player_allow_animated_camera("oc_player_allow_animated_camera", "0", FCVAR_ARCHIVE, "Player allow anim-event camera");
ConVar oc_player_allow_nightvision("oc_player_allow_nightvision", "0", FCVAR_ARCHIVE, "Player allow nightvision");
ConVar oc_player_flashlight_onweapon("oc_player_flashlight_onweapon", "0", FCVAR_ARCHIVE, "Flashlight attached to weapon attachment. default 'muzzle'.");
ConVar oc_player_animated_camera_scale("oc_player_animated_camera_scale", "0", FCVAR_ARCHIVE, "Player's animated camera scale");
ConVar oc_player_camera_landing_bob("oc_player_camera_landing_bob", "0", FCVAR_ARCHIVE, "Enable/Disable landing bob");
ConVar oc_player_camera_landing_bob_scale("oc_player_camera_landing_bob_scale", "0", FCVAR_ARCHIVE, "Enable/Disable landing bob scale");
ConVar oc_player_death_sound("oc_player_death_sound", "0", FCVAR_ARCHIVE, "Play music if player is dead.");
ConVar oc_player_death_sound_name("oc_player_death_sound_name", "Music/HL2_song28.mp3", FCVAR_ARCHIVE, "Name of the death music.");
ConVar oc_player_nofalldamage("oc_player_nofalldamage", "0", FCVAR_ARCHIVE, "Disable fall damage like in portal game.");
ConVar oc_playerview_attach_deathcam("oc_playerview_attach_deathcam", "0", FCVAR_ARCHIVE, "First person Attach/Detach player death cam to player ragdoll model, default attachment index 1.");
ConVar oc_playerview_draw_body("oc_playerview_draw_body", "0", FCVAR_ARCHIVE, "Draw player body after death & cam attach");
ConVar viewmodel_fov("viewmodel_fov", "54", FCVAR_ARCHIVE, "Enable/Disable weapon holster animation.");
ConVar oc_player_slowmo_timescale("oc_player_slowmo_timescale", "0.3", FCVAR_ARCHIVE, "Slow Motion regen delay");
ConVar oc_player_slowmo_increase_delay("oc_player_slowmo_increase_delay", "0.1", FCVAR_ARCHIVE, "Slow Motion regen delay");
ConVar oc_player_slowmo_decrease_delay("oc_player_slowmo_decrease_delay", "0.02", FCVAR_ARCHIVE, "Slow Motion drop delay");
ConVar oc_player_weap_selection_enable_fade("oc_player_weap_selection_enable_fade", "1", FCVAR_ARCHIVE, "Enable open selection menu time change");
//weapon_selection.cpp - oc_player_weap_selection_enable_fade
//weapon_selection.cpp - oc_player_weap_selection_time_threshold

//ConVar oc_player_weap_selection_fadein("oc_player_weap_selection_fadein", "3.0", FCVAR_ARCHIVE, "Selection menu time fadein");
//ConVar oc_player_weap_selection_fadeout("oc_player_weap_selection_fadeout", "3.0", FCVAR_ARCHIVE, "Selection menu time fadeout");

//TODO: REFACTOR BELOW
//FREE AIM 
ConVar oc_weapon_free_aim("oc_weapon_free_aim", "0", FCVAR_ARCHIVE, "Player is firing state.");
ConVar oc_using_entity("oc_using_entity", "0", FCVAR_HIDDEN, "");


// BriJee: Important! Game state hidden commands.
ConVar oc_state_PlrIsDead("oc_state_PlrIsDead", "0", FCVAR_HIDDEN, "State if player is dead.");
ConVar oc_state_is_running("oc_state_is_running", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_is_runningLAG("oc_state_is_runningLAG", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_player_velocity("oc_state_player_velocity", "0", FCVAR_HIDDEN, "Player is firing state.");



//GUNPLAY
ConVar oc_weapons_enable_dynamic_bullets("oc_weapons_enable_dynamic_bullets", "1", FCVAR_ARCHIVE, "Dynamic bullets. 0 - disabled, 1 - enabled, 2 - ebabled only in slowmo mode");
ConVar oc_weapons_enable_dynamic_bullets_penetration("oc_weapons_enable_dynamic_bullets_penetration", "0", FCVAR_ARCHIVE, "Dynamic bullets. 0 - disabled, 1 - enabled, 2 - ebabled only in slowmo mode");
ConVar oc_weapons_enable_drop_mags("oc_weapons_enable_drop_mags", "1", FCVAR_ARCHIVE, "Dropping empty clips");


//PROJECTED MUZZLE FLASHES
ConVar oc_ProjectedMuzzleFlash_LifeTime("oc_ProjectedMuzzleFlash_LifeTime", "0.01", FCVAR_ARCHIVE);
ConVar oc_ProjectedMuzzleFlash_SpeedOfTime("oc_ProjectedMuzzleFlash_SpeedOfTime", "0.5", FCVAR_ARCHIVE);
ConVar oc_ProjectedMuzzleFlash_test("oc_ProjectedMuzzleFlash_test", "0", FCVAR_REPLICATED);



//WEAPON STATES
ConVar oc_ironsight_type("oc_ironsight_type", "0", FCVAR_ARCHIVE, "Player reload state.");
ConVar oc_state_IRsight_on("oc_state_IRsight_on", "0", FCVAR_HIDDEN, "Player iron sight state.");
ConVar oc_state_InSecondFire("oc_state_InSecondFire", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_IRsight_bytime("oc_state_IRsight_bytime", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_InSecondFire_sniper("oc_state_InSecondFire_sniper", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_InSecondFire_crossbow("oc_state_InSecondFire_crossbow", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_InSecondFire_sg552("oc_state_InSecondFire_sg552", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_InSecondFire_357("oc_state_InSecondFire_357", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_InSecondFire_oicw("oc_state_InSecondFire_oicw", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_state_near_wall_standing("oc_state_near_wall_standing", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar oc_use_second_ballel("oc_use_second_ballel", "0", FCVAR_HIDDEN | FCVAR_ARCHIVE, "Player is firing state.");


//WEAPON CLIENT-SERVER COMMUNICATE
ConVar oc_recoil_x("oc_recoil_x", "0", FCVAR_HIDDEN);
ConVar oc_recoil_y("oc_recoil_y", "0", FCVAR_HIDDEN);

//RAGDOLL
ConVar oc_ragdoll_show_blood("oc_ragdoll_show_blood", "0", FCVAR_ARCHIVE, "Player is firing state.");
ConVar oc_ragdoll_serverside("oc_ragdoll_serverside", "0", FCVAR_ARCHIVE, "Enable/Disable weapon holster animation.");
ConVar oc_ragdoll_death_weapon_holding("oc_ragdoll_death_weapon_holding", "0", FCVAR_ARCHIVE, "Enable/Disable weapon hold after death");
ConVar oc_ragdoll_death_weapon_holding_rand_range("oc_ragdoll_death_weapon_holding_rand_range", "0", FCVAR_ARCHIVE, "random posibility; more value - less posibility, zero value - always");
ConVar oc_ragdoll_death_weapon_holding_time("oc_ragdoll_death_weapon_holding_time", "0", FCVAR_ARCHIVE, "Weapon holding time");
ConVar oc_ragdoll_death_weapon_firing("oc_ragdoll_death_weapon_firing", "0", FCVAR_ARCHIVE, "Enable/Disable weapon firing after death (percentage of clip size; zero value - disable)");
ConVar oc_ragdoll_enable_blooddrips("oc_ragdoll_enable_blooddrips", "0", FCVAR_ARCHIVE, "Enable/Disable ragdoll blood drips");
ConVar oc_ragdoll_enable_bloodstream("oc_ragdoll_enable_bloodstream", "0", FCVAR_ARCHIVE, "Enable/Disable ragdoll blood stream");
ConVar oc_ragdoll_serverside_interrupt("oc_ragdoll_serverside_interrupt", "0", FCVAR_HIDDEN);
ConVar oc_ragdoll_serverside_dissolve_impulse("oc_ragdoll_serverside_dissolve_impulse", "0", FCVAR_ARCHIVE, "Enable/Disable weapon holster animation.");
ConVar oc_ragdoll_serverside_collision_type("oc_ragdoll_serverside_collision_type", "0", FCVAR_ARCHIVE, "Enable/Disable weapon holster animation.");
ConVar oc_ragdoll_serverside_collision_group("oc_ragdoll_serverside_collision_group", "0", FCVAR_ARCHIVE, "Enable/Disable weapon holster animation.");



//MAPADD TEST (UNUSED)
ConVar oc_mapadd_changelevel("oc_mapadd_changelevel", "0", FCVAR_HIDDEN);
ConVar PlayerHealth("PlayerHealth", "0", FCVAR_HIDDEN);
ConVar PlayerArmor("PlayerArmor", "0", FCVAR_HIDDEN);
ConVar weapon_airboatgun("weapon_airboatgun", "0", FCVAR_HIDDEN);
ConVar weapon_bunkergun("weapon_bunkergun", "0", FCVAR_HIDDEN);
ConVar Weapon_ak47("Weapon_ak47", "0", FCVAR_HIDDEN);
ConVar Weapon_autoshotgun("Weapon_autoshotgun", "0", FCVAR_HIDDEN);
ConVar Weapon_barnacle("Weapon_barnacle", "0", FCVAR_HIDDEN);
ConVar Weapon_beretta("Weapon_beretta", "0", FCVAR_HIDDEN);
ConVar Weapon_binoculars("Weapon_binoculars", "0", FCVAR_HIDDEN);
ConVar Weapon_bee("Weapon_bee", "0", FCVAR_HIDDEN);
ConVar Weapon_adrenaline("Weapon_adrenaline", "0", FCVAR_HIDDEN);
ConVar Weapon_cguard("Weapon_cguard", "0", FCVAR_HIDDEN);
ConVar Weapon_deagle("Weapon_deagle", "0", FCVAR_HIDDEN);
ConVar Weapon_disc("Weapon_disc", "0", FCVAR_HIDDEN);
ConVar Weapon_displacer("Weapon_displacer", "0", FCVAR_HIDDEN);
ConVar Weapon_dual_pistols("Weapon_dual_pistols", "0", FCVAR_HIDDEN);
ConVar Weapon_egon("Weapon_egon", "0", FCVAR_HIDDEN);
ConVar Weapon_extinguisher("Weapon_extinguisher", "0", FCVAR_HIDDEN);
ConVar Weapon_fireball("Weapon_fireball", "0", FCVAR_HIDDEN);
ConVar Weapon_flamethrower("Weapon_flamethrower", "0", FCVAR_HIDDEN);
ConVar Weapon_flaregun("Weapon_flaregun", "0", FCVAR_HIDDEN);
ConVar Weapon_gauss("Weapon_gauss", "0", FCVAR_HIDDEN);
ConVar Weapon_glock("Weapon_glock", "0", FCVAR_HIDDEN);
ConVar Weapon_grapple("Weapon_grapple", "0", FCVAR_HIDDEN);
ConVar Weapon_grenadelauncher("Weapon_grenadelauncher", "0", FCVAR_HIDDEN);
ConVar Weapon_hopwire("Weapon_hopwire", "0", FCVAR_HIDDEN);
ConVar Weapon_iceaxe("Weapon_iceaxe", "0", FCVAR_HIDDEN);
ConVar Weapon_immolator("Weapon_immolator", "0", FCVAR_HIDDEN);
ConVar Weapon_ionrifle("Weapon_ionrifle", "0", FCVAR_HIDDEN);
ConVar Weapon_knife("Weapon_knife", "0", FCVAR_HIDDEN);
ConVar Weapon_laser("Weapon_laser", "0", FCVAR_HIDDEN);
ConVar Weapon_laserpistol("Weapon_laserpistol", "0", FCVAR_HIDDEN);
ConVar Weapon_m4("Weapon_m4", "0", FCVAR_HIDDEN);
ConVar Weapon_machinegun("Weapon_machinegun", "0", FCVAR_HIDDEN);
ConVar Weapon_molotov("Weapon_molotov", "0", FCVAR_HIDDEN);
ConVar Weapon_mp5("Weapon_mp5", "0", FCVAR_HIDDEN);
ConVar Weapon_objective("Weapon_objective", "0", FCVAR_HIDDEN);
ConVar Weapon_oicw("Weapon_oicw", "0", FCVAR_HIDDEN);
ConVar Weapon_physgun("Weapon_physgun", "0", FCVAR_HIDDEN);
ConVar Weapon_pistol_silent("Weapon_pistol_silent", "0", FCVAR_HIDDEN);
ConVar Weapon_sg552("Weapon_sg552", "0", FCVAR_HIDDEN);
ConVar Weapon_shockrifle("Weapon_shockrifle", "0", FCVAR_HIDDEN);
ConVar Weapon_smg2("Weapon_smg2", "0", FCVAR_HIDDEN);
ConVar Weapon_smokegrenade("Weapon_smokegrenade", "0", FCVAR_HIDDEN);
ConVar Weapon_sniper("Weapon_sniper", "0", FCVAR_HIDDEN);
ConVar Weapon_teleport("Weapon_teleport", "0", FCVAR_HIDDEN);
ConVar Weapon_thumper("Weapon_thumper", "0", FCVAR_HIDDEN);
ConVar Weapon_wrench("Weapon_wrench", "0", FCVAR_HIDDEN);
ConVar grenade_ar2("grenade_ar2", "0", FCVAR_HIDDEN);
ConVar Weapon_frag("Weapon_frag", "0", FCVAR_HIDDEN);
ConVar Weapon_bugbait("Weapon_bugbait", "0", FCVAR_HIDDEN);
ConVar Weapon_357("Weapon_357", "0", FCVAR_HIDDEN);
ConVar Weapon_pistol("Weapon_pistol", "0", FCVAR_HIDDEN);
ConVar Weapon_ar2("Weapon_ar2", "0", FCVAR_HIDDEN);
ConVar Weapon_smg1("Weapon_smg1", "0", FCVAR_HIDDEN);
ConVar Weapon_rpg("Weapon_rpg", "0", FCVAR_HIDDEN);
ConVar Weapon_shotgun("Weapon_shotgun", "0", FCVAR_HIDDEN);
ConVar Weapon_stunstick("Weapon_stunstick", "0", FCVAR_HIDDEN);
ConVar Weapon_crowbar("Weapon_crowbar", "0", FCVAR_HIDDEN);
ConVar Weapon_cubemap("Weapon_cubemap", "0", FCVAR_HIDDEN);
ConVar Weapon_crossbow("Weapon_crossbow", "0", FCVAR_HIDDEN);
ConVar Weapon_physcannon("Weapon_physcannon", "0", FCVAR_HIDDEN);
ConVar Weapon_alyxgun("Weapon_alyxgun", "0", FCVAR_HIDDEN);
ConVar Weapon_annabelle("Weapon_annabelle", "0", FCVAR_HIDDEN);
ConVar item_suit("item_suit", "0", FCVAR_HIDDEN);
ConVar last_weapon("last_weapon", "", FCVAR_HIDDEN);


//Vectors
ConVar oc_muzzle_tracer_x("oc_muzzle_tracer_x", "0", FCVAR_HIDDEN);
ConVar oc_muzzle_tracer_y("oc_muzzle_tracer_y", "0", FCVAR_HIDDEN);
ConVar oc_muzzle_tracer_z("oc_muzzle_tracer_z", "0", FCVAR_HIDDEN);

ConVar oc_muzzle_vector_x("oc_muzzle_vector_x", "0", FCVAR_HIDDEN);
ConVar oc_muzzle_vector_y("oc_muzzle_vector_y", "0", FCVAR_HIDDEN);
ConVar oc_muzzle_vector_z("oc_muzzle_vector_z", "0", FCVAR_HIDDEN);

ConVar oc_muzzle_dynamic_vector_x("oc_muzzle_dynamic_vector_x", "0", FCVAR_HIDDEN);
ConVar oc_muzzle_dynamic_vector_y("oc_muzzle_dynamic_vector_y", "0", FCVAR_HIDDEN);
ConVar oc_muzzle_dynamic_vector_z("oc_muzzle_dynamic_vector_z", "0", FCVAR_HIDDEN);

ConVar oc_muzzle_angle_x("oc_muzzle_angle_x", "0", FCVAR_HIDDEN);
ConVar oc_muzzle_angle_y("oc_muzzle_angle_y", "0", FCVAR_HIDDEN);
ConVar oc_muzzle_angle_z("oc_muzzle_angle_z", "0", FCVAR_HIDDEN);

ConVar oc_RT_vector_x("oc_RT_vector_x", "0", FCVAR_HIDDEN);
ConVar oc_RT_vector_y("oc_RT_vector_y", "0", FCVAR_HIDDEN);
ConVar oc_RT_vector_z("oc_RT_vector_z", "0", FCVAR_HIDDEN);

ConVar oc_viewport_vector_x("oc_viewport_vector_x", "0", FCVAR_HIDDEN);
ConVar oc_viewport_vector_y("oc_viewport_vector_y", "0", FCVAR_HIDDEN);
ConVar oc_viewport_vector_z("oc_viewport_vector_z", "0", FCVAR_HIDDEN);





ConVar oc_level_shutdown("oc_level_shutdown", "0", FCVAR_HIDDEN, "Player is firing state.");
ConVar aa_cammode("aa_cammode", "0", FCVAR_ARCHIVE);
ConVar oc_weapon_is_lowered("oc_weapon_is_lowered", "0", FCVAR_HIDDEN);


//For testing positions and angles
ConVar oc_mag_test_pos_enabled("oc_mag_test_pos_enabled", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_pos_x("oc_mag_test_pos_x", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_pos_y("oc_mag_test_pos_y", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_pos_z("oc_mag_test_pos_z", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_ang_x("oc_mag_test_ang_x", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_ang_y("oc_mag_test_ang_y", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_ang_z("oc_mag_test_ang_z", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_vel_enabled("oc_mag_test_vel_enabled", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_vel_far("oc_mag_test_vel_far", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_vel_x("oc_mag_test_vel_x", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_vel_y("oc_mag_test_vel_y", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_vel_z("oc_mag_test_vel_z", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_angvel_x("oc_mag_test_angvel_x", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_angvel_y("oc_mag_test_angvel_y", "0", FCVAR_REPLICATED);
ConVar oc_mag_test_angvel_z("oc_mag_test_angvel_z", "0", FCVAR_REPLICATED);


void SharedProject_Information(void)
{
#ifdef oc_CUSTOMTITLE
	// Active title
	Warning("========================================Project OVERCHARGED================================\n", FCVAR_ARCHIVE);
	//Msg("Project Overcharged is a big HL2 enhancement mod.\n", FCVAR_ARCHIVE);
	//Msg("TEXT TEXT TEXT.\n", FCVAR_ARCHIVE);
	//Msg("In this mod you can TEXT TEXT TEXT.\n", FCVAR_ARCHIVE);
	//Msg("Project made by TEXT TEXT TEXT.\n", FCVAR_ARCHIVE);
	//Warning("===========================================================================================\n", FCVAR_ARCHIVE);

#else

	// Custom title
	Warning("========================================INFO===============================================\n", FCVAR_ARCHIVE);
	Msg("TEXT.\n", FCVAR_ARCHIVE);
	Msg("TEXT.\n", FCVAR_ARCHIVE);
	Msg("TEXT.\n", FCVAR_ARCHIVE);
	Msg("TEXT. \n", FCVAR_ARCHIVE);
	Warning("===========================================================================================\n", FCVAR_ARCHIVE);
#endif
	Msg("Build 11.5 \n", FCVAR_ARCHIVE);
	/*
	Msg("Fixed: \n", FCVAR_ARCHIVE);
	Msg(" \n", FCVAR_ARCHIVE);
	Msg(" \n");

	Msg("Changed: \n", FCVAR_ARCHIVE);
	Msg(" \n");

	Msg("Added: \n", FCVAR_ARCHIVE);
	Msg("New weapons, weapon lower \n");
	Msg(" \n");
	*/

	Warning("===========================================================================================\n", FCVAR_ARCHIVE);
}

ConCommand oc_about("oc_about", SharedProject_Information, "Show information about project.", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);

/*
// Additional info
void SharedProject_Copyright(void)
{
#ifdef oc_CUSTOMTITLE
	Warning("======================================COPYRIGHT=========================================\n", FCVAR_ARCHIVE);
	Msg(" copyright :\n", FCVAR_ARCHIVE);
	Warning("===========================================================================================\n", FCVAR_ARCHIVE);

#else

	Warning("======================================COPYRIGHT============================================\n", FCVAR_ARCHIVE);
	Msg(" copyright :\n", FCVAR_ARCHIVE);
	Warning("===========================================================================================\n", FCVAR_ARCHIVE);
#endif
}

ConCommand oc_copyright("oc_copyright", SharedProject_Copyright, "Shows copyright info about project.", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
*/

// BriJee - TEST all medkits remove
static void RemoveMedkits_cvar(void)
{
	if (cvar->FindVar("oc_remove_medkits")->GetInt() == 1)
	{
		engine->ClientCmd("ent_remove_all item_healthvial\n");
		engine->ClientCmd("ent_remove_all item_healthkit\n");
	}
	//DevMsg("All medkits removed from the map.");
}
static ConCommand oc_remove_medkits("oc_remove_medkits", RemoveMedkits_cvar);


/*
// BriJee: Custom script 1
ConVar oc_customcfgscript1_value("oc_customcfgscript1_value", "0", FCVAR_ARCHIVE, "Custom config script 1.");
static void custom_script1_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 1)
		engine->ClientCmd("exec custom1cfg1");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 2)
		engine->ClientCmd("exec custom1cfg2");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 3)
		engine->ClientCmd("exec custom1cfg3");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 4)
		engine->ClientCmd("exec custom1cfg4");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 5)
		engine->ClientCmd("exec custom1cfg5");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 6)
		engine->ClientCmd("exec custom1cfg6");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 7)
		engine->ClientCmd("exec custom1cfg7");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 8)
		engine->ClientCmd("exec custom1cfg8");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 9)
		engine->ClientCmd("exec custom1cfg9");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 10)
		engine->ClientCmd("exec custom1cfg10");

}
static ConCommand oc_customcfgscript1("oc_customcfgscript1", custom_script1_cvar);

// BriJee: Custom script 2
ConVar oc_customcfgscript2_value("oc_customcfgscript2_value", "0", FCVAR_ARCHIVE, "Custom config script 2.");
static void custom_script2_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 1)
		engine->ClientCmd("exec custom2cfg1");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 2)
		engine->ClientCmd("exec custom2cfg2");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 3)
		engine->ClientCmd("exec custom2cfg3");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 4)
		engine->ClientCmd("exec custom2cfg4");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 5)
		engine->ClientCmd("exec custom2cfg5");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 6)
		engine->ClientCmd("exec custom2cfg6");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 7)
		engine->ClientCmd("exec custom2cfg7");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 8)
		engine->ClientCmd("exec custom2cfg8");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 9)
		engine->ClientCmd("exec custom2cfg9");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 10)
		engine->ClientCmd("exec custom2cfg10");

}
static ConCommand oc_customcfgscript2("oc_customcfgscript2", custom_script2_cvar);

// BriJee: Custom script 3
ConVar oc_customcfgscript3_value("oc_customcfgscript3_value", "0", FCVAR_ARCHIVE, "Custom config script 3.");
static void custom_script3_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 1)
		engine->ClientCmd("exec custom3cfg1");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 2)
		engine->ClientCmd("exec custom3cfg2");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 3)
		engine->ClientCmd("exec custom3cfg3");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 4)
		engine->ClientCmd("exec custom3cfg4");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 5)
		engine->ClientCmd("exec custom3cfg5");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 6)
		engine->ClientCmd("exec custom3cfg6");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 7)
		engine->ClientCmd("exec custom3cfg7");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 8)
		engine->ClientCmd("exec custom3cfg8");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 9)
		engine->ClientCmd("exec custom3cfg9");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 10)
		engine->ClientCmd("exec custom3cfg10");

}
static ConCommand oc_customcfgscript3("oc_customcfgscript3", custom_script3_cvar);

// BriJee: Custom script 4
ConVar oc_customcfgscript4_value("oc_customcfgscript4_value", "0", FCVAR_ARCHIVE, "Custom config script 4.");
static void custom_script4_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 1)
		engine->ClientCmd("exec custom4cfg1");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 2)
		engine->ClientCmd("exec custom4cfg2");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 3)
		engine->ClientCmd("exec custom4cfg3");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 4)
		engine->ClientCmd("exec custom4cfg4");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 5)
		engine->ClientCmd("exec custom4cfg5");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 6)
		engine->ClientCmd("exec custom4cfg6");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 7)
		engine->ClientCmd("exec custom4cfg7");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 8)
		engine->ClientCmd("exec custom4cfg8");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 9)
		engine->ClientCmd("exec custom4cfg9");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 10)
		engine->ClientCmd("exec custom4cfg10");

}
static ConCommand oc_customcfgscript4("oc_customcfgscript4", custom_script4_cvar);

// BriJee: Custom script 5
ConVar oc_customcfgscript5_value("oc_customcfgscript5_value", "0", FCVAR_ARCHIVE, "Custom config script 5.");
static void custom_script5_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 1)
		engine->ClientCmd("exec custom5cfg1");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 2)
		engine->ClientCmd("exec custom5cfg2");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 3)
		engine->ClientCmd("exec custom5cfg3");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 4)
		engine->ClientCmd("exec custom5cfg4");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 5)
		engine->ClientCmd("exec custom5cfg5");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 6)
		engine->ClientCmd("exec custom5cfg6");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 7)
		engine->ClientCmd("exec custom5cfg7");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 8)
		engine->ClientCmd("exec custom5cfg8");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 9)
		engine->ClientCmd("exec custom5cfg9");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 10)
		engine->ClientCmd("exec custom5cfg10");

}
static ConCommand oc_customcfgscript5("oc_customcfgscript5", custom_script5_cvar);
*/

// 1
ConVar oc_customcfgscript1_value("oc_customcfgscript1_value", "0", FCVAR_ARCHIVE, "Custom config script 1.");
static void custom_script1_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 1)
		engine->ClientCmd("exec custom1cfg1");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 2)
		engine->ClientCmd("exec custom1cfg2");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 3)
		engine->ClientCmd("exec custom1cfg3");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 4)
		engine->ClientCmd("exec custom1cfg4");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 5)
		engine->ClientCmd("exec custom1cfg5");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 6)
		engine->ClientCmd("exec custom1cfg6");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 7)
		engine->ClientCmd("exec custom1cfg7");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 8)
		engine->ClientCmd("exec custom1cfg8");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 9)
		engine->ClientCmd("exec custom1cfg9");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 10)
		engine->ClientCmd("exec custom1cfg10");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 11)
		engine->ClientCmd("exec custom1cfg11");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 12)
		engine->ClientCmd("exec custom1cfg12");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 13)
		engine->ClientCmd("exec custom1cfg13");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 14)
		engine->ClientCmd("exec custom1cfg14");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 15)
		engine->ClientCmd("exec custom1cfg15");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 16)
		engine->ClientCmd("exec custom1cfg16");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 17)
		engine->ClientCmd("exec custom1cfg17");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 18)
		engine->ClientCmd("exec custom1cfg18");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 19)
		engine->ClientCmd("exec custom1cfg19");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 20)
		engine->ClientCmd("exec custom1cfg20");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 21)
		engine->ClientCmd("exec custom1cfg21");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 22)
		engine->ClientCmd("exec custom1cfg22");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 23)
		engine->ClientCmd("exec custom1cfg23");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 24)
		engine->ClientCmd("exec custom1cfg24");
	if (cvar->FindVar("oc_customcfgscript1_value")->GetInt() == 25)
		engine->ClientCmd("exec custom1cfg25");
}
static ConCommand oc_customcfgscript1("oc_customcfgscript1", custom_script1_cvar);

// 2
ConVar oc_customcfgscript2_value("oc_customcfgscript2_value", "0", FCVAR_ARCHIVE, "Custom config script 2.");
static void custom_script2_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 1)
		engine->ClientCmd("exec custom2cfg1");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 2)
		engine->ClientCmd("exec custom2cfg2");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 3)
		engine->ClientCmd("exec custom2cfg3");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 4)
		engine->ClientCmd("exec custom2cfg4");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 5)
		engine->ClientCmd("exec custom2cfg5");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 6)
		engine->ClientCmd("exec custom2cfg6");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 7)
		engine->ClientCmd("exec custom2cfg7");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 8)
		engine->ClientCmd("exec custom2cfg8");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 9)
		engine->ClientCmd("exec custom2cfg9");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 10)
		engine->ClientCmd("exec custom2cfg10");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 11)
		engine->ClientCmd("exec custom2cfg11");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 12)
		engine->ClientCmd("exec custom2cfg12");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 13)
		engine->ClientCmd("exec custom2cfg13");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 14)
		engine->ClientCmd("exec custom2cfg14");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 15)
		engine->ClientCmd("exec custom2cfg15");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 16)
		engine->ClientCmd("exec custom2cfg16");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 17)
		engine->ClientCmd("exec custom2cfg17");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 18)
		engine->ClientCmd("exec custom2cfg18");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 19)
		engine->ClientCmd("exec custom2cfg19");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 20)
		engine->ClientCmd("exec custom2cfg20");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 21)
		engine->ClientCmd("exec custom2cfg21");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 22)
		engine->ClientCmd("exec custom2cfg22");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 23)
		engine->ClientCmd("exec custom2cfg23");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 24)
		engine->ClientCmd("exec custom2cfg24");
	if (cvar->FindVar("oc_customcfgscript2_value")->GetInt() == 25)
		engine->ClientCmd("exec custom2cfg25");
}
static ConCommand oc_customcfgscript2("oc_customcfgscript2", custom_script2_cvar);

// 3
ConVar oc_customcfgscript3_value("oc_customcfgscript3_value", "0", FCVAR_ARCHIVE, "Custom config script 3.");
static void custom_script3_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 1)
		engine->ClientCmd("exec custom3cfg1");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 2)
		engine->ClientCmd("exec custom3cfg2");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 3)
		engine->ClientCmd("exec custom3cfg3");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 4)
		engine->ClientCmd("exec custom3cfg4");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 5)
		engine->ClientCmd("exec custom3cfg5");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 6)
		engine->ClientCmd("exec custom3cfg6");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 7)
		engine->ClientCmd("exec custom3cfg7");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 8)
		engine->ClientCmd("exec custom3cfg8");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 9)
		engine->ClientCmd("exec custom3cfg9");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 10)
		engine->ClientCmd("exec custom3cfg10");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 11)
		engine->ClientCmd("exec custom3cfg11");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 12)
		engine->ClientCmd("exec custom3cfg12");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 13)
		engine->ClientCmd("exec custom3cfg13");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 14)
		engine->ClientCmd("exec custom3cfg14");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 15)
		engine->ClientCmd("exec custom3cfg15");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 16)
		engine->ClientCmd("exec custom3cfg16");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 17)
		engine->ClientCmd("exec custom3cfg17");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 18)
		engine->ClientCmd("exec custom3cfg18");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 19)
		engine->ClientCmd("exec custom3cfg19");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 20)
		engine->ClientCmd("exec custom3cfg20");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 21)
		engine->ClientCmd("exec custom3cfg21");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 22)
		engine->ClientCmd("exec custom3cfg22");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 23)
		engine->ClientCmd("exec custom3cfg23");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 24)
		engine->ClientCmd("exec custom3cfg24");
	if (cvar->FindVar("oc_customcfgscript3_value")->GetInt() == 25)
		engine->ClientCmd("exec custom3cfg25");
}
static ConCommand oc_customcfgscript3("oc_customcfgscript3", custom_script3_cvar);

// 4
ConVar oc_customcfgscript4_value("oc_customcfgscript4_value", "0", FCVAR_ARCHIVE, "Custom config script 4.");
static void custom_script4_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 1)
		engine->ClientCmd("exec custom4cfg1");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 2)
		engine->ClientCmd("exec custom4cfg2");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 3)
		engine->ClientCmd("exec custom4cfg3");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 4)
		engine->ClientCmd("exec custom4cfg4");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 5)
		engine->ClientCmd("exec custom4cfg5");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 6)
		engine->ClientCmd("exec custom4cfg6");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 7)
		engine->ClientCmd("exec custom4cfg7");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 8)
		engine->ClientCmd("exec custom4cfg8");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 9)
		engine->ClientCmd("exec custom4cfg9");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 10)
		engine->ClientCmd("exec custom4cfg10");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 11)
		engine->ClientCmd("exec custom4cfg11");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 12)
		engine->ClientCmd("exec custom4cfg12");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 13)
		engine->ClientCmd("exec custom4cfg13");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 14)
		engine->ClientCmd("exec custom4cfg14");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 15)
		engine->ClientCmd("exec custom4cfg15");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 16)
		engine->ClientCmd("exec custom4cfg16");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 17)
		engine->ClientCmd("exec custom4cfg17");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 18)
		engine->ClientCmd("exec custom4cfg18");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 19)
		engine->ClientCmd("exec custom4cfg19");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 20)
		engine->ClientCmd("exec custom4cfg20");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 21)
		engine->ClientCmd("exec custom4cfg21");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 22)
		engine->ClientCmd("exec custom4cfg22");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 23)
		engine->ClientCmd("exec custom4cfg23");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 24)
		engine->ClientCmd("exec custom4cfg24");
	if (cvar->FindVar("oc_customcfgscript4_value")->GetInt() == 25)
		engine->ClientCmd("exec custom4cfg25");
}
static ConCommand oc_customcfgscript4("oc_customcfgscript4", custom_script4_cvar);

// 5
ConVar oc_customcfgscript5_value("oc_customcfgscript5_value", "0", FCVAR_ARCHIVE, "Custom config script 5.");
static void custom_script5_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 1)
		engine->ClientCmd("exec custom5cfg1");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 2)
		engine->ClientCmd("exec custom5cfg2");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 3)
		engine->ClientCmd("exec custom5cfg3");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 4)
		engine->ClientCmd("exec custom5cfg4");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 5)
		engine->ClientCmd("exec custom5cfg5");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 6)
		engine->ClientCmd("exec custom5cfg6");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 7)
		engine->ClientCmd("exec custom5cfg7");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 8)
		engine->ClientCmd("exec custom5cfg8");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 9)
		engine->ClientCmd("exec custom5cfg9");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 10)
		engine->ClientCmd("exec custom5cfg10");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 11)
		engine->ClientCmd("exec custom5cfg11");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 12)
		engine->ClientCmd("exec custom5cfg12");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 13)
		engine->ClientCmd("exec custom5cfg13");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 14)
		engine->ClientCmd("exec custom5cfg14");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 15)
		engine->ClientCmd("exec custom5cfg15");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 16)
		engine->ClientCmd("exec custom5cfg16");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 17)
		engine->ClientCmd("exec custom5cfg17");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 18)
		engine->ClientCmd("exec custom5cfg18");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 19)
		engine->ClientCmd("exec custom5cfg19");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 20)
		engine->ClientCmd("exec custom5cfg20");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 21)
		engine->ClientCmd("exec custom5cfg21");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 22)
		engine->ClientCmd("exec custom5cfg22");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 23)
		engine->ClientCmd("exec custom5cfg23");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 24)
		engine->ClientCmd("exec custom5cfg24");
	if (cvar->FindVar("oc_customcfgscript5_value")->GetInt() == 25)
		engine->ClientCmd("exec custom5cfg25");
}
static ConCommand oc_customcfgscript5("oc_customcfgscript5", custom_script5_cvar);

// 6
ConVar oc_customcfgscript6_value("oc_customcfgscript6_value", "0", FCVAR_ARCHIVE, "Custom config script 6.");
static void custom_script6_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 1)
		engine->ClientCmd("exec custom6cfg1");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 2)
		engine->ClientCmd("exec custom6cfg2");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 3)
		engine->ClientCmd("exec custom6cfg3");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 4)
		engine->ClientCmd("exec custom6cfg4");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 5)
		engine->ClientCmd("exec custom6cfg5");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 6)
		engine->ClientCmd("exec custom6cfg6");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 7)
		engine->ClientCmd("exec custom6cfg7");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 8)
		engine->ClientCmd("exec custom6cfg8");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 9)
		engine->ClientCmd("exec custom6cfg9");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 10)
		engine->ClientCmd("exec custom6cfg10");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 11)
		engine->ClientCmd("exec custom6cfg11");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 12)
		engine->ClientCmd("exec custom6cfg12");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 13)
		engine->ClientCmd("exec custom6cfg13");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 14)
		engine->ClientCmd("exec custom6cfg14");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 15)
		engine->ClientCmd("exec custom6cfg15");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 16)
		engine->ClientCmd("exec custom6cfg16");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 17)
		engine->ClientCmd("exec custom6cfg17");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 18)
		engine->ClientCmd("exec custom6cfg18");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 19)
		engine->ClientCmd("exec custom6cfg19");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 20)
		engine->ClientCmd("exec custom6cfg20");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 21)
		engine->ClientCmd("exec custom6cfg21");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 22)
		engine->ClientCmd("exec custom6cfg22");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 23)
		engine->ClientCmd("exec custom6cfg23");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 24)
		engine->ClientCmd("exec custom6cfg24");
	if (cvar->FindVar("oc_customcfgscript6_value")->GetInt() == 25)
		engine->ClientCmd("exec custom6cfg25");

}
static ConCommand oc_customcfgscript6("oc_customcfgscript6", custom_script6_cvar);

// 7
ConVar oc_customcfgscript7_value("oc_customcfgscript7_value", "0", FCVAR_ARCHIVE, "Custom config script 7.");
static void custom_script7_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 1)
		engine->ClientCmd("exec custom7cfg1");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 2)
		engine->ClientCmd("exec custom7cfg2");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 3)
		engine->ClientCmd("exec custom7cfg3");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 4)
		engine->ClientCmd("exec custom7cfg4");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 5)
		engine->ClientCmd("exec custom7cfg5");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 6)
		engine->ClientCmd("exec custom7cfg6");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 7)
		engine->ClientCmd("exec custom7cfg7");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 8)
		engine->ClientCmd("exec custom7cfg8");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 9)
		engine->ClientCmd("exec custom7cfg9");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 10)
		engine->ClientCmd("exec custom7cfg10");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 11)
		engine->ClientCmd("exec custom7cfg11");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 12)
		engine->ClientCmd("exec custom7cfg12");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 13)
		engine->ClientCmd("exec custom7cfg13");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 14)
		engine->ClientCmd("exec custom7cfg14");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 15)
		engine->ClientCmd("exec custom7cfg15");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 16)
		engine->ClientCmd("exec custom7cfg16");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 17)
		engine->ClientCmd("exec custom7cfg17");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 18)
		engine->ClientCmd("exec custom7cfg18");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 19)
		engine->ClientCmd("exec custom7cfg19");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 20)
		engine->ClientCmd("exec custom7cfg20");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 21)
		engine->ClientCmd("exec custom7cfg21");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 22)
		engine->ClientCmd("exec custom7cfg22");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 23)
		engine->ClientCmd("exec custom7cfg23");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 24)
		engine->ClientCmd("exec custom7cfg24");
	if (cvar->FindVar("oc_customcfgscript7_value")->GetInt() == 25)
		engine->ClientCmd("exec custom7cfg25");
}
static ConCommand oc_customcfgscript7("oc_customcfgscript7", custom_script7_cvar);

// 8
ConVar oc_customcfgscript8_value("oc_customcfgscript8_value", "0", FCVAR_ARCHIVE, "Custom config script 8.");
static void custom_script8_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 1)
		engine->ClientCmd("exec custom8cfg1");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 2)
		engine->ClientCmd("exec custom8cfg2");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 3)
		engine->ClientCmd("exec custom8cfg3");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 4)
		engine->ClientCmd("exec custom8cfg4");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 5)
		engine->ClientCmd("exec custom8cfg5");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 6)
		engine->ClientCmd("exec custom8cfg6");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 7)
		engine->ClientCmd("exec custom8cfg7");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 8)
		engine->ClientCmd("exec custom8cfg8");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 9)
		engine->ClientCmd("exec custom8cfg9");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 10)
		engine->ClientCmd("exec custom8cfg10");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 11)
		engine->ClientCmd("exec custom8cfg11");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 12)
		engine->ClientCmd("exec custom8cfg12");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 13)
		engine->ClientCmd("exec custom8cfg13");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 14)
		engine->ClientCmd("exec custom8cfg14");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 15)
		engine->ClientCmd("exec custom8cfg15");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 16)
		engine->ClientCmd("exec custom8cfg16");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 17)
		engine->ClientCmd("exec custom8cfg17");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 18)
		engine->ClientCmd("exec custom8cfg18");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 19)
		engine->ClientCmd("exec custom8cfg19");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 20)
		engine->ClientCmd("exec custom8cfg20");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 21)
		engine->ClientCmd("exec custom8cfg21");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 22)
		engine->ClientCmd("exec custom8cfg22");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 23)
		engine->ClientCmd("exec custom8cfg23");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 24)
		engine->ClientCmd("exec custom8cfg24");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 25)
		engine->ClientCmd("exec custom8cfg25");
}
static ConCommand oc_customcfgscript8("oc_customcfgscript8", custom_script8_cvar);

// 9
ConVar oc_customcfgscript9_value("oc_customcfgscript9_value", "0", FCVAR_ARCHIVE, "Custom config script 9.");
static void custom_script9_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 1)
		engine->ClientCmd("exec custom9cfg1");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 2)
		engine->ClientCmd("exec custom9cfg2");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 3)
		engine->ClientCmd("exec custom9cfg3");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 4)
		engine->ClientCmd("exec custom9cfg4");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 5)
		engine->ClientCmd("exec custom9cfg5");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 6)
		engine->ClientCmd("exec custom9cfg6");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 7)
		engine->ClientCmd("exec custom9cfg7");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 8)
		engine->ClientCmd("exec custom9cfg8");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 9)
		engine->ClientCmd("exec custom9cfg9");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 10)
		engine->ClientCmd("exec custom9cfg10");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 11)
		engine->ClientCmd("exec custom9cfg11");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 12)
		engine->ClientCmd("exec custom9cfg12");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 13)
		engine->ClientCmd("exec custom9cfg13");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 14)
		engine->ClientCmd("exec custom9cfg14");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 15)
		engine->ClientCmd("exec custom9cfg15");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 16)
		engine->ClientCmd("exec custom9cfg16");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 17)
		engine->ClientCmd("exec custom9cfg17");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 18)
		engine->ClientCmd("exec custom9cfg18");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 19)
		engine->ClientCmd("exec custom9cfg19");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 20)
		engine->ClientCmd("exec custom9cfg20");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 21)
		engine->ClientCmd("exec custom9cfg21");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 22)
		engine->ClientCmd("exec custom9cfg22");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 23)
		engine->ClientCmd("exec custom9cfg23");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 24)
		engine->ClientCmd("exec custom9cfg24");
	if (cvar->FindVar("oc_customcfgscript9_value")->GetInt() == 25)
		engine->ClientCmd("exec custom9cfg25");

}
static ConCommand oc_customcfgscript9("oc_customcfgscript9", custom_script9_cvar);

// 10
ConVar oc_customcfgscript10_value("oc_customcfgscript10_value", "0", FCVAR_ARCHIVE, "Custom config script 10.");
static void custom_script10_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 1)
		engine->ClientCmd("exec custom10cfg1");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 2)
		engine->ClientCmd("exec custom10cfg2");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 3)
		engine->ClientCmd("exec custom10cfg3");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 4)
		engine->ClientCmd("exec custom10cfg4");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 5)
		engine->ClientCmd("exec custom10cfg5");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 6)
		engine->ClientCmd("exec custom10cfg6");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 7)
		engine->ClientCmd("exec custom10cfg7");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 8)
		engine->ClientCmd("exec custom10cfg8");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 9)
		engine->ClientCmd("exec custom10cfg9");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 10)
		engine->ClientCmd("exec custom10cfg10");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 11)
		engine->ClientCmd("exec custom10cfg11");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 12)
		engine->ClientCmd("exec custom10cfg12");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 13)
		engine->ClientCmd("exec custom10cfg13");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 14)
		engine->ClientCmd("exec custom10cfg14");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 15)
		engine->ClientCmd("exec custom10cfg15");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 16)
		engine->ClientCmd("exec custom10cfg16");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 17)
		engine->ClientCmd("exec custom10cfg17");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 18)
		engine->ClientCmd("exec custom10cfg18");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 19)
		engine->ClientCmd("exec custom10cfg19");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 20)
		engine->ClientCmd("exec custom10cfg20");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 21)
		engine->ClientCmd("exec custom10cfg21");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 22)
		engine->ClientCmd("exec custom10cfg22");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 23)
		engine->ClientCmd("exec custom10cfg23");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 24)
		engine->ClientCmd("exec custom10cfg24");
	if (cvar->FindVar("oc_customcfgscript10_value")->GetInt() == 25)
		engine->ClientCmd("exec custom10cfg25");
}
static ConCommand oc_customcfgscript10("oc_customcfgscript10", custom_script10_cvar);

// 11
ConVar oc_customcfgscript11_value("oc_customcfgscript11_value", "0", FCVAR_ARCHIVE, "Custom config script 11.");
static void custom_script11_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 1)
		engine->ClientCmd("exec custom11cfg1");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 2)
		engine->ClientCmd("exec custom11cfg2");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 3)
		engine->ClientCmd("exec custom11cfg3");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 4)
		engine->ClientCmd("exec custom11cfg4");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 5)
		engine->ClientCmd("exec custom11cfg5");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 6)
		engine->ClientCmd("exec custom11cfg6");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 7)
		engine->ClientCmd("exec custom11cfg7");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 8)
		engine->ClientCmd("exec custom11cfg8");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 9)
		engine->ClientCmd("exec custom11cfg9");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 10)
		engine->ClientCmd("exec custom11cfg10");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 11)
		engine->ClientCmd("exec custom11cfg11");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 12)
		engine->ClientCmd("exec custom11cfg12");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 13)
		engine->ClientCmd("exec custom11cfg13");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 14)
		engine->ClientCmd("exec custom11cfg14");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 15)
		engine->ClientCmd("exec custom11cfg15");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 16)
		engine->ClientCmd("exec custom11cfg16");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 17)
		engine->ClientCmd("exec custom11cfg17");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 18)
		engine->ClientCmd("exec custom11cfg18");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 19)
		engine->ClientCmd("exec custom11cfg19");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 20)
		engine->ClientCmd("exec custom11cfg20");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 21)
		engine->ClientCmd("exec custom11cfg21");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 22)
		engine->ClientCmd("exec custom11cfg22");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 23)
		engine->ClientCmd("exec custom11cfg23");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 24)
		engine->ClientCmd("exec custom11cfg24");
	if (cvar->FindVar("oc_customcfgscript11_value")->GetInt() == 25)
		engine->ClientCmd("exec custom11cfg25");
}
static ConCommand oc_customcfgscript11("oc_customcfgscript11", custom_script11_cvar);

// 12
ConVar oc_customcfgscript12_value("oc_customcfgscript12_value", "0", FCVAR_ARCHIVE, "Custom config script 12.");
static void custom_script12_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 1)
		engine->ClientCmd("exec custom12cfg1");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 2)
		engine->ClientCmd("exec custom12cfg2");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 3)
		engine->ClientCmd("exec custom12cfg3");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 4)
		engine->ClientCmd("exec custom12cfg4");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 5)
		engine->ClientCmd("exec custom12cfg5");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 6)
		engine->ClientCmd("exec custom12cfg6");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 7)
		engine->ClientCmd("exec custom12cfg7");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 8)
		engine->ClientCmd("exec custom12cfg8");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 9)
		engine->ClientCmd("exec custom12cfg9");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 10)
		engine->ClientCmd("exec custom12cfg10");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 11)
		engine->ClientCmd("exec custom12cfg11");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 12)
		engine->ClientCmd("exec custom12cfg12");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 13)
		engine->ClientCmd("exec custom12cfg13");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 14)
		engine->ClientCmd("exec custom12cfg14");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 15)
		engine->ClientCmd("exec custom12cfg15");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 16)
		engine->ClientCmd("exec custom12cfg16");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 17)
		engine->ClientCmd("exec custom12cfg17");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 18)
		engine->ClientCmd("exec custom12cfg18");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 19)
		engine->ClientCmd("exec custom12cfg19");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 20)
		engine->ClientCmd("exec custom12cfg20");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 21)
		engine->ClientCmd("exec custom12cfg21");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 22)
		engine->ClientCmd("exec custom12cfg22");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 23)
		engine->ClientCmd("exec custom12cfg23");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 24)
		engine->ClientCmd("exec custom12cfg24");
	if (cvar->FindVar("oc_customcfgscript12_value")->GetInt() == 25)
		engine->ClientCmd("exec custom12cfg25");
}
static ConCommand oc_customcfgscript12("oc_customcfgscript12", custom_script12_cvar);

// 13
ConVar oc_customcfgscript13_value("oc_customcfgscript13_value", "0", FCVAR_ARCHIVE, "Custom config script 13.");
static void custom_script13_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 1)
		engine->ClientCmd("exec custom13cfg1");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 2)
		engine->ClientCmd("exec custom13cfg2");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 3)
		engine->ClientCmd("exec custom13cfg3");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 4)
		engine->ClientCmd("exec custom13cfg4");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 5)
		engine->ClientCmd("exec custom13cfg5");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 6)
		engine->ClientCmd("exec custom13cfg6");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 7)
		engine->ClientCmd("exec custom13cfg7");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 8)
		engine->ClientCmd("exec custom13cfg8");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 9)
		engine->ClientCmd("exec custom13cfg9");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 10)
		engine->ClientCmd("exec custom13cfg10");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 11)
		engine->ClientCmd("exec custom13cfg11");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 12)
		engine->ClientCmd("exec custom13cfg12");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 13)
		engine->ClientCmd("exec custom13cfg13");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 14)
		engine->ClientCmd("exec custom13cfg14");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 15)
		engine->ClientCmd("exec custom13cfg15");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 16)
		engine->ClientCmd("exec custom13cfg16");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 17)
		engine->ClientCmd("exec custom13cfg17");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 18)
		engine->ClientCmd("exec custom13cfg18");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 19)
		engine->ClientCmd("exec custom13cfg19");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 20)
		engine->ClientCmd("exec custom13cfg20");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 21)
		engine->ClientCmd("exec custom13cfg21");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 22)
		engine->ClientCmd("exec custom13cfg22");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 23)
		engine->ClientCmd("exec custom13cfg23");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 24)
		engine->ClientCmd("exec custom13cfg24");
	if (cvar->FindVar("oc_customcfgscript13_value")->GetInt() == 25)
		engine->ClientCmd("exec custom13cfg25");
}
static ConCommand oc_customcfgscript13("oc_customcfgscript13", custom_script13_cvar);

// 14
ConVar oc_customcfgscript14_value("oc_customcfgscript14_value", "0", FCVAR_ARCHIVE, "Custom config script 14.");
static void custom_script14_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 1)
		engine->ClientCmd("exec custom14cfg1");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 2)
		engine->ClientCmd("exec custom14cfg2");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 3)
		engine->ClientCmd("exec custom14cfg3");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 4)
		engine->ClientCmd("exec custom14cfg4");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 5)
		engine->ClientCmd("exec custom14cfg5");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 6)
		engine->ClientCmd("exec custom14cfg6");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 7)
		engine->ClientCmd("exec custom14cfg7");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 8)
		engine->ClientCmd("exec custom14cfg8");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 9)
		engine->ClientCmd("exec custom14cfg9");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 10)
		engine->ClientCmd("exec custom14cfg10");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 11)
		engine->ClientCmd("exec custom14cfg11");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 12)
		engine->ClientCmd("exec custom14cfg12");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 13)
		engine->ClientCmd("exec custom14cfg13");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 14)
		engine->ClientCmd("exec custom14cfg14");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 15)
		engine->ClientCmd("exec custom14cfg15");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 16)
		engine->ClientCmd("exec custom14cfg16");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 17)
		engine->ClientCmd("exec custom14cfg17");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 18)
		engine->ClientCmd("exec custom14cfg18");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 19)
		engine->ClientCmd("exec custom14cfg19");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 20)
		engine->ClientCmd("exec custom14cfg20");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 21)
		engine->ClientCmd("exec custom14cfg21");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 22)
		engine->ClientCmd("exec custom14cfg22");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 23)
		engine->ClientCmd("exec custom14cfg23");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 24)
		engine->ClientCmd("exec custom14cfg24");
	if (cvar->FindVar("oc_customcfgscript14_value")->GetInt() == 25)
		engine->ClientCmd("exec custom14cfg25");
}
static ConCommand oc_customcfgscript14("oc_customcfgscript14", custom_script14_cvar);

// 15
ConVar oc_customcfgscript15_value("oc_customcfgscript15_value", "0", FCVAR_ARCHIVE, "Custom config script 15.");
static void custom_script15_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 1)
		engine->ClientCmd("exec custom15cfg1");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 2)
		engine->ClientCmd("exec custom15cfg2");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 3)
		engine->ClientCmd("exec custom15cfg3");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 4)
		engine->ClientCmd("exec custom15cfg4");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 5)
		engine->ClientCmd("exec custom15cfg5");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 6)
		engine->ClientCmd("exec custom15cfg6");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 7)
		engine->ClientCmd("exec custom15cfg7");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 8)
		engine->ClientCmd("exec custom15cfg8");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 9)
		engine->ClientCmd("exec custom15cfg9");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 10)
		engine->ClientCmd("exec custom15cfg10");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 11)
		engine->ClientCmd("exec custom15cfg11");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 12)
		engine->ClientCmd("exec custom15cfg12");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 13)
		engine->ClientCmd("exec custom15cfg13");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 14)
		engine->ClientCmd("exec custom15cfg14");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 15)
		engine->ClientCmd("exec custom15cfg15");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 16)
		engine->ClientCmd("exec custom15cfg16");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 17)
		engine->ClientCmd("exec custom15cfg17");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 18)
		engine->ClientCmd("exec custom15cfg18");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 19)
		engine->ClientCmd("exec custom15cfg19");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 20)
		engine->ClientCmd("exec custom15cfg20");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 21)
		engine->ClientCmd("exec custom15cfg21");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 22)
		engine->ClientCmd("exec custom15cfg22");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 23)
		engine->ClientCmd("exec custom15cfg23");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 24)
		engine->ClientCmd("exec custom15cfg24");
	if (cvar->FindVar("oc_customcfgscript15_value")->GetInt() == 25)
		engine->ClientCmd("exec custom15cfg25");
}
static ConCommand oc_customcfgscript15("oc_customcfgscript15", custom_script15_cvar);

// 16
ConVar oc_customcfgscript16_value("oc_customcfgscript16_value", "0", FCVAR_ARCHIVE, "Custom config script 16.");
static void custom_script16_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 1)
		engine->ClientCmd("exec custom16cfg1");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 2)
		engine->ClientCmd("exec custom16cfg2");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 3)
		engine->ClientCmd("exec custom16cfg3");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 4)
		engine->ClientCmd("exec custom16cfg4");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 5)
		engine->ClientCmd("exec custom16cfg5");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 6)
		engine->ClientCmd("exec custom16cfg6");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 7)
		engine->ClientCmd("exec custom16cfg7");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 8)
		engine->ClientCmd("exec custom16cfg8");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 9)
		engine->ClientCmd("exec custom16cfg9");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 10)
		engine->ClientCmd("exec custom16cfg10");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 11)
		engine->ClientCmd("exec custom16cfg11");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 12)
		engine->ClientCmd("exec custom16cfg12");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 13)
		engine->ClientCmd("exec custom16cfg13");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 14)
		engine->ClientCmd("exec custom16cfg14");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 15)
		engine->ClientCmd("exec custom16cfg15");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 16)
		engine->ClientCmd("exec custom16cfg16");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 17)
		engine->ClientCmd("exec custom16cfg17");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 18)
		engine->ClientCmd("exec custom16cfg18");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 19)
		engine->ClientCmd("exec custom16cfg19");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 20)
		engine->ClientCmd("exec custom16cfg20");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 21)
		engine->ClientCmd("exec custom16cfg21");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 22)
		engine->ClientCmd("exec custom16cfg22");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 23)
		engine->ClientCmd("exec custom16cfg23");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 24)
		engine->ClientCmd("exec custom16cfg24");
	if (cvar->FindVar("oc_customcfgscript16_value")->GetInt() == 25)
		engine->ClientCmd("exec custom16cfg25");

}
static ConCommand oc_customcfgscript16("oc_customcfgscript16", custom_script16_cvar);

// 17
ConVar oc_customcfgscript17_value("oc_customcfgscript17_value", "0", FCVAR_ARCHIVE, "Custom config script 17.");
static void custom_script17_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 1)
		engine->ClientCmd("exec custom17cfg1");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 2)
		engine->ClientCmd("exec custom17cfg2");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 3)
		engine->ClientCmd("exec custom17cfg3");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 4)
		engine->ClientCmd("exec custom17cfg4");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 5)
		engine->ClientCmd("exec custom17cfg5");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 6)
		engine->ClientCmd("exec custom17cfg6");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 7)
		engine->ClientCmd("exec custom17cfg7");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 8)
		engine->ClientCmd("exec custom17cfg8");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 9)
		engine->ClientCmd("exec custom17cfg9");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 10)
		engine->ClientCmd("exec custom17cfg10");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 11)
		engine->ClientCmd("exec custom17cfg11");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 12)
		engine->ClientCmd("exec custom17cfg12");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 13)
		engine->ClientCmd("exec custom17cfg13");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 14)
		engine->ClientCmd("exec custom17cfg14");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 15)
		engine->ClientCmd("exec custom17cfg15");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 16)
		engine->ClientCmd("exec custom17cfg16");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 17)
		engine->ClientCmd("exec custom17cfg17");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 18)
		engine->ClientCmd("exec custom17cfg18");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 19)
		engine->ClientCmd("exec custom17cfg19");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 20)
		engine->ClientCmd("exec custom17cfg20");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 21)
		engine->ClientCmd("exec custom17cfg21");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 22)
		engine->ClientCmd("exec custom17cfg22");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 23)
		engine->ClientCmd("exec custom17cfg23");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 24)
		engine->ClientCmd("exec custom17cfg24");
	if (cvar->FindVar("oc_customcfgscript17_value")->GetInt() == 25)
		engine->ClientCmd("exec custom17cfg25");
}
static ConCommand oc_customcfgscript17("oc_customcfgscript17", custom_script17_cvar);

// 18
ConVar oc_customcfgscript18_value("oc_customcfgscript18_value", "0", FCVAR_ARCHIVE, "Custom config script 18.");
static void custom_script18_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 1)
		engine->ClientCmd("exec custom18cfg1");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 2)
		engine->ClientCmd("exec custom18cfg2");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 3)
		engine->ClientCmd("exec custom18cfg3");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 4)
		engine->ClientCmd("exec custom18cfg4");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 5)
		engine->ClientCmd("exec custom18cfg5");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 6)
		engine->ClientCmd("exec custom18cfg6");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 7)
		engine->ClientCmd("exec custom18cfg7");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 8)
		engine->ClientCmd("exec custom18cfg8");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 9)
		engine->ClientCmd("exec custom18cfg9");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 10)
		engine->ClientCmd("exec custom18cfg10");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 11)
		engine->ClientCmd("exec custom18cfg11");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 12)
		engine->ClientCmd("exec custom18cfg12");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 13)
		engine->ClientCmd("exec custom18cfg13");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 14)
		engine->ClientCmd("exec custom18cfg14");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 15)
		engine->ClientCmd("exec custom18cfg15");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 16)
		engine->ClientCmd("exec custom18cfg16");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 17)
		engine->ClientCmd("exec custom18cfg17");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 18)
		engine->ClientCmd("exec custom18cfg18");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 19)
		engine->ClientCmd("exec custom18cfg19");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 20)
		engine->ClientCmd("exec custom18cfg20");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 21)
		engine->ClientCmd("exec custom18cfg21");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 22)
		engine->ClientCmd("exec custom18cfg22");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 23)
		engine->ClientCmd("exec custom18cfg23");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 24)
		engine->ClientCmd("exec custom18cfg24");
	if (cvar->FindVar("oc_customcfgscript8_value")->GetInt() == 25)
		engine->ClientCmd("exec custom18cfg25");
}
static ConCommand oc_customcfgscript18("oc_customcfgscript18", custom_script18_cvar);

// 19
ConVar oc_customcfgscript19_value("oc_customcfgscript19_value", "0", FCVAR_ARCHIVE, "Custom config script 19.");
static void custom_script19_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 1)
		engine->ClientCmd("exec custom19cfg1");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 2)
		engine->ClientCmd("exec custom19cfg2");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 3)
		engine->ClientCmd("exec custom19cfg3");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 4)
		engine->ClientCmd("exec custom19cfg4");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 5)
		engine->ClientCmd("exec custom19cfg5");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 6)
		engine->ClientCmd("exec custom19cfg6");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 7)
		engine->ClientCmd("exec custom19cfg7");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 8)
		engine->ClientCmd("exec custom19cfg8");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 9)
		engine->ClientCmd("exec custom19cfg9");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 10)
		engine->ClientCmd("exec custom19cfg10");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 11)
		engine->ClientCmd("exec custom19cfg11");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 12)
		engine->ClientCmd("exec custom19cfg12");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 13)
		engine->ClientCmd("exec custom19cfg13");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 14)
		engine->ClientCmd("exec custom19cfg14");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 15)
		engine->ClientCmd("exec custom19cfg15");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 16)
		engine->ClientCmd("exec custom19cfg16");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 17)
		engine->ClientCmd("exec custom19cfg17");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 18)
		engine->ClientCmd("exec custom19cfg18");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 19)
		engine->ClientCmd("exec custom19cfg19");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 20)
		engine->ClientCmd("exec custom19cfg20");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 21)
		engine->ClientCmd("exec custom19cfg21");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 22)
		engine->ClientCmd("exec custom19cfg22");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 23)
		engine->ClientCmd("exec custom19cfg23");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 24)
		engine->ClientCmd("exec custom19cfg24");
	if (cvar->FindVar("oc_customcfgscript19_value")->GetInt() == 25)
		engine->ClientCmd("exec custom19cfg25");

}
static ConCommand oc_customcfgscript19("oc_customcfgscript19", custom_script19_cvar);

// 20
ConVar oc_customcfgscript20_value("oc_customcfgscript20_value", "0", FCVAR_ARCHIVE, "Custom config script 20.");
static void custom_script20_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 1)
		engine->ClientCmd("exec custom20cfg1");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 2)
		engine->ClientCmd("exec custom20cfg2");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 3)
		engine->ClientCmd("exec custom20cfg3");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 4)
		engine->ClientCmd("exec custom20cfg4");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 5)
		engine->ClientCmd("exec custom20cfg5");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 6)
		engine->ClientCmd("exec custom20cfg6");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 7)
		engine->ClientCmd("exec custom20cfg7");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 8)
		engine->ClientCmd("exec custom20cfg8");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 9)
		engine->ClientCmd("exec custom20cfg9");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 10)
		engine->ClientCmd("exec custom20cfg10");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 11)
		engine->ClientCmd("exec custom20cfg11");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 12)
		engine->ClientCmd("exec custom20cfg12");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 13)
		engine->ClientCmd("exec custom20cfg13");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 14)
		engine->ClientCmd("exec custom20cfg14");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 15)
		engine->ClientCmd("exec custom20cfg15");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 16)
		engine->ClientCmd("exec custom20cfg16");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 17)
		engine->ClientCmd("exec custom20cfg17");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 18)
		engine->ClientCmd("exec custom20cfg18");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 19)
		engine->ClientCmd("exec custom20cfg19");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 20)
		engine->ClientCmd("exec custom20cfg20");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 21)
		engine->ClientCmd("exec custom20cfg21");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 22)
		engine->ClientCmd("exec custom20cfg22");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 23)
		engine->ClientCmd("exec custom20cfg23");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 24)
		engine->ClientCmd("exec custom20cfg24");
	if (cvar->FindVar("oc_customcfgscript20_value")->GetInt() == 25)
		engine->ClientCmd("exec custom20cfg25");
}
static ConCommand oc_customcfgscript20("oc_customcfgscript20", custom_script20_cvar);

// 21
ConVar oc_customcfgscript21_value("oc_customcfgscript21_value", "0", FCVAR_ARCHIVE, "Custom config script 21.");
static void custom_script21_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 1)
		engine->ClientCmd("exec custom21cfg1");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 2)
		engine->ClientCmd("exec custom21cfg2");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 3)
		engine->ClientCmd("exec custom21cfg3");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 4)
		engine->ClientCmd("exec custom21cfg4");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 5)
		engine->ClientCmd("exec custom21cfg5");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 6)
		engine->ClientCmd("exec custom21cfg6");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 7)
		engine->ClientCmd("exec custom21cfg7");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 8)
		engine->ClientCmd("exec custom21cfg8");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 9)
		engine->ClientCmd("exec custom21cfg9");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 10)
		engine->ClientCmd("exec custom21cfg10");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 11)
		engine->ClientCmd("exec custom21cfg11");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 12)
		engine->ClientCmd("exec custom21cfg12");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 13)
		engine->ClientCmd("exec custom21cfg13");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 14)
		engine->ClientCmd("exec custom21cfg14");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 15)
		engine->ClientCmd("exec custom21cfg15");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 16)
		engine->ClientCmd("exec custom21cfg16");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 17)
		engine->ClientCmd("exec custom21cfg17");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 18)
		engine->ClientCmd("exec custom21cfg18");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 19)
		engine->ClientCmd("exec custom21cfg19");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 20)
		engine->ClientCmd("exec custom21cfg20");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 21)
		engine->ClientCmd("exec custom21cfg21");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 22)
		engine->ClientCmd("exec custom21cfg22");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 23)
		engine->ClientCmd("exec custom21cfg23");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 24)
		engine->ClientCmd("exec custom21cfg24");
	if (cvar->FindVar("oc_customcfgscript21_value")->GetInt() == 25)
		engine->ClientCmd("exec custom21cfg25");
}
static ConCommand oc_customcfgscript21("oc_customcfgscript21", custom_script21_cvar);

// 22
ConVar oc_customcfgscript22_value("oc_customcfgscript22_value", "0", FCVAR_ARCHIVE, "Custom config script 22.");
static void custom_script22_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 1)
		engine->ClientCmd("exec custom22cfg1");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 2)
		engine->ClientCmd("exec custom22cfg2");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 3)
		engine->ClientCmd("exec custom22cfg3");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 4)
		engine->ClientCmd("exec custom22cfg4");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 5)
		engine->ClientCmd("exec custom22cfg5");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 6)
		engine->ClientCmd("exec custom22cfg6");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 7)
		engine->ClientCmd("exec custom22cfg7");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 8)
		engine->ClientCmd("exec custom22cfg8");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 9)
		engine->ClientCmd("exec custom22cfg9");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 10)
		engine->ClientCmd("exec custom22cfg10");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 11)
		engine->ClientCmd("exec custom22cfg11");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 12)
		engine->ClientCmd("exec custom22cfg12");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 13)
		engine->ClientCmd("exec custom22cfg13");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 14)
		engine->ClientCmd("exec custom22cfg14");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 15)
		engine->ClientCmd("exec custom22cfg15");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 16)
		engine->ClientCmd("exec custom22cfg16");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 17)
		engine->ClientCmd("exec custom22cfg17");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 18)
		engine->ClientCmd("exec custom22cfg18");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 19)
		engine->ClientCmd("exec custom22cfg19");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 20)
		engine->ClientCmd("exec custom22cfg20");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 21)
		engine->ClientCmd("exec custom22cfg21");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 22)
		engine->ClientCmd("exec custom22cfg22");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 23)
		engine->ClientCmd("exec custom22cfg23");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 24)
		engine->ClientCmd("exec custom22cfg24");
	if (cvar->FindVar("oc_customcfgscript22_value")->GetInt() == 25)
		engine->ClientCmd("exec custom22cfg25");
}
static ConCommand oc_customcfgscript22("oc_customcfgscript22", custom_script22_cvar);

// 23
ConVar oc_customcfgscript23_value("oc_customcfgscript23_value", "0", FCVAR_ARCHIVE, "Custom config script 23.");
static void custom_script23_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 1)
		engine->ClientCmd("exec custom23cfg1");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 2)
		engine->ClientCmd("exec custom23cfg2");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 3)
		engine->ClientCmd("exec custom23cfg3");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 4)
		engine->ClientCmd("exec custom23cfg4");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 5)
		engine->ClientCmd("exec custom23cfg5");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 6)
		engine->ClientCmd("exec custom23cfg6");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 7)
		engine->ClientCmd("exec custom23cfg7");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 8)
		engine->ClientCmd("exec custom23cfg8");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 9)
		engine->ClientCmd("exec custom23cfg9");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 10)
		engine->ClientCmd("exec custom23cfg10");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 11)
		engine->ClientCmd("exec custom23cfg11");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 12)
		engine->ClientCmd("exec custom23cfg12");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 13)
		engine->ClientCmd("exec custom23cfg13");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 14)
		engine->ClientCmd("exec custom23cfg14");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 15)
		engine->ClientCmd("exec custom23cfg15");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 16)
		engine->ClientCmd("exec custom23cfg16");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 17)
		engine->ClientCmd("exec custom23cfg17");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 18)
		engine->ClientCmd("exec custom23cfg18");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 19)
		engine->ClientCmd("exec custom23cfg19");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 20)
		engine->ClientCmd("exec custom23cfg20");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 21)
		engine->ClientCmd("exec custom23cfg21");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 22)
		engine->ClientCmd("exec custom23cfg22");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 23)
		engine->ClientCmd("exec custom23cfg23");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 24)
		engine->ClientCmd("exec custom23cfg24");
	if (cvar->FindVar("oc_customcfgscript23_value")->GetInt() == 25)
		engine->ClientCmd("exec custom23cfg25");
}
static ConCommand oc_customcfgscript23("oc_customcfgscript23", custom_script23_cvar);

// 24
ConVar oc_customcfgscript24_value("oc_customcfgscript24_value", "0", FCVAR_ARCHIVE, "Custom config script 24.");
static void custom_script24_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 1)
		engine->ClientCmd("exec custom24cfg1");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 2)
		engine->ClientCmd("exec custom24cfg2");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 3)
		engine->ClientCmd("exec custom24cfg3");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 4)
		engine->ClientCmd("exec custom24cfg4");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 5)
		engine->ClientCmd("exec custom24cfg5");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 6)
		engine->ClientCmd("exec custom24cfg6");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 7)
		engine->ClientCmd("exec custom24cfg7");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 8)
		engine->ClientCmd("exec custom24cfg8");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 9)
		engine->ClientCmd("exec custom24cfg9");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 10)
		engine->ClientCmd("exec custom24cfg10");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 11)
		engine->ClientCmd("exec custom24cfg11");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 12)
		engine->ClientCmd("exec custom24cfg12");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 13)
		engine->ClientCmd("exec custom24cfg13");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 14)
		engine->ClientCmd("exec custom24cfg14");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 15)
		engine->ClientCmd("exec custom24cfg15");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 16)
		engine->ClientCmd("exec custom24cfg16");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 17)
		engine->ClientCmd("exec custom24cfg17");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 18)
		engine->ClientCmd("exec custom24cfg18");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 19)
		engine->ClientCmd("exec custom24cfg19");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 20)
		engine->ClientCmd("exec custom24cfg20");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 21)
		engine->ClientCmd("exec custom24cfg21");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 22)
		engine->ClientCmd("exec custom24cfg22");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 23)
		engine->ClientCmd("exec custom24cfg23");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 24)
		engine->ClientCmd("exec custom24cfg24");
	if (cvar->FindVar("oc_customcfgscript24_value")->GetInt() == 25)
		engine->ClientCmd("exec custom24cfg25");
}
static ConCommand oc_customcfgscript24("oc_customcfgscript24", custom_script24_cvar);

// 25
ConVar oc_customcfgscript25_value("oc_customcfgscript25_value", "0", FCVAR_ARCHIVE, "Custom config script 25.");
static void custom_script25_cvar(void)
{
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 1)
		engine->ClientCmd("exec custom25cfg1");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 2)
		engine->ClientCmd("exec custom25cfg2");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 3)
		engine->ClientCmd("exec custom25cfg3");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 4)
		engine->ClientCmd("exec custom25cfg4");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 5)
		engine->ClientCmd("exec custom25cfg5");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 6)
		engine->ClientCmd("exec custom25cfg6");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 7)
		engine->ClientCmd("exec custom25cfg7");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 8)
		engine->ClientCmd("exec custom25cfg8");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 9)
		engine->ClientCmd("exec custom25cfg9");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 10)
		engine->ClientCmd("exec custom25cfg10");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 11)
		engine->ClientCmd("exec custom25cfg11");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 12)
		engine->ClientCmd("exec custom25cfg12");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 13)
		engine->ClientCmd("exec custom25cfg13");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 14)
		engine->ClientCmd("exec custom25cfg14");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 15)
		engine->ClientCmd("exec custom25cfg15");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 16)
		engine->ClientCmd("exec custom25cfg16");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 17)
		engine->ClientCmd("exec custom25cfg17");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 18)
		engine->ClientCmd("exec custom25cfg18");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 19)
		engine->ClientCmd("exec custom25cfg19");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 20)
		engine->ClientCmd("exec custom25cfg20");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 21)
		engine->ClientCmd("exec custom25cfg21");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 22)
		engine->ClientCmd("exec custom25cfg22");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 23)
		engine->ClientCmd("exec custom25cfg23");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 24)
		engine->ClientCmd("exec custom25cfg24");
	if (cvar->FindVar("oc_customcfgscript25_value")->GetInt() == 25)
		engine->ClientCmd("exec custom25cfg25");
}
static ConCommand oc_customcfgscript25("oc_customcfgscript25", custom_script25_cvar);
