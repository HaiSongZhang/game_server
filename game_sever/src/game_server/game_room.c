#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../netbus/netbus.h"
#include "../game_stype.h"
#include "../game_command.h"
#include "../game_result.h"
#include "../utils/timestamp.h"
#include "../utils/random.h"
#include "../utils/cache_allocer.h"
#include "../utils/vector_array.h"

#include "models/model_internet_server.h"

#include "internet_server.h"

#include "gserver_config.h"
#include "game_player.h"

#include "game_room.h"

#define PLAYER_SEAT 2


enum {
	APPLE = 0,
	BANANA = 1,
	BASAHA = 2,
	PEACH = 3,
	SANDIA = 4,
	FRUIT_NUM,
};

struct game_fruit {
	unsigned int id; // ˮ����Ӧ��Ψһid��

	int f_type; // ˮ��������
	int G; // �������ٶ�
	int speed; // �����ٶ�
	int degree; // ����Ƕ�

	int xpos;
	int ypos;

	int status; // 0�� ����ˮ��û���е��������1����ʾˮ���Ѿ����е��ˡ�
	int slice_seatid; // ���ĸ�����е����ˮ��
};

struct game_seat {
	struct game_player* player;
	int is_sitdown;
	int seatid;

	int score;
};

struct game_room {
	// ���ӵ�����
	struct internet_server* server; // internet_server ��ս��ָ��

	int zid;  // �����ĸ���
	int roomid; // �����ĸ�����
	int game_time; // ��Ϸ������
	int checkout_time; // ��Ϸ�Ľ���ʱ��
	unsigned int checkout_timer;
	unsigned int throw_fruit_timer; // ��Ϸ��ˮ����ʱ��
	unsigned int auto_fruit_id; // ֻ���Ӳ����� 
	// end 
	
	
	

	// ��ҵ���λ
	struct game_seat player_seat[PLAYER_SEAT];
	int room_status; // �����״̬
	unsigned int start_timestamp; // ��Ϸ��ʼ��ʱ���
	// end 

	// ��Ϸ���ݵ�ά��
	struct vector_array fruit_array;
	// end
};

#define DEGREE_AREA 15
#define XPOS_AREA 100

static struct game_fruit*
gen_fruit(struct game_room* room) {
	struct game_fruit* fruit = cache_alloc(room->server->fruit_allocer, sizeof(struct game_fruit));
	fruit->f_type = random_int(APPLE, FRUIT_NUM);
	fruit->G = -1000;
	fruit->degree = random_int(90 - DEGREE_AREA, 90 + DEGREE_AREA + 1);
	fruit->speed = 1000 + random_int(0, 100);

	fruit->status = 0;
	fruit->ypos = 0; // ͳһ��������
	fruit->xpos = random_int(-XPOS_AREA, XPOS_AREA + 1);
	fruit->id = room->auto_fruit_id;
	room->auto_fruit_id++;

	return fruit;
}

static void
check_game_start(struct game_room* room);

static void
broadcast_json_cmd_in_room(struct game_room* room, json_t* json_pkg) {
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0) {
			continue;
		}

		struct game_player* to_player = room->player_seat[i].player;
		if (to_player->online == 0 || to_player->s == NULL) {
			continue;
		}

		json_object_update_number(json_pkg, "uid", to_player->uid);
		session_send_json(to_player->s, json_pkg);
	}
}

static 
int get_player_num(struct game_room* room) {
	int num = 0;
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown) {
			num++;
		}
	}

	return num;
}

struct cache_alloc*
create_room_allocer(int room_num) {
	struct cache_alloc* allocer = create_cache_alloc(room_num, sizeof(struct game_room));
	return allocer;
}

struct cache_alloc*
create_fruit_allocer(int fruit_num) {
	struct cache_alloc* allocer = create_cache_alloc(fruit_num, sizeof(struct game_fruit));
	return allocer;
}

struct game_room*
create_game_room(struct internet_server* s, int zid) {
	struct game_room* room = cache_alloc(s->room_allocer, sizeof(struct game_room));
	memset(room, 0, sizeof(struct game_room));

	room->server = s;
	room->zid = zid;
	room->roomid = s->auto_inc_roomid;
	s->auto_inc_roomid ++;
	room->room_status = ROOM_REDAY;
	room->game_time = GSERVER_CONF.zid_time_set[room->zid - 1];
	room->checkout_time = GSERVER_CONF.checkout_time;
	// ���뵽���ǵ�hash����
	set_hash_int_map(s->room_set, room->roomid, room);
	// end 

	// room
	vector_define(&room->fruit_array, sizeof(struct game_fruit*));
	// end 
	return room;
}

