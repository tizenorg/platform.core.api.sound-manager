/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */


#ifndef __TIZEN_MEDIA_SOUND_MANAGER_DOC_H__
#define __TIZEN_MEDIA_SOUND_MANAGER_DOC_H__


/**
 * @file sound_manager_doc.h
 * @brief This file contains high level documentation of the Sound Manager API.
 */

/**
 * @ingroup CAPI_MEDIA_FRAMEWORK
 * @defgroup CAPI_MEDIA_SOUND_MANAGER_MODULE Sound Manager
 * @brief  The @ref CAPI_MEDIA_SOUND_MANAGER_MODULE  API provides functions to get and set sound parameters like volume, sound stream, session policy and sound devices.
 *
 * @section CAPI_MEDIA_SOUND_MANAGER_MODULE_HEADER Required Header
 *   \#include <sound_manager.h>
 *
 * @section CAPI_MEDIA_SOUND_MANAGER_OVERVIEW Overview
 * The Sound Manager service allows APIs to manage audio output.
 *
 * The Sound Manager API allows you to:
 * - check/control output volumes
 * - handle a volume changed notification
 * - determine/control a sound stream policy
 * - handle the notification of a sound focus status change
 * - determine a sound session policy (Deprecated)
 * - handle the notification of a sound session interruption (Deprecated)
 * - query the basic information of connected sound devices
 * - handle the notification for connection of a sound device, and for changed information of a sound device
 *
 * @subsection CAPI_MEDIA_SOUND_MANAGER_LIFE_CYCLE_ASYNCHRONOUS_OPERATIONS Asynchronous operations
 * The Sound Manager API allows only asynchronous operations. \n
 * Thus the result will be passed to the application via the callback mechanism.
 * An appropriate callback can be called in response to changes initiated from outside the application. \n
 * Possible callbacks are listed below:
 * <p>
 * <ul>
 *    <li> sound_manager_volume_changed_cb() - invoked when volume level is changed.<p></li>
 *    <li> sound_stream_focus_state_changed_cb() - invoked when the state of focus that belongs to the stream_info is changed.<p></li>
 *    <li> sound_stream_focus_state_watch_cb() - invoked when the focus state for each sound stream type is changed.<p></li>
 *    <li> sound_session_interrupted_cb() - invoked when the session is interrupted.<p></li>
 *    <li> sound_device_connected_cb() - invoked when the connection of a sound device is changed.<p></li>
 *    <li> sound_device_information_changed_cb() - invoked when the information of a sound device is changed.<p></li>
 * </ul>
 *
 * @subsection CAPI_MEDIA_SOUND_MANAGER_MODULE_LIFE_CYCLE_CALLBACK_OPERATIONS  Callback(Event) Operations
 * The callback mechanism is used to notify the application about significant sound manager events.
 * <div><table class="doxtable" >
 *     <tr>
 *        <th><b> REGISTER</b></th>
 *        <th><b> UNREGISTER</b></th> 
 *        <th><b> CALLBACK</b></th>
 *        <th><b> DESCRIPTION</b></th>
 *     </tr>
 *     <tr>
 *        <td> sound_manager_set_volume_changed_cb()</td>
 *        <td> sound_manager_unset_volume_changed_cb()</td> 
 *        <td> sound_manager_volume_changed_cb()</td>
 *        <td> This callback is called when volume value is changed.</td>
 *     </tr>
 *     <tr>
 *        <td> sound_manager_create_stream_information()</td>
 *        <td> sound_manager_destroy_stream_information()</td>
 *        <td> sound_stream_focus_state_changed_cb()</td>
 *        <td> This callback is called when the state of focus that belongs to the stream_info is changed.</td>
 *     </tr>
 *     <tr>
 *        <td> sound_manager_set_focus_state_watch_cb()</td>
 *        <td> sound_manager_unset_foucs_state_watch_cb()</td>
 *        <td> sound_stream_focus_state_watch_cb()</td>
 *        <td> This callback is called when the focus state for each sound stream type is changed.</td>
 *     </tr>
 *     <tr>
 *        <td> sound_manager_set_session_interrupted_cb()</td>
 *        <td> sound_manager_unset_session_interrupted_cb()</td>
 *        <td> sound_session_interrupted_cb()</td>
 *        <td> This callback is called when audio session is interrupted.</td>
 *     </tr>
 *     <tr>
 *        <td> sound_manager_set_device_connected_cb()</td>
 *        <td> sound_manager_unset_device_connected_cb()</td>
 *        <td> sound_device_connected_cb()</td>
 *        <td> This callback is called when the connection of a sound device is changed.</td>
 *     </tr>
 *     <tr>
 *        <td> sound_manager_set_device_information_changed_cb()</td>
 *        <td> sound_manager_unset_device_information_changed_cb()</td>
 *        <td> sound_device_information_changed_cb()</td>
 *        <td> This callback is called when the information of a sound device is changed.</td>
 *     </tr>
 * </table></div>
 *
*/

