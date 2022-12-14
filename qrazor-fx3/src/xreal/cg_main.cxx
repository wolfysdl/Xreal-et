/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================


/// includes ===================================================================
// system -------------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "cg_local.h"





//
// cvars
//
cvar_t	*cg_gun;
cvar_t	*cg_footsteps;
cvar_t	*cg_crosshair;
cvar_t	*cg_crosshair_size;
cvar_t	*cg_stats;
cvar_t	*cg_vwep;
cvar_t	*cg_noskins;
cvar_t	*cg_showclamp;
cvar_t	*cg_shownet;
cvar_t	*cg_predict;
cvar_t	*cg_showmiss;
cvar_t	*cg_viewsize;
cvar_t	*cg_centertime;
cvar_t	*cg_showturtle;
cvar_t	*cg_showfps;
cvar_t	*cg_showlayout;
cvar_t	*cg_printspeed;
cvar_t	*cg_paused;
cvar_t	*cg_gravity;
cvar_t	*cg_thirdperson;
cvar_t	*cg_thirdperson_angle;
cvar_t	*cg_thirdperson_range;


//
// interface
//
cg_export_t	cg_globals;

cg_state_t	cg;

cg_shader_c	cg_shader_crosshairs[] =
{
	cg_shader_c("crosshairs/ch1"),
	cg_shader_c("crosshairs/ch2"),
	cg_shader_c("crosshairs/ch3"),
	cg_shader_c("crosshairs/ch4"),
	cg_shader_c("crosshairs/ch5"),
	cg_shader_c("crosshairs/ch6"),
	cg_shader_c("crosshairs/ch7"),
	cg_shader_c("crosshairs/ch8"),
	cg_shader_c("crosshairs/ch9"),
	cg_shader_c("crosshairs/ch10"),
	cg_shader_c("crosshairs/ch11"),
	cg_shader_c("crosshairs/ch12"),
	cg_shader_c("crosshairs/ch13"),
	cg_shader_c("crosshairs/ch14"),
	cg_shader_c("crosshairs/ch15"),
	cg_shader_c("crosshairs/ch16"),
	cg_shader_c("crosshairs/ch17"),
	cg_shader_c("crosshairs/ch18"),
	cg_shader_c("crosshairs/ch19"),
	cg_shader_c("crosshairs/ch20")
};

cg_shader_c	cg_shader_numbers_digi[] = 
{
	cg_shader_c("numbers/digital/zero_32b"),
	cg_shader_c("numbers/digital/one_32b"),
	cg_shader_c("numbers/digital/two_32b"),
	cg_shader_c("numbers/digital/three_32b"),
	cg_shader_c("numbers/digital/four_32b"),
	cg_shader_c("numbers/digital/five_32b"),
	cg_shader_c("numbers/digital/six_32b"),
	cg_shader_c("numbers/digital/seven_32b"),
	cg_shader_c("numbers/digital/eight_32b"),
	cg_shader_c("numbers/digital/nine_32b"),
	cg_shader_c("numbers/digital/minus_32b"),
};

#if defined(ODE)
d_world_c*		cg_ode_world;
d_space_c*		cg_ode_space;
d_joint_group_c*	cg_ode_contact_group;
#endif



cg_shader_c::cg_shader_c(const std::string &name)
{
	_name	= name;
	_handle	= -1;
}
	
int	cg_shader_c::getHandle()
{
	if(_handle == -1)
	{
		_handle = trap_R_RegisterPic(_name);
	}

	return _handle;
}


cg_sound_c::cg_sound_c(const std::string &name)
{
	_name	= name;
	_handle	= -1;
}
	
int	cg_sound_c::getHandle()
{
	if(_handle == -1)
	{
		_handle = trap_S_RegisterSound(_name);
	}

	return _handle;
}



void	CG_ParseLayout(bitmessage_c &msg)
{
	const char *s = msg.readString();
	
	strncpy(cg.layout, s, sizeof(cg.layout)-1);
}


