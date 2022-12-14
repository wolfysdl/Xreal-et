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
#ifndef X_SHARED_H
#define X_SHARED_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include <boost/dynamic_bitset.hpp>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <deque>
#include <algorithm>
// qrazor-fx ----------------------------------------------------------------





/*
================================================================================
				MISC DEFINES
================================================================================
*/
typedef unsigned char 		byte;
typedef unsigned char 		byte_t;
typedef unsigned char 		uchar_t;
typedef unsigned short		ushort_t;
typedef unsigned int		uint_t;
typedef unsigned int		index_t;

#define VFS_BASEDIRNAME		"xreal"


#if (defined _M_IX86 || defined __i386__) && !defined C_ONLY && !defined __sun__
#define id386	1
#else
#define id386	0
#endif

#if defined _M_ALPHA && !defined C_ONLY
#define idaxp	1
#else
#define idaxp	0
#endif


short	BigShort(short l);
short	LittleShort(short l);
int	BigLong(int l);
int	LittleLong(int l);
float	BigFloat(float l);
float	LittleFloat(float l);

void	Swap_Init();


char*	va(char *format, ...);


enum sys_event_type_e
{
	SE_CONSOLE,
	SE_KEY,
	SE_MOUSE,
	SE_PACKET
};

enum err_type_e
{
	ERR_FATAL		= 0,		// exit the entire game with a popup window
	ERR_DROP		= 1,		// print to console and disconnect from game
	ERR_DISCONNECT		= 2,		// don't kill server
	ERR_WARNING		= 3
};


void	Com_Printf(const char *fmt, ...);
void 	Com_Error(err_type_e type, const char *fmt, ...);
void*	Com_Alloc(int size);	// returns 0 filled memory
void*	Com_ReAlloc(void *ptr, int size);
void	Com_Free(void *ptr);


//
// memory handling
//
#if 0
inline void*	operator new (size_t size)
{
	return Com_Alloc(size);
}

inline void	operator delete (void *ptr)
{
	Com_Free(ptr);
}
#endif




/*
================================================================================
				WIN32 DEFINES
================================================================================
*/
/*
#ifdef _WIN32
// unknown pragmas are SUPPOSED to be ignored, but....
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)		// truncation from const double to float

#endif
*/


#ifdef _WIN32
	#define BUILDHOST		"Win32"
	#define VERSION			"0.3.5"
	
	#ifndef VFS_PKGDATADIR
		#define VFS_PKGDATADIR		"."
	#endif
	
	#define VFS_USERDATADIR		VFS_PKGDATADIR
	#define VFS_PKGLIBDIR		VFS_PKGDATADIR
#endif

/*
================================================================================
				LINUX DEFINES
================================================================================
*/
#ifdef __linux__
	#define BUILDHOST		"Linux"
	#define VERSION			"0.3.5"
	#define VFS_USERDATADIR		"~/.qrazor-fx"
	
	#ifndef VFS_PKGDATADIR
		#define VFS_PKGDATADIR		"."
	#endif
	
	#ifndef VFS_PKGLIBDIR
		#define VFS_PKGLIBDIR		"."
	#endif
#endif


/*
================================================================================
				MISC DEFINES 2
================================================================================
*/



#ifndef NULL
#define NULL ((void *)0)
#endif


#define	MAX_STRING_CHARS		1024		// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS		1024		// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS			1024		// max length of an individual token

#define	MAX_QPATH			128		// max length of a quake game pathname	Tr3B - increased to 128

#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			128		// max length of a filesystem pathname
#endif


enum
{
	

	MAX_CLIENTS			= 256,				// absolute limit
	
	MAX_ENTITIES_BITS		= 11,				// use only 11 bits to represent the entity number
	MAX_ENTITIES			= (1 << MAX_ENTITIES_BITS),	// must change protocol to increase more
	
	MAX_MODELS_BITS			= 12,
	MAX_MODELS			= (1 << MAX_MODELS_BITS),
	
	MAX_SHADERS_BITS		= 12,
	MAX_SHADERS			= (1 << MAX_SHADERS_BITS),
	
	MAX_ANIMATIONS_BITS		= 12,
	MAX_ANIMATIONS			= (1 << MAX_ANIMATIONS_BITS),
	
	MAX_SOUNDS_BITS			= 12,
	MAX_SOUNDS			= (1 << MAX_SOUNDS_BITS),
	
	MAX_LIGHTS_BITS			= 12,
	MAX_LIGHTS			= (1 << MAX_LIGHTS_BITS),
	
	MAX_ITEMS			= 256,
	MAX_GENERAL			= (MAX_CLIENTS*2)		// general config strings
};


enum exec_type_e
{
	EXEC_NOW		= 0,		// don't return until completed
	EXEC_INSERT		= 1,		// insert at current position, but don't run yet
	EXEC_APPEND		= 2		// add to end of the command buffer
};

