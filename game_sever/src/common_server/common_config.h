#ifndef __CENTER_CONFIG_H__
#define __CENTER_CONFIG_H__

struct common_config {
	char* ip;
	int port;

	// ÿ�յ�¼��������
	int login_bonues_straight[7]; // ��������Ŀ
	int max_straight_days; // �������������¼������
	// end

	// ��һ��ע����Ϸ�˺��͵Ľ��
	int uchip;
	int uexp;
	int uvip;
	// end
};

extern struct common_config COMMON_CONF;
#endif

