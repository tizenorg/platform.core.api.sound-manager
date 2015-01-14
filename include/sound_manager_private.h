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

#ifndef __TIZEN_MEDIA_SOUND_MANAGER_PRIVATE_H__
#define __TIZEN_MEDIA_SOUND_MANAGER_PRIVATE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <dlog.h>
#include <mm_session.h>
#include <mm_session_private.h>
#include <mm_sound.h>
#include <mm_sound_private.h>
#include "sound_manager_internal.h"

#define SOUND_SESSION_TYPE_DEFAULT SOUND_SESSION_TYPE_MEDIA

typedef struct {
	int is_registered;
	void *user_data;
	sound_session_interrupted_cb user_cb;
}_session_interrupt_info_s;

typedef struct {
	void *user_data;
	sound_manager_volume_changed_cb user_cb;
}_volume_changed_info_s;

typedef struct {
	void *user_data;
	sound_device_connected_cb user_cb;
}_device_connected_info_s;

typedef struct {
	void *user_data;
	sound_device_information_changed_cb user_cb;
}_device_changed_info_s;

int __convert_sound_manager_error_code(const char *func, int code);

void __session_interrupt_cb(session_msg_t msg, session_event_t event, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_SOUND_MANAGER_PRIVATE_H__ */
