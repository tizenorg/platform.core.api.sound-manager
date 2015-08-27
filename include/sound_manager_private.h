/*
* Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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

#ifndef __TIZEN_MEDIA_SOUND_MANAGER_PRIVATE_H__
#define __TIZEN_MEDIA_SOUND_MANAGER_PRIVATE_H__

#ifdef __cplusplus
extern "C"
{
#endif
#define LOG_TAG "TIZEN_N_SOUND_MANAGER"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <dlog.h>
#include <pulse/error.h>
#include <pulse/proplist.h>
#include <pulse/channelmap.h>
#include <pulse/pulseaudio.h>

#include <gio/gio.h>
#include <glib.h>
#include <mm_session.h>
#include <mm_session_private.h>
#include <mm_sound.h>
#include <mm_sound_focus.h>
#include <mm_sound_private.h>
#include "sound_manager_internal.h"

#define _CHECK_CONDITION(condition,error,msg)     \
    if(condition) {} else \
    { LOGE("[%s] %s(0x%08x)",__FUNCTION__, msg,error); return error;}; \

#define SM_INSTANCE_CHECK(handle)   \
        _CHECK_CONDITION(handle != NULL, SOUND_MANAGER_ERROR_INVALID_PARAMETER,"SOUND_MANAGER_ERROR_INVALID_PARAMETER")

#define SM_NULL_ARG_CHECK(arg)      \
        _CHECK_CONDITION(arg != NULL,SOUND_MANAGER_ERROR_INVALID_PARAMETER,"SOUND_MANAGER_ERROR_INVALID_PARAMETER")

#define SM_STATE_CHECK(handle,expected_state)       \
        _CHECK_CONDITION(handle->state == expected_state,SOUND_MANAGER_ERROR_INVALID_STATE,"SOUND_MANAGER_ERROR_INVALID_STATE")

#define SM_RANGE_ARG_CHECK(arg, min, max)      \
        _CHECK_CONDITION(arg <= max,SOUND_MANAGER_ERROR_INVALID_PARAMETER,"SOUND_MANAGER_ERROR_INVALID_PARAMETER") \
        _CHECK_CONDITION(arg >= min,SOUND_MANAGER_ERROR_INVALID_PARAMETER,"SOUND_MANAGER_ERROR_INVALID_PARAMETER")

#define SM_ENTER_CRITICAL_SECTION_WITH_RETURN(x_mutex,x_return) \
switch ( pthread_mutex_lock( x_mutex ) ) \
{ \
case EINVAL: \
	LOGW("try mutex init..\n"); \
	if( 0 > pthread_mutex_init( x_mutex, NULL) ) { \
		return x_return; \
	} else { \
		break; \
	} \
	return x_return; \
case 0: \
	break; \
default: \
	LOGE("mutex lock failed\n"); \
	return x_return; \
}

#define SM_LEAVE_CRITICAL_SECTION(x_mutex) \
if( pthread_mutex_unlock( x_mutex ) ) { \
	LOGE("mutex unlock failed\n"); \
}

#define SM_REF_FOR_STREAM_INFO(x_count, x_return) \
{ \
	if (!x_count) { \
		/* send signal to other framework to release internal focus */ \
		x_return = mm_sound_send_signal(MM_SOUND_SIGNAL_RELEASE_INTERNAL_FOCUS, 1); \
	} \
	if (x_return) \
		LOGW("failed to send signal for stream info creation"); \
	else \
		g_stream_info_count++; \
} \

#define SM_UNREF_FOR_STREAM_INFO(x_count, x_return) \
{ \
	x_count--; \
} \

#define SOUND_SESSION_TYPE_DEFAULT SOUND_SESSION_TYPE_MEDIA
#define SOUND_STREAM_INFO_ARR_MAX 128
#define SOUND_STREAM_TYPE_LEN 64
#define SOUND_STREAM_DIRECTION_MAX 2
#define SOUND_DEVICE_TYPE_LEN 64

typedef enum _sound_stream_direction {
	SOUND_STREAM_DIRECTION_OUTPUT = 1,
	SOUND_STREAM_DIRECTION_INPUT
} sound_stream_direction_e;

/* it should be synchronized with pulseaudio's */
typedef enum stream_route_type {
	STREAM_ROUTE_TYPE_AUTO,     /* the policy of decision device(s) is automatic and it's routing path is particular to one device */
	STREAM_ROUTE_TYPE_AUTO_ALL, /* the policy of decision device(s) is automatic and it's routing path can be several devices */
	STREAM_ROUTE_TYPE_MANUAL,   /* the policy of decision device(s) is manual */
} stream_route_type;
#define AVAIL_DEVICES_MAX 16
#define AVAIL_FRAMEWORKS_MAX 16

typedef struct _stream_conf_info_s {
	int priority;
	int route_type;
	gchar *avail_in_devices[AVAIL_DEVICES_MAX];
	gchar *avail_out_devices[AVAIL_DEVICES_MAX];
	gchar *avail_frameworks[AVAIL_FRAMEWORKS_MAX];
} stream_conf_info_s;

typedef struct _manual_route_info_s {
	unsigned int route_in_devices[AVAIL_DEVICES_MAX];
	unsigned int route_out_devices[AVAIL_DEVICES_MAX];
	bool is_set;
} manual_route_info_s;