int 
is_room_can_enter(struct game_room* room, int* player_num) {
	if (room->room_status != ROOM_REDAY) {
		return 0;
	}

	int num = get_player_num(room);
	if (num >= PLAYER_SEAT) {
		return 0;
	}

	*player_num = num;
	return 1;
}

static int
search_empty_seat(struct game_room* room) {
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0) {
			return i;
		}
	}

	return -1;
}
/*
0: ����� 
1: ����� GAME_START
2: game_time: 60s,
*/
static json_t*
pack_game_start_cmd(struct game_room* room) {
	json_t* json = json_new_server_return_cmd(STYPE_GAME_INTERNET, GAME_START, 0, 0);
	json_object_push_number(json, "2", room->game_time);
	return json;
}


struct game_result {
	int total_num;
	int fruit_score[FRUIT_NUM];
};

static void
comp_score_fruit_per(struct game_room* room, struct game_result* result_array) {
	int size;
	struct game_fruit** begin;

	begin = vector_begin(&room->fruit_array);
	size = vector_size(&room->fruit_array);
	for (int i = 0; i < size; i++) {
		struct game_fruit* fruit = begin[i];
		if (fruit->status == 0) {
			continue;
		}

		int seatid = fruit->slice_seatid;
		result_array[seatid].fruit_score[fruit->f_type] ++;
	}

	for (int i = 0; i < PLAYER_SEAT; i++) {
		result_array[i].total_num = 0;
		for (int j = 0; j < FRUIT_NUM; j++) {
			result_array[i].total_num += result_array[i].fruit_score[j];
		}
	}
}

/*
0:�����
1:�����
2: winer -1, ��ʾ������ƽ�֣�����Ļ�����Ӯ�˵���ΪID
3: [[unick, total, apple, banana, basaha, peach, sandia], [unick, total, apple, banana, basaha, peach, sandia]...]
*/
static json_t*
pack_game_checkout_cmd(struct game_room* room) {
	json_t* json = json_new_server_return_cmd(STYPE_GAME_INTERNET, GAME_CHECKOUT, 0, 0);
	struct game_result result_set[PLAYER_SEAT];
	memset(result_set, 0, sizeof(struct game_result) * PLAYER_SEAT);

	comp_score_fruit_per(room, result_set);

	// �ҳ��� winner, ��ƽ��
	int winer = -1;
	int winer_num = 0;
	int max_score = -1;

	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown) {
			if (room->player_seat[i].score > max_score) {
				winer = room->player_seat[i].seatid;
				winer_num = 1;
				max_score = room->player_seat[i].score;
			}
			else if (room->player_seat[i].score == max_score) {
				winer_num ++;
			}
		}
	}

	// ƽ��
	if (winer_num > 1) {
		winer = -1;
	}
	// end 

	json_object_push_number(json, "2", winer);

	json_t* score_result_array = json_new_array();

	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0) {
			continue;
		}

		player_on_game_checkout(room->player_seat[i].player, winer);

		json_t* one_player = json_new_array();
		json_array_push_string(one_player, room->player_seat[i].player->unick);
		json_array_push_number(one_player, room->player_seat[i].score);
		int seatid = room->player_seat[i].seatid;

		for (int j = 0; j < FRUIT_NUM; j++) {
			json_array_push_number(one_player, result_set[seatid].fruit_score[j]);
		}

		json_insert_child(score_result_array, one_player);
	}

	json_insert_pair_into_object(json, "3", score_result_array);

	return json;
}

/*
0:�����
1:�����
2: [id, f_type, G, speed, degree, xpos, ypos]
*/
static json_t*
pack_gen_fruit(struct game_room* room, struct game_fruit* fruit) {
	json_t* json = json_new_server_return_cmd(STYPE_GAME_INTERNET, GEN_FRUIT, 0, 0);
	json_t* fruit_array = json_new_array();
	json_array_push_number(fruit_array, fruit->id);
	json_array_push_number(fruit_array, fruit->f_type);
	json_array_push_number(fruit_array, fruit->G);
	json_array_push_number(fruit_array, fruit->speed);
	json_array_push_number(fruit_array, fruit->degree);
	json_array_push_number(fruit_array, fruit->xpos);
	json_array_push_number(fruit_array, fruit->ypos);
	json_insert_pair_into_object(json, "2", fruit_array);
	
	return json;
}


static void
reset_room(struct game_room* room) {
	room->room_status = ROOM_REDAY;

	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0) {
			continue;
		}

		player_on_after_checkout(room->player_seat[i].player);
	}
}

