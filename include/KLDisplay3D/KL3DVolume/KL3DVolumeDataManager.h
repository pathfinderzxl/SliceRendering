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
	\ brief  根据数据适配器获取数据内存地址或生成cache
	\ return 
	\ param  	
	*/
	bool initData(KL3DVolumeDataAdapter* dataAdapter);
	
   /*!
	\brief 给绘制类提供体数据基本信息
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
	\ brief  构建一个体数据管理器之后就要对其的大小做出规定，
	\ brief  本函数一定要在初始化数据之前才有效
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
	\ brief  数据是否发生变化检测并处理，数据发生改变就重新读数据
	\ return 
	\ param  	
	*/
	bool OnDataChanged();
	/*
	\ brief  根据给定的八叉树和cache的块数生成新的cache
	\ return 
	\ param  	
	*/
	void buildCache(KL3DVOctree *octree,int cachesize);
	/*
	\ brief  释放cache占有的空间,将其置为NULL
	\ return 
	\ param  	
	*/
	//通过对适配器内存地址和八叉树文件的比较，检查是否数据发生变化
	bool dataHasChanged();
	//销毁有的八叉树，置为空
	void resetOctree();

	void resetCache();

	//释放内存地址，重新置为NULL
	void resetVolumeData();

	/*!
	\brief 为体块设置状态,包括读取体块数据、生成三维纹理,体块对应八叉树结点
	\	   现在兼容小数据的纹理生成。data指针为内存指针而已
	\	   还包括实现坐标轴旋转切换也都在本函数中实现
	\param  dim[3]是原始三个维度的采样点数
	*/
	osg::ref_ptr<osg::StateSet> createState(int dim[3], float *data);
	/*!
	\brief生成纹理
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
	//用于记录前一次使用的数据
	float* m_preVolumeData;
	QString m_preOctreeFileName;

	//数据状态信息
	struct VolumeHeader m_volHeader;

	KL3DDisplayList *m_displayList;
	//当前数据适配器
	KL3DVolumeDataAdapter* m_dataAdapter;

	//大数据文件对应的八叉树
	KL3DVOctree* m_octree;
	
	// 体块数据的缓存管理对象
	KL3DVolumeBlockCache* m_cache;

	//cache分块的数量
	int m_cacheSize;

	// 小数据时，直接将数据写入内存的地址
	float *m_volumeData;
	
	std::map<KLOctreeNode*, osg::StateSet*> m_nodeStateMap;


	std::list<KLOctreeNode*> m_nodeTexInCache;

	std::map<KLOctreeNode *, osg::ref_ptr<osg::Texture3D>> m_nodeTextureMap;
	//std::map<KLOctreeNode *, osg::Texture3D*> m_nodeTextureMap;

	osgViewer::Viewer *m_viewer;
};


END_KLDISPLAY3D_NAMESPACE
#endif