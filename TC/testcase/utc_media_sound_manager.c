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




#include <tet_api.h>
#include <media/sound_manager.h>

enum {
	POSITIVE_TC_IDX = 0x01,
	NEGATIVE_TC_IDX,
};

static void startup(void);
static void cleanup(void);

void (*tet_startup)(void) = startup;
void (*tet_cleanup)(void) = cleanup;

static void utc_media_sound_manager_get_max_volume_p(void);
static void utc_media_sound_manager_get_max_volume_n(void);
static void utc_media_sound_manager_set_volume_p(void);
static void utc_media_sound_manager_set_volume_n(void);
static void utc_media_sound_manager_get_volume_p(void);
static void utc_media_sound_manager_get_volume_n(void);
static void utc_media_sound_manager_set_route_policy_p(void);
static void utc_media_sound_manager_set_route_policy_n(void);
static void utc_media_sound_manager_get_route_policy_p(void);
static void utc_media_sound_manager_get_route_policy_n(void);
static void utc_media_sound_manager_get_current_sound_type_p(void);
static void utc_media_sound_manager_get_current_sound_type_n(void);
static void utc_media_sound_manager_get_current_sound_device_p(void);
static void utc_media_sound_manager_get_current_sound_device_n(void);
static void utc_media_sound_manager_set_session_type_p(void);
static void utc_media_sound_manager_set_session_type_n(void);
static void utc_media_sound_manager_get_a2dp_status_p(void);
static void utc_media_sound_manager_get_a2dp_status_n(void);

static void utc_media_sound_manager_foreach_available_route_p(void);
static void utc_media_sound_manager_foreach_available_route_n(void);
static void utc_media_sound_manager_set_active_route_p(void);
static void utc_media_sound_manager_set_active_route_n(void);
static void utc_media_sound_manager_get_active_device_p(void);
static void utc_media_sound_manager_get_active_device_n(void);
static void utc_media_sound_manager_is_route_available_p(void);
static void utc_media_sound_manager_is_route_available_n(void);

static void utc_media_sound_manager_call_session_create_p(void);
static void utc_media_sound_manager_call_session_create_n(void);
static void utc_media_sound_manager_call_session_set_mode_p(void);
static void utc_media_sound_manager_call_session_set_mode_n(void);
static void utc_media_sound_manager_call_session_get_mode_p(void);
static void utc_media_sound_manager_call_session_get_mode_n(void);
static void utc_media_sound_manager_call_session_destroy_p(void);
static void utc_media_sound_manager_call_session_destroy_n(void);

struct tet_testlist tet_testlist[] = {
	{ utc_media_sound_manager_get_max_volume_p , 1 },
	{ utc_media_sound_manager_get_max_volume_n , 2 },
	{ utc_media_sound_manager_set_volume_p, 3 },
	{ utc_media_sound_manager_set_volume_n, 4 },
	{ utc_media_sound_manager_get_volume_p, 5 },
	{ utc_media_sound_manager_get_volume_n, 6 },
	{ utc_media_sound_manager_set_route_policy_p, 7 },
	{ utc_media_sound_manager_set_route_policy_n, 8 },
	{ utc_media_sound_manager_get_route_policy_p, 9 },
	{ utc_media_sound_manager_get_route_policy_n, 10 },
	{ utc_media_sound_manager_get_current_sound_type_p , 11 },
	{ utc_media_sound_manager_get_current_sound_type_n, 12 },
	{ utc_media_sound_manager_get_current_sound_device_p, 13 },
	{ utc_media_sound_manager_get_current_sound_device_n, 14 },
	{ utc_media_sound_manager_get_a2dp_status_p, 15 },
	{ utc_media_sound_manager_get_a2dp_status_n, 16 },
	{ utc_media_sound_manager_foreach_available_route_p, 17 },
	{ utc_media_sound_manager_foreach_available_route_n, 18 },
	{ utc_media_sound_manager_set_active_route_p, 19 },
	{ utc_media_sound_manager_set_active_route_n, 20 },
	{ utc_media_sound_manager_get_active_device_p, 21 },
	{ utc_media_sound_manager_get_active_device_n, 22 },
	{ utc_media_sound_manager_is_route_available_p, 23 },
	{ utc_media_sound_manager_is_route_available_n, 24 },
	{ utc_media_sound_manager_call_session_create_p, 25 },
	{ utc_media_sound_manager_call_session_create_n, 26 },
	{ utc_media_sound_manager_call_session_set_mode_p, 27 },
	{ utc_media_sound_manager_call_session_set_mode_n, 28 },
	{ utc_media_sound_manager_call_session_get_mode_p, 29 },
	{ utc_media_sound_manager_call_session_get_mode_n, 30 },
	{ utc_media_sound_manager_call_session_destroy_p, 31 },
	{ utc_media_sound_manager_call_session_destroy_n, 32 },
	{ utc_media_sound_manager_set_session_type_p, 33 },
	{ utc_media_sound_manager_set_session_type_n, 34 },
	{ NULL, 0 },
};

