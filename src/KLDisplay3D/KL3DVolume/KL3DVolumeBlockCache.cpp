#include "KL3DVolumeBlockCache.h"
#include "KL3DVolumeNode.h"
#include <vector>
using namespace std;
BEGIN_KLDISPLAY3D_NAMESPACE

	KL3DVolumeBlockCache::KL3DVolumeBlockCache()
#ifdef unix
	: m_opMutexInsert(OpenThreads::Mutex::MUTEX_RECURSIVE)
#endif
{
	m_thread = NULL;
	m_cacheMaxSize = 0;
	m_freeBlockList = NULL;

	m_initialized = false;
}
void KL3DVolumeBlockCache::initCache(KL3DVOctree *octree)
{
	if(m_initialized) return;

	m_blockSize = BLOCKSIZE * BLOCKSIZE * BLOCKSIZE * sizeof(float);
	//m_blockNum = m_cacheMaxSize / (BLOCKSIZE /128);//64
	m_blockNum = m_cacheMaxSize ;
	m_volBlockChunk = new char[m_blockNum*m_blockSize];

	m_freeBlockStartFlag = 0;
	m_freeBlockEndFlag = m_blockNum;
	m_freeBlockList = new int[m_blockNum];

	for (int i = 0; i < m_blockNum; ++i)
	{
		m_freeBlockList[i] = i;
	}	

	m_dataFileName = octree->getOctreeFileName();

	// �򻺴�����Ԥ���ؽ��
	KLOctreeNode *root = octree->m_rootNode;
	insertList(root); // ��0������

	// ��1���ӽ��
	for (int i = 0; i < 8; ++i)
	{
		if (root->m_children[i])
		{
			insertList(root->m_children[i]);
		}
	}

	m_nodeListDisplay.push_back(root);
	m_oldListDisplay.push_back(root);

	m_thread = new KL3DVolumeBlockCacheManageThread(this);
	m_thread->startThread();

	m_initialized = true;
}

KL3DVolumeBlockCache::KL3DVolumeBlockCache(KL3DVOctree *octree)
#ifdef unix
	: m_opMutexInsert(OpenThreads::Mutex::MUTEX_RECURSIVE)
#endif
{
	//m_cacheSize = 300;
	m_blockSize = BLOCKSIZE * BLOCKSIZE * BLOCKSIZE * sizeof(float);
	m_blockNum = m_cacheMaxSize / (BLOCKSIZE / 64);
	m_volBlockChunk = new char[m_blockNum*m_blockSize];

	m_freeBlockStartFlag = 0;
	m_freeBlockEndFlag = m_blockNum;
	m_freeBlockList = new int[m_blockNum];

	for (int i = 0; i < m_blockNum; ++i)
	{
		m_freeBlockList[i] = i;
	}	

	m_dataFileName = octree->getOctreeFileName();

	// �򻺴�����Ԥ���ؽ��
	KLOctreeNode *root = octree->m_rootNode;
	insertList(root); // ��0������

	// ��1���ӽ��
	for (int i = 0; i < 8; ++i)
	{
		if (root->m_children[i])
		{
			insertList(root->m_children[i]);
		}
	}

	// ��2���ӽ��
	//for (int i = 0; i < 8; ++i)
	//{
	//	if (root->_children[i])
	//	{
	//		for (int j = 0; j < 8; ++j)
	//		{
	//			if (root->_children[i]->_children[j])
	//			{
	//				insertList(root->_children[i]->_children[j]);
	//			}
	//		}
	//	}
	//}

	m_nodeListDisplay.push_back(root);
	m_oldListDisplay.push_back(root);

	m_thread = new KL3DVolumeBlockCacheManageThread(this);
	m_thread->startThread();
}

KL3DVolumeBlockCache::~KL3DVolumeBlockCache()
{
	delete m_thread;
	m_thread = NULL;

	// ��ջ�������
	std::map<KLSeis::KLDisplay3D::KLOctreeNode *, float *>::iterator itrMap;
	for (itrMap = m_nodeCacheMap.begin(); itrMap != m_nodeCacheMap.end();)
	{
		if (itrMap->second != NULL)
		{
			/*delete itrMap->first;
			itrMap->first = NULL;*/
			itrMap->second = NULL;
		}
		itrMap = m_nodeCacheMap.erase(itrMap);
	}

	delete m_volBlockChunk;
}

