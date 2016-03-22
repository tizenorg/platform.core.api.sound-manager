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
#include <vconf.h>

#define PA_BUS_NAME                                             "org.pulseaudio.Server"
#define PA_STREAM_MANAGER_OBJECT_PATH                           "/org/pulseaudio/StreamManager"
#define PA_STREAM_MANAGER_INTERFACE                             "org.pulseaudio.StreamManager"
#define PA_STREAM_MANAGER_METHOD_NAME_GET_STREAM_INFO           "GetStreamInfo"
#define PA_STREAM_MANAGER_METHOD_NAME_SET_STREAM_ROUTE_DEVICES  "SetStreamRouteDevices"
#define PA_STREAM_MANAGER_METHOD_NAME_SET_STREAM_ROUTE_OPTION   "SetStreamRouteOption"
#define PA_STREAM_MANAGER_METHOD_NAME_GET_VOLUME_MAX_LEVEL      "GetVolumeMaxLevel"
#define PA_STREAM_MANAGER_METHOD_NAME_GET_VOLUME_LEVEL          "GetVolumeLevel"
#define PA_STREAM_MANAGER_METHOD_NAME_SET_VOLUME_LEVEL          "SetVolumeLevel"
#define PA_STREAM_MANAGER_METHOD_NAME_GET_CURRENT_VOLUME_TYPE   "GetCurrentVolumeType"
#define PA_STREAM_MANAGER_METHOD_NAME_UPDATE_FOCUS_STATUS       "UpdateFocusStatus"
#define VCONF_PATH_PREFIX_VOLUME                                "file/private/sound/volume/"
#define VCONF_PATH_MAX                                          64

extern _session_interrupt_info_s g_session_interrupt_cb_table;
extern _session_mode_e g_cached_session_mode;
extern int g_cached_voip_device_id;
extern _focus_watch_info_s g_focus_watch_cb_table;
sound_stream_info_s *sound_stream_info_arr[SOUND_STREAM_INFO_ARR_MAX];
sound_stream_info_s *g_voip_stream_info = NULL;
virtual_sound_stream_info_s *g_voip_vstream_h = NULL;
int g_stream_info_count = 0;
pthread_mutex_t g_stream_info_count_mutex = PTHREAD_MUTEX_INITIALIZER;

int _convert_sound_manager_error_code(const char *func, int code)
{
	int ret = SOUND_MANAGER_ERROR_NONE;
	char *errorstr = NULL;

	switch (code) {
	case MM_ERROR_FILE_WRITE:
	case MM_ERROR_INVALID_HANDLE:
	case MM_ERROR_SOUND_INVALID_OPERATION:
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
		ret = SOUND_MANAGER_ERROR_NOT_SUPPORTED;
		errorstr = "NOT_SUPPORTED";
		break;
	case MM_ERROR_SOUND_INVALID_STATE:
		ret = SOUND_MANAGER_ERROR_INVALID_STATE;
		errorstr = "INVALID_STATE";
		break;
	default:
		LOGW("it should not be reached here, this error(0x%x) should be defined.", code);
		ret = SOUND_MANAGER_ERROR_INTERNAL;
		errorstr = "INTERNAL";
		break;
	}
	if (ret)
		LOGE("[%s] >> leave : %s(0x%08x), mm_error(0x%08x)", func, errorstr, ret, code);
	else
		LOGD("[%s] >> leave : %s(0x%08x)", func, errorstr, ret);

	return ret;
}

int _convert_stream_type(sound_stream_type_e stream_type_enum, char **stream_type)
{
	int ret = MM_ERROR_NONE;

	if (stream_type == NULL)
		return MM_ERROR_INVALID_ARGUMENT;

	switch (stream_type_enum) {
	case SOUND_STREAM_TYPE_MEDIA:
		*stream_type = "media";
		break;
	case SOUND_STREAM_TYPE_SYSTEM:
		*stream_type = "system";
		break;
	case SOUND_STREAM_TYPE_ALARM:
		*stream_type = "alarm";
		break;
	case SOUND_STREAM_TYPE_NOTIFICATION:
		*stream_type = "notification";
		break;
	case SOUND_STREAM_TYPE_EMERGENCY:
		*stream_type = "emergency";
		break;
	case SOUND_STREAM_TYPE_VOICE_INFORMATION:
		*stream_type = "voice-information";
		break;
	case SOUND_STREAM_TYPE_VOICE_RECOGNITION:
		*stream_type = "voice-recognition";
		break;
	case SOUND_STREAM_TYPE_RINGTONE_VOIP:
		*stream_type = "ringtone-voip";
		break;
	case SOUND_STREAM_TYPE_VOIP:
		*stream_type = "voip";
		break;
	case SOUND_STREAM_TYPE_MEDIA_EXTERNAL_ONLY:
		*stream_type = "ext-media";
		break;
	default:
		LOGE("could not find the stream_type[%d] in this switch case statement", stream_type_enum);
		ret = MM_ERROR_SOUND_INTERNAL;
		break;
	}
	LOGI("stream_type[%s]", *stream_type);

	return ret;
}

int _convert_stream_type_for_internal(sound_stream_type_internal_e stream_type_enum, char **stream_type)
{
	int ret = MM_ERROR_NONE;

	if (stream_type == NULL)
		return MM_ERROR_INVALID_ARGUMENT;

	switch (stream_type_enum) {
	case SOUND_STREAM_TYPE_RINGTONE_CALL:
		*stream_type = "ringtone-call";
		break;
	case SOUND_STREAM_TYPE_RINGBACKTONE_CALL:
		*stream_type = "ringbacktone-call";
		break;
	case SOUND_STREAM_TYPE_VOICE_CALL:
		*stream_type = "call-voice";
		break;
	case SOUND_STREAM_TYPE_VIDEO_CALL:
		*stream_type = "call-video";
		break;
	case SOUND_STREAM_TYPE_RADIO:
		*stream_type = "radio";
		break;
	case SOUND_STREAM_TYPE_LOOPBACK:
		*stream_type = "loopback";
		break;
	case SOUND_STREAM_TYPE_SOLO:
		*stream_type = "solo";
		break;
	default:
		LOGE("could not find the stream_type[%d] in this switch case statement", stream_type_enum);
		ret = MM_ERROR_SOUND_INTERNAL;
		break;
	}
	LOGI("stream_type_for_internal[%s]", *stream_type);

	return ret;
}

void _set_focus_availability(sound_stream_info_s *stream_info)
{
	if (stream_info == NULL || stream_info->stream_type == NULL) {
		LOGE("invalid argument");
		return;
	}
	if (!strncmp(stream_info->stream_type, "solo", SOUND_STREAM_TYPE_LEN)) {
		stream_info->is_focus_unavailable = true;
		LOGI("this stream_type[%s] does not support focus", stream_info->stream_type);
	}

	return;
}

