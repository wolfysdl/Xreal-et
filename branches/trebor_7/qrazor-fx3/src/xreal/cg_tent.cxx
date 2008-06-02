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


typedef enum
{
	ex_free, ex_explosion, ex_misc, ex_flash, ex_mflash, ex_poly, ex_poly2
} exptype_t;

typedef struct
{
	exptype_t	type;
	r_entity_t	ent;

	int		frames;
	float		light;
	vec3_t		lightcolor;
	float		start;
	int		baseframe;
} explosion_t;



#define	MAX_EXPLOSIONS	32
explosion_t	cl_explosions[MAX_EXPLOSIONS];


#define	MAX_BEAMS	32
typedef struct
{
	int			entity;
	int			dest_entity;
	int			model;
	int			endtime;
	vec3_t			offset;
	vec3_t			start, end;
} beam_t;

beam_t		cl_beams[MAX_BEAMS];
beam_t		cl_playerbeams[MAX_BEAMS];


#define	MAX_LASERS	32
typedef struct
{
	r_entity_t	ent;
	int			endtime;
} laser_t;
laser_t		cl_lasers[MAX_LASERS];


int	cl_sfx_ric1;
int	cl_sfx_ric2;
int	cl_sfx_ric3;
int	cl_sfx_lashit;
int	cl_sfx_spark5;
int	cl_sfx_spark6;
int	cl_sfx_spark7;
int	cl_sfx_railg;
int	cl_sfx_rockexp;
int	cl_sfx_grenexp;
int	cl_sfx_watrexp;
int	cl_sfx_footsteps[4];

/*
int		cl_mod_explode;
int		cl_mod_smoke;
int		cl_mod_flash;
int		cl_mod_parasite_segment;
int		cl_mod_grapple_cable;
int		cl_mod_parasite_tip;
int		cl_mod_explo4;
int		cl_mod_bfg_explo;
*/

void	CG_RegisterTEntSounds()
{
	char	name[MAX_QPATH];


	cl_sfx_ric1 = cgi.S_RegisterSound("world/ric1.wav");
	cl_sfx_ric2 = cgi.S_RegisterSound("world/ric2.wav");
	cl_sfx_ric3 = cgi.S_RegisterSound("world/ric3.wav");
	cl_sfx_lashit = cgi.S_RegisterSound("weapons/lashit.wav");
	cl_sfx_spark5 = cgi.S_RegisterSound("world/spark5.wav");
	cl_sfx_spark6 = cgi.S_RegisterSound("world/spark6.wav");
	cl_sfx_spark7 = cgi.S_RegisterSound("world/spark7.wav");
	cl_sfx_railg = cgi.S_RegisterSound("weapons/railgf1a.wav");
	cl_sfx_rockexp = cgi.S_RegisterSound("sounds/e1/we_sidewinderexp.wav");
	cl_sfx_grenexp = cgi.S_RegisterSound("weapons/grenlx1a.wav");
	cl_sfx_watrexp = cgi.S_RegisterSound("weapons/xpld_wat.wav");
	
	cgi.S_RegisterSound("player/land1.wav");
	cgi.S_RegisterSound("player/fall2.wav");
	cgi.S_RegisterSound("player/fall1.wav");

	for(int i=0; i<4; i++)
	{
		Com_sprintf(name, sizeof(name), "sound/player/step%i.wav", i+1);
		cl_sfx_footsteps[i] = cgi.S_RegisterSound(name);
	}
}

void	CG_RegisterTEntModels()
{
	/*
	cl_mod_explode = cgi.R_RegisterModel ("models/objects/explode/tris.md2");
	cl_mod_smoke = cgi.R_RegisterModel ("models/objects/smoke/tris.md2");
	cl_mod_flash = cgi.R_RegisterModel ("models/objects/flash/tris.md2");
	cl_mod_parasite_segment = cgi.R_RegisterModel ("models/monsters/parasite/segment/tris.md2");
	cl_mod_grapple_cable = cgi.R_RegisterModel ("models/ctf/segment/tris.md2");
	cl_mod_parasite_tip = cgi.R_RegisterModel ("models/monsters/parasite/tip/tris.md2");
	cl_mod_explo4 = cgi.R_RegisterModel ("models/objects/r_explode/tris.md2");
	cl_mod_bfg_explo = cgi.R_RegisterModel ("sprites/s_bfg2.sp2");
	*/

	/*
	cgi.R_RegisterModel ("models/objects/laser/tris.md2");
	cgi.R_RegisterModel ("models/objects/grenade2/tris.md2");
	cgi.R_RegisterModel ("models/weapons/v_machn/tris.md2");
	cgi.R_RegisterModel ("models/weapons/v_handgr/tris.md2");
	cgi.R_RegisterModel ("models/weapons/v_shotg2/tris.md2");
	cgi.R_RegisterModel ("models/objects/gibs/bone/tris.md2");
	cgi.R_RegisterModel ("models/objects/gibs/sm_meat/tris.md2");
	cgi.R_RegisterModel ("models/objects/gibs/bone2/tris.md2");
	

	cgi.R_RegisterPic("textures/pics/w_machinegun.pcx");
	cgi.R_RegisterPic("textures/pics/a_bullets.pcx");
	cgi.R_RegisterPic("textures/pics/i_health.pcx");
	cgi.R_RegisterPic("textures/pics/a_grenades.pcx");
	*/
}	

