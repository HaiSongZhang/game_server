#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../database/game_database.h"
#include "model_login_bonues.h"
#include "../common_config.h"
#include "../../utils/timestamp.h"

int
model_check_login_bonues(unsigned int uid) {
	struct login_bonues_info binfo;
	if (get_login_bonues_info_by_uid(uid, &binfo) < 0) {
		return MODEL_COMMON_SYSTEM_ERROR;
	}
	int ret = 0;
	// �Ƿ�����ֽ���
	// 
	unsigned int time_today = timestamp_today();
	if (binfo.bonues_time < time_today) { // ���ε�½�н���
		// �����������ڼ��ε�½
		int is_straight = (binfo.bonues_time >= timestamp_yesterday()) ? 1 : 0;
		// end 
		binfo.bonues_time = timestamp(); // ��λ���˵�¼������ʱ��
		if (is_straight) {
			binfo.udays += 1; // �����˶�����
			if (binfo.udays > COMMON_CONF.max_straight_days) {
				// binfo.udays = 1;
				binfo.udays = COMMON_CONF.max_straight_days;
			}
		}
		else {
			binfo.udays = 1; // ��һ��û�е�½�����¿�ʼ
		}
		binfo.bonues_value = COMMON_CONF.login_bonues_straight[binfo.udays - 1];
		// ��ȡ��½����������
		if (binfo.never_login_bonues) { // ����һ����¼
			ret = insert_login_bonues_info_by_uid(uid, &binfo);
		}
		else { // ���µ�½�����ļ�¼
			ret = update_login_bonues_info_by_uid(uid, &binfo);
		}
		// end 
	}
	// end 

	if (ret < 0) {
		return MODEL_COMMON_SYSTEM_ERROR;
	}
	return MODEL_COMMON_SUCCESS;
}

int
model_get_login_bonues_info(unsigned int uid, struct login_bonues_info* binfo) {
	
	if (get_login_bonues_info_by_uid(uid, binfo) < 0) {
		return MODEL_COMMON_SYSTEM_ERROR;
	}

	if (binfo->never_login_bonues || binfo->status != 0) {
		return MODEL_COMMON_NO_LOGIN_BONUES;
	}

	return MODEL_COMMON_SUCCESS;
}


int
model_recv_login_bonues_info(unsigned int uid, struct login_bonues_info* binfo) {
	if (get_login_bonues_info_by_uid(uid, binfo) < 0) {
		return MODEL_COMMON_SYSTEM_ERROR;
	}

	if (binfo->never_login_bonues || binfo->status != 0) {
		return MODEL_COMMON_NO_LOGIN_BONUES;
	}

	// ��ȡ����
	if (update_login_bonues_status(uid, 1) < 0) {
		return MODEL_COMMON_SYSTEM_ERROR;
	}
	//  end

	// ���������뵽��Ϸ��ֵ���棬 uchip ����
	if (add_ugame_uchip(uid, binfo->bonues_value) < 0) {
		return MODEL_COMMON_SYSTEM_ERROR;
	}
	// end


	// ����reids
	struct ugame_common_info ugame_info;
	get_ugame_common_info_by_uid(uid, &ugame_info);
	set_ugame_common_info_inredis(&ugame_info);
	// end 
	return MODEL_COMMON_SUCCESS;
}