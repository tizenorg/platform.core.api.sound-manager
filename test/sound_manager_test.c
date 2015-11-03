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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sound_manager.h>
#include <sound_manager_internal.h>
#include <pthread.h>
#include <glib.h>

#define MAX_STRING_LEN	2048

enum {
	CURRENT_STATUS_MAINMENU,
	CURRENT_STATUS_GET_MAX_VOLUME,
	CURRENT_STATUS_SET_VOLUME,
	CURRENT_STATUS_GET_VOLUME,
	CURRENT_STATUS_SET_CURRENT_SOUND_TYPE,
	CURRENT_STATUS_GET_CURRENT_SOUND_TYPE,
	CURRENT_STATUS_UNSET_CURRENT_SOUND_TYPE,
	CURRENT_STATUS_SET_VOLUME_CHANGED_CB,
	CURRENT_STATUS_UNSET_VOLUME_CHANGED_CB,
	CURRENT_STATUS_SET_SESSION_TYPE,
	CURRENT_STATUS_GET_SESSION_TYPE,
	CURRENT_STATUS_SET_MEDIA_SESSION_OPTION,
	CURRENT_STATUS_GET_MEDIA_SESSION_OPTION,
	CURRENT_STATUS_SET_MEDIA_SESSION_RESUMPTION_OPTION,
	CURRENT_STATUS_GET_MEDIA_SESSION_RESUMPTION_OPTION,
	CURRENT_STATUS_SET_VOIP_SESSION_MODE,
	CURRENT_STATUS_GET_VOIP_SESSION_MODE,
	CURRENT_STATUS_SET_CALL_SESSION_MODE,
	CURRENT_STATUS_GET_CALL_SESSION_MODE,
	CURRENT_STATUS_SET_SESSION_INTERRUPTED_CB,
	CURRENT_STATUS_UNSET_SESSION_INTERRUPTED_CB,
	CURRENT_STATUS_SET_DEVICE_MASK,
	CURRENT_STATUS_GET_DEVICE_MASK,
	CURRENT_STATUS_GET_DEVICE_LIST,
	CURRENT_STATUS_FREE_DEVICE_LIST,
	CURRENT_STATUS_GET_DEVICE_NEXT,
	CURRENT_STATUS_GET_DEVICE_PREV,
	CURRENT_STATUS_SET_DEVICE_CONNECTED_CB,
	CURRENT_STATUS_UNSET_DEVICE_CONNECTED_CB,
	CURRENT_STATUS_SET_DEVICE_INFO_CHANGED_CB,
	CURRENT_STATUS_UNSET_DEVICE_INFO_CHANGED_CB,
	CURRENT_STATUS_CREATE_STREAM_INFO,
	CURRENT_STATUS_ADD_DEVICE_FOR_STREAM_ROUTING,
	CURRENT_STATUS_REMOVE_DEVICE_FOR_STREAM_ROUTING,
	CURRENT_STATUS_APPLY_STREAM_ROUTING,
	CURRENT_STATUS_SET_STREAM_ROUTING_OPTION,
	CURRENT_STATUS_ACQUIRE_FOCUS,
	CURRENT_STATUS_RELEASE_FOCUS,
	CURRENT_STATUS_GET_ACQUIRED_FOCUS,
	CURRENT_STATUS_GET_SOUND_TYPE,
	CURRENT_STATUS_DESTROY_STREAM_INFO,
	CURRENT_STATUS_SET_FOCUS_WATCH_CB,
	CURRENT_STATUS_UNSET_FOCUS_WATCH_CB,
	CURRENT_STATUS_CREATE_VIRTUAL_STREAM,
	CURRENT_STATUS_START_VIRTUAL_STREAM,
	CURRENT_STATUS_STOP_VIRTUAL_STREAM,
	CURRENT_STATUS_DESTROY_VIRTUAL_STREAM,
	CURRENT_STATUS_GET_MAX_MASTER_VOLUME,
	CURRENT_STATUS_SET_MASTER_VOLUME,
	CURRENT_STATUS_GET_MASTER_VOLUME,
};


static int g_menu_state = CURRENT_STATUS_MAINMENU;

GMainLoop* g_loop;
sound_device_list_h g_device_list = NULL;
sound_device_mask_e g_device_mask = SOUND_DEVICE_ALL_MASK;
sound_stream_info_h g_stream_info_h = NULL;
virtual_sound_stream_h g_vstream_h = NULL;

void focus_callback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	int ret = SOUND_MANAGER_ERROR_NONE;
	sound_stream_focus_state_e playback_focus_state;
	sound_stream_focus_state_e recording_focus_state;
	g_print("*** FOCUS callback is called, stream_info(%p) ***\n", stream_info);
	g_print(" - reason_for_change(%d), additional_info(%s), user_data(%p)\n", reason_for_change, additional_info, user_data);

	ret = sound_manager_get_focus_state(stream_info, &playback_focus_state, &recording_focus_state);
	if (!ret)
		g_print(" - focus_state(playback_focus:%d, recording_focus:%d)\n", playback_focus_state, recording_focus_state);

	if (playback_focus_state == SOUND_STREAM_FOCUS_STATE_ACQUIRED)
		g_print(" -- PLAYBACK_FOCUS acquired\n");
	else
		g_print(" -- PLAYBACK_FOCUS released\n");

	if (recording_focus_state == SOUND_STREAM_FOCUS_STATE_ACQUIRED)
		g_print(" -- FOCUS_RECORDING acquired\n");
	else
		g_print(" -- FOCUS_RECORDING released\n");

	g_print("*** FOCUS callback is ended, stream_info(%p) ****\n", stream_info);

	return;
}

void focus_watch_callback(sound_stream_focus_mask_e  changed_focus_type, sound_stream_focus_state_e changed_focus_state, sound_stream_focus_change_reason_e reason_for_change, const char *additional_info, void *user_data)
{
	g_print("*** FOCUS WATCH callback is called ***\n");
	g_print(" - changed_focus_type(%d), changed_focus_state(%d), reason_for_change(%d), additional_info(%s), user_data(%p)\n",
				changed_focus_type, changed_focus_state, reason_for_change, additional_info, user_data);
	return;
}

void quit_program()
{
	g_main_loop_quit(g_loop);
}

