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
#include "ui_public.h"
#include "ui_local.h"



static ui_export_t	ui_globals;


#define	MAIN_ITEMS	5


class menu_main_console_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		// the proper way to do this is probably to have ToggleConsole_f accept a parameter
		/*
		if(trap_CL_GetAttractloop())
		{
			trap_Cbuf_AddText("killserver\n");
			return;
		}
		*/

		trap_Con_ClearTyping();
		trap_Con_ClearNotify();

		M_ForceMenuOff();
	
		trap_Key_SetKeyDest(KEY_CONSOLE);
	}
};

class menu_main_multiplayer_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_Multiplayer_f();
	}
};

class menu_main_audio_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_Audio_f();
	}
};

class menu_main_video_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_Video_f();
	}
};

class menu_main_options_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_Options_f();
	}
};

class menu_main_controls_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_Keys_f();
	}
};

class menu_main_credicts_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_Credits_f();
	}
};

class menu_main_quit_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_Quit_f();
	}
};


static menu_framework_c	s_main_menu;
static menu_main_console_c	s_main_console_action;
static menu_main_multiplayer_c	s_main_multiplayer_action;
static menu_main_audio_c	s_main_audio_action;
static menu_main_video_c	s_main_video_action;
static menu_main_options_c	s_main_options_action;
static menu_main_controls_c	s_main_controls_action;
static menu_main_credicts_c	s_main_credits_action;
static menu_main_quit_c		s_main_quit_action;


const char *menu_in_sound		= "sound/misc/menu1.wav";
const char *menu_move_sound		= "sound/misc/menu2.wav";
const char *menu_out_sound		= "sound/misc/menu3.wav";


static void	M_Main_MenuInit()
{
	int	y = 0;
	int	y_offset = CHAR_BIG_HEIGHT + 10;
	
	s_main_menu._x = (int)(trap_VID_GetWidth() * 0.50) - (5*CHAR_BIG_WIDTH);

	s_main_console_action._flags 		= QMF_LEFT_JUSTIFY;
	s_main_console_action._fontflags	= FONT_CHROME | FONT_BIG;
	s_main_console_action._x		= 0;
	s_main_console_action._y		= y = 0;
	s_main_console_action._name		= "CONSOLE";
	
	s_main_multiplayer_action._flags  	= QMF_LEFT_JUSTIFY;
	s_main_multiplayer_action._fontflags	= FONT_CHROME | FONT_BIG;
	s_main_multiplayer_action._x		= 0;
	s_main_multiplayer_action._y		= y += y_offset;
	s_main_multiplayer_action._name		= "MULTIPLAYER";
	
	s_main_audio_action._flags  		= QMF_LEFT_JUSTIFY;
	s_main_audio_action._fontflags		= FONT_CHROME | FONT_BIG;
	s_main_audio_action._x			= 0;
	s_main_audio_action._y			= y += y_offset;
	s_main_audio_action._name		= "AUDIO";
	
	s_main_video_action._flags  		= QMF_LEFT_JUSTIFY;
	s_main_video_action._fontflags		= FONT_CHROME | FONT_BIG;
	s_main_video_action._x			= 0;
	s_main_video_action._y			= y += y_offset;
	s_main_video_action._name		= "VIDEO";
	
	s_main_options_action._flags  		= QMF_LEFT_JUSTIFY;
	s_main_options_action._fontflags	= FONT_CHROME | FONT_BIG;
	s_main_options_action._x		= 0;
	s_main_options_action._y		= y += y_offset;
	s_main_options_action._name		= "OPTIONS";
	
	s_main_controls_action._flags  		= QMF_LEFT_JUSTIFY;
	s_main_controls_action._fontflags	= FONT_CHROME | FONT_BIG;
	s_main_controls_action._x		= 0;
	s_main_controls_action._y		= y += y_offset;
	s_main_controls_action._name		= "CONTROLS";
	
	s_main_credits_action._flags  		= QMF_LEFT_JUSTIFY;
	s_main_credits_action._fontflags	= FONT_CHROME | FONT_BIG;
	s_main_credits_action._x		= 0;
	s_main_credits_action._y		= y += y_offset;
	s_main_credits_action._name		= "CREDITS";
	
	s_main_quit_action._flags  		= QMF_LEFT_JUSTIFY;
	s_main_quit_action._fontflags		= FONT_CHROME | FONT_BIG;
	s_main_quit_action._x			= 0;
	s_main_quit_action._y			= y += y_offset;
	s_main_quit_action._name		= "QUIT";
	
	
	s_main_menu.addItem(&s_main_console_action);
	s_main_menu.addItem(&s_main_multiplayer_action);
	s_main_menu.addItem(&s_main_audio_action);
	s_main_menu.addItem(&s_main_video_action);
	s_main_menu.addItem(&s_main_options_action);
	s_main_menu.addItem(&s_main_controls_action);
	s_main_menu.addItem(&s_main_credits_action);
	s_main_menu.addItem(&s_main_quit_action);
	
	s_main_menu.center();
	s_main_menu.setStatusBar("");
}

void	M_Menu_Main_f()
{
	M_Main_MenuInit();
	M_PushMenu(&s_main_menu);
}



static void	M_Shutdown()
{
	//TODO
}

/*
=================
GetUIAPI

Returns a pointer to the structure with all entry points
and global variables
=================
*/

#ifdef __cplusplus
extern "C" {
#endif

ui_export_t*	GetUIAPI (ui_import_t *import)
{
	extern ui_import_t uii;
	uii = *import;

	ui_globals.apiversion		= UI_API_VERSION;


	ui_globals.Init			= M_Init;
	ui_globals.Shutdown		= M_Shutdown;
	ui_globals.M_Keydown		= M_Keydown;
	ui_globals.M_Draw		= M_Draw;
	ui_globals.M_Menu_Main_f	= M_Menu_Main_f;
	ui_globals.M_ForceMenuOff	= M_ForceMenuOff;
	ui_globals.M_AddToServerList	= M_AddToServerList;

	return &ui_globals;
}


#ifdef __cplusplus
}
#endif


#ifndef UI_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void 	Com_Error(err_type_e type, const char *fmt, ...)
{
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	extern ui_import_t uii;
	uii.Com_Error(type, "%s", text);
}

void 	Com_Printf(const char *fmt, ...)
{
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	extern ui_import_t uii;
	uii.Com_Printf("%s", text);
}
#endif