int _convert_stream_type_to_change_reason(const char *stream_type, sound_stream_focus_change_reason_e *change_reason)
{
	SM_NULL_ARG_CHECK_FOR_PRIV(stream_type);
	SM_NULL_ARG_CHECK_FOR_PRIV(change_reason);

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
			!strncmp(stream_type, "ringtone-call", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "ringbacktone-call", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_RINGTONE;

	} else if (!strncmp(stream_type, "voip", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_VOIP;

	} else if (!strncmp(stream_type, "call-voice", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "call-video", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_CALL;

	} else if (!strncmp(stream_type, "ext-media", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_MEDIA_EXTERNAL_ONLY;

	} else {
		LOGE("not supported stream_type(%s)", stream_type);
		return MM_ERROR_INVALID_ARGUMENT;
	}

	return MM_ERROR_NONE;
}

static int _convert_stream_type_to_interrupt_reason(const char *stream_type, sound_session_interrupted_code_e *change_reason)
{
	SM_NULL_ARG_CHECK_FOR_PRIV(stream_type);
	SM_NULL_ARG_CHECK_FOR_PRIV(change_reason);

	if (!strncmp(stream_type, "media", SOUND_STREAM_TYPE_LEN) ||
		!strncmp(stream_type, "radio", SOUND_STREAM_TYPE_LEN) ||
		!strncmp(stream_type, "system", SOUND_STREAM_TYPE_LEN) ||
		!strncmp(stream_type, "voice-information", SOUND_STREAM_TYPE_LEN) ||
		!strncmp(stream_type, "voice-recognition", SOUND_STREAM_TYPE_LEN) ||
		!strncmp(stream_type, "loopback", SOUND_STREAM_TYPE_LEN) ||
		!strncmp(stream_type, "ext-media", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_SESSION_INTERRUPTED_BY_MEDIA;

	} else if (!strncmp(stream_type, "alarm", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_SESSION_INTERRUPTED_BY_ALARM;

	} else if (!strncmp(stream_type, "notification", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_SESSION_INTERRUPTED_BY_NOTIFICATION;

	} else if (!strncmp(stream_type, "emergency", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_SESSION_INTERRUPTED_BY_EMERGENCY;

	} else if (!strncmp(stream_type, "ringtone-voip", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "ringtone-call", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "ringbacktone-call", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "voip", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "call-voice", SOUND_STREAM_TYPE_LEN) ||
			!strncmp(stream_type, "call-video", SOUND_STREAM_TYPE_LEN)) {
		*change_reason = SOUND_SESSION_INTERRUPTED_BY_CALL;

	} else {
		LOGE("not supported stream_type(%s)", stream_type);
		return MM_ERROR_INVALID_ARGUMENT;
	}

	return MM_ERROR_NONE;
}

int _convert_sound_type(sound_type_e sound_type, const char **volume_type)
{
	SM_NULL_ARG_CHECK_FOR_PRIV(volume_type);

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
	LOGI("volume_type[%s]", *volume_type);

	return MM_ERROR_NONE;
}

int _convert_sound_type_to_enum(const char *sound_type, sound_type_e *sound_type_enum)
{
	SM_NULL_ARG_CHECK_FOR_PRIV(sound_type);
	SM_NULL_ARG_CHECK_FOR_PRIV(sound_type_enum);

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
		LOGE("not supported sound_type(%s)", sound_type);
		return MM_ERROR_INVALID_ARGUMENT;
	}

	return MM_ERROR_NONE;
}

int _convert_device_type(sound_device_type_e device_type_enum, char **device_type)
{
	SM_NULL_ARG_CHECK_FOR_PRIV(device_type);

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
	default:
		LOGE("could not find the device_type[%d] in this switch case statement", device_type_enum);
		return MM_ERROR_SOUND_INTERNAL;
	}

	LOGI("device_type[%s]", *device_type);

	return MM_ERROR_NONE;
}

int _convert_device_io_direction(mm_sound_device_io_direction_e io_direction, sound_device_io_direction_e *sound_io_direction)
{
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

	return MM_ERROR_NONE;
}

const char* _convert_api_name(native_api_e api_name)
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

void _focus_state_change_callback(int index, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason, const char *extra_info, void *user_data)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	sound_stream_focus_change_reason_e change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_MEDIA;

	LOGI(">> enter");

	ret = _convert_stream_type_to_change_reason(reason, &change_reason);
	if (ret) {
		LOGE("failed to _convert_stream_type_to_enum(), reason(%s), err(0x%08x)", reason, ret);
		goto LEAVE;
	}

	for (i = 0; i < SOUND_STREAM_INFO_ARR_MAX; i++) {
		if (sound_stream_info_arr[i] && sound_stream_info_arr[i]->index == index) {
			if (state == FOCUS_IS_RELEASED)
				sound_stream_info_arr[i]->acquired_focus &= ~focus_type;
			else if (state == FOCUS_IS_ACQUIRED)
				sound_stream_info_arr[i]->acquired_focus |= focus_type;

			LOGI("[FOCUS USER CALLBACK(%p) START]", sound_stream_info_arr[i]->user_cb);
			sound_stream_info_arr[i]->user_cb((sound_stream_info_h)sound_stream_info_arr[i], change_reason, extra_info, sound_stream_info_arr[i]->user_data);
			LOGI("[FOCUS USER CALLBACK(%p) END]", sound_stream_info_arr[i]->user_cb);
			break;
		}
	}
	if (i == SOUND_STREAM_INFO_ARR_MAX)
		LOGE("could not find index(%d), failed to call user callback", index);

LEAVE:
	LOGI("<< leave");

	return;
}

void _focus_watch_callback(int index, mm_sound_focus_type_e focus_type, mm_sound_focus_state_e state, const char *reason, const char *extra_info, void *user_data)
{
	int ret = MM_ERROR_NONE;
	sound_stream_focus_change_reason_e change_reason = SOUND_STREAM_FOCUS_CHANGED_BY_MEDIA;

	ret = _convert_stream_type_to_change_reason(reason, &change_reason);
	if (ret)
		LOGE("failed to _convert_stream_type_to_enum(), reason(%s), err(0x%08x)", reason, ret);

	if (g_focus_watch_cb_table.user_cb)
		g_focus_watch_cb_table.user_cb(focus_type, state, change_reason, extra_info, g_focus_watch_cb_table.user_data);

	return;
}

void _pa_context_state_cb(pa_context *c, void *userdata)
{
	pa_context_state_t state;
	sound_stream_info_s *stream_info_h = (sound_stream_info_s*)userdata;

	assert(c);

	state = pa_context_get_state(c);
	LOGI("[%p] context state = [%d]", stream_info_h, state);
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

	return;
}

void _pa_stream_state_cb(pa_stream *s, void * userdata)
{
	pa_stream_state_t state;
	virtual_sound_stream_info_s *vstream_h = (virtual_sound_stream_info_s*)userdata;

	assert(s);

	state = pa_stream_get_state(s);
	LOGI("[%p] stream [%d] state = [%d]", vstream_h, pa_stream_get_index(s), state);

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

	return;
}

int _get_stream_conf_info(const char *stream_type, stream_conf_info_s *info)
{
	int ret = MM_ERROR_NONE;
	GVariant *result = NULL;
	GVariant *child = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;
	GVariantIter iter;
	GVariant *item = NULL;
	gchar *name = NULL;
	gsize size = 0;
	int i = 0;

	assert(stream_type);
	assert(info);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn && err) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free(err);
		return MM_ERROR_SOUND_INTERNAL;
	}
	result = g_dbus_connection_call_sync(conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_GET_STREAM_INFO,
							g_variant_new("(s)", stream_type),
							G_VARIANT_TYPE("(vvvvvv)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result) {
		LOGE("g_dbus_connection_call_sync() for GET_STREAM_INFO error (%s)", err->message);
		g_error_free(err);
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	/* get priority */
	child = g_variant_get_child_value(result, 0);
	item = g_variant_get_variant(child);
	info->priority = g_variant_get_int32(item);
	g_variant_unref(item);
	g_variant_unref(child);
	LOGI("priority(%d)", info->priority);

	/* get route type */
	child = g_variant_get_child_value(result, 1);
	item = g_variant_get_variant(child);
	info->route_type = g_variant_get_int32(item);
	g_variant_unref(item);
	g_variant_unref(child);
	LOGI("route_type(%d)", info->route_type);

	/* get volume types */
	child = g_variant_get_child_value(result, 2);
	item = g_variant_get_variant(child);
	g_variant_iter_init(&iter, item);
	while (g_variant_iter_loop(&iter, "&s", &name)) {
		if (name && !strncmp(name, "none", strlen("none")))
			continue;
		/* we use volume type only for out direction */
		if (name) {
			LOGI(" volume-type : %s", name);
			info->volume_type = strdup(name);
			break;
		}
	}
	g_variant_unref(item);
	g_variant_unref(child);

	/* get availabe in-devices */
	child = g_variant_get_child_value(result, 3);
	item = g_variant_get_variant(child);
	size = g_variant_n_children(item);
	LOGI("num of avail-in-devices are %d", size);
	g_variant_iter_init(&iter, item);
	i = 0;
	while (g_variant_iter_loop(&iter, "&s", &name)) {
		if (size == 1 && !strncmp(name, "none", strlen("none"))) {
			LOGI(" in-device is [%s], skip it", name);
			break;
		}
		LOGI(" in-device name : %s", name);
		info->avail_in_devices[i++] = strdup(name);
	}
	g_variant_unref(item);
	g_variant_unref(child);

	/* get available out-devices */
	child = g_variant_get_child_value(result, 4);
	item = g_variant_get_variant(child);
	size = g_variant_n_children(item);
	LOGI("num of avail-out-devices are %d", size);
	g_variant_iter_init(&iter, item);
	i = 0;
	while (g_variant_iter_loop(&iter, "&s", &name)) {
		if (size == 1 && !strncmp(name, "none", strlen("none"))) {
			LOGI(" out-device is [%s], skip it", name);
			break;
		}
		LOGI(" out-device name : %s", name);
		info->avail_out_devices[i++] = strdup(name);
	}
	g_variant_unref(item);
	g_variant_unref(child);

	/* get available frameworks */
	child = g_variant_get_child_value(result, 5);
	item = g_variant_get_variant(child);
	size = g_variant_n_children(item);
	LOGI("num of avail-frameworks are %d", size);
	g_variant_iter_init(&iter, item);
	i = 0;
	while (g_variant_iter_loop(&iter, "&s", &name)) {
		if (size == 1 && !strncmp(name, "none", strlen("none"))) {
			LOGI(" framework is [%s], skip it", name);
			break;
		}
		LOGI(" framework name : %s", name);
		info->avail_frameworks[i++] = strdup(name);
	}
	g_variant_unref(item);
	g_variant_unref(child);
	g_variant_unref(result);

	if (info->priority == -1) {
		LOGE("could not find the info of stream type(%s)", stream_type);
		ret = MM_ERROR_NOT_SUPPORT_API;
	}

LEAVE:
	g_object_unref(conn);

	return ret;
}

int _set_manual_route_info(unsigned int index, manual_route_info_s *info)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	GVariantBuilder *builder_for_in_devices;
	GVariantBuilder *builder_for_out_devices;
	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;
	const gchar *dbus_ret = NULL;

	assert(info);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free(err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	LOGI("index(%u)", index);

	builder_for_in_devices = g_variant_builder_new(G_VARIANT_TYPE("au"));
	builder_for_out_devices = g_variant_builder_new(G_VARIANT_TYPE("au"));
	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (!info->route_in_devices[i])
			break;
		g_variant_builder_add(builder_for_in_devices, "u", info->route_in_devices[i]);
		LOGI("[IN] device_id:%u", info->route_in_devices[i]);
	}
	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (!info->route_out_devices[i])
			break;
		g_variant_builder_add(builder_for_out_devices, "u", info->route_out_devices[i]);
		LOGI("[OUT] device_id:%u", info->route_out_devices[i]);
	}

	result = g_dbus_connection_call_sync(conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_SET_STREAM_ROUTE_DEVICES,
							g_variant_new("(uauau)", index, builder_for_in_devices, builder_for_out_devices),
							G_VARIANT_TYPE("(s)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result) {
		LOGE("g_dbus_connection_call_sync() for SET_STREAM_ROUTE_DEVICES error (%s)", err->message);
		g_error_free(err);
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	g_variant_get(result, "(&s)", &dbus_ret);
	LOGI("g_dbus_connection_call_sync() success, method return value is (%s)", dbus_ret);
	if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret)))
		ret = MM_ERROR_SOUND_INVALID_STATE;
	else
		info->is_set = true;

	g_variant_unref(result);
	g_variant_builder_unref(builder_for_in_devices);
	g_variant_builder_unref(builder_for_out_devices);
LEAVE:
	g_object_unref(conn);

	return ret;
}

int _set_route_option(unsigned int index, const char *name, int value)
{
	int ret = MM_ERROR_NONE;
	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;
	const gchar *dbus_ret = NULL;

	assert(name);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn && err) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free(err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	LOGI("[OPTION] %s(%d)", name, value);

	result = g_dbus_connection_call_sync(conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_SET_STREAM_ROUTE_OPTION,
							g_variant_new("(usi)", index, name, value),
							G_VARIANT_TYPE("(s)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result) {
		LOGE("g_dbus_connection_call_sync() for SET_STREAM_ROUTE_OPTION error (%s)", err->message);
		g_error_free(err);
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	g_variant_get(result, "(&s)", &dbus_ret);
	LOGI("g_dbus_connection_call_sync() success, method return value is (%s)", dbus_ret);
	if (!strncmp("STREAM_MANAGER_RETURN_ERROR_NO_STREAM", dbus_ret, strlen(dbus_ret)))
		ret = MM_ERROR_SOUND_INVALID_STATE;
	else if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret)))
		ret = MM_ERROR_SOUND_INTERNAL;

	g_variant_unref(result);
LEAVE:
	g_object_unref(conn);

	return ret;
}

