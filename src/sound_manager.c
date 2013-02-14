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
#include <malloc.h>
#include <unistd.h>
#include <dlog.h>
#include <mm_session.h>
#include <mm_session_private.h>

#define MAX_VOLUME_TYPE 5

typedef struct {
	void *user_data;
	sound_manager_volume_changed_cb user_cb;
}_changed_volume_info_s;

typedef struct {
	int is_registered;
	void *user_data;
	sound_session_notify_cb user_cb;
	void *interrupted_user_data;
	sound_interrupted_cb interrupted_cb;
}_session_notify_info_s;

typedef struct {
	void *user_data;
	sound_available_route_changed_cb user_cb;
}_changed_available_route_info_s;

typedef struct {
	void *user_data;
	sound_active_device_changed_cb user_cb;
}_changed_active_device_info_s;

static _changed_volume_info_s g_volume_changed_cb_table;
static _session_notify_info_s g_session_notify_cb_table = {0, NULL, NULL, NULL, NULL};
static sound_session_type_e g_cached_session = -1;

#define SOUND_SESSION_TYPE_DEFAULT SOUND_SESSION_TYPE_SHARE

static void __volume_changed_cb(void *user_data)
{
	int ret = SOUND_MANAGER_ERROR_NONE;
	sound_type_e type = (sound_type_e)user_data;

	int new_volume;
	ret = sound_manager_get_volume(type, &new_volume);
	if (ret == SOUND_MANAGER_ERROR_NONE) {
		if(g_volume_changed_cb_table.user_cb)
			(g_volume_changed_cb_table.user_cb)(type, new_volume, g_volume_changed_cb_table.user_data);
	} else {
		LOGE("sound_manager_get_volume() error code(0x%08x)", ret);
	}
}

static void __session_notify_cb(session_msg_t msg, session_event_t event, void *user_data){
	if(g_session_notify_cb_table.user_cb){
		g_session_notify_cb_table.user_cb(msg, g_session_notify_cb_table.user_data);
	}
	if( g_session_notify_cb_table.interrupted_cb ){
		sound_interrupted_code_e e = SOUND_INTERRUPTED_COMPLETED;
		if( msg == MM_SESSION_MSG_RESUME )
			e = SOUND_INTERRUPTED_COMPLETED;
		else{
			switch(event){
				case MM_SESSION_EVENT_MEDIA :
					e = SOUND_INTERRUPTED_BY_MEDIA;
					break;
				case MM_SESSION_EVENT_RESUMABLE_MEDIA :
					e = SOUND_INTERRUPTED_BY_RESUMABLE_MEDIA;
					break;
				case MM_SESSION_EVENT_CALL :
					e = SOUND_INTERRUPTED_BY_CALL;
					break;
				case MM_SESSION_EVENT_ALARM :
					e = SOUND_INTERRUPTED_BY_ALARM;
					break;
				case MM_SESSION_EVENT_EARJACK_UNPLUG:
					e = SOUND_INTERRUPTED_BY_EARJACK_UNPLUG;
					break;
				case MM_SESSION_EVENT_RESOURCE_CONFLICT:
					e = SOUND_INTERRUPTED_BY_RESOURCE_CONFLICT;
					break;
				case MM_SESSION_EVENT_EMERGENCY:
					e = SOUND_INTERRUPTED_BY_EMERGENCY;
					break;
				default :
					e = SOUND_INTERRUPTED_BY_MEDIA;
					break;
			}
		}
		g_session_notify_cb_table.interrupted_cb(e, g_session_notify_cb_table.interrupted_user_data);
	}
}