/**
 * @ingroup CAPI_MEDIA_SOUND_MANAGER_MODULE
 * @defgroup CAPI_MEDIA_SOUND_MANAGER_VOLUME_MODULE Volume
 * @brief The @ref CAPI_MEDIA_SOUND_MANAGER_VOLUME_MODULE API provides functions to check and control volumes.
 * @section CAPI_MEDIA_SOUND_MANAGER_VOLUME_MODULE_HEADER Required Header
 *    \#include <sound_manager.h>
 *
 * @section CAPI_MEDIA_SOUND_MANAGER_VOLUME_MODULE_OVERVIEW Overview
 * The Sound Manager Volume API allows you to:
 * - check/control output volumes
 * - handle a volume changed notification
 *
 * The Sound Manager has predefined types of sounds.(system, notification, alarm, ringtone, media, call, voip, voice).
 *
 * Current volume level of each type can be checked with sound_manager_get_volume().
 * To get the maximum supported volume level of each type, call sound_manager_get_max_volume().
 * The volume level of each type can be adjusted with sound_manager_set_volume().
 *
 * The type of currently playing sound may be obtained by calling sound_manager_get_current_sound_type().
 * To set the type of the currently playing sound forcibly, call sound_manager_set_current_sound_type().
 *
 *
*/

/**
 * @ingroup CAPI_MEDIA_SOUND_MANAGER_MODULE
 * @defgroup CAPI_MEDIA_SOUND_MANAGER_STREAM_MODULE Stream
 * @brief The @ref CAPI_MEDIA_SOUND_MANAGER_STREAM_MODULE API provides functions to control a sound stream.
 * @section CAPI_MEDIA_SOUND_MANAGER_STREAM_MODULE_HEADER Required Header
 *    \#include <sound_manager.h>
 *
 * @section CAPI_MEDIA_SOUND_MANAGER_STREAM_MODULE_OVERVIEW Overview
 * The Sound Manager Stream API allows you to:
 * - determine/ control a sound stream policy
 * - handle the notification of a sound focus status change
 *
 * The Sound Manager has predefined sound stream types (media, system, alarm, notification, emergency, voip, etc)
 *
 * @section CAPI_MEDIA_SOUND_MANAGER_MODULE_FEATURE Related Features
 * APIs for voip stream type are related with the following features:\n
 *  - http://tizen.org/feature/microphone\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="../org.tizen.mobile.native.appprogramming/html/ide_sdk_tools/feature_element.htm"><b>Feature Element</b>.</a>
 *
*/

/**
 * @ingroup CAPI_MEDIA_SOUND_MANAGER_MODULE
 * @defgroup CAPI_MEDIA_SOUND_MANAGER_SESSION_MODULE Session
 * @brief The @ref CAPI_MEDIA_SOUND_MANAGER_SESSION_MODULE API provides functions to control a session.
 * @section CAPI_MEDIA_SOUND_MANAGER_SESSION_MODULE_HEADER Required Header
 *    \#include <sound_manager.h>
 *
 * @section CAPI_MEDIA_SOUND_MANAGER_SESSION_MODULE_OVERVIEW Overview
 * The Sound Manager Session API allows you to:
 * - determine a sound session policy
 * - handle the notification of a sound session interruption
 *
 * The Sound Manager has predefined sound sessions (media, alarm, notification, emergency, voip)
 *
 * @section CAPI_MEDIA_SOUND_MANAGER_MODULE_FEATURE Related Features
 * APIs for voip sound session and voip stream type are related with the following features:\n
 *  - http://tizen.org/feature/microphone\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="../org.tizen.mobile.native.appprogramming/html/ide_sdk_tools/feature_element.htm"><b>Feature Element</b>.</a>
 *
*/

/**
 * @ingroup CAPI_MEDIA_SOUND_MANAGER_MODULE
 * @defgroup CAPI_MEDIA_SOUND_MANAGER_DEVICE_MODULE Device
 * @brief The @ref CAPI_MEDIA_SOUND_MANAGER_DEVICE_MODULE API provides functions to query the information of sound devices.
 * @section CAPI_MEDIA_SOUND_MANAGER_DEIVCE_MODULE_HEADER Required Header
 *    \#include <sound_manager.h>
 *
 * @section CAPI_MEDIA_SOUND_MANAGER_DEVICE_MODULE_OVERVIEW Overview
 * The Sound Manager Device API allows you to:
 * - query the basic information of connected sound devices.
 * - handle the sound device's connection and information change notification.
 *
 * The list of currently connected sound device can be obtained by calling sound_manager_get_current_device_list().
 * To get the handle of each sound device, call sound_manager_get_next_device() and sound_manager_get_prev_device().
 *
 * Device information, such as "type", "IO direction", "state", "ID", "name", can be obtained by calling corresponding APIs.
 *
 *
*/

#endif /* __TIZEN_MEDIA_SOUND_MANAGER_DOC_H__ */
