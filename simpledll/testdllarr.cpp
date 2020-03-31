#include "pch.h"
#include "testdllarr.h"

void testBuf(int *in, int *out,int len)
{
	/*
	for (int i = 0; i <  len; i++)
	{
		out[i] = in[i] + 10;
	}
	*/
	memset(out, 0xff, len * 4);
}
//转置方法
	//参数分别为输入数组、输出数组、有效位数（DAC位数,本工程为24bit）
	//函数为固定的32位转置，后期改进
int transpositionInt(unsigned int in[32], unsigned int out[32], char bitWide)
{
	unsigned int padBit = 0x01 << (bitWide - 1);//有效位的最高位取1
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