/*
=================
CL_Skins_f

Load or download any custom player skins and models
=================
*/
/*
static void	CG_Skins_f()
{
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(!trap_CL_GetConfigString(CS_PLAYERSKINS+i)[0])
			continue;
			
		Com_Printf("client %i: %s\n", i, trap_CL_GetConfigString(CS_PLAYERSKINS+i));
		CG_UpdateScreen();
		trap_Sys_SendKeyEvents();	// pump message loop
		CG_ParseClientinfo(i);
	}
}
*/


/*
=================
CL_Snd_Restart_f

Restart the sound subsystem so it can pick up
new parameters and flush all sounds
=================
*/
static void	CG_Snd_Restart_f()
{
	trap_S_Shutdown();
	trap_S_Init();
	CG_RegisterSounds();
}



static void	CG_ClearEntities()
{
	// clear all entities
	cg.entities			= std::vector<cg_entity_t>(MAX_ENTITIES);
	cg.entities_parse		= std::vector<entity_state_t>(MAX_ENTITIES);
	cg.entities_first		= 0;		// index (not anded off) into cl_parse_entities[]
}


void	CG_ClearState()
{
	cg.frame_lerp			= 0;			// between oldframe and frame
	
	for(int i=0; i<CMD_BACKUP; i++)
		cg.predicted_origins[i].clear();
	
	cg.predicted_step		= 0;				// for stair up smoothing
	cg.predicted_step_time		= 0;

	cg.predicted_origin.clear();	// generated by CL_PredictMovement
	cg.predicted_angles.clear();
	cg.prediction_error.clear();
	
	
	cg.refdef.clear();

	cg.v_blend.clear();
	
	cg.v_forward.clear();
	cg.v_right.clear();
	cg.v_up.clear();


	CG_ClearEntities();

	CG_ClearParticles();
	//CG_ClearLights();
	CG_ClearTEnts();
	
	for(int i=0; i<MAX_CLIENTS; i++)
		cg.clientinfo[i].clear();
		
	cg.baseclientinfo.clear();
	
	
	memset(cg.layout, 0, sizeof(cg.layout));
	memset(cg.inventory, 0, sizeof(cg.inventory));
	
	
	memset(cg.model_draw, -1, sizeof(cg.model_draw));
	memset(cg.model_clip, 0, sizeof(cg.model_clip));

	cg.world_cmodel			= NULL;

	memset(cg.shader_precache, -1, sizeof(cg.shader_precache));
	memset(cg.animation_precache, -1, sizeof(cg.animation_precache));
	memset(cg.sound_precache, -1, sizeof(cg.sound_precache));
	memset(cg.light_precache, -1, sizeof(cg.light_precache));
}

void	CG_RegisterSounds()
{
	trap_S_BeginRegistration();
	
	CG_RegisterTEntSounds();
	
	for(int i=1; i<MAX_SOUNDS; i++)
	{
		if(!trap_CL_GetConfigString(CS_SOUNDS+i)[0])
			break;
			
		cg.sound_precache[i] = trap_S_RegisterSound(trap_CL_GetConfigString(CS_SOUNDS+i));
	}
	
	trap_S_EndRegistration();
}



