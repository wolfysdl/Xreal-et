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
#include "cl_local.h"
#include "s_local.h"

#include "vfs.h"
#include "cm.h"

// OpenAL on Windows is different (Apple should be as well)
#if defined(_WIN32) || defined( __APPLE__ )
#define AL_ILLEGAL_ENUM AL_INVALID_ENUM
#define AL_ILLEGAL_COMMAND AL_INVALID_OPERATION
#endif


static alconfig_t  al_config;

static ALCcontext*	s_context_id = NULL;
static ALCdevice*	s_dev = NULL;

// OpenAL extension functions
static ALboolean (*s_alutLoadVorbis_LOKI)(ALuint bid, ALvoid *data, ALint size);
static ALboolean (*s_alutLoadMP3_LOKI)(ALuint bid, ALvoid *data, ALint size);


void	S_CheckForError_(const char *filename, int line)
{
	int err = alGetError();
	char* errstr;
	
	if(err != AL_NO_ERROR)
	{
		switch(err)
		{
			case AL_INVALID_NAME:
				errstr = "AL_INVALID_NAME";
				break;
			
			case AL_ILLEGAL_ENUM:
				errstr = "AL_ILLEGAL_ENUM";
				break;
			
			case AL_INVALID_VALUE:
				errstr = "AL_INVALID_VALUE";
				break;
			
			case AL_ILLEGAL_COMMAND:
				errstr = "AL_ILLEGAL_COMMAND";
				break;
			
			case AL_OUT_OF_MEMORY:
				errstr = "AL_OUT_OF_MEMORY";
				break;
			
			default:
				errstr = "unknown error";
		}
		
		Com_Error(ERR_DROP, "S_CheckForError: %s in file %s, line %i", errstr, filename, line);
	}
}

static void	S_CheckOpenALExtensions()
{
	al_config.ext_vorbis = false;
	al_config.ext_mp3 = false;
	
	if(strstr(al_config.extensions_string, "AL_EXT_vorbis"))
	{
		if(s_ext_vorbis->getInteger())
		{
			Com_Printf("...using AL_EXT_vorbis\n");
			al_config.ext_vorbis = true;
			s_alutLoadVorbis_LOKI = (ALboolean (*)(ALuint bid, ALvoid *data, ALint size))alGetProcAddress((ALubyte*) "alutLoadVorbis_LOKI");
		}
		else
		{
			Com_Printf("...ignoring AL_EXT_vorbis\n");
		}
	}
	else
	{
		Com_Printf("...AL_EXT_vorbis not found\n");
	}
	
	if(strstr(al_config.extensions_string, "AL_EXT_MP3"))
	{
		if(s_ext_mp3->getInteger())
		{
			Com_Printf("...using AL_EXT_MP3\n");
			al_config.ext_mp3 = true;
			s_alutLoadMP3_LOKI = (ALboolean (*)(ALuint bid, ALvoid *data, ALint size))alGetProcAddress((ALubyte*) "alutLoadMP3_LOKI");
		}
		else
		{
			Com_Printf("...ignoring AL_EXT_MP3\n");
		}
	}
	else
	{
		Com_Printf("...AL_EXT_MP3 not found\n");
	}
}




void	S_InitOpenAL()
{
	//TODO make use of s_kHz

	int attrlist[] = { ALC_FREQUENCY, 44100, 0 };

#ifdef __linux__
	if(X_strcaseequal(s_backend->getString(), "alsa"))
	{
		Com_Printf("trying to open ALSA device ...\n");
		
		s_dev = alcOpenDevice((const ALubyte*)"'( ( devices '( alsa ) ) )");
	}
	else if(X_strcaseequal(s_backend->getString(), "arts"))
	{
		Com_Printf("trying to open ARTS device ...\n");
		
		s_dev = alcOpenDevice((const ALubyte*)"'( ( devices '( arts ) ) )");
	}
	else
	{
		Com_Printf("trying to open OSS device ...\n");
		
		s_dev = alcOpenDevice((const ALubyte*)"'( ( devices '( native ) ) )");
	}
	
#elif _WIN32
	s_dev = alcOpenDevice(NULL);
#endif
	if(s_dev == NULL)
	{
		Com_Error(ERR_FATAL, "S_InitOpenAL: alcOpenDevice failed");
	}

	s_context_id = alcCreateContext(s_dev, attrlist);
	if(s_context_id == NULL)
	{
		alcCloseDevice(s_dev);
		
		Com_Error(ERR_FATAL, "S_InitOpenAL: alcCreateContext failed");
	}
	
	alcMakeContextCurrent(s_context_id);

	al_config.vendor_string = (const char*)alGetString(AL_VENDOR);
	Com_Printf("AL_VENDOR: %s\n", al_config.vendor_string);
	
	al_config.renderer_string = (const char*)alGetString(AL_RENDERER);
	Com_Printf("AL_RENDERER: %s\n", al_config.renderer_string);
	
	al_config.version_string = (const char*)alGetString(AL_VERSION);
	Com_Printf("AL_VERSION: %s\n", al_config.version_string);
	
	al_config.extensions_string = (const char*)alGetString(AL_EXTENSIONS);	
	Com_Printf("AL_EXTENSIONS: %s\n", al_config.extensions_string);
	
	S_CheckOpenALExtensions();
	
	S_CheckForError();
	
	s_initialized = true;
}

