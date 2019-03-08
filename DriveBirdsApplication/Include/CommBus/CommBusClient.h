#pragma once

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "..\protected_container.h"
#include "..\includes.h"
#include "CommBusLib.h"

#pragma comment(lib, ".\\lib\\CommBusLib.lib")

using namespace std;

class CCommBusClient
{
public:
	CCommBusClient(string s, string h);
	~CCommBusClient(void);

	struct poppedMessage
	{
		poppedMessage(string s, string t, string d){sender_ins = s; message_type = t; message_data = d;};

		string sender_ins;
		string message_type;
		string message_data;
	};

	static bool lookForCommBus(string& commbus_ip)
	{
		char ip[32] = {0};
		char* ppszIP = ip;

		bool result = lookForFirstCommBus(ppszIP);
		if(result)
			commbus_ip = ip;

		return result;
	};

	bool initialize();
	void release();

	bool pushMessage(string dest_ins, string message_type, string message_data);
	void pushMessage(vector<string> dest_inss, string message_type, string message_data);
	bool popMessage(string& sender_ins, string& message_type, string& message_data, unsigned int interval);
	bool queryInstanceStatus(string dest_ins, string& dest_ip, bool& online_flag, time_t& last_heartbeat_time, unsigned int interval);

	string getCommBusIP(){return host;};

private:
	unsigned int pop();
	static unsigned int _stdcall threadPop(void* p);
	void clearPoppedMessage();

    volatile BOOL m_register_flag;

public:
	//  表示与Commbus链接状态的两个事件，通过 registerToCommBus 函数获得状态
	HANDLE event_link_on;
	HANDLE event_link_off;

private:
	string ins_name;
	string host;

    // 断网重连的时间间隔
    int m_relinkInterval;

	//  接收线程句柄
	HANDLE thread_pop;
    //  停止“接收线程消息循环”事件
	HANDLE event_stop;

	protected_list<poppedMessage*> popped_message;
};