void 	CG_ClearTEnts()
{
	memset(cl_beams, 0, sizeof(cl_beams));
	memset(cl_explosions, 0, sizeof(cl_explosions));
	memset(cl_lasers, 0, sizeof(cl_lasers));
}

explosion_t*	CG_AllocExplosion()
{
	int		i;
	int		time;
	int		index;
	
	for (i=0 ; i<MAX_EXPLOSIONS ; i++)
	{
		if (cl_explosions[i].type == ex_free)
		{
			memset (&cl_explosions[i], 0, sizeof (cl_explosions[i]));
			return &cl_explosions[i];
		}
	}
	
	// find the oldest explosion
	time = cgi.CL_GetTime();
	
	index = 0;

	for (i=0 ; i<MAX_EXPLOSIONS ; i++)
		if (cl_explosions[i].start < time)
		{
			time = (int)cl_explosions[i].start;
			index = i;
		}
	memset (&cl_explosions[index], 0, sizeof (cl_explosions[index]));
	return &cl_explosions[index];
}

void 	CG_SmokeAndFlash(vec3_t origin)
{
	/*
	explosion_t	*ex;

	ex = CG_AllocExplosion ();
	Vector3_Copy (origin, ex->ent.origin);
	ex->type = ex_misc;
	ex->frames = 4;
	ex->start = cgi.cl->frame.servertime - 100;
	ex->ent.model = cl_mod_smoke;

	ex = CG_AllocExplosion ();
	Vector3_Copy (origin, ex->ent.origin);
	ex->type = ex_flash;
	ex->ent.flags = RF_FULLBRIGHT;
	ex->frames = 2;
	ex->start = cgi.cl->frame.servertime - 100;
	ex->ent.model = cl_mod_flash;
	*/
}



static void	CG_ParseLaser(message_c &msg, int colors)
{
	vec3_c	start;
	vec3_c	end;
	laser_t	*l;
	int		i;

	msg.readVector3(start);
	msg.readVector3(end);

	for(i=0, l=cl_lasers ; i< MAX_LASERS ; i++, l++)
	{
		if(l->endtime < cgi.CL_GetTime())
		{
			l->ent.flags = RF_TRANSLUCENT;
			Vector3_Copy(start, l->ent.origin);
			Vector3_Copy(end, l->ent.origin2);
			//l->ent.color[3] = 0.30;
			l->ent.custom_shader = -1;
			l->ent.custom_skin = -1;//(colors >> ((rand() % 4)*8)) & 0xff;
			l->ent.model = -1;
			l->ent.frame = 4;
			l->endtime = cgi.CL_GetTime() + 100;
			return;
		}
	}
}

