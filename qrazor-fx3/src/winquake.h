/*
Copyright (C) 1997-2001 Id Software, Inc.

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
// winquake.h: Win32-specific Quake header file

//#pragma warning( disable : 4229 )  // mgraph gets this

#include <windows.h>

#if ! defined(BSPCOMPILER_ONLY) && ! defined(DEDICATED_ONLY)
//#include <dsound.h>

#define	WINDOW_STYLE	(WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE)
#endif


extern	HINSTANCE	global_hInstance;

extern DWORD gSndBufSize;

extern bool		ActiveApp, Minimized;

#if ! defined(BSPCOMPILER_ONLY) && ! defined(DEDICATED_ONLY)
//extern LPDIRECTSOUND pDS;
//extern LPDIRECTSOUNDBUFFER pDSBuf;

extern HWND cl_hwnd;
extern int		window_center_x, window_center_y;
extern RECT		window_rect;

void IN_Activate (bool active);
void IN_MouseEvent (int mstate);
#endif