// game print flags
enum g_print_level_e
{
	PRINT_LOW		= 0,		// pickup messages
	PRINT_MEDIUM		= 1,		// death messages
	PRINT_HIGH		= 2,		// critical messages
	PRINT_CHAT		= 3		// chat messages
};

// destination class for gi.multicast()
enum multicast_type_e
{
	MULTICAST_ALL,
	MULTICAST_PHS,
	MULTICAST_PVS,
	MULTICAST_ALL_R,
	MULTICAST_PHS_R,
	MULTICAST_PVS_R
};

enum
{
	FONT_NONE		= 0,
	FONT_SMALL		= 1,
	FONT_MEDIUM		= 2,
	FONT_BIG		= 4,
	FONT_SHADOWED		= 8,
	FONT_ALT		= 16,
	FONT_CHROME		= 32
};

enum char_width_e
{
	CHAR_SMALL_WIDTH	= 8,
	CHAR_MEDIUM_WIDTH	= 16,
	CHAR_BIG_WIDTH		= 32
};

enum char_height_e
{
	CHAR_SMALL_HEIGHT	= 8,
	CHAR_MEDIUM_HEIGHT	= 16,
	CHAR_BIG_HEIGHT		= 32
};


#define DEFAULT_PLAYERMODEL	"marine"
#define DEFAULT_PLAYERSKIN	"default"


/*
================================================================================
				MATH LIB
================================================================================
*/

#include "x_math.h"


/*
================================================================================
				COMMON UTILITIES
================================================================================
*/

const char*	Com_StripExtension(const std::string &name);

void		Com_StripFileName(const char *path, char *filename);
void		Com_FilePath(const char *in, char *out);

// data is an in/out parm, returns a parsed out token
char*		Com_Parse(char **data_p, bool allow_next_line = true);
int		Com_ParseInt(char **data_p, bool allow_next_line = true);
float		Com_ParseFloat(char **data_p, bool allow_next_line = true);
vec3_c		Com_ParseVec3(char **data_p);

void 		Com_sprintf(char *dest, int size, char *fmt, ...);


void		Com_Printf(const char *fmt, ...);
void 		Com_DPrintf(const char *fmt, ...);
void 		Com_Error(err_type_e type, const char *fmt, ...);

unsigned	Com_BlockChecksum(void *buffer, int length);



inline int	X_stricmp(const char *s1, const char *s2)
{
#if defined(WIN32)
	return _stricmp(s1, s2);
#else
	return strcasecmp(s1, s2);
#endif
}

inline int 	X_strcasecmp(const char *s1, const char *s2)
{
//	return X_strncasecmp (s1, s2, 99999);
	return X_stricmp(s1, s2);
}

inline bool	X_strequal(const char *a, const char *b)
{
	return (strcmp (a, b) == 0);
}

bool	X_strcaseequal(const char *a, const char *b);

inline bool	X_strnequal(const char *a, const char *b, unsigned int c)
{
	return (strncmp (a, b, c) == 0);
}

inline bool	X_strncaseequal(const char *a, const char *b, unsigned int c)
{
	return (strncasecmp (a, b, c) == 0);
}

int 	X_strncasecmp(const char *s1, const char *s2, int n);

char*	X_strlwr(char *s);

std::string	X_strlwr(const std::string &s);

char*	X_strupr(char *s);

void	X_strncpyz(char *dest, const char *src, int size);



// Tr3B - non-case-sensitive compare function object
class strcasecmp_c : public std::binary_function<std::string, std::string, bool>
{
public:
	bool operator()(const std::string &x, const std::string &y) const
	{
		return X_strcasecmp(x.c_str(), y.c_str()) < 0;
	}
};


// Tr3B - delete all slots != NULL
template<class seq>
inline void	X_purge(seq& c)
{
	typename seq::iterator i;
	
	for(i = c.begin(); i != c.end(); i++)
	{
		if(*i)
		{
			delete *i;
			*i = NULL;
		}
	}
}


// Tr3B - discover the size of an array
template<typename T, int size>
inline int	X_asz(T (&)[size])
{
	return size;
}




inline int	bitCount(uint_t n)
{
	assert(n);
		
	return (int)ceil(log2(n));
}

inline uint_t	toBytes(uint_t bits_num)
{
//	return (bits_num + 7) >> 3;
	return (bits_num % 8) ? (bits_num / 8)+1 : (bits_num / 8);
}

inline uint_t	toBits(uint_t bytes_num)
{
	return bytes_num*8;
}


inline void	bytesToBits(const std::vector<byte> &bytes, boost::dynamic_bitset<byte> &bits)
{
	int bits_num = toBits(bytes.size());
	
	bits = boost::dynamic_bitset<byte>(bits_num);
	
	for(int i=0; i<bits_num; i++)
		bits[i] = bytes[i >> 3] & (1 << (i & 7));
}

