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




#ifndef __TIZEN_MEDIA_SOUND_MANAGER_H__
#define __TIZEN_MEDIA_SOUND_MANAGER_H__

#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define SOUND_MANAGER_ERROR_CLASS          TIZEN_ERROR_MULTIMEDIA_CLASS | 0x30

/**
 * @file sound_manager.h
 * @brief This file contains the Sound Manager API
 */

/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_MODULE
 * @{
 */

/**
 * @brief Enumerations of sound type
 */
typedef enum
{
    SOUND_TYPE_SYSTEM,          /**< Sound type for system */
    SOUND_TYPE_NOTIFICATION,    /**< Sound type for notifications */
    SOUND_TYPE_ALARM,           /**< Sound type for alarm */
    SOUND_TYPE_RINGTONE,        /**< Sound type for ringtones */
    SOUND_TYPE_MEDIA,           /**< Sound type for media */
    SOUND_TYPE_CALL,            /**< Sound type for call */
} sound_type_e;

/**
 * @brief Enumerations of volume key type
 */
typedef enum
{
    VOLUME_KEY_TYPE_NONE=-1,          /**< Volume key type for current played sound */
    VOLUME_KEY_TYPE_SYSTEM=0,          /**< Volume key type for system sound */
    VOLUME_KEY_TYPE_NOTIFICATION,    /**< Volume key type for notifications sound*/
    VOLUME_KEY_TYPE_ALARM,           /**< Volume key type for alarm sound */
    VOLUME_KEY_TYPE_RINGTONE,        /**< Volume key type for ringtones sound */
    VOLUME_KEY_TYPE_MEDIA,           /**< Volume key type for media sound */
    VOLUME_KEY_TYPE_CALL,            /**< Volume key type for call sound */
} volume_key_type_e;

/**
 * @brief Enumerations of sound route policy
 */
typedef enum {
    SOUND_ROUTE_DEFAULT,        /**< Audio device priority: 1. bluetooth headset, 2. wired headset 3. built-in speaker and microphone. */
    SOUND_ROUTE_IGNORE_A2DP,    /**< Audio device priority: 1. wired headset, 2. built-in speaker and microphone. */
    SOUND_ROUTE_HANDSET_ONLY    /**< Use only built-in speaker and microphone */
} sound_route_policy_e;

/**
 * @brief Enumerations of sound device
 */
typedef enum {
    SOUND_DEVICE_NONE,          /**< Abnormal case */
    SOUND_DEVICE_HANDSET,       /**< Speaker, Wired headset, Wired earpiece */
    SOUND_DEVICE_BLUETOOTH      /**< Bluetooth */
} sound_device_e;

/**
 * @brief error codes for sound manager
 */
typedef enum{
    SOUND_MANAGER_ERROR_NONE        = TIZEN_ERROR_NONE,                      /**< Successful */
    SOUND_MANAGER_ERROR_OUT_OF_MEMORY	    = TIZEN_ERROR_OUT_OF_MEMORY,           /**< Out of memory */
    SOUND_MANAGER_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,       /**< Invalid parameter */
    SOUND_MANAGER_ERROR_INVALID_OPERATION = TIZEN_ERROR_INVALID_OPERATION,       /**< Invalid operation */
    SOUND_MANAGER_ERROR_NO_PLAYING_SOUND  = SOUND_MANAGER_ERROR_CLASS | 01,    /**< No playing sound */
} sound_manager_error_e;

/**
 * @brief Enumerations of session type
 */
typedef enum{
	SOUND_SESSION_TYPE_SHARE = 0,			/**< Share type : shares it's session with other share type application. */
	SOUND_SESSION_TYPE_EXCLUSIVE,			/**< Exclusive type : make previous session stop.*/
} sound_session_type_e;

/**
 * @brief Enumerations of session notification
 */
typedef enum{
	SOUND_SESSION_NOTIFY_STOP = 0, 			/**< Stop : session of application has interrupted by policy. */
	SOUND_SESSION_NOTIFY_RESUME, 		   /**< Resume : session interrupt of application has ended. */
} sound_session_notify_e;


/**
 * @brief Called when the sound session notification has occured.
 * @param[in]   notify	The sound session notification
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre You should register this callback by sound_manager_set_session_notify_cb()
 * @see sound_manager_set_session_notify_cb()
 */
typedef void (*sound_session_notify_cb) (sound_session_notify_e notify, void *user_data);

/**
 * @brief Called when the system volume has changed.
 * @param[in]   type	The sound type of changed volume
 * @param[in]   volume	The new volume value
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre sound_manager_set_volume() will invoke this callback if you register it using sound_manager_set_volume_changed_cb()
 * @see sound_manager_set_volume_changed_cb()
 * @see sound_manager_unset_volume_changed_cb()
 */
typedef void (*sound_manager_volume_changed_cb)(sound_type_e type, unsigned int volume, void *user_data);

/**
 * @brief Called when the sound route policy has changes.
 * @param[in]   route The new sound route policy
 * @param[in]   user_data The user data passed from the callback registration function
 * @pre sound_manager_set_route_policy() will invoke this callback if you register it using sound_manager_set_route_policy_changed_cb()
 * @see sound_manager_set_route_policy_changed_cb()
 * @see sound_manager_unset_route_policy_changed_cb()
 */
typedef void (*sound_manager_route_policy_changed_cb)(sound_route_policy_e route, void *user_data);

/**
 * @brief Gets the maximum volume level supported for a particular sound type
 * @param[in]		type The sound type
 * @param[out]	max	The maximum volume level
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_set_volume()
 */
int sound_manager_get_max_volume(sound_type_e type, int *max);

/**
 * @brief Sets the volume level specified for a particular sound type
 * @param[in]		type The sound type
 * @param[out]	volume	The volume level to be set
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_max_volume()
 * @see sound_manager_get_volume()
 */
int sound_manager_set_volume(sound_type_e type, int volume);

/**
 * @brief Gets the volume level specified for a particular sound type
 * @param[in]		type The sound type
 * @param[out]	volume	The current volume level
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_max_volume()
 * @see sound_manager_set_volume()
 */
int sound_manager_get_volume(sound_type_e type, int *volume);

/**
 * @brief Sets the application's sound route policy
 * @param[in]		route  The route policy to set
 * @return 0 on success, otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_route_policy()
 */
int sound_manager_set_route_policy (sound_route_policy_e route);

/**
 * @brief Gets the current application's route policy
 * @param[out]	route  The current route policy
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_set_route_policy()
 */
int sound_manager_get_route_policy (sound_route_policy_e *route);

/**
 * @brief Gets the current playing sound type
 * @param[out]		type The current sound type
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NO_PLAYING_SOUND No playing sound
 * @see sound_manager_get_current_sound_device()
 * @see player_set_sound_type()
 * @see audio_out_create()
 * @see wav_player_start()
 */
int sound_manager_get_current_sound_type(sound_type_e *type);

/**
 * @brief Gets the current device type
 * @param[out]	device The current sound device
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NO_PLAYING_SOUND No playing sound 
 * @see sound_manager_get_current_sound_type()
 */
int sound_manager_get_current_sound_device(sound_device_e *device);

/**
 * @brief Registers a callback function to be invoked when the volume level is changed.
 * @param[in]	callback	Callback function to indicate change in volume
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  sound_manager_volume_changed_cb() will be invoked
 * @see sound_manager_unset_volume_changed_cb()
 * @see sound_manager_volume_changed_cb()
 */
int sound_manager_set_volume_changed_cb(sound_manager_volume_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the volume change callback
 * @see sound_manager_set_volume_changed_cb()
 */
void sound_manager_unset_volume_changed_cb(void);

/**
 * @brief Registers a callback function to be invoked when the route policy is changed.
 * @param[in]	callback	Callback function to indicate change in route policy 
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  sound_manager_route_policy_changed_cb() will be invoked
 * @see sound_manager_unset_route_policy_changed_cb()
 * @see sound_manager_route_policy_changed_cb()
 */
int sound_manager_set_route_policy_changed_cb(sound_manager_route_policy_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback for sound route policy change
 * @see sound_manager_set_route_policy_changed_cb()
 */
void sound_manager_unset_route_policy_changed_cb(void);


/**
 * @brief Gets the A2DP activation information.
 * @remarks If @a connected is @c true,  @a bt_name must be released with free() by you. If @a connected is @c false, @a bt_name is set to NULL.
 * @param[out] connected The Bluetooth A2DP connection status (@c true = connected, @c false = disconnected)
 * @param[out] bt_name The Bluetooth A2DP connected device name
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_OPERATION Invalid operation
 */
int sound_manager_get_a2dp_status(bool *connected, char **bt_name);


/**
 * @brief Sets the application's sound session type
 * @param[in] type The session type to set
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 */
int sound_manager_set_session_type(sound_session_type_e type);

/**
 * @brief Registers a callback function to be invoked when the sound session notification is occured.
 * @param[in]	callback	The session notify callback function
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  sound_session_notify_cb() will be invoked
 * @see sound_manager_unset_session_notify_cb()
 * @see sound_session_notify_cb()
 */
int sound_manager_set_session_notify_cb(sound_session_notify_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function which is called when the session notification is occured
 * @see sound_manager_set_session_notify_cb()
 */
void sound_manager_unset_session_notify_cb(void);

/**
 * @brief Sets the volume key type
 * @param[in] type The volume key type to set
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 */
int sound_manager_set_volume_key_type(volume_key_type_e type);


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_SOUND_MANAGER_H__ */