static int __convert_sound_manager_error_code(const char *func, int code){
	int ret = SOUND_MANAGER_ERROR_NONE;
	char *errorstr = NULL;

	switch(code)
	{
		case SOUND_MANAGER_ERROR_INVALID_PARAMETER:
			ret = SOUND_MANAGER_ERROR_INVALID_PARAMETER;
			errorstr = "INVALID_PARAMETER";
			break;
		case SOUND_MANAGER_ERROR_INVALID_OPERATION:
			ret = SOUND_MANAGER_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION";
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
		case MM_ERROR_POLICY_DUPLICATED:
		case MM_ERROR_POLICY_BLOCKED:
		case MM_ERROR_POLICY_INTERNAL:
			ret = SOUND_MANAGER_ERROR_INVALID_OPERATION;
			errorstr = "INVALID_OPERATION"	;
			break;
		case MM_ERROR_SOUND_VOLUME_NO_INSTANCE:
		case MM_ERROR_SOUND_VOLUME_CAPTURE_ONLY:
			ret = SOUND_MANAGER_ERROR_NO_PLAYING_SOUND;
			errorstr = "NO_PLAYING_SOUND"	;
			break;
	}
	LOGE("[%s] %s(0x%08x) : core frameworks error code(0x%08x)",func, errorstr, ret, code);
	return ret;
}

int sound_manager_get_max_volume(sound_type_e type, int *max)
{
	int volume;
	if(max == NULL)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);

	if(type > MAX_VOLUME_TYPE || type < 0)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret = mm_sound_volume_get_step(type, &volume);

	if(ret == 0)
		*max = volume -1;	// actual volume step can be max step - 1

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume(sound_type_e type, int volume)
{
	if(type > MAX_VOLUME_TYPE || type < 0)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	if(volume < 0)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);

	int ret = mm_sound_volume_set_value(type, volume);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_volume(sound_type_e type, int *volume)
{
	unsigned int uvolume;
	if(type > MAX_VOLUME_TYPE || type < 0)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	if(volume == NULL)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret = mm_sound_volume_get_value(type, &uvolume);

	if(ret == 0)
		*volume = uvolume;

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_current_sound_type(sound_type_e *type)
{
	if(type == NULL)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret;
	ret = mm_sound_volume_get_current_playing_type((volume_type_t *)type);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume_changed_cb(sound_manager_volume_changed_cb callback, void* user_data)
{
	if(callback == NULL)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int i;
	g_volume_changed_cb_table.user_cb = callback;
	g_volume_changed_cb_table.user_data = user_data;
	for(i = 0 ; i <= MAX_VOLUME_TYPE ; i++)
	{
		mm_sound_volume_add_callback(i , __volume_changed_cb ,(void*) i);
	}
	return 0;
}

void sound_manager_unset_volume_changed_cb(void)
{
	int i;
	for(i = 0 ; i <= MAX_VOLUME_TYPE ; i++)
	{
		mm_sound_volume_remove_callback(i);
	}
	g_volume_changed_cb_table.user_cb = NULL;
	g_volume_changed_cb_table.user_data = NULL;
}

int sound_manager_get_a2dp_status(bool *connected , char** bt_name){
	int ret = mm_sound_route_get_a2dp_status(connected , bt_name);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_session_type(sound_session_type_e type){
	int ret = 0;
	int session = 0;
	if(type < 0 || type >  SOUND_SESSION_TYPE_EMERGENCY)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);

	/* if call session is activated, can't set session */
	ret = mm_session_get_current_type(&session);
	if( ret == 0 && session >= MM_SESSION_TYPE_CALL ) {
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_OPERATION);
	}

	if(g_session_notify_cb_table.is_registered){
		ret = mm_session_finish();
		if (ret != MM_ERROR_NONE) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		g_session_notify_cb_table.is_registered = 0;
	}

	ret = mm_session_init_ex(type , __session_notify_cb, NULL);
	if(ret == 0){
		g_session_notify_cb_table.is_registered = 1;
	}
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_session_type(sound_session_type_e *type){
	if( type == NULL )
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret = 0;
	int session;
	ret = mm_session_get_current_type(&session);
	if( ret !=0 )
		session = SOUND_SESSION_TYPE_DEFAULT;
	if( session > SOUND_SESSION_TYPE_EMERGENCY ){ // call session or voip session
		if( g_cached_session != -1 )
			session = g_cached_session;	 // saved session
		else //will be never reach here. just prevent code
			session = SOUND_SESSION_TYPE_DEFAULT;
	}

	*type = session;
	return 0;
}


int sound_manager_set_session_notify_cb(sound_session_notify_cb callback , void *user_data){
	int ret =0 ;
	if(callback == NULL)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);


	if(g_session_notify_cb_table.is_registered ==0){
		ret = mm_session_init_ex(SOUND_SESSION_TYPE_DEFAULT /*default*/ , __session_notify_cb, NULL);
		if(ret != 0)
			return __convert_sound_manager_error_code(__func__, ret);
		g_session_notify_cb_table.is_registered = 1;
	}

	g_session_notify_cb_table.user_cb = callback;
	g_session_notify_cb_table.user_data  = user_data;
	return SOUND_MANAGER_ERROR_NONE;
}

void sound_manager_unset_session_notify_cb(void){
	g_session_notify_cb_table.user_cb = NULL;
	g_session_notify_cb_table.user_data  = NULL;
}

int sound_manager_set_interrupted_cb(sound_interrupted_cb callback, void *user_data){
	int ret =0 ;
	if(callback == NULL)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);

	if(g_session_notify_cb_table.is_registered ==0){
		ret = mm_session_init_ex(SOUND_SESSION_TYPE_DEFAULT /*default*/ , __session_notify_cb, NULL);
		if(ret != 0)
			return __convert_sound_manager_error_code(__func__, ret);
		g_session_notify_cb_table.is_registered = 1;
	}

	g_session_notify_cb_table.interrupted_cb= callback;
	g_session_notify_cb_table.interrupted_user_data = user_data;
	return SOUND_MANAGER_ERROR_NONE;
}