int _get_volume_max_level(const char *direction, const char *volume_type, unsigned int *max_level)
{
	int ret = MM_ERROR_NONE;
	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;
	const gchar *dbus_ret = NULL;

	assert(direction);
	assert(volume_type);
	assert(max_level);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free(err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	result = g_dbus_connection_call_sync(conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_GET_VOLUME_MAX_LEVEL,
							g_variant_new("(ss)", direction, volume_type),
							G_VARIANT_TYPE("(us)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result) {
		LOGE("g_dbus_connection_call_sync() for GET_VOLUME_MAX_LEVEL error (%s)", err->message);
		g_error_free(err);
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	g_variant_get(result, "(u&s)", max_level, &dbus_ret);
	LOGI("g_dbus_connection_call_sync() success, method return value is (%u, %s)", *max_level, dbus_ret);
	if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret)))
		ret = MM_ERROR_SOUND_INTERNAL;

	g_variant_unref(result);
LEAVE:
	g_object_unref(conn);

	return ret;
}

int _get_volume_level(const char *direction, const char *volume_type, unsigned int *level)
{
	int ret = MM_ERROR_NONE;
	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;
	const gchar *dbus_ret = NULL;

	assert(direction);
	assert(volume_type);
	assert(level);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free(err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	result = g_dbus_connection_call_sync(conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_GET_VOLUME_LEVEL,
							g_variant_new("(ss)", direction, volume_type),
							G_VARIANT_TYPE("(us)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result) {
		LOGE("g_dbus_connection_call_sync() for GET_VOLUME_LEVEL error (%s)", err->message);
		g_error_free(err);
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	g_variant_get(result, "(u&s)", level, &dbus_ret);
	LOGI("g_dbus_connection_call_sync() success, method return value is (%u, %s)", *level, dbus_ret);
	if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret)))
		ret = MM_ERROR_SOUND_INTERNAL;

	g_variant_unref(result);
LEAVE:
	g_object_unref(conn);

	return ret;
}