static void 
kick_offline_player(struct game_room* room) {
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0) {
			continue;
		}

		if (room->player_seat[i].player->online != 0) {
			continue;
		}

		// ���ߵ����
		model_exit_internet_server(room->server, room->player_seat[i].player->uid, 0);
		// end 
	}
}

static json_t* 
pack_game_after_checkout_cmd(struct game_room* room) {
	json_t* json = json_new_server_return_cmd(STYPE_GAME_INTERNET, AFTER_CHECKOUT, 0, 0);
	return json;
}

static void
after_checkout(struct game_room* room) {
	// �߿����ߵ����
	kick_offline_player(room);
	// end 

	// �������״̬��Ϊ��ʼ����Ϸ״̬
	// ����п�λ����������ҿ��Խ�����
	reset_room(room);
	// end 

	

	// �㲥���ͻ��ˣ�����������ÿͻ������»ص�׼����״̬
	json_t* json_after_checkout = pack_game_after_checkout_cmd(room);
	broadcast_json_cmd_in_room(room, json_after_checkout);
	json_free_value(&json_after_checkout);
	// end 
}

// ��Ϸ�������
static void 
on_game_checkout(struct game_room* room) {
	netbus_cancel_timer(room->checkout_timer);

	// �������״̬��Ϊ����״̬
	room->room_status = ROOM_INCHECKOUT;
	// end 

	// ֪ͨÿ����ҽ��㡣
	/*for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0) {
			continue;
		}

		player_on_game_checkout(room->player_seat[i].player);
	}*/
	// end

	// �㲥������
	json_t* json_checkout = pack_game_checkout_cmd(room);
	broadcast_json_cmd_in_room(room, json_checkout);
	json_free_value(&json_checkout);
	// end 

	// ������ɣ���Ϸ����ָ�����ʱ��Ϸ��״̬
	netbus_add_timer(after_checkout, room, (float)room->checkout_time);
	// end 
}

#define THROW_DT 0.8f
#define UPPER_DT 0.2f

static void
throw_fruit(struct game_room* room) {
	if (room->room_status != ROOM_INGAME) {
		return;
	}

	// �׳�һ��ˮ��
	struct game_fruit* fruit = gen_fruit(room);
	vector_push_back(&room->fruit_array, &fruit);

	json_t* gen_fruit_cmd = pack_gen_fruit(room, fruit);
	broadcast_json_cmd_in_room(room, gen_fruit_cmd);
	json_free_value(&gen_fruit_cmd);
	// end 


	float time = THROW_DT + UPPER_DT * random_float(); // [0.5, 0.7)
	room->throw_fruit_timer = netbus_add_timer(throw_fruit, room, time);
}

static void 
clear_fruit_array(struct game_room* room) {
	int size = vector_size(&room->fruit_array);
	struct game_fruit** fruit_set = (struct game_fruit**)vector_begin(&room->fruit_array);
	for (int i = 0; i < size; i++) {
		cache_free(room->server->fruit_allocer, fruit_set[i]);
	}
	vector_pop_all(&room->fruit_array);
}

static void 
start_game(struct game_room* room) { 
	

	// ��¼��Ϸ��ʼ��ʱ��
	room->start_timestamp = timestamp();
	room->checkout_timer = netbus_add_timer(on_game_checkout, room, (float)room->game_time);
	// end 

	float time = THROW_DT + UPPER_DT * random_float(); // [0.5, 0.7)

	room->auto_fruit_id = 1; // ˮ����id
	room->throw_fruit_timer = netbus_add_timer(throw_fruit, room, time);
	
	// �㲥��Ϸ��ʼ
	json_t* json_start_game = pack_game_start_cmd(room);
	broadcast_json_cmd_in_room(room, json_start_game);
	json_free_value(&json_start_game);
	// end 
}

static void
clear_player_score(struct game_room* room) {
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown) {
			room->player_seat[i].score = 0;
		}
	}
}

static int 
get_player_ready_num(struct game_room* room) {
	int num = 0;
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown && room->player_seat[i].player->status == PLAYER_READY) {
			num++;
		}
	}

	return num;
}

static void 
check_game_start(struct game_room* room) {
	if (get_player_ready_num(room) < 2) { // ��һ�������һ�֡�
		return;
	}

	// ���Կ�ʼ��Ϸ��,��Ϸ���ݵ�����
	clear_player_score(room);
	clear_fruit_array(room);
	room->room_status = ROOM_INGAME;
	// end 

	// ֪ͨ���׼����,��Ϸ��ʼ�ˡ�
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0) {
			continue;
		}

		player_on_game_start(room->player_seat[i].player);
	}
	// end

	netbus_add_timer(start_game, room, 1);
}