inline void	bitsToBytes(const boost::dynamic_bitset<byte> &bits, std::vector<byte> &bytes)
{
	bytes = std::vector<byte>(toBytes(bits.size()), 0);

	for(uint_t i=0; i<bits.size(); i++)
		bytes[i >> 3] |= (bits[i] << (i % 8));
}

/*
inline std::vector<byte>&	operator = (std::vector<byte> &bytes, const boost::dynamic_bitset<byte> &bits)
{
	bitsToBytes(bits, bytes);
	return bytes;
}
*/


/*
================================================================================
				VFS DEFINES
================================================================================
*/
enum vfs_mode_e 
{
	VFS_READ,
	VFS_WRITE,
	VFS_APPEND,
	VFS_APPEND_SYNC
};

enum vfs_seek_e
{
	VFS_SEEK_CUR,
	VFS_SEEK_END,
	VFS_SEEK_SET
};

enum vfs_filetype_e
{
	FT_REAL,
	FT_ZIP,
	FT_GZIP
};

struct VFILE
{
	std::string		filename;
	std::string		fullpath;
	
	vfs_filetype_e		type;
	
	bool			force_flush;
	int			pos;
	int			size;
	int			offset;
	
	void*			file;
};


/*
================================================================================
			KEY/VALUE INFO STRINGS
================================================================================
*/
#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192
#define	BIG_INFO_KEY		8192
#define	BIG_INFO_VALUE		8192


class info_c
{
public:
	// key/value access
	void 		setValueForKey(const std::string &key, const std::string &value);
	const char*	getValueForKey(const std::string &key);
	
	void 		removeKey(const std::string &key);
	void		clear()			{_map.clear();}
	
	// "\key\value" grammar import/export
	void		fromString(const std::string &s);
	const char*	toString() const;
	
	// console friendly print
	void		print() const;
	
private:
	std::map<std::string, std::string>	_map;
};



/*
================================================================================
				CVARS
================================================================================
*/
enum
{
	CVAR_NONE		= 0,
	CVAR_ARCHIVE		= (1<<0),	// set to cause it to be saved to vars.rc
								// used for system variables, not for player
								// specific configurations
	CVAR_USERINFO		= (1<<1),	// sent to server on connect or change
	CVAR_SERVERINFO		= (1<<2),	// sent in response to front end requests
	CVAR_SYSTEMINFO		= (1<<3),	// these cvars will be duplicated on all clients
	CVAR_INIT		= (1<<4),	// don't allow change from console at all, but can be set from the command line
	CVAR_LATCH		= (1<<5),	// will only change when C code next does
								// a Cvar_Get(), so it can't be changed
								// without proper initialization.  modified
								// will be set, even though the value hasn't
								// changed yet
	CVAR_ROM		= (1<<6),	// display only, cannot be set by user at all
	CVAR_USER_CREATED	= (1<<7),	// created by a set command
	CVAR_TEMP		= (1<<8),	// can be set even when cheats are disabled, but is not archived
	CVAR_CHEAT		= (1<<9),	// can not be changed if cheats are disabled
	CVAR_NORESTART		= (1<<10)	// do not clear when a cvar_restart is issued
};

// nothing outside the Cvar_*() functions should modify these fields!
class cvar_t
{
private:
	friend cvar_t*	Cvar_Get(const std::string &name, const std::string &values, uint_t flags);
	friend cvar_t*	Cvar_Set2(const std::string &name, const std::string &value, uint_t flags, bool force);
	friend void 	Cvar_SetLatchedVars();
	friend bool 	Cvar_Command();
	
	cvar_t(const std::string &name, const std::string &value, const std::vector<std::string> &values, uint_t flags)
	{
		assert(values.size());

		_name			= name;
		_values			= values;
		_index			= 0;
		_string			= value;
		_string_latched		= "";
		_flags			= flags;
		_modified		= true;
		_value			= atof(value.c_str());
		_integer		= atoi(value.c_str());
	}
public:
	inline const char*	getName() const			{return _name.c_str();}
	inline const char*	getString() const		{return _string.c_str();}
	inline const char*	getResetString() const		{return _values[0].c_str();}
	inline const char*	getNextString()
	{
		_index	= (_index+1) % _values.size();
		return _values[_index].c_str();
	}
	//! return values as comma separated list
	inline const char*	getValues() const
	{
		std::string s;
		for(uint_t i=0; i<_values.size(); i++)
		{
			s += _values[i];

			if(i != _values.size()-1)
				s += ",";
		}
		return s.c_str();
	}
	inline uint_t		getValuesNum() const		{return _values.size();}
	inline bool		hasValues() const		{return !_values.empty();}
	
	inline uint_t		getFlags() const		{return _flags;}
	inline bool		hasFlags(uint_t flag) const	{return _flags & flag;}
	
	inline bool		isModified() const		{return _modified;}
	inline void		isModified(bool val)		{_modified = val;}
	
