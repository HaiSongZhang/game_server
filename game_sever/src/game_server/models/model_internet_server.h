#ifndef __MODEL_INTERNET_SERVER_H__
#define __MODEL_INTERNET_SERVER_H__

#include "../internet_server.h"

enum {
	MODEL_INTERNET_SERVER_SUCCESS = 0, // �����ɹ�
	MODEL_INTERNET_SERVER_ERROR = -1, // ͨ�õ�error
	MODEL_INTERNET_SERVER_USER_IS_NOT_IN_SERVER = -2, // ��Ҳ��ڷ�������
	MODEL_INTERNET_SERVER_USER_IS_IN_ZONE = -3, // �û��Ѿ��ڷ�������
	MODEL_INTERNET_SERVER_INVALID_ZONE = -4, // 
	MODEL_INTERNET_SERVER_USER_IS_NOT_IN_ZONE = -5, // �û���û�б�����
	MODEL_INTERNET_SERVER_USER_IS_IN_GAME = -6, // �û�������Ϸ�У��޷��˳�
	MODEL_INTERNET_SERVER_USER_RECONNECT = -7, // �û����ڷ������棬׼�����ߴ���
	MODEL_INTERNET_SERVER_USER_IS_NOT_IN_ROOM = -8, // �û����ڷ�������
	MODEL_INTERNET_SERVER_ROOM_IS_NOT_EXIST = -9, // ���䲻����
	MODEL_INTERNET_SERVER_ROOM_FUIT_IS_NOT_EXIST = -10, // ˮ��������
	MODEL_INTERNET_SERVER_FRUIT_IS_SLICED = -11,  // ˮ���Ѿ����е�
	MODEL_INTERNET_SERVER_INVALID_OPT = -12, // �Ƿ��Ĳ���
	MODEL_INTERNET_SERVER_INVALID_PARAMS = -13, // ��������
};

int
model_enter_internet_server(struct internet_server* server, unsigned int uid, struct session* s);

int
model_exit_internet_server(struct internet_server* server, unsigned int uid, int is_online);

int
model_enter_internet_server_zone(struct internet_server* server, unsigned int uid, int zid);

int
model_auto_get_zoneid(struct internet_server* server, unsigned int uid);

int
model_exit_internet_server_zone(struct internet_server* server, unsigned int uid);

int
model_player_reconnect_to_room(struct internet_server* server, unsigned int uid);

int
model_slice_fruit(struct internet_server* server, unsigned int uid, int fruit_id, int degree);

int
model_player_set_ready_in_room(struct internet_server* server, unsigned int uid);

int
model_player_send_prop_in_room(struct internet_server* server, unsigned int uid, int to_seatid, int prop_id);
#endif