void	CG_ParseTEnt(message_c &msg)
{
	int		type;
	vec3_c		pos;
	vec3_c		pos2;
	vec3_c		dir;
	//explosion_t	*ex;
	int		cnt;
	vec4_c		color;
	int		r;


	type = msg.readByte();
	
	color.clear();

	switch(type)
	{
	case TE_BLOOD:			// bullet hitting flesh
		msg.readVector3(pos);
		msg.readDir(dir);
		
		CG_ParticleSpray (PART_BLOOD4, pos, dir, color, 20+(int)X_crand()*100);
		break;

	case TE_GUNSHOT:			// bullet hitting wall
	case TE_SPARKS:
	case TE_BULLET_SPARKS:
		msg.readVector3(pos);
		msg.readDir(dir);
		
		if(type == TE_GUNSHOT)
		{	
			color.set(0.5, 0.5, 0.5, 1.0);
			CG_ParticleSpray(PART_SPARK2, pos, dir, color, 40);
		}
		else if(type == TE_SPARKS)
		{
			CG_ParticleSpray((cg_particle_type_e)(PART_SPARK+(int)X_crand()*2), pos, dir, color, 10);
		}
		
		if(type != TE_SPARKS)
		{
			CG_SmokeAndFlash(pos);
			
			// impact sound
			cnt = rand()&15;
			if(cnt == 1)
				cgi.S_StartSound(pos, 0, 0, cl_sfx_ric1);
				
			else if(cnt == 2)
				cgi.S_StartSound(pos, 0, 0, cl_sfx_ric2);
				
			else if(cnt == 3)
				cgi.S_StartSound(pos, 0, 0, cl_sfx_ric3);
		}

		break;
		
	case TE_SCREEN_SPARKS:
	case TE_SHIELD_SPARKS:
		msg.readVector3(pos);
		msg.readDir(dir);
		
		if(type == TE_SCREEN_SPARKS)
		{
			//CG_ParticleEffect (pos, dir, 0xd0, 40);	
			color.set(0, 0, 1, 1);
			CG_ParticleSpray(PART_SPARK2, pos, dir, color, 40);
		}
		else
		{
			//CG_ParticleEffect (pos, dir, 0xb0, 40);
			color.set(0, 0, 1, 1);
			CG_ParticleSpray(PART_SPARK2, pos, dir, color, 40);
		}
		//FIXME : replace or remove this sound
		cgi.S_StartSound(pos, 0, 0, cl_sfx_lashit);
		break;
		
	case TE_SHOTGUN:			// bullet hitting wall
		msg.readVector3(pos);
		msg.readDir(dir);
		
		//CG_ParticleEffect (pos, dir, 0, 20);
		color.set(0.5, 0.5, 0.5, 1);
		CG_ParticleSpray (PART_SPARK2, pos, dir, color, 20);
		CG_SmokeAndFlash(pos);
		break;

	case TE_SPLASH:			// bullet hitting water
		cnt = msg.readByte();
		msg.readVector3(pos);
		msg.readDir(dir);
		r = msg.readByte();
		
		/*if (r > 6)
			color = 0x00;
		else
			color = splash_color[r];
		*/
		
		//CG_ParticleEffect (pos, dir, color, cnt);

		if(r == SPLASH_SPARKS)
		{
			CG_ParticleSpray((cg_particle_type_e)(PART_SPARK+(int)X_frand()*2), pos, dir, color, (int)cnt/2);
		
			r = rand() & 3;
			if(r == 0)
				cgi.S_StartSound(pos, 0, 0, cl_sfx_spark5);
				
			else if (r == 1)
				cgi.S_StartSound(pos, 0, 0, cl_sfx_spark6);
				
			else
				cgi.S_StartSound(pos, 0, 0, cl_sfx_spark7);
		}
		else
		{
			color.set(0.85, 0.85, 1.0, 0.8);
			dir.clear();
			
			CG_ParticleSpray(PART_SPLASH, pos, dir, color, cnt);
		}
		break;

	case TE_LASER_SPARKS:
		cnt = msg.readByte();
		msg.readVector3(pos);
		msg.readDir(dir);
		msg.readByte();
		//color = cgi.MSG_ReadByte (msg);
		
		//CG_ParticleEffect2 (pos, dir, color, cnt);
		CG_ParticleSpray (PART_SPARK2, pos, dir, color, 30+(int)X_frand()*30);		//well ignore particle count
		break;

	case TE_BLASTER:			// blaster hitting wall
		msg.readVector3(pos);
		msg.readDir(dir);
		
		CG_ParticleSpray(PART_BLASTER, pos, dir, color, 30+(int)X_frand()*30);

		/*
		ex = CG_AllocExplosion ();
		Vector3_Copy (pos, ex->ent.origin);
		ex->ent.angles[0] = acos(dir[2])/M_PI*180;
	
		if (dir[0])
			ex->ent.angles[1] = atan2(dir[1], dir[0])/M_PI*180;
		else if (dir[1] > 0)
			ex->ent.angles[1] = 90;
		else if (dir[1] < 0)
			ex->ent.angles[1] = 270;
		else
			ex->ent.angles[1] = 0;

		ex->type = ex_misc;
		ex->ent.flags = RF_FULLBRIGHT|RF_TRANSLUCENT;
		ex->start = cgi.cl->frame.servertime - 100;
		ex->light = 150;
		ex->lightcolor[0] = 1;
		ex->lightcolor[1] = 1;
		ex->ent.model = cl_mod_explode;
		ex->frames = 4;
		cgi.S_StartSound (pos,  0, 0, cl_sfx_lashit, 1, ATTN_NORM, 0);
		*/
		break;
		
	case TE_RAILTRAIL:			// railgun effect
		msg.readVector3(pos);
		msg.readVector3(pos2);
		
		//CG_RailTrail (pos, pos2);
		CG_ParticleTrail(PART_RAIL, pos, pos2, color, 0.05);
		
		cgi.S_StartSound (pos2, 0, 0, cl_sfx_railg);
		break;

	case TE_EXPLOSION2:
	case TE_GRENADE_EXPLOSION:
	case TE_GRENADE_EXPLOSION_WATER:
		msg.readVector3(pos);
	
		/*
		ex = CG_AllocExplosion ();
		Vector3_Copy (pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
		ex->start = cgi.cl->frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 1.0;
		ex->lightcolor[1] = 0.5;
		ex->lightcolor[2] = 0.5;
		ex->ent.model = cl_mod_explo4;
		ex->frames = 19;
		ex->baseframe = 30;
		ex->ent.angles[1] = rand() % 360;
		*/
		
		CG_ExplosionParticles(pos);
		if (type == TE_GRENADE_EXPLOSION_WATER)
			cgi.S_StartSound (pos, 0, 0, cl_sfx_watrexp);
		else
			cgi.S_StartSound (pos, 0, 0, cl_sfx_grenexp);
		break;
	
	case TE_EXPLOSION1:
	case TE_ROCKET_EXPLOSION:
	case TE_ROCKET_EXPLOSION_WATER:
		msg.readVector3(pos);
		
		/*
		ex = CG_AllocExplosion ();
		Vector3_Copy (pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT | RF_NOSHADOW;
		ex->start = cgi.cl->frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 1.0;
		ex->lightcolor[1] = 0.5;
		ex->lightcolor[2] = 0.5;
		ex->ent.angles[1] = rand() % 360;
		ex->ent.model = cl_mod_explo4;
		
		if (X_frand() < 0.5)
			ex->baseframe = 15;
		ex->frames = 15;
		*/
		
		//CG_ExplosionParticles(pos);
		
		if(type == TE_ROCKET_EXPLOSION_WATER)
		{
			cgi.S_StartSound(pos, 0, 0, cl_sfx_watrexp);
		}
		else
		{
			cgi.S_StartSound(pos, 0, 0, cl_sfx_rockexp);
			
			CG_ParticleSpray(PART_EXPSMOKE, pos, dir, color, 1024);
		}
		break;

	case TE_BFG_EXPLOSION:
		msg.readVector3(pos);
		/*
		ex = CG_AllocExplosion ();
		Vector3_Copy (pos, ex->ent.origin);
		ex->type = ex_poly;
		ex->ent.flags = RF_FULLBRIGHT;
		ex->start = cgi.cl->frame.servertime - 100;
		ex->light = 350;
		ex->lightcolor[0] = 0.0;
		ex->lightcolor[1] = 1.0;
		ex->lightcolor[2] = 0.0;
		ex->ent.model = cl_mod_bfg_explo;
		ex->ent.flags |= RF_TRANSLUCENT;
		ex->ent.color[3] = 0.30;
		ex->frames = 4;
		*/
		break;

	case TE_BFG_BIGEXPLOSION:
		msg.readVector3(pos);
		CG_BFGExplosionParticles (pos);
		break;

	case TE_BFG_LASER:
		CG_ParseLaser(msg, 0xd0d1d2d3);
		break;

	case TE_BUBBLETRAIL:
		msg.readVector3(pos);
		msg.readVector3(pos2);
		CG_BubbleTrail (pos, pos2);
		break;
	
	


	default:
		cgi.Com_Error (ERR_DROP, "CG_ParseTEnt: bad type");
	}
}

