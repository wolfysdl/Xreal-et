/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
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
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_projectile.h"
#include "g_local.h"

/*
================================================================================
				BOLT PROJECTILE
================================================================================
*/
g_projectile_bolt_c::g_projectile_bolt_c(g_entity_c *activator, const vec3_c &start, const vec3_c &dir, int damage, vec_t speed)
{
	vec3_c tmp(dir);
	tmp.normalize();
	
	vectoangles(tmp, _angles);
	
	// setup network entity state
	_s.type = ET_LIGHT_OMNI;	//TODO change to ET_PROJECTILE_BOLT
	_s.origin = start;
	_s.quat.fromAngles(_angles);	// set rotation
	
	_s.lengths.set(50, 50, 50);	// light radius
	_s.shaderparms[0] = color_yellow[0];	// light color
	_s.shaderparms[1] = color_yellow[1];	// light color
	_s.shaderparms[2] = color_yellow[2];	// light color
	
//	_s.index_model = gi.SV_ModelIndex("models/items/projectiles/rocket.md3");
//	_s.index_sound = gi.SV_SoundIndex("sounds/e1/we_ionflyby.wav");
	_s.index_light = gi.SV_LightIndex("lights/defaultpointlight");
	
	_s.renderfx = RF_NOSHADOW;	// don't cast a shadow in the renderer
	
	// setup shared values between server and game
	_r.inuse = true;
	_r.owner = activator;
	
	// setup values private to game code and this entity
	_classname = "bolt";
	_nextthink = level.time + 2;	// die after 2 seconds
	_dmg = damage;
	
	// setup ODE rigid body
	_body->setPosition(start);
	_body->setQuaternion(_s.quat);
	_body->setLinearVel(dir * speed);
	_body->setGravityMode(0);
	
	dMass m;
	dMassSetSphereTotal(&m, 0.1, 8);
	_body->setMass(&m);
	
	// setup ODE collision
	g_geom_info_c *geom_info = new g_geom_info_c(this, NULL, NULL);
	
	d_geom_c *geom = new d_sphere_c(g_ode_space->getId(), 8);
	geom->setBody(_body->getId());
	geom->setData(geom_info);
	geom->setCollideBits(MASK_SHOT);
	
	_geoms.insert(std::make_pair(geom, geom_info));
}

void	g_projectile_bolt_c::think()
{
	remove();
}


bool	g_projectile_bolt_c::touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf)
{
	if(other == _r.owner)
		return false;
		
	gi.Com_Printf("g_projectile_bolt_c::touch: touching entity %i '%s'\n", other->_s.getNumber(), other->getClassName());

	if(surf && surf->hasFlags(X_SURF_SKY))
	{
		remove();
		return false;
	}

	if(other->_takedamage)
	{
		updateVelocity();
	
		// damage the other entity
		other->takeDamage(this, (g_entity_c*)_r.owner, _s.velocity, _s.origin, plane._normal, _dmg, 1, DAMAGE_ENERGY, MOD_BLASTER);
	}
	else
	{
		// bolt hitting wall, do some sparks particle effects
		gi.SV_WriteByte(SVC_TEMP_ENTITY);
		gi.SV_WriteByte(TE_BLASTER);
		gi.SV_WritePosition(_s.origin);
		gi.SV_WriteDir(plane._normal);
			
		gi.SV_Multicast(_s.origin, MULTICAST_PVS);
	}

	remove();	// remove from world when done
	
	return true;	// create ODE contact joint
}