int _set_volume_level(const char *direction, const char *volume_type, unsigned int level)
{
	int ret = MM_ERROR_NONE;
	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;
	const gchar *dbus_ret = NULL;
	int vret = 0;
	char volume_path[VCONF_PATH_MAX] = {0,};

	assert(direction);
	assert(volume_type);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free(err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	result = g_dbus_connection_call_sync(conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_SET_VOLUME_LEVEL,
							g_variant_new("(ssu)", direction, volume_type, level),
							G_VARIANT_TYPE("(s)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result) {
		LOGE("g_dbus_connection_call_sync() for SET_VOLUME_LEVEL error (%s)", err->message);
		g_error_free(err);
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	g_variant_get(result, "(&s)", &dbus_ret);
	LOGI("g_dbus_connection_call_sync() success, method return value is (%s)", dbus_ret);
	if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret)))
		ret = MM_ERROR_SOUND_INTERNAL;
	else {
		/* Set volume value to VCONF */
		snprintf(volume_path, sizeof(volume_path)-1, "%s%s", VCONF_PATH_PREFIX_VOLUME, volume_type);
		if ((vret = vconf_set_int(volume_path, (int)level)))
			LOGE("vconf_set_int(%s) failed..ret[%d]\n", volume_path, vret);
	}
	g_variant_unref(result);
LEAVE:
	g_object_unref(conn);

	return ret;
}

int _get_current_volume_type(const char *direction, char **volume_type)
{
	int ret = MM_ERROR_NONE;
	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;
	const gchar *dbus_volume_type = NULL;
	const gchar *dbus_ret = NULL;

	assert(direction);
	assert(volume_type);

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free(err);
		return MM_ERROR_SOUND_INTERNAL;
	}

	result = g_dbus_connection_call_sync(conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_GET_CURRENT_VOLUME_TYPE,
							g_variant_new("(s)", direction),
							G_VARIANT_TYPE("(ss)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result) {
		LOGE("g_dbus_connection_call_sync() for GET_CURRENT_VOLUME_TYPE error (%s)", err->message);
		g_error_free(err);
		ret = MM_ERROR_SOUND_INTERNAL;
		goto LEAVE;
	}

	g_variant_get(result, "(&s&s)", &dbus_volume_type, &dbus_ret);
	LOGI("g_dbus_connection_call_sync() success, method return value is (%s, %s)", dbus_volume_type, dbus_ret);
	if (!strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret))) {
		ret = MM_ERROR_NONE;
		*volume_type = strdup(dbus_volume_type);
	} else if (!strncmp("STREAM_MANAGER_RETURN_ERROR_NO_STREAM", dbus_ret, strlen(dbus_ret)))
		ret = MM_ERROR_SOUND_VOLUME_NO_INSTANCE;
	else
		ret = MM_ERROR_SOUND_INTERNAL;

	g_variant_unref(result);
LEAVE:
	g_object_unref(conn);

	return ret;
}

void _update_focus_status(unsigned int index, unsigned int acquired_focus_status)
{
	GVariant *result = NULL;
	GDBusConnection *conn = NULL;
	GError *err = NULL;
	const gchar *dbus_ret = NULL;

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &err);
	if (!conn) {
		LOGE("g_bus_get_sync() error (%s)", err->message);
		g_error_free(err);
		return;
	}

	result = g_dbus_connection_call_sync(conn,
							PA_BUS_NAME,
							PA_STREAM_MANAGER_OBJECT_PATH,
							PA_STREAM_MANAGER_INTERFACE,
							PA_STREAM_MANAGER_METHOD_NAME_UPDATE_FOCUS_STATUS,
							g_variant_new("(uu)", index, acquired_focus_status),
							G_VARIANT_TYPE("(s)"),
							G_DBUS_CALL_FLAGS_NONE,
							2000,
							NULL,
							&err);
	if (!result) {
		LOGE("g_dbus_connection_call_sync() for UPDATE_FOCUS_STATUS error (%s)", err->message);
		g_error_free(err);
		goto LEAVE;
	}
	g_variant_get(result, "(&s)", &dbus_ret);
	LOGI("g_dbus_connection_call_sync() success, method return value is (%s)", dbus_ret);
	if (strncmp("STREAM_MANAGER_RETURN_OK", dbus_ret, strlen(dbus_ret)))
		LOGE("failed to UPDATE_FOCUS_STATUS error (%s)", dbus_ret);

	g_variant_unref(result);
LEAVE:
	g_object_unref(conn);

	return;
}

void _focus_session_interrupt_cb(mm_sound_focus_state_e state, const char *reason, bool is_wcb, void *user_data)
{
	sound_session_interrupted_code_e e;
	LOGE("session interrupted by [%s]", reason);
	if (g_session_interrupt_cb_table.user_cb) {
		if (is_wcb) {
			if (state == FOCUS_IS_RELEASED)
				e = SOUND_SESSION_INTERRUPTED_COMPLETED;
			else
				_convert_stream_type_to_interrupt_reason(reason, &e);
		} else {
			if (state == FOCUS_IS_ACQUIRED)
				e = SOUND_SESSION_INTERRUPTED_COMPLETED;
			else
				_convert_stream_type_to_interrupt_reason(reason, &e);
		}
		g_session_interrupt_cb_table.user_cb(e, g_session_interrupt_cb_table.user_data);
	}

	return;
}

void _device_connected_cb(sound_device_h device, bool is_connected, void *user_data)
{
	mm_sound_device_type_e type;
	if (mm_sound_get_device_type(device, &type) != MM_ERROR_NONE) {
		LOGE("getting device type failed");
		return;
	}

	switch (type) {
	case MM_SOUND_DEVICE_TYPE_AUDIOJACK:
	case MM_SOUND_DEVICE_TYPE_BLUETOOTH:
	case MM_SOUND_DEVICE_TYPE_HDMI:
	case MM_SOUND_DEVICE_TYPE_MIRRORING:
	case MM_SOUND_DEVICE_TYPE_USB_AUDIO:
		if (!is_connected) {
			LOGI("sound device unplugged");
			g_session_interrupt_cb_table.user_cb(SOUND_SESSION_INTERRUPTED_BY_EARJACK_UNPLUG, g_session_interrupt_cb_table.user_data);
		}
		break;
	default:
		break;
	}

	return;
}

