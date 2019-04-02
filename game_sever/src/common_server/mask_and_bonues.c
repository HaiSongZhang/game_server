#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../game_stype.h"
#include "../game_command.h"
#include "../game_result.h"

#include "ugame_common.h"
#include "models/model_ugame_common.h"
#include "models/model_mask_and_bonues.h"
/*
0: �����
1: �����
����
2: status -- OK
3: [�Ѿ���ɣ�����û����ȡ�Ľ���]
4: [������ɵ�����]
... �޲�������
5: [δ��ʼ������]
6: [�Ѿ���ɣ��Ѿ���ȡ�Ľ���]
...
*/

void
get_mask_and_bonues_info(void* module_data, json_t* json,
                        struct session* s,
                        unsigned int uid, unsigned int s_key) {
	int len = json_object_size(json);
	if (len != 2 + 2) {
		write_error(s, STYPE_SYSTEM, GET_MASK_AND_BONUES_INFO, INVALID_PARAMS, uid, s_key);
		return;
	}

	json_t* bonus_array = NULL;
	json_t* mask_array = NULL;

	int ret = model_get_mask_and_bonues_info(uid, &bonus_array, &mask_array);
	if (ret != MODEL_MASK_AND_BONUES_SUCCESS) {
		write_error(s, STYPE_SYSTEM, GET_MASK_AND_BONUES_INFO, INVALID_PARAMS, uid, s_key);
		return;
	}


	// mask bonues���͸����ǿͻ���
	json_t* ret_cmd = json_new_server_return_cmd(STYPE_SYSTEM, GET_MASK_AND_BONUES_INFO, uid, s_key);
	json_object_push_number(ret_cmd, "2", OK);
	json_insert_pair_into_object(ret_cmd, "3", bonus_array);
	json_insert_pair_into_object(ret_cmd, "4", mask_array);

	session_send_json(s, ret_cmd);
	json_free_value(&ret_cmd);
	// end 
	// end 
}

void
task_and_bonues_win_one_round(void* module_data, json_t* json,
                              struct session* s,
                              unsigned int uid, unsigned int s_key) {
	int len = json_object_size(json);
	if (len != 2 + 2) {
		return;
	}

	// uid Ӯ��һ�֣�Ȼ���ƶ������ҵ�������ǰ
	model_winner_task_win_round(uid);
	// end 
}

/*
0: �����
1: �����
2: task_id
����:
0: �����
1: �����
2: ״̬�� --OK
3: ����1
4: ����2..
*/

void 
recv_task_bonues(void* module_data, json_t* json,
                 struct session* s,
				  unsigned int uid, unsigned int s_key) {
	
	int len = json_object_size(json);
	if (len != 3 + 2) {
		write_error(s, STYPE_SYSTEM, RECV_TASK_BONUES, INVALID_PARAMS, uid, s_key);
		return;
	}

	int task_id = json_object_get_number(json, "2");
	if (task_id <= 0) {
		write_error(s, STYPE_SYSTEM, RECV_TASK_BONUES, INVALID_PARAMS, uid, s_key);
		return;
	}

	struct task_bonues_info info;
	int ret = model_recv_task_bonues(uid, task_id, &info);
	if (ret != MODEL_MASK_AND_BONUES_SUCCESS) {
		write_error(s, STYPE_SYSTEM, RECV_TASK_BONUES, SYSTEM_ERROR, uid, s_key);
		return;
	}

	// ok 
	json_t*  ret_cmd = json_new_server_return_cmd(STYPE_SYSTEM, RECV_TASK_BONUES, uid, s_key);
	json_object_push_number(ret_cmd, "2", OK);
	json_object_push_number(ret_cmd, "3", info.bonues_chip);
	session_send_json(s, ret_cmd);
	json_free_value(&ret_cmd);
	// end
}
