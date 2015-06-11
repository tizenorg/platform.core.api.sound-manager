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

#include <sound_manager.h>
#include <sound_manager_private.h>

#include <mm_sound.h>
#include <dlog.h>

extern _session_interrupt_info_s g_session_interrupt_cb_table;
extern _session_mode_e g_cached_session_mode;

int __convert_sound_manager_error_code(const char *func, int code) {
	int ret = SOUND_MANAGER_ERROR_NONE;
	char *errorstr = NULL;

	switch(code)
	{
		case MM_ERROR_FILE_WRITE:
		case MM_ERROR_INVALID_HANDLE:
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
		case MM_ERROR_SOUND_PERMISSION_DENIED:
			ret = SOUND_MANAGER_ERROR_PERMISSION_DENIED;
			errorstr = "PERMISSION_DENIED";
			break;
		case MM_ERROR_SOUND_NO_DATA:
			ret = SOUND_MANAGER_ERROR_NO_DATA;
			errorstr = "NO_DATA";
			break;
		case MM_ERROR_SOUND_INTERNAL:
		case MM_ERROR_SOUND_VOLUME_CAPTURE_ONLY:
			ret = SOUND_MANAGER_ERROR_INTERNAL;
			errorstr = "INTERNAL";
			break;
		case MM_ERROR_POLICY_DUPLICATED:
		case MM_ERROR_POLICY_INTERNAL:
		case MM_ERROR_POLICY_BLOCKED:
			ret = SOUND_MANAGER_ERROR_POLICY;
			errorstr = "POLICY";
			break;
		case MM_ERROR_SOUND_VOLUME_NO_INSTANCE:
			ret = SOUND_MANAGER_ERROR_NO_PLAYING_SOUND;
			errorstr = "NO_PLAYING_SOUND";
			break;
	}
	LOGE("[%s] %s(0x%08x) : core frameworks error code(0x%08x)",func, errorstr, ret, code);
	return ret;
}

void __session_interrupt_cb(session_msg_t msg, session_event_t event, void *user_data){
	if( g_session_interrupt_cb_table.user_cb ){
		sound_session_interrupted_code_e e = SOUND_SESSION_INTERRUPTED_COMPLETED;
		if( msg == MM_SESSION_MSG_RESUME )
			e = SOUND_SESSION_INTERRUPTED_COMPLETED;
		else{
			switch(event){
				case MM_SESSION_EVENT_MEDIA :
					e = SOUND_SESSION_INTERRUPTED_BY_MEDIA;
					break;
				case MM_SESSION_EVENT_CALL :
					e = SOUND_SESSION_INTERRUPTED_BY_CALL;
					break;
				case MM_SESSION_EVENT_ALARM :
					e = SOUND_SESSION_INTERRUPTED_BY_ALARM;
					break;
				case MM_SESSION_EVENT_EARJACK_UNPLUG:
					e = SOUND_SESSION_INTERRUPTED_BY_EARJACK_UNPLUG;
					break;
				case MM_SESSION_EVENT_RESOURCE_CONFLICT:
					e = SOUND_SESSION_INTERRUPTED_BY_RESOURCE_CONFLICT;
					break;
				case MM_SESSION_EVENT_EMERGENCY:
					e = SOUND_SESSION_INTERRUPTED_BY_EMERGENCY;
					break;
				case MM_SESSION_EVENT_NOTIFICATION :
					e = SOUND_SESSION_INTERRUPTED_BY_NOTIFICATION;
					break;
				default :
					e = SOUND_SESSION_INTERRUPTED_BY_MEDIA;
					break;
			}
		}
		g_session_interrupt_cb_table.user_cb(e, g_session_interrupt_cb_table.user_data);
	}
}

