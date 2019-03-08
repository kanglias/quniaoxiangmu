
#pragma once

//int startDownloadTask(char* server, char* url, char* local_path, char* local_file_name);
int startDownloadTask(char* server, char* url, char* local_path, char* local_file_name, char* md5, int handler_count);
int stopDownloadTask(char* server, char* url);
int deleteDownloadTask(char* server, char* url);
long getTaskDownloadingPercent(char* server, char* url);

//int startUploadTask(char* url, char* local_filepath);
int startUploadTask(char* url, char* local_filepath, char* cookie_string  , int handler_count = 5);
int stopUploadTask(int tid);
int remoteUploadTask(int tid);
long getTaskUploadingPercent(int tid);

