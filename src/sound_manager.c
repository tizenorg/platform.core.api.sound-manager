/*
* Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License. 
*/


#define LOG_TAG "TIZEN_N_SOUND_MANGER"

#include <sound_manager.h>
#include <mm_sound.h>
#include <mm_sound_private.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <dlog.h>
#include <mm_session.h>

#define MAX_VOLUME_TYPE 5

typedef struct {
	void *user_data;
	sound_manager_volume_changed_cb user_cb;
}_changed_volume_info_s;

typedef struct {
	void *user_data;
	sound_manager_route_policy_changed_cb user_cb;
}_changed_policy_info_s;

typedef struct {
	int is_registered;
	void *user_data;
	sound_session_notify_cb user_cb;
}_session_notify_info_s;

static _changed_volume_info_s g_volume_changed_cb_table;
static _changed_policy_info_s g_policy_changed_cb_table;
static _session_notify_info_s g_session_notify_cb_table = {0, NULL, NULL };

static void _volume_changed_cb(void *user_data)
{
	sound_type_e  type = (sound_type_e)user_data;
	
	int new_volume;
	sound_manager_get_volume(	type, &new_volume);
	if( g_volume_changed_cb_table.user_cb )
		(g_volume_changed_cb_table.user_cb)(type, new_volume, g_volume_changed_cb_table.user_data);
}

static void _policy_changed_cb(void *user_data,  system_audio_route_t policy){
	if( g_policy_changed_cb_table.user_cb)
		g_policy_changed_cb_table.user_cb(policy, g_policy_changed_cb_table.user_data);
}

static void _session_notify_cb(session_msg_t msg, void *user_data){
	if( g_session_notify_cb_table.user_cb ){
		g_session_notify_cb_table.user_cb(msg, g_session_notify_cb_table.user_data);
	}
}


int _convert_sound_manager_error_code(const char *func, int code){
	int ret = SOUND_MANAGER_ERROR_NONE;
	char *errorstr = NULL;
	
	switch(code)
	{
		case SOUND_MANAGER_ERROR_INVALID_PARAMETER:
			ret = SOUND_MANAGER_ERROR_INVALID_PARAMETER;
			errorstr = "INVALID_PARAMETER";
			break;
		case MM_ERROR_NONE:
			ret = SOUND_MANAGER_ERROR_NONE;
			errorstr = "ERROR_NONE";
			break;
		case MM_ERROR_INVALID_ARGUMENT: 
		case MM_ERROR_SOUND_INVALID_POINTER:
			ret = SOUND_MANAGER_ERROR_INVALID_PARAMETER;
			errorstr = "INVALID_PARAMETER";
			break;
		case MM_ERROR_SOUND_INTERNAL:
			ret = SOUND_MANAGER_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION"	;		
			break;
		case MM_ERROR_SOUND_VOLUME_NO_INSTANCE:
		case MM_ERROR_SOUND_VOLUME_CAPTURE_ONLY:
			ret = SOUND_MANAGER_ERROR_NO_PLAYING_SOUND;
			errorstr = "NO_PLAYING_SOUND"	;		
			break;
	}	
	LOGE( "[%s] %s(0x%08x) : core frameworks error code(0x%08x)",func, errorstr, ret, code);
	return ret;
}