/* This is an internal callback for the VOIP SESSION */
void _voip_focus_state_change_callback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason, const char *extra_info, void *user_data)
{
	sound_stream_info_s *info = (sound_stream_info_s *)stream_info;
	LOGI(">> enter, stream_info(%p), change_reason(%d), extra_info(%s)", stream_info, reason, extra_info);

	if (!info) {
		LOGE("stream info is null");
		return;
	}

	if (!(info->acquired_focus & SOUND_STREAM_FOCUS_FOR_PLAYBACK) ||
		!(info->acquired_focus & SOUND_STREAM_FOCUS_FOR_RECORDING)) {
		/* stop virtual stream for rintone-voip or voip */
		if (g_voip_vstream_h) {
			/* stop virtual stream handle */
			_stop_virtual_stream(g_voip_vstream_h);
			/* destroy virtual stream handle */
			_destroy_virtual_stream(g_voip_vstream_h);
			g_voip_vstream_h = NULL;
			LOGW("internal voip stream is interrupted by (%d)", reason);
		}
	}

	LOGI("<< leave");

	return;
}

int _set_session_mode(_session_mode_e mode)
{
	int ret = MM_ERROR_NONE;
	int w_ret = MM_ERROR_NONE;
	int i_ret = SOUND_MANAGER_ERROR_NONE;
	bool is_found = false;
	MMSoundDeviceList_t device_list;
	MMSoundDevice_t tmp_device = NULL;
	MMSoundDevice_t proper_device = NULL;
	MMSoundDevice_t prev_device = NULL;
	mm_sound_device_type_e type;
	int id = -1;

	if (g_cached_session_mode == mode) {
		LOGW("session_mode(%d) is same as before", mode);
		return ret;
	}

	switch (mode) {
	case _SESSION_MODE_RINGTONE:
		if (g_voip_vstream_h) {
			/* stop vstream and destroy vstream */
			_stop_virtual_stream(g_voip_vstream_h);
			_destroy_virtual_stream(g_voip_vstream_h);
			g_voip_vstream_h = NULL;
			/* destroy stream info */
			_destroy_pa_connection_and_unregister_focus(g_voip_stream_info);
			free(g_voip_stream_info);
			g_voip_stream_info = NULL;
		}
		if (!g_voip_stream_info) {
			g_voip_stream_info = malloc(sizeof(sound_stream_info_s));
			if (!g_voip_stream_info) {
				ret = MM_ERROR_OUT_OF_MEMORY;
				goto ERROR_CASE;
			}
			memset(g_voip_stream_info, 0, sizeof(sound_stream_info_s));
		}
		/* create stream info and acquire focus for rintone-voip stream */
		g_voip_stream_info->stream_type = "ringtone-voip";
		ret = _make_pa_connection_and_register_focus(g_voip_stream_info, _voip_focus_state_change_callback, NULL);
		if (ret != MM_ERROR_NONE) {
			free(g_voip_stream_info);
			g_voip_stream_info = NULL;
			goto ERROR_CASE;
		} else {
			/* acquire focus */
			ret = mm_sound_acquire_focus(g_voip_stream_info->index, FOCUS_FOR_BOTH, "for voip session");
			if (ret == MM_ERROR_NONE) {
				g_voip_stream_info->acquired_focus |= FOCUS_FOR_BOTH;
				_update_focus_status(g_voip_stream_info->index, (unsigned int)g_voip_stream_info->acquired_focus);
			} else
				goto ERROR_CASE;

		}
		/* create virtual stream for ringtone-voip */
		i_ret = _create_virtual_stream(g_voip_stream_info, &g_voip_vstream_h);
		if (i_ret == SOUND_MANAGER_ERROR_NONE) {
			i_ret = _start_virtual_stream(g_voip_vstream_h);
			if (i_ret != SOUND_MANAGER_ERROR_NONE)
				goto ERROR_CASE;
		} else {
			ret = MM_ERROR_SOUND_INTERNAL;
			goto ERROR_CASE;
		}
		break;
	case _SESSION_MODE_VOICE_WITH_BUILTIN_RECEIVER: /* Built-in RCV and Built-in MIC */
	case _SESSION_MODE_VOICE_WITH_BUILTIN_SPEAKER:  /* Built-in SPK and Built-in MIC */
	case _SESSION_MODE_VOICE_WITH_AUDIO_JACK:       /* Earphone spk & mic */
	case _SESSION_MODE_VOICE_WITH_BLUETOOTH:        /* Bluetooth spk & mic */
		/* check if the device is available now */
		ret = mm_sound_get_current_device_list(MM_SOUND_DEVICE_ALL_FLAG, &device_list);
		if (ret != MM_ERROR_NONE) {
			LOGE("failed to get current device list");
			goto ERROR_CASE;
		} else {
			while (!is_found && ((w_ret = mm_sound_get_next_device(device_list, &tmp_device)) == MM_ERROR_NONE)) {
				ret = mm_sound_get_device_type(tmp_device, &type);
				if (ret != MM_ERROR_NONE)
					goto ERROR_CASE;
				ret = mm_sound_get_device_id(tmp_device, &id);
				if (ret != MM_ERROR_NONE)
					goto ERROR_CASE;
				else {
					if (g_cached_voip_device_id != -1 && g_cached_voip_device_id == id)
						prev_device = tmp_device;
				}
				switch (mode) {
				case _SESSION_MODE_VOICE_WITH_BUILTIN_RECEIVER:
					if (type == MM_SOUND_DEVICE_TYPE_BUILTIN_RECEIVER) {
						is_found = true;
						proper_device = tmp_device;
						break;
					}
				case _SESSION_MODE_VOICE_WITH_BUILTIN_SPEAKER:
					if (type == MM_SOUND_DEVICE_TYPE_BUILTIN_SPEAKER) {
						is_found = true;
						proper_device = tmp_device;
						break;
					}
				case _SESSION_MODE_VOICE_WITH_AUDIO_JACK:
					if (type == MM_SOUND_DEVICE_TYPE_AUDIOJACK) {
						is_found = true;
						proper_device = tmp_device;
						break;
					}
					break;
				case _SESSION_MODE_VOICE_WITH_BLUETOOTH:
					if (type == MM_SOUND_DEVICE_TYPE_BLUETOOTH) {
						is_found = true;
						proper_device = tmp_device;
						break;
					}
				default:
					break;
				}
			}
			if (!is_found) {
				LOGE("could not found a proper connected device for this mode(%d)", mode);
				ret = MM_ERROR_SOUND_INTERNAL;
				goto ERROR_CASE_NO_DESTROY;
			}
		}
		/* if ok for the device, go below */
		if (g_cached_session_mode == _SESSION_MODE_RINGTONE) {
			/* stop vstream and destroy vstream */
			_stop_virtual_stream(g_voip_vstream_h);
			_destroy_virtual_stream(g_voip_vstream_h);
			g_voip_vstream_h = NULL;
			/* destroy stream info */
			_destroy_pa_connection_and_unregister_focus(g_voip_stream_info);
			free(g_voip_stream_info);
			g_voip_stream_info = NULL;
		}
		/* create stream info and acquire focus for voip stream */
		if (g_cached_session_mode == -1 || g_cached_session_mode == _SESSION_MODE_RINGTONE) {
			if (!g_voip_stream_info) {
				g_voip_stream_info = malloc(sizeof(sound_stream_info_s));
				if (!g_voip_stream_info) {
					ret = MM_ERROR_OUT_OF_MEMORY;
					goto ERROR_CASE;
				}
				memset(g_voip_stream_info, 0, sizeof(sound_stream_info_s));
			}
			g_voip_stream_info->stream_type = "voip";
			ret = _make_pa_connection_and_register_focus(g_voip_stream_info, _voip_focus_state_change_callback, NULL);
			if (ret != MM_ERROR_NONE) {
				free(g_voip_stream_info);
				g_voip_stream_info = NULL;
				goto ERROR_CASE;
			}
			/* set device for routing to the device */
			i_ret = _add_device_for_stream_routing(g_voip_stream_info, proper_device);
			if (i_ret == SOUND_MANAGER_ERROR_NONE) {
				i_ret = _apply_stream_routing(g_voip_stream_info);
				if (i_ret != SOUND_MANAGER_ERROR_NONE)
					goto ERROR_CASE;
			} else
				goto ERROR_CASE;

			/* acquire focus */
			ret = mm_sound_acquire_focus(g_voip_stream_info->index, FOCUS_FOR_BOTH, "for voip session");
			if (ret == MM_ERROR_NONE) {
				g_voip_stream_info->acquired_focus |= FOCUS_FOR_BOTH;
				_update_focus_status(g_voip_stream_info->index, (unsigned int)g_voip_stream_info->acquired_focus);
			} else
				goto ERROR_CASE;

			/* create virtual stream for voip */
			i_ret = _create_virtual_stream(g_voip_stream_info, &g_voip_vstream_h);
			if (i_ret == SOUND_MANAGER_ERROR_NONE) {
				i_ret = _start_virtual_stream(g_voip_vstream_h);
				if (i_ret != SOUND_MANAGER_ERROR_NONE)
					goto ERROR_CASE;
			} else
				goto ERROR_CASE;

		} else {
			if (g_cached_voip_device_id != -1 && prev_device) {
				/* remove cached device */
				i_ret = _remove_device_for_stream_routing(g_voip_stream_info, prev_device);
				if (i_ret != SOUND_MANAGER_ERROR_NONE)
					goto ERROR_CASE;
				/* set device for routing to the device */
				i_ret = _add_device_for_stream_routing(g_voip_stream_info, proper_device);
				if (i_ret == SOUND_MANAGER_ERROR_NONE) {
					i_ret = _apply_stream_routing(g_voip_stream_info);
					if (i_ret != SOUND_MANAGER_ERROR_NONE)
						goto ERROR_CASE;
				} else
					goto ERROR_CASE;
			} else
				goto ERROR_CASE;
		}
		mm_sound_free_device_list(device_list);
		break;
	}
	g_cached_session_mode = mode;
	g_cached_voip_device_id = id;

	return ret;
ERROR_CASE:
	if (g_voip_vstream_h) {
		/* destroy virtual stream handle */
		_destroy_virtual_stream(g_voip_vstream_h);
		g_voip_vstream_h = NULL;
		ret = MM_ERROR_SOUND_INTERNAL;
	}
	if (g_voip_stream_info) {
		/* destroy stream info */
		_destroy_pa_connection_and_unregister_focus(g_voip_stream_info);
		free(g_voip_stream_info);
		g_voip_stream_info = NULL;
	}
ERROR_CASE_NO_DESTROY:
	return ret;
}

