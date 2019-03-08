
#include ".\CommBusDefinition.h"

CCommBusMessage::CCommBusMessage()
{
	message_id = genarateGUID();
}

CCommBusMessage::CCommBusMessage(string id)
{
	message_id = id;
}

CCommBusMessage::CCommBusMessage(string rn, string sn, string mt)
{
	receiver_name = rn;
	sender_name = sn;
	message_type = mt;
	message_id = genarateGUID();
}

CCommBusMessage::CCommBusMessage(string rn, string sn, string mt, string id)
{
	receiver_name = rn;
	sender_name = sn;
	message_type = mt;
	message_id = id;
}

CCommBusMessage::CCommBusMessage(const char* bytes)
{
	fillFromBytesStream(bytes);
}

CCommBusMessage::~CCommBusMessage()
{
}

void CCommBusMessage::operator=(CCommBusMessage& msg)
{
	receiver_name = msg.getReceiverName();
	sender_name = msg.getSenderName();
	message_type = msg.getMessageType();
	message_id = msg.getMessageID();

	message_data = msg.getMessageDataString();
}

string CCommBusMessage::getMessageDataString()
{
	return message_data;
}

void CCommBusMessage::setMessageDataString(string data_string)
{
	message_data = data_string;
}

void CCommBusMessage::fillFromBytesStream(const char* bytes)
{
	if(!isBytesStreamValid(bytes))
		return;

	unsigned short length = sizeofStandardBytesStream(bytes) + sizeofCustomBytesStream(bytes);
	if(length < sizeof_commbusmessage_fixed)
		return;

	char buf[max_commbusmessage_length];
	memset(buf, 0, max_commbusmessage_length);
	memcpy(buf, bytes + sizeof_commbusmessage_fixed, sizeofStandardBytesStream(bytes));

	JsonObject data;
	parseJsonString((const char*)buf, data);

	receiver_name = getJsonPairValue(data, "receiverName");
	eraseJsonPair(data, "receiverName");

	sender_name = getJsonPairValue(data, "senderName");
	eraseJsonPair(data, "senderName");

	message_id = getJsonPairValue(data, "messageID");
	eraseJsonPair(data, "messageID");

	message_type = getJsonPairValue(data, "messageType");
	eraseJsonPair(data, "messageType");

	memset(buf, 0, max_commbusmessage_length);
	memcpy(buf, bytes + sizeof_commbusmessage_fixed + sizeofStandardBytesStream(bytes), sizeofCustomBytesStream(bytes));

	message_data = buf;
}

void CCommBusMessage::outputToBytesStream(char* bytes, unsigned short& length)
{
	if(bytes == NULL)
		return;

	JsonObject data;
	data.push_back(JsonPair("receiverName", receiver_name));
	data.push_back(JsonPair("senderName", sender_name));
	data.push_back(JsonPair("messageID", message_id));
	data.push_back(JsonPair("messageType", message_type));

	string json_string;
	createJsonString(data, json_string);

	*bytes = 0xA;
	*(bytes + 1) = 0xB;
	*(bytes + 2) = 0xC;
	*(bytes + 3) = 0xD;
	*(unsigned short*)(bytes + sizeof_commbusmessage_header) = json_string.length();
	*(unsigned short*)(bytes + sizeof_commbusmessage_header + sizeof(unsigned short)) = message_data.length();

	strncpy(bytes + sizeof_commbusmessage_fixed, json_string.c_str(), json_string.length());
	strncpy(bytes + sizeof_commbusmessage_fixed + json_string.length(), message_data.c_str(), message_data.length());

	length = sizeof_commbusmessage_fixed + json_string.length() + message_data.length();
}