/*
================================================================================
				GRENADE PROJECTILE
================================================================================
*/
g_projectile_grenade_c::g_projectile_grenade_c(g_entity_c *activator, const vec3_c &start, const vec3_c &aimdir, int damage, int speed, float timer, float damage_radius)
{
	//gi.Com_Printf("g_projectile_grenade_c::ctor:\n");

	vec3_c	dir;
	vec3_c	forward, right, up;

	vectoangles (aimdir, dir);
	Angles_ToVectors (dir, forward, right, up);
	
	_s.origin = start;
	
	_s.velocity = aimdir * speed;
	_s.velocity += up * (200 + crandom() * 10.0);
	_s.velocity += right * (crandom() * 10.0);
	
	_movetype = MOVETYPE_BOUNCE;
	_r.inuse = true;
	_r.clipmask = MASK_SHOT;
	_r.solid = SOLID_BBOX;
	_s.effects |= EF_GRENADE;
	_s.renderfx = RF_NOSHADOW;
	_r.bbox.zero();
	_s.index_model = gi.SV_ModelIndex ("models/ammo/grenade1.md3");
	_r.owner = activator;
	//touch = Grenade_Touch;
	_nextthink = level.time + timer;
	//grenade->think = Grenade_Explode;
	_dmg = damage;
	_dmg_radius = damage_radius;
	_classname = "grenade";
	
	_body->setLinearVel(_s.velocity);
	_body->setAngularVel(300, 300, 300);
}

