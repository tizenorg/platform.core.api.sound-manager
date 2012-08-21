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

static void utc_media_sound_manager_set_volume_changed_cb_p(void);
static void utc_media_sound_manager_set_volume_changed_cb_n(void);
static void utc_media_sound_manager_set_route_policy_changed_cb_p(void);
static void utc_media_sound_manager_set_route_policy_changed_cb_n(void);
static void utc_media_sound_manager_set_session_notify_cb_p(void);
static void utc_media_sound_manager_set_session_notify_cb_n(void);

static void utc_media_sound_manager_set_available_route_changed_cb_p(void);
static void utc_media_sound_manager_set_available_route_changed_cb_n(void);
static void utc_media_sound_manager_set_active_device_changed_cb_p(void);
static void utc_media_sound_manager_set_active_device_changed_cb_n(void);

struct tet_testlist tet_testlist[] = {
	{ utc_media_sound_manager_set_volume_changed_cb_p , 1 },
	{ utc_media_sound_manager_set_volume_changed_cb_n , 2 },
	{ utc_media_sound_manager_set_route_policy_changed_cb_p, 3 },
	{ utc_media_sound_manager_set_route_policy_changed_cb_n, 4 },
	{ utc_media_sound_manager_set_session_notify_cb_p, 5 },
	{ utc_media_sound_manager_set_session_notify_cb_n, 6 },
	{ utc_media_sound_manager_set_available_route_changed_cb_p, 7 },
	{ utc_media_sound_manager_set_available_route_changed_cb_n, 8 },
	{ utc_media_sound_manager_set_active_device_changed_cb_p, 9 },
	{ utc_media_sound_manager_set_active_device_changed_cb_n, 10 },
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

static void sound_manager_vol_changed_cb(sound_route_policy_e type, void *user_data)
{
}

static void sound_manager_routepolicy_changed_cb(sound_type_e type, unsigned int volume, void *user_data)
{
}

static void _sound_available_route_changed_cb (sound_route_e route, bool available, void *user_data)
{
}

static void _sound_active_device_changed_cb (sound_device_in_e in, sound_device_out_e out, void *user_data)
{
}

static void utc_media_sound_manager_set_volume_changed_cb_p(void)
{
	char* api_name = "sound_manager_set_volume_changed_cb";
	int ret;
	if ((ret = sound_manager_set_volume_changed_cb(sound_manager_vol_changed_cb, &ret)) == SOUND_MANAGER_ERROR_NONE)
	{
		sound_manager_unset_volume_changed_cb();
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_set_volume_changed_cb_n(void)
{
	char* api_name = "sound_manager_set_volume_changed_cb";
	int ret;
	if ((ret = sound_manager_set_volume_changed_cb(NULL, &ret)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}


static void utc_media_sound_manager_set_route_policy_changed_cb_p(void)
{
	char* api_name = "sound_manager_set_route_policy_changed_cb";
	int ret;
	if ((ret = sound_manager_set_route_policy_changed_cb(sound_manager_routepolicy_changed_cb, &ret)) == SOUND_MANAGER_ERROR_NONE)
	{
		sound_manager_unset_route_policy_changed_cb();
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_set_route_policy_changed_cb_n(void)
{
	char* api_name = "sound_manager_set_route_policy_changed_cb";
	int ret;
	if ((ret = sound_manager_set_route_policy_changed_cb(NULL, &ret)) != SOUND_MANAGER_ERROR_NONE)
	{
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

void _session_notify_cb(sound_session_notify_e notify, void *user_data)
{
}


static void utc_media_sound_manager_set_session_notify_cb_p(void)
{
	char* api_name = "sound_manager_set_route_policy_changed_cb";
	int ret;
	if ((ret = sound_manager_set_session_notify_cb(_session_notify_cb, NULL)) == SOUND_MANAGER_ERROR_NONE)
	{
		sound_manager_unset_session_notify_cb();
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);

}

static void utc_media_sound_manager_set_session_notify_cb_n(void)
{
	char* api_name = "sound_manager_set_route_policy_changed_cb";
	int ret;
	if ((ret = sound_manager_set_session_notify_cb(NULL, NULL)) != SOUND_MANAGER_ERROR_NONE)
	{
		sound_manager_unset_session_notify_cb();
		dts_pass(api_name);
	}
	dts_message(api_name, "Call log: %d", ret);
	dts_fail(api_name);
}

static void utc_media_sound_manager_set_available_route_changed_cb_p(void)
{
	int ret = sound_manager_set_available_route_changed_cb(_sound_available_route_changed_cb, NULL);
	if (ret == SOUND_MANAGER_ERROR_NONE) {
		sound_manager_unset_available_route_changed_cb ();
	}
	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_set_available_route_changed_cb_n(void)
{
	int ret = sound_manager_set_available_route_changed_cb(NULL, NULL);
	if (ret == SOUND_MANAGER_ERROR_NONE) {
		sound_manager_unset_available_route_changed_cb ();
	}
	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_set_active_device_changed_cb_p(void)
{
	int ret = sound_manager_set_active_device_changed_cb(_sound_active_device_changed_cb, NULL);
	if (ret == SOUND_MANAGER_ERROR_NONE) {
		sound_manager_unset_active_device_changed_cb ();
	}
	dts_check_eq(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}

static void utc_media_sound_manager_set_active_device_changed_cb_n(void)
{
	int ret = sound_manager_set_active_device_changed_cb(NULL, NULL);
	if (ret == SOUND_MANAGER_ERROR_NONE) {
		sound_manager_unset_active_device_changed_cb ();
	}
	dts_check_ne(__func__, ret, SOUND_MANAGER_ERROR_NONE);
}
