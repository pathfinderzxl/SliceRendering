#include "KL3DVolumeBlockCache.h"

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DVolumeBlockCache::KL3DVolumeBlockCache()
#ifdef unix
	: m_opMutexInsert(OpenThreads::Mutex::MUTEX_RECURSIVE)
#endif
{
	m_blockSize = 64 * 64 * 64 * sizeof(float);
	m_blockNum = 300;
	m_volBlockChunk = new char[m_blockNum*m_blockSize];

	m_freeBlockFlag = 0;
	m_freeBlockList = new int[m_blockNum];

	for (int i = 0; i < m_blockNum; ++i)
	{
		m_freeBlockList[i] = i;
	}

	m_thread = new KL3DVolumeBlockCacheManageThread(this);
	m_thread->startThread();
}

KL3DVolumeBlockCache::KL3DVolumeBlockCache(Octree *octree)
#ifdef unix
	: m_opMutexInsert(OpenThreads::Mutex::MUTEX_RECURSIVE)
#endif
{
	m_blockSize = 64 * 64 * 64 * sizeof(float);
	m_blockNum = 800;
	m_volBlockChunk = new char[m_blockNum*m_blockSize];

	m_freeBlockFlag = 0;
	m_freeBlockList = new int[m_blockNum];

	for (int i = 0; i < m_blockNum; ++i)
	{
		m_freeBlockList[i] = i;
	}	

	m_dataFileName = octree->getOctreeFileName();

	// �򻺴�����Ԥ���ؽ��
	OctreeNode *root = octree->_rootNode;
	insertList(root);
	for (int i = 0; i < 8; ++i)
	{
		if (root->_children[i])
		{
			insertList(root->_children[i]);
		}
	}

	m_thread = new KL3DVolumeBlockCacheManageThread(this);
	m_thread->startThread();
}

KL3DVolumeBlockCache::~KL3DVolumeBlockCache()
{
	delete m_thread;
	m_thread = NULL;

	// ��ջ�������
	std::map<KLSeis::KLDisplay3D::OctreeNode *, float *>::iterator itrMap;
	for (itrMap = m_nodeCacheMap.begin(); itrMap != m_nodeCacheMap.end();)
	{
		if (itrMap->second != NULL)
		{
			delete itrMap->second;
			itrMap->second = NULL;
		}
		itrMap = m_nodeCacheMap.erase(itrMap);
	}

	delete m_volBlockChunk;
}

// ���ҽ���Ƿ��ڻ�����
bool KL3DVolumeBlockCache::checkList(OctreeNode *node)
{
	std::list<OctreeNode *>::iterator itrnode;
	itrnode = find(m_nodeListInCache.begin(), m_nodeListInCache.end(), node);
	if (itrnode == m_nodeListInCache.end())
	{
		return false;
	}
	else 
	{
		// ����ڻ����У�������Ƶ���β�� LRU
		m_nodeListInCache.erase(itrnode);
		m_nodeListInCache.push_back(node);
		return true;
	}
}

// �򻺴���ж���������
float *KL3DVolumeBlockCache::readNodeData(KLSeis::KLDisplay3D::OctreeNode *node)
{
	int size = node->_sampleNum * node->_CMPNum * node->_recordNum;
	// ����һ������������׵�ַ�������ڴ�ռ�
	float *data = new (m_volBlockChunk + m_freeBlockList[m_freeBlockFlag] * m_blockSize) float[size];
	m_freeBlockFlag += ceil((float)size * 4 / m_blockSize);
	m_freeBlockFlag %= m_blockNum;

	std::fstream fs;
	// �����ļ�����KL3DVolume�õ�
	fs.open(m_dataFileName, std::ios::binary | std::ios::in);
	std::ios::sync_with_stdio(false);
	fs.seekg(node->_location * 4);
	fs.read((char *)data, size * sizeof(float));
	fs.close();

	return data;
}

// ����µĽ������
void KL3DVolumeBlockCache::insertList(OctreeNode *node)
{
	float *data = NULL;
	//int begTime = clock();
	data = readNodeData(node);
	//int endTime = clock();	

	if (data)
	{
		// ���»����б�ͽ�㻺���ӳ��
		m_nodeListInCache.push_back(node);
		m_nodeCacheMap.insert(std::make_pair(node, data));
		//std::cout<<"cache Block "<<m_octreeNodeList.size()<<":"<<endTime - begTime<<"ms"<<std::endl;
	}
}

// ������ʾ�б�
void KL3DVolumeBlockCache::setNodesIn(std::list<OctreeNode *> list)
{
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_thread->m_ckMutexFlag);
		m_thread->m_updateFlag = 1; // ��ʶ��ʾ�б����
		m_nodeListDisplay = list;
	}
	
	// ��ʾ�б���£��߳��ͷ�������������Ԥ��������
	m_thread->updateBlock();

}