void	CG_AddBeams()
{
	int			i,j;
	beam_t		*b;
	vec3_t		dist, org;
	float		d;
	r_entity_t	ent;
	float		yaw, pitch;
	float		forward;
	float		len, steps;
	float		model_length;
	
	// update beams
	for (i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++)
	{
		if (!b->model || b->endtime < cgi.CL_GetTime())
			continue;

		// if coming from the player, update the start position
		if (b->entity == cgi.playernum+1)	// entity 0 is the world
		{
			Vector3_Copy(cg.refdef.view_origin, b->start);
			b->start[2] -= 22;	// adjust for view height
		}
		Vector3_Add (b->start, b->offset, org);

		// calculate pitch and yaw
		Vector3_Subtract (b->end, org, dist);

		if (dist[1] == 0 && dist[0] == 0)
		{
			yaw = 0;
			if (dist[2] > 0)
				pitch = 90;
			else
				pitch = 270;
		}
		else
		{
		// PMM - fixed to correct for pitch of 0
			if (dist[0])
				yaw = (atan2(dist[1], dist[0]) * 180 / M_PI);
			else if (dist[1] > 0)
				yaw = 90;
			else
				yaw = 270;
			if (yaw < 0)
				yaw += 360;
	
			forward = sqrt (dist[0]*dist[0] + dist[1]*dist[1]);
			pitch = (atan2(dist[2], forward) * -180.0 / M_PI);
			if (pitch < 0)
				pitch += 360.0;
		}

		// add new entities for the beams
		d = Vector3_Normalize(dist);

		memset (&ent, 0, sizeof(ent));
		
		model_length = 30.0;
		
		steps = ceil(d/model_length);
		len = (d-model_length)/(steps-1);

		while(d > 0)
		{
			Vector3_Copy (org, ent.origin);
			ent.model = b->model;
			{
				vec3_c angles;
				angles.set(pitch, yaw, rand()%360);
				ent.quat.fromAngles(angles);
			}
			
//			cgi.Com_Printf("B: %d -> %d\n", b->entity, b->dest_entity);
//			cgi.R_AddEntityToScene(ent);

			for (j=0 ; j<3 ; j++)
				org[j] += dist[j]*len;
			d -= model_length;
		}
	}
}



