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
#include "ui_local.h"


char *bindnames[][2] =
{
{"+attack1",		"1st attack"},
{"+attack2",		"2nd attack"},
{"weapprev", 		"prev weapon"},
{"weapnext", 		"next weapon"},
{"+forward", 		"walk forward"},
{"+back", 		"backpedal"},
{"+left", 		"turn left"},
{"+right", 		"turn right"},
{"+speed", 		"run"},
{"+moveleft", 		"step left"},
{"+moveright", 		"step right"},
{"+strafe", 		"sidestep"},
{"+lookup", 		"look up"},
{"+lookdown", 		"look down"},
{"centerview", 		"center view"},
{"+mlook", 		"mouse look"},
{"+klook", 		"keyboard look"},
{"+moveup",		"up / jump"},
{"+movedown",		"down / crouch"},

{"inven",		"inventory"},
{"invuse",		"use item"},
{"invdrop",		"drop item"},
{"invprev",		"prev item"},
{"invnext",		"next item"},

{"cmd help", 		"help computer" },
{ 0, 0 }
};

int		keys_cursor;
static int		bind_grab;


static void	M_UnbindCommand(char *command)
{
	int		j;
	int		l;
	char	*b;

	l = strlen(command);

	for (j=0 ; j<256 ; j++)
	{
		b = trap_Key_GetBinding(j);
		if (!b)
			continue;
		if (!strncmp (b, command, l) )
			trap_Key_SetBinding (j, "");
	}
}

static void	M_FindKeysForCommand(char *command, int *twokeys)
{
	//Com_Printf("M_FindKeysForCommand: '%s' '%s'\n", command, twokeys);

	int		count;
	int		j;
	int		l;
	char	*b;

	twokeys[0] = twokeys[1] = -1;
	l = strlen(command);
	count = 0;

	for(j=0; j<256; j++)
	{
		b = trap_Key_GetBinding(j);
		if(!b)
			continue;

		if(!strncmp(b, command, l))
		{
			twokeys[count] = j;
			count++;
			
			if(count == 2)
				break;
		}
	}
}

static void	KeyCursorDrawFunc(menu_framework_c *menu)
{
	if(bind_grab)
		Menu_DrawChar(menu->_x, menu->_y + menu->_cursor * 9, '=', color_white, FONT_NONE);
	else
		Menu_DrawChar(menu->_x, menu->_y + menu->_cursor * 9, 12 + ((int)(trap_Sys_Milliseconds() / 250) & 1), color_white, FONT_NONE);
}

static void	DrawKeyBindingFunc(void *self)
{
	int keys[2];
	menu_action_c *a = (menu_action_c *) self;

	M_FindKeysForCommand( bindnames[a->_localdata[0]][0], keys);
	
	int cwidth = Menu_GetCharWidth(a->_fontflags);

	if(keys[0] == -1)
	{
		
		Menu_DrawString(a->_name.length()*(cwidth+1) + a->_x + a->_parent->_x, a->_y + a->_parent->_y, "???", a->_fontflags);
		//Menu_DrawString(a->_x + a->_parent->_x + (CHAR_SMALL_WIDTH*30), a->_y + a->_parent->_y, "???", FONT_NONE);
	}
	else
	{
		const std::string name = trap_Key_KeynumToString(keys[0]);

		Menu_DrawString(a->_x + a->_parent->_x + 16, a->_y + a->_parent->_y, name, a->_fontflags);

		int x = name.length() * cwidth;

		if(keys[1] != -1)
		{
			Menu_DrawString(a->_name.length()*(cwidth+1) + a->_x + a->_parent->_x + 24 + x, a->_y + a->_parent->_y, "or", FONT_NONE);
			Menu_DrawString(a->_name.length()*(cwidth+1) + a->_x + a->_parent->_x + 48 + x, a->_y + a->_parent->_y, trap_Key_KeynumToString(keys[1]), FONT_NONE);
		}
	}
}

static void	KeyBindingFunc(void *self)
{
	menu_action_c *a = (menu_action_c*) self;
	int keys[2];

	M_FindKeysForCommand(bindnames[a->_localdata[0]][0], keys);

	if(keys[1] != -1)
		M_UnbindCommand(bindnames[a->_localdata[0]][0]);

	bind_grab = true;
}

