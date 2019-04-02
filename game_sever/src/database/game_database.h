#ifndef __GAME_DATABASE_H__
#define __GAME_DATABASE_H__

#include "../3rd/mjson/json.h"
#include "../utils/vector_array.h"
struct game_mask {
	int main_type; // �����������
	int level; // ����ļ��� 

	char mask_desic[128]; // ���������
	int max_cond; // ����������

	int bonues1; //  ����������ֵ
	int bonues2; // ����������ֵ
	int bonues3; // ����������ֵ
};

#define MASK_NUM 3
enum {
	WIND_ROUND_MASK = 1,
	SLICE_MASK = 2,
	EXPERIENCE_TASK = 3,
};

struct game_mask_config {
	struct vector_array mask_config_set[MASK_NUM];
	// struct vector_array win_round_mask_config;
	// struct vector_array slice_fruit_mask_config;
};

extern struct game_mask_config MASK_CONFIG;

void connect_to_game();

struct login_bonues_info {
	unsigned int bonues_value;
	unsigned int bonues_time; // ��һ����ȡ��½������ʱ��
	int udays; // ������½�˶�����;
	unsigned int never_login_bonues; // ��������û����ȡ������ô�����Ϊ1
	int status; // 0��ʾû����ȡ��1��ʾ�Ѿ���ȡ
};

struct ugame_common_info {
	unsigned int uid;
	int uchip;
	int uexp;
	int uvip;
	int ustatus; // ����Ƿ��ţ�
	// ....
};

#define MAX_RANK_OPTION 30
struct rank_option_info {
	char unick[64];
	int chip;
	int face;
	int sex;
};

struct ugame_rank_info {
	int option_num;
	struct rank_option_info option_set[MAX_RANK_OPTION];
};

struct game_rank_user {
	int rank_num;
	unsigned int rank_user_uid[MAX_RANK_OPTION];
};

struct task_cond_info {
	int level;
	int now;
	int end_cond;

	int has_info;
};

struct task_bonues_info {
	int main_type;
	int task_level;

	int bonues_chip;

	int has_info;
};

int
get_login_bonues_info_by_uid(unsigned int uid, 
                             struct login_bonues_info* bonues_info);

int
insert_login_bonues_info_by_uid(unsigned int uid, 
                              struct login_bonues_info* bonues_info);

int
update_login_bonues_info_by_uid(unsigned int uid, 
                              struct login_bonues_info* bonues_info);

int
update_task_cond(unsigned int uid, int main_type);

int
get_task_cond_info(unsigned int uid, int main_type, struct task_cond_info* cond_info);

int
update_task_level_complet(unsigned int uid, int main_type, int level);

int
update_task_on_going(unsigned int uid, int main_type, int level);

int 
update_login_bonues_status(unsigned int uid, int status);

int
update_task_bonues_status(int taskid, int status);

int
get_ugame_common_info_by_uid(unsigned int uid, struct ugame_common_info* ugame_info);

int
get_user_bonues_info_by_uid(unsigned int uid, json_t** bonues_array);

int
get_user_mask_info_by_uid(unsigned int uid, json_t** mask_array);

int
insert_ugame_common_info_by_uid(unsigned int uid, struct ugame_common_info* info);

int
insert_game_task(unsigned int uid);

int
add_ugame_uchip(unsigned int uid, int chip);

int
get_task_bonues_recode_info(unsigned int uid, int taskid, struct task_bonues_info* info);
// end

// redis 
void
set_ugame_common_info_inredis(struct ugame_common_info* info);

int
get_ugame_common_info_inredis(unsigned uid, struct ugame_common_info* uinfo);

void
flush_game_rank_info_inredis(struct ugame_common_info* info);

int
get_game_rank_user_inredis(struct game_rank_user* rank_user);

#endif

