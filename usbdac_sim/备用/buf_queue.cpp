

#include "stdafx.h"
#include "buf_queue.h"

BufQueue::BufQueue(int nodes) :
	maxNodes(nodes),
	mInindex(0),
	mOutindex(0),
	mEffectives(0)
{
	//为队列分配整块空间
	que = (QUEUE_NODE *)malloc(nodes * sizeof(QUEUE_NODE));
	if (que == NULL)
	{
		printf("erro to malloc\n");
	}

	
	// 设置互斥锁
	//HANDLE *mmutex;
	
	int i = 0;
	for (i = 0; i++; i < maxNodes)
	{
		 que[i].mutex= CreateMutex(NULL,false,NULL);
	}
	//pthread_mutex_init(&mIndexlock, &mutextattr);
	mindex_mutex = CreateMutex(NULL, false, NULL);
	mSem = CreateSemaphore(NULL, MAX_SEM_COUNT, MAX_SEM_COUNT,NULL);//！！注意，设置初始信号数和最大信号数，此处默认设置
	if (mSem == NULL)
	{
		printf("CreateSemaphore error: %d\n", GetLastError());
		exit(1);
	}
}

BufQueue::~BufQueue()
{
	int i = 0;
	for (i = 0; i++; i < maxNodes)
	{
		CloseHandle(&que[i].mutex);
	}
	CloseHandle(mindex_mutex);
	CloseHandle(mSem);
	if (que != NULL)
	{
		free(que);
		que = NULL;
	}
}

int BufQueue::push(QUEUE_NODE *node)
{
	int iIn = mInindex;
	DWORD dwWaitResult;
	WaitForSingleObject(que[iIn].mutex, TIMEOUT_MS);
	if (node != NULL)
	{
		memcpy(que[iIn].data, node->data, node->datalen);
		//que[iIn].datalen = len;
		
	}
	addInindex();
	//信号量加1
	if(!ReleaseSemaphore(mSem, 1, NULL))
		printf("ReleaseSemaphore error: %d\n", GetLastError());
	if (IncreaseEffectives() == maxNodes)
	{//当队列满时，覆盖老数据
		//DEBUG("cover  mEffectives %d ,maxFrames %d\n", mEffectives, maxFrames);
		//DEBUG("cover  mInindex %d ,mOutindex %d\n",mInindex,mOutindex);
		//信号量减1
		dwWaitResult = WaitForSingleObject(
			mSem,   // handle to semaphore
			0L);           // zero-second time-out interval

		switch (dwWaitResult)
		{
			// The semaphore object was signaled.
		case WAIT_OBJECT_0:
			// TODO: Perform task
			//printf("Thread %d: wait succeeded\n", GetCurrentThreadId());
			
			//关键函数，使输出指针+1
			addOutindex();

			break;

			// The semaphore was nonsignaled, so a time-out occurred.
		case WAIT_TIMEOUT:
			printf("Thread %d: wait timed out\n", GetCurrentThreadId());
			break;
		}
	}
	

	ReleaseMutex(que[iIn].mutex);

	return 0;
}
int BufQueue::pop(QUEUE_NODE *node)
{
	DWORD dwWaitResult;
	//信号量减1
	dwWaitResult = WaitForSingleObject(mSem, 0L);

	switch (dwWaitResult)
	{
		// The semaphore object was signaled.
	case WAIT_OBJECT_0:
		// TODO: Perform task
		//printf("Thread %d: wait succeeded\n", GetCurrentThreadId());

		//关键过程
		int iOut = mOutindex;

		WaitForSingleObject(que[iOut].mutex, TIMEOUT_MS);

		if (reduceEffectives() == 0)
		{
			mEffectives = 0;
			ReleaseMutex(que[iOut].mutex);
			printf("no data\n");
			return 0;
		}
		//节点出队到data
		if (node != NULL)
		{
			memcpy(node, que[iOut].data, que[iOut].datalen);
		}
		addOutindex();
		ReleaseMutex(que[iOut].mutex);
		break;

		// The semaphore was nonsignaled, so a time-out occurred.
	case WAIT_TIMEOUT:
		printf("Thread %d: wait timed out\n", GetCurrentThreadId());
		break;
	}


	
	return 1;
}
void BufQueue::addInindex()
{
	WaitForSingleObject(mindex_mutex,TIMEOUT_MS);
	mInindex = LOOP_ADD(mInindex,maxNodes);
	ReleaseMutex(mindex_mutex);
}
void BufQueue::addOutindex()
{
	WaitForSingleObject(mindex_mutex, TIMEOUT_MS);
	mOutindex = LOOP_ADD(mOutindex, maxNodes);
	ReleaseMutex(mindex_mutex);
}
int BufQueue::IncreaseEffectives()
{
	WaitForSingleObject(mindex_mutex,TIMEOUT_MS);
	int  ret = mEffectives;
	mEffectives++;
	if (mEffectives > maxNodes)
		mEffectives = maxNodes;
	ReleaseMutex(mindex_mutex);
	return ret;
}
int BufQueue::reduceEffectives()
{
	WaitForSingleObject(mindex_mutex, TIMEOUT_MS);
	int  ret = mEffectives;
	mEffectives--;
	if (mEffectives <0)
		mEffectives = 0;
	ReleaseMutex(mindex_mutex);
	return ret;
}