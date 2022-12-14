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
// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_player.h"
#include "g_item.h"
#include "g_target.h"



void	G_BeginIntermission(const g_target_changelevel_c *target)
{
	g_entity_c	*spot;
	g_player_c	*player;

	if(level.intermission_time)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for(int i=0; i<maxclients->getInteger(); i++)
	{
		player = (g_player_c*)g_entities[1+i];
		
		if(!player->_r.inuse)
			continue;
			
		if(player->_health <= 0)
			player->respawn();
	}

	level.intermission_time = level.time;
	level.changemap = target->getMapName();

	if(level.changemap.find("*"))
	{
		if(g_coop->getInteger())
		{
			for(int i=0; i<maxclients->getInteger(); i++)
			{
				player = (g_player_c*)g_entities[1+i];
				
				if(!player->_r.inuse)
					continue;
					
				// strip players of all keys between units
				/*
				for(int n=0; n < (int)g_items.size(); n++)
				{
					if(g_items[n]->getFlags() & IT_KEY)
						player->_pers.inventory[n] = 0;
				}
				*/
			}
		}
	}
	else
	{
		if(!g_deathmatch->getInteger())
		{
			level.intermission_exit = true;		// go immediately to the next level
			return;
		}
	}

	level.intermission_exit = false;

	// find an intermission spot
	spot = G_FindByClassName(NULL, "info_player_intermission");
	if(!spot)
	{	
		// the map creator forgot to put in an intermission point...
		spot = G_FindByClassName(NULL, "info_player_start");
		
		if(!spot)
			spot = G_FindByClassName(NULL, "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		int r = rand() & 3;
		while(r--)
		{
			spot = G_FindByClassName(spot, "info_player_intermission");
			
			if(!spot)	// wrap around the list
				spot = G_FindByClassName(spot, "info_player_intermission");
		}
	}

	level.intermission_origin = spot->_s.origin;
	level.intermission_angles = spot->_angles;

	// move all clients to the intermission point
	for(int i=0; i<maxclients->getInteger(); i++)
	{
		player = (g_player_c*)g_entities[1+i];
		
		if(!player->_r.inuse)
			continue;
			
		player->moveToIntermission();
	}
}




