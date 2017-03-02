#ifndef __KL3DVOLUME_KL3DVOLUMEBLOCKCACHE_H__
#define __KL3DVOLUME_KL3DVOLUMEBLOCKCACHE_H__

#include "KL3DVolumeMacro.h"
//#include "TextureObjectCache.h"


#include "OpenThreads/Thread"
#include "OpenThreads/Mutex"
#include "OpenThreads/ScopedLock"
#include "osg/OperationThread"

#include "KL3DDataModel/KL3DVOctree.h"


#include <iostream>
#include <list>
#include <map>
#include <algorithm>
#include <fstream>

BEGIN_KLDISPLAY3D_NAMESPACE

//#define MAXBLOCKNUM 300

// 数据块缓存，用来缓存体块数据，由多线程进行管理
class KL3DVOLUME_EXPORT KL3DVolumeBlockCache
{
public:
	KL3DVolumeBlockCache();
	KL3DVolumeBlockCache(KL3DVOctree *octree);
	~KL3DVolumeBlockCache();

	/*!
	\brief 缓存初始化
	*/
	void initCache(KL3DVOctree *octree);

	/*!
	\brief 检查结点数据是否在缓存中
	\note  查找不到，返回false
	\      查找到，将该结点从map中移到尾部，保持最近使用的结点在map的尾部，以实现LRU调度
	*/
	bool checkList(KLOctreeNode *node);

	/*!
	\brief 向缓存中加入新的结点
	\note  结点加入缓存的同时 需要同时更新map和list
	*/
	void insertList(KLOctreeNode *node);

	/*!
	\brief 将结点数据读入缓存，返回数据块指针
	*/
	float *readNodeData(KLOctreeNode *node);

	/*!
	\brief 拷贝显示列表
	\note  显示列表更新之后，拷贝给缓存，线程对拷贝进行操作，不影响原始列表的更新
	*/
	void setNodesIn(std::list<KLOctreeNode *> list);

	/*!
	\brief 从缓存中获取结点数据
	\note  从map中查找结点数据所在缓存块，从缓存块中拷贝数据，返回拷贝数据块指针
	\      不能直接返回缓存块中数据指针，防止数据调度时发生冲突------------可以直接返回指针，LRU使块使用时不会调度覆盖
	*/
	float *getNodeDataFromMap(KLOctreeNode *node);

	void setFileName(QString fileName) {m_dataFileName = fileName;}
	
	void setCacheSize(int size)
	{
		m_cacheMaxSize = size;
	}

	int getCacheSize()
	{
		return m_cacheMaxSize;
	}
public:
	// 缓存管理线程
	class KL3DVolumeBlockCacheManageThread : public OpenThreads::Thread
	{
	public:

		friend class KL3DVolumeBlockCache;

		KL3DVolumeBlockCacheManageThread();
		KL3DVolumeBlockCacheManageThread(KL3DVolumeBlockCache* cache);
		~KL3DVolumeBlockCacheManageThread();
		
		/*!
		\brief 释放锁
		*/
		void updateBlock()
		{
			m_updateBlock->release();
		}

		/*!
		\brief 线程执行
		*/
		void run();

		/*!
		\brief 停止线程
		*/
		int cancelThread();
		
		bool isCancel()
		{
			return m_isCancel;
		}

		/*!
		\brief 更新预加载列表
		\param list 更新的显示列表
		\note  根据显示列表，通过预测，得到预加载列表，即下次可能会显示的结点列表
		\      列表包含当前的显示列表，以及显示列表中各结点的父结点和子结点
		*/
		void setNodesInTh(std::list<KLOctreeNode *> list);
		

	private:

		// 线程对应的缓存区，缓存管理线程与缓存区是一一对应的，即一个缓存区块对应一个管理线程
		KL3DVolumeBlockCache* m_cache;
		
		// 自旋锁，用于此线程的阻塞，直到被调用stopBlock
		osg::RefBlock* m_updateBlock;
		
		// 预加载列表，保存下一次可能显示的所有体块结点
		std::list<KLOctreeNode *> m_nodeListInThread;

		// 线程控制变量
		volatile bool m_isCancel;
		volatile bool m_isRunning;

		// 显示列表的更新标识
		volatile int m_updateFlag;
		OpenThreads::Mutex m_ckMutexFlag;       // 用来锁定m_updateListFlag
	};

private:

	friend class KL3DVolumeBlockCacheManageThread;
	//friend class TextureObjectCache;

	// 缓存区对应的结点列表和结点与缓存区块的映射map
	std::list<KLOctreeNode *> m_nodeListInCache;
	std::map<KLOctreeNode *, float *> m_nodeCacheMap;

	// 拷贝每次更新得到的新的显示列表
	std::list<KLOctreeNode *> m_nodeListDisplay;

	// 旧的显示列表
	std::list<KLOctreeNode *> m_oldListDisplay;

	// 缓存管理线程对象
	KL3DVolumeBlockCacheManageThread *m_thread;

	// 缓存操作互斥锁
	OpenThreads::Mutex m_opMutexInsert;

	// 缓存区
	char *m_volBlockChunk;

	int m_blockNum;
	int m_blockSize;

	int *m_freeBlockList; // 空闲缓存块列表
	
	// 空闲缓存块列表的有效区段标志
	int m_freeBlockStartFlag;
	int m_freeBlockEndFlag;

	// 体数据文件名
	QString m_dataFileName;
	//cache最大块数
	int  m_cacheMaxSize;

	bool m_initialized;
};

END_KLDISPLAY3D_NAMESPACE

#endif
