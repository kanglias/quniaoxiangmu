#define no_error (-1)
#define invalid_iid (-2)
#define comm_error (-3)
#define unknown_exception (-4)
#define no_priviledge (-5)
#define time_out (-6)
#define invalid_message (-7)
#define already_exist (-8)

#define max_commbusmessage_length 8192

bool registerToCommBus(char* host, char* ins_name, int& error);
bool cancelFromCommBus(char* host, char* ins_name, int& error);
bool pushMessageThroughCommBus(char* host, char* ins_name, char* dest_ins, char* message_type, char* message_data, int& error);
bool popMessageThroughCommBus(char* host, char* ins_name, char*& sender_ins, char*& message_type, char*& message_data, unsigned int interval, int& error);
bool registerToLocalCommBus(char*, int&);
bool cancelFromLocalCommBus(char* ins_name, int& error);
bool pushMessageThroughLocalCommBus(char* ins_name, char* dest_ins, char* message_type, char* message_data, int& error);
bool popMessageThroughLocalCommBus(char* ins_name, char*& sender_ins, char*& message_type, char*& message_data, unsigned int interval, int& error);
bool lookForFirstCommBus(char*& commbus_ip);
bool queryInstanceStatusFromCommBus(char* host, char* ins_name, char* dest_ins, char*& dest_ip, bool& online_flag, time_t& last_heartbeat_time, unsigned int interval, int& error);
bool queryInstanceStatusFromLocalCommBus(char* ins_name, char* dest_ins, char*& dest_ip, bool& online_flag, time_t& last_heartbeat_time, unsigned int interval, int& error);
