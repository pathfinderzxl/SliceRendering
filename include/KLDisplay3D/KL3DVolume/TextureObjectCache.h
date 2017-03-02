#ifndef __TEXTUREOBJECTCACHE_H__
#define __TEXTUREOBJECTCACHE_H__

#include <list>
#include <map>
#include "KL3DVolumeMacro.h"
#include <OpenThreads/Thread>
#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>
#include <osgViewer/Viewer>
#include "TextureItem.h"
#include "KL3DDataModel/KLOctreeNode.h"


using namespace std;
BEGIN_KLDISPLAY3D_NAMESPACE

class KL3DVolumeDataManager;
class KL3DVOLUME_EXPORT TextureObjectCache
{
public:
	TextureObjectCache(void);
	TextureObjectCache(KL3DVolumeDataManager *dataManager);
	virtual ~TextureObjectCache(void);
	void setDataManager(KL3DVolumeDataManager *dataManager)
	{
		m_pDataManager = dataManager;
	}

	void setViewer(osgViewer::Viewer *pViewer)
	{
		m_viewer = pViewer;
		if (m_pTexObjManagerThread != NULL)
		{
			m_pTexObjManagerThread->setViewer(pViewer);
		}
	}

	/*
	\缓存区管理线程
	*/
	class TexObjCahceManageThread : public OpenThreads::Thread
	{
	public:
		friend class TextureObjectCache;
		TexObjCahceManageThread();
		~TexObjCahceManageThread();

		void blockThread()
		{
			m_pBlock->block();
		}
		void releaseThread()
		{
			m_pBlock->release();
			m_pBlock->reset();
		}

		void setNodeListInThread(list<KLOctreeNode*> nodelist);
		void setTexObjCache(TextureObjectCache *texObjCache){m_texObjCache = texObjCache;}
		void setDataManager(KL3DVolumeDataManager *dataManager){m_pDataManager = dataManager;}
		
		void run();
		void setViewer(osgViewer::Viewer *pViewer)
		{
			m_viewer = pViewer;
		}
		

	protected:
		void cancelThread();

		bool m_isCancel;
		TextureObjectCache *m_texObjCache;
		KL3DVolumeDataManager *m_pDataManager;
		bool m_updateFlag;
		list<KLOctreeNode*> m_nodeListInThread;
		osg::RefBlock* m_pBlock;
		OpenThreads::Mutex m_nodeListInThMutex;
		osgViewer::Viewer *m_viewer;
	};
	TexObjCahceManageThread *m_pTexObjManagerThread;
protected:
	friend class TexObjCahceManageThread;

	
	map<KLOctreeNode*, TextureItem*> m_textureMapInGPU;
	list<KLOctreeNode*> m_cacheNodeList;
	KL3DVolumeDataManager *m_pDataManager;
	osgViewer::Viewer *m_viewer;

};

END_KLDISPLAY3D_NAMESPACE
#endif