static void	CG_InitClientGame()
{
	//
	// register our variables
	//
	cg_gun 			= trap_Cvar_Get("cg_gun", "1", CVAR_ARCHIVE);
	cg_footsteps		= trap_Cvar_Get("cg_footsteps", "1", CVAR_NONE);
	cg_crosshair 		= trap_Cvar_Get("cg_crosshair", "0", CVAR_ARCHIVE);
	cg_crosshair_size 	= trap_Cvar_Get("cg_crosshair_size", "24", CVAR_ARCHIVE);
	cg_stats 		= trap_Cvar_Get("cg_stats", "0", 0);
	cg_vwep			= trap_Cvar_Get("cg_vwep", "1", CVAR_ARCHIVE);
	cg_noskins		= trap_Cvar_Get("cg_noskins", "0", 0);
	cg_showclamp		= trap_Cvar_Get("showclamp", "0", 0);
	cg_shownet		= trap_Cvar_Get("shownet", "0", 0);
	cg_predict		= trap_Cvar_Get("cg_predict", "0,1", CVAR_NONE);
	cg_showmiss		= trap_Cvar_Get("cg_showmiss", "1,0", CVAR_NONE);
	cg_viewsize		= trap_Cvar_Get("cg_viewsize", "100", CVAR_ARCHIVE);
	cg_centertime		= trap_Cvar_Get("cg_centertime", "2.5", 0);
	cg_showturtle		= trap_Cvar_Get("cg_showturtle", "0", 0);
	cg_showfps		= trap_Cvar_Get("cg_showfps", "1", CVAR_ARCHIVE);
	cg_showlayout		= trap_Cvar_Get("cg_showlayout", "1", CVAR_ARCHIVE);
	cg_printspeed		= trap_Cvar_Get("cg_printspeed", "8", CVAR_NONE);
	cg_paused		= trap_Cvar_Get("paused", "0", CVAR_NONE);
	cg_gravity		= trap_Cvar_Get("cg_gravity", "1", CVAR_NONE);
	cg_thirdperson		= trap_Cvar_Get("cg_thirdperson", "0,1", CVAR_CHEAT);
	cg_thirdperson_angle	= trap_Cvar_Get("cg_thirdperson_angle", "0,90,180,270", CVAR_NONE);
	cg_thirdperson_range	= trap_Cvar_Get("cg_thirdperson_range", "70,140,210", CVAR_NONE);
	
//	trap_Cmd_AddCommand("skins", 		CG_Skins_f);
	trap_Cmd_AddCommand("snd_restart",	CG_Snd_Restart_f);


	//
	// initialize subsystems
	//
	CG_InitScreen();

#if defined(ODE)
	CG_InitDynamics();
#endif

	CG_InitView();
	
	CG_InitWeapon();
	
	//CG_InitEntities();
	
	
	//
	// create new dynamic lists
	//
	/*
	for(int i=0; i<MAX_EDICTS; i++)
	{
		centity_t *ent = new centity_t;
		memset(ent, 0, sizeof(*ent));
		cl_entities.pushBack(ent);
	}
	
	for(int i=0; i<MAX_PARSE_EDICTS; i++)
	{
		entity_state_t *ent = new entity_state_t;
		memset(ent, 0, sizeof(*ent));
		cl_parse_entities.pushBack(ent);
	}
	*/
}

static void	CG_ShutdownClientGame()
{
#if defined(ODE)
	CG_ShutdownDynamics();
#endif
}


static void	CG_RunFrame()
{
	// predict all unacknowledged movements
	CG_PredictMovement();

	CG_CalcVrect();
	
	CG_TileClear();

	CG_RenderView();
	
	// update audio
	if(trap_CLS_GetConnectionState() != CA_ACTIVE)
		trap_S_Update(vec3_origin, vec3_origin, vec3_c(1, 0, 0), vec3_c(0, 1, 0), vec3_c(0, 0, 1));
	else
		trap_S_Update(cg.refdef.view_origin, cg.v_velocity, cg.v_forward, cg.v_right, cg.v_up);
}


