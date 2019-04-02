#ifndef __GAME_ROOM_H__
#define __GAME_ROOM_H__

#include "../netbus/netbus.h"
#include "../utils/cache_allocer.h"

#include "internet_server.h"

struct internet_server;

enum {
	ROOM_REDAY = 0, // �����Ѿ�׼������
	ROOM_INGAME = 1, // �����Ѿ���ʼ��Ϸ��
	ROOM_INCHECKOUT = 2, // �����ڽ�����
};
// ��������
struct game_room;

struct cache_alloc* 
create_room_allocer(int room_num);

struct cache_alloc*
create_fruit_allocer(int fruit_num);

struct game_room*
create_game_room(struct internet_server* s, int zid);

// ���ٷ���
void
destroy_game_room(struct game_room* room);

int 
is_room_can_enter(struct game_room* room, int* player_num);

// ��ҽ��뷿��
void
enter_game_room(struct game_room* room, struct game_player* player);

// ��ҳ����˳�room
// ����˳��ɹ���return 1,����return 0
int
player_try_exit_room(struct game_room* room, struct game_player* player);

// ���ǿ�˳�
int
player_force_exit_room(struct game_room* room, struct game_player* player);

// ��Ҷ�������
int
player_reconnect_room(struct game_room* room, struct game_player* player);

// ��ȡ��������ڷ������Ϣ
json_t*
get_other_players_info(struct game_room* room, struct game_player* yourself);
//

// ��ȡ�����״̬��Ϣ
json_t*
get_room_status_info(struct game_room* room);

// end
int
player_slice_fruit(struct game_room* room, struct game_player* player, int fruit_id, int degree);

int
player_set_ready_in_room(struct game_room* room, struct game_player* player);

int
player_send_prop_in_room(struct game_room* room, struct game_player* player, int to_seatid, int prop_id);
#endif