// ���ҽ���Ƿ��ڻ�����
bool KL3DVolumeBlockCache::checkList(KLOctreeNode *node)
{
	std::list<KLOctreeNode *>::iterator itrnode;
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
float *KL3DVolumeBlockCache::readNodeData(KLSeis::KLDisplay3D::KLOctreeNode *node)
{
	//int size = node->m_sampleNum * node->m_CMPNum * node->m_recordNum;
	int size = 64*64*64;
	// ����һ������������׵�ַ�������ڴ�ռ�
 	float *data = new (m_volBlockChunk + m_freeBlockList[m_freeBlockStartFlag] * m_blockSize) float[size];
	m_freeBlockStartFlag += ceil((float)size * 4 / m_blockSize);
	m_freeBlockStartFlag %= m_blockNum;

	std::fstream fs;
	// �����ļ�����KL3DVolume�õ�
	fs.open(m_dataFileName.toStdString(), std::ios::binary | std::ios::in);
	std::ios::sync_with_stdio(false);

	fs.seekg(node->m_location * 4);
	fs.read((char *)data, size * sizeof(float));

	//fs.seekg(40);
	//fs.read((char *)data, size * sizeof(float));
	//int m = node->m_recordNum;
	//int n = node->m_CMPNum;
	//int p = node->m_sampleNum;
	////���node->m_parent��
	//vector<vector<vector<float>>> Vector(m,vector<vector<float>>(n,vector<float>(p)));
	//
	////float *data1 = new float[node->m_sampleNum*node->m_CMPNum*node->m_recordNum];
	//float*ttemp = data;
	//int i=0;
	//int r,t,s;
	//for (r = 0; r < node->m_recordNum; ++r)
	//{
	//	for (t = 0; t < node->m_CMPNum; ++t)
	//	{
	//		for (s = 0; s < node->m_sampleNum; ++s)
	//		{
	//			Vector[r][t][s]=*ttemp;
	//			ttemp++;
	//		}
	//	}
	//}



	fs.close();

	return data;
}

// ����µĽ������
void KL3DVolumeBlockCache::insertList(KLOctreeNode *node)
{
	// �����ж��Ƿ��пյĻ�������
	// ������������������1 / 4
	if (m_nodeListInCache.size() >= m_blockNum)
	{
		int num = m_blockNum / 4;

		for (int i = 0; i < num; ++i)
		{
			float *tmpData = NULL;
			std::map<KLOctreeNode *, float *>::iterator itrMap;
			std::list<KLOctreeNode *>::iterator itrNode;
			KLOctreeNode *tmpNode = NULL;
			int j;

			// 
			for (j = m_nodeListInCache.size(); j > 0;)
			{
				tmpNode = m_nodeListInCache.front();
				itrMap = m_nodeCacheMap.find(tmpNode);
				if (itrMap != m_nodeCacheMap.end())
				{
					tmpData = itrMap->second;
				}

				// ���ҽ���Ƿ���Ԥ�����б���
				itrNode = find(m_thread->m_nodeListInThread.begin(), m_thread->m_nodeListInThread.end(), tmpNode);

				// ��㲻��Ԥ�����б��������δ���ص�������
				if (tmpData == NULL || itrNode == m_thread->m_nodeListInThread.end())
				{
					break ;
				}

				// ���Ӷ����Ƶ���β��LRU
				m_nodeListInCache.erase(m_nodeListInCache.begin());
				m_nodeListInCache.push_back(tmpNode);
				tmpData = NULL;
				--j;
			}

			if (j <= 0)
			{
				break ;
			}

			// �����������
			int blockNum = ((char *)tmpData - m_volBlockChunk) / m_blockSize;
			m_freeBlockEndFlag = m_freeBlockEndFlag % m_blockNum;
			m_freeBlockList[m_freeBlockEndFlag++] = blockNum;

			//delete tmpData;
			tmpData = NULL;

			m_nodeListInCache.erase(m_nodeListInCache.begin());
			m_nodeCacheMap.erase(itrMap);
		}
	}

	float *data = NULL;
	data = readNodeData(node);

	if (data)
	{
		// ���»����б�ͽ�㻺���ӳ��
		m_nodeListInCache.push_back(node);
		m_nodeCacheMap.insert(std::make_pair(node, data));
	}
}

// ������ʾ�б�
void KL3DVolumeBlockCache::setNodesIn(std::list<KLOctreeNode *> list)
{
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_thread->m_ckMutexFlag);
		m_oldListDisplay = m_nodeListDisplay;
		m_nodeListDisplay = list;
	}
	
	bool flag = TRUE;

	if (m_nodeListDisplay.size() != m_oldListDisplay.size())
	{
		flag = FALSE;
	}
	else 
	{
		std::list<KLOctreeNode *>::iterator itr1, itr2;
		for (itr1 = m_nodeListDisplay.begin(), itr2 = m_oldListDisplay.begin(); itr1 != m_nodeListDisplay.end(); ++itr1, ++itr2)
		{
			if (*itr1 == *itr2)
			{
				continue ;
			}

			flag = FALSE;
			break ;
		}
	}

	if (!flag)
	{
		// ��ʾ�б���£��߳��ͷ�������������Ԥ��������
		m_thread->m_updateFlag = 1; // ��ʶ��ʾ�б����
		m_thread->updateBlock();
	}
}

