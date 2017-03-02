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

	// 向缓存区中预加载结点
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

	// 清空缓存区块
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

// 查找结点是否在缓存中
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
		// 结点在缓存中，将结点移到队尾， LRU
		m_nodeListInCache.erase(itrnode);
		m_nodeListInCache.push_back(node);
		return true;
	}
}

// 向缓存块中读入结点数据
float *KL3DVolumeBlockCache::readNodeData(KLSeis::KLDisplay3D::OctreeNode *node)
{
	int size = node->_sampleNum * node->_CMPNum * node->_recordNum;
	// 在下一个空闲区块的首地址处开辟内存空间
	float *data = new (m_volBlockChunk + m_freeBlockList[m_freeBlockFlag] * m_blockSize) float[size];
	m_freeBlockFlag += ceil((float)size * 4 / m_blockSize);
	m_freeBlockFlag %= m_blockNum;

	std::fstream fs;
	// 数据文件名从KL3DVolume得到
	fs.open(m_dataFileName, std::ios::binary | std::ios::in);
	std::ios::sync_with_stdio(false);
	fs.seekg(node->_location * 4);
	fs.read((char *)data, size * sizeof(float));
	fs.close();

	return data;
}

// 添加新的结点数据
void KL3DVolumeBlockCache::insertList(OctreeNode *node)
{
	float *data = NULL;
	//int begTime = clock();
	data = readNodeData(node);
	//int endTime = clock();	

	if (data)
	{
		// 更新缓存列表和结点缓存块映射
		m_nodeListInCache.push_back(node);
		m_nodeCacheMap.insert(std::make_pair(node, data));
		//std::cout<<"cache Block "<<m_octreeNodeList.size()<<":"<<endTime - begTime<<"ms"<<std::endl;
	}
}

// 拷贝显示列表
void KL3DVolumeBlockCache::setNodesIn(std::list<OctreeNode *> list)
{
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_thread->m_ckMutexFlag);
		m_thread->m_updateFlag = 1; // 标识显示列表更新
		m_nodeListDisplay = list;
	}
	
	// 显示列表更新，线程释放阻塞锁，启动预加载数据
	m_thread->updateBlock();

}

// 线程
void KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::run()
{
	m_isRunning = true;
	int size = 0;

	std::map<OctreeNode *, float *>::iterator itrMap;
	
	// 拷贝显示列表传入线程后，线程得到预加载列表，同时将拷贝显示列表清空
	// 如果显示列表未发生更新，拷贝显示列表保持为空
	// 预加载列表加载完毕，线程阻塞，直到显示列表发生变化，唤醒线程
	do 
	{
		m_updateBlock->reset();
		
		{
			OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_ckMutexFlag);
			size = m_cache->m_nodeListDisplay.size();
		}
 
		
		if (size == 0)
		{
			// 显示列表未更新，阻塞线程，等待列表更新
			m_updateBlock->block();
		}
		else 
		{
			{
				OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_ckMutexFlag);
				
				// 显示列表更新，生成新的预加载列表
				setNodesInTh(m_cache->m_nodeListDisplay);
				
				// 已响应列表更新
				m_updateFlag = 0;

				// 清空拷贝显示列表
				std::list<OctreeNode *> tmpList;
				m_cache->m_nodeListDisplay.swap(tmpList);
			}

			// 遍历预加载列表
			while (m_nodeListInThread.size() > 0)
			{
				// 显示列表更新，重置预加载列表
				if (m_updateFlag)
				{
					// 清空当前预加载列表
					std::list<OctreeNode *> tmpList;
					m_nodeListInThread.swap(tmpList);

					{
						OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_ckMutexFlag);
						// 重置预加载列表
						setNodesInTh(m_cache->m_nodeListDisplay);
						m_updateFlag = 0;

						// 清空拷贝显示列表
						std::list<OctreeNode *> tmpList;
						m_cache->m_nodeListDisplay.swap(tmpList);
					}

					// 重新遍历预加载列表
					continue ;
				}
				else 
				{
					// 将当前遍历结点从预加载列表中取出
					KLSeis::KLDisplay3D::OctreeNode *node = m_nodeListInThread.front();
					m_nodeListInThread.pop_front();

					{
						OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_cache->m_opMutexInsert);
						// 查看当前结点数据是否已经读入缓存
						if (!m_cache->checkList(node))
						{
							// 缓存中加入新的结点
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
	
	// 唤醒阻塞线程
	updateBlock();

	// 等待正在执行的线程结束
	while(m_isRunning)
	{
		OpenThreads::Thread::microSleep(10);
	}

	return 0;
}

// 根据显示列表生成预加载列表
void KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::setNodesInTh(std::list<OctreeNode *> list)
{
	std::list<OctreeNode *> parentList;
	std::list<OctreeNode *>::iterator itrlist, itrparent;

	m_nodeListInThread = list;

	for (itrlist = list.begin(); itrlist != list.end(); ++itrlist)
	{
		// 非根节点，将其父结点加入父结点列表
		// 先单独生成父结点列表，再将父结点加入预加载列表，可降低父结点查重的时间复杂度
		if ((*itrlist)->_level > 0)
		{
			OctreeNode *parentNode = (*itrlist)->_parent;
			// 避免重复添加，多个结点具有同一父结点
			itrparent = find(parentList.begin(), parentList.end(), parentNode);
			if (itrparent == parentList.end())
			{
				parentList.push_back(parentNode);
			}
		}

		// 非叶子结点，将孩子结点加入预加载列表
		// 每个非叶结点都有互不相同的孩子结点，不会重复添加
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

	// 将父结点列表元素加入预加载列表
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
	// 结束线程
	cancelThread();
	delete m_updateBlock;
}

// 从缓存获取体块数据
float *KL3DVolumeBlockCache::getNodeDataFromMap(OctreeNode *node)
{
	float *data = NULL;
	int size = node->_sampleNum * node->_CMPNum * node->_recordNum * sizeof(float);

	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_opMutexInsert);
		// 如果体块不在缓存中，将数据读入缓存
		if (!checkList(node))
			insertList(node);
		
		//data = new float [size];
		//// 拷贝数据
		//memcpy((char *)data, m_nodeCacheMap[node], size);
		data = m_nodeCacheMap[node];
	}
	return data;
}

END_KLDISPLAY3D_NAMESPACE