void _interpret_main_menu(char *cmd)
{
	if (strncmp(cmd, "gx", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_MAX_VOLUME;
	else if (strncmp(cmd, "sv", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_VOLUME;
	else if (strncmp(cmd, "gv", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_VOLUME;
	else if (strncmp(cmd, "st", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_CURRENT_SOUND_TYPE;
	else if (strncmp(cmd, "gt", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_CURRENT_SOUND_TYPE;
	else if (strncmp(cmd, "ut", 3) == 0)
		g_menu_state = CURRENT_STATUS_UNSET_CURRENT_SOUND_TYPE;
	else if (strncmp(cmd, "vc", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_VOLUME_CHANGED_CB;
	else if (strncmp(cmd, "uv", 3) == 0)
		g_menu_state = CURRENT_STATUS_UNSET_VOLUME_CHANGED_CB;
	else if (strncmp(cmd, "ss", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_SESSION_TYPE;
	else if (strncmp(cmd, "gs", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_SESSION_TYPE;
	else if (strncmp(cmd, "sm", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_MEDIA_SESSION_OPTION;
	else if (strncmp(cmd, "gm", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_MEDIA_SESSION_OPTION;
	else if (strncmp(cmd, "sr", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_MEDIA_SESSION_RESUMPTION_OPTION;
	else if (strncmp(cmd, "gr", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_MEDIA_SESSION_RESUMPTION_OPTION;
	else if (strncmp(cmd, "so", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_VOIP_SESSION_MODE;
	else if (strncmp(cmd, "go", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_VOIP_SESSION_MODE;
	else if (strncmp(cmd, "sl", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_CALL_SESSION_MODE;
	else if (strncmp(cmd, "gc", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_CALL_SESSION_MODE;
	else if (strncmp(cmd, "sc", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_SESSION_INTERRUPTED_CB;
	else if (strncmp(cmd, "us", 3) == 0)
		g_menu_state = CURRENT_STATUS_UNSET_SESSION_INTERRUPTED_CB;
	else if (strncmp(cmd, "sk", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_DEVICE_MASK;
	else if (strncmp(cmd, "gk", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_DEVICE_MASK;
	else if (strncmp(cmd, "gl", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_DEVICE_LIST;
	else if (strncmp(cmd, "fl", 3) == 0)
		g_menu_state = CURRENT_STATUS_FREE_DEVICE_LIST;
	else if (strncmp(cmd, "gn", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_DEVICE_NEXT;
	else if (strncmp(cmd, "gp", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_DEVICE_PREV;
	else if (strncmp(cmd, "sd", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_DEVICE_CONNECTED_CB;
	else if (strncmp(cmd, "ud", 3) == 0)
		g_menu_state = CURRENT_STATUS_UNSET_DEVICE_CONNECTED_CB;
	else if (strncmp(cmd, "si", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_DEVICE_INFO_CHANGED_CB;
	else if (strncmp(cmd, "ui", 3) == 0)
		g_menu_state = CURRENT_STATUS_UNSET_DEVICE_INFO_CHANGED_CB;
	else if (strncmp(cmd, "csi", 3) == 0)
		g_menu_state = CURRENT_STATUS_CREATE_STREAM_INFO;
	else if (strncmp(cmd, "ads", 3) == 0)
		g_menu_state = CURRENT_STATUS_ADD_DEVICE_FOR_STREAM_ROUTING;
	else if (strncmp(cmd, "rds", 3) == 0)
		g_menu_state = CURRENT_STATUS_REMOVE_DEVICE_FOR_STREAM_ROUTING;
	else if (strncmp(cmd, "aps", 3) == 0)
		g_menu_state = CURRENT_STATUS_APPLY_STREAM_ROUTING;
	else if (strncmp(cmd, "sso", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_STREAM_ROUTING_OPTION;
	else if (strncmp(cmd, "afc", 3) == 0)
		g_menu_state = CURRENT_STATUS_ACQUIRE_FOCUS;
	else if (strncmp(cmd, "rfc", 3) == 0)
		g_menu_state = CURRENT_STATUS_RELEASE_FOCUS;
	else if (strncmp(cmd, "gfs", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_ACQUIRED_FOCUS;
	else if (strncmp(cmd, "gst", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_SOUND_TYPE;
	else if (strncmp(cmd, "sfw", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_FOCUS_WATCH_CB;
	else if (strncmp(cmd, "ufw", 3) == 0)
		g_menu_state = CURRENT_STATUS_UNSET_FOCUS_WATCH_CB;
	else if (strncmp(cmd, "dsi", 3) == 0)
		g_menu_state = CURRENT_STATUS_DESTROY_STREAM_INFO;
	else if (strncmp(cmd, "vcr", 3) == 0)
		g_menu_state = CURRENT_STATUS_CREATE_VIRTUAL_STREAM;
	else if (strncmp(cmd, "vsr", 3) == 0)
		g_menu_state = CURRENT_STATUS_START_VIRTUAL_STREAM;
	else if (strncmp(cmd, "vst", 3) == 0)
		g_menu_state = CURRENT_STATUS_STOP_VIRTUAL_STREAM;
	else if (strncmp(cmd, "vdt", 3) == 0)
		g_menu_state = CURRENT_STATUS_DESTROY_VIRTUAL_STREAM;
	if (strncmp(cmd, "mgx", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_MAX_MASTER_VOLUME;
	else if (strncmp(cmd, "msv", 3) == 0)
		g_menu_state = CURRENT_STATUS_SET_MASTER_VOLUME;
	else if (strncmp(cmd, "mgv", 3) == 0)
		g_menu_state = CURRENT_STATUS_GET_MASTER_VOLUME;
	else if (strncmp(cmd, "q", 3) == 0) {
		g_print("closing the test suite\n");
		quit_program();
	} else
		g_print("unknown menu \n");
}

void display_sub_basic()
{
	g_print("\n");
	g_print("=========================================================================================\n");
	g_print("                          Sound Manager Test (press q to quit) \n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("                                    VOLUME MODULE \n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("gx. Get Max Volume \t");
	g_print("gv. Get Volume \t");
	g_print("sv. Set Volume \n");
	g_print("st. Set Current Sound Type \t");
	g_print("gt. Get Current Sound Type \t");
	g_print("ut. Unset Current Sound Type \n");
	g_print("vc. Set Volume Changed CB \t");
	g_print("uv. Unset Volume Changed CB \n");
	g_print("mgx. *Get Max Master Volume \t");
	g_print("mgv. *Get Master Volume \t");
	g_print("msv. *Set Master Volume \n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("                                    SESSION MODULE \n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("ss. Set Session Type\t\t");
	g_print("gs. Get Session Type\n");
	g_print("sm. Set Media Session Option \t");
	g_print("gm. Get Media Session Option \n");
	g_print("sr. Set Media Session Resumption Option \t");
	g_print("gr. Get Media Session Resumption Option \n");
	g_print("so. Set Voip Session Mode \t");
	g_print("go. Get Voip Session Mode \n");
	g_print("sc. Set Session Interruped CB \t");
	g_print("us. Unset Session Interrupted CB \n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("                                    DEVICE MODULE \n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("sk. Set Devices Mask(default ALL)\t");
	g_print("gk. Get Devices Mask\n");
	g_print("gl. Get Devices List\t");
	g_print("fl. Free Devices List\n");
	g_print("gn. Get Next Device\t\t");
	g_print("gp. Get Prev Device\n");
	g_print("sd. Set Device Connenected CB\t\t");
	g_print("ud. Unset Device Connenected CB\n");
	g_print("si. Set Device Information Changed CB\t");
	g_print("ui. Unset Device Information Changed CB\n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("                                    STREAM POLICY MODULE \n");
	g_print("-----------------------------------------------------------------------------------------\n");
	g_print("csi. Create Stream Info\t");
	g_print("dsi. Destroy Stream Info\n");
	g_print("gst. Get Sound Type\n");
	g_print("ads. Add device for stream routing\t");
	g_print("rds. Remove device for stream routing\t");
	g_print("aps. Apply devices for stream routing\n");
	g_print("afc. Acquire Focus\t");
	g_print("rfc. Release Focus\t");
	g_print("gfs. Get Focus State\n");
	g_print("sfw. Set Focus State Watch CB\t");
	g_print("ufw. Unset Focus State Watch CB\n");
	g_print("sso. *Set option for stream routing\n");
	g_print("vcr. *Create VStream\t");
	g_print("vsr. *Start VStream\t");
	g_print("vst. *Stop VStream\t");
	g_print("vdt. *Destroy VStream\n");
	g_print("                                                                 * is for internal usage.\n");
	g_print("=========================================================================================\n");
}

static void displaymenu()
{
	static int flag = 0;
	if (g_menu_state == CURRENT_STATUS_MAINMENU) {
		display_sub_basic();
		flag = 0;
	} else if (g_menu_state == CURRENT_STATUS_GET_MAX_VOLUME)
		g_print("*** input sound type(0:SYSTEM 1:NOTIFICATION 2:ALARM 3:RINGTONE 4:MEDIA 5:CALL 6:VOIP 7:VOICE)\n");
	else if (g_menu_state == CURRENT_STATUS_SET_VOLUME) {
		if (flag == 0)
			g_print("*** input sound type and desired volume level(0:SYSTEM 1:NOTIFICATION 2:ALARM 3:RINGTONE 4:MEDIA 5:CALL 6:VOIP 7:VOICE, (0~max volum).\n");
		flag = 1;
	} else if (g_menu_state == CURRENT_STATUS_GET_VOLUME)
		g_print("*** input sound type(0:SYSTEM 1:NOTIFICATION 2:ALARM 3:RINGTONE 4:MEDIA 5:CALL 6:VOIP 7:VOICE)\n");
	else if (g_menu_state == CURRENT_STATUS_SET_CURRENT_SOUND_TYPE)
		g_print("*** input sound type.(0:SYSTEM 1:NOTIFICATION 2:ALARM 3:RINGTONE 4:MEDIA 5:CALL 6:VOIP 7:VOICE)\n");
	else if (g_menu_state == CURRENT_STATUS_GET_CURRENT_SOUND_TYPE)
		g_print("*** press enter to get current sound type\n");
	else if (g_menu_state == CURRENT_STATUS_UNSET_CURRENT_SOUND_TYPE)
		g_print("*** press enter to unset current sound type\n");
	else if (g_menu_state == CURRENT_STATUS_SET_VOLUME_CHANGED_CB)
		g_print("*** press enter to set volume changed cb\n");
	else if (g_menu_state == CURRENT_STATUS_UNSET_VOLUME_CHANGED_CB)
		g_print("*** press enter to unset volume changed cb\n");
	else if (g_menu_state == CURRENT_STATUS_SET_SESSION_TYPE)
		g_print("*** input session type(0:MEDIA, 1:ALARM, 2:NOTIFICATION, 3:EMERGENCY, 4:VOIP)\n");
	else if (g_menu_state == CURRENT_STATUS_GET_SESSION_TYPE)
		g_print("*** press enter to get session type\n");
	else if (g_menu_state == CURRENT_STATUS_SET_MEDIA_SESSION_OPTION) {
		if (flag == 0)
			g_print("*** input starting option, and ongoing(for starting(0:MIX WITH OTHERS, 1:PAUSE OTHERS), for ongoing(0:INTERRUPTABLE, 1:UNINTERRUPABLE)\n");
		flag = 1;
	} else if (g_menu_state == CURRENT_STATUS_GET_MEDIA_SESSION_OPTION)
		g_print("*** press enter to get media session option\n");
	else if (g_menu_state == CURRENT_STATUS_SET_MEDIA_SESSION_RESUMPTION_OPTION)
		g_print("*** input media session resumption option(0:BY SYSTEM, 1:OR MEDIA PAUSE)\n");
	else if (g_menu_state == CURRENT_STATUS_GET_MEDIA_SESSION_RESUMPTION_OPTION)
		g_print("*** press enter to get media session resumption option\n");
	else if (g_menu_state == CURRENT_STATUS_SET_VOIP_SESSION_MODE)
		g_print("*** input voip session mode (0:RINGTONE, 1:VOICE with RCV, 2:VOICE with SPK, 3:VOICE with AudioJack, 4:VOICE with BT)\n");
	else if (g_menu_state == CURRENT_STATUS_GET_VOIP_SESSION_MODE)
		g_print("*** press enter to get voip session mode\n");
	else if (g_menu_state == CURRENT_STATUS_SET_CALL_SESSION_MODE)
		g_print("*** input call session mode (0:RINGTONE, 1:VOICE with RCV, 2:VOICE with SPK, 3:VOICE with AudioJack, 4:VOICE with BT)\n");
	else if (g_menu_state == CURRENT_STATUS_GET_CALL_SESSION_MODE)
		g_print("*** press enter to get call session mode\n");
	else if (g_menu_state == CURRENT_STATUS_SET_SESSION_INTERRUPTED_CB)
		g_print("*** press enter to set session interrupted cb\n");
	else if (g_menu_state == CURRENT_STATUS_UNSET_SESSION_INTERRUPTED_CB)
		g_print("*** press enter to unset session interrupted cb\n");
	else if (g_menu_state == CURRENT_STATUS_SET_DEVICE_MASK)
		g_print("*** input device mask (0:ALL, 1:INTERNAL, 2:EXTERNAL, 3:INPUT, 4:OUTPUT, 5:BOTH, 6:ACTIVATED, 7:DEACTIVATED, b:back to the menu\n");
	else if (g_menu_state == CURRENT_STATUS_GET_DEVICE_MASK)
		g_print("*** press enter to get device mask\n");
	else if (g_menu_state == CURRENT_STATUS_GET_DEVICE_LIST)
		g_print("*** press enter to get device list\n");
	else if (g_menu_state == CURRENT_STATUS_FREE_DEVICE_LIST)
		g_print("*** press enter to free device list\n");
	else if (g_menu_state == CURRENT_STATUS_GET_DEVICE_NEXT)
		g_print("*** press enter to get next device from the list\n");
	else if (g_menu_state == CURRENT_STATUS_GET_DEVICE_PREV)
		g_print("*** press enter to get previous device from the list\n");
	else if (g_menu_state == CURRENT_STATUS_SET_DEVICE_CONNECTED_CB)
		g_print("*** press enter to set device connected cb\n");
	else if (g_menu_state == CURRENT_STATUS_UNSET_DEVICE_CONNECTED_CB)
		g_print("*** press enter to unset device connected cb\n");
	else if (g_menu_state == CURRENT_STATUS_SET_DEVICE_INFO_CHANGED_CB)
		g_print("*** press enter to set device information changed cb\n");
	else if (g_menu_state == CURRENT_STATUS_UNSET_DEVICE_INFO_CHANGED_CB)
		g_print("*** press enter to unset device information changed cb\n");
	else if (g_menu_state == CURRENT_STATUS_CREATE_STREAM_INFO)
		g_print("*** input stream type to create stream information (0:media, 1:alarm, 2:notification, 3:ringtone-call, 4:voice-call, 5:voip, 6:media-ext-only)\n");
	else if (g_menu_state == CURRENT_STATUS_ADD_DEVICE_FOR_STREAM_ROUTING)
		g_print("*** input device type to add (0:built-in mic, 1:built-in spk, 2:built-in rcv, 3:audio-jack, 4:bt)\n");
	else if (g_menu_state == CURRENT_STATUS_REMOVE_DEVICE_FOR_STREAM_ROUTING)
		g_print("*** input device type to remove (0:built-in mic, 1:built-in spk, 2:built-in rcv, 3:audio-jack, 4:bt)\n");
	else if (g_menu_state == CURRENT_STATUS_APPLY_STREAM_ROUTING)
		g_print("*** press enter to apply devices for stream routing\n");
	else if (g_menu_state == CURRENT_STATUS_SET_STREAM_ROUTING_OPTION)
		g_print("*** input option(name/value) for routing (0:option_1/0, 1:option_1/1, 2:option_2/0, 3:option_2:1)\n");
	else if (g_menu_state == CURRENT_STATUS_ACQUIRE_FOCUS)
		g_print("*** input focus type to acquire (0:playback, 1:recording, 2:both)\n");
	else if (g_menu_state == CURRENT_STATUS_RELEASE_FOCUS)
		g_print("*** input focus type to release (0:playback, 1:recording, 2:both)\n");
	else if (g_menu_state == CURRENT_STATUS_GET_ACQUIRED_FOCUS)
		g_print("*** press enter to get focus state\n");
	else if (g_menu_state == CURRENT_STATUS_GET_SOUND_TYPE)
		g_print("*** press enter to get sound type\n");
	else if (g_menu_state == CURRENT_STATUS_DESTROY_STREAM_INFO)
		g_print("*** press enter to destroy stream information\n");
	else if (g_menu_state == CURRENT_STATUS_SET_FOCUS_WATCH_CB)
		g_print("*** input focus type to watch for (0:playback, 1:recording, 2:both)\n");
	else if (g_menu_state == CURRENT_STATUS_UNSET_FOCUS_WATCH_CB)
		g_print("*** press enter to unset focus state watch cb\n");
	else if (g_menu_state == CURRENT_STATUS_CREATE_VIRTUAL_STREAM)
		g_print("*** press enter to create virtual stream\n");
	else if (g_menu_state == CURRENT_STATUS_START_VIRTUAL_STREAM)
		g_print("*** press enter to start virtual stream\n");
	else if (g_menu_state == CURRENT_STATUS_STOP_VIRTUAL_STREAM)
		g_print("*** press enter to stop virtual stream\n");
	else if (g_menu_state == CURRENT_STATUS_DESTROY_VIRTUAL_STREAM)
		g_print("*** press enter to destroy virtual stream\n");
	else if (g_menu_state == CURRENT_STATUS_GET_MAX_MASTER_VOLUME)
		g_print("*** press enter to get max master volume level\n");
	else if (g_menu_state == CURRENT_STATUS_SET_MASTER_VOLUME)
		g_print("*** input master volume level\n");
	else if (g_menu_state == CURRENT_STATUS_GET_MASTER_VOLUME)
		g_print("*** press enter to get master volume level\n");
	else {
		g_print("*** unknown status.\n");
		quit_program();
	}
	g_print(" >>> ");
}

gboolean timeout_menu_display(void* data)
{
	displaymenu();
	return FALSE;
}

int convert_sound_type(sound_type_e *type, char *cmd)
{
	int sound_type_n = atoi(cmd);
	if (SOUND_TYPE_SYSTEM > sound_type_n || sound_type_n > SOUND_TYPE_VOICE) {
		g_print("not supported sound type(%d)\n", sound_type_n);
		return 0;
	} else {
		switch (sound_type_n) {
		case 0:
			*type = SOUND_TYPE_SYSTEM;
			break;
		case 1:
			*type = SOUND_TYPE_NOTIFICATION;
			break;
		case 2:
			*type = SOUND_TYPE_ALARM;
			break;
		case 3:
			*type = SOUND_TYPE_RINGTONE;
			break;
		case 4:
			*type = SOUND_TYPE_MEDIA;
			break;
		case 5:
			*type = SOUND_TYPE_CALL;
			break;
		case 6:
			*type = SOUND_TYPE_VOIP;
			break;
		case 7:
			*type = SOUND_TYPE_VOICE;
			break;
		}
	}
	return 1;
}

int convert_session_type(sound_session_type_e *type, char *cmd)
{
	int session_type_n = atoi(cmd);
	if (SOUND_SESSION_TYPE_MEDIA > session_type_n || session_type_n > SOUND_SESSION_TYPE_VOIP) {
		g_print("not supported session type(%d)\n", session_type_n);
		return 0;
	} else {
		switch (session_type_n) {
		case 0:
			*type = SOUND_SESSION_TYPE_MEDIA;
			break;
		case 1:
			*type = SOUND_SESSION_TYPE_ALARM;
			break;
		case 2:
			*type = SOUND_SESSION_TYPE_NOTIFICATION;
			break;
		case 3:
			*type = SOUND_SESSION_TYPE_EMERGENCY;
			break;
		case 4:
			*type = SOUND_SESSION_TYPE_VOIP;
			break;
		}
	}
	return 1;
}

void _set_volume_changed_cb(sound_type_e type, unsigned int volume, void *user_data)
{
	g_print("***the volume has changed. the volume of this sound type(%d) is : %d \n", type, volume);
}

void _set_session_interrupted_cb(sound_session_interrupted_code_e code, void *user_data)
{
	g_print("***your session has been interrupted by (%d)\n", code);
}

void _set_device_connected_cb(sound_device_h device, bool is_connected, void *user_data)
{
	sound_device_type_e type;
	sound_device_io_direction_e io_direction;
	sound_device_state_e state;
	int id;
	char *name;
	int ret = SOUND_MANAGER_ERROR_NONE;

	g_print("***device connected callback is called, is_connected[%d]\n", is_connected);

	if ((ret = sound_manager_get_device_type(device, &type)))
		g_print("failed to get device type, ret[0x%x]\n", ret);
	if ((ret = sound_manager_get_device_io_direction(device, &io_direction)))
		g_print("failed to get device io direction, ret[0x%x]\n", ret);
	if ((ret = sound_manager_get_device_id(device, &id)))
		g_print("failed to get device id, ret[0x%x]\n", ret);
	if ((ret = sound_manager_get_device_name(device, &name)))
		g_print("failed to get device name, ret[0x%x]\n", ret);
	if ((ret = sound_manager_get_device_state(device, &state)))
		g_print("failed to get device state, ret[0x%x]\n", ret);
	if (!ret)
		g_print(" -- device type[%d], io_direction[%d], id[%d], name[%s], state[%d]\n", type, io_direction, id, name, state);
}

void _set_device_info_changed_cb(sound_device_h device, sound_device_changed_info_e changed_info, void *user_data)
{
	sound_device_type_e type;
	sound_device_io_direction_e io_direction;
	sound_device_state_e state;
	int id;
	char *name;
	int ret = SOUND_MANAGER_ERROR_NONE;

	g_print("***device information changed callback is called, changed_info[%d](0:STATE 1:IO_DIRECTION)\n", changed_info);

	if ((ret = sound_manager_get_device_type(device, &type)))
		g_print("failed to get device type, ret[0x%x]\n", ret);
	if ((ret = sound_manager_get_device_io_direction(device, &io_direction)))
		g_print("failed to get device io direction, ret[0x%x]\n", ret);
	if ((ret = sound_manager_get_device_id(device, &id)))
		g_print("failed to get device id, ret[0x%x]\n", ret);
	if ((ret = sound_manager_get_device_name(device, &name)))
		g_print("failed to get device name, ret[0x%x]\n", ret);
	if ((ret = sound_manager_get_device_state(device, &state)))
		g_print("failed to get device state, ret[0x%x]\n", ret);
	if (!ret)
		g_print(" -- device type[%d], io_direction[%d], id[%d], name[%s], state[%d]\n", type, io_direction, id, name, state);
}

void reset_menu_state(void)
{
	g_menu_state = CURRENT_STATUS_MAINMENU;
}

static void interpret(char *cmd)
{
	switch (g_menu_state) {
	case CURRENT_STATUS_MAINMENU:
		_interpret_main_menu(cmd);
		break;
	case CURRENT_STATUS_GET_MAX_VOLUME: {
		sound_type_e type;
		int max;
		if (convert_sound_type(&type, cmd) == 1) {
			if (sound_manager_get_max_volume(type, &max) != SOUND_MANAGER_ERROR_NONE)
				g_print("failt to get max volume\n");
			else
				g_print("the max volume of this type(%d) is %d\n", type, max);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_VOLUME: {
		static int cnt = 0;
		sound_type_e type;
		int volume;
		switch (cnt) {
		case 0:
			if (convert_sound_type(&type, cmd) == 1)
				cnt++;
			else
				reset_menu_state();
			break;
		case 1:
			volume = atoi(cmd);
			if (sound_manager_set_volume(type, volume) != SOUND_MANAGER_ERROR_NONE)
				g_print("fail to set volume(%d) check sound type(%d)'s available volume level\n", volume, type);
			else
				g_print("set volume success : sound type(%d), volume(%d)\n", type, volume);
			cnt = 0;
			reset_menu_state();
			break;
		default:
			break;
		}
		break;
	}
	case CURRENT_STATUS_GET_VOLUME: {
		sound_type_e type;
		int volume;
		if (convert_sound_type(&type, cmd) == 1) {
			if (sound_manager_get_volume(type, &volume) != SOUND_MANAGER_ERROR_NONE)
				g_print("fail to get volume\n");
			else
				g_print("current volume of this type(%d) is : %d\n", type, volume);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_CURRENT_SOUND_TYPE: {
		sound_type_e type;
		if (convert_sound_type(&type, cmd) == 1) {
			if (sound_manager_set_current_sound_type(type) != SOUND_MANAGER_ERROR_NONE)
				g_print("fail to set sound type(%d)\n", type);
			else
				g_print("success to set sound type(%d)\n", type);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_CURRENT_SOUND_TYPE: {
		sound_type_e type;
		if (sound_manager_get_current_sound_type(&type) != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to get current sound type\n");
		else
			g_print("current sound type is (%d)\n", type);
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_UNSET_CURRENT_SOUND_TYPE: {
		if (sound_manager_unset_current_sound_type() != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to unset current sound type\n");
		else
			g_print("success to unset current sound type\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_VOLUME_CHANGED_CB: {
		if (sound_manager_set_volume_changed_cb(_set_volume_changed_cb, NULL) != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to set volume changed cb\n");
		else
			g_print("success to set volume changed cb\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_UNSET_VOLUME_CHANGED_CB: {
		if (sound_manager_unset_volume_changed_cb() != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to unset volume changed cb\n");
		else
			g_print("success to unset volume changed cb\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_SESSION_TYPE: {
		sound_session_type_e type;
		if (convert_session_type(&type, cmd) == 1) {
			if (sound_manager_set_session_type(type) != SOUND_MANAGER_ERROR_NONE)
				g_print("fail to set session type\n");
			else
				g_print("success to set session type(%d)\n", type);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_SESSION_TYPE: {
		sound_session_type_e type;
		if (sound_manager_get_session_type(&type) != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to get session type\n");
		else
			g_print("current session type is : %d (0:MEDIA, 1:ALARM, 2:NOTIFICATION, 3:EMERGENCY, 4:VOIP, 5:CALL)\n", type);
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_MEDIA_SESSION_OPTION: {
		sound_session_type_e type = SOUND_SESSION_TYPE_MEDIA;
		static sound_session_option_for_starting_e option_s;
		static sound_session_option_for_during_play_e option_d;
		static int cnt = 0;
		if (sound_manager_set_session_type(type) != SOUND_MANAGER_ERROR_NONE) {
			g_print("fail to set media session type\n");
			reset_menu_state();
		} else {
			switch (cnt) {
			case 0:
				option_s = (sound_session_option_for_starting_e)atoi(cmd);
				if (SOUND_SESSION_OPTION_MIX_WITH_OTHERS_WHEN_START > option_s || SOUND_SESSION_OPTION_PAUSE_OTHERS_WHEN_START < option_s)
					g_print("not supported option type\n");
				else
					cnt++;
				break;
			case 1:
				option_d = (sound_session_option_for_during_play_e)atoi(cmd);
				if (SOUND_SESSION_OPTION_INTERRUPTIBLE_DURING_PLAY > option_d || SOUND_SESSION_OPTION_UNINTERRUPTIBLE_DURING_PLAY < option_d)
					g_print("not supported option type\n");
				else {
					if (sound_manager_set_media_session_option(option_s, option_d) != SOUND_MANAGER_ERROR_NONE)
						g_print("fail to set media session option\n");
					else
						g_print("success to set media session option\n");
					cnt = 0;
				}
				reset_menu_state();
				break;
			default:
				break;
			}
		}
		break;
	}
	case CURRENT_STATUS_GET_MEDIA_SESSION_OPTION: {
		sound_session_type_e type = SOUND_SESSION_TYPE_MEDIA;
		sound_session_option_for_starting_e option_s;
		sound_session_option_for_during_play_e option_d;
		if (sound_manager_set_session_type(type) != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to set media session type\n");
		else {
			if (sound_manager_get_media_session_option(&option_s, &option_d) != SOUND_MANAGER_ERROR_NONE)
				g_print("fail to get media session option\n");
			else
				g_print("current media session options are (%d) for starting, (%d) for ongoing\n", option_s, option_d);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_MEDIA_SESSION_RESUMPTION_OPTION: {
		sound_session_type_e type = SOUND_SESSION_TYPE_MEDIA;
		sound_session_option_for_resumption_e option_r;
		if (sound_manager_set_session_type(type) != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to set media session type\n");
		else {
			option_r = (sound_session_option_for_resumption_e)atoi(cmd);
			if (SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM > option_r || SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM_OR_MEDIA_PAUSED < option_r)
				g_print("not supported option type\n");
			else {
				if (sound_manager_set_media_session_resumption_option(option_r) != SOUND_MANAGER_ERROR_NONE)
					g_print("fail to set media session resumption option\n");
				else
					g_print("succese to set media session resumption option\n");
			}
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_MEDIA_SESSION_RESUMPTION_OPTION: {
		sound_session_type_e type = SOUND_SESSION_TYPE_MEDIA;
		sound_session_option_for_resumption_e option_r;
		if (sound_manager_set_session_type(type) != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to set media session type\n");
		else {
			if (sound_manager_get_media_session_resumption_option(&option_r) != SOUND_MANAGER_ERROR_NONE)
				g_print("fail to get media session resumption option\n");
			else
				g_print("current media session resumption option is : %d\n", option_r);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_VOIP_SESSION_MODE: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		sound_session_voip_mode_e mode;
		mode = (sound_session_voip_mode_e)atoi(cmd);
		ret = sound_manager_set_voip_session_mode(mode);
		if (ret)
			g_print("failed to set voip session mode(%d), ret[0x%x]\n", mode, ret);
		else
			g_print("success to set voip session mode\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_VOIP_SESSION_MODE: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		sound_session_voip_mode_e mode;
		ret = sound_manager_get_voip_session_mode(&mode);
		if (ret)
			g_print("fail to get voip session mode, ret[0x%x]\n", ret);
		else
			g_print("success to get voip session mode, mode[%d]\n", mode);
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_SESSION_INTERRUPTED_CB: {
		if (sound_manager_set_session_interrupted_cb(_set_session_interrupted_cb, NULL) != SOUND_MANAGER_ERROR_NONE)
			g_print("fail to set interrupted changed cb\n");
		else
			g_print("success to set interrupted changed cb\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_UNSET_SESSION_INTERRUPTED_CB: {
		if (sound_manager_unset_session_interrupted_cb() != 0)
			g_print("fail to unset interrupted changed cb\n");
		else
			g_print("success to unset interrupted changed cb\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_DEVICE_MASK: {
		if (strncmp(cmd, "0", 1) == 0) {
				g_device_mask = SOUND_DEVICE_ALL_MASK;
				reset_menu_state();
		} else if (strncmp(cmd, "1", 1) == 0) {
			if (g_device_mask == SOUND_DEVICE_ALL_MASK)
				g_device_mask = SOUND_DEVICE_TYPE_INTERNAL_MASK;
			else
				g_device_mask |= SOUND_DEVICE_TYPE_INTERNAL_MASK;
			g_print("add TYPE_INTERNAL MASK\n");
		} else if (strncmp(cmd, "2", 1) == 0) {
			if (g_device_mask == SOUND_DEVICE_ALL_MASK)
				g_device_mask = SOUND_DEVICE_TYPE_EXTERNAL_MASK;
			else
				g_device_mask |= SOUND_DEVICE_TYPE_EXTERNAL_MASK;
			g_print("add TYPE_EXTERNAL MASK\n");
		} else if (strncmp(cmd, "3", 1) == 0) {
			if (g_device_mask == SOUND_DEVICE_ALL_MASK)
				g_device_mask = SOUND_DEVICE_IO_DIRECTION_IN_MASK;
			else
				g_device_mask |= SOUND_DEVICE_IO_DIRECTION_IN_MASK;
			g_print("add IO_DIRECTION_IN MASK\n");
		} else if (strncmp(cmd, "4", 1) == 0) {
			if (g_device_mask == SOUND_DEVICE_ALL_MASK)
				g_device_mask = SOUND_DEVICE_IO_DIRECTION_OUT_MASK;
			else
				g_device_mask |= SOUND_DEVICE_IO_DIRECTION_OUT_MASK;
			g_print("add IO_DIRECTION_OUT MASK\n");
		} else if (strncmp(cmd, "5", 1) == 0) {
			if (g_device_mask == SOUND_DEVICE_ALL_MASK)
				g_device_mask = SOUND_DEVICE_IO_DIRECTION_BOTH_MASK;
			else
				g_device_mask |= SOUND_DEVICE_IO_DIRECTION_BOTH_MASK;
			g_print("add IO_DIRECTION_BOTH MASK\n");
		} else if (strncmp(cmd, "6", 1) == 0) {
			if (g_device_mask == SOUND_DEVICE_ALL_MASK)
				g_device_mask = SOUND_DEVICE_STATE_ACTIVATED_MASK;
			else
				g_device_mask |= SOUND_DEVICE_STATE_ACTIVATED_MASK;
			g_print("add STATE_ACTIVATED MASK\n");
		} else if (strncmp(cmd, "7", 1) == 0) {
			if (g_device_mask == SOUND_DEVICE_ALL_MASK)
				g_device_mask = SOUND_DEVICE_STATE_DEACTIVATED_MASK;
			else
				g_device_mask |= SOUND_DEVICE_STATE_DEACTIVATED_MASK;
			g_print("add STATE_DEACTIVATED MASK\n");
		} else if (strncmp(cmd, "b", 1) == 0) {
			g_print("device mask[0x%x]\n", g_device_mask);
			reset_menu_state();
		} else {
			g_print("invalid selection, please select again..\n");
		}
		break;
	}
	case CURRENT_STATUS_GET_DEVICE_MASK: {
		g_print("current device mask[0x%x]\n", g_device_mask);
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_DEVICE_LIST: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		if (!(ret = sound_manager_get_current_device_list(g_device_mask, &g_device_list)))
			g_print("success to get current device list\n");
		else
			g_print("fail to get current device list, ret[0x%x]\n", ret);
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_FREE_DEVICE_LIST: {
		sound_manager_free_device_list(g_device_list);
		g_device_list = NULL;
		g_print("device list freed\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_DEVICE_NEXT: {
		sound_device_h device;
		sound_device_type_e type;
		sound_device_io_direction_e io_direction;
		sound_device_state_e state;
		int id;
		char *name;
		int ret = SOUND_MANAGER_ERROR_NONE;
		if (!(ret = sound_manager_get_next_device(g_device_list, &device))) {
			g_print("success to get next device\n");
			if ((ret = sound_manager_get_device_type(device, &type)))
				g_print("failed to get device type, ret[0x%x]\n", ret);
			if ((ret = sound_manager_get_device_io_direction(device, &io_direction)))
				g_print("failed to get device io direction, ret[0x%x]\n", ret);
			if ((ret = sound_manager_get_device_id(device, &id)))
				g_print("failed to get device id, ret[0x%x]\n", ret);
			if ((ret = sound_manager_get_device_name(device, &name)))
				g_print("failed to get device name, ret[0x%x]\n", ret);
			if ((ret = sound_manager_get_device_state(device, &state)))
				g_print("failed to get device state, ret[0x%x]\n", ret);
			if (!ret)
				g_print("-- NEXT device type[%d], io_direction[%d], id[%d], name[%s], state[%d]\n", type, io_direction, id, name, state);
		} else {
			g_print("failed to get next device, ret[0x%x]\n", ret);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_DEVICE_PREV: {
		sound_device_h device;
		sound_device_type_e type;
		sound_device_io_direction_e io_direction;
		sound_device_state_e state;
		int id;
		char *name;
		int ret = SOUND_MANAGER_ERROR_NONE;
		if (!(ret = sound_manager_get_prev_device(g_device_list, &device))) {
			g_print("success to get previous device\n");
			if ((ret = sound_manager_get_device_type(device, &type)))
				g_print("failed to get device type, ret[0x%x]\n", ret);
			if ((ret = sound_manager_get_device_io_direction(device, &io_direction)))
				g_print("failed to get device io direction, ret[0x%x]\n", ret);
			if ((ret = sound_manager_get_device_id(device, &id)))
				g_print("failed to get device id, ret[0x%x]\n", ret);
			if ((ret = sound_manager_get_device_name(device, &name)))
				g_print("failed to get device name, ret[0x%x]\n", ret);
			if ((ret = sound_manager_get_device_state(device, &state)))
				g_print("failed to get device state, ret[0x%x]\n", ret);
			if (!ret)
				g_print("-- PREV device type[%d], io_direction[%d], id[%d], name[%s], state[%d]\n", type, io_direction, id, name, state);
		} else {
			g_print("failed to get previous device, ret[0x%x]\n", ret);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_DEVICE_CONNECTED_CB: {
		if (sound_manager_set_device_connected_cb(g_device_mask, _set_device_connected_cb, NULL))
			g_print("fail to set device connected cb\n");
		else
			g_print("success to set device connected cb\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_UNSET_DEVICE_CONNECTED_CB: {
		if (sound_manager_unset_device_connected_cb())
			g_print("fail to unset device connected cb\n");
		else
			g_print("success to unset device connected cb\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_DEVICE_INFO_CHANGED_CB: {
		if (sound_manager_set_device_information_changed_cb(g_device_mask, _set_device_info_changed_cb, NULL))
			g_print("fail to set device information changed cb\n");
		else
			g_print("success to set device information changed cb\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_UNSET_DEVICE_INFO_CHANGED_CB: {
		if (sound_manager_unset_device_information_changed_cb())
			g_print("fail to unset device information changed cb\n");
		else
			g_print("success to unset device information changed cb\n");
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_CREATE_STREAM_INFO: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		int stream_type = 0;
		sound_stream_type_e type = SOUND_STREAM_TYPE_MEDIA;
		if (g_stream_info_h) {
			g_print("fail to create stream information, g_stream_info_h(%p) is already set\n", g_stream_info_h);
			reset_menu_state();
			break;
		}
		stream_type = atoi(cmd);
		switch (stream_type) {
		case 0: /* media */
			type = SOUND_STREAM_TYPE_MEDIA;
			break;
		case 1: /* alarm */
			type = SOUND_STREAM_TYPE_ALARM;
			break;
		case 2: /* notification */
			type = SOUND_STREAM_TYPE_NOTIFICATION;
			break;
		case 3: /* ringtone for call*/
			type = SOUND_STREAM_TYPE_RINGTONE_CALL;
			break;
		case 4: /* voice call */
			type = SOUND_STREAM_TYPE_VOICE_CALL;
			break;
		case 5: /* voip */
			type = SOUND_STREAM_TYPE_VOIP;
			break;
		case 6: /* media only for external devices */
			type = SOUND_STREAM_TYPE_MEDIA_EXTERNAL_ONLY;
			break;
		default:
			type = SOUND_STREAM_TYPE_MEDIA;
			break;
		}
		if (type == (int)SOUND_STREAM_TYPE_RINGTONE_CALL || type == (int)SOUND_STREAM_TYPE_VOICE_CALL)
			ret = sound_manager_create_stream_information_internal(type, focus_callback, NULL, &g_stream_info_h);
		else
			ret = sound_manager_create_stream_information(type, focus_callback, NULL, &g_stream_info_h);

		if (ret)
			g_print("fail to sound_manager_create_stream_information(), ret(0x%x)\n", ret);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_ADD_DEVICE_FOR_STREAM_ROUTING: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		int device_type = 0;
		sound_device_h device = NULL;
		sound_device_type_e selected_type = SOUND_DEVICE_BUILTIN_SPEAKER;
		sound_device_type_e type = SOUND_DEVICE_BUILTIN_SPEAKER;
		bool need_to_go = false;

		device_type = atoi(cmd);
		switch (device_type) {
		case 0: /* built-in mic */
			selected_type = SOUND_DEVICE_BUILTIN_MIC;
			break;
		case 1: /* built-in spk */
			selected_type = SOUND_DEVICE_BUILTIN_SPEAKER;
			break;
		case 2: /* built-in rcv */
			selected_type = SOUND_DEVICE_BUILTIN_RECEIVER;
			break;
		case 3: /* audio-jack */
			selected_type = SOUND_DEVICE_AUDIO_JACK;
			break;
		case 4: /* bt */
			selected_type = SOUND_DEVICE_BLUETOOTH;
			break;
		default:
			g_print("invalid argument, device_type(%d) is not valid for this feature\n", device_type);
			reset_menu_state();
			break;
		}
		if (!(ret = sound_manager_get_current_device_list(SOUND_DEVICE_ALL_MASK, &g_device_list))) {
			g_print("success to get current device list\n");
			while (!sound_manager_get_next_device(g_device_list, &device)) {
				if (!(ret = sound_manager_get_device_type(device, &type))) {
					if (selected_type == type) {
						need_to_go = true;
						break;
					}
				} else {
					g_print("fail to get type of device, ret(0x%x)\n", ret);
					reset_menu_state();
					break;
				}
			}
			if (need_to_go) {
				ret = sound_manager_add_device_for_stream_routing(g_stream_info_h, device);
				if (ret)
					g_print("failed to sound_manager_add_device_for_stream_routing(), ret(0x%x)\n", ret);
			} else
				g_print("the device is not available now\n");
			reset_menu_state();
		} else {
			g_print("fail to get current device list, ret(0x%x)\n", ret);
			reset_menu_state();
		}
		break;
	}
	case CURRENT_STATUS_REMOVE_DEVICE_FOR_STREAM_ROUTING: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		int device_type = 0;
		sound_device_h device = NULL;
		sound_device_type_e selected_type = SOUND_DEVICE_BUILTIN_SPEAKER;
		sound_device_type_e type = SOUND_DEVICE_BUILTIN_SPEAKER;
		bool need_to_go = false;

		device_type = atoi(cmd);
		switch (device_type) {
		case 0: /* built-in mic */
			selected_type = SOUND_DEVICE_BUILTIN_MIC;
			break;
		case 1: /* built-in spk */
			selected_type = SOUND_DEVICE_BUILTIN_SPEAKER;
			break;
		case 2: /* built-in rcv */
			selected_type = SOUND_DEVICE_BUILTIN_RECEIVER;
			break;
		case 3: /* audio-jack */
			selected_type = SOUND_DEVICE_AUDIO_JACK;
			break;
		case 4: /* bt */
			selected_type = SOUND_DEVICE_BLUETOOTH;
			break;
		default:
			g_print("invalid argument, device_type(%d) is not valid for this feature\n", device_type);
			reset_menu_state();
			break;
		}
		if (!(ret = sound_manager_get_current_device_list(SOUND_DEVICE_ALL_MASK, &g_device_list))) {
			g_print("success to get current device list\n");
			while (!sound_manager_get_next_device(g_device_list, &device)) {
				if (!(ret = sound_manager_get_device_type(device, &type))) {
					if (selected_type == type) {
						need_to_go = true;
						break;
					}
				} else {
					g_print("fail to get type of device, ret(0x%x)\n", ret);
					reset_menu_state();
					break;
				}
			}
			if (need_to_go) {
				ret = sound_manager_remove_device_for_stream_routing(g_stream_info_h, device);
				if (ret)
					g_print("failed to sound_manager_remove_device_for_stream_routing(), ret(0x%x)\n", ret);
				else
					g_print("the device is not available now\n");
				reset_menu_state();
			} else {
				g_print("fail to get current device list, ret(0x%x)\n", ret);
				reset_menu_state();
			}
		}
		break;
	}
	case CURRENT_STATUS_APPLY_STREAM_ROUTING: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		ret = sound_manager_apply_stream_routing(g_stream_info_h);
		if (ret)
			g_print("failed to sound_manager_apply_stream_routing(), ret(0x%x)\n", ret);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_STREAM_ROUTING_OPTION: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		int selection = 0;
		char *name = NULL;
		int value = -1;

		selection = atoi(cmd);
		switch (selection) {
		case 0:
			name = "option_1";
			value = 0;
			break;
		case 1:
			name = "option_1";
			value = 1;
			break;
		case 2:
			name = "option_2";
			value = 0;
			break;
		case 3:
			name = "option_2";
			value = 1;
			break;
		default:
			g_print("invalid argument, try again..\n");
			reset_menu_state();
			break;
		}
		ret = sound_manager_set_stream_routing_option(g_stream_info_h, name, value);
		if (ret)
			g_print("failed to sound_manager_set_stream_routing_option(), ret(0x%x)\n", ret);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_ACQUIRE_FOCUS: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		int focus_type = 0;
		sound_stream_focus_mask_e focus_mask;

		focus_type = atoi(cmd);
		switch (focus_type) {
		case 0: /* playback */
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK;
			break;
		case 1: /* recording */
			focus_mask = SOUND_STREAM_FOCUS_FOR_RECORDING;
			break;
		case 2: /* all */
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK | SOUND_STREAM_FOCUS_FOR_RECORDING;
			break;
		default:
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK;
			break;
		}
		ret = sound_manager_acquire_focus(g_stream_info_h, focus_mask, NULL);
		if (ret)
			g_print("fail to sound_manager_acquire_focus(), ret(0x%x)\n", ret);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_RELEASE_FOCUS: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		int focus_type = 0;
		sound_stream_focus_mask_e focus_mask;

		focus_type = atoi(cmd);
		switch (focus_type) {
		case 0: /* playback */
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK;
			break;
		case 1: /* recording */
			focus_mask = SOUND_STREAM_FOCUS_FOR_RECORDING;
			break;
		case 2: /* both */
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK | SOUND_STREAM_FOCUS_FOR_RECORDING;
			break;
		default:
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK;
			break;
		}
		ret = sound_manager_release_focus(g_stream_info_h, focus_mask, NULL);
		if (ret)
				g_print("fail to sound_manager_acquire_focus(), ret(0x%x)\n", ret);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_ACQUIRED_FOCUS: {
		sound_stream_focus_state_e for_playback;
		sound_stream_focus_state_e for_recording;
		int ret = SOUND_MANAGER_ERROR_NONE;
		if (g_stream_info_h) {
			ret = sound_manager_get_focus_state(g_stream_info_h, &for_playback, &for_recording);
			if (ret)
				g_print("fail to sound_manager_get_focus_state(), ret(0x%x)\n", ret);
			else
				g_print("focus_state(playback:%d, capture:%d)\n", for_playback, for_recording);

		} else
			g_print("please create stream info. first\n");

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_SOUND_TYPE: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		sound_type_e sound_type;
		if (g_stream_info_h) {
			ret = sound_manager_get_sound_type(g_stream_info_h, &sound_type);
			if (ret)
				g_print("fail to sound_manager_get_sound_type(), ret(0x%x)\n", ret);
			else
				g_print("sound_type(%d)\n", sound_type);

		} else
			g_print("please create stream info. first\n");

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_DESTROY_STREAM_INFO: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		ret = sound_manager_destroy_stream_information(g_stream_info_h);
		if (ret)
			g_print("fail to sound_manager_destroy_stream_information(), ret(0x%x)\n", ret);
		else
			g_stream_info_h = NULL;

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_FOCUS_WATCH_CB: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		int focus_type = 0;
		sound_stream_focus_mask_e focus_mask;

		focus_type = atoi(cmd);
		switch (focus_type) {
		case 0: /* playback */
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK;
			break;
		case 1: /* recording */
			focus_mask = SOUND_STREAM_FOCUS_FOR_RECORDING;
			break;
		case 2: /* both */
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK | SOUND_STREAM_FOCUS_FOR_RECORDING;
			break;
		default:
			focus_mask = SOUND_STREAM_FOCUS_FOR_PLAYBACK;
			break;
		}
		ret = sound_manager_set_focus_state_watch_cb(focus_mask, focus_watch_callback, NULL);
		if (ret)
			g_print("fail to sound_manager_set_focus_state_watch_cb(), ret(0x%x)\n", ret);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_UNSET_FOCUS_WATCH_CB: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		ret = sound_manager_unset_focus_state_watch_cb();
		if (ret)
			g_print("fail to sound_manager_unset_focus_state_watch_cb(), ret(0x%x)\n", ret);
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_CREATE_VIRTUAL_STREAM: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		if (!g_stream_info_h || g_vstream_h) {
			g_print("fail to create virtual stream, g_stream_info_h(%p)/g_vstream_h(%p)\n", g_stream_info_h, g_vstream_h);
			reset_menu_state();
			break;
		}
		ret = sound_manager_create_virtual_stream(g_stream_info_h, &g_vstream_h);
		if (ret)
			g_print("fail to sound_manager_create_virtual_stream(), ret(0x%x)\n", ret);
		else
			g_print("success to sound_manager_create_virtual_stream(), ret(0x%x)\n", ret);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_START_VIRTUAL_STREAM: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		if (g_vstream_h) {
			ret = sound_manager_start_virtual_stream(g_vstream_h);
			if (ret)
				g_print("fail to sound_manager_start_virtual_stream(), ret(0x%x)\n", ret);
			else
				g_print("success to sound_manager_start_virtual_stream(), ret(0x%x)\n", ret);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_STOP_VIRTUAL_STREAM: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		if (g_vstream_h) {
			ret = sound_manager_stop_virtual_stream(g_vstream_h);
			if (ret)
				g_print("fail to sound_manager_stop_virtual_stream(), ret(0x%x)\n", ret);
			else
				g_print("success to sound_manager_stop_virtual_stream(), ret(0x%x)\n", ret);
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_DESTROY_VIRTUAL_STREAM: {
		int ret = SOUND_MANAGER_ERROR_NONE;
		if (g_vstream_h) {
			ret = sound_manager_destroy_virtual_stream(g_vstream_h);
			if (ret)
				g_print("fail to sound_manager_destroy_virtual_stream(), ret(0x%x)\n", ret);
			else
				g_print("success to sound_manager_destroy_virtual_stream(), ret(0x%x)\n", ret);
			g_vstream_h = NULL;
		}
		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_MAX_MASTER_VOLUME: {
		int max_level;
		if (sound_manager_get_max_master_volume(&max_level) != SOUND_MANAGER_ERROR_NONE)
			g_print("failed to get max master volume\n");
		else
			g_print("the max level of master volume is %d\n", max_level);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_SET_MASTER_VOLUME: {
		int level;
		level = atoi(cmd);
		if (sound_manager_set_master_volume(level) != SOUND_MANAGER_ERROR_NONE)
			g_print("failed to set master volume(%d)\n", level);
		else
			g_print("set master volume success : level(%d)\n", level);

		reset_menu_state();
		break;
	}
	case CURRENT_STATUS_GET_MASTER_VOLUME: {
		int level;
		if (sound_manager_get_master_volume(&level) != SOUND_MANAGER_ERROR_NONE)
			g_print("failed to get master volume\n");
		else
			g_print("current master volume is : %d\n", level);

		reset_menu_state();
		break;
	}
	}
	g_timeout_add(100, timeout_menu_display, 0);
}

gboolean input(GIOChannel *channel)
{
	gchar buf[MAX_STRING_LEN];
	gsize read;
	GError *error = NULL;

	g_io_channel_read_chars(channel, buf, MAX_STRING_LEN, &read, &error);
	buf[read] = '\0';
	g_strstrip(buf);
	interpret(buf);

	return TRUE;
}

int main(int argc, char *argv[])
{
	GIOChannel *stdin_channel;
	stdin_channel = g_io_channel_unix_new(0);
	g_io_channel_set_flags(stdin_channel, G_IO_FLAG_NONBLOCK, NULL);
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)input, NULL);
	g_loop = g_main_loop_new(NULL, 1);

	displaymenu();
	g_main_loop_run(g_loop);

	return 0;
}
