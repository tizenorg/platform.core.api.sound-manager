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
#include <sound_manager.h>
#include <glib.h>
#include <mm_sound.h>
#include <unistd.h>
#include <string.h>
#include <player.h>
#include <stdlib.h>

static GMainLoop *g_mainloop = NULL;
static GThread *event_thread;
#define MAX_VOLUME_TYPE 5

gpointer GmainThread(gpointer data){
	g_mainloop = g_main_loop_new (NULL, 0);
	g_main_loop_run (g_mainloop);

	return NULL;
}

void _sound_manager_volume_changed_cb(sound_type_e type, unsigned int volume, void* user_data)
{
	printf("changed!! type=%d, volume = %d\n", type, volume);
}

int set_volume_test()
{
	printf("set volume test\n");
	int i;
	sound_manager_set_volume_changed_cb(_sound_manager_volume_changed_cb ,NULL);
	for( i = SOUND_TYPE_SYSTEM ; i <=MAX_VOLUME_TYPE; i++){
		int max_value;
		int j;
		sound_manager_get_max_volume(i, &max_value);
		printf(" sound type = %d , max volume = %d\n", i, max_value);
		for( j = 0; j <= max_value+1 ; j++){
			int ret;
			int getvalue;
			ret = sound_manager_set_volume(i, j);
			if( j <=  max_value && ret == 0 ){
				sound_manager_get_volume(i, &getvalue);
				if( j == getvalue )
					printf("PASS\n");
				else
					printf("FAIL!set=%d, get=%d\n", j, getvalue);
			} else if( j > max_value && ret == 0 )
				printf("FAIL! max_value over!!\n");

		}
	}
	//sound_manager_unset_volume_changed_cb();
	printf("end set volume test!!!\n");
	sleep(10);
	return 0;
}

void _sound_manager_route_policy_changed_cb(sound_route_policy_e route , void* user_data)
{
	printf("new route policy %d\n", route);
}

int set_policy_test()
{
	printf("-----------set policy test--------------\n");
	int ret;
	//int i;
	//sound_route_policy_e value;
	sound_manager_set_route_policy_changed_cb(_sound_manager_route_policy_changed_cb,NULL);
	/*
	for( i = SOUND_ROUTE_DEFAULT ; i <= SOUND_ROUTE_HANDSET_ONLY ; i++){
		ret = sound_manager_set_route_policy(i);
		ret = sound_manager_get_route_policy(&value);
		if( i == value )
			printf("PASS\n");
		else
			printf("FAIL\n");
	}
	*/
	ret = sound_manager_set_route_policy(SOUND_ROUTE_DEFAULT);
	ret = sound_manager_set_route_policy(SOUND_ROUTE_DEFAULT);
	ret = sound_manager_set_route_policy(SOUND_ROUTE_DEFAULT);
	ret = sound_manager_set_route_policy(SOUND_ROUTE_DEFAULT);
	
	
	return 0;
}


static void mm_volume_changed_cb(void *user_data)
{
	int v = (int)user_data;
	unsigned int value = 0;
	int ret = mm_sound_volume_get_value(v, &value);
	printf("mm_volume_changed_cb type = %d ,volume = %d , ret = %x\n", v, value, ret );
}


int mm_bug_test()
{
	int ret = 0;
	mm_sound_volume_add_callback(0 , mm_volume_changed_cb , (void*)0);		
	mm_sound_volume_add_callback(1 , mm_volume_changed_cb ,(void*) 1);	
	mm_sound_volume_add_callback(2 , mm_volume_changed_cb , (void*)2);	
	mm_sound_volume_add_callback(5 , mm_volume_changed_cb , (void*)5);		

	int i;

	for( i = 0 ; i < 10 ; i++)
	{
		ret = mm_sound_volume_set_value(0, i);
		printf("type = 0 , volume = %d set , ret = %x\n", i, ret);
		ret = mm_sound_volume_set_value(1, i);	
		printf("type = 1 , volume = %d set , ret = %x\n", i, ret);		
		ret = mm_sound_volume_set_value(2, i);
		printf("type = 2 , volume = %d set , ret = %x\n", i, ret);		
		ret = mm_sound_volume_set_value(5, i);
		printf("type = 5 , volume = %d set , ret = %x\n", i, ret);		
		sleep(1);	
	}
	
	//mm_sound_volume_remove_callback(0);
	//mm_sound_volume_remove_callback(1);
	//mm_sound_volume_remove_callback(2);	
	//mm_sound_volume_remove_callback(5);	

	
	ret = mm_sound_volume_set_value(5, 4);	
	
	mm_sound_route_set_system_policy(SOUND_ROUTE_DEFAULT);
	mm_sound_route_set_system_policy(SOUND_ROUTE_DEFAULT);
	mm_sound_route_set_system_policy(SOUND_ROUTE_DEFAULT);	
	return 0;	
}

void mm_test(){

	char path[255];
	int id;
	int i;
	getcwd(path, 255);
	strcat(path, "/test2.wav");
	for( i =0 ; i < 10 ; i++){
		mm_sound_play_sound(path, SOUND_TYPE_MEDIA, NULL ,NULL, &id);
		mm_sound_stop_sound(id);
	}
}

void session_notify_cb(sound_session_notify_e notify, void *user_data)
{
	printf("notify %d\n", notify);


}


void session_test(){
	printf("session_test\n");
	int ret = 0;
	player_h player;

	ret = sound_manager_set_session_type(SOUND_SESSION_TYPE_EXCLUSIVE);
	sound_manager_set_session_notify_cb(session_notify_cb,NULL);	



	ret = player_create(&player);
	printf("player_create ret =%x\n", ret);
	ret = player_set_uri(player, "title_theme.mp3");
	printf("player_set_uri ret =%x\n", ret);
	ret =player_prepare(player);
	printf("player_prepare ret =%x\n", ret);		
	ret = player_start(player);
	printf("player_start ret =%x\n", ret);	

	while(1){
		player_state_e state;
		player_get_state(player, &state);
		printf("state %d\n", state);
		//if( state == 4)
			//player_start(player);
		sleep(1);
	 	//wav_player_start("test.wav", SOUND_TYPE_MEDIA,NULL, NULL,NULL);
	}
	
	
}


void a2dp_test(){
	bool isconnected;
	char * name;
	sound_manager_get_a2dp_status(&isconnected, &name);
	if( isconnected ){
		printf("name = %s\n", name);
		free(name);
	}else{
		printf("disconnected\n");
	}
	sound_device_e device;
	sound_manager_get_current_sound_device(&device);
	printf("device =%d\n", device);
		
}

int main()
{
	if( !g_thread_supported() )
	{
		g_thread_init(NULL);
	}

	GError *gerr = NULL;
	event_thread = g_thread_create(GmainThread, NULL, 1, &gerr);



	//set_volume_test();
	//set_policy_test();
	//mm_bug_test();
	//wav_play_test();
	//tone_play_test();
	//wav_play_test();
	session_test();
	//a2dp_test();
	return 0;
}
