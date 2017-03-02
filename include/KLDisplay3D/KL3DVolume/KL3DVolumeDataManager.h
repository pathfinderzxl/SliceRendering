#pragma once
#ifndef __KL3DVOLUME_KL3DVOLUMEDATAMANAGER_H__
#define __KL3DVOLUME_KL3DVOLUMEDATAMANAGER_H__

#include "boost/function.hpp"
#include "KL3DVolumeMacro.h"
#include "osg/Group"
#include "osg/Vec3d"
#include "osg/StateSet"
#include "osgViewer/Viewer"
#include "osg/NodeCallback"
#include "osg/NodeVisitor"
#include "osgUtil/CullVisitor"

#include <osg/Texture3D>

#include <string>
#include <list>
#include <map>

#include <osg/PolygonMode>

#include "KL3DDataModel/KL3DVOctree.h"

#include "KL3DVolumeBlockCache.h"
#include "KL3DDataModel/KL3DVTestSgyDataAdapter.h"
#include "KL3DDataModel/KL3DVolumeDataAdapter.h"
#include "KL3DDisplayList.h"


using namespace std;
BEGIN_KLDISPLAY3D_NAMESPACE



class KL3DVOLUME_EXPORT KL3DVolumeDataManager
{
public:
	KL3DVolumeDataManager();
protected:
	~KL3DVolumeDataManager();
public:
	/*
	\ brief  ����������������ȡ�����ڴ��ַ������cache
	\ return 
	\ param  	
	*/
	bool initData(KL3DVolumeDataAdapter* dataAdapter);
	
   /*!
	\brief ���������ṩ�����ݻ�����Ϣ
	*/
	KL3DVOctree* getOctree()
	{
		return m_octree;
	}
	KL3DVolumeBlockCache *getCache()
	{
		return m_cache;
	}
	void setCache(KL3DVolumeBlockCache *cache)
	{
		m_cache = cache;
	}
	int getCacheSize()
	{
		return m_cacheSize;
	}
	/*
	\ brief  ����һ�������ݹ�����֮���Ҫ����Ĵ�С�����涨��
	\ brief  ������һ��Ҫ�ڳ�ʼ������֮ǰ����Ч
	*/
	void setCacheSize(int size)
	{
		m_cacheSize = size;
	}
	void setDataAdapter(KL3DVolumeDataAdapter* dataAdapter)
	{
		m_dataAdapter = dataAdapter;
	}
	KL3DVolumeDataAdapter* getDataAdapter()
	{
		return m_dataAdapter;
	}
	void setVolumeData(float* data)
	{
		m_volumeData = data;
	}
	float* getVolumeData()
	{
		return m_volumeData;
	}
	/*
	\ brief  �����Ƿ����仯��Ⲣ�������ݷ����ı�����¶�����
	\ return 
	\ param  	
	*/
	bool OnDataChanged();
	/*
	\ brief  ���ݸ����İ˲�����cache�Ŀ��������µ�cache
	\ return 
	\ param  	
	*/
	void buildCache(KL3DVOctree *octree,int cachesize);
	/*
	\ brief  �ͷ�cacheռ�еĿռ�,������ΪNULL
	\ return 
	\ param  	
	*/
	//ͨ�����������ڴ��ַ�Ͱ˲����ļ��ıȽϣ�����Ƿ����ݷ����仯
	bool dataHasChanged();
	//�����еİ˲�������Ϊ��
	void resetOctree();

	void resetCache();

	//�ͷ��ڴ��ַ��������ΪNULL
	void resetVolumeData();

	/*!
	\brief Ϊ�������״̬,������ȡ������ݡ�������ά����,����Ӧ�˲������
	\	   ���ڼ���С���ݵ��������ɡ�dataָ��Ϊ�ڴ�ָ�����
	\	   ������ʵ����������ת�л�Ҳ���ڱ�������ʵ��
	\param  dim[3]��ԭʼ����ά�ȵĲ�������
	*/
	osg::ref_ptr<osg::StateSet> createState(int dim[3], float *data);
	/*!
	\brief��������
	*/
	osg::ref_ptr<osg::Texture3D> createTexture(int dim[3], float *data);

	osg::Texture3D* createTextureP(int dim[3], float *data);

	osg::ref_ptr<osg::Texture3D> createTexture(KLOctreeNode* node);

	osg::ref_ptr<osg::Texture3D> createTexture();

	KL3DDisplayList* getDisplayList(){return m_displayList;}
	void setDisplayList(KL3DDisplayList *displayList){m_displayList = displayList;}

	osg::ref_ptr<osg::Texture3D> getNodeTextureFromMap(KLOctreeNode* node);
	/*solve memory leak problem*/
	osg::ref_ptr<osg::Texture3D> getTextureFromMap(KLOctreeNode* node, list<KLOctreeNode*>& worksetList);

	osg::StateSet* getStateFromMap(KLOctreeNode* node);

	//TextureObjectCache* getTextureObjecCache(){return m_texObjectCache;}

	void setViewer(osgViewer::Viewer *pViewer){
		this->m_viewer = pViewer;
	}

	osgViewer::Viewer* getViewer(){return m_viewer;}
protected:
	//���ڼ�¼ǰһ��ʹ�õ�����
	float* m_preVolumeData;
	QString m_preOctreeFileName;

	//����״̬��Ϣ
	struct VolumeHeader m_volHeader;

	KL3DDisplayList *m_displayList;
	//��ǰ����������
	KL3DVolumeDataAdapter* m_dataAdapter;

	//�������ļ���Ӧ�İ˲���
	KL3DVOctree* m_octree;
	
	// ������ݵĻ���������
	KL3DVolumeBlockCache* m_cache;

	//cache�ֿ������
	int m_cacheSize;

	// С����ʱ��ֱ�ӽ�����д���ڴ�ĵ�ַ
	float *m_volumeData;
	
	std::map<KLOctreeNode*, osg::StateSet*> m_nodeStateMap;


	std::list<KLOctreeNode*> m_nodeTexInCache;

	std::map<KLOctreeNode *, osg::ref_ptr<osg::Texture3D>> m_nodeTextureMap;
	//std::map<KLOctreeNode *, osg::Texture3D*> m_nodeTextureMap;

	osgViewer::Viewer *m_viewer;
};


END_KLDISPLAY3D_NAMESPACE
#endif