void	S_ShutdownOpenAL()
{
	alcDestroyContext(s_context_id);
	
	alcCloseDevice(s_dev);
}



static s_buffer_c*	S_LoadWAV(const std::string &name)
{
	ALbyte*		data = NULL;
	
	ALsizei		size;
	ALsizei 	bits = 0;
	ALsizei 	freq;
	ALenum 		format;
	ALboolean	loop;
	ALuint		buffer;

	ALvoid*		wave = NULL;
	
	
	VFS_FLoad(name, (void **)&data);
	if(!data)
	{
		return NULL;
	}
	
	alGenBuffers(1, &buffer);
	
	alutLoadWAVMemory(data, &format, &wave, &size, &freq, &loop);
	if(!size)
	{
		Com_DPrintf("S_LoadWAV: couldn't load '%s' because alutLoadWAVMemory failed\n", name.c_str());
		return NULL;
	}
	
	VFS_FFree(data);
	
	alBufferData(buffer, format, wave, size, freq);
	
	S_CheckForError();

	return (new s_buffer_c(name, buffer, size, format, bits, freq, loop));
}

static s_buffer_c*	S_LoadOGG(const std::string &name)
{
	ALbyte*		data = NULL;
	
	ALsizei		size;
	ALuint		buffer;
		
	
	if(!al_config.ext_vorbis || !s_alutLoadVorbis_LOKI)
		return NULL;
	

	size = VFS_FLoad(name, (void **)&data) -1;
	if(!data)
	{
		return NULL;
	}
	
	alGenBuffers(1, &buffer);
	
	if(!(s_alutLoadVorbis_LOKI(buffer, data, size)))
	{
		Com_DPrintf("S_LoadOGG: couldn't load '%s' because s_alutLoadVorbis_LOKI failed\n", name.c_str());
		return NULL;
	}
	
	VFS_FFree(data);
	
	S_CheckForError();
	
	return (new s_buffer_c(name, buffer, size, 0, 0, 0, 0));
}

static s_buffer_c*	S_LoadMP3(const std::string &name)
{
	ALbyte*		data = NULL;
	
	ALsizei		size;
	ALuint		buffer;
		
	
	if(!al_config.ext_mp3 || !s_alutLoadMP3_LOKI)
		return NULL;
	

	size = VFS_FLoad(name, (void**)&data) -1;
	if(!data)
	{
		return NULL;
	}
	
	alGenBuffers(1, &buffer);
	
	if(!(s_alutLoadMP3_LOKI(buffer, data, size)))
	{
		Com_DPrintf("S_LoadMP3: couldn't load '%s' because s_alutLoadMP3_LOKI failed\n", name.c_str());
		return NULL;
	}
	
	VFS_FFree(data);
	
	S_CheckForError();
	
	return (new s_buffer_c(name, buffer, size, 0, 0, 0, 0));
}

s_buffer_c*	S_LoadBuffer(const std::string &name)
{
	s_buffer_c*	sfx = NULL;
	
	if(name[0] == '*')
		return NULL;
		
	Com_DPrintf("loading '%s' ...\n", name.c_str());

	
	// load it in
	//if (s->truename)
	//	name = s->truename;
	//else
	//	name = (char*)s->name.c_str();
	
	
	if((sfx = S_LoadWAV(name + ".wav")))
	{
		return sfx;
	}
	else if((sfx = S_LoadOGG(name + ".ogg")))
	{
		return sfx;
	}
	else if((sfx = S_LoadMP3(name + ".mp3")))
	{
		return sfx;
	}
	else
	{
		Com_Printf("S_LoadSound: couldn't handle '%s'\n", name.c_str());
		return NULL;
	}	
}



