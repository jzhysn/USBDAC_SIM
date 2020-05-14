#pragma once
#ifndef __HJKDATADEFINE_
#define __HJKDATADEFINE_

using namespace std;

typedef unsigned int UINT ;
typedef unsigned char UCHAR;
typedef UINT dacData;
//存放输入数据的缓存结构
struct bufSource
{
	dacData *dac_data_buf;
	int buf_len;
	int next_pos;
	int read_pos;

};
//输出数据的节点结构
typedef struct _QUEUE_NODE_
{
	UINT datalen;
	UCHAR *data;
} QUEUE_NODE;
//一个节点可以传送ch*n个32位int型数据
typedef struct _QUEUE_NODE_INT32_ARR_
{
	__int32 **data;
	int		ch;//通道数
	int     n;//每通道一次采样数	
} qnodeInt32s;
typedef struct NODE_INT32_ARR_
{
	UINT		datalen;//通道数
	__int32		*data;
	OVERLAPPED	ovLap;
	UCHAR		*context;
} qnodeChInt32;
typedef struct _QUEUE_NODE_A_
{
	UINT datalen=16;
	UCHAR data[16];
} QUEUE_NODE_A;
typedef struct _QUEUE_NODE_B_
{
	UINT datalen = 80;
	UCHAR data[80];
} QUEUE_NODE_B;
class HjkQueueElement
{
public:
	UINT datalen;
	dacData *data;
	HjkQueueElement(UINT datalen):
		datalen(datalen)
	{

	}
	~HjkQueueElement();
};
class HjkDataDefine
{

};
#endif
