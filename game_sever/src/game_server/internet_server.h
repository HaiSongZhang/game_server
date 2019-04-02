#ifndef __INTERNET_SERVER_H__
#define __INTERNET_SERVER_H__

#include "../utils/hash_int_map.h"
#include "../utils/cache_allocer.h"

#include "game_player.h"
#include "game_room.h"

#include "../netbus/netbus.h"

enum {
	INVALIDI_ZONE = 0,
	NEWUSER_ZONE = 1,
	MASTER_ZONE = 2,
	GREATMASTER_ZONE = 3,
};

#define ZONE_NUM 3

struct internet_server {
	// ����������Ϣ
	int stype;
	char* desic;
	int online_num;
	int room_num;
	// end 

	// uid --> player�����ӳ��
	struct hash_int_map* player_set;
	// end 


	// roomid --> room����ӳ���
	struct hash_int_map* room_set;
	// end 

	// ���ֳ�, ���ֳ�, ��ʦ��
	struct game_player* wait_list_set[ZONE_NUM];

	// ��ҵ��ڴ����������cache alloc
	struct cache_alloc* player_allocer;
	// end 

	// ��Ϸ������ڴ������������cache alloc
	struct cache_alloc* room_allocer;
	int auto_inc_roomid;
	// end 

	// ˮ�����ڴ������
	struct cache_alloc* fruit_allocer;
	// end
};

struct internet_server*
create_ineternet_server();

void
enter_internet_server(void* module_data, json_t* json,
             struct session* s,
             unsigned int uid, unsigned int s_key);

void
exit_internet_server(void* module_data, json_t* json,
             struct session* s,
             unsigned int uid, unsigned int s_key);

void
enter_internet_server_zone(void* module_data, json_t* json,
             struct session* s,
             unsigned int uid, unsigned int s_key);

void
exit_internet_server_zone(void* module_data, json_t* json,
			 struct session* s,
             unsigned int uid, unsigned int s_key);

void
send_user_enter_room(struct game_room* room, struct game_player* player);


void
send_user_arrive_room(struct game_room* room, struct game_player* player, struct game_player* to_player);


json_t*
pack_player_user_arrive_cmd(struct game_player* player);

void
send_user_standup_room(struct game_room* room, struct game_player* to_player, int sv_seatid);

void
send_user_reconnect_room(struct game_room* room, struct game_player* to_player);

void
slice_fruit_internet_server(void* module_data, json_t* json,
                            struct session* s,
							unsigned int uid, unsigned int s_key);
void 
player_send_ready_in_room(void* module_data, json_t* json,
                          struct session* s,
						  unsigned int uid, unsigned int s_key);

void
send_prop_in_room(void* module_data, json_t* json,
                  struct session* s,
                  unsigned int uid, unsigned int s_key);
#endif