/*
static s_buffer_c*	S_AliasName(char *aliasname, char *truename)
{
	s_buffer_c	*sfx;
	char	*s;
	int		i;

	s = (char*)Com_Malloc (MAX_QPATH);
	strcpy (s, truename);

	// find a free sfx
	for (i=0 ; i < num_sfx ; i++)
		if (!known_sfx[i].name[0])
			break;

	if (i == num_sfx)
	{
		if (num_sfx == MAX_SFX)
			Com_Error (ERR_FATAL, "S_FindName: out of s_buffer_c");
		num_sfx++;
	}
	
	sfx = &known_sfx[i];
	memset (sfx, 0, sizeof(*sfx));
	strcpy (sfx->name, aliasname);
	sfx->registration_sequence = s_registration_sequence;
	sfx->truename = s;

	return sfx;
}
*/




#if 0
static s_source_c*	S_PickSoundSource(int entnum, int entchannel, s_buffer_c *buffer, bool autosound)
{
	if(entchannel < 0)
		Com_Error(ERR_DROP, "S_PickChannel: entchannel < 0");
		
		
	//Com_Printf("S_PickChannel: %i %i\n", entnum, entchannel);
	
	for(std::vector<s_source_c*>::iterator ir = s_sources.begin(); ir != s_sources.end(); ir++)
	{
		s_source_c* source = *ir;
		
		
		if(entchannel != CHAN_AUTO && source->entnum == entnum && source->entchannel == entchannel)
		{
			// always override sound from same entity
			if(source->isPlaying())
			{
				alSourceStop(source->getId());
				source->clear();
				return source;
			}
		}
		
		/*
		if(	source->entnum == entnum &&
			source->entchannel == entchannel &&
			source->autosound && autosound	)
		{
			// override only when it's a different autosound
			if(!X_strcaseequal(s_sources[ch_idx]->buffer->getName(), buffer->getName()))
			{
				if(ch->isPlaying())
				{
					alSourceStop(ch->getId());
				}
			
				first_to_die = ch_idx;
				break;
			}
			else
			{
				return NULL;	
			}
		}
		*/
	

		// don't let monster sounds override player sounds
		//if(s_sources[ch_idx].entnum == cl.playernum+1 && entnum != cl.playernum+1 && s_sources[ch_idx].buffer)
		//	continue;
		
		if(source->isPlaying())
		{
			alSourceStop(source->getId());
			source->clear();
			return source;
		}
	}

	return NULL;
}
#endif

 
         



/*
struct buffer_s*	S_RegisterSexedSound(entity_state_c *ent, char *base)
{
	int				n;
	char			*p;
	struct buffer_s	*buffer;
	char			model[MAX_QPATH];
	char			sexedFilename[MAX_QPATH];
	char			maleFilename[MAX_QPATH];
	
	VFILE*			f = NULL;

	// determine what model the client is using
	model[0] = 0;
	n = CS_PLAYERSKINS + ent->number - 1;
	if (cl.configstrings[n][0])
	{
		p = strchr(cl.configstrings[n], '\\');
		if (p)
		{
			p += 1;
			strcpy(model, p);
			p = strchr(model, '/');
			if (p)
				*p = 0;
		}
	}
	// if we can't figure it out, they're male
	if (!model[0])
		strcpy(model, "male");

	// see if we already know of the model specific sound
	Com_sprintf (sexedFilename, sizeof(sexedFilename), "#players/%s/%s", model, base+1);
	buffer = S_FindName (sexedFilename, false);

	if (!buffer)
	{
		// no, so see if it exists
		VFS_FOpenRead (&sexedFilename[1], &f);
		if (f)
		{
			// yes, close the file and register it
			VFS_FClose (f);
			buffer = S_RegisterSound (sexedFilename);
		}
		else
		{
			// no, revert to the male sound in the pak0.pak
			Com_sprintf (maleFilename, sizeof(maleFilename), "sound/player/%s/%s", "male", base+1);
			buffer = S_AliasName (sexedFilename, maleFilename);
		}
	}

	return buffer;
}
*/