int sound_manager_get_max_volume(sound_type_e type, int *max)
{
	int volume;
	if( max == NULL )
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	
	if( type > MAX_VOLUME_TYPE || type < 0 )
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret = mm_sound_volume_get_step(type, &volume);

	if( ret == 0 )
		*max = volume -1;	// actual volume step can be max step - 1
	
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume(sound_type_e type, int volume)
{
	if( type > MAX_VOLUME_TYPE || type < 0 )
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	if( volume < 0 )
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);

	int ret = mm_sound_volume_set_value(type, volume);
	
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_volume(sound_type_e type, int *volume)
{
	unsigned int uvolume;
	if( type > MAX_VOLUME_TYPE || type < 0 )
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	if( volume == NULL)
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret = mm_sound_volume_get_value(type, &uvolume);

	if( ret == 0)
		*volume = uvolume;
		
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_route_policy (sound_route_policy_e route)
{
	if( route < 0 || route > SOUND_ROUTE_HANDSET_ONLY)
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);

	int ret;
	ret = mm_sound_route_set_system_policy(route);
	
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_route_policy (sound_route_policy_e *route)
{
	if( route == NULL)
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret;
	ret= mm_sound_route_get_system_policy((system_audio_route_t *)route);
	
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_current_sound_type(sound_type_e *type)
{
	if( type == NULL)
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret;
	ret = mm_sound_volume_get_current_playing_type((volume_type_t *)type);
	
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_current_sound_device(sound_device_e *device)
{
	if( device == NULL)
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret;
	ret = mm_sound_route_get_playing_device((system_audio_route_device_t*)device);
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume_changed_cb(sound_manager_volume_changed_cb callback, void* user_data)
{
	if( callback == NULL )
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int i;
	g_volume_changed_cb_table.user_cb = callback;
	g_volume_changed_cb_table.user_data = user_data;
	for(i = 0 ; i <= MAX_VOLUME_TYPE ; i++)
	{
		mm_sound_volume_add_callback( i , _volume_changed_cb ,(void*) i);
	}
	return 0;
}

void sound_manager_unset_volume_changed_cb(void)
{
	int i;
	for(i = 0 ; i <= MAX_VOLUME_TYPE ; i++)
	{
		mm_sound_volume_remove_callback( i );
	}
	g_volume_changed_cb_table.user_cb = NULL;
	g_volume_changed_cb_table.user_data = NULL;	
}

int sound_manager_set_route_policy_changed_cb(sound_manager_route_policy_changed_cb callback, void* user_data)
{
	if( callback == NULL)
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	g_policy_changed_cb_table.user_cb = callback;
	g_policy_changed_cb_table.user_data = user_data;	
	mm_sound_route_add_change_callback(_policy_changed_cb, user_data);
	return 0;
}

void sound_manager_unset_route_policy_changed_cb(void)
{
	mm_sound_route_remove_change_callback();
	g_policy_changed_cb_table.user_cb = NULL;
	g_policy_changed_cb_table.user_data = NULL;		
}

int sound_manager_get_a2dp_status(bool *connected , char** bt_name){
	int ret;
	int connect;
	ret = mm_sound_route_get_a2dp_status(&connect , bt_name );
	if( ret == 0 )
		*connected = connect;
	return _convert_sound_manager_error_code(__func__, ret);
}



int sound_manager_set_session_type(sound_session_type_e type){
	int ret = 0;
	if( type < 0 || type >  SOUND_SESSION_TYPE_EXCLUSIVE )
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
		
	if( g_session_notify_cb_table.is_registered ){
		mm_session_finish();
		g_session_notify_cb_table.is_registered = 0;
	}

	ret = mm_session_init_ex( type , _session_notify_cb, NULL);
	if( ret == 0 ){
		g_session_notify_cb_table.is_registered = 1;
	}
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_session_notify_cb(sound_session_notify_cb callback , void *user_data){
	int ret =0 ;
	if( callback == NULL)
		return _convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	
	
	if( g_session_notify_cb_table.is_registered ==0 ){
		ret = mm_session_init_ex( SOUND_SESSION_TYPE_SHARE /*default*/ , _session_notify_cb, NULL);
		if( ret != 0 )
			return _convert_sound_manager_error_code(__func__, ret);
		g_session_notify_cb_table.is_registered = 1;
	}

	g_session_notify_cb_table.user_cb = callback ; 
	g_session_notify_cb_table.user_data  = user_data;
	return SOUND_MANAGER_ERROR_NONE;
}

void sound_manager_unset_session_notify_cb(void){
	g_session_notify_cb_table.user_cb = NULL; 
	g_session_notify_cb_table.user_data  = NULL;
}

