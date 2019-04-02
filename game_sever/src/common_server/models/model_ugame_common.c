#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "model_ugame_common.h"
#include "../common_config.h"

#include "../../database/center_database.h"
#include "../../database/game_database.h"


int
model_get_ugame_common_info(unsigned int uid, struct ugame_common_info* info) {
	// ��ѯ�����Ϸ����Ϸ��Ϣ
	if (get_ugame_common_info_by_uid(uid, info) == 0) {
		set_ugame_common_info_inredis(info);
		if (info->ustatus != 0) { // �Ѿ����,���Ի�ȡ��Ϣʧ��
			return MODEL_UGAME_USER_IS_CLOSE_DOWN;
		}

		// �������ǵĸ��µ�����
		if (insert_game_task(uid) < 0) {
			return MODEL_UGAME_SYSTEM_ERR;
		}
		// end 

		// ˢ��һ�����а�
		flush_game_rank_info_inredis(info);
		// end 

		return MODEL_UGAME_SUCCESS;
	}

	// �����µ�
	info->uchip = COMMON_CONF.uchip;
	info->uexp = COMMON_CONF.uexp;
	info->uvip = COMMON_CONF.uvip; 
	info->ustatus = 0;
	info->uid = uid;

	if (insert_ugame_common_info_by_uid(uid, info) < 0) {
		return MODEL_UGAME_SYSTEM_ERR;
	}

	// �������ǵ�����
	if (insert_game_task(uid) < 0) {
		return MODEL_UGAME_SYSTEM_ERR;
	}
	// end 

	// end 

	// д�뵽redis
	set_ugame_common_info_inredis(info);
	// end 
	
	// ˢ��һ�����а�
	flush_game_rank_info_inredis(info);
	// end 

	

	return MODEL_UGAME_SUCCESS;
	// end 
	
}

int
model_get_ugame_rank_info(unsigned int uid, struct ugame_rank_info* rank_info) {
	struct game_rank_user rank_user;
	if (get_game_rank_user_inredis(&rank_user) != 0) {
		return MODEL_UGAME_SYSTEM_ERR;
	}

	memset(rank_info, 0, sizeof(struct ugame_rank_info));
	rank_info->option_num = rank_user.rank_num;

	// for uid
	for (int i = 0; i < rank_user.rank_num; i++) {

		struct user_info uinfo;
		get_uinfo_inredis(rank_user.rank_user_uid[i], &uinfo);

		struct ugame_common_info ugame_info;
		get_ugame_common_info_by_uid(rank_user.rank_user_uid[i], &ugame_info);

		rank_info->option_set[i].chip = ugame_info.uchip;
		rank_info->option_set[i].face = uinfo.uface;
		strcpy(rank_info->option_set[i].unick, uinfo.unick);
		rank_info->option_set[i].sex = uinfo.usex;
	}
	// end 
	return MODEL_UGAME_SUCCESS;
}