static void	CG_UpdateConfig(int index, const std::string &configstring)
{
	//trap_Com_Printf("CG_UpdateConfig: %i '%s'\n", index, configstring.c_str());

	if(index == CS_MAPNAME)
	{
		std::string mapname = trap_CL_GetConfigString(CS_MAPNAME);
		
		trap_Com_Printf("CG_UpdateConfig: map '%s'\n", mapname.c_str());
		
		unsigned	map_checksum;		// for detecting cheater maps
		cg.world_cmodel = trap_CM_BeginRegistration(mapname, true, &map_checksum);
		
		if((int)map_checksum != atoi(trap_CL_GetConfigString(CS_MAPCHECKSUM)))
		{
			trap_Com_Error(ERR_DROP, "Local map version differs from server: %i != '%s'\n", map_checksum, trap_CL_GetConfigString(CS_MAPCHECKSUM));
			return;
		}
		trap_R_BeginRegistration(mapname);
	}
	else if(index >= CS_MODELS && index < CS_MODELS+MAX_MODELS)
	{
		//if(trap_CL_GetRefreshPrepped())
		{
			cg.model_draw[index-CS_MODELS] = trap_R_RegisterModel(configstring);
			cg.model_clip[index-CS_MODELS] = trap_CM_RegisterModel(configstring);
		}
	}
	else if(index >= CS_SHADERS && index < CS_SHADERS+MAX_SHADERS)
	{
		//if(trap_CL_GetRefreshPrepped())
			cg.shader_precache[index-CS_SHADERS] = trap_R_RegisterPic(configstring);
	}
	else if(index >= CS_ANIMATIONS && index < CS_ANIMATIONS+MAX_ANIMATIONS)
	{
		//if(trap_CL_GetRefreshPrepped())
			cg.animation_precache[index-CS_ANIMATIONS] = trap_R_RegisterAnimation(configstring);
	}
	else if(index >= CS_SOUNDS && index < CS_SOUNDS+MAX_SOUNDS)
	{
		//if(trap_CL_GetRefreshPrepped())
			cg.sound_precache[index-CS_SOUNDS] = trap_S_RegisterSound(configstring);
	}
	else if(index >= CS_LIGHTS && index < CS_LIGHTS+MAX_LIGHTS)
	{
		//if(trap_CL_GetRefreshPrepped())
			cg.light_precache[index-CS_LIGHTS] = trap_R_RegisterLight(configstring);
	}
	else if(index >= CS_PLAYERSKINS && index < CS_PLAYERSKINS+MAX_CLIENTS)
	{
		//if(trap_CL_GetRefreshPrepped())
			CG_ParseClientinfo(index-CS_PLAYERSKINS);
	}
}



/*
=================
GetCCameAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/

#ifdef __cplusplus
extern "C" {
#endif

cg_export_t*	GetCGameAPI(cg_import_t *import)
{
	extern cg_import_t cgi;
	cgi = *import;

	cg_globals.apiversion			= CG_API_VERSION;

	cg_globals.Init				= CG_InitClientGame;
	cg_globals.Shutdown			= CG_ShutdownClientGame;
	
	cg_globals.CG_BeginFrame		= CG_BeginFrame;
	cg_globals.CG_AddEntity			= CG_AddEntity;
	cg_globals.CG_UpdateEntity		= CG_UpdateEntity;
	cg_globals.CG_RemoveEntity		= CG_RemoveEntity;
	cg_globals.CG_EndFrame			= CG_EndFrame;
	
	cg_globals.CG_RunFrame			= CG_RunFrame;
	cg_globals.CG_UpdateConfig		= CG_UpdateConfig;
	
	cg_globals.CG_ParseLayout		= CG_ParseLayout;
	cg_globals.CG_ParseTEnt			= CG_ParseTEnt;
	cg_globals.CG_ParseMuzzleFlash		= CG_ParseMuzzleFlash;
	cg_globals.CG_PrepRefresh		= CG_PrepRefresh;
	cg_globals.CG_GetEntitySoundOrigin	= CG_GetEntitySoundOrigin;
	
	cg_globals.CG_ClearState		= CG_ClearState;
			
	cg_globals.CG_ParseInventory		= CG_ParseInventory;
	
	cg_globals.CG_DrawChar			= CG_DrawChar;
	cg_globals.CG_DrawString		= CG_DrawString;
	
	cg_globals.CG_CenterPrint		= CG_CenterPrint;
			
	cg_globals.CG_ParseClientinfo		= CG_ParseClientinfo;
	
	Swap_Init();

	return &cg_globals;
}


#ifdef __cplusplus
}
#endif



#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void 	Com_Error(err_type_e type, const char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	trap_Com_Error(type, "%s", text);
}

void 	Com_Printf(const char *fmt, ...)
{
	va_list		argptr;
	char		text[1024];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	trap_Com_Printf("%s", text);
}
#endif



