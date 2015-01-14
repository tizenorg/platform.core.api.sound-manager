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
#include <dlog.h>

extern _session_interrupt_info_s g_session_interrupt_cb_table;

int __convert_sound_manager_error_code(const char *func, int code) {
	int ret = SOUND_MANAGER_ERROR_NONE;
	char *errorstr = NULL;

	switch(code)
	{
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
	}
	LOGE("[%s] %s(0x%08x) : core frameworks error code(0x%08x)",func, errorstr, ret, code);
	return ret;
}

void __session_interrupt_cb(session_msg_t msg, session_event_t event, void *user_data){
	if( g_session_interrupt_cb_table.user_cb ){
		sound_session_interrupted_code_e e = SOUND_SESSION_INTERRUPTED_COMPLETED;
		if( msg == MM_SESSION_MSG_RESUME )
			e = SOUND_SESSION_INTERRUPTED_COMPLETED;
		else{
#ifndef TEMP_COMMENT_UNTIL_MM_SOUND_READY
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
#endif
		}
		g_session_interrupt_cb_table.user_cb(e, g_session_interrupt_cb_table.user_data);
	}
}