void	S_StartShader(int sound)
{
	if(!s_initialized)
		return;
		
	if(sound == -1)
		return;

	
}

/*
====================
S_StartSound

Validates the parms and ques the sound up
if pos is NULL, the sound will be dynamically sourced from the entity
Entchannel 0 will never override a playing sound
====================
*/
void	S_StartSound(const vec3_c &origin, int sound)
{
	if(!s_initialized)
		return;

	if(sound == -1)
		return;
	
	s_shader_c *shader = S_GetShaderByNum(sound);
	if(!shader)
	{
		Com_Error(ERR_DROP, "S_StartSound: bad sound number %i", sound);
		return;
	}
	
	shader->createSource(origin, vec3_origin, 0, false);
}


void	S_StartLocalSound(const std::string &name)
{
	if(!s_initialized)
		return;
		
	int sound = S_RegisterSound(name);
	if(sound == -1)
	{
		Com_Printf("S_StartLocalSound: can't cache %s\n", name.c_str());
		return;
	}
	
	S_StartSound(s_origin, sound);
}


void	S_StartLoopSound(const vec3_c &origin, const vec3_c &velocity, int entity, int sound)
{
	if(!s_initialized)
		return;

	if(sound == -1)
		return;
		
//	Com_DPrintf("S_StartLoopSound: entity %i", entity_num);

	s_shader_c *shader = S_GetShaderByNum(sound);
	if(!shader)
	{
		Com_Error(ERR_DROP, "S_StartLoopSound: bad sound number %i", sound);
		return;
	}
	
	for(std::vector<s_source_c*>::const_iterator ir = s_sources.begin(); ir != s_sources.end(); ++ir)
	{
		s_source_c *source = *ir;
		
		if(!source)
			continue;
	
		if(source->getEntityNum() == entity && source->isLoopSound())
			return;
	}
	
	shader->createSource(origin, velocity, entity, true);
}

void	S_UpdateLoopSound(const vec3_c &origin, const vec3_c &velocity, int entity, int sound)
{
	for(std::vector<s_source_c*>::iterator ir = s_sources.begin(); ir != s_sources.end(); ++ir)
	{
		s_source_c *source = *ir;
		
		if(!source)
			continue;
	
		if(source->getEntityNum() == entity)
		{
			(*ir)->setPosition(origin);
			(*ir)->setVelocity(velocity);
			return;
		}
	}
	
	if(sound == -1)
		return;
	
	s_shader_c *shader = S_GetShaderByNum(sound);
	if(!shader)
	{
		Com_Error(ERR_DROP, "S_StartLoopSound: bad sound number %i", sound);
		return;
	}
	
	shader->createSource(origin, velocity, entity, true);
	
//	Com_DPrintf("S_UpdateLoopSound: couldn't find sound source with entity %i\n", entity_num);
}

void	S_StopLoopSound(int entity)
{
	for(std::vector<s_source_c*>::iterator ir = s_sources.begin(); ir != s_sources.end(); ++ir)
	{
		s_source_c *source = *ir;
		
		if(!source)
			continue;
	
		if(source->getEntityNum() == entity && source->isLoopSound())
		{
			source->stop();
		
			delete source;
			*ir = NULL;
		}
	}
	
//	Com_DPrintf("S_StopLoopSound: couldn't find sound source with entity %i\n", entity_num);
}

/*
static bool 	S_InPVS(const vec3_c &p1, const vec3_c &p2)
{
	//int		leafnum;
	//int		cluster;
	int		area1, area2;
	//byte	*mask;

	//leafnum = CM_PointLeafnum(p1);
	//cluster = CM_LeafCluster(leafnum);
	//area1 = CM_LeafArea(leafnum);
	//mask = CM_ClusterPVS(cluster);
	area1 = CM_PointAreanum(p1);

	//leafnum = CM_PointLeafnum(p2);
	//cluster = CM_LeafCluster(leafnum);
	//area2 = CM_LeafArea(leafnum);
	area2 = CM_PointAreanum(p2);
	
	//if(mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
	//	return false;
	
	if(!CM_AreasConnected(area1, area2))
		return false;		// a door blocks sight
	
	return true;
}
*/