int _make_pa_connection_and_register_focus(sound_stream_info_s *stream_h, sound_stream_focus_state_changed_cb callback, void *user_data)
{
	int ret = MM_ERROR_NONE;
	int pa_ret = PA_OK;
	int i = 0;
	bool is_focus_cb_thread = false;

	if ((ret = mm_sound_focus_is_cb_thread(&is_focus_cb_thread)))
		return ret;

	if (is_focus_cb_thread)
		return MM_ERROR_SOUND_INVALID_OPERATION;

	/* get configuration information of this stream type */
	if ((ret = _get_stream_conf_info(stream_h->stream_type, &stream_h->stream_conf_info)))
		return ret;

	LOGI("stream_conf_info : stream type[%s], priority[%d], route type[%d]",
		stream_h->stream_type, stream_h->stream_conf_info.priority, stream_h->stream_conf_info.route_type);

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

	/* get index of this context */
	stream_h->index = pa_context_get_index(stream_h->pa_context);

	pa_threaded_mainloop_unlock(stream_h->pa_mainloop);

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
			goto PA_ERROR;
		}
	} else {
		LOGE("failed to register focus, ret(0x%x)", ret);
		/* disconnect */
		goto PA_ERROR;
	}
	goto SUCCESS;
PA_ERROR_WITH_UNLOCK:
	pa_threaded_mainloop_unlock(stream_h->pa_mainloop);
PA_ERROR:
	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (stream_h->stream_conf_info.avail_in_devices[i])
			free(stream_h->stream_conf_info.avail_in_devices[i]);
		else
			break;
	}
	for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
		if (stream_h->stream_conf_info.avail_out_devices[i])
			free(stream_h->stream_conf_info.avail_out_devices[i]);
		else
			break;
	}
	for (i = 0; i < AVAIL_FRAMEWORKS_MAX; i++) {
		if (stream_h->stream_conf_info.avail_frameworks[i])
			free(stream_h->stream_conf_info.avail_frameworks[i]);
		else
			break;
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
	ret = MM_ERROR_SOUND_INTERNAL;
	LOGE("pa_ret(%d), ret(0x%x)", pa_ret, ret);

SUCCESS:
	return ret;
}

int _destroy_pa_connection_and_unregister_focus(sound_stream_info_s *stream_h)
{
	int i = 0;
	int ret = MM_ERROR_NONE;
	bool is_focus_cb_thread = false;

	ret = mm_sound_focus_is_cb_thread(&is_focus_cb_thread);
	if (ret)
		return ret;

	if (is_focus_cb_thread)
		return MM_ERROR_SOUND_INVALID_OPERATION;

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
		if (stream_h->stream_conf_info.avail_in_devices[i])
			free(stream_h->stream_conf_info.avail_in_devices[i]);
		if (stream_h->stream_conf_info.avail_out_devices[i])
			free(stream_h->stream_conf_info.avail_out_devices[i]);
	}
	for (i = 0; i < AVAIL_FRAMEWORKS_MAX; i++) {
		if (stream_h->stream_conf_info.avail_frameworks[i])
			free(stream_h->stream_conf_info.avail_frameworks[i]);
	}

	for (i = 0; i < SOUND_STREAM_INFO_ARR_MAX; i++) {
		if (sound_stream_info_arr[i] && sound_stream_info_arr[i]->index == stream_h->index) {
			sound_stream_info_arr[i] = NULL;
			break;
		}
	}

	return ret;
}

int _add_device_for_stream_routing(sound_stream_info_s *stream_info, sound_device_h device)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	int j = 0;
	bool added_successfully = false;
#if 0
	/* not ready yet. after preparing in libmm-sound, it'll be enabled */
	bool use_internal_codec = false;
#endif
	char *device_type_str = NULL;
	int device_id = 0;
	mm_sound_device_type_e device_type;
	mm_sound_device_io_direction_e device_direction;

	SM_INSTANCE_CHECK_FOR_PRIV(stream_info);
	SM_NULL_ARG_CHECK_FOR_PRIV(device);

	if (stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL ||
			stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL_EXT) {
#if 0
		/* not ready yet. after preparing in libmm-sound, it'll be enabled */
		if (stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL_EXT) {
			ret = mm_sound_get_device_use_internal_codec(device, &use_internal_codec);
			if (ret)
				return _convert_sound_manager_error_code(__func__, ret);
			if (use_internal_codec)
				return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_INTERNAL);
		}
