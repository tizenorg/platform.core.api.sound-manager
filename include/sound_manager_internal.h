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

#ifndef __TIZEN_MEDIA_SOUND_MANAGER_INTERNAL_H__
#define __TIZEN_MEDIA_SOUND_MANAGER_INTERNAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file sound_manager_internal.h
 * @brief This file contains the Sound Manager Internal API.
 */

/**
 * @addtogroup CAPI_MEDIA_SOUND_MANAGER_MODULE
 * @{
 */

#define SOUND_TYPE_NUM                SOUND_TYPE_VOICE + 1

/**
 * @internal
 * @brief Virtual sound stream handle.
 * @since_tizen 3.0
 */
typedef struct virtual_sound_stream_s* virtual_sound_stream_h;

/**
 * @internal
 * @brief Enumeration for Native API.
 * @since_tizen 3.0
 */
typedef enum {
	NATIVE_API_SOUND_MANAGER,   /**< Sound-manager Native API */
	NATIVE_API_PLAYER,          /**< Player Native API */
	NATIVE_API_WAV_PLAYER,      /**< Wav-player Native API */
	NATIVE_API_TONE_PLAYER,     /**< Tone-player Native API */
	NATIVE_API_AUDIO_IO,        /**< Audio-io Native API */
} native_api_e;

/**
 * @internal
 * @brief Enumeration for sound stream type for internal.
 * @since_tizen 3.0
 */
typedef enum {
	SOUND_STREAM_TYPE_RINGTONE_CALL = 100, /**< Sound stream type for ringtone for call */
	SOUND_STREAM_TYPE_VOICE_CALL,          /**< Sound stream type for voice-call */
	SOUND_STREAM_TYPE_VIDEO_CALL,          /**< Sound stream type for video-call */
	SOUND_STREAM_TYPE_RADIO,               /**< Sound stream type for radio */
	SOUND_STREAM_TYPE_LOOPBACK,            /**< Sound stream type for loopback */
} sound_stream_type_internal_e;

/**
 * @internal
 * @brief Creates a handle for stream information.
 * @since_tizen 3.0
 * @details	To apply the stream policy according to this stream information, this handle should be passed to other APIs\n
 * 	related to playback or recording. (e.g., player, wav-player, audio-io, etc.)
 * @param[in]	stream_type	The type of stream for internal usage
 * @param[in]	callback	The focus state change callback function (mandatory)
 * @param[in]	user_data	The user data to be passed to the callback function
 * @param[out]	stream_info	The handle of stream information
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_destroy_stream_information()
 * @see sound_manager_add_device_for_stream_routing()
 * @see sound_manager_remove_device_for_stream_routing()
 * @see sound_manager_apply_stream_routing()
 * @see sound_manager_acquire_focus()
 * @see sound_manager_destroy_focus()
 * @see sound_manager_get_focus_state()
 */
int sound_manager_create_stream_information_internal (sound_stream_type_internal_e stream_type, sound_stream_focus_state_changed_cb callback, void *user_data, sound_stream_info_h *stream_info);

/**
 * @internal
 * @brief Sets the stream routing option.
 * @since_tizen 3.0
 * @param[in]	stream_info	The handle of stream information
 * @param[in]	name	The name of option
 * @param[in]	value	The value of option
 *
 * @remarks	@a If the stream has not been made yet, this setting will be applied when the stream starts to play.\n
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_STATE Invalid state
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @pre Call sound_manager_create_stream_information() before calling this function.
 * @see sound_manager_create_stream_information()
 * @see sound_manager_create_stream_information_internal()
 * @see sound_manager_destroy_stream_information()
 */
int sound_manager_set_stream_routing_option (sound_stream_info_h stream_info, const char *name, int value);

/**
 * @internal
 * @brief Queries if this stream information handle is available for the API.
 * @since_tizen 3.0
 * @param[in]	stream_info	The handle of stream information
 * @param[in]	api_name	The native API name
 * @param[out]	is_available	If @c true the api_name is available, @c false the api_name is not available for this strema_info
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_create_stream_information()
 * @see sound_manager_destroy_stream_information()
 */
int sound_manager_is_available_stream_information (sound_stream_info_h stream_info, native_api_e api_name, bool *is_available);

/**
 * @internal
 * @brief Gets stream type from the stream information handle.
 * @since_tizen 3.0
 * @param[in]	stream_info	The handle of stream information
 * @param[out]	type	The stream type
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_create_stream_information()
 * @see sound_manager_destroy_stream_information()
 */
int sound_manager_get_type_from_stream_information (sound_stream_info_h stream_info, char **type);

/**
 * @internal
 * @brief Gets the index of the stream information handle.
 * @since_tizen 3.0
 * @param[in]	stream_info	The handle of stream information
 * @param[out]	index	The unique index
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @see sound_manager_create_stream_information()
 * @see sound_manager_destroy_stream_information()
 */
int sound_manager_get_index_from_stream_information (sound_stream_info_h stream_info, int *index);

/**
 * @internal
 * @brief Creates a virtual stream handle.
 * @since_tizen 3.0
 * @param[in]	stream_info	The handle of stream information
 * @param[out]	virtual_stream	The handle of virtual stream
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @retval #SOUND_MANAGER_ERROR_NOT_SUPPORTED Not supported
 * @see sound_manager_create_stream_information()
 * @see sound_manager_destroy_stream_information()
 * @see sound_manager_destroy_virtual_stream()
 * @see sound_manager_start_virtual_stream()
 * @see sound_manager_stop_virtual_stream()
 */
int sound_manager_create_virtual_stream (sound_stream_info_h stream_info, virtual_sound_stream_h *virtual_stream);

/**
 * @internal
 * @brief Destroys the virtual stream handle.
 * @since_tizen 3.0
 * @param[in]	virtual_stream	The handle of virtual stream
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_STATE Invalid state
 * @see sound_manager_create_virtual_stream()
 * @see sound_manager_start_virtual_stream()
 * @see sound_manager_stop_virtual_stream()
 */
int sound_manager_destroy_virtual_stream (virtual_sound_stream_h virtual_stream);

/**
 * @internal
 * @brief Starts the virtual stream.
 * @since_tizen 3.0
 * @param[in]	virtual_stream	The handle of virtual stream
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_STATE Invalid state
 * @retval #SOUND_MANAGER_ERROR_INTERNAL Internal error inside the sound system
 * @see sound_manager_create_virtual_stream()
 * @see sound_manager_destroy_virtual_stream()
 * @see sound_manager_stop_virtual_stream()
 */
int sound_manager_start_virtual_stream (virtual_sound_stream_h virtual_stream);

/**
 * @internal
 * @brief Stops the virtual stream.
 * @since_tizen 3.0
 * @param[in]	virtual_stream	The handle of virtual stream
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 * @retval #SOUND_MANAGER_ERROR_NONE Success
 * @retval #SOUND_MANAGER_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #SOUND_MANAGER_ERROR_INVALID_STATE Invalid state
 * @see sound_manager_create_virtual_stream()
 * @see sound_manager_destroy_virtual_stream()
 * @see sound_manager_start_virtual_stream()
 */
int sound_manager_stop_virtual_stream (virtual_sound_stream_h virtual_stream);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __TIZEN_MEDIA_SOUND_MANAGER_INTERNAL_H__ */
