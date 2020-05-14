#pragma once
#ifndef __HJKDATAFORMAT_
#define __HJKDATAFORMAT_
#include "HjkDataDefine.h"

#define CHANLES_NUM 64
#define DAC_BITS 32
using namespace std;

class HjkDataFormat
{
private:
	QUEUE_NODE * inNode;
	QUEUE_NODE * midNode;
	UINT index;
	UINT method;
	__int32 *oddGroup;
	__int32 *evenGroup;
	bool isFrist;
	
public:
	HjkDataFormat();
	HjkDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	
	HjkDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index);
	//~HjkDataFormat();
	//index 是midNode的有效字节索引，其值为字节数，即记录下一个写入位置,返回midNode的
	//处理inNode中的数据，将处理后的数据放入midNode的data中从index开始的内存中，返回处理后的数据大小.
	//注意：此处插入的数据不能超出midNode的范围！！（需要加强）
	 UINT waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index);
	 //
	 UINT waveDataFormat(qnodeChInt32 * inNode, qnodeChInt32 * midNode, UINT index);
	 UINT waveDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index);
private:
	UINT transpositionInt(__int32 *in, __int32 *out, char bitWide);

};

#endif // !