#endif
		ret = mm_sound_get_device_id(device, &device_id);
		if (ret)
			return _convert_sound_manager_error_code(__func__, ret);
		ret = mm_sound_get_device_type(device, &device_type);
		if (ret)
			return _convert_sound_manager_error_code(__func__, ret);
		ret = _convert_device_type(device_type, &device_type_str);
		if (ret)
			return _convert_sound_manager_error_code(__func__, ret);
		ret = mm_sound_get_device_io_direction(device, &device_direction);
		if (ret)
			return _convert_sound_manager_error_code(__func__, ret);

		if (device_direction == MM_SOUND_DEVICE_IO_DIRECTION_IN || device_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
			for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
				if (!stream_info->stream_conf_info.avail_in_devices[i])
					break;

				if (!strncmp(stream_info->stream_conf_info.avail_in_devices[i], device_type_str, SOUND_DEVICE_TYPE_LEN)) {
					for (j = 0; j < AVAIL_DEVICES_MAX; j++) {
						if (!stream_info->manual_route_info.route_in_devices[j]) {
							stream_info->manual_route_info.route_in_devices[j] = (unsigned int)device_id;
							added_successfully = true;
							break;
						}
						if (stream_info->manual_route_info.route_in_devices[j] == (unsigned int)device_id) {
							/* it was already set */
							return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_DUPLICATED);
						}
					}
				}
			}
		}
		if (device_direction == MM_SOUND_DEVICE_IO_DIRECTION_OUT || device_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
			for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
				if (!stream_info->stream_conf_info.avail_out_devices[i])
					break;

				if (!strncmp(stream_info->stream_conf_info.avail_out_devices[i], device_type_str, SOUND_DEVICE_TYPE_LEN)) {
					for (j = 0; j < AVAIL_DEVICES_MAX; j++) {
						if (!stream_info->manual_route_info.route_out_devices[j]) {
							stream_info->manual_route_info.route_out_devices[j] = (unsigned int)device_id;
							added_successfully = true;
							break;
						}
						if (stream_info->manual_route_info.route_out_devices[j] == (unsigned int)device_id) {
							/* it was already set */
							return _convert_sound_manager_error_code(__func__, MM_ERROR_POLICY_DUPLICATED);
						}
					}
				}
			}
		}
	}

	if (!added_successfully)
		ret = MM_ERROR_POLICY_INTERNAL;

	return ret;
}

int _remove_device_for_stream_routing(sound_stream_info_s *stream_info, sound_device_h device)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	int j = 0;
	bool removed_successfully = false;
	char *device_type_str = NULL;
	int device_id = 0;
	mm_sound_device_type_e device_type;
	mm_sound_device_io_direction_e device_direction;

	SM_INSTANCE_CHECK_FOR_PRIV(stream_info);
	SM_NULL_ARG_CHECK_FOR_PRIV(device);

	if (stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL ||
			stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL_EXT) {
		ret = mm_sound_get_device_id(device, &device_id);
		if (ret)
			return _convert_sound_manager_error_code(__func__, ret);
		ret = mm_sound_get_device_type(device, &device_type);
		if (ret)
			return _convert_sound_manager_error_code(__func__, ret);
		ret = _convert_device_type(device_type, &device_type_str);
		ret = mm_sound_get_device_io_direction(device, &device_direction);
		if (ret)
			return _convert_sound_manager_error_code(__func__, ret);

		if (device_direction == MM_SOUND_DEVICE_IO_DIRECTION_IN || device_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
			for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
				if (!stream_info->stream_conf_info.avail_in_devices[i])
					break;

				if (!strncmp(stream_info->stream_conf_info.avail_in_devices[i], device_type_str, SOUND_DEVICE_TYPE_LEN)) {
					for (j = 0; j < AVAIL_DEVICES_MAX; j++) {
						if (stream_info->manual_route_info.route_in_devices[j] == (unsigned int)device_id) {
							removed_successfully = true;
							stream_info->manual_route_info.route_in_devices[j] = 0;
							break;
						}
					}
				}
			}
		}
		if (device_direction == MM_SOUND_DEVICE_IO_DIRECTION_OUT || device_direction == MM_SOUND_DEVICE_IO_DIRECTION_BOTH) {
			for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
				if (!stream_info->stream_conf_info.avail_out_devices[i])
					break;

				if (!strncmp(stream_info->stream_conf_info.avail_out_devices[i], device_type_str, SOUND_DEVICE_TYPE_LEN)) {
					for (j = 0; j < AVAIL_DEVICES_MAX; j++) {
						if (stream_info->manual_route_info.route_out_devices[j] == (unsigned int)device_id) {
							removed_successfully = true;
							stream_info->manual_route_info.route_out_devices[j] = 0;
							break;
						}
					}
				}
			}
		}
	}

	if (!removed_successfully)
		ret = MM_ERROR_INVALID_ARGUMENT;

	return ret;
}

int _apply_stream_routing(sound_stream_info_s *stream_info)
{
	int ret = MM_ERROR_NONE;
	int i = 0;
	bool need_to_apply = false;

	SM_INSTANCE_CHECK_FOR_PRIV(stream_info);

	if (stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL ||
			stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL_EXT) {
		for (i = 0; i < AVAIL_DEVICES_MAX; i++) {
			if (stream_info->manual_route_info.route_in_devices[i]) {
				need_to_apply = true;
				break;
			}
			if (stream_info->manual_route_info.route_out_devices[i]) {
				need_to_apply = true;
				break;
			}
		}
		if (need_to_apply)
			ret = _set_manual_route_info(stream_info->index, &stream_info->manual_route_info);
		else
			_convert_sound_manager_error_code(__func__, MM_ERROR_SOUND_INVALID_STATE);

	} else
		ret = MM_ERROR_SOUND_INVALID_STATE;

	return ret;
}

int _create_virtual_stream(sound_stream_info_s *stream_info, virtual_sound_stream_info_s **virtual_stream)
{
	int ret = MM_ERROR_NONE;
	bool result = false;
	const char *name = NULL;
	int i = 0;

	SM_INSTANCE_CHECK_FOR_PRIV(virtual_stream);
	SM_INSTANCE_CHECK_FOR_PRIV(stream_info);

	/* check if this stream_info is available for virtual stream */
	name = _convert_api_name(NATIVE_API_SOUND_MANAGER);
	for (i = 0; i < AVAIL_FRAMEWORKS_MAX; i++) {
		if (stream_info->stream_conf_info.avail_frameworks[i] && !strncmp(stream_info->stream_conf_info.avail_frameworks[i], name, strlen(name))) {
			result = true;
			break;
		}
	}
	LOGI("stream_type[%s], native api[%s], is_available[%d]", stream_info->stream_type, name, result);
	if (result == false) {
		ret = MM_ERROR_NOT_SUPPORT_API;
		goto LEAVE;
	}

	(*virtual_stream) = malloc(sizeof(virtual_sound_stream_info_s));
	if (!(*virtual_stream)) {
		ret = MM_ERROR_OUT_OF_MEMORY;
		goto LEAVE;
	}

	memset((*virtual_stream), 0, sizeof(virtual_sound_stream_info_s));
	(*virtual_stream)->stream_type = stream_info->stream_type;
	(*virtual_stream)->pa_mainloop = stream_info->pa_mainloop;
	(*virtual_stream)->pa_context = stream_info->pa_context;
	(*virtual_stream)->pa_proplist = pa_proplist_new();
	pa_proplist_sets((*virtual_stream)->pa_proplist, PA_PROP_MEDIA_ROLE, (*virtual_stream)->stream_type);
	pa_proplist_setf((*virtual_stream)->pa_proplist, PA_PROP_MEDIA_PARENT_ID, "%u", stream_info->index);
	(*virtual_stream)->state = _VSTREAM_STATE_READY;
	(*virtual_stream)->stream_info = stream_info;

LEAVE:
	return ret;
}

