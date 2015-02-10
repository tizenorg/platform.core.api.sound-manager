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

#include <sound_manager.h>
#include <sound_manager_private.h>
#include <sound_manager_internal.h>
#include <mm_sound.h>

int sound_manager_create_stream_information_internal (sound_stream_type_internal_e stream_type, sound_stream_focus_state_changed_cb callback, void *user_data, sound_stream_info_h *stream_info)
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
		ret = __convert_stream_type_for_internal(stream_type, stream_h->stream_type);
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

int sound_manager_add_option_for_stream_routing (sound_stream_info_h stream_info, const char *option)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	bool added_successfully = false;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(option);

	for (i = 0; i < ROUTE_OPTIONS_MAX; i++) {
		if (stream_h->route_options[i]) {
			if (!strncmp (stream_h->route_options[i], option, strlen(option))) {
				return __convert_sound_manager_error_code(__func__, MM_ERROR_INVALID_ARGUMENT);
			}
			continue;
		} else {
			stream_h->route_options[i] = strdup(option);
			added_successfully = true;
			break;
		}
	}

	if (!added_successfully) {
		ret = MM_ERROR_SOUND_INTERNAL;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_remove_option_for_stream_routing (sound_stream_info_h stream_info, const char *option)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	bool removed_successfully = false;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(option);

	for (i = 0; i < ROUTE_OPTIONS_MAX; i++) {
		if (stream_h->route_options[i] && !strncmp (stream_h->route_options[i], option, strlen(option))) {
			free(stream_h->route_options[i]);
			stream_h->route_options[i] = NULL;
			removed_successfully = true;
			break;
		}
	}

	if (!removed_successfully) {
		ret = MM_ERROR_INVALID_ARGUMENT;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_apply_stream_routing_options (sound_stream_info_h stream_info)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	bool need_to_apply = false;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);

	for (i = 0; i < ROUTE_OPTIONS_MAX; i++) {
		if (stream_h->route_options[i]) {
			need_to_apply = true;
			break;
		}
	}
	if (need_to_apply) {
		ret = __set_route_options(stream_h->index, stream_h->route_options);
	} else {
		__convert_sound_manager_error_code(__func__, MM_ERROR_SOUND_INTERNAL);
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_is_available_stream_information (sound_stream_info_h stream_info, native_api_e api_name, bool *is_available)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	const char *name = NULL;
	sound_stream_info_s* stream_h = (sound_stream_info_s*)stream_info;

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(is_available);
	*is_available = false;

	name = __convert_api_name(api_name);
	for (i = 0; i < AVAIL_FRAMEWORKS_MAX; i++) {
		if (stream_h->stream_conf_info.avail_frameworks[i] && !strncmp(stream_h->stream_conf_info.avail_frameworks[i], name, strlen(name))) {
			*is_available = true;
		} else {
			break;
		}
	}
	LOGI("stream_type[%s], native api[%s], is_available[%d]", stream_h->stream_type, name, *is_available);
	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_type_from_stream_information (sound_stream_info_h stream_info, char **type)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s* stream_h = (sound_stream_info_s*)stream_info;

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(type);

	*type = stream_h->stream_type;
	LOGI("stream_type[%s]", *type);

	return __convert_sound_manager_error_code(__func__, ret);
}
int sound_manager_get_index_from_stream_information (sound_stream_info_h stream_info, int *index)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s* stream_h = (sound_stream_info_s*)stream_info;
	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(index);

	*index = stream_h->index;
	LOGI("stream_index[%d]", stream_h->index);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_create_virtual_stream (sound_stream_info_h stream_info, virtual_sound_stream_h *virtual_stream)
{
	int ret = MM_ERROR_NONE;
	bool result = false;
	virtual_sound_stream_info_s *vstream_h = NULL;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(virtual_stream);
	SM_INSTANCE_CHECK(stream_h);

	/* check if this stream_info is available for virtual stream */
	ret = sound_manager_is_available_stream_information(stream_info, NATIVE_API_SOUND_MANAGER, &result);
	if (ret == MM_ERROR_NONE && result == true) {
		vstream_h = malloc(sizeof(virtual_sound_stream_info_s));
		if(!vstream_h) {
			ret = MM_ERROR_OUT_OF_MEMORY;
		} else {
			memset(vstream_h, 0, sizeof(virtual_sound_stream_info_s));
			memcpy(vstream_h->stream_type, stream_h->stream_type, SOUND_STREAM_TYPE_LEN);
			vstream_h->pa_mainloop = stream_h->pa_mainloop;
			vstream_h->pa_context = stream_h->pa_context;
			vstream_h->pa_proplist = pa_proplist_new();
			pa_proplist_sets(vstream_h->pa_proplist, PA_PROP_MEDIA_ROLE, vstream_h->stream_type);
			pa_proplist_setf(vstream_h->pa_proplist, PA_PROP_MEDIA_PARENT_ID, "%u", stream_h->index);
			vstream_h->state = _VSTREAM_STATE_READY;
			vstream_h->stream_conf_info = &(stream_h->stream_conf_info);
			*virtual_stream = (virtual_sound_stream_h)vstream_h;
		}
	} else {
		ret = MM_ERROR_SOUND_NOT_SUPPORTED_OPERATION;
	}

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_destroy_virtual_stream (virtual_sound_stream_h virtual_stream)
{
	int ret = MM_ERROR_NONE;
	virtual_sound_stream_info_s *vstream_h = (virtual_sound_stream_info_s*)virtual_stream;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(vstream_h);
	SM_STATE_CHECK(vstream_h, _VSTREAM_STATE_READY);

	vstream_h->pa_mainloop = NULL;
	vstream_h->pa_context = NULL;
	if (vstream_h->pa_proplist)
		pa_proplist_free(vstream_h->pa_proplist);

	free(vstream_h);

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_start_virtual_stream (virtual_sound_stream_h virtual_stream)
{
	int ret = MM_ERROR_NONE;
	int pa_ret = PA_OK;
	int i = 0;
	int io_direction = 0;
	pa_sample_spec ss;
	pa_channel_map maps;

	virtual_sound_stream_info_s *vstream_h = (virtual_sound_stream_info_s*)virtual_stream;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(vstream_h);
	SM_STATE_CHECK(vstream_h, _VSTREAM_STATE_READY);

	/* check route-type(auto/manual) and error handling as per the type */
	if (vstream_h->stream_conf_info->route_type == STREAM_ROUTE_TYPE_MANUAL) {
		/* TODO : check if the device info. is set when it comes to the manual route type */
		/* if no, return error */
	}

	/* fill up with default value */
	ss.channels = 2;
	ss.rate = 44100;
	ss.format = PA_SAMPLE_S16LE;
	pa_channel_map_init_auto(&maps, ss.channels, PA_CHANNEL_MAP_ALSA);

	/* check direction of this stream */
	if (vstream_h->stream_conf_info->avail_in_devices[0] != NULL)
		io_direction |= SOUND_STREAM_DIRECTION_INPUT;
	if (vstream_h->stream_conf_info->avail_out_devices[0] != NULL)
		io_direction |= SOUND_STREAM_DIRECTION_OUTPUT;

	/* LOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_lock(vstream_h->pa_mainloop);

	for (i = 0; i < SOUND_STREAM_DIRECTION_MAX; i++) {
		if (io_direction & (i + 1)) {
			vstream_h->pa_stream[i] = pa_stream_new_with_proplist(vstream_h->pa_context, "VIRTUAL_STREAM", &ss, &maps, vstream_h->pa_proplist);
			if(vstream_h->pa_stream[i] == NULL) {
				LOGE("failed to pa_stream_new_with_proplist()");
				pa_ret = pa_context_errno(vstream_h->pa_context);
				goto ERROR_WITH_UNLOCK;
			}
			pa_stream_set_state_callback(vstream_h->pa_stream[i], _pa_stream_state_cb, vstream_h);

			if ((i + 1) == SOUND_STREAM_DIRECTION_OUTPUT) {
				pa_ret = pa_stream_connect_playback(vstream_h->pa_stream[i], NULL, NULL, 0, NULL, NULL);
				if (pa_ret < 0) {
					LOGE("failed to pa_stream_connect_playback()");
					pa_ret = pa_context_errno(vstream_h->pa_context);
					goto ERROR_WITH_UNLOCK;
				}
			} else if ((i + 1) == SOUND_STREAM_DIRECTION_INPUT) {
				pa_ret = pa_stream_connect_record(vstream_h->pa_stream[i], NULL, NULL, 0);
				if (pa_ret < 0) {
					LOGE("failed to pa_stream_connect_record()");
					pa_ret = pa_context_errno(vstream_h->pa_context);
					goto ERROR_WITH_UNLOCK;
				}
			}

			/* wait for ready state of the stream */
			for (;;) {
				pa_stream_state_t state;
				state = pa_stream_get_state(vstream_h->pa_stream[i]);
				if (state == PA_STREAM_READY) {
					break;
				}
				if (!PA_STREAM_IS_GOOD(state)) {
					pa_ret = pa_context_errno(vstream_h->pa_context);
				}
				pa_threaded_mainloop_wait(vstream_h->pa_mainloop);
			}
		}
	}
	vstream_h->state = _VSTREAM_STATE_RUNNING;

	/* UNLOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_unlock(vstream_h->pa_mainloop);
	goto SUCCESS;
ERROR_WITH_UNLOCK:
	/* UNLOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_unlock(vstream_h->pa_mainloop);

	for (i = 0; i < SOUND_STREAM_DIRECTION_MAX; i++) {
		if (vstream_h->pa_stream[i]) {
			pa_stream_unref(vstream_h->pa_stream[i]);
			vstream_h->pa_stream[i] = NULL;
		}
	}
	LOGE("pa_ret(%d)", pa_ret);
	ret = MM_ERROR_SOUND_INTERNAL;
SUCCESS:
	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_stop_virtual_stream (virtual_sound_stream_h virtual_stream)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	virtual_sound_stream_info_s *vstream_h = (virtual_sound_stream_info_s*)virtual_stream;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(vstream_h);
	SM_STATE_CHECK(vstream_h, _VSTREAM_STATE_RUNNING);

	/* LOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_lock(vstream_h->pa_mainloop);

	for (i = 0; i < SOUND_STREAM_DIRECTION_MAX; i++) {
		if (vstream_h->pa_stream[i]) {
			pa_stream_disconnect(vstream_h->pa_stream[i]);
			pa_stream_unref(vstream_h->pa_stream[i]);
			vstream_h->pa_stream[i] = NULL;
		}
	}

	/* UNLOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_unlock(vstream_h->pa_mainloop);

	vstream_h->state = _VSTREAM_STATE_READY;

	LOGI("<< leave : ret(%p)", ret);

	return __convert_sound_manager_error_code(__func__, ret);
}