void sound_manager_unset_interrupted_cb(void){
	g_session_notify_cb_table.interrupted_cb= NULL;
	g_session_notify_cb_table.interrupted_user_data = NULL;
}


int sound_manager_set_volume_key_type(volume_key_type_e type){
	if(type < VOLUME_KEY_TYPE_NONE || type > VOLUME_KEY_TYPE_CALL)
		return __convert_sound_manager_error_code(__func__, SOUND_MANAGER_ERROR_INVALID_PARAMETER);
	int ret;
	if(type == VOLUME_KEY_TYPE_NONE)
		ret = mm_sound_volume_primary_type_clear();
	else
		ret = mm_sound_volume_primary_type_set(type);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_foreach_available_route (sound_available_route_cb callback, void *user_data)
{
	int ret;
	ret = mm_sound_foreach_available_route_cb((mm_sound_available_route_cb)callback, user_data);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_active_route (sound_route_e route)
{
	int ret;
	ret = mm_sound_set_active_route(route);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_active_device (sound_device_in_e *in, sound_device_out_e *out)
{
	int ret;
	ret = mm_sound_get_active_device((mm_sound_device_in *)in, (mm_sound_device_out *)out);

	return __convert_sound_manager_error_code(__func__, ret);
}

bool sound_manager_is_route_available (sound_route_e route)
{
	bool is_available;
	mm_sound_is_route_available(route, &is_available);

	return is_available;
}

int sound_manager_set_available_route_changed_cb (sound_available_route_changed_cb callback, void *user_data)
{
	int ret;
	ret = mm_sound_add_available_route_changed_callback((mm_sound_available_route_changed_cb)callback, user_data);

	return __convert_sound_manager_error_code(__func__, ret);
}

void sound_manager_unset_available_route_changed_cb (void)
{
	mm_sound_remove_available_route_changed_callback();
}

int sound_manager_set_active_device_changed_cb (sound_active_device_changed_cb callback, void *user_data)
{
	int ret;
	ret = mm_sound_add_active_device_changed_callback((mm_sound_active_device_changed_cb)callback, user_data);

	return __convert_sound_manager_error_code(__func__, ret);
}

void sound_manager_unset_active_device_changed_cb (void)
{
	mm_sound_remove_active_device_changed_callback();
}

struct sound_call_session_s
{
	int previous_session;
};

int sound_manager_call_session_create(sound_call_session_type_e type, sound_call_session_h *session)
{
	int ret = SOUND_MANAGER_ERROR_NONE;
	sound_call_session_h handle = NULL;

	/* Check input parameters */
	if(type < SOUND_SESSION_TYPE_CALL || type > SOUND_SESSION_TYPE_VOIP || session == NULL) {
		ret = SOUND_MANAGER_ERROR_INVALID_PARAMETER;
		goto ERROR;
	}

	/* Allocate handle */
	handle = malloc(sizeof(struct sound_call_session_s));
	if(!handle) {
		ret = SOUND_MANAGER_ERROR_OUT_OF_MEMORY;
		goto ERROR;
	}
	memset(handle, 0, sizeof(struct sound_call_session_s));

	/* Finish previous session if exists */
	if(g_session_notify_cb_table.is_registered){
		sound_manager_get_session_type((sound_session_type_e*)&handle->previous_session);
		g_cached_session = handle->previous_session;

		ret = mm_session_finish();
		if (ret != MM_ERROR_NONE) {
			goto ERROR;
		}
		g_session_notify_cb_table.is_registered = 0;

	}else{
		g_cached_session = SOUND_SESSION_TYPE_DEFAULT;
		handle->previous_session = SOUND_SESSION_TYPE_DEFAULT;
	}

	/* Initialize session */
	switch(type) {
	case SOUND_SESSION_TYPE_CALL:
		ret = mm_session_init_ex(MM_SESSION_TYPE_CALL, __session_notify_cb, NULL);
		break;
	case SOUND_SESSION_TYPE_VOIP:
		ret = mm_session_init_ex(MM_SESSION_TYPE_VIDEOCALL,__session_notify_cb, NULL);
		break;
	}
	if(ret != MM_ERROR_NONE)
		goto ERROR;

	g_session_notify_cb_table.is_registered = 1;
	*session = handle;

	return SOUND_MANAGER_ERROR_NONE;

ERROR:
	if(handle)
		free(handle);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_call_session_set_mode(sound_call_session_h session, sound_call_session_mode_e mode)
{
	int ret = SOUND_MANAGER_ERROR_NONE;

	if(mode < SOUND_CALL_SESSION_MODE_VOICE || mode > SOUND_CALL_SESSION_MODE_MEDIA || session == NULL) {
		ret = SOUND_MANAGER_ERROR_INVALID_PARAMETER;
		goto ERROR;
	}

	ret = mm_session_set_subsession ((mm_subsession_t)mode);

	if(ret != MM_ERROR_NONE)
		goto ERROR;

	return SOUND_MANAGER_ERROR_NONE;

ERROR:
	return __convert_sound_manager_error_code(__func__, ret);
}

int  sound_manager_call_session_get_mode(sound_call_session_h session, sound_call_session_mode_e *mode)
{
	int ret = SOUND_MANAGER_ERROR_NONE;

	if(mode == NULL || session == NULL) {
		ret = SOUND_MANAGER_ERROR_INVALID_PARAMETER;
		goto ERROR;
	}

	ret = mm_session_get_subsession ((mm_subsession_t *)mode);

	if(ret != MM_ERROR_NONE)
		goto ERROR;

	return SOUND_MANAGER_ERROR_NONE;

ERROR:
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_call_session_destroy(sound_call_session_h session)
{
	int ret = SOUND_MANAGER_ERROR_NONE;

	if(session == NULL) {
		ret = SOUND_MANAGER_ERROR_INVALID_PARAMETER;
		goto ERROR;
	}

	if(g_session_notify_cb_table.is_registered){
		ret = mm_session_finish();
		if(ret != MM_ERROR_NONE)
			goto ERROR;
		g_session_notify_cb_table.is_registered = 0;
	}

	/* Restore previous session */
	ret = mm_session_init_ex(session->previous_session , __session_notify_cb, NULL);
	if(ret == 0){
		g_session_notify_cb_table.is_registered = 1;
	}

	if(session)
		free(session);

	return SOUND_MANAGER_ERROR_NONE;

ERROR:
	return __convert_sound_manager_error_code(__func__, ret);
}