/*
============
S_Update

Called once each time through the main loop
============
*/
void	S_Update(const vec3_c &origin, const vec3_c &velocity, const vec3_c &v_forward, const vec3_c &v_right, const vec3_c &v_up)
{
	if(!s_initialized)
		return;

	/*
	if(cls.disable_screen)
	{
		//S_ClearBuffer();
		return;
	}
	*/

	// rebuild scale tables if volume is modified
	if(s_sfxvolume->isModified())
		alListenerf(AL_GAIN, s_sfxvolume->getValue());
		
	S_CheckForError();
	
	// setup listener origin
	s_origin = origin;
	
	// convert listener origin to openal format
	vec3_c al_listener_origin;
	al_listener_origin[0] =  s_origin[1];
	al_listener_origin[1] =  s_origin[2];
	al_listener_origin[2] = -s_origin[0];
	al_listener_origin.scale(1.0/32.0);
		
	alListenerfv(AL_POSITION, al_listener_origin);		S_CheckForError();
	
	
	// setup listener velocity
	vec3_c al_listener_velocity;
	al_listener_velocity[0] =  s_velocity[1];
	al_listener_velocity[1] =  s_velocity[2];
	al_listener_velocity[2] = -s_velocity[0];
	al_listener_velocity.scale(1.0/32.0);
		
	alListenerfv(AL_VELOCITY, al_listener_velocity);	S_CheckForError();
	
	
	// setup listener orientation
	s_forward = v_forward;
	s_right = v_right;
	s_up = v_up;
	
	float al_listener_orientation[6];
	al_listener_orientation[0] =  s_forward[1];
	al_listener_orientation[1] = -s_forward[2];
	al_listener_orientation[2] = -s_forward[0];
	
	al_listener_orientation[3] =  s_up[1];
	al_listener_orientation[4] = -s_up[2];
	al_listener_orientation[5] = -s_up[0];
	
	alListenerfv(AL_ORIENTATION, al_listener_orientation);	S_CheckForError();
	
//	alDistanceModel(AL_NONE);
//	alDistanceModel(AL_INVERSE_DISTANCE);
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	
	S_CheckForError();
	

//	alDopplerFactor(1.0);		// don't exaggerate doppler shift
//	alDopplerVelocity(343.0);	// using meters/second
	
	/*
	if(s_show->getInteger())
	{
		Com_Printf("listener at %s\n", s_origin.toString());
		Com_Printf("listener vf %s\n", s_forward.toString());
		Com_Printf("listener vu %s\n", s_up.toString());
	}
	*/
	
	// kill stopped sound sources
	for(std::vector<s_source_c*>::iterator ir = s_sources.begin(); ir != s_sources.end(); ++ir)
	{
		s_source_c *source = *ir;
		
		if(!source)
			continue;
	
		if(!source->hasBuffer())
			continue;
		
		if(source->isStopped())// && source->isActivated())
		{
			delete source;
			*ir = NULL;
		}
	}
	
	S_CheckForError();


	// paint in the channels
	cmodel_c* cworld = CM_GetModelByNum(0);
	const byte *pvs = NULL;
	if(cworld && cls.state == CA_ACTIVE)
	{
		int leafnum = cworld->pointLeafnum(origin);
		int cluster = cworld->leafCluster(leafnum);
		pvs = cworld->clusterPVS(cluster);
	}
	
	for(std::vector<s_source_c*>::iterator ir = s_sources.begin(); ir != s_sources.end(); ++ir)
	{
		s_source_c *source = *ir;
		
		if(!source)
			continue;
		
		if(!source->hasBuffer())
			continue;
			
		if(pvs)
		{
			if(source->getCluster() >= 0)
			{	
				if(pvs[source->getCluster() >> 3] & (1 << (source->getCluster() & 7)))
				{
					// source is in PVS
					if(source->isPlaying())
						continue;
					
					// if not playing yet start it now
					source->play();
				}
				else
				{
					// source is not in the PVS
					if(source->isPlaying())
					{
						if(source->isLoopSound())
							source->pause();
					}
					else if(!source->isLoopSound())
					{
						// kill source
						delete source;
						*ir = NULL;
					}
				}
			}
			else
			{
				if(source->isPlaying())
					continue;
			
				source->play();
			}
		}
		else
		{
			// only listen to non-looping sounds
			if(source->isPlaying())
			{
				if(source->isLoopSound())
					source->pause();
			}
		}
	}
	
	S_CheckForError();
}


