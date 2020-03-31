//wave_out_sim.cpp
//建立一个波形发生器，启动后，可通过对一个波形进行变换，同时输出1~n个波形数据。（用不同函数模拟，波的峰值范围从0到1。）
//为简化设计，用一个三角函数产生数据，并将产生的数据复制n次
#include "stdafx.h"
#include "usbdac_sim.h"

using namespace std;
//定义一个波函数需要的参数结构体
struct waveSet
{
	double scale;//峰值缩放因数
	double x;//参数初值组
	double step;//参数组的步进值
	double frequency;//输出频率
	stringbuf *buf;//流缓冲指针
	int bufsize;//缓冲区大小


};

void waveSine(void *wavSet)
{
	
	waveSet *wset = (waveSet *)wavSet;
	double x = wset->x;
	double scale = wset->scale;
	double step = wset->step;
	double freq=wset->frequency;
	stringbuf *sbuf=wset->buf;
	int sizeofInt = sizeof(int);
	int inttem;
	basic_string<char> res;
	while(1)
	{
		inttem = (int)sin(x)*scale;//根据需要转为int或其他
	//	res =(char *)&inttem;
		sbuf->sputn((char *)&inttem,sizeofInt);
		x+=step;
		res = sbuf->str();
		Sleep(1000/freq);//这里是个bug，Sleep需要DWORD
	}
	
}

int wave(char *cbuf,int bufsize)
{
	double x=0;
	double step =0.1;



	waveSet wSet;
	wSet.scale=WAV_SCALE;
	wSet.x=0;
	wSet.step=0.1;
	wSet.frequency=1000;
	//char cbuf[32];
	//wSet.buf=cbuf;

	int waveThreadID = WAVE_THREAD_ID;

	
	stringbuf sbuf;
	sbuf.str(cbuf);
	//sbuf.pubsetbuf(cbuf,bufsize);//将缓冲数组传入stringbuf
	wSet.buf=&sbuf;//设定waveSet的stringbuf
	//streamsize i = sbuf.in_avail();
	//char sen []={65,66,67,68};
	//sbuf.sputn(sen,10);
	//sbuf.str();
	_beginthread(waveSine,0,&wSet);
	while(1);
	return 0;
}