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

#define PA_BUS_NAME                                             "org.pulseaudio.Server"
#define PA_STREAM_MANAGER_OBJECT_PATH                           "/org/pulseaudio/StreamManager"
#define PA_STREAM_MANAGER_INTERFACE                             "org.pulseaudio.StreamManager"
#define PA_STREAM_MANAGER_METHOD_NAME_GET_STREAM_INFO           "GetStreamInfo"
#define PA_STREAM_MANAGER_METHOD_NAME_SET_STREAM_ROUTE_DEVICES  "SetStreamRouteDevices"
#define PA_STREAM_MANAGER_METHOD_NAME_SET_STREAM_ROUTE_OPTIONS  "SetStreamRouteOptions"
#define PA_STREAM_MANAGER_METHOD_NAME_GET_VOLUME_MAX_LEVEL      "GetVolumeMaxLevel"
#define PA_STREAM_MANAGER_METHOD_NAME_GET_CURRENT_VOLUME_TYPE   "GetCurrentVolumeType"

extern _session_interrupt_info_s g_session_interrupt_cb_table;
extern _session_mode_e g_cached_session_mode;
extern _focus_watch_info_s g_focus_watch_cb_table;
extern sound_stream_info_s* sound_stream_info_arr[SOUND_STREAM_INFO_ARR_MAX];
int g_stream_info_count = 0;
pthread_mutex_t g_stream_info_count_mutex = PTHREAD_MUTEX_INITIALIZER;

int __convert_sound_manager_error_code (const char *func, int code) {
	int ret = SOUND_MANAGER_ERROR_NONE;
	char *errorstr = NULL;

	switch(code) {
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
	case MM_ERROR_OUT_OF_MEMORY:
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
	case MM_ERROR_NOT_SUPPORT_API:
	case MM_ERROR_SOUND_NOT_SUPPORTED_OPERATION:
		ret = SOUND_MANAGER_ERROR_NOT_SUPPORTED;
		errorstr = "NOT_SUPPORTED";
		break;
	case MM_ERROR_SOUND_INVALID_STATE:
		ret = SOUND_MANAGER_ERROR_INVALID_STATE;
		errorstr = "INVALID_STATE";
		break;
	}
	if (ret) {
		LOGE("[%s] %s(0x%08x) : core frameworks error code(0x%08x)",func, errorstr, ret, code);
	} else {
		LOGD("[%s] %s(0x%08x) : core frameworks error code(0x%08x)",func, errorstr, ret, code);
	}
	return ret;
}

