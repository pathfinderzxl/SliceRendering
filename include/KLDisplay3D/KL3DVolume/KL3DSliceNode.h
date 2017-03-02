#ifndef __KL3DSLICENODE_H__
#define __KL3DSLICENODE_H__

#include <map>
#include <list>
#include <vector>
#include "KL3DVolumeDataManager.h"
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Geode>
#include <osgGA/CameraManipulator>
#include <osgViewer/Viewer>
#include "KL3DDisplayList.h"



BEGIN_KLDISPLAY3D_NAMESPACE
class KLOctreeNode;
class KL3DSliceSet;
using namespace std;
class KL3DVOLUME_EXPORT KL3DSliceNode : public osg::Group
{
public:
	KL3DSliceNode(void);
	virtual ~KL3DSliceNode(void);


	/*
	*设置切片参数
	*/
	void setNormal(float x,float y,float z);
	float * getNormal();
	void setNormal(float * norm);
	void setOrigin(float x,float y,float z);
	float * getOrigin();
	void setOrigin(float * orig);
	int getDirection();
	void setDirection(int direct)
	{
		m_direct = direct;
	}
	/*
	*切片拖拽更新
	*/
	void updateDrag();

	/*
	*切片释放更新
	*/
	void updateRelease();

	/*
	*更新绘制,根据对应的map
	*/
	void updateDraw();

	/*
	*完成切片绘制
	*/
	void initDraw();


	/*
	*设置细化到第几层
	*/
	void setRefineLevel(unsigned int level);

	/*
	*设置边界
	* xmin ymin zmin xmax ymax zmax
	*/
	void setBounds(float * bounds);

	/*
	*获得边界
	* xmin ymin zmin xmax ymax zmax
	*/
	const float * getBounds();

	/*
	*设置活动状态为关闭
	*/
	void setActiveOff()
	{
		m_isActive = false;
	}

	/*
	*设置活动状态为开启
	*/
	void setActiveOn()
	{
		m_isActive = true;
	}

	bool isSliceActive()
	{
		return m_isActive;
	}

	void setDataManager(KL3DVolumeDataManager* dataManager)
	{
		m_pDataManager = dataManager;
		
	}
	void setSliceSet(KL3DSliceSet* sliceset);

	void setViewer(osgViewer::Viewer *viewer)
	{
		this->m_pViewer = viewer;
	}

	osg::Geode* drawSlice2Refine(std::list<KLOctreeNode*> nodeList);
private:
	void computeBigPolygon(osg::Vec3Array * verAry,osg::Vec3Array * texAry,float * b);
	/*!
	\brief 更新切片显示为多切片
	\param bool flag 用于判定是否需要删除之前绘制的图元： true--删除； false--不删除 
	*/
	void updateSlices(std::list<KLOctreeNode*> nodelist, bool flag);

	
	float m_normal[3];
	float m_origin[3];
	float *PolygonBuffer;
	float *IntersectionBuffer;
	unsigned int m_refineLevel;
	float m_bounds[6];
	osg::ref_ptr<osg::Geode> m_highLightGeode;//切片边框图元
	osg::ref_ptr<osg::Geode> m_sliceGeode;//切片图元
	//CUPsliceRefineThread * m_refineThread;
	//定义切片方向 0-x 1-y 2-z
	int m_direct;
	//判断切片是否活动
	bool m_isActive;

	KL3DVolumeDataManager *m_pDataManager;


	KL3DSliceSet *m_pSliceSet;

	std::vector<KL3DSliceNode*> m_sliceNodeVec;
	bool m_isInteractive;
	osg::ref_ptr<osgViewer::Viewer> m_pViewer;



public:
	static int m_activeNum;
};

END_KLDISPLAY3D_NAMESPACE

#endif