void	g_projectile_grenade_c::think()
{
	vec3_t		origin;
	int		mod;

	//if (_r.owner->getClient())
	//	PlayerNoise((g_entity_c*)_r.owner, _s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (_enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		Vector3_Add (_enemy->_r.bbox._mins, _enemy->_r.bbox._maxs, v);
		Vector3_MA (_enemy->_s.origin, 0.5, v, v);
		Vector3_Subtract (_s.origin, v, v);
		points = _dmg - 0.5 * Vector3_Length (v);
		Vector3_Subtract (_enemy->_s.origin, _s.origin, dir);
		
		if (_spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
			
		_enemy->takeDamage(this, (g_entity_c*)_r.owner, dir, _s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (_spawnflags & 2)
		mod = MOD_HELD_GRENADE;
		
	else if (_spawnflags & 1)
		mod = MOD_HG_SPLASH;
		
	else
		mod = MOD_G_SPLASH;
		
	//T_RadiusDamage(this, (g_entity_c*)_r.owner, _dmg, _enemy, _dmg_radius, mod);

//	Vector3_MA(_s.origin, -0.02, _velocity, origin);
	gi.SV_WriteByte (SVC_TEMP_ENTITY);
	
	if (_waterlevel)
	{
		if (_groundentity)
			gi.SV_WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.SV_WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (_groundentity)
			gi.SV_WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.SV_WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.SV_WritePosition (origin);
	gi.SV_Multicast (_s.origin, MULTICAST_PHS);

	remove();
}



bool	g_projectile_grenade_c::touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf)
{
	if(other == _r.owner)
		return false;

	if(surf && surf->hasFlags(X_SURF_SKY))
	{
		remove();
		return false;
	}

	if(!other->_takedamage)
	{
		if(_spawnflags & 1)
		{
			if(random() > 0.5)
				gi.SV_StartSound(NULL, this, CHAN_VOICE, gi.SV_SoundIndex("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.SV_StartSound(NULL, this, CHAN_VOICE, gi.SV_SoundIndex("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.SV_StartSound(NULL, this, CHAN_VOICE, gi.SV_SoundIndex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return true;
	}

	_enemy = other;
	
	think();
	
	return true;
}

/*
static void	Grenade_Proxy(g_entity_c *ent)
{
	g_entity_c	*blip = NULL;

	if (level.time > ent->delay)
	{
		Grenade_Explode(ent);
		return;
	}

	ent->think = Grenade_Proxy;
	
	while ((blip = findradius (blip, ent->s.origin, 100)) != NULL)
	{
		if (!blip->r.client)
			continue;

		if (blip == ent->r.owner)
			continue;

		if (!blip->takedamage)
			continue;

		if (!blip->health)
			continue;

		if (!G_IsVisible (ent, blip))
			continue;

		ent->think = Grenade_Explode;
		break;
	}

	ent->nextthink = level.time + FRAMETIME;
}
*/




/*
================================================================================
				Q2/Q3A ROCKET PROJECTILE
================================================================================
*/
g_projectile_rocket_c::g_projectile_rocket_c(g_entity_c *activator, const vec3_c &start, const vec3_c &dir, int damage, int speed, float damage_radius, float radius_damage)
{
	//gi.Com_Printf("g_projectile_rocket_c::ctor\n");

	_s.origin = start;
	vectoangles(dir, _angles);
	_s.quat.fromAngles(_angles);

	_s.type = ET_GENERIC;
	_s.velocity = dir * speed;
	_s.lengths.set(200, 200, 200);
//	_s.color = color_yellow;
	_s.shaderparms[0] = color_yellow[0];	// light color
	_s.shaderparms[1] = color_yellow[1];	// light color
	_s.shaderparms[2] = color_yellow[2];	// light color
	_s.index_model = gi.SV_ModelIndex("models/items/projectiles/rocket.md3");
//	_s.index_sound = gi.SV_SoundIndex("sounds/weapons/sidewinder/we_sidewinderfly.wav");
	_s.index_light = gi.SV_LightIndex("lights/defaultpointlight");
	
	_r.inuse = true;
	_r.clipmask = MASK_SHOT;
	_r.solid = SOLID_BBOX;
	_s.effects |= EF_ROCKET;
	_s.renderfx = RF_NOSHADOW;
	_r.bbox.zero();
	_r.owner = activator;
	
	_speed = speed;
	_nextthink = level.time + 5;	//level.time + FRAMETIME;
	_dmg = damage;
	_radius_dmg = (int)radius_damage;
	_dmg_radius = damage_radius;
	_classname = "rocket";
	
	
	// setup rigid body
	_body->setPosition(start);
	_body->setQuaternion(_s.quat);
	_body->setLinearVel(_s.velocity);
	_body->setGravityMode(0);
	
	// setup mass
	dMass m;
//	dMassSetSphereTotal(&m, 5.0, 8);
	dMassSetBoxTotal(&m, 5.0, 12, 4, 4);
	_body->setMass(&m);
	
	// setup geom
	g_geom_info_c *geom_info = new g_geom_info_c(this, NULL, NULL);
	
//	d_geom_c *geom = new d_sphere_c(g_ode_space->getId(), 8);
	d_geom_c *geom = new d_box_c(g_ode_space->getId(), 12, 4, 4);
	
	geom->setBody(_body->getId());
	geom->setData(geom_info);
	
	_geoms.insert(std::make_pair(geom, geom_info));
}

void	g_projectile_rocket_c::think()
{
	remove();

	/*
	if((getSpawnTime() + 20) == level.time)	// die after 20 seconds
	{
		remove();
	}
	
	_nextthink = level.time + FRAMETIME;
	*/
}

bool	g_projectile_rocket_c::touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf)
{
	if(other == _r.owner)
		return false;
		
	gi.Com_Printf("g_projectile_rocket_c::touch: touching entity %i '%s'\n", other->_s.getNumber(), other->getClassName());
	
	if(surf && surf->hasFlags(X_CONT_AREAPORTAL))
	{
		gi.Com_Printf("g_projectile_rocket_c::touch: touching areaportal\n");
	}

	if(surf && surf->hasFlags(X_SURF_NOIMPACT))
	{
		remove();
		return false;
	}

	// calculate position for the explosion entity
	vec3_c origin = _s.origin + _s.velocity * -0.02;

	if(other->_takedamage)
	{
		updateVelocity();
		other->takeDamage(this, (g_entity_c*)_r.owner, _s.velocity, _s.origin, plane._normal, _dmg, 0, 0, MOD_ROCKET);
	}
	
	G_RadiusDamage(this, (g_entity_c*)_r.owner, _radius_dmg, other, _dmg_radius, MOD_R_SPLASH);

	gi.SV_WriteByte(SVC_TEMP_ENTITY);
	
	if(_waterlevel)
		gi.SV_WriteByte(TE_ROCKET_EXPLOSION_WATER);
	else
		gi.SV_WriteByte(TE_ROCKET_EXPLOSION);
		
	gi.SV_WritePosition(origin);
	gi.SV_Multicast(_s.origin, MULTICAST_ALL);
	
	remove();
	
	return true;
}