int _destroy_virtual_stream(virtual_sound_stream_info_s *virtual_stream)
{
	int ret = MM_ERROR_NONE;

	SM_INSTANCE_CHECK_FOR_PRIV(virtual_stream);
	SM_STATE_CHECK_FOR_PRIV(virtual_stream, _VSTREAM_STATE_READY);

	virtual_stream->pa_mainloop = NULL;
	virtual_stream->pa_context = NULL;
	if (virtual_stream->pa_proplist)
		pa_proplist_free(virtual_stream->pa_proplist);

	free(virtual_stream);
	virtual_stream = NULL;

	return ret;
}

int _start_virtual_stream(virtual_sound_stream_info_s *virtual_stream)
{
	int ret = MM_ERROR_NONE;
	int pa_ret = PA_OK;
	int i = 0;
	int io_direction = 0;
	pa_sample_spec ss;
	pa_channel_map maps;

	SM_INSTANCE_CHECK_FOR_PRIV(virtual_stream);
	SM_STATE_CHECK_FOR_PRIV(virtual_stream, _VSTREAM_STATE_READY);

	if (virtual_stream->stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL ||
			virtual_stream->stream_info->stream_conf_info.route_type == STREAM_ROUTE_TYPE_MANUAL_EXT) {
		/* check if the manual route info. is set when it comes to the manual route type */
		if (virtual_stream->stream_info->manual_route_info.is_set == false) {
			ret = MM_ERROR_SOUND_INVALID_STATE;
			goto ERROR;
		}
	}

	/* fill up with default value */
	ss.channels = 2;
	ss.rate = 44100;
	ss.format = PA_SAMPLE_S16LE;
	pa_channel_map_init_auto(&maps, ss.channels, PA_CHANNEL_MAP_ALSA);

	/* check direction of this stream */
	if (virtual_stream->stream_info->stream_conf_info.avail_in_devices[0] != NULL)
		io_direction |= SOUND_STREAM_DIRECTION_INPUT;
	if (virtual_stream->stream_info->stream_conf_info.avail_out_devices[0] != NULL)
		io_direction |= SOUND_STREAM_DIRECTION_OUTPUT;

	/* LOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_lock(virtual_stream->pa_mainloop);

	for (i = 0; i < SOUND_STREAM_DIRECTION_MAX; i++) {
		if (io_direction & (i + 1)) {
			virtual_stream->pa_stream[i] = pa_stream_new_with_proplist(virtual_stream->pa_context, "VIRTUAL_STREAM", &ss, &maps, virtual_stream->pa_proplist);
			if (virtual_stream->pa_stream[i] == NULL) {
				LOGE("failed to pa_stream_new_with_proplist()");
				pa_ret = pa_context_errno(virtual_stream->pa_context);
				ret = MM_ERROR_SOUND_INTERNAL;
				goto ERROR_WITH_UNLOCK;
			}
			pa_stream_set_state_callback(virtual_stream->pa_stream[i], _pa_stream_state_cb, virtual_stream);

			if ((i + 1) == SOUND_STREAM_DIRECTION_OUTPUT) {
				pa_ret = pa_stream_connect_playback(virtual_stream->pa_stream[i], NULL, NULL, 0, NULL, NULL);
				if (pa_ret < 0) {
					LOGE("failed to pa_stream_connect_playback()");
					pa_ret = pa_context_errno(virtual_stream->pa_context);
					ret = MM_ERROR_SOUND_INTERNAL;
					goto ERROR_WITH_UNLOCK;
				}
			} else if ((i + 1) == SOUND_STREAM_DIRECTION_INPUT) {
				pa_ret = pa_stream_connect_record(virtual_stream->pa_stream[i], NULL, NULL, 0);
				if (pa_ret < 0) {
					LOGE("failed to pa_stream_connect_record()");
					pa_ret = pa_context_errno(virtual_stream->pa_context);
					ret = MM_ERROR_SOUND_INTERNAL;
					goto ERROR_WITH_UNLOCK;
				}
			}

			/* wait for ready state of the stream */
			for (;;) {
				pa_stream_state_t state;
				state = pa_stream_get_state(virtual_stream->pa_stream[i]);
				if (state == PA_STREAM_READY)
					break;
				if (!PA_STREAM_IS_GOOD(state)) {
					LOGE("stream(%d) is not good, state : %d", i, state);
					pa_ret = pa_context_errno(virtual_stream->pa_context);
					ret = MM_ERROR_SOUND_INTERNAL;
					goto ERROR_WITH_UNLOCK;
				}

				pa_threaded_mainloop_wait(virtual_stream->pa_mainloop);
			}
		}
	}
	virtual_stream->state = _VSTREAM_STATE_RUNNING;

	/* UNLOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_unlock(virtual_stream->pa_mainloop);

	return ret;

ERROR_WITH_UNLOCK:
	/* UNLOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_unlock(virtual_stream->pa_mainloop);

	for (i = 0; i < SOUND_STREAM_DIRECTION_MAX; i++) {
		if (virtual_stream->pa_stream[i]) {
			pa_stream_unref(virtual_stream->pa_stream[i]);
			virtual_stream->pa_stream[i] = NULL;
		}
	}
	LOGE("pa_ret(%d)", pa_ret);

ERROR:
	return ret;
}

int _stop_virtual_stream(virtual_sound_stream_info_s *virtual_stream)
{
	int ret = MM_ERROR_NONE;
	int i = 0;

	SM_INSTANCE_CHECK_FOR_PRIV(virtual_stream);
	SM_STATE_CHECK_FOR_PRIV(virtual_stream, _VSTREAM_STATE_RUNNING);

	/* LOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_lock(virtual_stream->pa_mainloop);

	for (i = 0; i < SOUND_STREAM_DIRECTION_MAX; i++) {
		if (virtual_stream->pa_stream[i]) {
			pa_stream_disconnect(virtual_stream->pa_stream[i]);

			/* wait for terminated state of the stream */
			for (;;) {
				pa_stream_state_t state;
				state = pa_stream_get_state(virtual_stream->pa_stream[i]);
				if (state == PA_STREAM_TERMINATED)
					break;
				pa_threaded_mainloop_wait(virtual_stream->pa_mainloop);
			}

			pa_stream_unref(virtual_stream->pa_stream[i]);
			virtual_stream->pa_stream[i] = NULL;
		}
	}

	/* UNLOCK the pa_threaded_mainloop */
	pa_threaded_mainloop_unlock(virtual_stream->pa_mainloop);

	virtual_stream->state = _VSTREAM_STATE_READY;

	return ret;
}