	inline float		getValue() const		{return _value;}
	inline int		getInteger() const		{return _integer;}
	
private:	
	std::string	_name;
	std::vector<std::string>	
			_values;
	uint_t		_index;
	std::string	_string;
	std::string	_string_latched;	// for CVAR_LATCH vars
	uint_t		_flags;
	bool		_modified;		// set each time the cvar is changed
	uint_t		_modification_count;
	float		_value;
	int		_integer;
};


/*
================================================================================
			COLLISION DETECTION
================================================================================
*/
// contents flags are seperate bits
// a given brush can contribute multiple content bits

#define	X_CONT_NONE			0
#define	X_CONT_SOLID			(1<<0)		// an eye is never valid in a solid
#define	X_CONT_WINDOW			(1<<1)		// translucent, but not watery
#define	X_CONT_LAVA			(1<<2)
#define	X_CONT_SLIME			(1<<3)
#define	X_CONT_WATER			(1<<4)
#define X_CONT_BLOOD			(1<<5)
#define	X_CONT_FOG			(1<<6)
	
#define	X_CONT_NOTTEAM1			(1<<7)
#define	X_CONT_NOTTEAM2			(1<<8)
#define	X_CONT_NOBOTCLIP		(1<<9)
	
#define	X_CONT_AREAPORTAL		(1<<10)
	
#define	X_CONT_PLAYERCLIP		(1<<11)
#define	X_CONT_MONSTERCLIP		(1<<12)

#define	X_CONT_TELEPORTER		(1<<13)
#define	X_CONT_JUMPPAD			(1<<14)
#define	X_CONT_CLUSTERPORTAL		(1<<15)
#define	X_CONT_DONOTENTER		(1<<16)
#define	X_CONT_BOTCLIP			(1<<17)
#define	X_CONT_MOVER			(1<<18)
	
#define	X_CONT_ORIGIN			(1<<19)		// removed before bsping an entity

#define	X_CONT_BODY			(1<<20)		// should never be on a brush, only in game
#define	X_CONT_CORPSE			(1<<21)
#define	X_CONT_DETAIL			(1<<22)		// brushes to be added after vis leafs
#define	X_CONT_STRUCTURAL		(1<<23)
#define	X_CONT_TRANSLUCENT		(1<<24)		// auto set if any surface has trans
#define	X_CONT_TRIGGER			(1<<25)
#define	X_CONT_NODROP			(1<<26)		// don't leave bodies or items (death fog, lava)


#define X_SURF_NONE			0
#define	X_SURF_NODAMAGE			(1<<0)		// never give falling damage
#define	X_SURF_SLICK			(1<<1)		// effects game physics
#define	X_SURF_SKY			(1<<2)		// don't draw, but add to skybox
#define	X_SURF_LADDER			(1<<3)		// not used
#define	X_SURF_NOIMPACT			(1<<4)		// don't make missile explosions
#define	X_SURF_NOMARKS			(1<<5)		// don't leave missile marks
#define	X_SURF_FLESH			(1<<6)		// make flesh sounds and effects
#define	X_SURF_NODRAW			(1<<7)		// don't generate a drawsurface at all
#define	X_SURF_HINT			(1<<8)		// make a primary bsp splitter
#define	X_SURF_SKIP			(1<<9)		// completely ignore, allowing non-closed brushes
#define	X_SURF_NOLIGHTMAP		(1<<10)		// surface doesn't need a lightmap
#define	X_SURF_POINTLIGHT		(1<<11)		// generate lighting info at vertexes
#define	X_SURF_METALSTEPS		(1<<12)		// clanking footsteps
#define	X_SURF_NOSTEPS			(1<<13)		// no footstep sounds
#define	X_SURF_NONSOLID			(1<<14)		// don't collide against curves with this set
#define	X_SURF_LIGHTFILTER		(1<<15)		// act as a light filter during q3map -light
#define	X_SURF_ALPHASHADOW		(1<<16)		// do per-pixel light shadow casting in q3map
#define	X_SURF_NODLIGHT			(1<<17)		// never add dynamic lights
#define X_SURF_NOSHADOWS		(1<<18)		// don't cast shadows
#define X_SURF_NOSELFSHADOW		(1<<19)
#define	X_SURF_DUST			(1<<20)		// leave a dust trail when walking on this surface
#define	X_SURF_DISCRETE			(1<<21)		// never merge with other surfaces
#define	X_SURF_BLOOD			(1<<22)
#define X_SURF_STONE			(1<<23)


// content masks
#define	MASK_SOLID			(X_CONT_SOLID)
#define	MASK_PLAYERSOLID		(X_CONT_SOLID|X_CONT_PLAYERCLIP|X_CONT_BODY)
#define	MASK_DEADSOLID			(X_CONT_SOLID|X_CONT_PLAYERCLIP)
#define	MASK_MONSTERSOLID		(X_CONT_SOLID|X_CONT_MONSTERCLIP|X_CONT_BODY)
#define	MASK_WATER			(X_CONT_WATER|X_CONT_LAVA|X_CONT_SLIME)
#define	MASK_OPAQUE			(X_CONT_SOLID|X_CONT_SLIME|X_CONT_LAVA)
#define	MASK_SHOT			(X_CONT_SOLID|X_CONT_BODY|X_CONT_CORPSE)


