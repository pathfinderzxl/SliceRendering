#include "TextureObjectCache.h"
#include "KL3DVolumeDataManager.h"



BEGIN_KLDISPLAY3D_NAMESPACE
TextureObjectCache::TextureObjectCache(void)
{
	m_pTexObjManagerThread = new TextureObjectCache::TexObjCahceManageThread();
	m_pTexObjManagerThread->setDataManager(m_pDataManager);
	m_pTexObjManagerThread->setTexObjCache(this);
	m_pTexObjManagerThread->start();
}

TextureObjectCache::TextureObjectCache(KL3DVolumeDataManager *dataManager) : m_pDataManager(dataManager)
{
	m_pTexObjManagerThread = new TextureObjectCache::TexObjCahceManageThread();
	m_pTexObjManagerThread->setDataManager(m_pDataManager);
	m_pTexObjManagerThread->setTexObjCache(this);
	m_pTexObjManagerThread->start();
}




TextureObjectCache::~TextureObjectCache(void)
{
	if (m_pTexObjManagerThread != NULL)
	{
		delete m_pTexObjManagerThread;
		m_pTexObjManagerThread = NULL;
	}
}






TextureObjectCache::TexObjCahceManageThread::TexObjCahceManageThread()
{
	m_pBlock = new osg::RefBlock();
}
TextureObjectCache::TexObjCahceManageThread::~TexObjCahceManageThread()
{
	if (m_pBlock != NULL)
	{
		delete m_pBlock;
		m_pBlock = NULL;
	}
}

void TextureObjectCache::TexObjCahceManageThread::
			setNodeListInThread(list<KLOctreeNode*> nodelist)
{
	{
		OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_nodeListInThMutex);
		m_nodeListInThread.clear();
		for (list<KLOctreeNode*>::iterator itr = nodelist.begin(); 
			itr!=nodelist.end(); ++itr)
		{

			m_nodeListInThread.push_back(*itr);
		}
	}
	releaseThread();

}

void TextureObjectCache::TexObjCahceManageThread::run()
{
	int xx = 0;
	while (true)
	{
		blockThread();
		cout<<"========"<<xx++<<"=============\n";
		{
			OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_nodeListInThMutex);
			list<KLOctreeNode*>::iterator itr = m_nodeListInThread.begin();
			int x = 0;
			for (; itr!=m_nodeListInThread.end(); ++itr)
			{
				//有新的显示列表产生
				if (m_updateFlag)
				{
					setNodeListInThread(m_texObjCache->m_cacheNodeList);
					itr = m_nodeListInThread.begin();
					m_updateFlag = false;
					break;
				}
				else
				{
					KLOctreeNode *tmpNode;
					tmpNode = *itr;
					map<KLOctreeNode*, TextureItem*>::iterator mapItr;
					//查找m_textureMapInGPU中是否有该节点
					mapItr = m_texObjCache->m_textureMapInGPU.find(tmpNode);
					osg::ref_ptr<osg::Texture3D> tex3D = NULL;
					if (mapItr != m_texObjCache->m_textureMapInGPU.end())
					{
						TextureItem *texItem = mapItr->second;
						tex3D = texItem->m_pTexture;				
					}
					else
					{
						tex3D = m_pDataManager->getNodeTextureFromMap(tmpNode);
						m_texObjCache->m_textureMapInGPU
							.insert(make_pair(tmpNode, new TextureItem(tex3D, 0, 0, 0)));
					}

					const osg::Texture3D::Extensions *extensions 
						= osg::Texture3D::getExtensions(0, false);
					if (extensions == NULL)
					{
						continue;
					}
				/*	while (extensions==NULL)
					{
						extensions = osg::Texture3D::getExtensions(0, false);
					}*/
					/*const osg::Texture3D::Extensions *extensions = NULL;
					osg::Texture3D::getExtensions(0, extensions);*/
					osg::Texture::TextureObject* texObj
						= tex3D->getTextureObject(0);
					if (texObj != NULL)
					{
						++x;
					}
					//纹理未加载到显存
					if (texObj == NULL)
					{
						osg::ref_ptr<osg::Image> img3D = tex3D->getImage();

						/*if (m_viewer->getCamera()->getGraphicsContext())
						{
							tex3D->apply(*(m_viewer->getCamera()
								->getGraphicsContext()->getState()));
						}*/
				}
					m_texObjCache->m_textureMapInGPU[tmpNode]->m_iState = 1;
				}
			}
			if (m_nodeListInThread.size()>0)
			{
				double t = x;
				double percent = t/m_nodeListInThread.size();
				cout<<percent<<endl;
			}
			

		}

	}
	
}


void TextureObjectCache::TexObjCahceManageThread::cancelThread()
{
	m_isCancel = true;
	releaseThread();
}

END_KLDISPLAY3D_NAMESPACE