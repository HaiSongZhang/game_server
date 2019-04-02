#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "model_internet_server.h"

#include "../game_player.h"
#include "../../database/center_database.h"
#include "../../database/game_database.h"
#include "../game_room.h"


// ������������ϡ�
int
model_enter_internet_server(struct internet_server* server,
                      unsigned int uid, struct session* s) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (!player) { // ���û�н������ǵķ�����
		player = cache_alloc(server->player_allocer, sizeof(struct game_player));
		player_init(player, uid, server->stype);
		
		// uid --> player
		set_hash_int_map(server->player_set, uid, player);
	}
	// ��ʾ�������Ѿ�����
	player->online = 1;
	player->s = s;
	// ���û����ĵ�redis�����ȡuinfo;
	player_load_uinfo(player);
	// end 

	// ����Ϸredis�����ȡ���ǵ�ͨ�õ���Ϸ��Ϣ
	player_load_ugame_info(player);
	// end 

	// ����Ϸredis����������ǵ�ս����Ϣ
	player_load_uscore_info(player);
	// end 

	server->online_num ++;

	return MODEL_INTERNET_SERVER_SUCCESS;
}

static void
add_to_wait_list(struct internet_server* server, struct game_player* player) {
	struct game_player** walk = &server->wait_list_set[player->zid - 1];
	while (*walk) {
		walk = &(*walk)->wait_next;
	}

	*walk = player;
}

static void
remove_from_wait_list(struct internet_server* server, struct game_player* player) {
	struct game_player** walk = &server->wait_list_set[player->zid - 1];
	while (*walk != player) {
		walk = &((*walk)->wait_next);
	}

	if (*walk) {
		*walk = player->wait_next;
		player->wait_next = NULL;
		player->zid = 0;
	}
}

int
model_exit_internet_server(struct internet_server* server, unsigned int uid, int is_online) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (player == NULL) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_SERVER;
	}

	if (is_online == 0) {
		player->online = 0; // ����Ѿ�����
		player->s = NULL;
	}
	// ���ӵ�״̬
	if (player->zid != 0 && player->roomid == 0) { // ������ڵȴ���״̬
		remove_from_wait_list(server, player);
	}
	else if (player->zid != 0 && player->roomid != 0) {// ���������Ϸ��״̬
		struct game_room* room = get_value_with_key(server->room_set, player->roomid);
		// int is_removed = player_try_exit_room(room, player);
		int is_removed;
		if (is_online == 0) { // �û����ȵ��� 
			is_removed = player_try_exit_room(room, player);
		}
		else { // �����˳������ǿ��
			is_removed = player_force_exit_room(room, player);
		}
		if (!is_removed) {
			return MODEL_INTERNET_SERVER_USER_IS_IN_GAME;
		}
	}
	// end 
	server->online_num --;
	remove_hash_int_key(server->player_set, uid);
	cache_free(server->player_allocer, player);


	return MODEL_INTERNET_SERVER_SUCCESS;
}


static int
_enter_server_zone(struct internet_server* server, struct game_player* player, int zid) {
	if (zid < NEWUSER_ZONE || zid > GREATMASTER_ZONE) {
		return MODEL_INTERNET_SERVER_INVALID_ZONE;
	}

	// ��Ϸ�������
	// end 

	// ����ȴ��б�
	player->zid = zid;
	add_to_wait_list(server, player);
	// end

	return MODEL_INTERNET_SERVER_SUCCESS;
}

int
model_auto_get_zoneid(struct internet_server* server, unsigned int uid) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (player == NULL) {
		return INVALIDI_ZONE;
	}

	// int zid = 1 + rand() % 3; // ���Զ��� // model_auto_get_zone_id;
	int zid = 3;
	return zid;
}

int
model_enter_internet_server_zone(struct internet_server* server, 
	unsigned int uid, int zid) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (player == NULL) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_SERVER;
	}

	// ������Ϸ��״̬,��������
	if (player->zid != INVALIDI_ZONE && player->roomid != 0) {
		return MODEL_INTERNET_SERVER_USER_RECONNECT;
	}
	// end 

	if (player->zid != INVALIDI_ZONE) {
		return MODEL_INTERNET_SERVER_USER_IS_IN_ZONE;
	}


	int ret = _enter_server_zone(server, player, zid);
	return ret;
}

int
model_exit_internet_server_zone(struct internet_server* server, unsigned int uid) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (player == NULL) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_SERVER;
	}

	if (player->zid == 0) { // ��������,���뵽�ȴ��б�
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_ZONE;
	}
	// �������Ҵӵȴ��б�����ɾ��,��һ�û�з��䷿���ʱ��
	if (player->roomid == 0) { // ��ʾ�������ڵȴ��б��С�
		remove_from_wait_list(server, player);
	}
	else if (player->roomid != 0) {// ���������Ϸ��״̬
		struct game_room* room = get_value_with_key(server->room_set, player->roomid);
		int is_removed = player_try_exit_room(room, player);
		if (!is_removed) {
			return MODEL_INTERNET_SERVER_USER_IS_IN_GAME;
		}
	}
	// end 

	return MODEL_INTERNET_SERVER_SUCCESS;
}

// �û���������
int
model_player_reconnect_to_room(struct internet_server* server, unsigned int uid) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (player == NULL) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_SERVER;
	}

	if (player->zid == INVALIDI_ZONE || player->roomid == 0) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_ROOM;
	}


	struct game_room* room = get_value_with_key(server->room_set, player->roomid);
	if (room == NULL) {
		return MODEL_INTERNET_SERVER_ROOM_IS_NOT_EXIST;
	}

	player_reconnect_room(room, player);
	return MODEL_INTERNET_SERVER_SUCCESS;
}

int
model_slice_fruit(struct internet_server* server, unsigned int uid, int fruit_id, int degree) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (player == NULL) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_SERVER;
	}

	if (player->zid == INVALIDI_ZONE || player->roomid == 0) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_ROOM;
	}

	struct game_room* room = get_value_with_key(server->room_set, player->roomid);
	if (room == NULL) {
		return MODEL_INTERNET_SERVER_ROOM_IS_NOT_EXIST;
	}

	int ret = player_slice_fruit(room, player, fruit_id, degree);

	return ret;
}

int
model_player_set_ready_in_room(struct internet_server* server, unsigned int uid) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (player == NULL) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_SERVER;
	}

	if (player->zid == INVALIDI_ZONE || player->roomid == 0) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_ROOM;
	}

	struct game_room* room = get_value_with_key(server->room_set, player->roomid);
	if (room == NULL) {
		return MODEL_INTERNET_SERVER_ROOM_IS_NOT_EXIST;
	}

	int ret = player_set_ready_in_room(room, player);
	return ret;
}

int
model_player_send_prop_in_room(struct internet_server* server, 
                               unsigned int uid, int to_seatid, int prop_id) {
	struct game_player* player = get_value_with_key(server->player_set, uid);
	if (player == NULL) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_SERVER;
	}

	if (player->zid == INVALIDI_ZONE || player->roomid == 0) {
		return MODEL_INTERNET_SERVER_USER_IS_NOT_IN_ROOM;
	}

	struct game_room* room = get_value_with_key(server->room_set, player->roomid);
	if (room == NULL) {
		return MODEL_INTERNET_SERVER_ROOM_IS_NOT_EXIST;
	}

	int ret = player_send_prop_in_room(room, player, to_seatid, prop_id);
	return ret;
}