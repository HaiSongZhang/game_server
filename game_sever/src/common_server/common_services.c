#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../game_stype.h"
#include "../game_command.h"
#include "../game_result.h"
#include "../netbus/netbus.h"
#include "common_services.h"
#include "login_bonueses.h"
#include "ugame_common.h"
#include "mask_and_bonues.h"


static void  // ģ���ʼ�����
init_service_module(struct service_module* module) {

}

// ϵͳͨ�÷�����������
static int
on_json_protocal_recv(void* module_data, struct session* s,
json_t* json, unsigned char* data, int len) {
	if (data != NULL) {
		LOGINFO("center_service data = %s", data);
	}
	
	int cmd = json_object_get_number(json, "1");

	// ����
	unsigned int uid, s_key;
	json_get_command_tag(json, &uid, &s_key);
	// end 

	// ���uid�����ط����������ǵģ�������һ���ǿ��ŵġ�
	if (uid == 0) {
		return 0;
	}

	switch (cmd) {
		// �û���¼�ɹ����Ƿ��е�½����
		case CHECK_LOGIN_BONUES:
		{
			check_login_bonues(module_data, json, s, uid, s_key);
		}
		break;
		case GET_LOGIN_BONUSES_INFO: 
		{
			get_login_bonues_info(module_data, json, s, uid, s_key);
		}
		break;
		case RECV_LOGIN_BONUSES:
		{
			recv_login_bonues(module_data, json, s, uid, s_key);
		}
		break;
		case RECV_TASK_BONUES:
		{
			recv_task_bonues(module_data, json, s, uid, s_key);
		}
		break;
		case GET_UGAME_COMMON_INFO:
		{
			get_ugame_common_info(module_data, json, s, uid, s_key);
		}
		break;
		case GET_GAME_RANK_INFO: 
		{
			get_ugame_rank_info(module_data, json, s, uid, s_key);
		}
		break;

		case GET_MASK_AND_BONUES_INFO: {
			get_mask_and_bonues_info(module_data, json, s, uid, s_key);
		}
		break;
		// ���Ӯ��һ�������ƶ��������ǰ��չ
		case TASK_WIN_EVENT: 
		{
			task_and_bonues_win_one_round(module_data, json, s, uid, s_key);
		}
		break;
	}

	return 0;
}

static void
on_connect_lost(void* module_data, struct session* s) {
#ifndef GAME_DEVLOP
	LOGWARNING("gateway lost connected !!!!!");
#endif
}

struct service_module COMMON_SERVICES = {
	STYPE_SYSTEM,
	init_service_module,
	NULL,
	on_json_protocal_recv,
	on_connect_lost,
	NULL,
};