// �߳�
void KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::run()
{
	m_isRunning = true;
	int size = 0;

	std::map<KLOctreeNode *, float *>::iterator itrMap;
	
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
 
		
		//if (size == 0)
		if (!m_updateFlag)
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
				//std::list<KLOctreeNode *> tmpList;
				//m_cache->m_nodeListDisplay.swap(tmpList);

				// ��Ԥ�����б�Ԥ����ɾ�����ڻ����еĽ��
				int size = m_nodeListInThread.size();
				for (int i = 0; i < size; ++i)
				{
					KLSeis::KLDisplay3D::KLOctreeNode *node = m_nodeListInThread.front();
					m_nodeListInThread.pop_front();

					if (!m_cache->checkList(node))
					{
						m_nodeListInThread.push_back(node);
					}
				}
			}//release Muxlock

			// ����Ԥ�����б�
			while (m_nodeListInThread.size() > 0)
			{
				// ��ʾ�б���£�����Ԥ�����б�
				if (m_updateFlag)
				{
					// ��յ�ǰԤ�����б�
					std::list<KLOctreeNode *> tmpList;
					m_nodeListInThread.swap(tmpList);//���ռ�һ���ͷŵĺ÷���

					{
						OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_ckMutexFlag);
						// ����Ԥ�����б�
						setNodesInTh(m_cache->m_nodeListDisplay);
						m_updateFlag = 0;

						// ��տ�����ʾ�б�
						//std::list<KLOctreeNode *> tmpList;
						//m_cache->m_nodeListDisplay.swap(tmpList);

						// ��Ԥ�����б�Ԥ����ɾ�����ڻ����еĽ��
						int size = m_nodeListInThread.size();
						for (int i = 0; i < size; ++i)
						{
							KLSeis::KLDisplay3D::KLOctreeNode *node = m_nodeListInThread.front();
							m_nodeListInThread.pop_front();

							if (!m_cache->checkList(node))
							{
								m_nodeListInThread.push_back(node);
							}
						}
					}

					// ���±���Ԥ�����б�
					continue ;
				}
				else 
				{
					// ����ǰ��������Ԥ�����б���ȡ��
					KLSeis::KLDisplay3D::KLOctreeNode *node = m_nodeListInThread.front();
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
void KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::setNodesInTh(std::list<KLOctreeNode *> list)
{
	std::list<KLOctreeNode *> parentList;
	std::list<KLOctreeNode *>::iterator itrlist, itrparent;

	m_nodeListInThread = list;

	for (itrlist = list.begin(); itrlist != list.end(); ++itrlist)
	{
		// �Ǹ��ڵ㣬���丸�����븸����б�
		// �ȵ������ɸ�����б��ٽ���������Ԥ�����б��ɽ��͸������ص�ʱ�临�Ӷ�
		if ((*itrlist)->m_level > 0)
		{
			KLOctreeNode *parentNode = (*itrlist)->m_parent;
			// �����ظ���ӣ����������ͬһ�����
			itrparent = find(parentList.begin(), parentList.end(), parentNode);
			if (itrparent == parentList.end())
			{
				parentList.push_back(parentNode);
			}
		}

		// ��Ҷ�ӽ�㣬�����ӽ�����Ԥ�����б�(������ɼ��Ͳ��������Ԥ�����ӽڵ�)
		// ÿ����Ҷ��㶼�л�����ͬ�ĺ��ӽ�㣬�����ظ����
		if ((*itrlist)->m_numChild && (*itrlist)->m_visible)
		{
			if (m_nodeListInThread.size() + (*itrlist)->m_numChild < m_cache->m_blockNum)
			{
				for (int i = 0; i < 8; ++i)
				{
					if ((*itrlist)->m_children[i])
					{
						m_nodeListInThread.push_back((*itrlist)->m_children[i]);
					}
				}
			}
			else // Ԥ�����б�ﵽ��󳤶�
			{
				return ;
			}
		}
	}


	m_nodeListInThread.insert(m_nodeListInThread.end(), parentList.begin(), parentList.end());
	// ��������б�Ԫ�ؼ���Ԥ�����б�
	//for (itrparent = parentList.begin(); itrparent != parentList.end(); ++itrparent)
	//{
	//	m_nodeListInThread.push_back(*itrparent);
	//}

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
float *KL3DVolumeBlockCache::getNodeDataFromMap(KLOctreeNode *node)
{
	float *data = NULL;
	int size = node->m_sampleNum * node->m_CMPNum * node->m_recordNum * sizeof(float);

	

	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_opMutexInsert);
		osg::Timer *t = new osg::Timer;
		
		osg::Timer_t s = t->tick();
		// �����鲻�ڻ����У������ݶ��뻺��
		if (!checkList(node))
		{
			insertList(node);
		}
		data = m_nodeCacheMap[node];
	}
	return data;
}

END_KLDISPLAY3D_NAMESPACE