int __set_session_mode(_session_mode_e mode)
{
	int ret = MM_ERROR_NONE;
	mm_sound_route route = MM_SOUND_ROUTE_IN_MIC_OUT_SPEAKER;
	bool need_to_check_device = false;
	bool do_subsession = true;

	switch (mode) {
		case _SESSION_MODE_RINGTONE:
			if (g_cached_session_mode != mode) {
				/* sub-session */
				ret = mm_session_set_subsession (MM_SUBSESSION_TYPE_RINGTONE, MM_SUBSESSION_OPTION_NONE);
				if (ret != MM_ERROR_NONE) {
					goto ERROR_CASE;
				}
			}
			g_cached_session_mode = mode;
			do_subsession = false;
			break;
		case _SESSION_MODE_VOICE_WITH_BUILTIN_RECEIVER:
			route = MM_SOUND_ROUTE_IN_MIC_OUT_RECEIVER;
			break;
		case _SESSION_MODE_VOICE_WITH_BUILTIN_SPEAKER:
			route = MM_SOUND_ROUTE_IN_MIC_OUT_SPEAKER;
			break;
		case _SESSION_MODE_VOICE_WITH_AUDIO_JACK:
			route = MM_SOUND_ROUTE_IN_MIC_OUT_HEADPHONE;
			need_to_check_device = true;
			break;
		case _SESSION_MODE_VOICE_WITH_BLUETOOTH:
			route = MM_SOUND_ROUTE_INOUT_BLUETOOTH;
			need_to_check_device = true;
			break;
	}

	if (need_to_check_device) {
		int w_ret = MM_ERROR_NONE;
		MMSoundDeviceList_t device_list;
		MMSoundDevice_t device;
		do_subsession = false;

		ret = mm_sound_get_current_device_list(MM_SOUND_DEVICE_STATE_DEACTIVATED_FLAG, &device_list);
		if (ret != MM_ERROR_NONE) {
			goto ERROR_CASE;
		} else {
			while ((w_ret = mm_sound_get_next_device(device_list, &device)) == MM_ERROR_NONE) {
				mm_sound_device_type_e type;
				ret = mm_sound_get_device_type(device, &type);
				if (ret != MM_ERROR_NONE)
					goto ERROR_CASE;

				switch (mode) {
					case _SESSION_MODE_VOICE_WITH_AUDIO_JACK:
						if (type == MM_SOUND_DEVICE_TYPE_AUDIOJACK) {
							mm_sound_device_io_direction_e io_direction;
							ret = mm_sound_get_device_io_direction(device, &io_direction);
							if (ret != MM_ERROR_NONE)
								goto ERROR_CASE;
							if (io_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH)
								route = MM_SOUND_ROUTE_INOUT_HEADSET;
							do_subsession = true;
						}
						break;
					case _SESSION_MODE_VOICE_WITH_BLUETOOTH:
						if (type == MM_SOUND_DEVICE_TYPE_BLUETOOTH) {
							mm_sound_device_io_direction_e io_direction;
							ret = mm_sound_get_device_io_direction(device, &io_direction);
							if (ret != MM_ERROR_NONE)
								goto ERROR_CASE;
							if (io_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH)
								do_subsession = true;
						}
						break;
					default:
						break;
				}
			}
		}
	}
	/* sub-session */
	if (do_subsession && (g_cached_session_mode != mode)) {
		ret = mm_session_set_subsession (MM_SUBSESSION_TYPE_VOICE, MM_SUBSESSION_OPTION_NONE);
		if (ret != MM_ERROR_NONE) {
			goto ERROR_CASE;
		}
		/* route */
		ret = mm_sound_set_active_route(route);
		if (ret != MM_ERROR_NONE) {
			goto ERROR_CASE;
		}
		g_cached_session_mode = mode;
	} else {
		if (!do_subsession && mode != _SESSION_MODE_RINGTONE) {
			ret = MM_ERROR_SOUND_INTERNAL;
		}
	}
ERROR_CASE:
	return ret;
}

int __get_session_mode(_session_mode_e *mode)
{
	int ret = MM_ERROR_NONE;
	int subsession = 0;
	ret = mm_session_get_subsession ((mm_subsession_t *)&subsession);
	if(ret != MM_ERROR_NONE) {
		goto ERROR_CASE;
	}
	switch (subsession) {
	case MM_SUBSESSION_TYPE_VOICE:
	{
		int w_ret = MM_ERROR_NONE;
		bool need_to_out = false;
		MMSoundDeviceList_t device_list;
		MMSoundDevice_t device;
		ret = mm_sound_get_current_device_list(MM_SOUND_DEVICE_STATE_ACTIVATED_FLAG, &device_list);
		if (ret != MM_ERROR_NONE) {
			goto ERROR_CASE;
		} else {
			while ((w_ret = mm_sound_get_next_device(device_list, &device)) == MM_ERROR_NONE) {
				mm_sound_device_type_e type;
				ret = mm_sound_get_device_type(device, &type);
				if (ret != MM_ERROR_NONE)
					goto ERROR_CASE;
				switch (type) {
				case MM_SOUND_DEVICE_TYPE_BUILTIN_SPEAKER:
					*mode = _SESSION_MODE_VOICE_WITH_BUILTIN_SPEAKER;
					need_to_out = true;
					break;
				case MM_SOUND_DEVICE_TYPE_BUILTIN_RECEIVER:
					*mode = _SESSION_MODE_VOICE_WITH_BUILTIN_RECEIVER;
					need_to_out = true;
					break;
				case MM_SOUND_DEVICE_TYPE_AUDIOJACK:
					*mode = _SESSION_MODE_VOICE_WITH_AUDIO_JACK;
					need_to_out = true;
					break;
				case MM_SOUND_DEVICE_TYPE_BLUETOOTH:
					*mode = _SESSION_MODE_VOICE_WITH_BLUETOOTH;
					need_to_out = true;
					break;
				default:
					break;
				}
				if (need_to_out)
					break;
			}
		}
	}
		break;
	case MM_SUBSESSION_TYPE_RINGTONE:
		*mode = _SESSION_MODE_RINGTONE;
		break;
	default:
		break;
	}
ERROR_CASE:
	return ret;
}
