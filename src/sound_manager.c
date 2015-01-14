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


#define LOG_TAG "TIZEN_N_SOUND_MANAGER"

#include <sound_manager.h>
#include <sound_manager_private.h>

typedef struct {
	void *user_data;
	sound_manager_volume_changed_cb user_cb;
}_changed_volume_info_s;

_session_interrupt_info_s g_session_interrupt_cb_table = {0, NULL, NULL};
_volume_changed_info_s g_volume_changed_cb_table = {NULL, NULL};
_device_connected_info_s g_device_connected_cb_table = {NULL, NULL};
_device_changed_info_s g_device_info_changed_cb_table = {NULL, NULL};

sound_session_type_e g_cached_session = -1;
sound_session_voip_mode_e g_cached_voip_mode = -1;

int sound_manager_get_max_volume(sound_type_e type, int *max)
{
	int volume;
	if(max == NULL)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	if(type >= SOUND_TYPE_NUM || type < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	int ret = mm_sound_volume_get_step(type, &volume);

	if(ret == 0)
		*max = volume -1;	// actual volume step can be max step - 1

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume(sound_type_e type, int volume)
{
	if(type >= SOUND_TYPE_NUM || type < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	if(volume < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	int ret = mm_sound_volume_set_value(type, volume);
	LOGI("returns : type=%d, volume=%d, ret=0x%x", type, volume, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_volume(sound_type_e type, int *volume)
{
	unsigned int uvolume;
	if(type >= SOUND_TYPE_NUM || type < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	if(volume == NULL)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	int ret = mm_sound_volume_get_value(type, &uvolume);

	if(ret == 0)
		*volume = uvolume;

	LOGI("returns : type=%d, volume=%d, ret=0x%x", type, *volume, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_current_sound_type(sound_type_e type)
{
	int ret = MM_ERROR_NONE;
	if(type >= SOUND_TYPE_NUM || type < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_sound_volume_primary_type_set(type);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_current_sound_type(sound_type_e *type)
{
	int ret = MM_ERROR_NONE;
	if(type == NULL)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	ret = mm_sound_volume_get_current_playing_type((volume_type_t *)type);

	LOGI("returns : type=%d, ret=0x%x", *type, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_current_sound_type(void)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_volume_primary_type_clear();

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume_changed_cb(sound_manager_volume_changed_cb callback, void* user_data)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_add_volume_changed_callback((mm_sound_volume_changed_cb)callback, user_data);
	if (ret == MM_ERROR_NONE) {
		g_volume_changed_cb_table.user_cb = (sound_manager_volume_changed_cb)callback;
		g_volume_changed_cb_table.user_data = user_data;
	}

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_volume_changed_cb(void)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	if (g_volume_changed_cb_table.user_cb) {
		ret = mm_sound_remove_volume_changed_callback();
		if (ret == MM_ERROR_NONE) {
			g_volume_changed_cb_table.user_cb = NULL;
			g_volume_changed_cb_table.user_data = NULL;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_session_type(sound_session_type_e type)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	int cur_session = -1;
	int new_session = MM_SESSION_TYPE_MEDIA;

	LOGI(">> enter : type=%d", type);

	if(type < SOUND_SESSION_TYPE_MEDIA || type >  SOUND_SESSION_TYPE_VOIP)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	switch(type) {
	case SOUND_SESSION_TYPE_MEDIA:
		new_session = MM_SESSION_TYPE_MEDIA;
		break;
	case SOUND_SESSION_TYPE_ALARM:
		new_session = MM_SESSION_TYPE_ALARM;
		break;
	case SOUND_SESSION_TYPE_NOTIFICATION:
		new_session = MM_SESSION_TYPE_NOTIFY;
		break;
	case SOUND_SESSION_TYPE_EMERGENCY:
		new_session = MM_SESSION_TYPE_EMERGENCY;
		break;
	case SOUND_SESSION_TYPE_VOIP:
		new_session = MM_SESSION_TYPE_VOIP;
		break;
	}

	/* valid session check */
	ret = mm_session_get_current_type(&cur_session);
	if(ret == 0) {
		if (cur_session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (type > SOUND_SESSION_TYPE_MEDIA) {
				LOGE("<< leave : Could not set this type(%d) during camera/recorder/audio-io(in)/radio", type);
				return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		}
		if (cur_session == MM_SESSION_TYPE_CALL ||
			cur_session == MM_SESSION_TYPE_VIDEOCALL ||
			cur_session >= MM_SESSION_TYPE_VOICE_RECOGNITION) {
			return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
		}
	}

	if(g_session_interrupt_cb_table.is_registered) {
		if (new_session == cur_session ||
			((new_session == SOUND_SESSION_TYPE_MEDIA) && (cur_session == MM_SESSION_TYPE_MEDIA_RECORD))) {
			LOGI("<< leave : already set type=%d, ret=0x%x", type, ret);
			return SOUND_MANAGER_ERROR_NONE;
		} else {
			ret = mm_session_finish();
			if (ret != MM_ERROR_NONE) {
				return __convert_sound_manager_error_code(__func__, ret);
			}
			g_session_interrupt_cb_table.is_registered = 0;
			g_cached_voip_mode = -1;
		}
	}
	ret = mm_session_init_ex(new_session , __session_interrupt_cb, NULL);
	if(ret == 0){
		g_session_interrupt_cb_table.is_registered = 1;
	}
	if (new_session == MM_SESSION_TYPE_VOIP) {
		/* set default sub-session for voip */
		ret = mm_session_set_subsession (MM_SUBSESSION_TYPE_RINGTONE, MM_SUBSESSION_OPTION_NONE);
		if (ret != MM_ERROR_NONE) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		g_cached_voip_mode = SOUND_SESSION_VOIP_MODE_RINGTONE;
	}
	LOGI("<< leave : type=%d, ret=0x%x", type, ret);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_session_type(sound_session_type_e *type)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	int cur_session;

	if( type == NULL )
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	ret = mm_session_get_current_type(&cur_session);
	if (ret != 0)
		cur_session = SOUND_SESSION_TYPE_DEFAULT;
	if ((cur_session > MM_SESSION_TYPE_EMERGENCY) && (cur_session != MM_SESSION_TYPE_VOIP)) {
		if( g_cached_session != -1 )
			cur_session = g_cached_session;
		else //will be never reach here. just prevent code
			cur_session = SOUND_SESSION_TYPE_DEFAULT;
	}

	switch(cur_session) {
	case MM_SESSION_TYPE_MEDIA:
	case MM_SESSION_TYPE_MEDIA_RECORD:
		*type = SOUND_SESSION_TYPE_MEDIA;
		break;
	case MM_SESSION_TYPE_ALARM:
		*type = SOUND_SESSION_TYPE_ALARM;
		break;
	case MM_SESSION_TYPE_NOTIFY:
		*type = SOUND_SESSION_TYPE_NOTIFICATION;
		break;
	case MM_SESSION_TYPE_EMERGENCY:
		*type = SOUND_SESSION_TYPE_EMERGENCY;
		break;
	case MM_SESSION_TYPE_VOIP:
		*type = SOUND_SESSION_TYPE_VOIP;
		break;
	default:
		*type = cur_session;
		break;
	}

	LOGI("returns : type=%d, ret=0x%x", *type, ret);

#endif
	return 0;
}

int sound_manager_set_media_session_option(sound_session_option_for_starting_e s_option, sound_session_option_for_during_play_e d_option)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	int session = 0;
	int session_option = 0;
	int updated = 0;

	LOGI(">> enter : option for starting=%d, for during play=%d", s_option, d_option);

	if(s_option < 0 || s_option >  SOUND_SESSION_OPTION_PAUSE_OTHERS_WHEN_START)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	if(d_option < 0 || d_option >  SOUND_SESSION_OPTION_UNINTERRUPTIBLE_DURING_PLAY)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_session_get_current_information(&session, &session_option);
	if ( ret != 0 || !g_session_interrupt_cb_table.is_registered) {
		LOGW("need to set session type first");
		return __convert_sound_manager_error_code(__func__, ret);
	} else if ( ret == 0 && session > MM_SESSION_TYPE_MEDIA ) {
		if (session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (!g_session_interrupt_cb_table.is_registered) {
				LOGE("Already set by camera/recorder/audio-io(in)/radio API, but need to set session to Media first");
				return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		} else {
			return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
		}
	}

	switch (s_option) {
	case SOUND_SESSION_OPTION_MIX_WITH_OTHERS_WHEN_START:
		if (session_option & MM_SESSION_OPTION_PAUSE_OTHERS) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_REMOVE, MM_SESSION_OPTION_PAUSE_OTHERS);
			if(ret){
				return __convert_sound_manager_error_code(__func__, ret);
			}
			updated = 1;
		}
		break;
	case SOUND_SESSION_OPTION_PAUSE_OTHERS_WHEN_START:
		if (!(session_option & MM_SESSION_OPTION_PAUSE_OTHERS)) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_ADD, MM_SESSION_OPTION_PAUSE_OTHERS);
			if(ret){
				return __convert_sound_manager_error_code(__func__, ret);
			}
			updated = 1;
		}
		break;
	}

	switch (d_option) {
	case SOUND_SESSION_OPTION_INTERRUPTIBLE_DURING_PLAY:
		if (session_option & MM_SESSION_OPTION_UNINTERRUPTIBLE) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_REMOVE, MM_SESSION_OPTION_UNINTERRUPTIBLE);
			if(ret){
				return __convert_sound_manager_error_code(__func__, ret);
			}
			updated = 1;
		}
		break;
	case SOUND_SESSION_OPTION_UNINTERRUPTIBLE_DURING_PLAY:
		if (!(session_option & MM_SESSION_OPTION_UNINTERRUPTIBLE)) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_ADD, MM_SESSION_OPTION_UNINTERRUPTIBLE);
			if(ret){
				return __convert_sound_manager_error_code(__func__, ret);
			}
			updated = 1;
		}
		break;
	}

	if (updated) {
		LOGI("<< leave : updated");
	} else {
		LOGI("<< leave : already set same option(%x), skip it", session_option);
	}

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_media_session_option(sound_session_option_for_starting_e *s_option, sound_session_option_for_during_play_e *d_option)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	int session = 0;
	int session_options = 0;

	LOGI(">> enter");

	if( s_option == NULL || d_option == NULL )
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_session_get_current_information(&session, &session_options);
	if( ret != 0 ) {
		return __convert_sound_manager_error_code(__func__, ret);
	} else if (session > SOUND_SESSION_TYPE_MEDIA ) {
		if (session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (!g_session_interrupt_cb_table.is_registered) {
				LOGE("Already set by camera/recorder/audio-io(in)/radio API, but need to set session to Media first");
				return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		} else {
			return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
		}
	}
	/* get option */
	if (session_options & MM_SESSION_OPTION_PAUSE_OTHERS) {
		*s_option = SOUND_SESSION_OPTION_PAUSE_OTHERS_WHEN_START;
	} else {
		*s_option = SOUND_SESSION_OPTION_MIX_WITH_OTHERS_WHEN_START;
	}
	if (session_options & MM_SESSION_OPTION_UNINTERRUPTIBLE) {
		*d_option = SOUND_SESSION_OPTION_UNINTERRUPTIBLE_DURING_PLAY;
	} else {
		*d_option = SOUND_SESSION_OPTION_INTERRUPTIBLE_DURING_PLAY;
	}

	LOGI("<< leave : option for starting=%d, for during play=%d", *s_option, *d_option);

#endif
	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_set_media_session_resumption_option(sound_session_option_for_resumption_e option)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	int session = 0;
	int session_option = 0;
	int updated = 0;

	LOGI(">> enter : option for resumption=%d (0:by system, 1:by system or media paused)", option);

	if(option < SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM || option > SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM_OR_MEDIA_PAUSED)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_session_get_current_information(&session, &session_option);
	if ( ret != 0 || !g_session_interrupt_cb_table.is_registered) {
		LOGW("need to set session type first");
		return __convert_sound_manager_error_code(__func__, ret);
	} else if ( ret == 0 && session > MM_SESSION_TYPE_MEDIA ) {
		if (session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (!g_session_interrupt_cb_table.is_registered) {
				LOGE("Already set by camera/recorder/audio-io(in)/radio API, but need to set session to Media first");
				return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		} else {
			return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
		}
	}

	switch (option) {
	case SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM:
		if (session_option & MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_REMOVE, MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED);
			if(ret){
				return __convert_sound_manager_error_code(__func__, ret);
			}
			updated = 1;
		}
		break;
	case SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM_OR_MEDIA_PAUSED:
		if (!(session_option & MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED)) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_ADD, MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED);
			if(ret){
				return __convert_sound_manager_error_code(__func__, ret);
			}
			updated = 1;
		}
		break;
	}

	if (updated) {
		LOGI("<< leave : updated");
	} else {
		LOGI("<< leave : already set same option(%x), skip it", session_option);
	}

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_media_session_resumption_option(sound_session_option_for_resumption_e *option)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	int session = 0;
	int session_options = 0;

	LOGI(">> enter");

	if( option == NULL )
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	ret = mm_session_get_current_information(&session, &session_options);
	if( ret != 0 ) {
		return __convert_sound_manager_error_code(__func__, ret);
	} else if (session > SOUND_SESSION_TYPE_MEDIA ) {
		if (session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (!g_session_interrupt_cb_table.is_registered) {
				LOGE("Already set by camera/recorder/audio-io(in)/radio API, but need to set session to Media first");
				return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		} else {
			return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
		}
	}
	/* get option */
	if (session_options & MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED) {
		*option = SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM_OR_MEDIA_PAUSED;
	} else {
		*option = SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM;
	}

	LOGI("<< leave : option for resumption=%d (0:by system, 1:by system or media paused)", *option);

#endif
	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_set_voip_session_mode(sound_session_voip_mode_e mode)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	int session = 0;
	int session_options = 0;

	LOGI(">> enter : mode=%d", mode);

	ret = mm_session_get_current_information(&session, &session_options);
	if( ret != MM_ERROR_NONE ) {
		return __convert_sound_manager_error_code(__func__, ret);
	} else if (session != MM_SESSION_TYPE_VOIP ) {
		return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
	}

	if(mode < SOUND_SESSION_VOIP_MODE_RINGTONE || mode > SOUND_SESSION_VOIP_MODE_VOICE_WITH_BLUETOOTH) {
		ret = MM_ERROR_INVALID_ARGUMENT;
		return __convert_sound_manager_error_code(__func__, ret);
	}

	if (mode == SOUND_SESSION_VOIP_MODE_RINGTONE) {
		/* sub-session */
		if (g_cached_voip_mode != mode) {
			ret = mm_session_set_subsession (MM_SUBSESSION_TYPE_RINGTONE, MM_SUBSESSION_OPTION_NONE);
			if (ret != MM_ERROR_NONE) {
				return __convert_sound_manager_error_code(__func__, ret);
			}
		}
		g_cached_voip_mode = mode;
	} else {
		mm_sound_route route;
		bool need_to_check_device = false;
		bool do_subsession = true;
		switch (mode) {
		case SOUND_SESSION_VOIP_MODE_RINGTONE:
			do_subsession = false;
			break;
		case SOUND_SESSION_VOIP_MODE_VOICE_WITH_BUILTIN_RECEIVER:
			route = MM_SOUND_ROUTE_IN_MIC_OUT_RECEIVER;
			break;
		case SOUND_SESSION_VOIP_MODE_VOICE_WITH_BUILTIN_SPEAKER:
			route = MM_SOUND_ROUTE_IN_MIC_OUT_SPEAKER;
			break;
		case SOUND_SESSION_VOIP_MODE_VOICE_WITH_AUDIO_JACK:
			route = MM_SOUND_ROUTE_IN_MIC_OUT_HEADPHONE;
			need_to_check_device = true;
			break;
		case SOUND_SESSION_VOIP_MODE_VOICE_WITH_BLUETOOTH:
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
				return __convert_sound_manager_error_code(__func__, ret);
			} else {
				while ((w_ret = mm_sound_get_next_device(device_list, &device)) == MM_ERROR_NONE) {
					mm_sound_device_type_e type;
					ret = mm_sound_get_device_type(device, &type);
					if (ret != MM_ERROR_NONE)
						return __convert_sound_manager_error_code(__func__, ret);

					switch (mode) {
					case SOUND_SESSION_VOIP_MODE_VOICE_WITH_AUDIO_JACK:
						if (type == MM_SOUND_DEVICE_TYPE_AUDIOJACK)
							do_subsession = true;
						break;
					case SOUND_SESSION_VOIP_MODE_VOICE_WITH_BLUETOOTH:
						if (type == MM_SOUND_DEVICE_TYPE_BLUETOOTH) {
							mm_sound_device_io_direction_e io_direction;
							ret = mm_sound_get_device_io_direction(device, &io_direction);
							if (ret != MM_ERROR_NONE)
								return __convert_sound_manager_error_code(__func__, ret);
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
		if (do_subsession && (g_cached_voip_mode != mode)) {
			ret = mm_session_set_subsession (MM_SUBSESSION_TYPE_VOICE, MM_SUBSESSION_OPTION_NONE);
			if (ret != MM_ERROR_NONE) {
				return __convert_sound_manager_error_code(__func__, ret);
			}
			/* route */
			ret = mm_sound_set_active_route(route);
			if (ret != MM_ERROR_NONE) {
				return __convert_sound_manager_error_code(__func__, ret);
			}
			g_cached_voip_mode = mode;
		} else {
			if (!do_subsession)
				ret = MM_ERROR_SOUND_INTERNAL;
		}
	}

	LOGI("<< leave : session=%p, mode=%d, ret=0x%x", session, mode, ret);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_voip_session_mode(sound_session_voip_mode_e *mode)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	int session = 0;
	int session_options = 0;
	int subsession = 0;

	if(mode == NULL) {
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	}

	ret = mm_session_get_current_information(&session, &session_options);
	if( ret != MM_ERROR_NONE ) {
		return __convert_sound_manager_error_code(__func__, ret);
	} else if (session != MM_SESSION_TYPE_VOIP ) {
		return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
	}

	ret = mm_session_get_subsession ((mm_subsession_t *)&subsession);
	if(ret != MM_ERROR_NONE) {
		return __convert_sound_manager_error_code(__func__, ret);
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
			return __convert_sound_manager_error_code(__func__, ret);
		} else {
			while ((w_ret = mm_sound_get_next_device(device_list, &device)) == MM_ERROR_NONE) {
				mm_sound_device_type_e type;
				ret = mm_sound_get_device_type(device, &type);
				if (ret != MM_ERROR_NONE)
					return __convert_sound_manager_error_code(__func__, ret);
				switch (type) {
				case MM_SOUND_DEVICE_TYPE_BUILTIN_SPEAKER:
					*mode = SOUND_SESSION_VOIP_MODE_VOICE_WITH_BUILTIN_SPEAKER;
					need_to_out = true;
					break;
				case MM_SOUND_DEVICE_TYPE_BUILTIN_RECEIVER:
					*mode = SOUND_SESSION_VOIP_MODE_VOICE_WITH_BUILTIN_RECEIVER;
					need_to_out = true;
					break;
				case MM_SOUND_DEVICE_TYPE_AUDIOJACK:
					*mode = SOUND_SESSION_VOIP_MODE_VOICE_WITH_AUDIO_JACK;
					need_to_out = true;
					break;
				case MM_SOUND_DEVICE_TYPE_BLUETOOTH:
					*mode = SOUND_SESSION_VOIP_MODE_VOICE_WITH_BLUETOOTH;
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
		*mode = SOUND_SESSION_VOIP_MODE_RINGTONE;
		break;
	default:
		break;
	}

	LOGI("returns : session=%p, mode=%d, ret=0x%x", session, *mode, ret);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_session_interrupted_cb(sound_session_interrupted_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	if(callback == NULL)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	if(g_session_interrupt_cb_table.is_registered ==0){
		ret = mm_session_init_ex(SOUND_SESSION_TYPE_DEFAULT /*default*/ , __session_interrupt_cb, NULL);
		if(ret != 0)
			return __convert_sound_manager_error_code(__func__, ret);
		g_session_interrupt_cb_table.is_registered = 1;
	}

	g_session_interrupt_cb_table.user_cb = (sound_session_interrupted_cb)callback;
	g_session_interrupt_cb_table.user_data = user_data;
	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_unset_session_interrupted_cb(void)
{
	int ret = MM_ERROR_NONE;
	if (g_session_interrupt_cb_table.user_cb) {
		g_session_interrupt_cb_table.user_cb = NULL;
		g_session_interrupt_cb_table.user_data = NULL;
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_current_device_list(sound_device_mask_e device_mask, sound_device_list_h *device_list)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_get_current_device_list((mm_sound_device_flags_e)device_mask, device_list);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_next_device (sound_device_list_h device_list, sound_device_h *device)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_get_next_device(device_list, device);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_prev_device (sound_device_list_h device_list, sound_device_h *device)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_get_prev_device(device_list, device);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_type (sound_device_h device, sound_device_type_e *type)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_get_device_type(device, (mm_sound_device_type_e*)type);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_io_direction (sound_device_h device, sound_device_io_direction_e *io_direction)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_get_device_io_direction(device, (mm_sound_device_io_direction_e*)io_direction);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_id (sound_device_h device, int *id)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_get_device_id(device, id);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_name (sound_device_h device, char **name)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_get_device_name(device, name);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_state (sound_device_h device, sound_device_state_e *state)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_get_device_state(device, (mm_sound_device_state_e*)state);

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_device_connected_cb (sound_device_mask_e device_mask, sound_device_connected_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_add_device_connected_callback((mm_sound_device_flags_e)device_mask, (mm_sound_device_connected_cb)callback, user_data);
	if (ret == MM_ERROR_NONE) {
		g_device_connected_cb_table.user_cb = (sound_device_connected_cb)callback;
		g_device_connected_cb_table.user_data = user_data;
	}

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_device_connected_cb (void)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	if (g_device_connected_cb_table.user_cb) {
		ret = mm_sound_remove_device_connected_callback();
		if (ret == MM_ERROR_NONE) {
			g_device_connected_cb_table.user_cb = NULL;
			g_device_connected_cb_table.user_data = NULL;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_device_information_changed_cb (sound_device_mask_e device_mask, sound_device_information_changed_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	ret = mm_sound_add_device_information_changed_callback((mm_sound_device_flags_e)device_mask, (mm_sound_device_info_changed_cb)callback, user_data);
	if (ret == MM_ERROR_NONE) {
		g_device_info_changed_cb_table.user_cb = (sound_device_information_changed_cb)callback;
		g_device_info_changed_cb_table.user_data = user_data;
	}

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_device_information_changed_cb (void)
{
	int ret = MM_ERROR_NONE;
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
	if (g_device_info_changed_cb_table.user_cb) {
		ret = mm_sound_remove_device_information_changed_callback();
		if (ret == MM_ERROR_NONE) {
			g_device_info_changed_cb_table.user_cb = NULL;
			g_device_info_changed_cb_table.user_data = NULL;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

#endif
	return __convert_sound_manager_error_code(__func__, ret);
}

/* below APIs are already deprecated Tizen 2.3, leave it temporarily */
int sound_manager_get_a2dp_status(bool *connected , char** bt_name)
{
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

int sound_manager_set_session_notify_cb(sound_session_notify_cb callback , void *user_data)
{
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

void sound_manager_unset_session_notify_cb(void)
{
	return;
}

int sound_manager_set_interrupted_cb(sound_interrupted_cb callback, void *user_data){
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

void sound_manager_unset_interrupted_cb(void){
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

int sound_manager_set_volume_key_type(volume_key_type_e type){
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

int sound_manager_foreach_available_route (sound_available_route_cb callback, void *user_data)
{
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

int sound_manager_set_active_route (sound_route_e route)
{
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

int sound_manager_get_active_device (sound_device_in_e *in, sound_device_out_e *out)
{
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

bool sound_manager_is_route_available (sound_route_e route)
{
	return false;
}

int sound_manager_set_available_route_changed_cb (sound_available_route_changed_cb callback, void *user_data)
{
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

void sound_manager_unset_available_route_changed_cb (void)
{
	return;
}

int sound_manager_set_active_device_changed_cb (sound_active_device_changed_cb callback, void *user_data)
{
	return SOUND_MANAGER_ERROR_NOT_SUPPORTED;
}

void sound_manager_unset_active_device_changed_cb (void)
{
	return;
}

__attribute__ ((destructor))
void __sound_manager_finalize(void)
{
	int ret = MM_ERROR_NONE;

	if(g_session_interrupt_cb_table.is_registered){
		LOGI("<ENTER>");
		ret = mm_session_finish();
		if (ret != MM_ERROR_NONE) {
			LOGE("[%s] failed to mm_session_finish(), ret(0x%x)", __func__, ret);
		}
		g_session_interrupt_cb_table.is_registered = 0;
		LOGI("<LEAVE>");
	}
}

__attribute__ ((constructor))
void __sound_manager_initialize(void)
{

}
