#ifndef __KL3DORTHOSLICESHAPE_H__ 
#define __KL3DORTHOSLICESHAPE_H__

#include "KL3DDataModel/KL3DDataModelMacro.h"
#include <osg/Switch>
#include <osgViewer/Viewer>

#include "KL3DBaseShape.h"
#include "KL3DSliceNode.h"
BEGIN_KLDISPLAY3D_NAMESPACE

class KL3DVOLUME_EXPORT KL3DOrthoSliceShape :public KL3DBaseShape
{
public:
	KL3DOrthoSliceShape(int direct, KL3DSliceSet* sliceset);
	virtual ~KL3DOrthoSliceShape(void);

	/*
	*初始化图元
	*/
	virtual void initShape();

	/*
	*返回图形节点，用于向场景添加
	*/
	virtual osg::ref_ptr<osg::Switch> getShapeNode();

	//Unique

	/*
	*初始化切片数据
	*/
	void initData();

	/*
	*初始化切片
	*/
	void initSlice();

	/*
	*返回切片图元
	*/
	KL3DSliceNode * getSliceNode();
	/*
	*设置切片方向 0-x 1-y 2-z
	*/
	void setDirect(int d);

	void setDataManager(KL3DVolumeDataManager* dataManager){m_dataManager = dataManager;}

	void setViewer(osgViewer::Viewer *pViewer){
		this->m_viewer = pViewer;
		if (m_dataManager != NULL)
		{
			m_dataManager->setViewer(pViewer);
		}
	}
	osgViewer::Viewer* getViewer(){return m_viewer;}
	void setSliceSet(KL3DSliceSet* sliceSet)
	{
		m_sliceSet = sliceSet;
	}
private:
	osg::ref_ptr<osg::Switch> m_switch;
	int m_direct;//切片方向 0-x 1-y 2-z
	KL3DSliceNode * m_sliceNode;
	float m_bounds[6];
	osgViewer::Viewer *m_viewer;
	KL3DVolumeDataManager* m_dataManager;

	KL3DSliceSet* m_sliceSet;

};

END_KLDISPLAY3D_NAMESPACE
#endif