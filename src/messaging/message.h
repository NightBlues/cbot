#pragma once
#define HEADER_SIZE 8;

void * message_decode(char * msg, int len);
int message_encode(void * msg, char ** result, int * len);
int message_cksum(char * data, int len);
int message_read(int sock, void ** result);
int message_send(void * msg);

