#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../database/game_database.h"
#include "model_mask_and_bonues.h"
#include "../common_config.h"

#include "../../utils/timestamp.h"

int
model_get_mask_and_bonues_info(unsigned int uid,
                               json_t** bonues_array,
							   json_t** mask_array) {

	// ��ȡ�Ѿ��ɵ���û����ȡ�Ľ�������
	if (get_user_bonues_info_by_uid(uid, bonues_array) < 0) {
		return MODEL_MASK_AND_BONUES_SYSTEM_ERROR;
	}
	// end 

	// ��ȡ���ڽ����е�����
	if (get_user_mask_info_by_uid(uid, mask_array) < 0) {
		return MODEL_MASK_AND_BONUES_SYSTEM_ERROR;
	}
	// end 

	return MODEL_MASK_AND_BONUES_SUCCESS;
}

int
model_winner_task_win_round(unsigned int uid) {
	// �������ݿ⣬�ƽ�Ӯ�ֵ�������ǰ
	if (update_task_cond(uid, WIND_ROUND_MASK) < 0) {
		return MODEL_MASK_AND_BONUES_SYSTEM_ERROR;
	}
	// end

	// �����Ƿ�ﵽ�������
	struct task_cond_info info;
	if (get_task_cond_info(uid, WIND_ROUND_MASK, &info) < 0) {
		return MODEL_MASK_AND_BONUES_SYSTEM_ERROR;
	}
	// end 

	if (!info.has_info) {
		return MODEL_MASK_AND_BONUES_TASK_COMPLETED;
	}

	if (info.now >= info.end_cond) { // �������,�������˽���
		update_task_level_complet(uid, WIND_ROUND_MASK, info.level);
		// ������һ���ȼ�������
		update_task_on_going(uid, WIND_ROUND_MASK, info.level + 1);
	}

	return MODEL_MASK_AND_BONUES_SUCCESS;
}

int
model_recv_task_bonues(unsigned int uid, int task_id, struct task_bonues_info* info) {
	
	if (get_task_bonues_recode_info(uid, task_id, info) < 0) {
		return MODEL_MASK_AND_BONUES_INVALID_OPT;
	}
	if (!info->has_info) {
		return MODEL_MASK_AND_BONUES_INVALID_OPT;
	}

	// �������ĵȼ�, main_level, task_level
	// ���������뵽��Ϸ��ֵ���棬 uchip ����
	if (add_ugame_uchip(uid, info->bonues_chip) < 0) {
		return MODEL_MASK_AND_BONUES_SYSTEM_ERROR;
	}
	// end

	// ��������״̬��ǳ�����ȡ
	if (update_task_bonues_status(task_id, 3) < 0) {
		return MODEL_MASK_AND_BONUES_SYSTEM_ERROR;
	}
	// end 

	// ����reids
	struct ugame_common_info ugame_info;
	get_ugame_common_info_by_uid(uid, &ugame_info);
	set_ugame_common_info_inredis(&ugame_info);
	// end

	// ��֤�����ĺϷ���;
	return MODEL_MASK_AND_BONUES_SUCCESS;
}