void	CG_AddExplosions()
{
	r_entity_t	*ent;
	int			i;
	explosion_t	*ex;
	float		frac;
	int			f;

	memset (&ent, 0, sizeof(ent));

	for (i=0, ex=cl_explosions ; i< MAX_EXPLOSIONS ; i++, ex++)
	{
		if (ex->type == ex_free)
			continue;
			
		frac = (cgi.CL_GetTime() - ex->start)/100.0;
		f = (int)floor(frac);

		ent = &ex->ent;

		switch (ex->type)
		{
		case ex_mflash:
			if (f >= ex->frames-1)
				ex->type = ex_free;
			break;
		case ex_misc:
			if (f >= ex->frames-1)
			{
				ex->type = ex_free;
				break;
			}
			//ent->color[3] = 1.0 - frac/(ex->frames-1);
			break;
		case ex_flash:
			if (f >= 1)
			{
				ex->type = ex_free;
				break;
			}
			//ent->color[3] = 1.0;
			break;
		case ex_poly:
			if (f >= ex->frames-1)
			{
				ex->type = ex_free;
				break;
			}

			//ent->color[3] = (16.0 - (float)f)/16.0;

			if (f < 10)
			{
				ent->custom_shader = -1;//_num = (f>>1);
				//if (ent->skin_num < 0)
				//	ent->skin_num = 0;
			}
			else
			{
				ent->flags |= RF_TRANSLUCENT;
				if (f < 13)
					ent->custom_shader = -1; //_num = 5;
				else
					ent->custom_shader = -1; //_num = 6;
			}
			break;
		case ex_poly2:
			if (f >= ex->frames-1)
			{
				ex->type = ex_free;
				break;
			}

			//ent->color[3] = (5.0 - (float)f)/5.0;
			ent->custom_skin = -1;
			ent->flags |= RF_TRANSLUCENT;
			break;
		case ex_free:		//Tr3B -  gcc3 -Wall
		case ex_explosion:
			break;
		}

		if (ex->type == ex_free)
			continue;
		
		/*
		if (ex->light)
		{
			r_light_t light;
			
			light.origin = ent->origin;
			//light.radius = ex->light * ent->color[3];
			light.color = ex->lightcolor;
							
			cgi.R_AddLightToScene(light);
		}
		*/

		Vector3_Copy (ent->origin, ent->origin2);

		if (f < 0)
			f = 0;
		ent->frame = ex->baseframe + f + 1;
		ent->frame_old = ex->baseframe + f;
		ent->backlerp = 1.0 - cg.frame_lerp;

//		cgi.R_AddEntityToScene(*ent);
	}
}

void	CG_AddLasers()
{
	laser_t		*l;
	int			i;

	for (i=0, l=cl_lasers ; i< MAX_LASERS ; i++, l++)
	{
		//if (l->endtime >= cgi.CL_GetTime())
//			cgi.R_AddEntityToScene(l->ent);
	}
}