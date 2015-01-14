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

#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif

#include <tizen.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file sound_manager.h
 * @brief This file contains the Sound Manager API.
 */

/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_MODULE
 * @{
 */

/**
 * @brief Enumeration for sound manager's error codes.
 * @since_tizen 2.3
 */
typedef enum
{
    SOUND_MANAGER_ERROR_NONE              = TIZEN_ERROR_NONE,                    /**< Successful */
    SOUND_MANAGER_ERROR_OUT_OF_MEMORY     = TIZEN_ERROR_OUT_OF_MEMORY,           /**< Out of memory */
    SOUND_MANAGER_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER,       /**< Invalid parameter */
    SOUND_MANAGER_ERROR_INVALID_OPERATION = TIZEN_ERROR_INVALID_OPERATION,       /**< Invalid operation */
    SOUND_MANAGER_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED,       /**< Permission denied */
    SOUND_MANAGER_ERROR_NOT_SUPPORTED     = TIZEN_ERROR_NOT_SUPPORTED,           /**< Not supported */
    SOUND_MANAGER_ERROR_NO_DATA           = TIZEN_ERROR_NO_DATA,                 /**< No data */
    SOUND_MANAGER_ERROR_INTERNAL          = TIZEN_ERROR_SOUND_MANAGER | 01,      /**< Internal error inside the sound system */
    SOUND_MANAGER_ERROR_POLICY            = TIZEN_ERROR_SOUND_MANAGER | 02,      /**< Noncompliance with the sound system policy */
    SOUND_MANAGER_ERROR_NO_PLAYING_SOUND  = TIZEN_ERROR_SOUND_MANAGER | 03,      /**< No playing sound */
} sound_manager_error_e;

/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_VOLUME_MODULE
 * @{
 */

/**
 * @brief Enumeration for sound type.
 * @since_tizen 2.3
 */
typedef enum
{
    SOUND_TYPE_SYSTEM,          /**< Sound type for system */
    SOUND_TYPE_NOTIFICATION,    /**< Sound type for notifications */
    SOUND_TYPE_ALARM,           /**< Sound type for alarm */
    SOUND_TYPE_RINGTONE,        /**< @internal Sound type for ringtones */
    SOUND_TYPE_MEDIA,           /**< Sound type for media */
    SOUND_TYPE_CALL,            /**< @internal Sound type for call */
    SOUND_TYPE_VOIP,            /**< @internal Sound type for voip */
    SOUND_TYPE_VOICE,           /**< Sound type for voice */
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
} volume_key_type_e DEPRECATED; // will be deprecated

/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_SESSION_MODULE
 * @{
 */

/**
 * @brief Enumeration for session type.
 * @since_tizen 2.3
 */
typedef enum
{
	SOUND_SESSION_TYPE_MEDIA = 0,		/**< Media type */
	SOUND_SESSION_TYPE_ALARM,			/**< Alarm type */
	SOUND_SESSION_TYPE_NOTIFICATION,		/**< Notification type */
	SOUND_SESSION_TYPE_EMERGENCY,			/**< Emergency type */
	SOUND_SESSION_TYPE_VOIP,			/**< VOIP type */
	SOUND_SESSION_TYPE_SHARE = SOUND_SESSION_TYPE_MEDIA,		/**< will be deprecated */
	SOUND_SESSION_TYPE_EXCLUSIVE = SOUND_SESSION_TYPE_MEDIA,	/**< will be deprecated */
} sound_session_type_e;

/**
 * @brief Enumeration for session option for starting.
 * @since_tizen 2.3
 */
typedef enum
{
	SOUND_SESSION_OPTION_MIX_WITH_OTHERS_WHEN_START = 0, /**< This session will be mixed with others when starting (default) */
	SOUND_SESSION_OPTION_PAUSE_OTHERS_WHEN_START,        /**< This session will interrupt other sessions when starting */
} sound_session_option_for_starting_e;

/**
 * @brief Enumeration for session option during play.
 * @since_tizen 2.3
 */
typedef enum
{
	SOUND_SESSION_OPTION_INTERRUPTIBLE_DURING_PLAY = 0,  /**< This session will be interrupted by other sessions during play (default) */
	SOUND_SESSION_OPTION_UNINTERRUPTIBLE_DURING_PLAY,    /**< This session will not be interrupted by other media sessions */
} sound_session_option_for_during_play_e;

/**
 * @brief Enumeration for session option for resumption.
 * @since_tizen 2.3
 */
typedef enum
{
	SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM = 0,             /**< This session will be resumed according to system policy (default) */
	SOUND_SESSION_OPTION_RESUMPTION_BY_SYSTEM_OR_MEDIA_PAUSED, /**< This session will be resumed according to system policy and when the media session which interrupted this session is paused */
} sound_session_option_for_resumption_e;

/**
 * @brief Enumeration for voip session mode.
 * @since_tizen 2.3
 */
typedef enum
{
	SOUND_SESSION_VOIP_MODE_RINGTONE = 0,                /**< voip mode for ringtone */
	SOUND_SESSION_VOIP_MODE_VOICE_WITH_BUILTIN_RECEIVER, /**< voip mode for during call with built-in receiver */
	SOUND_SESSION_VOIP_MODE_VOICE_WITH_BUILTIN_SPEAKER,  /**< voip mode for during call with built-in speaker */
	SOUND_SESSION_VOIP_MODE_VOICE_WITH_AUDIO_JACK,       /**< voip mode for during call with audio jack */
	SOUND_SESSION_VOIP_MODE_VOICE_WITH_BLUETOOTH,        /**< voip mode for during call with bluetooth */
} sound_session_voip_mode_e;

/**
 * @brief Enumeration for sound session interrupted type.
 * @since_tizen 2.3
 */
typedef enum
{
	SOUND_SESSION_INTERRUPTED_COMPLETED = 0, 				/**< Interrupt completed*/
	SOUND_SESSION_INTERRUPTED_BY_MEDIA, 				/**< Interrupted by media application*/
	SOUND_SESSION_INTERRUPTED_BY_CALL,						/**< Interrupted by an incoming call*/
	SOUND_SESSION_INTERRUPTED_BY_EARJACK_UNPLUG,			/**< Interrupted by unplugging headphones*/
	SOUND_SESSION_INTERRUPTED_BY_RESOURCE_CONFLICT,		/**< Interrupted by a resource conflict*/
	SOUND_SESSION_INTERRUPTED_BY_ALARM,					/**< Interrupted by an alarm*/
	SOUND_SESSION_INTERRUPTED_BY_EMERGENCY,					/**< Interrupted by an emergency*/
	SOUND_SESSION_INTERRUPTED_BY_NOTIFICATION,					/**< Interrupted by a notification*/
} sound_session_interrupted_code_e;

/**
 * @brief Called when the playing sound session is interrupted.
 * @since_tizen 2.3
 * @param[in]   code	The interrupted code
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre You should register this callback using sound_manager_set_session_interrupted_cb().
 * @see sound_manager_set_session_interrupted_cb()
 * @see sound_manager_unset_session_interrupted_cb()
 */
typedef void(* sound_session_interrupted_cb)(sound_session_interrupted_code_e code, void *user_data);



/**
 * @brief Enumerations of session notification
 */
typedef enum{
	SOUND_SESSION_NOTIFY_STOP = 0, 			/**< Stop : session of application has interrupted by policy. */
	SOUND_SESSION_NOTIFY_RESUME, 		   /**< Resume : session interrupt of application has ended. */
} sound_session_notify_e DEPRECATED; // will be deprecated

/**
 * @brief Enumerations of sound interrupted type
 */
typedef enum
{
       SOUND_INTERRUPTED_COMPLETED = 0, 				/**< Interrupt completed*/
       SOUND_INTERRUPTED_BY_MEDIA, 				/**< Interrupted by non-resumable media application*/
       SOUND_INTERRUPTED_BY_CALL,						/**< Interrupted by incoming call*/
       SOUND_INTERRUPTED_BY_EARJACK_UNPLUG,			/**< Interrupted by unplugging headphone*/
       SOUND_INTERRUPTED_BY_RESOURCE_CONFLICT,		/**< Interrupted by resource conflict*/
       SOUND_INTERRUPTED_BY_ALARM,					/**< Interrupted by alarm*/
       SOUND_INTERRUPTED_BY_EMERGENCY,					/**< Interrupted by emergency*/
       SOUND_INTERRUPTED_BY_RESUMABLE_MEDIA, 				/**< Interrupted by resumable media application*/
} sound_interrupted_code_e DEPRECATED; // will be deprecated

/**
 * @brief Called when the sound session notification has occured.
 * @param[in]   notify	The sound session notification
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre You should register this callback by sound_manager_set_session_notify_cb()
 * @see sound_manager_set_session_notify_cb()
 */
typedef void (*sound_session_notify_cb) (sound_session_notify_e notify, void *user_data) DEPRECATED; // will be deprecated

/**
 * @brief Called when the playing sound was interrupted.
 * @param[in]   code	The interrupted code
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre You should register this callback by sound_manager_set_interrupted_cb()
 * @see sound_manager_set_interrupted_cb()
 */
typedef void(* sound_interrupted_cb)(sound_interrupted_code_e code, void *user_data) DEPRECATED; // will be deprecated

/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_DEVICE_MODULE
 * @{
 */

/**
 * @brief sound device handle
 * @since_tizen 2.3
 */
typedef void* sound_device_h;

/**
 * @brief sound device list handle
 * @since_tizen 2.3
 */
typedef void* sound_device_list_h;

/**
* @brief Enumeration for sound device type.
* @since_tizen 2.3
*/
typedef enum{
	SOUND_DEVICE_BUILTIN_SPEAKER,   /**< Built-in speaker */
	SOUND_DEVICE_BUILTIN_RECEIVER,  /**< Built-in receiver */
	SOUND_DEVICE_BUILTIN_MIC,       /**< Built-in mic */
	SOUND_DEVICE_AUDIO_JACK,        /**< Audio jack that can be connected to wired accessory such as headphone, headset, and so on */
	SOUND_DEVICE_BLUETOOTH,         /**< Bluetooth */
	SOUND_DEVICE_HDMI,              /**< HDMI */
	SOUND_DEVICE_MIRRORING,         /**< MIRRORING */
	SOUND_DEVICE_USB_AUDIO,         /**< USB Audio */
} sound_device_type_e;

/**
* @brief Enumeration for sound device direction.
* @since_tizen 2.3
*/
typedef enum {
	SOUND_DEVICE_IO_DIRECTION_IN,   /**< Input device */
	SOUND_DEVICE_IO_DIRECTION_OUT,  /**< Output device */
	SOUND_DEVICE_IO_DIRECTION_BOTH, /**< Input/output device (both directions are available) */
} sound_device_io_direction_e;

/**
* @brief Enumeration for sound device state.
* @since_tizen 2.3
*/
typedef enum {
	SOUND_DEVICE_STATE_DEACTIVATED, /**< Deactivated state */
	SOUND_DEVICE_STATE_ACTIVATED,   /**< Activated state */
} sound_device_state_e;

/**
* @brief Enumeration for sound device mask.
* @since_tizen 2.3
*/
typedef enum {
	SOUND_DEVICE_IO_DIRECTION_IN_MASK      = 0x0001,  /**< Mask for input devices */
	SOUND_DEVICE_IO_DIRECTION_OUT_MASK     = 0x0002,  /**< Mask for output devices */
	SOUND_DEVICE_IO_DIRECTION_BOTH_MASK    = 0x0004,  /**< Mask for input/output devices (both directions are available) */
	SOUND_DEVICE_TYPE_INTERNAL_MASK        = 0x0010,  /**< Mask for built-in devices */
	SOUND_DEVICE_TYPE_EXTERNAL_MASK        = 0x0020,  /**< Mask for external devices */
	SOUND_DEVICE_STATE_DEACTIVATED_MASK    = 0x1000,  /**< Mask for deactivated devices */
	SOUND_DEVICE_STATE_ACTIVATED_MASK      = 0x2000,  /**< Mask for activated devices */
	SOUND_DEVICE_ALL_MASK                  = 0xFFFF,  /**< Mask for all devices */
} sound_device_mask_e;

/**
* @brief Enumeration for changed information of sound device.
* @since_tizen 2.3
*/
typedef enum {
	SOUND_DEVICE_CAHNGED_INFO_STATE,           /**< State of the device was changed */
	SOUND_DEVICE_CHANGED_INFO_IO_DIRECTION,    /**< IO direction of the device was changed */
} sound_device_changed_info_e;

/**
 * @brief Called when the state of connection of a sound device was changed.
 * @since_tizen 2.3
 * @param[in]   sound_device_h	The sound_device
 * @param[in]   is_connected	The state of device connection
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre You should register this callback using sound_manager_set_device_connected_cb().
 * @see sound_manager_set_device_connected_cb()
 * @see sound_manager_unset_device_connected_cb()
 */
typedef void(* sound_device_connected_cb)(sound_device_h device, bool is_connected, void *user_data);

/**
 * @brief Called when the information of a sound device was changed.
 * @since_tizen 2.3
 * @param[in]   sound_device_h	The sound_device
 * @param[in]   changed_info	The entry of sound device information
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre You should register this callback using sound_manager_set_device_information_changed_cb().
 * @see sound_manager_set_device_information_changed_cb()
 * @see sound_manager_unset_device_information_changed_cb()
 */
typedef void(* sound_device_information_changed_cb)(sound_device_h device, sound_device_changed_info_e changed_info, void *user_data);



/**
 * @brief Enumerations of audio input device type.
 */
typedef enum{
	SOUND_DEVICE_IN_MIC = 0x01, /**< Device builtin mic. */
	SOUND_DEVICE_IN_WIRED_ACCESSORY = 0x02,  	/**< Wired input devices */
	SOUND_DEVICE_IN_BT_SCO = 0x04,  	/**< Bluetooth SCO device */
} sound_device_in_e DEPRECATED;// will be deprecated

/**
 * @brief Enumerations of audio output device type.
 */
typedef enum{
	SOUND_DEVICE_OUT_SPEAKER = 0x01<<8, /**< Device builtin speaker */
	SOUND_DEVICE_OUT_RECEIVER = 0x02<<8, /**< Device builtin receiver */
	SOUND_DEVICE_OUT_WIRED_ACCESSORY = 0x04<<8,  	/**< Wired output devices such as headphone, headset, and so on. */
	SOUND_DEVICE_OUT_BT_SCO = 0x08<<8, /**< Bluetooth SCO device */
	SOUND_DEVICE_OUT_BT_A2DP = 0x10<<8,	/**< Bluetooth A2DP device */
	SOUND_DEVICE_OUT_DOCK = 0x020<<8,	/**< DOCK device */
	SOUND_DEVICE_OUT_HDMI = 0x040<<8,	/**< HDMI device */
	SOUND_DEVICE_OUT_WFD = 0x080<<8,	/**< WFD device */
	SOUND_DEVICE_OUT_USB_AUDIO = 0x100<<8,	/**< USB Audio device */
} sound_device_out_e DEPRECATED;// will be deprecated

/**
 * @brief Enumerations of route type.
 */
typedef enum{
	SOUND_ROUTE_OUT_SPEAKER = SOUND_DEVICE_OUT_SPEAKER, /**< Routing audio output to builtin device such as internal speaker. */
	SOUND_ROUTE_OUT_WIRED_ACCESSORY = SOUND_DEVICE_OUT_WIRED_ACCESSORY,/**< Routing audio output to wired accessory such as headphone, headset, and so on. */
	SOUND_ROUTE_OUT_BLUETOOTH = SOUND_DEVICE_OUT_BT_A2DP, /**< Routing audio output to bluetooth A2DP. */
	SOUND_ROUTE_OUT_DOCK = SOUND_DEVICE_OUT_DOCK, /**< Routing audio output to DOCK */
	SOUND_ROUTE_OUT_HDMI = SOUND_DEVICE_OUT_HDMI, /**< Routing audio output to HDMI */
	SOUND_ROUTE_OUT_WFD = SOUND_DEVICE_OUT_WFD, /**< Routing audio output to WFD */
	SOUND_ROUTE_OUT_USB_AUDIO = SOUND_DEVICE_OUT_USB_AUDIO, /**< Routing audio output to USB Audio */
	SOUND_ROUTE_IN_MIC = SOUND_DEVICE_IN_MIC, /**< Routing audio input to device builtin mic. */
	SOUND_ROUTE_IN_WIRED_ACCESSORY = SOUND_DEVICE_IN_WIRED_ACCESSORY, /**< Routing audio input to wired accessory. */
	SOUND_ROUTE_IN_MIC_OUT_RECEIVER = SOUND_DEVICE_IN_MIC |SOUND_DEVICE_OUT_RECEIVER, /**< Routing audio input to device builtin mic and routing audio output to builtin receiver*/
	SOUND_ROUTE_IN_MIC_OUT_SPEAKER = SOUND_DEVICE_IN_MIC |SOUND_DEVICE_OUT_SPEAKER , /**< Routing audio input to device builtin mic and routing audio output to builtin speaker */
	SOUND_ROUTE_IN_MIC_OUT_HEADPHONE = SOUND_DEVICE_IN_MIC | SOUND_DEVICE_OUT_WIRED_ACCESSORY,/**< Routing audio input to device builtin mic and routing audio output to headphone */
	SOUND_ROUTE_INOUT_HEADSET = SOUND_DEVICE_IN_WIRED_ACCESSORY | SOUND_DEVICE_OUT_WIRED_ACCESSORY,	/**< Routing audio input and output to headset*/
	SOUND_ROUTE_INOUT_BLUETOOTH = SOUND_DEVICE_IN_BT_SCO |SOUND_DEVICE_OUT_BT_SCO /**< Routing audio input and output to bluetooth SCO */
} sound_route_e DEPRECATED;// will be deprecated

/**
 * @}
 */


/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_VOLUME_MODULE
 * @{
 */

/**
 * @brief Called when the system volume has changed.
 * @since_tizen 2.3
 * @param[in]   type	The sound type of the changed volume
 * @param[in]   volume	The new volume value
 * @param[in]   user_data	The user data passed from the callback registration function
 * @pre sound_manager_set_volume() will invoke this callback if you register it using sound_manager_set_volume_changed_cb().
 * @see sound_manager_set_volume_changed_cb()
 * @see sound_manager_unset_volume_changed_cb()
 */
typedef void (*sound_manager_volume_changed_cb)(sound_type_e type, unsigned int volume, void *user_data);

/**
 * @brief Gets the maximum volume level supported for a particular sound type.
 * @since_tizen 2.3
 * @param[in]		type The sound type
 * @param[out]	max	The maximum volume level
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_set_volume()
 * @see sound_manager_get_volume()
 */
int sound_manager_get_max_volume(sound_type_e type, int *max);

/**
 * @brief Sets the volume level specified for a particular sound type.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/volume.set
 * @param[in]		type The sound type
 * @param[in]		volume	The volume level to be set
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_PERMISSION_DENIED Permission denied
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_get_max_volume()
 * @see sound_manager_get_volume()
 */
int sound_manager_set_volume(sound_type_e type, int volume);

/**
 * @brief Gets the volume level specified for a particular sound type.
 * @since_tizen 2.3
 * @param[in]		type The sound type
 * @param[out]	volume	The current volume level
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_get_max_volume()
 * @see sound_manager_set_volume()
 */
int sound_manager_get_volume(sound_type_e type, int *volume);

/**
 * @brief Sets the type of the sound being currently played.
 * @since_tizen 2.3
 * @param[in]		type The sound type to set
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_get_current_sound_type()
 * @see sound_manager_unset_current_sound_type()
 */
int sound_manager_set_current_sound_type(sound_type_e type);

/**
 * @brief Gets the type of the sound being currently played.
 * @since_tizen 2.3
 * @param[out]	type The current sound type
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NO_PLAYING_SOUND No playing sound
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_set_current_sound_type()
 * @see sound_manager_unset_current_sound_type()
 */
int sound_manager_get_current_sound_type(sound_type_e *type);

/**
 * @brief Unsets the type of the sound being currently played.
 * @since_tizen 2.3
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_set_current_sound_type()
 * @see sound_manager_get_current_sound_type()
 */
int sound_manager_unset_current_sound_type(void);

/**
 * @brief Registers a callback function to be invoked when the volume level is changed.
 * @since_tizen 2.3
 * @param[in]	callback	Callback function to indicate change in volume
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @post  sound_manager_volume_changed_cb() will be invoked.
 * @see sound_manager_unset_volume_changed_cb()
 * @see sound_manager_volume_changed_cb()
 */
int sound_manager_set_volume_changed_cb(sound_manager_volume_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the volume change callback.
 * @since_tizen 2.3
 * @return 0 on success, otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_set_volume_changed_cb()
 */
int sound_manager_unset_volume_changed_cb(void);




/**
 * @brief Sets the volume key type
 * @param[in] type The volume key type to set
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 */
int sound_manager_set_volume_key_type(volume_key_type_e type) DEPRECATED;// will be deprecated

/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_SESSION_MODULE
 * @{
 */

/**
 * @brief Sets the application's sound session type.
 * @since_tizen 2.3
 * @param[in] type The session type to set
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NOT_SUPPORTED Not supported
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @retval #SOUND_MANAGER_ERROR_POLICY Noncompliance with the sound system policy
 * @see sound_manager_get_session_type()
 * @see sound_manager_set_media_session_option()
 * @see sound_manager_get_media_session_option()
 * @see sound_manager_set_media_session_resumption_option()
 * @see sound_manager_get_media_session_resumption_option()
 * @see sound_manager_set_voip_session_mode()
 * @see sound_manager_get_voip_session_mode()
 */
int sound_manager_set_session_type(sound_session_type_e type);

/**
 * @brief Gets the application's sound session type.
 * @since_tizen 2.3
 * @param[in] type The session type
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_set_session_type()
 * @see sound_manager_set_media_session_option()
 * @see sound_manager_get_media_session_option()
 * @see sound_manager_set_media_session_resumption_option()
 * @see sound_manager_get_media_session_resumption_option()
 * @see sound_manager_set_voip_session_mode()
 * @see sound_manager_get_voip_session_mode()
 */
int sound_manager_get_session_type(sound_session_type_e *type);

/**
 * @brief Sets the media sound session option.
 * @since_tizen 2.3
 * @param[in] s_option The session option for starting
 * @param[in] d_option The session option for during play
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SOUND_MANAGER_ERROR_POLICY Noncompliance with the sound system policy
 * @pre Call sound_manager_set_session_type(SOUND_SESSION_TYPE_MEDIA) before calling this function.
 * @see sound_manager_set_session_type()
 * @see sound_manager_get_session_type()
 * @see sound_manager_get_media_session_option()
 * @see sound_manager_set_media_session_resumption_option()
 * @see sound_manager_get_media_session_resumption_option()
 */
int sound_manager_set_media_session_option(sound_session_option_for_starting_e s_option, sound_session_option_for_during_play_e d_option);

/**
 * @brief Gets the media sound session option.
 * @since_tizen 2.3
 * @param[out] s_option The session option for starting
 * @param[out] d_option The session option for during play
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SOUND_MANAGER_ERROR_POLICY Noncompliance with the sound system policy
 * @see sound_manager_set_session_type()
 * @see sound_manager_get_session_type()
 * @see sound_manager_set_media_session_option()
 * @see sound_manager_set_media_session_resumption_option()
 * @see sound_manager_get_media_session_resumption_option()
 */
int sound_manager_get_media_session_option(sound_session_option_for_starting_e *s_option, sound_session_option_for_during_play_e *d_option);

/**
 * @brief Sets the media sound session resumption option.
 * @since_tizen 2.3
 * @param[in] option The session resumption option
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SOUND_MANAGER_ERROR_POLICY Noncompliance with the sound system policy
 * @pre Call sound_manager_set_session_type(SOUND_SESSION_TYPE_MEDIA) before calling this function.
 * @see sound_manager_set_session_type()
 * @see sound_manager_get_session_type()
 * @see sound_manager_set_media_session_option()
 * @see sound_manager_get_media_session_option()
 * @see sound_manager_get_media_session_resumption_option()
 */
int sound_manager_set_media_session_resumption_option(sound_session_option_for_resumption_e option);

/**
 * @brief Gets the media sound session resumption option.
 * @since_tizen 2.3
 * @param[out] option The session resumption option
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_OPERATION Invalid operation
 * @retval #SOUND_MANAGER_ERROR_POLICY Noncompliance with the sound system policy
 * @see sound_manager_set_session_type()
 * @see sound_manager_get_session_type()
 * @see sound_manager_set_media_session_option()
 * @see sound_manager_get_media_session_option()
 * @see sound_manager_set_media_session_resumption_option()
 */
int sound_manager_get_media_session_resumption_option(sound_session_option_for_resumption_e *option);

/**
 * @brief Sets the mode of the voip sound session.
 * @since_tizen 2.3
 * @param[in] mode The voip session mode
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NOT_SUPPORTED Not supported
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @retval #SOUND_MANAGER_ERROR_POLICY Noncompliance with the sound system policy
 * @pre Call sound_manager_set_session_type(SOUND_SESSION_TYPE_VOIP) before calling this function.
 * @see sound_manager_set_session_type()
 * @see sound_manager_get_session_type()
 * @see sound_manager_get_voip_session_mode()
*/
int sound_manager_set_voip_session_mode(sound_session_voip_mode_e mode);

/**
 * @brief Gets the mode of the voip sound session.
 * @since_tizen 2.3
 * @param[out] mode The voip session mode
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NOT_SUPPORTED Not supported
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @retval #SOUND_MANAGER_ERROR_POLICY Noncompliance with the sound system policy
 * @pre Call sound_manager_set_session_type(SOUND_SESSION_TYPE_VOIP) before calling this function.
 * @see sound_manager_set_session_type()
 * @see sound_manager_get_session_type()
 * @see sound_manager_set_voip_session_mode()
*/
int sound_manager_get_voip_session_mode(sound_session_voip_mode_e *mode);

/**
 * @brief Registers a callback function to be invoked when the sound session being played was interrupted.
 * @since_tizen 2.3
 * @param[in]	callback	The interrupted callback function
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_POLICY Noncompliance with the sound system policy
 * @post  sound_session_interrupted_cb() will be invoked.
 * @see sound_manager_unset_session_interrupted_cb()
 * @see sound_session_interrupted_cb()
 */
int sound_manager_set_session_interrupted_cb(sound_session_interrupted_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function which is called when the sound session being played is interrupted.
 * @since_tizen 2.3
 * @return 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_set_session_interrupted_cb()
 */
int sound_manager_unset_session_interrupted_cb(void);

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
int sound_manager_set_session_notify_cb(sound_session_notify_cb callback, void *user_data) DEPRECATED;// will be deprecated

/**
 * @brief Unregisters the callback function which is called when the session notification is occured
 * @see sound_manager_set_session_notify_cb()
 */
void sound_manager_unset_session_notify_cb(void) DEPRECATED;// will be deprecated;

/**
 * @brief Registers a callback function to be invoked when the playing sound was interrupted.
 * @param[in]	callback	The interrupted callback function
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  sound_interrupted_cb() will be invoked
 * @see sound_manager_unset_interrupted_cb()
 * @see sound_interrupted_cb()
 */
int sound_manager_set_interrupted_cb(sound_interrupted_cb callback, void *user_data) DEPRECATED;// will be deprecated;

/**
 * @brief Unregisters the callback function which is called when the playing sound was interrupted
 * @see sound_manager_set_interrupted_cb()
 */
void sound_manager_unset_interrupted_cb(void) DEPRECATED;// will be deprecated;

/**
 * @}
 */

/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_DEVICE_MODULE
 * @{
 */

/**
 * @brief Gets the list consisting of connected devices.
 * @since_tizen 2.3
 * @param[in]	device_mask	The mask value
 * @param[out]	device_list	The list of connected devices
 *
 * @remarks	@a Use sound_manager_get_next_device() to get the first node of the list.
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NO_DATA No data
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_get_next_device()
 * @see sound_manager_get_prev_device()
 * @see sound_manager_get_device_type()
 * @see sound_manager_get_device_io_direction()
 * @see sound_manager_get_device_id()
 * @see sound_manager_get_device_name()
 * @see sound_manager_get_device_state()
 */
int sound_manager_get_current_device_list(sound_device_mask_e device_mask, sound_device_list_h *device_list);

/**
 * @brief Gets the next item of the device list.
 * @since_tizen 2.3
 * @param[in]	device_list	The list of connected devices
 * @param[out]	device	The device item
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NO_DATA No data
 * @see sound_manager_get_current_device_list()
 * @see sound_manager_get_prev_device()
 * @see sound_manager_get_device_type()
 * @see sound_manager_get_device_io_direction()
 * @see sound_manager_get_device_id()
 * @see sound_manager_get_device_name()
 * @see sound_manager_get_device_state()
 */
int sound_manager_get_next_device (sound_device_list_h device_list, sound_device_h *device);

/**
 * @brief Gets the previous item of the device list.
 * @since_tizen 2.3
 * @param[in]	device_list	The list of connected devices
 * @param[out]	device	The device item
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_NO_DATA No data
 * @see sound_manager_get_current_device_list()
 * @see sound_manager_get_next_device()
 * @see sound_manager_get_device_type()
 * @see sound_manager_get_device_io_direction()
 * @see sound_manager_get_device_id()
 * @see sound_manager_get_device_name()
 * @see sound_manager_get_device_state()
 */
int sound_manager_get_prev_device (sound_device_list_h device_list, sound_device_h *device);

/**
 * @brief Gets the type of the device.
 * @since_tizen 2.3
 * @param[in]	device	The device item
 * @param[out]	type	The type of the device
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_current_device_list()
 * @see sound_manager_get_next_device()
 * @see sound_manager_get_prev_device()
 * @see sound_manager_get_device_io_direction()
 * @see sound_manager_get_device_id()
 * @see sound_manager_get_device_name()
 * @see sound_manager_get_device_state()
 */
int sound_manager_get_device_type (sound_device_h device, sound_device_type_e *type);

/**
 * @brief Gets the io direction of the device.
 * @since_tizen 2.3
 * @param[in]	device	The device item
 * @param[out]	io_direction	The io direction of the device
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_current_device_list()
 * @see sound_manager_get_next_device()
 * @see sound_manager_get_prev_device()
 * @see sound_manager_get_device_type()
 * @see sound_manager_get_device_id()
 * @see sound_manager_get_device_name()
 * @see sound_manager_get_device_state()
 */
int sound_manager_get_device_io_direction (sound_device_h device, sound_device_io_direction_e *io_direction);

/**
 * @brief Gets the id of the device.
 * @since_tizen 2.3
 * @param[in]	device	The device item
 * @param[out]	id	The id of the device
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_current_device_list()
 * @see sound_manager_get_next_device()
 * @see sound_manager_get_prev_device()
 * @see sound_manager_get_device_type()
 * @see sound_manager_get_device_io_direction()
 * @see sound_manager_get_device_name()
 * @see sound_manager_get_device_state()
 */
int sound_manager_get_device_id (sound_device_h device, int *id);

/**
 * @brief Gets the name of the device.
 * @since_tizen 2.3
 * @param[in]	device	The device item
 * @param[out]	name	The name of the device
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_get_current_device_list()
 * @see sound_manager_get_next_device()
 * @see sound_manager_get_prev_device()
 * @see sound_manager_get_device_type()
 * @see sound_manager_get_device_io_direction()
 * @see sound_manager_get_device_id()
 * @see sound_manager_get_device_state()
 */
int sound_manager_get_device_name (sound_device_h device, char **name);

/**
 * @brief Gets the state of the device.
 * @since_tizen 2.3
 * @param[in]	device	The device item
 * @param[out]	state	The state of the device
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_current_device_list()
 * @see sound_manager_get_next_device()
 * @see sound_manager_get_prev_device()
 * @see sound_manager_get_device_type()
 * @see sound_manager_get_device_io_direction()
 * @see sound_manager_get_device_id()
 * @see sound_manager_get_device_name()
 */
int sound_manager_get_device_state (sound_device_h device, sound_device_state_e *state);

/**
 * @brief Registers a callback function to be invoked when the state of connection of a sound device was changed.
 * @since_tizen 2.3
 * @param[in]	device_mask	The mask value
 * @param[in]	callback	The interrupted callback function
 * @param[in]	user_data	The user data to be passed to the callback function
 *
 * @remarks	@a The initial state of sound devices connected is deactivated.
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @post  sound_device_connected_cb() will be invoked.
 * @see sound_manager_unset_device_connected_cb()
 * @see sound_device_connected_cb()
 */
int sound_manager_set_device_connected_cb (sound_device_mask_e device_mask, sound_device_connected_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function which is called when the state of connection of a sound device was changed.
 * @since_tizen 2.3
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_set_device_connected_cb()
 */
int sound_manager_unset_device_connected_cb (void);

/**
 * @brief Registers a callback function to be invoked when the information of a sound device was changed.
 * @since_tizen 2.3
 * @param[in]	device_mask	The mask value
 * @param[in]	callback	The interrupted callback function
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @post  sound_device_information_changed_cb() will be invoked.
 * @see sound_manager_unset_device_information_changed_cb()
 * @see sound_device_information_changed_cb()
 */
int sound_manager_set_device_information_changed_cb (sound_device_mask_e device_mask, sound_device_information_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function which is called when the information of a sound device was changed.
 * @since_tizen 2.3
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_set_device_information_changed_cb()
 */
int sound_manager_unset_device_information_changed_cb (void);



/**
 * @brief Gets called iteratively to notify you of available route.
 * @param[in]   route The available route
 * @param[in]   user_data The user data passed from the foreach function
 * @return @c true to continue with the next iteration of the loop, \n @c false to break out of the loop
 * @pre  sound_manager_foreach_available_route() will invoke this callback.
 * @see sound_manager_foreach_available_route()
 */
typedef bool(* sound_available_route_cb)(sound_route_e route, void *user_data) DEPRECATED;// will be deprecated

/**
 * @brief Called when the available audio route is changed.
 * @param[in]   route The audio route
 * @param[in]   available The status of given route
 * @param[in]   user_data The user data passed from the foreach function
 * @pre  sound_manager_foreach_available_route() will invoke this callback.
 * @see sound_manager_foreach_available_route()
 */
typedef void(* sound_available_route_changed_cb)(sound_route_e route, bool available, void *user_data) DEPRECATED;// will be deprecated

/**
 * @brief Called when the audio route is changed.
 * @param[in]   route The audio route
 * @param[in]   user_data The user data passed from the callback registration function
 * @pre  You should register this callback by sound_manager_set_active_device_changed_cb()
 * @see sound_manager_set_active_device_changed_cb()
 */
typedef void(* sound_active_device_changed_cb)(sound_device_in_e in, sound_device_out_e out, void *user_data) DEPRECATED;// will be deprecated

/**
 * @brief Retrieves all available audio routes by invoking a specific callback for each valid route.
 * @param[in]	callback	The session notify callback function
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  sound_available_route_cb() will be invoked
 * @see sound_available_route_cb()
 */
int sound_manager_foreach_available_route (sound_available_route_cb callback, void *user_data) DEPRECATED;// will be deprecated

/**
 * @brief Changes the audio routes.
 * @param[in] route The route to set
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_get_active_device()
 */
int sound_manager_set_active_route (sound_route_e route) DEPRECATED;// will be deprecated

/**
 * @brief Changes the audio route.
 * @param[out] in The current sound input device
 * @param[out] in The current sound output device
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_set_active_route()
 */
int sound_manager_get_active_device (sound_device_in_e *in, sound_device_out_e *out) DEPRECATED;// will be deprecated

/**
 * @brief Check if given audio route is available or not.
 * @param[in] route The route to set
 * @return 0 on success, otherwise a negative error value.
 * @return @c true if the specified route is supported, \n else @c false
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 */
bool sound_manager_is_route_available (sound_route_e route) DEPRECATED;// will be deprecated

/**
 * @brief Registers a callback function to be invoked when the available status is changed.
 * @param[in]	callback	The available status changed callback function
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  sound_available_route_changed_cb() will be invoked
 * @see sound_manager_unset_available_route_changed_cb()
 * @see sound_available_route_changed_cb()
 */
int sound_manager_set_available_route_changed_cb (sound_available_route_changed_cb callback, void *user_data) DEPRECATED;// will be deprecated

/**
 * @brief Unregisters the callback function.
 * @see sound_manager_set_available_route_changed_cb()
 */
void sound_manager_unset_available_route_changed_cb (void) DEPRECATED;// will be deprecated

/**
 * @brief Registers a callback function to be invoked when the audio device is changed.
 * @param[in]	callback	The session notify callback function
 * @param[in]	user_data	The user data to be passed to the callback function
 * @return 0 on success, otherwise a negative error value.
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @post  sound_active_device_changed_cb() will be invoked
 * @see sound_manager_unset_active_device_changed_cb()
 * @see sound_active_device_changed_cb()
 */
int sound_manager_set_active_device_changed_cb (sound_active_device_changed_cb callback, void *user_data);

/**
 * @brief Unregisters the callback function which is called when the route notification is occured.
 * @see sound_manager_set_active_device_changed_cb()
 */
void sound_manager_unset_active_device_changed_cb (void) DEPRECATED;// will be deprecated


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
int sound_manager_get_a2dp_status(bool *connected, char **bt_name) DEPRECATED;// will be deprecated

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_SOUND_MANAGER_H__ */
