#ifndef  __BUF_QUEUE_H__
#define __BUF_QUEUE_H__
//#include <pthread.h>
/*********8备用数据帧缓冲队列类 2020.3.21******************/
//在操作队列时进行互斥和信号量的操作，以实现并发
//参考：https://blog.csdn.net/u012459903/article/details/103959872
/************若有并发操作，可考虑无锁队列*********/
//工程地址：https://github.com/xant/libhl
//或：https://github.com/craflin/LockFreeQueue


#include "usbdac_sim.h"

#define MAX_SEM_COUNT 10
#define THREADCOUNT 12
#define TIMEOUT_MS  5000
#define TIMEOUT_ZERO  0L

#define LOOP_ADD(i,r) ((i)+1)>=(r)?0:(i+1)
#define SIGLEFRAME_LEN (1024*125) //定义队列结点的大小，即一个缓冲块的字节数

typedef struct _QUEUE_NODE_
{
	HANDLE mutex;
	int datalen;
	UCHAR data[SIGLEFRAME_LEN];
} QUEUE_NODE;
//BufQueue类实现的是多线程读写操作队列，队列有最多maxNodes个节点，可实现出队、入队
class BufQueue
{
private:
	HANDLE mindex_mutex;
	int maxNodes;
	QUEUE_NODE *que;
	int mInindex;
	int mOutindex;
	int mEffectives;
	HANDLE mSem;

public :
	BufQueue(int frames);
	BufQueue(dacData *databuf);
	~BufQueue();
	/*  出队，删除栈顶元素。
	*\\参数：
	*\\返回：
	*/
	int pop(QUEUE_NODE *node);
	/*入队，向队尾插入元素。
	*/
	int push(QUEUE_NODE *node);

	int getbuffer();
	int releasebuffer();
private:
	void addInindex();
	void addOutindex();
	//
	int IncreaseEffectives();
	int reduceEffectives();
};

#endif