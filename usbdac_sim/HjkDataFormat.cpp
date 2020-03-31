#include "stdafx.h"
#include "HjkDataFormat.h"



HjkDataFormat::HjkDataFormat(UINT method):
	method(method)
{
}

UINT HjkDataFormat::waveDataFormat(QUEUE_NODE * inNode, QUEUE_NODE * midNode, UINT index)
{
	
	//处理inNode中的数据
	//转为可为DAC使用的数据，
	//处理后数据的长度为size_x,存在tem中

	UINT size_x = inNode->datalen;
	UCHAR *tem = inNode->data;

	//处理过程


	//处理后检测index，不能超过midNode的datalen
	if(index+size_x <= midNode->datalen )
	//将处理结果写入从index位置开始的midNode中
	(UCHAR*)memcpy(midNode->data + index, tem, size_x);
	else
	{
		printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}

	return size_x;
}

UINT HjkDataFormat::waveDataFormat(QUEUE_NODE_A * inNode, QUEUE_NODE_B * midNode, UINT index)
{
	UINT size_x = inNode->datalen;
	UCHAR *tem = inNode->data;

	//处理过程


	//处理后检测index，不能超过midNode的datalen
	if (index + size_x <= midNode->datalen)
		//将处理结果写入从index位置开始的midNode中
		(UCHAR*)memcpy(midNode->data + index, tem, size_x);
	else
	{
		printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}

	return size_x;
}