static void startup(void)
{
	/* start of TC */
}

static void cleanup(void)
{
	/* end of TC */
}

static void utc_media_sound_manager_get_max_volume_p(void)
{
	char* api_name = "sound_manager_get_max_volume";
	int ret, max;
	if ((ret = sound_manager_get_max_volume(SOUND_TYPE_SYSTEM, &max)) == SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_max_volume_n(void)
{
	char* api_name = "sound_manager_get_max_volume";
	int ret, max;
	if ((ret = sound_manager_get_max_volume(-1, &max)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_set_volume_p(void)
{
	char* api_name = "sound_manager_set_volume";
	int ret, max;
	if ((ret = sound_manager_get_max_volume(SOUND_TYPE_SYSTEM, &max)) == SOUND_MANAGER_ERROR_NONE)
	{
		if ((ret = sound_manager_set_volume(SOUND_TYPE_NOTIFICATION, max/2)) == SOUND_MANAGER_ERROR_NONE)
		{
			dts_pass(api_name);
		}
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_set_volume_n(void)
{
	char* api_name = "sound_manager_set_volume";
	int ret;
	if ((ret = sound_manager_set_volume(-1, 10)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_volume_p(void)
{
	char* api_name = "sound_manager_get_volume";
	int ret, vol;
	if ((ret = sound_manager_get_volume(SOUND_TYPE_ALARM , &vol)) == SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_volume_n(void)
{
	char* api_name = "sound_manager_get_volume";
	int ret;
	if ((ret = sound_manager_get_volume(SOUND_TYPE_ALARM , NULL)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_set_route_policy_p(void)
{
	char* api_name = "sound_manager_set_route_policy";
	int ret;
	if ((ret = sound_manager_set_route_policy(SOUND_ROUTE_IGNORE_A2DP)) == SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_set_route_policy_n(void)
{
	char* api_name = "sound_manager_set_route_policy";
	int ret;
	if ((ret = sound_manager_set_route_policy(-1)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_route_policy_p(void)
{
	char* api_name = "sound_manager_get_route_policy";
	sound_route_policy_e route_policy;
	int ret;
	if ((ret = sound_manager_get_route_policy(&route_policy)) == SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_route_policy_n(void)
{
	char* api_name = "sound_manager_get_route_policy";
	int ret;
	if ((ret = sound_manager_get_route_policy(NULL)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_current_sound_type_p(void)
{
	char* api_name = "sound_manager_get_current_sound_type";
	sound_type_e type;
	int ret;
	ret = sound_manager_get_current_sound_type(&type);
	if (ret == SOUND_MANAGER_ERROR_NONE || ret == SOUND_MANAGER_ERROR_NO_PLAYING_SOUND)
	{
		dts_pass(api_name);
	}

	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_current_sound_type_n(void)
{
	char* api_name = "sound_manager_get_current_sound_type";
	int ret;
	if ((ret = sound_manager_get_current_sound_type(NULL)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_current_sound_device_p(void)
{
	char* api_name = "sound_manager_get_current_sound_device";
	sound_device_e device;
	int ret;
	if ((ret = sound_manager_get_current_sound_device(&device)) == SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_current_sound_device_n(void)
{
	char* api_name = "sound_manager_get_current_sound_device";
	int ret;
	if ((ret = sound_manager_get_current_sound_device(NULL)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_set_session_type_p(void)
{
	char* api_name = "sound_manager_set_session_type";
	sound_device_e device;
	int ret;
	if ((ret = sound_manager_set_session_type(SOUND_SESSION_TYPE_SHARE)) == SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}
static void utc_media_sound_manager_set_session_type_n(void)
{
	char* api_name = "sound_manager_set_session_type";
	sound_device_e device;
	int ret;
	if ((ret = sound_manager_set_session_type(-1)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_get_a2dp_status_p(void)
{
	char* api_name = "sound_manager_get_a2dp_status";
	bool connected;
	char *name;
	int ret;
	if ((ret = sound_manager_get_a2dp_status(&connected, &name)) == SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);	
}
static void utc_media_sound_manager_get_a2dp_status_n(void)
{
	char* api_name = "sound_manager_get_a2dp_status";
	int ret;
	if ((ret = sound_manager_get_a2dp_status(NULL, NULL)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);	
}

static bool _sound_available_route_cb (sound_route_e route, void *user_data)
{
	return false;
}

static void utc_media_sound_manager_foreach_available_route_p(void)
{
	int ret = sound_manager_foreach_available_route(_sound_available_route_cb, NULL);
	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_foreach_available_route_n(void)
{
	int ret = sound_manager_foreach_available_route(NULL, NULL);
	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_set_active_route_p(void)
{
	int ret = sound_manager_set_active_route(SOUND_ROUTE_OUT_SPEAKER);
	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_set_active_route_n(void)
{
	int ret = sound_manager_set_active_route(SOUND_ROUTE_INOUT_BLUETOOTH+1);
	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_get_active_device_p(void)
{
	sound_device_in_e in;
	sound_device_out_e out;
	int ret = sound_manager_get_active_device(&in, &out);
	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_get_active_device_n(void)
{
	int ret = sound_manager_get_active_device(NULL, NULL);
	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}
static void utc_media_sound_manager_is_route_available_p(void)
{
	int ret = sound_manager_is_route_available(SOUND_ROUTE_OUT_SPEAKER);
	dts_check_eq(__func__, ret, true);
}
static void utc_media_sound_manager_is_route_available_n(void)
{
	int ret = sound_manager_is_route_available(SOUND_ROUTE_INOUT_BLUETOOTH+1);
	dts_check_ne(__func__, ret, true);
}

static void utc_media_sound_manager_call_session_create_p(void)
{
	sound_call_session_h session;
	int ret = sound_manager_call_session_create(SOUND_SESSION_TYPE_CALL, &session);
	if (ret == SOUND_MANAGER_ERROR_NONE)
		sound_manager_call_session_destroy(session);
	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_call_session_create_n(void)
{
	int ret = sound_manager_call_session_create(SOUND_SESSION_TYPE_CALL, NULL);
	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}
static void utc_media_sound_manager_call_session_set_mode_p(void)
{
	sound_call_session_h session;
	int ret = sound_manager_call_session_create(SOUND_SESSION_TYPE_CALL, &session);
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		dts_fail(__func__);
	}
	ret = sound_manager_call_session_set_mode (session, SOUND_CALL_SESSION_MODE_RINGTONE);
	sound_manager_call_session_destroy(session);

	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_call_session_set_mode_n(void)
{
	sound_call_session_h session;
	int ret = sound_manager_call_session_create(SOUND_SESSION_TYPE_CALL, &session);
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		dts_fail(__func__);
	}
	ret = sound_manager_call_session_set_mode (NULL, SOUND_CALL_SESSION_MODE_RINGTONE);
	sound_manager_call_session_destroy(session);

	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}
static void utc_media_sound_manager_call_session_get_mode_p(void)
{
	sound_call_session_h session;
	sound_call_session_mode_e mode;
	int ret = sound_manager_call_session_create(SOUND_SESSION_TYPE_CALL, &session);
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		dts_fail(__func__);
	}
	ret = sound_manager_call_session_get_mode (session, &mode);
	sound_manager_call_session_destroy(session);

	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_call_session_get_mode_n(void)
{
	sound_call_session_h session;
	int ret = sound_manager_call_session_create(SOUND_SESSION_TYPE_CALL, &session);
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		dts_fail(__func__);
	}
	ret = sound_manager_call_session_get_mode (NULL, NULL);
	sound_manager_call_session_destroy(session);

	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_call_session_destroy_p(void)
{
	sound_call_session_h session;
	int ret = sound_manager_call_session_create(SOUND_SESSION_TYPE_CALL, &session);
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		dts_fail(__func__);
	}
	ret = sound_manager_call_session_destroy(session);
	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_call_session_destroy_n(void)
{
	int ret = sound_manager_call_session_destroy(NULL);
	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}