int __convert_stream_type (sound_stream_type_e stream_type_enum, char *stream_type)
{
	int ret = MM_ERROR_NONE;

	if (stream_type == NULL) {
		return MM_ERROR_INVALID_ARGUMENT;
	}

	switch (stream_type_enum) {
	case SOUND_STREAM_TYPE_MEDIA:
		SM_STRNCPY(stream_type,"media",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_SYSTEM:
		SM_STRNCPY(stream_type,"system",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_ALARM:
		SM_STRNCPY(stream_type,"alarm",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_NOTIFICATION:
		SM_STRNCPY(stream_type,"notification",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_EMERGENCY:
		SM_STRNCPY(stream_type,"emergency",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_VOICE_INFORMATION:
		SM_STRNCPY(stream_type,"voice-information",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_VOICE_RECOGNITION:
		SM_STRNCPY(stream_type,"voice-recognition",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_RINGTONE_VOIP:
		SM_STRNCPY(stream_type,"ringtone-voip",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_VOIP:
		SM_STRNCPY(stream_type,"voip",SOUND_STREAM_TYPE_LEN,ret);
		break;
	default:
		LOGE("could not find the stream_type[%d] in this switch case statement", stream_type_enum);
		ret = MM_ERROR_SOUND_INTERNAL;
		break;
	}
	LOGI("stream_type[%s]", stream_type);

	return ret;
}

int __convert_stream_type_for_internal (sound_stream_type_internal_e stream_type_enum, char *stream_type)
{
	int ret = MM_ERROR_NONE;

	if (stream_type == NULL) {
		return MM_ERROR_INVALID_ARGUMENT;
	}

	switch (stream_type_enum) {
	case SOUND_STREAM_TYPE_RINGTONE_CALL:
		SM_STRNCPY(stream_type,"ringtone-call",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_VOICE_CALL:
		SM_STRNCPY(stream_type,"call-voice",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_VIDEO_CALL:
		SM_STRNCPY(stream_type,"call-video",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_RADIO:
		SM_STRNCPY(stream_type,"radio",SOUND_STREAM_TYPE_LEN,ret);
		break;
	case SOUND_STREAM_TYPE_LOOPBACK:
		SM_STRNCPY(stream_type,"loopback",SOUND_STREAM_TYPE_LEN,ret);
		break;
	default:
		LOGE("could not find the stream_type[%d] in this switch case statement", stream_type_enum);
		ret = MM_ERROR_SOUND_INTERNAL;
		break;
	}
	LOGI("stream_type_for_internal[%s]", stream_type);

	return ret;
}

int __convert_stream_type_to_change_reason (const char *stream_type, sound_stream_focus_change_reason_e *change_reason)
{
	int ret = MM_ERROR_NONE;

	SM_NULL_ARG_CHECK(stream_type);
	SM_NULL_ARG_CHECK(change_reason);

	if (!strncmp(stream_type, "media", SOUND_STREAM_TYPE_LEN) ||
		!strncmp(stream_type, "radio", SOUND_STREAM_TYPE_LEN) ||
		!strncmp(stream_type, "loopback", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_MEDIA;

	} else if (!strncmp(stream_type, "system", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_SYSTEM;

	} else if (!strncmp(stream_type, "alarm", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_ALARM;

	} else if (!strncmp(stream_type, "notification", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_NOTIFICATION;

	} else if (!strncmp(stream_type, "emergency", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_EMERGENCY;

	} else if (!strncmp(stream_type, "voice-information", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_VOICE_INFORMATION;

	} else if (!strncmp(stream_type, "voice-recognition", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_VOICE_RECOGNITION;

	} else if (!strncmp(stream_type, "ringtone-voip", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "ringtone-call", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_RINGTONE;

	} else if (!strncmp(stream_type, "voip", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_VOIP;

	} else if (!strncmp(stream_type, "call-voice", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "call-video", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_CALL;
	} else {
		ret = MM_ERROR_INVALID_ARGUMENT;
		LOGE("not supported stream_type(%s), err(0x%08x)", stream_type, ret);
	}
	return ret;
}

int __convert_sound_type (sound_type_e sound_type, const char **volume_type)
{
	int ret = MM_ERROR_NONE;

	SM_NULL_ARG_CHECK(volume_type);

	switch (sound_type) {
	case SOUND_TYPE_SYSTEM:
		*volume_type = "system";
		break;
	case SOUND_TYPE_NOTIFICATION:
		*volume_type = "notification";
		break;
	case SOUND_TYPE_ALARM:
		*volume_type = "alarm";
		break;
	case SOUND_TYPE_RINGTONE:
		*volume_type = "ringtone";
		break;
	case SOUND_TYPE_MEDIA:
		*volume_type = "media";
		break;
	case SOUND_TYPE_CALL:
		*volume_type = "call";
		break;
	case SOUND_TYPE_VOIP:
		*volume_type = "voip";
		break;
	case SOUND_TYPE_VOICE:
		*volume_type = "voice";
		break;
	}
	if (!strncmp(*volume_type,"",SOUND_DEVICE_TYPE_LEN)) {
		LOGE("could not find the sound_type[%d] in this switch case statement", sound_type);
		ret = MM_ERROR_SOUND_INTERNAL;
	} else {
		LOGI("volume_type[%s]", *volume_type);
	}
	return ret;
}

int __convert_sound_type_to_enum (char *sound_type, sound_type_e *sound_type_enum)
{
	int ret = MM_ERROR_NONE;

	SM_NULL_ARG_CHECK(sound_type);
	SM_NULL_ARG_CHECK(sound_type_enum);

	if (!strncmp(sound_type, "system", strlen(sound_type))) {
		*sound_type_enum = SOUND_TYPE_SYSTEM;
	} else if (!strncmp(sound_type, "notification", strlen(sound_type))) {
		*sound_type_enum = SOUND_TYPE_NOTIFICATION;
	} else if (!strncmp(sound_type, "alarm", strlen(sound_type))) {
		*sound_type_enum = SOUND_TYPE_ALARM;
	} else if (!strncmp(sound_type, "ringtone", strlen(sound_type))) {
		*sound_type_enum = SOUND_TYPE_RINGTONE;
	} else if (!strncmp(sound_type, "media", strlen(sound_type))) {
		*sound_type_enum = SOUND_TYPE_MEDIA;
	} else if (!strncmp(sound_type, "call", strlen(sound_type))) {
		*sound_type_enum = SOUND_TYPE_CALL;
	} else if (!strncmp(sound_type, "voip", strlen(sound_type))) {
		*sound_type_enum = SOUND_TYPE_VOIP;
	} else if (!strncmp(sound_type, "voice", strlen(sound_type))) {
		*sound_type_enum = SOUND_TYPE_VOICE;
	} else {
		ret = MM_ERROR_INVALID_ARGUMENT;
		LOGE("not supported sound_type(%s), err(0x%08x)", sound_type, ret);
	}

	return ret;
}

int __convert_device_type (sound_device_type_e device_type_enum, char **device_type)
{
	int ret = MM_ERROR_NONE;

	SM_NULL_ARG_CHECK(device_type);

	switch (device_type_enum) {
	case SOUND_DEVICE_BUILTIN_SPEAKER:
		*device_type = "builtin-speaker";
		break;
	case SOUND_DEVICE_BUILTIN_RECEIVER:
		*device_type = "builtin-receiver";
		break;
	case SOUND_DEVICE_BUILTIN_MIC:
		*device_type = "builtin-mic";
		break;
	case SOUND_DEVICE_AUDIO_JACK:
		*device_type = "audio-jack";
		break;
	case SOUND_DEVICE_BLUETOOTH:
		*device_type = "bt";
		break;
	case SOUND_DEVICE_HDMI:
		*device_type = "hdmi";
		break;
	case SOUND_DEVICE_USB_AUDIO:
		*device_type = "usb-audio";
		break;
	case SOUND_DEVICE_FORWARDING:
		*device_type = "forwarding";
		break;
	}
	if (!strncmp(*device_type,"",SOUND_DEVICE_TYPE_LEN)) {
		LOGE("could not find the device_type[%d] in this switch case statement", device_type_enum);
		ret = MM_ERROR_SOUND_INTERNAL;
	} else {
		LOGI("device_type[%s]", *device_type);
	}
	return ret;
}

int __convert_device_io_direction (mm_sound_device_io_direction_e io_direction, sound_device_io_direction_e *sound_io_direction)
{
	int ret = MM_ERROR_NONE;

	SM_NULL_ARG_CHECK(sound_io_direction);

	switch (io_direction) {
	case MM_SOUND_DEVICE_IO_DIRECTION_IN:
		*sound_io_direction = SOUND_DEVICE_IO_DIRECTION_IN;
		break;
	case MM_SOUND_DEVICE_IO_DIRECTION_OUT:
		*sound_io_direction = SOUND_DEVICE_IO_DIRECTION_OUT;
		break;
	case MM_SOUND_DEVICE_IO_DIRECTION_BOTH:
		*sound_io_direction = SOUND_DEVICE_IO_DIRECTION_BOTH;
		break;
	}
	return ret;
}

const char* __convert_api_name (native_api_e api_name)
{
	const char* name = NULL;
	switch (api_name) {
	case NATIVE_API_SOUND_MANAGER:
		name = "sound-manager";
		break;
	case NATIVE_API_PLAYER:
		name = "player";
		break;
	case NATIVE_API_WAV_PLAYER:
		name = "wav-player";
		break;
	case NATIVE_API_TONE_PLAYER:
		name = "tone-player";
		break;
	case NATIVE_API_AUDIO_IO:
		name = "audio-io";
		break;
	case NATIVE_API_RECORDER:
		name = "recorder";
		break;
	}
	return name;
}

void _focus_state_change_callback (int index, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data)
{
	int ret = MM_ERROR_NONE;
	int i = 0;

	LOGI(">> enter");

	sound_stream_focus_change_reason_e change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_MEDIA;
	ret = __convert_stream_type_to_change_reason(reason_for_change, &change_reason);
	if (ret) {
		LOGE("failed to __convert_stream_type_to_enum(), reason_for_change(%s), err(0x%08x)", reason_for_change, ret);
	} else {
		for (i = 0; i < SOUND_STREAM_INFO_ARR_MAX; i++) {
			if (sound_stream_info_arr[i] && sound_stream_info_arr[i]->index == index) {
				if (state ==  FOCUS_IS_RELEASED) {
					sound_stream_info_arr[i]->acquired_focus &= ~focus_type;
				} else if (state == FOCUS_IS_ACQUIRED) {
					sound_stream_info_arr[i]->acquired_focus |= focus_type;
				}
				LOGI("[FOCUS USER CALLBACK(%p) START]", sound_stream_info_arr[i]->user_cb);
				sound_stream_info_arr[i]->user_cb((sound_stream_info_h)sound_stream_info_arr[i], change_reason, additional_info, sound_stream_info_arr[i]->user_data);
				LOGI("[FOCUS USER CALLBACK(%p) END]", sound_stream_info_arr[i]->user_cb);
				break;
			}
		}
		if (i == SOUND_STREAM_INFO_ARR_MAX) {
			LOGE("could not find index(%d), failed to call user callback", index);
		}
	}

	LOGI("<< leave");

	return;
}

void _focus_watch_callback (int index, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason_for_change, const char *additional_info, void *user_data)
{
	int ret = MM_ERROR_NONE;
	sound_stream_focus_change_reason_e change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_MEDIA;
	ret = __convert_stream_type_to_change_reason(reason_for_change, &change_reason);
	if (ret)
		LOGE("failed to __convert_stream_type_to_enum(), reason_for_change(%s), err(0x%08x)", reason_for_change, ret);
	if (g_focus_watch_cb_table.user_cb)
		g_focus_watch_cb_table.user_cb(focus_type, state, change_reason, additional_info, g_focus_watch_cb_table.user_data);
	return;
}

void _pa_context_state_cb (pa_context *c, void *userdata)
{
	pa_context_state_t state;
	sound_stream_info_s *stream_info_h = (sound_stream_info_s*)userdata;
	assert(c);

	state = pa_context_get_state(c);
	LOGI ("[%p] context state = [%d]", stream_info_h, state);
	switch (state) {
	case PA_CONTEXT_READY:
	case PA_CONTEXT_TERMINATED:
	case PA_CONTEXT_FAILED:
		pa_threaded_mainloop_signal(stream_info_h->pa_mainloop, 0);
		break;
	case PA_CONTEXT_UNCONNECTED:
	case PA_CONTEXT_CONNECTING:
	case PA_CONTEXT_AUTHORIZING:
	case PA_CONTEXT_SETTING_NAME:
		break;
	}
}

void _pa_stream_state_cb (pa_stream *s, void * userdata)
{
	pa_stream_state_t state;
	virtual_sound_stream_info_s *vstream_h = (virtual_sound_stream_info_s*)userdata;
	assert(s);

	state = pa_stream_get_state(s);
	LOGI ("[%p] stream [%d] state = [%d]", vstream_h, pa_stream_get_index(s), state);

	switch (state) {
	case PA_STREAM_READY:
	case PA_STREAM_FAILED:
	case PA_STREAM_TERMINATED:
		pa_threaded_mainloop_signal(vstream_h->pa_mainloop, 0);
		break;
	case PA_STREAM_UNCONNECTED:
	case PA_STREAM_CREATING:
		break;
	}
}

int __get_stream_conf_info (const char *stream_type, stream_conf_info_s *info)
{
	int ret = MM_ERROR_NONE;
	GVariant *result = NULL;
	GVariant *child = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;

	assert(stream_type);
	assert(info);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn && err) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free (err);
		return MM_ERROR_SOUND_INTERNAL;
	}
	result = g_dbus_connection_call_sync (conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_GET_STREAM_INFO,
							g_variant_new ("(s)", stream_type),
							G_VARIANT_TYPE("(vvvvv)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result && err) {
		LOGE("g_dbus_connection_call_sync() error (%s)", err->message);
		ret = MM_ERROR_SOUND_INTERNAL;
	} else {
		GVariantIter iter;
		GVariant *item = NULL;
		gchar *name = NULL;
		gsize size = 0;
		int i = 0;

		/* get priority */
		child = g_variant_get_child_value(result, 0);
		item = g_variant_get_variant(child);
		info->priority = g_variant_get_int32(item);
		g_variant_unref (item);
		g_variant_unref (child);
		LOGI ("priority(%d)", info->priority);

		/* get route type */
		child = g_variant_get_child_value(result, 1);
		item = g_variant_get_variant(child);
		info->route_type = g_variant_get_int32(item);
		g_variant_unref (item);
		g_variant_unref (child);
		LOGI ("route_type(%d)", info->route_type);

		/* get availabe in-devices */
		child = g_variant_get_child_value(result, 2);
		item = g_variant_get_variant(child);
		size = g_variant_n_children(item);
		LOGI ("num of avail-in-devices are %d", size);
		g_variant_iter_init(&iter, item);
		i = 0;
		while (g_variant_iter_loop(&iter, "&s", &name)) {
			if (size == 1 && !strncmp (name, "none", strlen("none"))) {
				LOGI (" in-device is [%s], skip it", name);
				break;
			} else {
				LOGI (" in-device name : %s", name);
				info->avail_in_devices[i++] = strdup(name);
			}
		}
		g_variant_iter_free (&iter);
		g_variant_unref (item);
		g_variant_unref (child);

		/* get available out-devices */
		child = g_variant_get_child_value(result, 3);
		item = g_variant_get_variant(child);
		size = g_variant_n_children(item);
		LOGI ("num of avail-out-devices are %d", size);
		g_variant_iter_init(&iter, item);
		i = 0;
		while (g_variant_iter_loop(&iter, "&s", &name)) {
			if (size == 1 && !strncmp (name, "none", strlen("none"))) {
				LOGI (" out-device is [%s], skip it", name);
				break;
			} else {
				LOGI (" out-device name : %s", name);
				info->avail_out_devices[i++] = strdup(name);
			}
		}
		g_variant_iter_free (&iter);
		g_variant_unref (item);
		g_variant_unref (child);

		/* get available frameworks */
		child = g_variant_get_child_value(result, 4);
		item = g_variant_get_variant(child);
		size = g_variant_n_children(item);
		LOGI ("num of avail-frameworks are %d", size);
		g_variant_iter_init(&iter, item);
		i = 0;
		while (g_variant_iter_loop(&iter, "&s", &name)) {
			if (size == 1 && !strncmp (name, "none", strlen("none"))) {
				LOGI (" framework is [%s], skip it", name);
				break;
			} else {
				LOGI (" framework name : %s", name);
				info->avail_frameworks[i++] = strdup(name);
			}
		}
		g_variant_iter_free (&iter);
		g_variant_unref (item);
		g_variant_unref (child);
		g_variant_unref(result);
	}
	g_object_unref(conn);
	return ret;
}

int __set_manual_route_info (unsigned int index, manual_route_info_s *info)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	GVariantBuilder *builder_for_in_devices;
	GVariantBuilder *builder_for_out_devices;
	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;

	assert(info);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn && err) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free (err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	builder_for_in_devices = g_variant_builder_new(G_VARIANT_TYPE("au"));
	builder_for_out_devices = g_variant_builder_new(G_VARIANT_TYPE("au"));
	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (info->route_in_devices[i]) {
			g_variant_builder_add(builder_for_in_devices, "u", info->route_in_devices[i]);
			LOGI("[IN] device_id:%u", info->route_in_devices[i]);
		} else {
			break;
		}
	}
	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (info->route_out_devices[i]) {
			g_variant_builder_add(builder_for_out_devices, "u", info->route_out_devices[i]);
			LOGI("[OUT] device_id:%u", info->route_out_devices[i]);
		} else {
			break;
		}
	}

	result = g_dbus_connection_call_sync (conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_SET_STREAM_ROUTE_DEVICES,
							g_variant_new ("(uauau)", index, builder_for_in_devices, builder_for_out_devices),
							G_VARIANT_TYPE("(s)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result && err) {
		LOGE("g_dbus_connection_call_sync() error (%s)", err->message);
		ret = MM_ERROR_SOUND_INTERNAL;
	} else {
		const gchar *dbus_ret = NULL;
		g_variant_get(result, "(&s)", &dbus_ret);
		LOGI("g_dbus_connection_call_sync() success, method return value is (%s)", dbus_ret);
		if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret))) {
			ret = MM_ERROR_SOUND_INVALID_STATE;
		}
		g_variant_unref(result);
	}
	g_variant_builder_unref(builder_for_in_devices);
	g_variant_builder_unref(builder_for_out_devices);
	g_object_unref(conn);
	return ret;
}

int __set_route_options (unsigned int index, char **route_options)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	GVariantBuilder *builder;

	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;

	assert(route_options);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn && err) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free (err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	builder = g_variant_builder_new(G_VARIANT_TYPE("as"));
	for (i = 0; i < ROUTE_OPTIONS_MAX; i++) {
		if (route_options[i]) {
			g_variant_builder_add(builder, "s", route_options[i]);
			LOGI("[OPTIONS] %s", route_options[i]);
		} else {
			break;
		}
	}

	result = g_dbus_connection_call_sync (conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_SET_STREAM_ROUTE_OPTIONS,
							g_variant_new ("(uas)", index, builder),
							G_VARIANT_TYPE("(s)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result && err) {
		LOGE("g_dbus_connection_call_sync() error (%s)", err->message);
		ret = MM_ERROR_SOUND_INTERNAL;
	} else {
		const gchar *dbus_ret = NULL;
		g_variant_get(result, "(&s)", &dbus_ret);
		LOGI("g_dbus_connection_call_sync() success, method return value is (%s)", dbus_ret);
		if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret))) {
			ret = MM_ERROR_SOUND_INTERNAL;
		}
		g_variant_unref(result);
	}
	g_variant_builder_unref(builder);
	g_object_unref(conn);
	return ret;
}

int __get_volume_max_level (const char *direction, const char *volume_type, unsigned int *max_level)
{
	int ret = MM_ERROR_NONE;

	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;

	assert(direction);
	assert(volume_type);
	assert(max_level);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn && err) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free (err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	result = g_dbus_connection_call_sync (conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_GET_VOLUME_MAX_LEVEL,
							g_variant_new ("(ss)", direction, volume_type),
							G_VARIANT_TYPE("(us)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result && err) {
		LOGE("g_dbus_connection_call_sync() error (%s)", err->message);
		ret = MM_ERROR_SOUND_INTERNAL;
	} else {
		const gchar *dbus_ret = NULL;
		g_variant_get(result, "(u&s)", max_level, &dbus_ret);
		LOGI("g_dbus_connection_call_sync() success, method return value is (%u, %s)", *max_level, dbus_ret);
		if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret))) {
			ret = MM_ERROR_SOUND_INTERNAL;
		}
		g_variant_unref(result);
	}
	g_object_unref(conn);
	return ret;
}

int __get_current_volume_type (const char *direction, char **volume_type)
{
	int ret = MM_ERROR_NONE;

	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;

	assert(direction);
	assert(volume_type);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn && err) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free (err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	result = g_dbus_connection_call_sync (conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_GET_CURRENT_VOLUME_TYPE,
							g_variant_new ("(s)", direction),
							G_VARIANT_TYPE("(ss)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result && err) {
		LOGE("g_dbus_connection_call_sync() error (%s)", err->message);
		ret = MM_ERROR_SOUND_INTERNAL;
	} else {
		const gchar *dbus_volume_type = NULL;
		const gchar *dbus_ret = NULL;
		g_variant_get(result, "(&s&s)", &dbus_volume_type, &dbus_ret);
		LOGI("g_dbus_connection_call_sync() success, method return value is (%s, %s)", dbus_volume_type, dbus_ret);
		if (!strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret))) {
			ret = MM_ERROR_NONE;
			*volume_type = strdup(dbus_volume_type);
		} else if (!strncmp("STREAM_MANAGER_RETURN_ERROR_NO_STREAM", dbus_ret, strlen(dbus_ret))) {
			ret = MM_ERROR_SOUND_VOLUME_NO_INSTANCE;
		} else {
			ret = MM_ERROR_SOUND_INTERNAL;
		}
		g_variant_unref(result);
	}
	g_object_unref(conn);
	return ret;
}

void __session_interrupt_cb (session_msg_t msg, session_event_t event, void *user_data){
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

int __set_session_mode (_session_mode_e mode)
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

		ret = mm_sound_get_current_device_list(MM_SOUND_DEVICE_ALL_FLAG, &device_list);
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
							else if (io_direction == MM_SOUND_DEVICE_IO_DIRECTION_OUT)
								route = MM_SOUND_ROUTE_IN_MIC_OUT_HEADPHONE;
							do_subsession = true;
						}
						break;
					case _SESSION_MODE_VOICE_WITH_BLUETOOTH:
						if (type == MM_SOUND_DEVICE_TYPE_BLUETOOTH) {
							mm_sound_device_io_direction_e io_direction;
							ret = mm_sound_get_device_io_direction(device, &io_direction);
							if (ret != MM_ERROR_NONE)
								goto ERROR_CASE;
							if (io_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
								route = MM_SOUND_ROUTE_INOUT_BLUETOOTH;
								do_subsession = true;
							}
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

#ifdef TMP_CODE
		mm_sound_set_active_route(route); //not getting return value temporarily. when 2.4 feature for routing is fully implemented, it will be recoverd.
#else
		ret = mm_sound_set_active_route(route);
#endif

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

int __get_session_mode (_session_mode_e *mode)
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

int _make_pa_connection_and_register_focus(sound_stream_info_s *stream_h, sound_stream_focus_state_changed_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	int pa_ret = PA_OK;
	int i = 0;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN( &g_stream_info_count_mutex, MM_ERROR_SOUND_INTERNAL);

	if (!(stream_h->pa_mainloop = pa_threaded_mainloop_new()))
		goto PA_ERROR;

	if (!(stream_h->pa_context = pa_context_new(pa_threaded_mainloop_get_api(stream_h->pa_mainloop), "SOUND_MANAGER_STREAM_INFO")))
		goto PA_ERROR;

	pa_context_set_state_callback(stream_h->pa_context, _pa_context_state_cb, stream_h);

	if (pa_context_connect(stream_h->pa_context, NULL, 0, NULL) < 0) {
		pa_ret = pa_context_errno(stream_h->pa_context);
		goto PA_ERROR;
	}

	pa_threaded_mainloop_lock(stream_h->pa_mainloop);

	if (pa_threaded_mainloop_start(stream_h->pa_mainloop) < 0)
		goto PA_ERROR_WITH_UNLOCK;

	/* wait for ready state of the context */
	for (;;) {
		pa_context_state_t state;
		state = pa_context_get_state(stream_h->pa_context);
		if (state == PA_CONTEXT_READY)
			break;
		if (!PA_CONTEXT_IS_GOOD(state)) {
			pa_ret = pa_context_errno(stream_h->pa_context);
			goto PA_ERROR_WITH_UNLOCK;
		}
		pa_threaded_mainloop_wait(stream_h->pa_mainloop);
	}

	/* get uniq id of this context */
	stream_h->index = pa_context_get_index(stream_h->pa_context);

	/* get configuration information of this stream type */
	ret = __get_stream_conf_info(stream_h->stream_type, &stream_h->stream_conf_info);
	if (ret) {
		goto PA_ERROR_WITH_UNLOCK;
	} else {
		LOGI("stream_conf_info : priority[%d], route type[%d]", stream_h->stream_conf_info.priority, stream_h->stream_conf_info.route_type);
	}

	pa_threaded_mainloop_unlock(stream_h->pa_mainloop);

	SM_REF_FOR_STREAM_INFO(g_stream_info_count, ret);

	/* register focus */
	ret = mm_sound_register_focus(stream_h->index, stream_h->stream_type, _focus_state_change_callback, user_data);
	if (ret == MM_ERROR_NONE) {
		int i = 0;
		stream_h->user_cb = callback;
		stream_h->user_data = user_data;
		for (i = 0; i < SOUND_STREAM_INFO_ARR_MAX; i++) {
			if (sound_stream_info_arr[i] == NULL) {
				sound_stream_info_arr[i] = stream_h;
				break;
			}
		}
		if (i == SOUND_STREAM_INFO_ARR_MAX) {
			LOGE("client sound stream info array is full");
			ret = mm_sound_unregister_focus(stream_h->index);
			SM_UNREF_FOR_STREAM_INFO(g_stream_info_count, ret);
			goto PA_ERROR;
		}
	} else {
		LOGE("failed to register focus, ret(0x%x)", ret);
		SM_UNREF_FOR_STREAM_INFO(g_stream_info_count, ret);
		/* disconnect */
		goto PA_ERROR;
	}
	goto SUCCESS;
PA_ERROR_WITH_UNLOCK:
	pa_threaded_mainloop_unlock(stream_h->pa_mainloop);
PA_ERROR:
	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (stream_h->stream_conf_info.avail_in_devices[i]) {
			free(stream_h->stream_conf_info.avail_in_devices[i]);
		} else {
			break;
		}
	}
	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (stream_h->stream_conf_info.avail_out_devices[i]) {
			free(stream_h->stream_conf_info.avail_out_devices[i]);
		} else {
			break;
		}
	}
	for (i = 0; i < AVAIL_FRAMEWORKS_MAX; i++) {
		if (stream_h->stream_conf_info.avail_frameworks[i]) {
			free(stream_h->stream_conf_info.avail_frameworks[i]);
		} else {
			break;
		}
	}
	if (stream_h->pa_context) {
		pa_context_disconnect(stream_h->pa_context);
		pa_context_unref(stream_h->pa_context);
		stream_h->pa_context = NULL;
	}
	if (stream_h->pa_mainloop) {
		pa_threaded_mainloop_free(stream_h->pa_mainloop);
		stream_h->pa_mainloop = NULL;
	}
	free(stream_h);
	ret = MM_ERROR_SOUND_INTERNAL;
	LOGE("pa_ret(%d), ret(%p)", pa_ret, ret);

SUCCESS:
	SM_LEAVE_CRITICAL_SECTION(&g_stream_info_count_mutex);

	return ret;
}

int _destroy_pa_connection_and_unregister_focus(sound_stream_info_s *stream_h)
{
	int i = 0;
	int ret = MM_ERROR_NONE;

	SM_ENTER_CRITICAL_SECTION_WITH_RETURN( &g_stream_info_count_mutex, MM_ERROR_SOUND_INTERNAL);

	if (stream_h->pa_context) {
		pa_context_disconnect(stream_h->pa_context);
		pa_context_unref(stream_h->pa_context);
		stream_h->pa_context = NULL;
	}

	if (stream_h->pa_mainloop) {
		pa_threaded_mainloop_free(stream_h->pa_mainloop);
		stream_h->pa_mainloop = NULL;
	}

	/* unregister focus */
	ret = mm_sound_unregister_focus(stream_h->index);
	if (ret)
		LOGE("failed to unregister focus, ret(0x%x)", ret);

	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (stream_h->stream_conf_info.avail_in_devices[i]) {
			free(stream_h->stream_conf_info.avail_in_devices[i]);
		}
		if (stream_h->stream_conf_info.avail_out_devices[i]) {
			free(stream_h->stream_conf_info.avail_out_devices[i]);
		}
	}
	for (i = 0; i < AVAIL_FRAMEWORKS_MAX; i++) {
		if (stream_h->stream_conf_info.avail_frameworks[i]) {
			free(stream_h->stream_conf_info.avail_frameworks[i]);
		}
	}
	for (i = 0; i < ROUTE_OPTIONS_MAX; i++) {
		if (stream_h->route_options[i]) {
			free(stream_h->route_options[i]);
		}
	}
	for (i = 0; i < SOUND_STREAM_INFO_ARR_MAX; i++) {
		if (sound_stream_info_arr[i] && sound_stream_info_arr[i]->index == stream_h->index) {
			sound_stream_info_arr[i] = NULL;
			break;
		}
	}
	free(stream_h);

	SM_UNREF_FOR_STREAM_INFO(g_stream_info_count, ret);

	SM_LEAVE_CRITICAL_SECTION(&g_stream_info_count_mutex);

	return ret;
}