class csurface_c
{
public:
	csurface_c(uint_t flags, uint_t contents)
	{
		_flags		= flags;
		_contents	= contents;
	}
	
	csurface_c(const std::vector<vec3_c> &vertexes, const std::vector<index_t> &indexes, uint_t flags, uint_t contents)
	{
//		Com_DPrintf("creating collisionSurface ...\n");
		
		_vertexes	= vertexes;
		
		_indexes	= indexes;
		reverse(_indexes.begin(), _indexes.end());
		
		_flags		= flags;
		_contents	= contents;
	}
	
	inline const std::vector<vec3_c>&	getVertexes() const			{return _vertexes;}
	inline const std::vector<index_t>&	getIndexes() const			{return _indexes;}
	
	inline uint_t				getFlags() const			{return _flags;}
	inline bool				hasFlags(uint_t flags) const		{return _flags & flags;}
	
	inline uint_t				getContents() const			{return _contents;}
	inline bool				hasContents(uint_t contents) const	{return _contents & contents;}
	
	
private:
	std::vector<vec3_c>	_vertexes;
	std::vector<index_t>	_indexes;
	
	uint_t			_flags;
	uint_t			_contents;
};


class entity_c;

// a trace is returned when a box is swept through the world
struct trace_t
{
	inline trace_t()
	{
		allsolid	= false;
		startsolid	= false;
		fraction	= 0.0;
		surface		= NULL;

		pos_flags	= X_SURF_NONE;
		pos_contents	= X_CONT_NONE;

		neg_flags	= X_SURF_NONE;
		neg_contents	= X_CONT_NONE;

		ent		= NULL;
	}

	bool			allsolid;	// if true, plane is not valid
	bool			startsolid;	// if true, the initial point was in a solid area
	vec_t			fraction;	// 0.0 = start, 1.0 = end
	vec3_c			pos;		// final position
	plane_c			plane;		// surface normal at impact
	csurface_c*		surface;	// surface hit
	
	uint_t			pos_flags;
	uint_t			pos_contents;
	
	uint_t			neg_flags;
	uint_t			neg_contents;	// contents on other side of surface hit
	
	entity_c*		ent;		// entity that was hit, not set by CM_ functions or collision model
};


class cmodel_c
{
public:
	cmodel_c(const std::string &name, byte *buffer, uint_t buffer_size)
	{
		_name = name;
		_buffer = buffer;
		_buffer_size = buffer_size;
	}
	
	virtual ~cmodel_c()				{}
	
	virtual void	load()				{}
	virtual int	leafContents(int leafnum) const	{return X_CONT_NONE;}
	virtual int	leafCluster(int leafnum) const	{return -1;}
	virtual int	leafArea(int leafnum) const	{return -1;}

	virtual const char*	entityString() const	{return "";}

	// returns an ORed contents mask
	virtual int	pointContents(const vec3_c &p) const	{return X_CONT_NONE;}
	virtual int	pointContents(const vec3_c &p, const vec3_c &origin, const quaternion_c &quat) const	{return X_CONT_NONE;}

	virtual int	pointLeafnum(const vec3_c &p) const	{return -1;}
	virtual int	pointAreanum(const vec3_c &p) const	{return -1;}

	virtual trace_t	traceAABB
	(
		const vec3_c &start, const vec3_c &end, const aabb_c &aabb, 
		int mask
	)
	{
		trace_t tr;
		tr.fraction = 1.0;
		tr.pos = end;
		return tr;
	}

	virtual trace_t	traceOBB
	(
		const vec3_c &start, const vec3_c &end, const aabb_c &aabb, 
		int mask,
		const vec3_c &origin, const quaternion_c &quat
	)
	{
		trace_t tr;
		tr.fraction = 1.0;
		tr.pos = end;
		return tr;
	}

	// returns topnode
	virtual int	boxLeafnums(const aabb_c &bbox, std::vector<int> &list, int max) const {return -1;}

	// returns NULL if bad cluster
	virtual const byte*	clusterPVS(int cluster)	const	{return NULL;}

	virtual int	getClosestAreaPortal(const vec3_c &p) const		{return -1;}
	virtual bool	getAreaPortalState(int portal) const			{return true;}
	virtual void	setAreaPortalState(int portal, bool open) const		{}
	virtual bool	areasConnected(int area1, int area2) const		{return true;}
	
	inline const char*	getName() const				{return _name.c_str();}
	inline uint_t		getRegistrationSequence() const		{return _registration_sequence;}
	inline void		setRegistrationSequence(uint_t seq)	{_registration_sequence = seq;}
	
