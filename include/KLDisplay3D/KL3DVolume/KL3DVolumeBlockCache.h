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

// ���ݿ黺�棬��������������ݣ��ɶ��߳̽��й���
class KL3DVOLUME_EXPORT KL3DVolumeBlockCache
{
public:
	KL3DVolumeBlockCache();
	KL3DVolumeBlockCache(KL3DVOctree *octree);
	~KL3DVolumeBlockCache();

	/*!
	\brief �����ʼ��
	*/
	void initCache(KL3DVOctree *octree);

	/*!
	\brief ����������Ƿ��ڻ�����
	\note  ���Ҳ���������false
	\      ���ҵ������ý���map���Ƶ�β�����������ʹ�õĽ����map��β������ʵ��LRU����
	*/
	bool checkList(KLOctreeNode *node);

	/*!
	\brief �򻺴��м����µĽ��
	\note  �����뻺���ͬʱ ��Ҫͬʱ����map��list
	*/
	void insertList(KLOctreeNode *node);

	/*!
	\brief ��������ݶ��뻺�棬�������ݿ�ָ��
	*/
	float *readNodeData(KLOctreeNode *node);

	/*!
	\brief ������ʾ�б�
	\note  ��ʾ�б����֮�󣬿��������棬�̶߳Կ������в�������Ӱ��ԭʼ�б�ĸ���
	*/
	void setNodesIn(std::list<KLOctreeNode *> list);

	/*!
	\brief �ӻ����л�ȡ�������
	\note  ��map�в��ҽ���������ڻ���飬�ӻ�����п������ݣ����ؿ������ݿ�ָ��
	\      ����ֱ�ӷ��ػ����������ָ�룬��ֹ���ݵ���ʱ������ͻ------------����ֱ�ӷ���ָ�룬LRUʹ��ʹ��ʱ������ȸ���
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
	// ��������߳�
	class KL3DVolumeBlockCacheManageThread : public OpenThreads::Thread
	{
	public:

		friend class KL3DVolumeBlockCache;

		KL3DVolumeBlockCacheManageThread();
		KL3DVolumeBlockCacheManageThread(KL3DVolumeBlockCache* cache);
		~KL3DVolumeBlockCacheManageThread();
		
		/*!
		\brief �ͷ���
		*/
		void updateBlock()
		{
			m_updateBlock->release();
		}

		/*!
		\brief �߳�ִ��
		*/
		void run();

		/*!
		\brief ֹͣ�߳�
		*/
		int cancelThread();
		
		bool isCancel()
		{
			return m_isCancel;
		}

		/*!
		\brief ����Ԥ�����б�
		\param list ���µ���ʾ�б�
		\note  ������ʾ�б�ͨ��Ԥ�⣬�õ�Ԥ�����б����´ο��ܻ���ʾ�Ľ���б�
		\      �б������ǰ����ʾ�б��Լ���ʾ�б��и����ĸ������ӽ��
		*/
		void setNodesInTh(std::list<KLOctreeNode *> list);
		

	private:

		// �̶߳�Ӧ�Ļ���������������߳��뻺������һһ��Ӧ�ģ���һ�����������Ӧһ�������߳�
		KL3DVolumeBlockCache* m_cache;
		
		// �����������ڴ��̵߳�������ֱ��������stopBlock
		osg::RefBlock* m_updateBlock;
		
		// Ԥ�����б�������һ�ο�����ʾ�����������
		std::list<KLOctreeNode *> m_nodeListInThread;

		// �߳̿��Ʊ���
		volatile bool m_isCancel;
		volatile bool m_isRunning;

		// ��ʾ�б�ĸ��±�ʶ
		volatile int m_updateFlag;
		OpenThreads::Mutex m_ckMutexFlag;       // ��������m_updateListFlag
	};

private:

	friend class KL3DVolumeBlockCacheManageThread;
	//friend class TextureObjectCache;

	// ��������Ӧ�Ľ���б�ͽ���뻺�������ӳ��map
	std::list<KLOctreeNode *> m_nodeListInCache;
	std::map<KLOctreeNode *, float *> m_nodeCacheMap;

	// ����ÿ�θ��µõ����µ���ʾ�б�
	std::list<KLOctreeNode *> m_nodeListDisplay;

	// �ɵ���ʾ�б�
	std::list<KLOctreeNode *> m_oldListDisplay;

	// ��������̶߳���
	KL3DVolumeBlockCacheManageThread *m_thread;

	// �������������
	OpenThreads::Mutex m_opMutexInsert;

	// ������
	char *m_volBlockChunk;

	int m_blockNum;
	int m_blockSize;

	int *m_freeBlockList; // ���л�����б�
	
	// ���л�����б����Ч���α�־
	int m_freeBlockStartFlag;
	int m_freeBlockEndFlag;

	// �������ļ���
	QString m_dataFileName;
	//cache������
	int  m_cacheMaxSize;

	bool m_initialized;
};

END_KLDISPLAY3D_NAMESPACE

#endif