typedef struct _sound_stream_info_s {
	unsigned int index;
	char *stream_type;
	pa_threaded_mainloop *pa_mainloop;
	pa_context *pa_context;
	stream_conf_info_s stream_conf_info;
	sound_stream_focus_mask_e acquired_focus;
	sound_stream_focus_state_changed_cb user_cb;
	void *user_data;
	manual_route_info_s manual_route_info;
} sound_stream_info_s;

typedef enum {
	_VSTREAM_STATE_READY,
	_VSTREAM_STATE_RUNNING,
} _vstream_state;

typedef struct _virtual_stream_info_s {
	_vstream_state state;
	char *stream_type;
	pa_threaded_mainloop *pa_mainloop;
	pa_context *pa_context;
	pa_stream *pa_stream[SOUND_STREAM_DIRECTION_MAX];
	pa_proplist *pa_proplist;
	sound_stream_info_s *stream_info;
} virtual_sound_stream_info_s;

typedef enum {
	_SESSION_MODE_RINGTONE = 0,                /**< session mode(voip/call) for ringtone */
	_SESSION_MODE_VOICE_WITH_BUILTIN_RECEIVER, /**< session mode(voip/call) for during call with built-in receiver */
	_SESSION_MODE_VOICE_WITH_BUILTIN_SPEAKER,  /**< session mode(voip/call) for during call with built-in speaker */
	_SESSION_MODE_VOICE_WITH_AUDIO_JACK,       /**< session mode(voip/call) for during call with audio jack */
	_SESSION_MODE_VOICE_WITH_BLUETOOTH,        /**< session mode(voip/call) for during call with bluetooth */
} _session_mode_e;

typedef struct {
	int is_registered;
	unsigned int subs_id; /* for internal device connected subscription */
	void *user_data;
	sound_session_interrupted_cb user_cb;
}_session_interrupt_info_s;

typedef struct {
	unsigned int subs_id;
	void *user_data;
	sound_manager_volume_changed_cb user_cb;
}_volume_changed_info_s;

typedef struct {
	int index;
	void *user_data;
	sound_stream_focus_state_watch_cb user_cb;
}_focus_watch_info_s;

typedef struct {
	unsigned int subs_id;
	void *user_data;
	sound_device_connected_cb user_cb;
}_device_connected_info_s;

typedef struct {
	unsigned int subs_id;
	void *user_data;
	sound_device_information_changed_cb user_cb;
}_device_changed_info_s;

void _focus_session_interrupt_cb (mm_sound_focus_state_e state, const char *reason_for_change, bool is_wcb, void *user_data);

void _voip_focus_state_change_callback (sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data);

void _device_connected_cb(sound_device_h device, bool is_connected, void *user_data);

void _focus_state_change_callback (int index, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data);

void _focus_watch_callback (int id, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data);

int _convert_sound_manager_error_code (const char *func, int code);

int _convert_stream_type (sound_stream_type_e enum_type, char **stream_type);

int _convert_stream_type_for_internal (sound_stream_type_internal_e stream_type_enum, char **stream_type);

int _convert_stream_type_to_change_reason (const char *stream_type, sound_stream_focus_change_reason_e *change_reason);

int _convert_device_type (sound_device_type_e device_type_enum, char **device_type);

int _convert_device_io_direction (mm_sound_device_io_direction_e io_direction, sound_device_io_direction_e *sound_io_direction);

const char* _convert_api_name (native_api_e api_name);

int _get_stream_conf_info (const char *stream_type, stream_conf_info_s *info);

int _set_manual_route_info (unsigned int index, manual_route_info_s *info);

int _set_route_option (unsigned int index, const char *key, int value);

int _convert_sound_type (sound_type_e sound_type, const char **volume_type);

int _convert_sound_type_to_enum (char *sound_type, sound_type_e *sound_type_enum);

int _get_volume_max_level (const char *direction, const char *volume_type, unsigned int *max_level);

int _get_current_volume_type (const char *direction, char **volume_type);

void _update_focus_status (unsigned int index, unsigned int acquired_focus_status);

void _pa_context_state_cb (pa_context *c, void *userdata);

void _pa_stream_state_cb (pa_stream *s, void * userdata);

int _set_session_mode (_session_mode_e mode);

int _make_pa_connection_and_register_focus(sound_stream_info_s *stream_h, sound_stream_focus_state_changed_cb callback, void *user_data);

int _destroy_pa_connection_and_unregister_focus(sound_stream_info_s *stream_h);

int _add_device_for_stream_routing (sound_stream_info_s *stream_info, sound_device_h device);

int _remove_device_for_stream_routing (sound_stream_info_s *stream_info, sound_device_h device);

int _apply_stream_routing (sound_stream_info_s *stream_info);

int _create_virtual_stream (sound_stream_info_s *stream_info, virtual_sound_stream_info_s **virtual_stream);

int _destroy_virtual_stream (virtual_sound_stream_info_s *virtual_stream);

int _start_virtual_stream (virtual_sound_stream_info_s *virtual_stream);

int _stop_virtual_stream (virtual_sound_stream_info_s *virtual_stream);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_SOUND_MANAGER_PRIVATE_H__ */
