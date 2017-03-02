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

	// 向缓存区中预加载结点
	KLOctreeNode *root = octree->m_rootNode;
	insertList(root); // 第0层根结点

	// 第1层子结点
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

	// 向缓存区中预加载结点
	KLOctreeNode *root = octree->m_rootNode;
	insertList(root); // 第0层根结点

	// 第1层子结点
	for (int i = 0; i < 8; ++i)
	{
		if (root->m_children[i])
		{
			insertList(root->m_children[i]);
		}
	}

	// 第2层子结点
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

	// 清空缓存区块
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

// 查找结点是否在缓存中
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
		// 结点在缓存中，将结点移到队尾， LRU
		m_nodeListInCache.erase(itrnode);
		m_nodeListInCache.push_back(node);
		return true;
	}
}

// 向缓存块中读入结点数据
float *KL3DVolumeBlockCache::readNodeData(KLSeis::KLDisplay3D::KLOctreeNode *node)
{
	//int size = node->m_sampleNum * node->m_CMPNum * node->m_recordNum;
	int size = 64*64*64;
	// 在下一个空闲区块的首地址处开辟内存空间
 	float *data = new (m_volBlockChunk + m_freeBlockList[m_freeBlockStartFlag] * m_blockSize) float[size];
	m_freeBlockStartFlag += ceil((float)size * 4 / m_blockSize);
	m_freeBlockStartFlag %= m_blockNum;

	std::fstream fs;
	// 数据文件名从KL3DVolume得到
	fs.open(m_dataFileName.toStdString(), std::ios::binary | std::ios::in);
	std::ios::sync_with_stdio(false);

	fs.seekg(node->m_location * 4);
	fs.read((char *)data, size * sizeof(float));

	//fs.seekg(40);
	//fs.read((char *)data, size * sizeof(float));
	//int m = node->m_recordNum;
	//int n = node->m_CMPNum;
	//int p = node->m_sampleNum;
	////存放node->m_parent块
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

// 添加新的结点数据
void KL3DVolumeBlockCache::insertList(KLOctreeNode *node)
{
	// 首先判断是否有空的缓存区块
	// 缓存区满，清除缓存的1 / 4
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

				// 查找结点是否在预加载列表中
				itrNode = find(m_thread->m_nodeListInThread.begin(), m_thread->m_nodeListInThread.end(), tmpNode);

				// 结点不在预加载列表或结点数据未加载到缓存中
				if (tmpData == NULL || itrNode == m_thread->m_nodeListInThread.end())
				{
					break ;
				}

				// 结点从队首移到队尾，LRU
				m_nodeListInCache.erase(m_nodeListInCache.begin());
				m_nodeListInCache.push_back(tmpNode);
				tmpData = NULL;
				--j;
			}

			if (j <= 0)
			{
				break ;
			}

			// 缓存区块回收
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
		// 更新缓存列表和结点缓存块映射
		m_nodeListInCache.push_back(node);
		m_nodeCacheMap.insert(std::make_pair(node, data));
	}
}

// 拷贝显示列表
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
		// 显示列表更新，线程释放阻塞锁，启动预加载数据
		m_thread->m_updateFlag = 1; // 标识显示列表更新
		m_thread->updateBlock();
	}
}

// 线程
void KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::run()
{
	m_isRunning = true;
	int size = 0;

	std::map<KLOctreeNode *, float *>::iterator itrMap;
	
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
 
		
		//if (size == 0)
		if (!m_updateFlag)
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
				//std::list<KLOctreeNode *> tmpList;
				//m_cache->m_nodeListDisplay.swap(tmpList);

				// 对预加载列表预处理，删除已在缓存中的结点
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

			// 遍历预加载列表
			while (m_nodeListInThread.size() > 0)
			{
				// 显示列表更新，重置预加载列表
				if (m_updateFlag)
				{
					// 清空当前预加载列表
					std::list<KLOctreeNode *> tmpList;
					m_nodeListInThread.swap(tmpList);//连空间一起释放的好方法

					{
						OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(m_ckMutexFlag);
						// 重置预加载列表
						setNodesInTh(m_cache->m_nodeListDisplay);
						m_updateFlag = 0;

						// 清空拷贝显示列表
						//std::list<KLOctreeNode *> tmpList;
						//m_cache->m_nodeListDisplay.swap(tmpList);

						// 对预加载列表预处理，删除已在缓存中的结点
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

					// 重新遍历预加载列表
					continue ;
				}
				else 
				{
					// 将当前遍历结点从预加载列表中取出
					KLSeis::KLDisplay3D::KLOctreeNode *node = m_nodeListInThread.front();
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
void KL3DVolumeBlockCache::KL3DVolumeBlockCacheManageThread::setNodesInTh(std::list<KLOctreeNode *> list)
{
	std::list<KLOctreeNode *> parentList;
	std::list<KLOctreeNode *>::iterator itrlist, itrparent;

	m_nodeListInThread = list;

	for (itrlist = list.begin(); itrlist != list.end(); ++itrlist)
	{
		// 非根节点，将其父结点加入父结点列表
		// 先单独生成父结点列表，再将父结点加入预加载列表，可降低父结点查重的时间复杂度
		if ((*itrlist)->m_level > 0)
		{
			KLOctreeNode *parentNode = (*itrlist)->m_parent;
			// 避免重复添加，多个结点具有同一父结点
			itrparent = find(parentList.begin(), parentList.end(), parentNode);
			if (itrparent == parentList.end())
			{
				parentList.push_back(parentNode);
			}
		}

		// 非叶子结点，将孩子结点加入预加载列表(如果不可见就不对其进行预加载子节点)
		// 每个非叶结点都有互不相同的孩子结点，不会重复添加
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
			else // 预加载列表达到最大长度
			{
				return ;
			}
		}
	}


	m_nodeListInThread.insert(m_nodeListInThread.end(), parentList.begin(), parentList.end());
	// 将父结点列表元素加入预加载列表
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
	// 结束线程
	cancelThread();
	delete m_updateBlock;
}

// 从缓存获取体块数据
float *KL3DVolumeBlockCache::getNodeDataFromMap(KLOctreeNode *node)
{
	float *data = NULL;
	int size = node->m_sampleNum * node->m_CMPNum * node->m_recordNum * sizeof(float);

	

	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_opMutexInsert);
		osg::Timer *t = new osg::Timer;
		
		osg::Timer_t s = t->tick();
		// 如果体块不在缓存中，将数据读入缓存
		if (!checkList(node))
		{
			insertList(node);
		}
		data = m_nodeCacheMap[node];
	}
	return data;
}

END_KLDISPLAY3D_NAMESPACE
