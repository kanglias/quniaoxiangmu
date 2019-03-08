
#pragma once

#include "..\includes.h"
#include "..\commonused.h"

using namespace std;
using namespace commonused;

#define commbus_standard_port 27503

#define sizeof_commbusmessage_fixed 8
#define sizeof_commbusmessage_header 4
#define max_commbusmessage_length 8192

//0xA 0xB 0xC 0xD(4 bytes)
//sizeofStandardBytesStream(2 bytes) 
//sizeofCustomBytesStream(2 bytes) 
//{'receiverName':'iPresentContainer','senderName':'hobiContainer','messageID':'12923-12sa12-12ada','messageType':'pause adv'} 
//{'***':'***','***':'***','***':'***'......}

class CCommBusMessage
{
public:
	CCommBusMessage();
	CCommBusMessage(string id);
	CCommBusMessage(string rn, string sn, string mt);
	CCommBusMessage(string rn, string sn, string mt, string id);
	CCommBusMessage(const char* bytes);
	~CCommBusMessage();

	void operator=(CCommBusMessage& msg);

	string getReceiverName(){return receiver_name;};
	string getSenderName(){return sender_name;};
	string getMessageID(){return message_id;};
	string getMessageType(){return message_type;};
	
	void setMessageDataString(string data_string);
	string getMessageDataString();

	static bool isBytesStreamValid(const char* bytes){return bytes[0] == 0xA && bytes[1] == 0xB && bytes[2] == 0xC && bytes[3] == 0xD;};

	static unsigned short sizeofStandardBytesStream(const char* bytes)
	{
		if(bytes == NULL)
			return 0;

		return *(unsigned short*)(bytes + sizeof_commbusmessage_header);
	};

	static unsigned short sizeofCustomBytesStream(const char* bytes)
	{
		if(bytes == NULL)
			return 0;

		return *(unsigned short*)(bytes + sizeof_commbusmessage_header + sizeof(unsigned short));
	};

	static unsigned short sizeofBytesStream(const char* bytes){return sizeofStandardBytesStream(bytes) + sizeofCustomBytesStream(bytes);};
	
	void fillFromBytesStream(const char* bytes);
	void outputToBytesStream(char* bytes, unsigned short& length);

private:
	string receiver_name;
	string sender_name;
	string message_id;
	string message_type;

	string message_data;
};