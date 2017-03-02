#ifndef __KL3DBASESHAPE_H__
#define __KL3DBASESHAPE_H__

#include "KL3DVolumeDataManager.h"
BEGIN_KLDISPLAY3D_NAMESPACE
class KL3DVolumeDataManager;
class KL3DVOLUME_EXPORT KL3DBaseShape
{
public:
	KL3DBaseShape(void);
	virtual ~KL3DBaseShape(void);

	/*
	*设置数据管理器
	*/
	void setDataManager(KL3DVolumeDataManager * dataManager);

	/*
	*初始化图元
	*/
	virtual void initShape();

	/*
	*返回图形节点，用于向场景添加
	*/
	virtual osg::ref_ptr<osg::Switch> getShapeNode() = 0;
protected:
	KL3DVolumeDataManager * m_dataManager;
};

END_KLDISPLAY3D_NAMESPACE
#endif

