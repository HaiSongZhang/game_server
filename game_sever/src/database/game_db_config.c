#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../netbus/netbus.h"

#include "game_db_config.h"

struct game_db_config GAME_DB_CONF = {
	// MYSQL
	"localhost",
	"root",
	3306,
	"123456",
	"bycw_game",
	// end

	// redis
	"localhost",
	6379,
	NULL, // û�������ΪNULL��"123456"
	0,   // 0�����ݲֿ���������ǵ����ķ����������ݵ�redis���ݿ�
	// end
};
