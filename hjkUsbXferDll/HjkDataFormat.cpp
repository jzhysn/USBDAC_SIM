#include "pch.h"
#include "HjkDataFormat.h"
#include <stdlib.h>





HjkDataFormat::HjkDataFormat()
{
	isFrist = true;
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
		//printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}

	return size_x;
}

UINT HjkDataFormat::waveDataFormat(qnodeChInt32 * inNode, qnodeChInt32 * midNode, UINT index)
{
	//处理inNode中的数据
	//转为可为DAC使用的数据，
	//处理后数据的长度为size_x,存在tem中

	UINT size_x = inNode->datalen *4;
	__int32 *tem = inNode->data;
	if (isFrist)
	{
		oddGroup = (__int32 *)malloc((inNode->datalen / 2) * sizeof(__int32));//设为0
		evenGroup = (__int32 *)malloc((inNode->datalen / 2) * sizeof(__int32));//设为0
		isFrist = false;
	}
	
	{
		//处理过程
		//将数据传入并分奇偶两组
		for (UCHAR i = 0; i < inNode->datalen; i++)
		{
			if (i % 2 == 0)
				oddGroup[i / 2] = inNode->data[i];
			else
				evenGroup[i / 2] = inNode->data[i];
		}

		//转置
		transpositionInt(oddGroup, tem, DAC_BITS);
		transpositionInt(evenGroup, tem + (inNode->datalen / 2), DAC_BITS);
	}


	//处理后检测index，不能超过midNode的datalen
	if (index + size_x <= (midNode->datalen*4) )
		//将处理结果写入从index位置开始的midNode中
		memcpy(midNode->data + (index/4), tem, size_x);
	else
	{
		//printf("SERROR:out of the len of midNode's data.\n");
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
		//printf("SERROR:out of the len of midNode's data.\n");
		return NULL;
	}

	return size_x;
}
//转置方法
	//参数分别为输入数组、输出数组、有效位数（DAC位数,本工程为24bit）
	//函数为固定的32位转置，
UINT HjkDataFormat::transpositionInt(__int32 *in, __int32 *out, char bitWide)
{
	unsigned __int32 padBit = 0x01 << (bitWide - 1);//有效位的最高位取1
	for (int i = 0; i < 32; i++)//32位无符号整型的转置输出，即32*4字节。
	{
		out[i] = 0;
		//第i个整型数的输出是32个数左移i位后（如都移至最高位）与某位相与后，再依次向右移动0-31位并相或的结果。即out_1 = data_1_bit&data_2_biit&...data_32_bit.其中，bit是某位代指。
		for (int j = 0; j < 32; j++)
		{
			out[i] |= (((in[j] << i) & padBit) >> j);

		}
	}
	return 0;

}