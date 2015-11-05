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

int sound_manager_get_master_max_volume(int *max_level)
{
	int ret = MM_ERROR_NONE;
	unsigned int volume_level = 0;

	LOGI(">> enter");

	SM_NULL_ARG_CHECK(max_level);

	ret = _get_volume_max_level("out", "master", &volume_level);
	if (ret == MM_ERROR_NONE)
		*max_level = (int)volume_level;

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_master_volume(int level)
{
	int ret = MM_ERROR_NONE;

	LOGI(">> enter");

	ret = _set_volume_level("out", "master", (unsigned int)level);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_master_volume(int *level)
{
	int ret = MM_ERROR_NONE;
	unsigned int volume_level = 0;

	LOGI(">> enter");

	SM_NULL_ARG_CHECK(level);

	ret = _get_volume_level("out", "master", &volume_level);
	if (ret == MM_ERROR_NONE)
		*level = (int)volume_level;

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_create_stream_information_internal(sound_stream_type_internal_e stream_type, sound_stream_focus_state_changed_cb callback, void *user_data, sound_stream_info_h *stream_info)
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
		ret = _convert_stream_type_for_internal(stream_type, &stream_h->stream_type);
		if (ret == MM_ERROR_NONE) {
			ret = _make_pa_connection_and_register_focus(stream_h, callback, user_data);
			if (!ret) {
				*stream_info = (sound_stream_info_h)stream_h;
				LOGI("<< leave : stream_h(%p), index(%u), user_cb(%p), ret(%p)", stream_h, stream_h->index, stream_h->user_cb, ret);
			}
		}
		if (ret)
			free(stream_h);
	}

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_set_stream_routing_option(sound_stream_info_h stream_info, const char *name, int value)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(name);

	ret = _set_route_option(stream_h->index, name, value);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_is_available_stream_information(sound_stream_info_h stream_info, native_api_e api_name, bool *is_available)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	const char *name = NULL;
	sound_stream_info_s* stream_h = (sound_stream_info_s*)stream_info;

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(is_available);
	*is_available = false;

	name = _convert_api_name(api_name);
	for (i = 0; i < AVAIL_FRAMEWORKS_MAX; i++) {
		if (stream_h->stream_conf_info.avail_frameworks[i] && !strncmp(stream_h->stream_conf_info.avail_frameworks[i], name, strlen(name))) {
			*is_available = true;
			break;
		}
	}
	LOGI("stream_type[%s], native api[%s], is_available[%d]", stream_h->stream_type, name, *is_available);
	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_get_type_from_stream_information(sound_stream_info_h stream_info, char **type)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s* stream_h = (sound_stream_info_s*)stream_info;

	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(type);

	*type = stream_h->stream_type;
	LOGI("stream_type[%s]", *type);

	return _convert_sound_manager_error_code(__func__, ret);
}
int sound_manager_get_index_from_stream_information(sound_stream_info_h stream_info, int *index)
{
	int ret = MM_ERROR_NONE;
	sound_stream_info_s* stream_h = (sound_stream_info_s*)stream_info;
	SM_INSTANCE_CHECK(stream_h);
	SM_NULL_ARG_CHECK(index);

	*index = stream_h->index;
	LOGI("stream_index[%d]", stream_h->index);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_create_virtual_stream(sound_stream_info_h stream_info, virtual_sound_stream_h *virtual_stream)
{
	int ret = MM_ERROR_NONE;
	virtual_sound_stream_info_s *vstream_h = NULL;
	sound_stream_info_s *stream_h = (sound_stream_info_s*)stream_info;

	LOGI(">> enter");

	ret = _create_virtual_stream(stream_h, &vstream_h);
	if (ret == MM_ERROR_NONE)
		*virtual_stream = (virtual_sound_stream_h)vstream_h;

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_destroy_virtual_stream(virtual_sound_stream_h virtual_stream)
{
	int ret = MM_ERROR_NONE;
	virtual_sound_stream_info_s *vstream_h = (virtual_sound_stream_info_s*)virtual_stream;

	LOGI(">> enter");

	ret = _destroy_virtual_stream(vstream_h);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_start_virtual_stream(virtual_sound_stream_h virtual_stream)
{
	int ret = MM_ERROR_NONE;
	virtual_sound_stream_info_s *vstream_h = (virtual_sound_stream_info_s*)virtual_stream;

	LOGI(">> enter");

	ret = _start_virtual_stream(vstream_h);

	return _convert_sound_manager_error_code(__func__, ret);
}

int sound_manager_stop_virtual_stream(virtual_sound_stream_h virtual_stream)
{
	int ret = MM_ERROR_NONE;
	virtual_sound_stream_info_s *vstream_h = (virtual_sound_stream_info_s*)virtual_stream;

	LOGI(">> enter");

	ret = _stop_virtual_stream(vstream_h);

	return _convert_sound_manager_error_code(__func__, ret);
}
