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


_session_interrupt_info_s g_session_interrupt_cb_table = {0, 0, NULL, NULL};
_volume_changed_info_s g_volume_changed_cb_table = {0, NULL, NULL};
_focus_watch_info_s g_focus_watch_cb_table = {-1, NULL, NULL};
_device_connected_info_s g_device_connected_cb_table = {0, NULL, NULL};
_device_changed_info_s g_device_info_changed_cb_table = {0, NULL, NULL};

sound_session_type_e g_cached_session = -1;
_session_mode_e g_cached_session_mode = -1;
int g_cached_voip_device_id = -1;
extern sound_stream_info_s *g_voip_stream_info;
extern virtual_sound_stream_info_s *g_voip_vstream_h;

/* These variables will be removed when session features are deprecated. */
extern int g_stream_info_count;
extern pthread_mutex_t g_stream_info_count_mutex;
pthread_mutex_t g_interrupt_cb_mutex, g_device_info_cb_mutex, g_device_conn_cb_mutex, g_volume_cb_mutex;

int sound_manager_get_max_volume(sound_type_e type, int *max)
{
	const char *volume_type = NULL;
	unsigned int max_level = 0;
	int ret = MM_ERROR_NONE;

	SM_NULL_ARG_CHECK(max);
	if (type >= SOUND_TYPE_NUM || type < 0)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = _convert_sound_type(type, &volume_type);
	if (ret == MM_ERROR_NONE) {
		ret = _get_volume_max_level(DIRECTION_OUT_STR, volume_type, &max_level);
		if (ret == MM_ERROR_NONE)
			*max = (int)max_level -1;	/* actual volume step can be max step - 1 */
	}

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume(sound_type_e type, int volume)
{
	int ret = MM_ERROR_NONE;

	if (type >= SOUND_TYPE_NUM || type < 0)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	if (volume < 0)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_sound_volume_set_value(type, volume);
	LOGI("type=%d, volume=%d", type, volume);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_volume(sound_type_e type, int *volume)
{
	int ret = MM_ERROR_NONE;
	unsigned int uvolume;

	if (type >= SOUND_TYPE_NUM || type < 0)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	if (volume == NULL)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_sound_volume_get_value(type, &uvolume);
	if (ret == MM_ERROR_NONE)
		*volume = uvolume;

	LOGI("type=%d, volume=%d", type, *volume);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_current_sound_type(sound_type_e type)
{
	int ret = MM_ERROR_NONE;

	if (type >= SOUND_TYPE_NUM || type < 0)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_sound_volume_primary_type_set(type);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_current_sound_type(sound_type_e *type)
{
	int ret = MM_ERROR_NONE;
	volume_type_t mm_sound_vol_type = VOLUME_TYPE_UNKNOWN;
	char *volume_type = NULL;

	if (type == NULL)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret  = mm_sound_volume_primary_type_get(&mm_sound_vol_type);
	if (ret == MM_ERROR_NONE) {
		if (mm_sound_vol_type == VOLUME_TYPE_UNKNOWN) {
			/* get the volume type of the current playing stream */
			ret = _get_current_volume_type(DIRECTION_OUT_STR, &volume_type);
			if (ret == MM_ERROR_NONE)
				ret = _convert_sound_type_to_enum((const char*)volume_type, type);
		} else {
			*type = mm_sound_vol_type;
		}
	}
	LOGI("type=%d", *type);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_current_sound_type(void)
{
	int ret = MM_ERROR_NONE;

	ret = mm_sound_volume_primary_type_set(VOLUME_TYPE_UNKNOWN);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_volume_changed_cb(sound_manager_volume_changed_cb callback, void* user_data)
{
	int ret = MM_ERROR_NONE;
	unsigned int subs_id = 0;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_volume_cb_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	ret = mm_sound_add_volume_changed_callback((mm_sound_volume_changed_cb)callback, user_data, &subs_id);
	if (ret == MM_ERROR_NONE) {
		g_volume_changed_cb_table.subs_id = subs_id;
		g_volume_changed_cb_table.user_cb = (sound_manager_volume_changed_cb)callback;
		g_volume_changed_cb_table.user_data = user_data;
	}

	SM_LEAVE_CRITICAL_SECTION(&g_volume_cb_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_volume_changed_cb(void)
{
	int ret = MM_ERROR_NONE;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_volume_cb_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	if (g_volume_changed_cb_table.subs_id > 0) {
		ret = mm_sound_remove_volume_changed_callback(g_volume_changed_cb_table.subs_id);
		if (ret == MM_ERROR_NONE) {
			g_volume_changed_cb_table.subs_id = 0;
			g_volume_changed_cb_table.user_cb = NULL;
			g_volume_changed_cb_table.user_data = NULL;
		}
	} else {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

	SM_LEAVE_CRITICAL_SECTION(&g_volume_cb_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_create_stream_information(sound_stream_type_e stream_type, sound_stream_focus_state_changed_cb callback, void *user_data, sound_stream_info_h *stream_info)
{
	int ret = MM_ERROR_NONE;

	LOGI(">> enter");

	SM_NULL_ARG_CHECK(stream_info);
	SM_NULL_ARG_CHECK(callback);

	if (g_session_interrupt_cb_table.is_registered)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_SOUND_INTERNAL);

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_stream_info_count_mutex, MM_ERROR_SOUND_INTERNAL);

	sound_stream_info_s *stream_h = malloc(sizeof(sound_stream_info_s));
	if (!stream_h) {
		ret = MM_ERROR_OUT_OF_MEMORY;
		goto LEAVE;
	}

	memset(stream_h, 0, sizeof(sound_stream_info_s));
	ret = _convert_stream_type(stream_type, &stream_h->stream_type);
	if (ret == MM_ERROR_NONE) {
		_set_focus_availability(stream_h);
		ret = _make_pa_connection_and_register_focus(stream_h, callback, user_data);
		if (ret == MM_ERROR_NONE) {
			*stream_info = (sound_stream_info_h)stream_h;
			SM_REF_FOR_STREAM_INFO(g_stream_info_count, ret);
			LOGI("stream_h(%p), index(%u), user_cb(%p), cnt(%d), ret(0x%x)", stream_h, stream_h->index, stream_h->user_cb, g_stream_info_count, ret);
		}
	}

LEAVE:
	if (ret && stream_h)
		free(stream_h);

	SM_LEAVE_CRITICAL_SECTION(&g_stream_info_count_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_destroy_stream_information(sound_stream_info_h stream_info)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_stream_info_count_mutex, MM_ERROR_SOUND_INTERNAL);
	ret = _destroy_pa_connection_and_unregister_focus(stream_h);
	if (ret == MM_ERROR_NONE) {
		free(stream_h);
		stream_h = NULL;
		SM_UNREF_FOR_STREAM_INFO(g_stream_info_count, ret);
	}
	SM_LEAVE_CRITICAL_SECTION(&g_stream_info_count_mutex);

	LOGD("cnt(%d)", g_stream_info_count);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_add_device_for_stream_routing(sound_stream_info_h stream_info, sound_device_h device)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	ret = _add_device_for_stream_routing(stream_h, device);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_remove_device_for_stream_routing(sound_stream_info_h stream_info, sound_device_h device)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	ret = _remove_device_for_stream_routing(stream_h, device);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_apply_stream_routing(sound_stream_info_h stream_info)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	ret = _apply_stream_routing(stream_h);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_focus_reacquisition(sound_stream_info_h stream_info, bool enable)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	ret = mm_sound_set_focus_reacquisition(stream_h->index, enable);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_focus_reacquisition(sound_stream_info_h stream_info, bool *enabled)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(enabled);

	ret = mm_sound_get_focus_reacquisition(stream_h->index, enabled);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_acquire_focus(sound_stream_info_h stream_info, sound_stream_focus_mask_e focus_mask, const char *additional_info)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	if (stream_h->is_focus_unavailable)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);

	ret = mm_sound_acquire_focus(stream_h->index, (mm_sound_focus_type_e)focus_mask, additional_info);
	if (ret == MM_ERROR_NONE) {
		stream_h->acquired_focus |= focus_mask;
		_update_focus_status(stream_h->index, (unsigned int)stream_h->acquired_focus);
	}

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_release_focus(sound_stream_info_h stream_info, sound_stream_focus_mask_e focus_mask, const char *additional_info)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	ret = mm_sound_release_focus(stream_h->index, (mm_sound_focus_type_e)focus_mask, additional_info);
	if (ret == MM_ERROR_NONE) {
		stream_h->acquired_focus &= ~focus_mask;
		_update_focus_status(stream_h->index, (unsigned int)stream_h->acquired_focus);
	}

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_focus_state(sound_stream_info_h stream_info, sound_stream_focus_state_e *state_for_playback, sound_stream_focus_state_e *state_for_recording)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	if (!state_for_playback && !state_for_recording)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	if (state_for_playback)
		*state_for_playback = ((stream_h->acquired_focus & SOUND_STREAM_FOCUS_FOR_PLAYBACK) ? (SOUND_STREAM_FOCUS_STATE_ACQUIRED) : (SOUND_STREAM_FOCUS_STATE_RELEASED));
	if (state_for_recording)
		*state_for_recording = ((stream_h->acquired_focus & SOUND_STREAM_FOCUS_FOR_RECORDING) ? (SOUND_STREAM_FOCUS_STATE_ACQUIRED) : (SOUND_STREAM_FOCUS_STATE_RELEASED));

	LOGI("acquired_focus(0x%x)", stream_h->acquired_focus);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_sound_type(sound_stream_info_h stream_info, sound_type_e *sound_type)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(sound_type);

	if (stream_h->stream_conf_info.volume_type == NULL)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_SOUND_NO_DATA);

	ret = _convert_sound_type_to_enum(stream_h->stream_conf_info.volume_type, sound_type);
	LOGI("sound type(%d)", *sound_type);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_focus_state_watch_cb(sound_stream_focus_mask_e focus_mask, sound_stream_focus_state_watch_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	int id = -1;

	LOGI(">> enter");

	SM_NULL_ARG_CHECK(callback);
	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_stream_info_count_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	if (g_focus_watch_cb_table.user_cb) {
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	SM_REF_FOR_STREAM_INFO(g_stream_info_count, ret);
	ret = mm_sound_set_focus_watch_callback((mm_sound_focus_type_e)focus_mask, _focus_watch_callback, user_data, &id);
	if (ret == MM_ERROR_NONE) {
		g_focus_watch_cb_table.index = id;
		g_focus_watch_cb_table.user_cb = callback;
		g_focus_watch_cb_table.user_data = user_data;
	}

LEAVE:
	SM_LEAVE_CRITICAL_SECTION(&g_stream_info_count_mutex);

	LOGD("cnt(%d)", g_stream_info_count);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_focus_state_watch_cb(void)
{
	int ret = MM_ERROR_NONE;

	LOGI(">> enter");

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_stream_info_count_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	if (!g_focus_watch_cb_table.user_cb) {
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	ret = mm_sound_unset_focus_watch_callback(g_focus_watch_cb_table.index);
	if (ret != MM_ERROR_NONE) {
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	g_focus_watch_cb_table.index = -1;
	g_focus_watch_cb_table.user_cb = NULL;
	g_focus_watch_cb_table.user_data = NULL;
	SM_UNREF_FOR_STREAM_INFO(g_stream_info_count, ret);

LEAVE:
	SM_LEAVE_CRITICAL_SECTION(&g_stream_info_count_mutex);

	LOGD("cnt(%d)", g_stream_info_count);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_session_type(sound_session_type_e type)
{
	int ret = MM_ERROR_NONE;
	int cur_session = -1;
	int new_session = MM_SESSION_TYPE_MEDIA;

	LOGI(">> enter : type=%d", type);

	if (type < SOUND_SESSION_TYPE_MEDIA || type >  SOUND_SESSION_TYPE_VOIP)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	/* it is not supported both session and stream feature at the same time */
	if (g_stream_info_count) {
		LOGE("Could not set this type(%d) because of being used stream feature", type);
		return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
	}

	switch (type) {
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
	if (ret == MM_ERROR_NONE) {
		if (cur_session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (type > SOUND_SESSION_TYPE_MEDIA) {
				LOGE("Could not set this type(%d) during camera/recorder/audio-io(in)/radio", type);
				return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		}
	}

	if (g_session_interrupt_cb_table.is_registered) {
		if (new_session == cur_session ||
			((new_session == SOUND_SESSION_TYPE_MEDIA) && (cur_session == MM_SESSION_TYPE_MEDIA_RECORD))) {
			LOGI("already set type=%d, ret=0x%x", type, ret);
			return SOUND_MANAGER_ERROR_NONE;
		} else {
			ret = mm_session_finish();
			if (ret != MM_ERROR_NONE)
				return _convert_sound_manager_error_code(__func__, ret);

			g_session_interrupt_cb_table.is_registered = 0;
			if (cur_session == MM_SESSION_TYPE_VOIP) {
				g_cached_session_mode = -1;
				g_cached_voip_device_id = -1;
				if (g_voip_vstream_h) {
					_stop_virtual_stream(g_voip_vstream_h);
					_destroy_virtual_stream(g_voip_vstream_h);
					g_voip_vstream_h = NULL;
				}
				/*voip stream destruction*/
				if (g_voip_stream_info) {
					ret = _destroy_pa_connection_and_unregister_focus(g_voip_stream_info);
					free(g_voip_stream_info);
					g_voip_stream_info = NULL;
					if (ret != MM_ERROR_NONE)
						return _convert_sound_manager_error_code(__func__, ret);
				}
			}
		}
	}
	ret = mm_session_init(new_session);
	if (ret == MM_ERROR_NONE)
		g_session_interrupt_cb_table.is_registered = 1;

	LOGI("type=%d", type);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_session_type(sound_session_type_e *type)
{
	int ret = MM_ERROR_NONE;
	int cur_session;

	if (type == NULL)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_session_get_current_type(&cur_session);
	if (ret != MM_ERROR_NONE) {
		LOGW("session hasn't been set, setting default session");
		cur_session = SOUND_SESSION_TYPE_DEFAULT;
		ret = mm_session_init(cur_session);
		if (ret == MM_ERROR_NONE)
			g_session_interrupt_cb_table.is_registered = 1;
	}
	if ((cur_session > MM_SESSION_TYPE_EMERGENCY) &&
			(cur_session != MM_SESSION_TYPE_VOIP)) {
		if (g_cached_session != -1)
			cur_session = g_cached_session;
		else /* will be never reached here. just prevent code */
			cur_session = SOUND_SESSION_TYPE_DEFAULT;
	}

	switch (cur_session) {
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

	LOGI("type=%d", *type);

	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_set_media_session_option(sound_session_option_for_starting_e s_option, sound_session_option_for_during_play_e d_option)
{
	int ret = MM_ERROR_NONE;
	int session = 0;
	int session_option = 0;
	int updated = 0;

	LOGI(">> enter : option for starting=%d, for during play=%d", s_option, d_option);

	if (s_option < 0 || s_option >  SOUND_SESSION_OPTION_PAUSE_OTHERS_WHEN_START)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
	if (d_option < 0 || d_option >  SOUND_SESSION_OPTION_UNINTERRUPTIBLE_DURING_PLAY)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_session_get_current_information(&session, &session_option);
	if (ret != 0 || !g_session_interrupt_cb_table.is_registered) {
		LOGW("session hasn't been set, setting default session");
		ret = mm_session_init(MM_SESSION_TYPE_MEDIA);
		if (ret == 0)
			g_session_interrupt_cb_table.is_registered = 1;

	} else if (ret == 0 && session > MM_SESSION_TYPE_MEDIA) {
		if (session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (!g_session_interrupt_cb_table.is_registered) {
				LOGE("Already set by camera/recorder/audio-io(in)/radio API, but need to set session to Media first");
				return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		} else {
			return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
		}
	}

	switch (s_option) {
	case SOUND_SESSION_OPTION_MIX_WITH_OTHERS_WHEN_START:
		if (session_option & MM_SESSION_OPTION_PAUSE_OTHERS) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_REMOVE, MM_SESSION_OPTION_PAUSE_OTHERS);
			if (ret)
				return _convert_sound_manager_error_code(__func__, ret);

			updated = 1;
		}
		break;
	case SOUND_SESSION_OPTION_PAUSE_OTHERS_WHEN_START:
		if (!(session_option & MM_SESSION_OPTION_PAUSE_OTHERS)) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_ADD, MM_SESSION_OPTION_PAUSE_OTHERS);
			if (ret)
				return _convert_sound_manager_error_code(__func__, ret);

			updated = 1;
		}
		break;
	}

	switch (d_option) {
	case SOUND_SESSION_OPTION_INTERRUPTIBLE_DURING_PLAY:
		if (session_option & MM_SESSION_OPTION_UNINTERRUPTIBLE) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_REMOVE, MM_SESSION_OPTION_UNINTERRUPTIBLE);
			if (ret)
				return _convert_sound_manager_error_code(__func__, ret);

			updated = 1;
		}
		break;
	case SOUND_SESSION_OPTION_UNINTERRUPTIBLE_DURING_PLAY:
		if (!(session_option & MM_SESSION_OPTION_UNINTERRUPTIBLE)) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_ADD, MM_SESSION_OPTION_UNINTERRUPTIBLE);
			if (ret)
				return _convert_sound_manager_error_code(__func__, ret);

			updated = 1;
		}
		break;
	}

	if (updated)
		LOGI("updated");
	else
		LOGI("already set same option(%x), skip it", session_option);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_media_session_option(sound_session_option_for_starting_e *s_option, sound_session_option_for_during_play_e *d_option)
{
	int ret = MM_ERROR_NONE;
	int session = 0;
	int session_options = 0;

	LOGI(">> enter");

	if (s_option == NULL || d_option == NULL)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_session_get_current_information(&session, &session_options);
	if (ret != 0) {
		ret = mm_session_init(MM_SESSION_TYPE_MEDIA);
		if (ret == 0)
			g_session_interrupt_cb_table.is_registered = 1;

	} else if (session > SOUND_SESSION_TYPE_MEDIA) {
		if (session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (!g_session_interrupt_cb_table.is_registered) {
				LOGE("Already set by camera/recorder/audio-io(in)/radio API, but need to set session to Media first");
				return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		} else
			return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
	}
	/* get option */
	if (session_options & MM_SESSION_OPTION_PAUSE_OTHERS)
		*s_option = SOUND_SESSION_OPTION_PAUSE_OTHERS_WHEN_START;
	else
		*s_option = SOUND_SESSION_OPTION_MIX_WITH_OTHERS_WHEN_START;

	if (session_options & MM_SESSION_OPTION_UNINTERRUPTIBLE)
		*d_option = SOUND_SESSION_OPTION_UNINTERRUPTIBLE_DURING_PLAY;
	else
		*d_option = SOUND_SESSION_OPTION_INTERRUPTIBLE_DURING_PLAY;

	LOGI(" option for starting=%d, for during play=%d", *s_option, *d_option);

	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_set_media_session_resumption_option(sound_session_option_for_resumption_e option)
{
	int ret = MM_ERROR_NONE;
	int session = 0;
	int session_option = 0;
	int updated = 0;

	LOGI(">> enter : option for resumption=%d (0:by system, 1:by system or media paused)", option);

	if (option < SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM || option > SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM_OR_MEDIA_PAUSED)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_session_get_current_information(&session, &session_option);
	if (ret != 0 || !g_session_interrupt_cb_table.is_registered) {
		LOGW("session hasn't been set, setting default session");
		ret = mm_session_init(MM_SESSION_TYPE_MEDIA);
		if (ret == 0)
			g_session_interrupt_cb_table.is_registered = 1;

	} else if (ret == 0 && session > MM_SESSION_TYPE_MEDIA) {
		if (session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (!g_session_interrupt_cb_table.is_registered) {
				LOGE("Already set by camera/recorder/audio-io(in)/radio API, but need to set session to Media first");
				return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		} else
			return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);

	}

	switch (option) {
	case SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM:
		if (session_option & MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_REMOVE, MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED);
			if (ret)
				return _convert_sound_manager_error_code(__func__, ret);

			updated = 1;
		}
		break;
	case SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM_OR_MEDIA_PAUSED:
		if (!(session_option & MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED)) {
			ret = mm_session_update_option(MM_SESSION_UPDATE_TYPE_ADD, MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED);
			if (ret)
				return _convert_sound_manager_error_code(__func__, ret);

			updated = 1;
		}
		break;
	}

	if (updated)
		LOGI("updated");
	else
		LOGI("already set same option(0x%x), skip it", session_option);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_media_session_resumption_option(sound_session_option_for_resumption_e *option)
{
	int ret = MM_ERROR_NONE;
	int session = 0;
	int session_options = 0;

	LOGI(">> enter");

	if (option == NULL)
		return _convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);

	ret = mm_session_get_current_information(&session, &session_options);
	if (ret != 0) {
		LOGW("session hasn't been set, setting default session");
		ret = mm_session_init(MM_SESSION_TYPE_MEDIA);
		if (ret == 0)
			g_session_interrupt_cb_table.is_registered = 1;

	} else if (session > SOUND_SESSION_TYPE_MEDIA) {
		if (session == MM_SESSION_TYPE_MEDIA_RECORD) {
			if (!g_session_interrupt_cb_table.is_registered) {
				LOGE("Already set by camera/recorder/audio-io(in)/radio API, but need to set session to Media first");
				return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
			}
		} else
			return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
	}
	/* get option */
	if (session_options & MM_SESSION_OPTION_RESUME_BY_SYSTEM_OR_MEDIA_PAUSED)
		*option = SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM_OR_MEDIA_PAUSED;
	else
		*option = SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM;

	LOGI("option for resumption=%d (0:by system, 1:by system or media paused)", *option);

	return SOUND_MANAGER_ERROR_NONE;
}

int sound_manager_set_voip_session_mode(sound_session_voip_mode_e mode)
{
	int ret = MM_ERROR_NONE;
	int session = 0;
	int session_options = 0;

	LOGI(">> enter : mode=%d", mode);

	ret = mm_session_get_current_information(&session, &session_options);
	if (ret != MM_ERROR_NONE)
		goto LEAVE;

	if (session != MM_SESSION_TYPE_VOIP) {
		ret = MM_ERROR_POLICY_INTERNAL;
		goto LEAVE;
	}

	if (mode < SOUND_SESSION_VOIP_MODE_RINGTONE || mode > SOUND_SESSION_VOIP_MODE_VOICE_WITH_BLUETOOTH) {
		ret = MM_ERROR_INVALID_ARGUMENT;
		goto LEAVE;
	}

	ret = _set_session_mode((_session_mode_e)mode);

	LOGI("session=%d, mode=%d", session, mode);

LEAVE:
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_voip_session_mode(sound_session_voip_mode_e *mode)
{
	int ret = MM_ERROR_NONE;
	int session = 0;
	int session_options = 0;

	if (mode == NULL) {
		LOGE("mode is null");
		ret = MM_ERROR_INVALID_ARGUMENT;
		goto LEAVE;
	}

	ret = mm_session_get_current_information(&session, &session_options);
	if (ret != MM_ERROR_NONE) {
		LOGI("session = %d, option = %d", session, session_options);
		goto LEAVE;
	}

	if (session != MM_SESSION_TYPE_VOIP || g_cached_session_mode == -1) {
		ret = MM_ERROR_POLICY_INTERNAL;
		goto LEAVE;
	}

	*mode = (sound_session_voip_mode_e)g_cached_session_mode;

	LOGI("session=%d, mode=%d", session, *mode);

LEAVE:
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_session_interrupted_cb(sound_session_interrupted_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	unsigned int subs_id = 0;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_interrupt_cb_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	if (callback == NULL) {
		ret = MM_ERROR_INVALID_ARGUMENT;
		goto LEAVE;
	}

	/* it is not supported both session and stream feature at the same time */
	if (g_stream_info_count) {
		ret =  MM_ERROR_POLICY_INTERNAL;
		goto LEAVE;
	}

	if (g_session_interrupt_cb_table.user_cb == NULL) {
		ret = mm_sound_add_device_connected_callback(SOUND_DEVICE_ALL_MASK, (mm_sound_device_connected_cb)_device_connected_cb, NULL, &subs_id);
		if (ret)
			goto LEAVE;
		ret = mm_sound_focus_set_session_interrupt_callback((mm_sound_focus_session_interrupt_cb)_focus_session_interrupt_cb, NULL);
		if (ret) {
			if (mm_sound_remove_device_connected_callback(subs_id) != MM_ERROR_NONE)
				LOGW("mm_sound_remove_device_connected_callback failed");
			goto LEAVE;
		}
		g_session_interrupt_cb_table.subs_id = subs_id;
	}
	g_session_interrupt_cb_table.user_cb = (sound_session_interrupted_cb)callback;
	g_session_interrupt_cb_table.user_data = user_data;

LEAVE:
	SM_LEAVE_CRITICAL_SECTION(&g_interrupt_cb_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_session_interrupted_cb(void)
{
	int ret = MM_ERROR_NONE;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_interrupt_cb_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	if (!g_session_interrupt_cb_table.user_cb) {
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	ret = mm_sound_focus_unset_session_interrupt_callback();
	if (ret) {
		if (mm_sound_remove_device_connected_callback(g_session_interrupt_cb_table.subs_id) != MM_ERROR_NONE)
			LOGW("mm_sound_remove_device_connected_callback failed");
		goto LEAVE;
	}
	ret = mm_sound_remove_device_connected_callback(g_session_interrupt_cb_table.subs_id);
	if (ret)
		goto LEAVE;

	g_session_interrupt_cb_table.subs_id = 0;
	g_session_interrupt_cb_table.user_cb = NULL;
	g_session_interrupt_cb_table.user_data = NULL;

LEAVE:
	SM_LEAVE_CRITICAL_SECTION(&g_interrupt_cb_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_current_device_list(sound_device_mask_e device_mask, sound_device_list_h *device_list)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_current_device_list((mm_sound_device_flags_e)device_mask, device_list);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_free_device_list(sound_device_list_h device_list)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_free_device_list(device_list);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_next_device(sound_device_list_h device_list, sound_device_h *device)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_next_device(device_list, device);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_prev_device(sound_device_list_h device_list, sound_device_h *device)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_prev_device(device_list, device);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_type(sound_device_h device, sound_device_type_e *type)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_type(device, (mm_sound_device_type_e*)type);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_io_direction(sound_device_h device, sound_device_io_direction_e *io_direction)
{
	int ret = MM_ERROR_NONE;
	mm_sound_device_io_direction_e mm_sound_io_direction;
	ret = mm_sound_get_device_io_direction(device, &mm_sound_io_direction);
	if (ret == MM_ERROR_NONE)
		ret = _convert_device_io_direction(mm_sound_io_direction, io_direction);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_id(sound_device_h device, int *id)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_id(device, id);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_name(sound_device_h device, char **name)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_name(device, name);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_device_state(sound_device_h device, sound_device_state_e *state)
{
	int ret = MM_ERROR_NONE;
	ret = mm_sound_get_device_state(device, (mm_sound_device_state_e*)state);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_device_connected_cb(sound_device_mask_e device_mask, sound_device_connected_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	unsigned int subs_id = 0;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_device_conn_cb_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	ret = mm_sound_add_device_connected_callback((mm_sound_device_flags_e)device_mask, (mm_sound_device_connected_cb)callback, user_data, &subs_id);
	if (ret == MM_ERROR_NONE) {
		g_device_connected_cb_table.subs_id = subs_id;
		g_device_connected_cb_table.user_cb = (sound_device_connected_cb)callback;
		g_device_connected_cb_table.user_data = user_data;
	}

	SM_LEAVE_CRITICAL_SECTION(&g_device_conn_cb_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_device_connected_cb(void)
{
	int ret = MM_ERROR_NONE;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_device_conn_cb_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	if (g_device_connected_cb_table.subs_id == 0) {
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	ret = mm_sound_remove_device_connected_callback(g_device_connected_cb_table.subs_id);
	if (ret == MM_ERROR_NONE) {
		g_device_connected_cb_table.subs_id = 0;
		g_device_connected_cb_table.user_cb = NULL;
		g_device_connected_cb_table.user_data = NULL;
	}

LEAVE:
	SM_LEAVE_CRITICAL_SECTION(&g_device_conn_cb_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_device_information_changed_cb(sound_device_mask_e device_mask, sound_device_information_changed_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	unsigned int subs_id = 0;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_device_info_cb_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	ret = mm_sound_add_device_information_changed_callback((mm_sound_device_flags_e)device_mask, (mm_sound_device_info_changed_cb)callback, user_data, &subs_id);
	if (ret == MM_ERROR_NONE) {
		g_device_info_changed_cb_table.subs_id = subs_id;
		g_device_info_changed_cb_table.user_cb = (sound_device_information_changed_cb)callback;
		g_device_info_changed_cb_table.user_data = user_data;
	}

	SM_LEAVE_CRITICAL_SECTION(&g_device_info_cb_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_unset_device_information_changed_cb(void)
{
	int ret = MM_ERROR_NONE;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN(&g_device_info_cb_mutex, SOUND_MANAGER_ERROR_INTERNAL);

	if (g_device_info_changed_cb_table.subs_id) {
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	ret = mm_sound_remove_device_information_changed_callback(g_device_info_changed_cb_table.subs_id);
	if (ret == MM_ERROR_NONE) {
		g_device_info_changed_cb_table.subs_id = 0;
		g_device_info_changed_cb_table.user_cb = NULL;
		g_device_info_changed_cb_table.user_data = NULL;
	}

LEAVE:
	SM_LEAVE_CRITICAL_SECTION(&g_device_info_cb_mutex);

	return _convert_sound_manager_error_code(__func__, ret);
}

__attribute__ ((destructor))
void __sound_manager_finalize(void)
{
	int ret = MM_ERROR_NONE;

	if (g_session_interrupt_cb_table.is_registered) {
		LOGI("<ENTER>");
		ret = mm_session_finish();
		if (ret != MM_ERROR_NONE)
			LOGE("[%s] failed to mm_session_finish(), ret(0x%x)", __func__, ret);

		g_session_interrupt_cb_table.is_registered = 0;
		LOGI("<LEAVE>");
	}
}

__attribute__ ((constructor))
void __sound_manager_initialize(void)
{

}
