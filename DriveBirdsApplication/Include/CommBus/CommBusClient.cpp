#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "CommBusClient.h"
//#pragma comment(lib,"..\\lib\\CommBusLib")

CCommBusClient::CCommBusClient(string h, string s)
{
	host = h;
	ins_name = s;
	event_stop = CreateEvent(NULL, TRUE, FALSE, NULL);
	event_link_on = CreateEvent(NULL, FALSE, FALSE, NULL);
	event_link_off = CreateEvent(NULL, FALSE, FALSE, NULL);
	thread_pop = NULL;
    m_relinkInterval = GetPrivateProfileInt(_T("CommbusRelinkInterval"),_T("relink"),5000,_T("./config.ini"));
    m_register_flag = FALSE;
}

CCommBusClient::~CCommBusClient(void)
{
	CloseHandle(event_stop);
}

bool CCommBusClient::initialize()
{
    thread_pop = (void*)_beginthreadex(NULL, 0, threadPop, this, 0, NULL);

	return true;
}

void CCommBusClient::release()
{
	SetEvent(event_stop);

	if(thread_pop != NULL)
	{
        WaitForSingleObject(thread_pop, 5000);
		CloseHandle(thread_pop);
		thread_pop = NULL;
	}

    m_register_flag = FALSE;
    ResetEvent(event_link_on);
    ResetEvent(event_link_off);
    CloseHandle(event_link_off);
	CloseHandle(event_link_on);
	int error = no_error;
	cancelFromLocalCommBus((char*)ins_name.c_str(), error);

	clearPoppedMessage();
}

void CCommBusClient::clearPoppedMessage()
{
	popped_message.enterExplicitProtectedMode();

	while(!popped_message.empty())
	{
		poppedMessage* popped = NULL;

		popped_message.removehead(popped);

		delete popped;
	}

	popped_message.leaveExplicitProtectedMode();
}

bool CCommBusClient::pushMessage(string dest_ins, string message_type, string message_data)
{
	int error = no_error;
	return pushMessageThroughCommBus((char*)host.c_str(), (char*)ins_name.c_str(), (char*)dest_ins.c_str(), (char*)message_type.c_str(), (char*)message_data.c_str(), error);
}

void CCommBusClient::pushMessage(vector<string> dest_inss, string message_type, string message_data)
{
	for(unsigned int ii = 0; ii < dest_inss.size(); ii ++)
		pushMessage(dest_inss[ii], message_type, message_data);
}

bool CCommBusClient::popMessage(string& sender_ins, string& message_type, string& message_data, unsigned int interval)
{
	poppedMessage* popped = NULL;

	popped_message.enterExplicitProtectedMode();
	if(popped_message.wait(interval))
		popped_message.removehead(popped);
	popped_message.leaveExplicitProtectedMode();

	if(popped != NULL)
	{
		sender_ins = popped->sender_ins;
		message_type = popped->message_type;
		message_data = popped->message_data;

		delete popped;

		return true;
	}

	return false;
}

unsigned int CCommBusClient::pop()
{
    int interval = 50;
	int error = no_error;

	char sender_ins[256] = {0}, message_type[256] = {0}, message_data[max_commbusmessage_length] = {0};
	char *pszSenderIns = sender_ins, *pszMessageType = message_type, *pszMessageData = message_data;

	//  等待event_stop事件的设置（在退出的时候）
    while(WaitForSingleObject(event_stop, 100) != WAIT_OBJECT_0)
	{
        // 先检测注册标志，再看是否执行注册函数
        // 1、如果注册标志为没成功，则注册之。
        // 2、如果注册标志为已成功，则跳过此段逻辑，继续做消息循环：popMessageThroughCommBus。
        if(!m_register_flag && !registerToCommBus((char*)host.c_str(), (char*)ins_name.c_str(), error))
        {
            //  如果标志为没成功，注册也没有成功
            Sleep(m_relinkInterval);  // 睡眠等待5秒，再重新注册
            continue;
        }

        // 注册标志没有成功，刚刚注册却成功了，改变注册标志，并更新注册状态显示
        if(m_register_flag == FALSE)
        {
            m_register_flag = TRUE;
            SetEvent(event_link_on);
        }

		memset(sender_ins, 0, 256);
		memset(message_type, 0, 256);
		memset(message_data, 0, max_commbusmessage_length);

		// 通过Commbus接口函数，接收消息，并把消息储存到临时消息数组message_data中
		if(!popMessageThroughCommBus((char*)host.c_str(), (char*)ins_name.c_str(), pszSenderIns, pszMessageType, pszMessageData, interval, error))
        {
            if(error == invalid_iid)
            {
                m_register_flag = FALSE;
                SetEvent(event_link_off);
            }
            continue;
        }

		// 创建“本次消息队列”
		poppedMessage* popped = new poppedMessage(sender_ins, message_type, message_data);

		//  把“本次取出的消息队列”popped，放到“主消息队列”popped_message中，等候处理
		popped_message.enterExplicitProtectedMode();
		popped_message.addtail(popped);
		popped_message.leaveExplicitProtectedMode();
	}

	return 0;
}

unsigned int _stdcall CCommBusClient::threadPop(void* p)
{
	return ((CCommBusClient*)p)->pop();
}

bool CCommBusClient::queryInstanceStatus(string dest_ins, string& dest_ip, bool& online_flag, time_t& last_heartbeat_time, unsigned int interval)
{
	int error = no_error;
	char ss[32] = {0}, *pss = ss;

	if(!queryInstanceStatusFromCommBus((char*)host.c_str(), (char*)ins_name.c_str(), (char*)dest_ins.c_str(), pss, online_flag, last_heartbeat_time, interval, error))
		return false;

	dest_ip = ss;

	return true;
}