void
enter_game_room(struct game_room* room, 
                struct game_player* player) {
	int seatid = search_empty_seat(room);
	if (seatid < 0 || seatid >= PLAYER_SEAT) {
		return;
	}

	room->player_seat[seatid].player = player;
	room->player_seat[seatid].is_sitdown = 1;
	room->player_seat[seatid].seatid = seatid;

	player->roomid = room->roomid;
	player->sv_seatid = seatid;

	// ��������Э����ͻ��ˣ���ʾ���Ѿ������ˡ�
	send_user_enter_room(room, player);
	// end 

	// ���������Ѿ��ڵ���ҵ����ݣ����͸�������������
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0 ||
			room->player_seat[i].seatid == player->sv_seatid) {
			continue;
		}
		send_user_arrive_room(room, room->player_seat[i].player, player);
	}
	// end 

	// �����������ҵ���Ϣ�㲥�����������
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0 ||
			room->player_seat[i].seatid == player->sv_seatid) {
			continue;
		}
		LOGINFO("\n===============\n");
		send_user_arrive_room(room, player, room->player_seat[i].player);
		LOGINFO("\n===============\n");
	}
	// end 

	// �����Ϸ�Ƿ�ʼ
	// check_game_start(room);
	// end 
}

static void
broadcast_user_standup_room(struct game_room* room, int sv_seatid) {
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown) {
			send_user_standup_room(room, room->player_seat[i].player, sv_seatid);
		}
	}
}

int 
player_try_exit_room(struct game_room* room, struct game_player* player) {
	if (room->room_status == ROOM_INGAME) {
		return 0;
	}

	int sv_seatid = player->sv_seatid;
	// �˳�
	room->player_seat[player->sv_seatid].player = NULL;
	room->player_seat[player->sv_seatid].is_sitdown = 0;
	room->player_seat[player->sv_seatid].seatid = 0;
	// end

	player->roomid = 0;
	player->sv_seatid = -1;
	player->zid = INVALIDI_ZONE;

	// ���������˳�����Ϣ���͸�����������������
	broadcast_user_standup_room(room, sv_seatid);
	// end 

	return 1;
}

int 
player_force_exit_room(struct game_room* room, struct game_player* player) {
	int sv_seatid = player->sv_seatid;
	
	

	// �˳�
	room->player_seat[player->sv_seatid].player = NULL;
	room->player_seat[player->sv_seatid].is_sitdown = 0;
	room->player_seat[player->sv_seatid].seatid = 0;
	// end
	player->roomid = 0;
	player->sv_seatid = -1;
	player->zid = INVALIDI_ZONE;

	// ���������˳�����Ϣ���͸�����������������
	broadcast_user_standup_room(room, sv_seatid);
	// end 

	if (room->room_status != ROOM_INGAME) {
		return 1;
	}

	// ���ǿ�˿۷�
	// end 

	// �㲥���ǿ�ˣ�������
	on_game_checkout(room);
	// end 
	
	return 1;
}

// [[[unick, uface, usex], []]...]
json_t*
get_other_players_info(struct game_room* room, struct game_player* yourself) {
	json_t* json_array = json_new_array();
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0 ||
			room->player_seat[i].player == yourself) {
			continue;
		}

		json_t* one_user = json_new_array();

		json_t* uinfo = get_player_user_info(room->player_seat[i].player);
		json_insert_child(one_user, uinfo);

		json_t* game_info = get_player_game_info(room->player_seat[i].player);
		json_insert_child(one_user, game_info);

		json_t* room_info = get_player_room_info(room->player_seat[i].player);
		json_insert_child(one_user, room_info);

		json_insert_child(json_array, one_user);
	}
	return json_array;
}

/*
[room_status, last_time, ...]
*/
json_t*
get_room_status_info(struct game_room* room) {
	json_t* room_info = json_new_array();
	json_array_push_number(room_info, room->room_status);

	// ������Ϸʣ�µ�ʱ��
	int last_time = 0;
	if (room->room_status != ROOM_INGAME) {
		return room_info;
	}
	// end 

	unsigned int time = timestamp();
	last_time = room->game_time - (time - room->start_timestamp);
	if (last_time < 0) {
		last_time = 0;
	}
	json_array_push_number(room_info, last_time);

	// ÿ����λ�ĵ÷���Ϣ
	json_t* score_set = json_new_array();
	for (int i = 0; i < PLAYER_SEAT; i++) {
		if (room->player_seat[i].is_sitdown == 0) {
			continue;
		}

		// 
		json_t* one_player = json_new_array();
		json_array_push_number(one_player, room->player_seat[i].seatid);
		json_array_push_number(one_player, room->player_seat[i].score);

		json_insert_child(score_set, one_player);
		// end 
	}
	// end 

	json_insert_child(room_info, score_set);
	return room_info;
}


