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

class menu_options_framework_c : public menu_framework_c
{
public:
	virtual void	draw()
	{
		M_Banner("OPTIONS");
	
		adjustCursor(1);
		drawGeneric();
	}
};

class menu_options_defaults_c : public menu_action_c
{
public:
	menu_options_defaults_c()
	{
		_name	= "reset defaults";
	}

	virtual void	callback()
	{
		trap_Cbuf_AddText("exec default.cfg\n");
		trap_Cbuf_Execute();
	
		M_PopMenu();
	}
};

class menu_options_customize_controls_c : public menu_action_c
{
public:
	menu_options_customize_controls_c()
	{
		_name	= "customize controls";
	}

	virtual void	callback()
	{
		M_Menu_Keys_f();
	}
};

class menu_options_sensitivity_c : public menu_slider_c
{
public:
	menu_options_sensitivity_c()
	{
		_name	= "mouse speed";
		
		_minvalue = 2;
		_maxvalue = 22;
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("sensitivity", _curvalue / 2.0F);
	}
};

class menu_options_alwaysrun_c : public menu_spincontrol_c
{
public:
	menu_options_alwaysrun_c()
	{
		_name	= "always run";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("cl_run", _curvalue);
	}
};

class menu_options_invertmouse_c : public menu_spincontrol_c
{
public:
	menu_options_invertmouse_c()
	{
		_name	= "invert mouse";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("m_pitch", -trap_Cvar_VariableValue("m_pitch"));
	}
};

class menu_options_crosshair_c : public menu_spincontrol_c
{
public:
	menu_options_crosshair_c()
	{
		_name	= "crosshair";
		
		_itemnames.push_back("none");
		_itemnames.push_back("cross");
		_itemnames.push_back("dot");
		_itemnames.push_back("angle");
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("cg_crosshair", _curvalue );
	}
};

class menu_options_joystick_c : public menu_spincontrol_c
{
public:
	menu_options_joystick_c()
	{
		_name	= "use joystick";
		
		_itemnames.push_back("no");
		_itemnames.push_back("yes");
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("in_joystick", _curvalue );
	}
};



static menu_options_framework_c			s_options_menu;
static menu_options_defaults_c				s_options_defaults_action;
//static menu_options_customize_controls_c		s_options_customize_controls_action;
static menu_options_sensitivity_c			s_options_sensitivity_slider;
static menu_options_alwaysrun_c				s_options_alwaysrun_box;
static menu_options_invertmouse_c			s_options_invertmouse_box;
static menu_options_crosshair_c				s_options_crosshair_box;
static menu_options_joystick_c				s_options_joystick_box;

static void	Options_MenuInit()
{
	int	y = 0;
	int	y_offset = CHAR_MEDIUM_HEIGHT + 5;

	s_options_menu._x = (int)(trap_VID_GetWidth() * 0.50 - (CHAR_MEDIUM_WIDTH * 10));
	
	s_options_sensitivity_slider._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_options_sensitivity_slider._x		= 0;
	s_options_sensitivity_slider._y		= y = 0;
	s_options_sensitivity_slider._curvalue	= trap_Cvar_VariableValue("sensitivity") * 2;
	
	s_options_alwaysrun_box._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_options_alwaysrun_box._x		= 0;
	s_options_alwaysrun_box._y		= y += y_offset;
	trap_Cvar_SetValue("cl_run", trap_Cvar_ClampVariable( "cl_run", 0 , 1));
	s_options_alwaysrun_box._curvalue		= trap_Cvar_VariableInteger("cl_run");
	
	s_options_invertmouse_box._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_options_invertmouse_box._x		= 0;
	s_options_invertmouse_box._y		= y += y_offset;
	s_options_invertmouse_box._curvalue	= trap_Cvar_VariableInteger("m_pitch") < 0;

	s_options_crosshair_box._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_options_crosshair_box._x		= 0;
	s_options_crosshair_box._y		= y += y_offset;
	trap_Cvar_SetValue("cg_crosshair", trap_Cvar_ClampVariable( "cg_crosshair", 0, 3));
	s_options_crosshair_box._curvalue		= trap_Cvar_VariableInteger("cg_crosshair");
	
	s_options_joystick_box._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_options_joystick_box._x		= 0;
	s_options_joystick_box._y		= y += y_offset;
	trap_Cvar_SetValue("in_joystick", trap_Cvar_ClampVariable( "in_joystick", 0, 1));
	s_options_joystick_box._curvalue		= trap_Cvar_VariableInteger("in_joystick");
	
//	s_options_customize_controls_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
//	s_options_customize_controls_action._x		= 0;
//	s_options_customize_controls_action._y		= y += y_offset;

	s_options_defaults_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_options_defaults_action._x		= 0;
	s_options_defaults_action._y		= y += y_offset;

	s_options_menu.addItem(&s_options_sensitivity_slider);
	s_options_menu.addItem(&s_options_alwaysrun_box);
	s_options_menu.addItem(&s_options_invertmouse_box);
	s_options_menu.addItem(&s_options_crosshair_box);
	s_options_menu.addItem(&s_options_joystick_box);
//	s_options_menu.addItem(&s_options_customize_controls_action);
	s_options_menu.addItem(&s_options_defaults_action);
	
	s_options_menu.center();
}

void	M_Menu_Options_f()
{
	Options_MenuInit();
	
	M_PushMenu(&s_options_menu);
}

