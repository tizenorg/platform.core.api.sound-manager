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

#include "sound_manager.h"
#include "sound_manager_private.h"

_session_interrupt_info_s g_session_interrupt_cb_table = {0, NULL, NULL};
_volume_changed_info_s g_volume_changed_cb_table = {NULL, NULL};
_focus_watch_info_s g_focus_watch_cb_table = {NULL, NULL};
_device_connected_info_s g_device_connected_cb_table = {NULL, NULL};
_device_changed_info_s g_device_info_changed_cb_table = {NULL, NULL};

sound_session_type_e g_cached_session = -1;
_session_mode_e g_cached_session_mode = -1;

#ifdef TMP_CODE
/*temporary variable for set/get voip session mode. When 2.4  feature for routing is fully implemented, it will be removed.*/
sound_session_voip_mode_e tmp_mode = -1;
#endif

int sound_manager_get_max_volume (sound_type_e type, int *max)
{
	const char *volume_type = NULL;
	unsigned int max_level = 0;
	int ret = MM_ERROR_NONE;
	if(max == NULL)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	if(type >= SOUND_TYPE_NUM || type < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	ret = __convert_sound_type (type, &volume_type);
	if (!ret)
		ret = __get_volume_max_level("out", volume_type, &max_level);

	if(ret == 0)
		*max = (int)max_level -1;	// actual volume step can be max step - 1

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume (sound_type_e type, int volume)
{
	if(type >= SOUND_TYPE_NUM || type < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	if(volume < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	int ret = mm_sound_volume_set_value(type, volume);
	LOGI("returns : type=%d, volume=%d, ret=%p", type, volume, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_volume (sound_type_e type, int *volume)
{
	unsigned int uvolume;
	if(type >= SOUND_TYPE_NUM || type < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	if(volume == NULL)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	int ret = mm_sound_volume_get_value(type, &uvolume);

	if(ret == 0)
		*volume = uvolume;

	LOGI("returns : type=%d, volume=%d, ret=%p", type, *volume, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_current_sound_type (sound_type_e type)
{
	int ret = MM_ERROR_NONE;
	if(type >= SOUND_TYPE_NUM || type < 0)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_sound_volume_primary_type_set(type);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_current_sound_type (sound_type_e *type)
{
	int ret = MM_ERROR_NONE;
	if(type == NULL)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	ret = mm_sound_volume_get_current_playing_type((volume_type_t *)type);

	LOGI("returns : type=%d, ret=%p", *type, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_current_sound_type (void)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_volume_primary_type_clear();

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume_changed_cb (sound_manager_volume_changed_cb callback, void* user_data)
{
	int ret = MM_ERROR_NONE;

	ret = mm_sound_add_volume_changed_callback((mm_sound_volume_changed_cb)callback, user_data);
	if (ret == MM_ERROR_NONE) {
		g_volume_changed_cb_table.user_cb = (sound_manager_volume_changed_cb)callback;
		g_volume_changed_cb_table.user_data = user_data;
	}

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_volume_changed_cb (void)
{
	int ret = MM_ERROR_NONE;

	if (g_volume_changed_cb_table.user_cb) {
		ret = mm_sound_remove_volume_changed_callback();
		if (ret == MM_ERROR_NONE) {
			g_volume_changed_cb_table.user_cb = NULL;
			g_volume_changed_cb_table.user_data = NULL;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_create_stream_information (sound_stream_type_e stream_type, sound_stream_focus_state_changed_cb callback, void *user_data, sound_stream_info_h *stream_info)
{
	int ret = MM_ERROR_NONE;

	LOGI(">> enter");

	SM_NULL_ARG_CHECK(stream_info);
	SM_NULL_ARG_CHECK(callback);

	sound_stream_info_s *stream_h = malloc(sizeof(sound_stream_info_s));
	if (!stream_h) {
		ret = MM_ERROR_OUT_OF_MEMORY;
	} else {
		memset(stream_h, 0, sizeof(sound_stream_info_s));
		ret = __convert_stream_type(stream_type, stream_h->stream_type);
		if (ret == MM_ERROR_NONE) {
			ret = _make_pa_connection_and_register_focus(stream_h, callback, user_data);
			if (!ret) {
				*stream_info = (sound_stream_info_h)stream_h;
				LOGI("<< leave : stream_h(%p), index(%u), user_cb(%p), ret(%p)", stream_h, stream_h->index, stream_h->user_cb, ret);
			}
		}
	}

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_destroy_stream_information (sound_stream_info_h stream_info)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	ret = _destroy_pa_connection_and_unregister_focus(stream_h);

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_add_device_for_stream_routing (sound_stream_info_h stream_info, sound_device_h device)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	int j = 0;
	bool added_successfully = false;
	char *device_type_str = NULL;
	int device_id = 0;
	mm_sound_device_type_e device_type;
	mm_sound_device_io_direction_e device_direction;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(device);

	if (stream_h->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL) {
		ret = mm_sound_get_device_id(device, &device_id);
		if (ret) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		ret = mm_sound_get_device_type(device, &device_type);
		if (ret) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		ret = __convert_device_type(device_type, &device_type_str);
		if (ret) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		ret = mm_sound_get_device_io_direction(device, &device_direction);
		if (ret) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		if (device_direction == MM_SOUND_DEVICE_IO_DIRECTION_IN || device_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
			for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
				if (stream_h->stream_conf_info.avail_in_devices[i]) {
					if(!strncmp(stream_h->stream_conf_info.avail_in_devices[i], device_type_str, SOUND_DEVICE_TYPE_LEN)) {
						for (j = 0; j < AVAIL_DEVICES_MAX; j++) {
							if (!stream_h->manual_route_info.route_in_devices[j]) {
								stream_h->manual_route_info.route_in_devices[j] = (unsigned int)device_id;
								added_successfully = true;
								break;
							}
							if (stream_h->manual_route_info.route_in_devices[j] == (unsigned int)device_id) {
								/* it was already set */
								return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_DUPLICATED);
							}
						}
					}
				} else {
					break;
				}
			}
		}
		if (device_direction == MM_SOUND_DEVICE_IO_DIRECTION_OUT || device_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
			for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
				if (stream_h->stream_conf_info.avail_out_devices[i]) {
					if(!strncmp(stream_h->stream_conf_info.avail_out_devices[i], device_type_str, SOUND_DEVICE_TYPE_LEN)) {
						for (j = 0; j < AVAIL_DEVICES_MAX; j++) {
							if (!stream_h->manual_route_info.route_out_devices[j]) {
								stream_h->manual_route_info.route_out_devices[j] = (unsigned int)device_id;
								added_successfully = true;
								break;
							}
							if (stream_h->manual_route_info.route_out_devices[j] == (unsigned int)device_id) {
								/* it was already set */
								return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_DUPLICATED);
							}
						}
					}
				} else {
					break;
				}
			}
		}
	}

	if (!added_successfully) {
		ret = MM_ERROR_POLICY_INTERNAL;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_remove_device_for_stream_routing (sound_stream_info_h stream_info, sound_device_h device)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	int j = 0;
	bool removed_successfully = false;
	char *device_type_str = NULL;
	int device_id = 0;
	mm_sound_device_type_e device_type;
	mm_sound_device_io_direction_e device_direction;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(device);

	if (stream_h->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL) {
		ret = mm_sound_get_device_id(device, &device_id);
		if (ret) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		ret = mm_sound_get_device_type(device, &device_type);
		if (ret) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		ret = __convert_device_type(device_type, &device_type_str);
		if (ret) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		ret = mm_sound_get_device_io_direction(device, &device_direction);
		if (ret) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		if (device_direction == MM_SOUND_DEVICE_IO_DIRECTION_IN || device_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
			for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
				if (stream_h->stream_conf_info.avail_in_devices[i]) {
					if(!strncmp(stream_h->stream_conf_info.avail_in_devices[i], device_type_str, SOUND_DEVICE_TYPE_LEN)) {
						for (j = 0; j < AVAIL_DEVICES_MAX; j++) {
							if (stream_h->manual_route_info.route_in_devices[j] == (unsigned int)device_id) {
								removed_successfully = true;
								stream_h->manual_route_info.route_in_devices[j] = 0;
								break;
							}
						}
					}
				} else {
					break;
				}
			}
		}
		if (device_direction == MM_SOUND_DEVICE_IO_DIRECTION_OUT || device_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
			for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
				if (stream_h->stream_conf_info.avail_out_devices[i]) {
					if(!strncmp(stream_h->stream_conf_info.avail_out_devices[i], device_type_str, SOUND_DEVICE_TYPE_LEN)) {
						for (j = 0; j < AVAIL_DEVICES_MAX; j++) {
							if (stream_h->manual_route_info.route_out_devices[j] == (unsigned int)device_id) {
								removed_successfully = true;
								stream_h->manual_route_info.route_out_devices[j] = 0;
								break;
							}
						}
					}
				} else {
					break;
				}
			}
		}
	}

	if (!removed_successfully) {
		ret = MM_ERROR_INVALID_ARGUMENT;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_apply_stream_routing (sound_stream_info_h stream_info)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	bool need_to_apply = false;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	if (stream_h->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL) {
		for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
			if (stream_h->manual_route_info.route_in_devices[i]) {
				need_to_apply = true;
				break;
			}
			if (stream_h->manual_route_info.route_out_devices[i]) {
				need_to_apply = true;
				break;
			}
		}
		if (need_to_apply) {
			ret = __set_manual_route_info(stream_h->index, &stream_h->manual_route_info);
		} else {
			__convert_sound_manager_error_code(__func__, MM_ERROR_SOUND_INVALID_STATE);
		}
	} else {
		ret = MM_ERROR_SOUND_INVALID_STATE;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_acquire_focus (sound_stream_info_h stream_info, sound_stream_focus_mask_e focus_mask, const char *additional_info)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	ret = mm_sound_acquire_focus(stream_h->index, (mm_sound_focus_type_e)focus_mask, additional_info);
	if (ret == MM_ERROR_NONE) {
		stream_h->acquired_focus |= focus_mask;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_release_focus (sound_stream_info_h stream_info, sound_stream_focus_mask_e focus_mask, const char *additional_info)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	ret = mm_sound_release_focus(stream_h->index, (mm_sound_focus_type_e)focus_mask, additional_info);
	if (ret == MM_ERROR_NONE) {
		stream_h->acquired_focus &= ~focus_mask;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_focus_state (sound_stream_info_h stream_info, sound_stream_focus_state_e *state_for_playback, sound_stream_focus_state_e *state_for_recording)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	if (!state_for_playback && !state_for_recording)
		ret = MM_ERROR_INVALID_ARGUMENT;

	if (state_for_playback)
		*state_for_playback = (stream_h->acquired_focus & SOUND_STREAM_FOCUS_FOR_PLAYBACK)?SOUND_STREAM_FOCUS_STATE_ACQUIRED:SOUND_STREAM_FOCUS_STATE_RELEASED;
	if (state_for_recording)
		*state_for_recording = (stream_h->acquired_focus & SOUND_STREAM_FOCUS_FOR_RECORDING)?SOUND_STREAM_FOCUS_STATE_ACQUIRED:SOUND_STREAM_FOCUS_STATE_RELEASED;

	LOGI("<< leave : acquired_focus(%p)", stream_h->acquired_focus);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_focus_state_watch_cb (sound_stream_focus_mask_e focus_mask, sound_stream_focus_state_watch_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;

	LOGI(">> enter");

	SM_NULL_ARG_CHECK(callback);

	if (!g_focus_watch_cb_table.user_cb) {
		ret = mm_sound_set_focus_watch_callback((mm_sound_focus_type_e)focus_mask, _focus_watch_callback, user_data);
		if (ret == MM_ERROR_NONE) {
			g_focus_watch_cb_table.user_cb = callback;
			g_focus_watch_cb_table.user_data = user_data;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_focus_state_watch_cb (void)
{
	int ret = MM_ERROR_NONE;

	LOGI(">> enter");

	if (g_focus_watch_cb_table.user_cb) {
		ret = mm_sound_unset_focus_watch_callback();
		if (ret == MM_ERROR_NONE) {
			g_focus_watch_cb_table.user_cb = NULL;
			g_focus_watch_cb_table.user_data = NULL;
		} else {
			ret = MM_ERROR_SOUND_INTERNAL;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_session_type (sound_session_type_e type)
{
	int ret = MM_ERROR_NONE;
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
		if (cur_session == MM_SESSION_TYPE_VIDEOCALL ||
			cur_session >= MM_SESSION_TYPE_VOICE_RECOGNITION) {
			return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
		}
	}

	if(g_session_interrupt_cb_table.is_registered) {
		if (new_session == cur_session ||
			((new_session == SOUND_SESSION_TYPE_MEDIA) && (cur_session == MM_SESSION_TYPE_MEDIA_RECORD))) {
			LOGI("<< leave : already set type=%d, ret=%p", type, ret);
			return SOUND_MANAGER_ERROR_NONE;
		} else {
			ret = mm_session_finish();
			if (ret != MM_ERROR_NONE) {
				return __convert_sound_manager_error_code(__func__, ret);
			}
			g_session_interrupt_cb_table.is_registered = 0;
			g_cached_session_mode = -1;
		}
	}
	ret = mm_session_init_ex(new_session , _session_interrupt_cb, NULL);
	if(ret == 0){
		g_session_interrupt_cb_table.is_registered = 1;
	}
	if (new_session == MM_SESSION_TYPE_VOIP || new_session == MM_SESSION_TYPE_CALL) {
		/* set default sub-session for voip */
		ret = mm_session_set_subsession (MM_SUBSESSION_TYPE_RINGTONE, MM_SUBSESSION_OPTION_NONE);
		if (ret != MM_ERROR_NONE) {
			return __convert_sound_manager_error_code(__func__, ret);
		}
		g_cached_session_mode = _SESSION_MODE_RINGTONE;
	}
	LOGI("<< leave : type=%d, ret=%p", type, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_session_type (sound_session_type_e *type)
{
	int ret = MM_ERROR_NONE;
	int cur_session;

	if( type == NULL )
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	ret = mm_session_get_current_type(&cur_session);
	if (ret != 0)
		cur_session = SOUND_SESSION_TYPE_DEFAULT;
	if ((cur_session > MM_SESSION_TYPE_EMERGENCY) &&
			(cur_session != MM_SESSION_TYPE_VOIP)) {
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

	LOGI("returns : type=%d, ret=%p", *type, ret);

	return 0;
}

int sound_manager_set_media_session_option (sound_session_option_for_starting_e s_option, sound_session_option_for_during_play_e d_option)
{
	int ret = MM_ERROR_NONE;
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

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_media_session_option (sound_session_option_for_starting_e *s_option, sound_session_option_for_during_play_e *d_option)
{
	int ret = MM_ERROR_NONE;
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

	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_set_media_session_resumption_option (sound_session_option_for_resumption_e option)
{
	int ret = MM_ERROR_NONE;
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

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_media_session_resumption_option (sound_session_option_for_resumption_e *option)
{
	int ret = MM_ERROR_NONE;
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

	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_set_voip_session_mode (sound_session_voip_mode_e mode)
{
	int ret = MM_ERROR_NONE;
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
	ret = __set_session_mode ((_session_mode_e)mode);

#ifdef TMP_CODE
	/* temporary code. When 2.4 feature for routing is fully implemented, it will be removed. */
	tmp_mode = mode;
#endif

	LOGI("<< leave : session=%p, mode=%d, ret=%p", session, mode, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_voip_session_mode (sound_session_voip_mode_e *mode)
{
	int ret = MM_ERROR_NONE;
	int session = 0;
	int session_options = 0;
	_session_mode_e _mode = 0;

	if(mode == NULL) {
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	}

	ret = mm_session_get_current_information(&session, &session_options);
	if( ret != MM_ERROR_NONE ) {
		return __convert_sound_manager_error_code(__func__, ret);
	} else if (session != MM_SESSION_TYPE_VOIP ) {
		return __convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
	}
	ret = __get_session_mode(&_mode);
	if (ret == MM_ERROR_NONE)
		*mode = (sound_session_voip_mode_e)_mode;

#ifdef TMP_CODE
	/* temporary code. When 2.4 feature for routing is fully implemented, it will be removed. */
	*mode = tmp_mode;
#endif

	LOGI("returns : session=%p, mode=%d, ret=%p", session, *mode, ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_session_interrupted_cb (sound_session_interrupted_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	if(callback == NULL)
		return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	if(g_session_interrupt_cb_table.is_registered ==0){
		ret = mm_session_init_ex(SOUND_SESSION_TYPE_DEFAULT /*default*/ , _session_interrupt_cb, NULL);
		if(ret != 0)
			return __convert_sound_manager_error_code(__func__, ret);
		g_session_interrupt_cb_table.is_registered = 1;
	}

	g_session_interrupt_cb_table.user_cb = (sound_session_interrupted_cb)callback;
	g_session_interrupt_cb_table.user_data = user_data;
	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_unset_session_interrupted_cb (void)
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

int sound_manager_get_current_device_list (sound_device_mask_e device_mask, sound_device_list_h *device_list)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_current_device_list((mm_sound_device_flags_e)device_mask, device_list);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_next_device (sound_device_list_h device_list, sound_device_h *device)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_next_device(device_list, device);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_prev_device (sound_device_list_h device_list, sound_device_h *device)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_prev_device(device_list, device);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_type (sound_device_h device, sound_device_type_e *type)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_type(device, (mm_sound_device_type_e*)type);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_io_direction (sound_device_h device, sound_device_io_direction_e *io_direction)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_io_direction(device, (mm_sound_device_io_direction_e*)io_direction);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_id (sound_device_h device, int *id)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_id(device, id);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_name (sound_device_h device, char **name)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_name(device, name);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_state (sound_device_h device, sound_device_state_e *state)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_state(device, (mm_sound_device_state_e*)state);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_device_connected_cb (sound_device_mask_e device_mask, sound_device_connected_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_add_device_connected_callback((mm_sound_device_flags_e)device_mask, (mm_sound_device_connected_cb)callback, user_data);
	if (ret == MM_ERROR_NONE) {
		g_device_connected_cb_table.user_cb = (sound_device_connected_cb)callback;
		g_device_connected_cb_table.user_data = user_data;
	}

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_device_connected_cb (void)
{
	int ret = MM_ERROR_NONE;
	if (g_device_connected_cb_table.user_cb) {
		ret = mm_sound_remove_device_connected_callback();
		if (ret == MM_ERROR_NONE) {
			g_device_connected_cb_table.user_cb = NULL;
			g_device_connected_cb_table.user_data = NULL;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_device_information_changed_cb (sound_device_mask_e device_mask, sound_device_information_changed_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_add_device_information_changed_callback((mm_sound_device_flags_e)device_mask, (mm_sound_device_info_changed_cb)callback, user_data);
	if (ret == MM_ERROR_NONE) {
		g_device_info_changed_cb_table.user_cb = (sound_device_information_changed_cb)callback;
		g_device_info_changed_cb_table.user_data = user_data;
	}

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_device_information_changed_cb (void)
{
	int ret = MM_ERROR_NONE;
	if (g_device_info_changed_cb_table.user_cb) {
		ret = mm_sound_remove_device_information_changed_callback();
		if (ret == MM_ERROR_NONE) {
			g_device_info_changed_cb_table.user_cb = NULL;
			g_device_info_changed_cb_table.user_data = NULL;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

	return __convert_sound_manager_error_code(__func__, ret);
}

__attribute__ ((destructor))
void __sound_manager_finalize (void)
{
	int ret = MM_ERROR_NONE;

	if(g_session_interrupt_cb_table.is_registered){
		LOGI("<ENTER>");
		ret = mm_session_finish();
		if (ret != MM_ERROR_NONE) {
			LOGE("[%s] failed to mm_session_finish(), ret(%p)", __func__, ret);
		}
		g_session_interrupt_cb_table.is_registered = 0;
		LOGI("<LEAVE>");
	}
}

__attribute__ ((constructor))
void __sound_manager_initialize (void)
{

}