int
player_reconnect_room(struct game_room* room, struct game_player* player) {
	LOGINFO("player_reconnect_room called");
	send_user_reconnect_room(room, player);
	return 0;
}

/*
0: �����
1: �����
2: status OK
3: ��λ��
4: ˮ����
5: ��ǰ���������µ÷�
6: ��ˮ���ĽǶ�
*/
static json_t*
pack_slice_fruit(int sv_seatid, int fruit_id, int score, int degree) {
	json_t* json = json_new_server_return_cmd(STYPE_GAME_INTERNET, SLICE_FRUIT, 0, 0);
	json_object_push_number(json, "2", OK);
	json_object_push_number(json, "3", sv_seatid);
	json_object_push_number(json, "4", fruit_id);
	json_object_push_number(json, "5", score);
	json_object_push_number(json, "6", degree);
	return json;
}

static json_t*
pack_send_prop_cmd(int from_seatid, int to_seatid, int prop_id) {
	json_t* json = json_new_server_return_cmd(STYPE_GAME_INTERNET, PLAYER_SEND_PROP, 0, 0);
	json_object_push_number(json, "2", OK);
	json_object_push_number(json, "3", from_seatid);
	json_object_push_number(json, "4", to_seatid);
	json_object_push_number(json, "5", prop_id);
	return json;
}

int 
player_slice_fruit(struct game_room* room, 
                   struct game_player* player, int fruit_id, int degree) {
	
	if (room->room_status != ROOM_INGAME) {
		return MODEL_INTERNET_SERVER_INVALID_OPT;
	}

	int size = vector_size(&room->fruit_array);
	if (fruit_id <= 0 || fruit_id > size) {
		return MODEL_INTERNET_SERVER_ROOM_FUIT_IS_NOT_EXIST;
	}

	struct game_fruit** elem_ptr = vector_iterator_at(&room->fruit_array, fruit_id - 1);
	struct game_fruit* fruit = *elem_ptr;
	if (fruit->status == 1) { // ˮ���Ѿ����е���
		return MODEL_INTERNET_SERVER_FRUIT_IS_SLICED;
	}

	// ��֤һ�£���ҵ��е�λ�ã��ǲ�������е���ˮ����
	// end 

	fruit->status = 1; // ˮ���Ѿ��е�
	fruit->slice_seatid = player->sv_seatid;
	room->player_seat[player->sv_seatid].score ++;

	// �㲥�����������, ĳĳ�е���ˮ��
	json_t* slice_fruit_cmd = pack_slice_fruit(player->sv_seatid, fruit_id, room->player_seat[player->sv_seatid].score, degree);
	broadcast_json_cmd_in_room(room, slice_fruit_cmd);
	json_free_value(&slice_fruit_cmd);
	// end 
	return MODEL_INTERNET_SERVER_SUCCESS;
}

int
player_set_ready_in_room(struct game_room* room, struct game_player* player) {
	if (room->room_status != ROOM_REDAY || player->status != PLAYER_INVIEW) {
		return MODEL_INTERNET_SERVER_INVALID_OPT;
	}

	player->status = PLAYER_READY;

	check_game_start(room);

	return MODEL_INTERNET_SERVER_SUCCESS;
}

int
player_send_prop_in_room(struct game_room* room, 
                        struct game_player* player, 
                        int to_seatid, int prop_id) {
	// �Լ����ܷ����Լ�,�Է�����Ϊ��λ
	if (player->sv_seatid == to_seatid || 
		room->player_seat[to_seatid].is_sitdown == 0) {
		return MODEL_INTERNET_SERVER_INVALID_OPT;
	}
	// end

	if (prop_id <= 0 || prop_id > 5) {
		return MODEL_INTERNET_SERVER_INVALID_PARAMS;
	}

	// �Ϸ��ĵ��ߣ��㲥�����еĿͻ������˶�������
	json_t* ret = pack_send_prop_cmd(player->sv_seatid, to_seatid, prop_id);
	broadcast_json_cmd_in_room(room, ret);
	json_free_value(&ret);
	// end 
	return MODEL_INTERNET_SERVER_SUCCESS;
}