	inline const aabb_c&	getAABB() const				{return _aabb;}
	inline int		getHeadNode() const			{return _headnode;}
	
//	inline const vec3_c*	getVertexes() const			{return &_vertexes[0];}
//	inline const index_t*	getIndexes() const			{return &_indexes[0];}
	
private:
	std::string		_name;
	uint_t			_registration_sequence;
	
protected:
	byte*			_buffer;		// for loading
	uint_t			_buffer_size;
	
public:	
	aabb_c			_aabb;
	int			_headnode;		// FIXME get rid of

//protected:	
	// these are for ODE
	std::vector<vec3_c>		vertexes;
	std::vector<index_t>		indexes;	// note indices have reversed order
							// to make them compatible with ODE
	
	std::vector<csurface_c>		surfaces;
};

class animation_c
{
public:
	animation_c(const std::string &name, int frames, int joints, int components, int framerate)
	{
		_name		= name;
		_num_frames	= frames;
		_num_joints	= joints;
		_frame_rate	= framerate;
	}
	
	virtual ~animation_c()						{}
	
	inline const char*	getName() const				{return _name.c_str();}
	
	inline uint_t		getRegistrationSequence() const		{return _registration_sequence;}
	inline void		setRegistrationSequence(uint_t seq)	{_registration_sequence = seq;}
	
	inline int		getFramesNum() const			{return _num_frames;}
	inline int		getJointsNum() const			{return _num_joints;}
	inline int		getComponentsNum() const		{return _num_components;}
	
	inline int		getFrameRate() const			{return _frame_rate;}

private:
	std::string		_name;
	uint_t			_registration_sequence;

//public:
	int			_num_frames;
	int			_num_joints;	
	int			_num_components;

	int			_frame_rate;
};


enum
{
	SHADERPARM_RED			= 0,
	SHADERPARM_GREEN		= 1,
	SHADERPARM_BLUE			= 2,
	SHADERPARM_ALPHA		= 3,
	SHADERPARM_TIMESCALE		= 4,
	SHADERPARM_TIMEOFFSET		= 4,
	SHADERPARM_DIVERSITY		= 5,
	SHADERPARM_MODE			= 7,
	SHADERPARM_TIME_OF_DEATH	= 7,
	MAX_SHADERPARMS			= 14
};

class shaderparms_iface_a
{
protected:
	inline shaderparms_iface_a()
	{
		clearShaderParms();
	}

public:

	inline void	clearShaderParms()
	{
		shader_parms[SHADERPARM_RED]	= 1.0;
		shader_parms[SHADERPARM_GREEN]	= 1.0;
		shader_parms[SHADERPARM_BLUE]	= 1.0;
		shader_parms[SHADERPARM_ALPHA]	= 1.0;
		shader_parms[4]			= 0.0;
		shader_parms[5]			= 0.0;
		shader_parms[6]			= 0.0;
		shader_parms[7]			= 0.0;
		shader_parms[8]			= 0.0;
		shader_parms[9]			= 0.0;
		shader_parms[10]		= 0.0;
		shader_parms[11]		= 0.0;
		shader_parms[12]		= 0.0;
		shader_parms[13]		= 0.0;
	}
	
	// renderer material shaders want these
	float	shader_parms[MAX_SHADERPARMS];
};


// button bits
enum
{
	BUTTON_ATTACK		= (1<<0),
	BUTTON_ATTACK2		= (1<<1),
	BUTTON_USE		= (1<<2),
	BUTTON_WALK		= (1<<3),
	BUTTON_ANY		= (1<<4)			// any key whatsoever
};


// usercmd_t is sent to the server each client frame
struct usercmd_t
{
	inline usercmd_t()
	{
		msec		= 0;
		buttons		= 0;
		
		forwardmove	= 0;
		rightmove	= 0;
		upmove		= 0;
	}

	inline void clear()
	{
		msec		= 0;
		buttons		= 0;
		
		angles.clear();
		
		forwardmove	= 0;
		rightmove	= 0;
		upmove		= 0;
	}

	int	msec;
	byte	buttons;
	
	vec3_c	angles;
	
	char	forwardmove;
	char	rightmove;
	char	upmove;
};

extern const usercmd_t	null_usercmd;


// entity_state_t->renderfx flags
enum
{
	RF_NONE			= 0,
	RF_VIEWERMODEL		= (1<<0),		// don't draw through eyes, only mirrors
	RF_WEAPONMODEL		= (1<<1),		// only draw through eyes
	RF_FULLBRIGHT		= (1<<2),		// allways draw full intensity
	RF_DEPTHHACK		= (1<<3),		// for view weapon Z crunching
	RF_TRANSLUCENT		= (1<<4),
	RF_NOSHADOW		= (1<<5),		// don't cast shadow
	RF_PORTALSURFACE	= (1<<6),
	RF_STATIC		= (1<<7),
	RF_AUTOANIM		= (1<<8)
};

// player_state_t->refdef flags
enum
{
	RDF_NONE		= 0,
	RDF_NOWORLDMODEL	= (1<<0),		// used for player configuration screen
	RDF_UNDERWATER		= (1<<1),		// warp the screen as apropriate
	RDF_BLOOM		= (1<<2),
	RDF_BLUR		= (1<<3)
};