// �߳�
void KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::run()
{
	m_isRunning = true;
	int size = 0;

	std::map<OctreeNode *, float *>::iterator itrMap;
	
	// ������ʾ�б����̺߳��̵߳õ�Ԥ�����б�ͬʱ��������ʾ�б����
	// �����ʾ�б�δ�������£�������ʾ�б���Ϊ��
	// Ԥ�����б������ϣ��߳�������ֱ����ʾ�б����仯�������߳�
	do 
	{
		m_updateBlock->reset();
		
		{
			OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_ckMutexFlag);
			size = m_cache->m_nodeListDisplay.size();
		}
 
		
		if (size == 0)
		{
			// ��ʾ�б�δ���£������̣߳��ȴ��б����
			m_updateBlock->block();
		}
		else 
		{
			{
				OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_ckMutexFlag);
				
				// ��ʾ�б���£������µ�Ԥ�����б�
				setNodesInTh(m_cache->m_nodeListDisplay);
				
				// ����Ӧ�б����
				m_updateFlag = 0;

				// ��տ�����ʾ�б�
				std::list<OctreeNode *> tmpList;
				m_cache->m_nodeListDisplay.swap(tmpList);
			}

			// ����Ԥ�����б�
			while (m_nodeListInThread.size() > 0)
			{
				// ��ʾ�б���£�����Ԥ�����б�
				if (m_updateFlag)
				{
					// ��յ�ǰԤ�����б�
					std::list<OctreeNode *> tmpList;
					m_nodeListInThread.swap(tmpList);

					{
						OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_ckMutexFlag);
						// ����Ԥ�����б�
						setNodesInTh(m_cache->m_nodeListDisplay);
						m_updateFlag = 0;

						// ��տ�����ʾ�б�
						std::list<OctreeNode *> tmpList;
						m_cache->m_nodeListDisplay.swap(tmpList);
					}

					// ���±���Ԥ�����б�
					continue ;
				}
				else 
				{
					// ����ǰ��������Ԥ�����б���ȡ��
					KLSeis::KLDisplay3D::OctreeNode *node = m_nodeListInThread.front();
					m_nodeListInThread.pop_front();

					{
						OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_cache->m_opMutexInsert);
						// �鿴��ǰ��������Ƿ��Ѿ����뻺��
						if (!m_cache->checkList(node))
						{
							// �����м����µĽ��
							m_cache->insertList(node);							
						}
					}
				}
			}
		}

	} while (!isCancel());

	m_isRunning = false;
}

int KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::cancelThread()
{
	m_isCancel = true;
	
	// ���������߳�
	updateBlock();

	// �ȴ�����ִ�е��߳̽���
	while(m_isRunning)
	{
		OpenThreads::Thread::microSleep(10);
	}

	return 0;
}

// ������ʾ�б�����Ԥ�����б�
void KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::setNodesInTh(std::list<OctreeNode *> list)
{
	std::list<OctreeNode *> parentList;
	std::list<OctreeNode *>::iterator itrlist, itrparent;

	m_nodeListInThread = list;

	for (itrlist = list.begin(); itrlist != list.end(); ++itrlist)
	{
		// �Ǹ��ڵ㣬���丸�����븸����б�
		// �ȵ������ɸ�����б��ٽ���������Ԥ�����б��ɽ��͸������ص�ʱ�临�Ӷ�
		if ((*itrlist)->_level > 0)
		{
			OctreeNode *parentNode = (*itrlist)->_parent;
			// �����ظ���ӣ����������ͬһ�����
			itrparent = find(parentList.begin(), parentList.end(), parentNode);
			if (itrparent == parentList.end())
			{
				parentList.push_back(parentNode);
			}
		}

		// ��Ҷ�ӽ�㣬�����ӽ�����Ԥ�����б�
		// ÿ����Ҷ��㶼�л�����ͬ�ĺ��ӽ�㣬�����ظ����
		if ((*itrlist)->_notEmptyChild)
		{
			for (int i = 0; i < 8; ++i)
			{
				if ((*itrlist)->_children[i])
				{
					m_nodeListInThread.push_back((*itrlist)->_children[i]);
				}
			}
		}
	}

	// ��������б�Ԫ�ؼ���Ԥ�����б�
	for (itrparent = parentList.begin(); itrparent != parentList.end(); ++itrparent)
	{
		m_nodeListInThread.push_back(*itrparent);
	}

}

KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::KL3DVolumeBlockCacheManageThread()
#ifdef unix
	: m_ckMutexFlag(OpenThreads::Mutex::MUTEX_RECURSIVE)
#endif
{
	m_cache = NULL;
	m_updateBlock = new osg::RefBlock();
	m_isCancel = false;
	m_isRunning = false;
}

KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::KL3DVolumeBlockCacheManageThread(KL3DVolumeBlockCache *cache)
#ifdef unix
	: m_ckMutexFlag(OpenThreads::Mutex::MUTEX_RECURSIVE)
#endif
{
	m_cache = cache;
	m_updateBlock = new osg::RefBlock();
	m_isCancel = false;
	m_isRunning = false;
}

KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::~KL3DVolumeBlockCacheManageThread()
{
	// �����߳�
	cancelThread();
	delete m_updateBlock;
}

// �ӻ����ȡ�������
float *KL3DVolumeBlockCache::getNodeDataFromMap(OctreeNode *node)
{
	float *data = NULL;
	int size = node->_sampleNum * node->_CMPNum * node->_recordNum * sizeof(float);

	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_opMutexInsert);
		// �����鲻�ڻ����У������ݶ��뻺��
		if (!checkList(node))
			insertList(node);
		
		//data = new float [size];
		//// ��������
		//memcpy((char *)data, m_nodeCacheMap[node], size);
		data = m_nodeCacheMap[node];
	}
	return data;
}

END_KLDISPLAY3D_NAMESPACE
