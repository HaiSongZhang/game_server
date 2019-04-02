#ifndef __MODEL_MASK_AND_BONUES_H__
#define __MODEL_MASK_AND_BONUES_H__

#include "../../3rd/mjson/json.h"

#include "../../database/game_database.h"
enum {
	MODEL_MASK_AND_BONUES_SUCCESS = 0,
	MODEL_MASK_AND_BONUES_SYSTEM_ERROR = -101, // ϵͳ��������
	MODEL_MASK_AND_BONUES_TASK_COMPLETED = -102, // ���������Ѿ����
	MODEL_MASK_AND_BONUES_INVALID_OPT = -103, // �Ƿ��Ĳ���
};

int
model_get_mask_and_bonues_info(unsigned int uid, 
                               json_t** bonues_array, 
							   json_t** mask_array);

int
model_winner_task_win_round(unsigned int uid);

int
model_recv_task_bonues(unsigned int uid, int task_id, struct task_bonues_info* info);


#endif