// sound channels
// channel 0 never willingly overrides
// other channels (1-7) allways override a playing sound on that channel
enum
{
	CHAN_AUTO               = 0,
	CHAN_WEAPON             = (1<<0),
	CHAN_VOICE              = (1<<1),
	CHAN_ITEM               = (1<<3),
	CHAN_BODY               = (1<<4),

	// modifier flags
	CHAN_NO_PHS_ADD		= (1<<5),	// send to all clients, not just ones in PHS (ATTN 0 will also do this)
	CHAN_RELIABLE		= (1<<6)	// send by reliable message, not datagram
};


// sound attenuation values
enum
{
	ATTN_NONE,			// full volume the entire level
	ATTN_NORM,
	ATTN_IDLE,
	ATTN_STATIC			// diminish very rapidly with distance
};


// player_state->stats[] indexes
enum
{	
	STAT_HEALTH_ICON,
	STAT_HEALTH,

	STAT_AMMO_ICON,
	STAT_AMMO,
	
	STAT_ARMOR_ICON,
	STAT_ARMOR,

	STAT_SELECTED_ICON,
	
	STAT_PICKUP_ICON,
	STAT_PICKUP_STRING,
	
	STAT_TIMER_ICON,
	STAT_TIMER,	
	STAT_HELPICON,	
	STAT_SELECTED_ITEM,
	STAT_LAYOUTS,
	
	STAT_FRAGS_ICON,		
	STAT_FRAGS,	
	
	STAT_FLASHES,			// cleared each frame, 1 = health, 2 = armor
	STAT_CHASE,			
	STAT_SPECTATOR,
	
	STAT_HUDBAR, 
	
	STAT_WEAPON_WINDOW0_ICON,
	STAT_WEAPON_WINDOW0_AMMO,
	STAT_WEAPON_WINDOW0_WEAPON,
	
	STAT_WEAPON_WINDOW1_ICON,
	STAT_WEAPON_WINDOW1,
	
	STAT_WEAPON_WINDOW2_ICON,
	STAT_WEAPON_WINDOW2,
	
	STAT_WEAPON_WINDOW3_ICON,
	STAT_WEAPON_WINDOW3,
	
	STAT_WEAPON_WINDOW4_ICON,
	STAT_WEAPON_WINDOW4,
	
	STAT_WEAPON_WINDOW5_ICON,
	STAT_WEAPON_WINDOW5,
	
	STAT_SELECTED_WEAPON_ICON,
	STAT_SELECTED_WEAPON,
	
	MAX_STATS
};

/*
================================================================================
			NETCODE ELEMENTS
================================================================================
*/

// config strings are a general means of communication from
// the server to all connected clients.
// Each config string can be at most MAX_QPATH characters.
//
enum
{
	CS_MAPCHECKSUM		= 0,		// for catching cheater maps
	CS_MAPNAME,
	CS_MAPMESSAGE,
	CS_MAPMUSIC,
	CS_STATUSBAR,				// display program string
	CS_MAXCLIENTS		= 30,
	CS_MODELS		= 31,
	CS_SHADERS		= CS_MODELS+MAX_MODELS,
	CS_ANIMATIONS		= CS_SHADERS+MAX_SHADERS,
	CS_SOUNDS		= CS_ANIMATIONS+MAX_ANIMATIONS,
	CS_LIGHTS		= CS_SOUNDS+MAX_SOUNDS,
	CS_PLAYERSKINS		= CS_LIGHTS+MAX_LIGHTS,
	CS_GENERAL		= CS_PLAYERSKINS+MAX_CLIENTS,
	MAX_CONFIGSTRINGS	= CS_GENERAL+MAX_GENERAL
};

enum solid_e
{
	SOLID_NOT,			// no interaction with other objects
	SOLID_TRIGGER,			// only touch when inside, after moving
	SOLID_BBOX,			// touch on edge
	SOLID_BSP			// bsp clip, touch on edge
};


//class message_c;