class menu_keys_framework_c : public menu_framework_c
{
public:
	virtual void	draw()
	{
		adjustCursor(1);
		drawGeneric();
	}
	
	virtual std::string	keyDown(int key)
	{
		menu_action_c *item = (menu_action_c*) getItemAtCursor();

		if(bind_grab)
		{
			if(key != K_ESCAPE && key != '`')
			{
				char cmd[1024];

				Com_sprintf(cmd, sizeof(cmd), "bind \"%s\" \"%s\"\n", trap_Key_KeynumToString(key), bindnames[item->_localdata[0]][0]);
				trap_Cbuf_InsertText(cmd);
			}
			
			setStatusBar("enter to change, backspace to clear");
			bind_grab = false;
			return menu_out_sound;
		}
		
		switch(key)
		{
			case K_KP_ENTER:
			case K_ENTER:
				KeyBindingFunc(item);
				setStatusBar("press a key or button for this action");
				return menu_in_sound;
			
			case K_BACKSPACE:		// delete bindings
			case K_DEL:				// delete bindings
			case K_KP_DEL:
				M_UnbindCommand(bindnames[item->_localdata[0]][0]);
				return menu_out_sound;
				
			default:
				return defaultKeyDown(key);
		}
	}
};

static menu_keys_framework_c	s_keys_menu;
static menu_action_c		s_keys_attack_action;
static menu_action_c		s_keys_attack2_action;
static menu_action_c		s_keys_change_prev_weapon_action;
static menu_action_c		s_keys_change_next_weapon_action;
static menu_action_c		s_keys_walk_forward_action;
static menu_action_c		s_keys_backpedal_action;
static menu_action_c		s_keys_turn_left_action;
static menu_action_c		s_keys_turn_right_action;
static menu_action_c		s_keys_run_action;
static menu_action_c		s_keys_step_left_action;
static menu_action_c		s_keys_step_right_action;
static menu_action_c		s_keys_sidestep_action;
static menu_action_c		s_keys_look_up_action;
static menu_action_c		s_keys_look_down_action;
static menu_action_c		s_keys_center_view_action;
static menu_action_c		s_keys_mouse_look_action;
static menu_action_c		s_keys_keyboard_look_action;
static menu_action_c		s_keys_move_up_action;
static menu_action_c		s_keys_move_down_action;
static menu_action_c		s_keys_inventory_action;
static menu_action_c		s_keys_inv_use_action;
static menu_action_c		s_keys_inv_drop_action;
static menu_action_c		s_keys_inv_prev_action;
static menu_action_c		s_keys_inv_next_action;

static menu_action_c		s_keys_help_computer_action;





