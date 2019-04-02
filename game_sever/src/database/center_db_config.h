#ifndef __CENTER_DB_CONFIG_H__
#define __CENTER_DB_CONFIG_H__


struct center_db_config {
	// ���ݿ�
	char* mysql_ip;
	char* mysql_name;
	int mysql_port;
	char* mysql_pwd;
	char* database_name;
	// end 

	// redis
	char* redis_ip; // redis��������IP��ַ
	int redis_port; // redis�����������Ӷ˿�
	char* redis_pwd; // redis������������
	int redis_dbid; // center���ڵ�redis���ݿ��id��
	// end
};

extern struct center_db_config CENTER_DB_CONF;
#endif