// entity_state_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
class entity_state_t :
public shaderparms_iface_a
{
	friend class g_entity_c;
	friend class message_c;
	friend class bitmessage_c;

public:	
	inline entity_state_t()
	{
		type		= 0;
		
		number		= 0;
		
		index_model	= 0;
		index_shader	= 0;
		index_animation	= 0;
		index_sound	= 0;
		index_light	= 0;
		
		frame		= 0;
		effects		= 0;
		renderfx	= RF_NONE;
				
		event		= 0;
	}
	
	inline void	clear()
	{
		type		= 0;
		
		number		= 0;
		
		origin.clear();
		origin2.clear();
		
		quat.identity();
		quat2.identity();
		
		velocity_linear.clear();
		velocity_angular.clear();
		
		index_model	= 0;
		index_shader	= 0;
		index_animation	= 0;
		index_sound	= 0;
		index_light	= 0;
		
		frame		= 0;
		effects		= 0;
		renderfx	= RF_NONE;
				
		event		= 0;
		
		vectors[0].clear();
		vectors[1].clear();
		vectors[2].clear();
	}
	
	inline int	getNumber() const	{return number;}


	int		type;

private:
	int		number;			// entity index only set by g_entity_c::ctor

public:

	vec3_c		origin;
	vec3_c		origin2;
	
	quaternion_c	quat;
	quaternion_c	quat2;
	
	vec3_c		velocity_linear;
	vec3_c		velocity_angular;
	
	int		index_model;
	int		index_shader;
	int		index_animation;
	int		index_sound;		// for looping sounds, to guarantee shutoff
	int		index_light;
	
	int		frame;
	uint_t		effects;		// PGM - we're filling it, so it needs to be unsigned
	uint_t		renderfx;
	
	byte		event;			// impulse events -- muzzle flashes, footsteps, etc
							// events only go out for a single frame, they
							// are automatically cleared each frame
	
	vec3_c		vectors[3];		// misc vectors. e.g. used by lights
};

extern const entity_state_t	null_entity_state;



// player_state_t is the information needed for client side movement
// prediction
// this structure needs to be communicated bit-accurate
// from the server to the client to guarantee that prediction stays in sync, 
// if any part of the game code modifies this struct, it
// will result in a prediction error of some degree.

// you can't add anything to this without modifying the code in msg.c

// player_state_t is a full superset of entityState_t as it is used by players,
// so if a player_state_t is transmitted, the entity_state_t can be fully derived
// from it.
class player_state_t // : public entity state
{
public:
	inline player_state_t()
	{
		clear();
	}

	inline void	clearPMove()
	{
		pm_type			= 0;
		
		origin.clear();
		velocity_linear.clear();
		velocity_angular.clear();
		
		pm_flags		= 0;
		pm_time			= 0;
		
		gravity			= 0;

		speed_cmd		= 320;
		speed_stop		= 100;
		speed_max		= 400;

		accelerate		= 10;
		accelerate_water	= 5;
		accelerate_air		= 1;
		accelerate_spectator	= 8;

		friction		= 6;
		friction_water		= 1;
		friction_air		= 3;
		friction_spectator	= 5;
		
		delta_angles.clear();
	}

	inline void	clear()
	{
		clearPMove();
		
		view_angles.clear();
		view_offset.clear();
		
		kick_angles.clear();
		
		gun_angles.clear();
		gun_offset.clear();
		gun_model_index	= 0;
		gun_anim_frame	= 0;
		gun_anim_index	= 0;
		
		blend.clear();
		
		fov		= 0;
		
		rdflags		= 0;
		
		memset(stats, 0, sizeof(stats));
	}

	int		pm_type;

	vec3_c		origin;
	vec3_c		velocity_linear;
	vec3_c		velocity_angular;
	
	byte		pm_flags;			// ducked, jump_held, etc
	byte		pm_time;			// each unit = 8 ms
	
	int		gravity;
	
	int		speed_cmd;			// for cmd scale
	int		speed_stop;
	int		speed_max;

	int		accelerate;
	int		accelerate_water;
	int		accelerate_air;
	int		accelerate_spectator;

	int		friction;
	int		friction_water;
	int		friction_air;
	int		friction_spectator;
	
	vec3_c		delta_angles;			// add to command angles to get view direction
							// changed by spawns, rotating objects, and teleporters

	// these fields do not need to be communicated bit-precise
	vec3_c		view_angles;			// for fixed views
	vec3_c		view_offset;			// add to pmovestate->origin
	
	vec3_c		kick_angles;			// add to view direction to get render angles
							// set by weapon kicks, pain effects, etc

	vec3_c		gun_angles;
	vec3_c		gun_offset;
	int		gun_model_index;
	int		gun_anim_frame;
	int		gun_anim_index;

	vec4_c		blend;				// rgba full screen effect
	
	float		fov;				// horizontal field of view

	int		rdflags;			// refdef flags

	int		stats[MAX_STATS];		// fast status bar updates
//	int		persistant[MAX_PERSISTANT];	// stats that aren't cleared on death
//	int		powerups[MAX_POWERUPS];		// level.time that the powerup runs out
//	int		ammo[MAX_WEAPONS];
};

extern const player_state_t	null_player_state;


class entity_c
{
public:
	entity_state_t		_s;	// <-- used in client too
};



enum netadr_type_e
{
	NA_BROADCAST,
	NA_IP, 
	NA_IP6
};

struct netadr_t
{
	netadr_type_e	type;

	byte	ip[4];
	byte	ip6[6];

	unsigned short	port;
};

struct viddef_t
{
	unsigned		width, height;			// coordinates from main game
};

enum keydest_t
{
	KEY_GAME,
	KEY_CONSOLE,
	KEY_MESSAGE,
	KEY_MENU
};

#endif	// X_SHARED_H

