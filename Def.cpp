#include <iostream>
#include <stdio.h>
#include "Def.h"


const rtsp_method s_method[RTSP_METHOD_MAX] = {
    {RTSP_OPTIONS, "OPTIONS"},
    {RTSP_DESCRIBE, "DESCRIBE"},
    {RTSP_SETUP, "SETUP"},
    {RTSP_PLAY, "PLAY"},
    {RTSP_PAUSE, "PAUSE"},
    {RTSP_TEARDOWN, "TEARDOWN"},
    {RTSP_SET_PARAMETER, "SET_PARAMETER"},
    {RTSP_GET_PARAMETER, "GET_PARAMETER"},
};
const rtsp_code s_code[15] = 
{
    {200, "OK"},
    {302, "Moved Temporarily"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{454, "Session Not Found"},
	{457, "Invalid Range"},
	{461, "Unsupported transport"},
	{500, "Internal Server Error"},
	{503, "Service Unavailable"},
	{505, "RTSP Version not supported"},
	{551, "Option not supported"},
	{0, NULL},
};
void print_data(unsigned char *buf, int len)
{
	for(int i = 0; i < len; i++)
	{
		if(i%16 == 0)
			printf("\n");
		printf("%02x ", buf[i]);
	}
	printf("\n");

	return;
}