static void	Keys_MenuInit()
{
	int	i = 0;
	int	y = 0;
	int	y_offset = CHAR_SMALL_HEIGHT + 5;

	s_keys_menu._x = (int)(trap_VID_GetWidth() * 0.30);
	s_keys_menu._cursordraw = KeyCursorDrawFunc;

	//s_keys_attack_action._type	= MTYPE_ACTION;
	//s_keys_attack_action._flags  = QMF_GRAYED;
	s_keys_attack_action._fontflags	= FONT_SMALL;
	s_keys_attack_action._x		= 0;
	s_keys_attack_action._y		= y;
	s_keys_attack_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_attack_action._localdata[0] = i;
	s_keys_attack_action._name	= bindnames[s_keys_attack_action._localdata[0]][1];

	//s_keys_attack2_action._type	= MTYPE_ACTION;
	//s_keys_attack2_action._flags  = QMF_GRAYED;
	s_keys_attack2_action._fontflags	= FONT_SMALL;
	s_keys_attack2_action._x		= 0;
	s_keys_attack2_action._y		= y += y_offset;
	s_keys_attack2_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_attack2_action._localdata[0] = ++i;
	s_keys_attack2_action._name	= bindnames[s_keys_attack2_action._localdata[0]][1];

	//s_keys_change_prev_weapon_action._type	= MTYPE_ACTION;
	//s_keys_change_prev_weapon_action._flags  = QMF_GRAYED;
	s_keys_change_prev_weapon_action._fontflags	= FONT_SMALL;
	s_keys_change_prev_weapon_action._x		= 0;
	s_keys_change_prev_weapon_action._y		= y += y_offset;
	s_keys_change_prev_weapon_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_change_prev_weapon_action._localdata[0] = ++i;
	s_keys_change_prev_weapon_action._name	= bindnames[s_keys_change_prev_weapon_action._localdata[0]][1];

	//s_keys_change_next_weapon_action._type	= MTYPE_ACTION;
	//s_keys_change_next_weapon_action._flags  = QMF_GRAYED;
	s_keys_change_next_weapon_action._fontflags	= FONT_SMALL;
	s_keys_change_next_weapon_action._x		= 0;
	s_keys_change_next_weapon_action._y		= y += y_offset;
	s_keys_change_next_weapon_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_change_next_weapon_action._localdata[0] = ++i;
	s_keys_change_next_weapon_action._name	= bindnames[s_keys_change_next_weapon_action._localdata[0]][1];

	//s_keys_walk_forward_action._type	= MTYPE_ACTION;
	//s_keys_walk_forward_action._flags  = QMF_GRAYED;
	s_keys_walk_forward_action._fontflags	= FONT_SMALL;
	s_keys_walk_forward_action._x		= 0;
	s_keys_walk_forward_action._y		= y += y_offset;
	s_keys_walk_forward_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_walk_forward_action._localdata[0] = ++i;
	s_keys_walk_forward_action._name	= bindnames[s_keys_walk_forward_action._localdata[0]][1];

	//s_keys_backpedal_action._type	= MTYPE_ACTION;
	//s_keys_backpedal_action._flags  = QMF_GRAYED;
	s_keys_backpedal_action._fontflags	= FONT_SMALL;
	s_keys_backpedal_action._x		= 0;
	s_keys_backpedal_action._y		= y += y_offset;
	s_keys_backpedal_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_backpedal_action._localdata[0] = ++i;
	s_keys_backpedal_action._name	= bindnames[s_keys_backpedal_action._localdata[0]][1];

	//s_keys_turn_left_action._type	= MTYPE_ACTION;
	//s_keys_turn_left_action._flags  = QMF_GRAYED;
	s_keys_turn_left_action._x		= 0;
	s_keys_turn_left_action._y		= y += y_offset;
	s_keys_turn_left_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_turn_left_action._localdata[0] = ++i;
	s_keys_turn_left_action._name	= bindnames[s_keys_turn_left_action._localdata[0]][1];

	//s_keys_turn_right_action._type	= MTYPE_ACTION;
	//s_keys_turn_right_action._flags  = QMF_GRAYED;
	s_keys_turn_right_action._x		= 0;
	s_keys_turn_right_action._y		= y += y_offset;
	s_keys_turn_right_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_turn_right_action._localdata[0] = ++i;
	s_keys_turn_right_action._name	= bindnames[s_keys_turn_right_action._localdata[0]][1];

	//s_keys_run_action._type	= MTYPE_ACTION;
	//s_keys_run_action._flags  = QMF_GRAYED;
	s_keys_run_action._x		= 0;
	s_keys_run_action._y		= y += y_offset;
	s_keys_run_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_run_action._localdata[0] = ++i;
	s_keys_run_action._name	= bindnames[s_keys_run_action._localdata[0]][1];

	//s_keys_step_left_action._type	= MTYPE_ACTION;
	//s_keys_step_left_action._flags  = QMF_GRAYED;
	s_keys_step_left_action._x		= 0;
	s_keys_step_left_action._y		= y += y_offset;
	s_keys_step_left_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_step_left_action._localdata[0] = ++i;
	s_keys_step_left_action._name	= bindnames[s_keys_step_left_action._localdata[0]][1];

	//s_keys_step_right_action._type	= MTYPE_ACTION;
	//s_keys_step_right_action._flags  = QMF_GRAYED;
	s_keys_step_right_action._x		= 0;
	s_keys_step_right_action._y		= y += y_offset;
	s_keys_step_right_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_step_right_action._localdata[0] = ++i;
	s_keys_step_right_action._name	= bindnames[s_keys_step_right_action._localdata[0]][1];

	//s_keys_sidestep_action._type	= MTYPE_ACTION;
	//s_keys_sidestep_action._flags  = QMF_GRAYED;
	s_keys_sidestep_action._x		= 0;
	s_keys_sidestep_action._y		= y += y_offset;
	s_keys_sidestep_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_sidestep_action._localdata[0] = ++i;
	s_keys_sidestep_action._name	= bindnames[s_keys_sidestep_action._localdata[0]][1];

	//s_keys_look_up_action._type	= MTYPE_ACTION;
	//s_keys_look_up_action._flags  = QMF_GRAYED;
	s_keys_look_up_action._x		= 0;
	s_keys_look_up_action._y		= y += y_offset;
	s_keys_look_up_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_look_up_action._localdata[0] = ++i;
	s_keys_look_up_action._name	= bindnames[s_keys_look_up_action._localdata[0]][1];

	//s_keys_look_down_action._type	= MTYPE_ACTION;
	//s_keys_look_down_action._flags  = QMF_GRAYED;
	s_keys_look_down_action._x		= 0;
	s_keys_look_down_action._y		= y += y_offset;
	s_keys_look_down_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_look_down_action._localdata[0] = ++i;
	s_keys_look_down_action._name	= bindnames[s_keys_look_down_action._localdata[0]][1];

	//s_keys_center_view_action._type	= MTYPE_ACTION;
	//s_keys_center_view_action._flags  = QMF_GRAYED;
	s_keys_center_view_action._x		= 0;
	s_keys_center_view_action._y		= y += y_offset;
	s_keys_center_view_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_center_view_action._localdata[0] = ++i;
	s_keys_center_view_action._name	= bindnames[s_keys_center_view_action._localdata[0]][1];

	//s_keys_mouse_look_action._type	= MTYPE_ACTION;
	//s_keys_mouse_look_action._flags  = QMF_GRAYED;
	s_keys_mouse_look_action._x		= 0;
	s_keys_mouse_look_action._y		= y += y_offset;
	s_keys_mouse_look_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_mouse_look_action._localdata[0] = ++i;
	s_keys_mouse_look_action._name	= bindnames[s_keys_mouse_look_action._localdata[0]][1];

	//s_keys_keyboard_look_action._type	= MTYPE_ACTION;
	//s_keys_keyboard_look_action._flags  = QMF_GRAYED;
	s_keys_keyboard_look_action._x		= 0;
	s_keys_keyboard_look_action._y		= y += y_offset;
	s_keys_keyboard_look_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_keyboard_look_action._localdata[0] = ++i;
	s_keys_keyboard_look_action._name	= bindnames[s_keys_keyboard_look_action._localdata[0]][1];

	//s_keys_move_up_action._type	= MTYPE_ACTION;
	//s_keys_move_up_action._flags  = QMF_GRAYED;
	s_keys_move_up_action._x		= 0;
	s_keys_move_up_action._y		= y += y_offset;
	s_keys_move_up_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_move_up_action._localdata[0] = ++i;
	s_keys_move_up_action._name	= bindnames[s_keys_move_up_action._localdata[0]][1];

	//s_keys_move_down_action._type	= MTYPE_ACTION;
	//s_keys_move_down_action._flags  = QMF_GRAYED;
	s_keys_move_down_action._x		= 0;
	s_keys_move_down_action._y		= y += y_offset;
	s_keys_move_down_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_move_down_action._localdata[0] = ++i;
	s_keys_move_down_action._name	= bindnames[s_keys_move_down_action._localdata[0]][1];

	//s_keys_inventory_action._type	= MTYPE_ACTION;
	//s_keys_inventory_action._flags  = QMF_GRAYED;
	s_keys_inventory_action._x		= 0;
	s_keys_inventory_action._y		= y += y_offset;
	s_keys_inventory_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_inventory_action._localdata[0] = ++i;
	s_keys_inventory_action._name	= bindnames[s_keys_inventory_action._localdata[0]][1];

	//s_keys_inv_use_action._type	= MTYPE_ACTION;
	//s_keys_inv_use_action._flags  = QMF_GRAYED;
	s_keys_inv_use_action._x		= 0;
	s_keys_inv_use_action._y		= y += y_offset;
	s_keys_inv_use_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_inv_use_action._localdata[0] = ++i;
	s_keys_inv_use_action._name	= bindnames[s_keys_inv_use_action._localdata[0]][1];

	//s_keys_inv_drop_action._type	= MTYPE_ACTION;
	//s_keys_inv_drop_action._flags  = QMF_GRAYED;
	s_keys_inv_drop_action._x		= 0;
	s_keys_inv_drop_action._y		= y += y_offset;
	s_keys_inv_drop_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_inv_drop_action._localdata[0] = ++i;
	s_keys_inv_drop_action._name	= bindnames[s_keys_inv_drop_action._localdata[0]][1];

	//s_keys_inv_prev_action._type	= MTYPE_ACTION;
	//s_keys_inv_prev_action._flags  = QMF_GRAYED;
	s_keys_inv_prev_action._x		= 0;
	s_keys_inv_prev_action._y		= y += y_offset;
	s_keys_inv_prev_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_inv_prev_action._localdata[0] = ++i;
	s_keys_inv_prev_action._name	= bindnames[s_keys_inv_prev_action._localdata[0]][1];

	//s_keys_inv_next_action._type	= MTYPE_ACTION;
	//s_keys_inv_next_action._flags  = QMF_GRAYED;
	s_keys_inv_next_action._x		= 0;
	s_keys_inv_next_action._y		= y += y_offset;
	s_keys_inv_next_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_inv_next_action._localdata[0] = ++i;
	s_keys_inv_next_action._name	= bindnames[s_keys_inv_next_action._localdata[0]][1];

	//s_keys_help_computer_action._type	= MTYPE_ACTION;
	//s_keys_help_computer_action._flags  = QMF_GRAYED;
	s_keys_help_computer_action._x		= 0;
	s_keys_help_computer_action._y		= y += y_offset;
	s_keys_help_computer_action._ownerdraw = DrawKeyBindingFunc;
	s_keys_help_computer_action._localdata[0] = ++i;
	s_keys_help_computer_action._name	= bindnames[s_keys_help_computer_action._localdata[0]][1];

	s_keys_menu.addItem(&s_keys_attack_action);
	s_keys_menu.addItem(&s_keys_attack2_action);
	s_keys_menu.addItem(&s_keys_change_prev_weapon_action);
	s_keys_menu.addItem(&s_keys_change_next_weapon_action);
	s_keys_menu.addItem(&s_keys_walk_forward_action);
	s_keys_menu.addItem(&s_keys_backpedal_action);
	s_keys_menu.addItem(&s_keys_turn_left_action);
	s_keys_menu.addItem(&s_keys_turn_right_action);
	s_keys_menu.addItem(&s_keys_run_action);
	s_keys_menu.addItem(&s_keys_step_left_action);
	s_keys_menu.addItem(&s_keys_step_right_action);
	s_keys_menu.addItem(&s_keys_sidestep_action);
	s_keys_menu.addItem(&s_keys_look_up_action);
	s_keys_menu.addItem(&s_keys_look_down_action);
	s_keys_menu.addItem(&s_keys_center_view_action);
	s_keys_menu.addItem(&s_keys_mouse_look_action);
	s_keys_menu.addItem(&s_keys_keyboard_look_action);
	s_keys_menu.addItem(&s_keys_move_up_action);
	s_keys_menu.addItem(&s_keys_move_down_action);

	s_keys_menu.addItem(&s_keys_inventory_action);
	s_keys_menu.addItem(&s_keys_inv_use_action);
	s_keys_menu.addItem(&s_keys_inv_drop_action);
	s_keys_menu.addItem(&s_keys_inv_prev_action);
	s_keys_menu.addItem(&s_keys_inv_next_action);

	s_keys_menu.addItem(&s_keys_help_computer_action);

	s_keys_menu.setStatusBar("enter to change, backspace to clear");
	s_keys_menu.center();
}

void	M_Menu_Keys_f()
{
	Keys_MenuInit();
	M_PushMenu(&s_keys_menu);
}
