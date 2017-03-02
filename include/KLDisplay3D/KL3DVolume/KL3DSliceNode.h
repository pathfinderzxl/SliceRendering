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
	*������Ƭ����
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
	*��Ƭ��ק����
	*/
	void updateDrag();

	/*
	*��Ƭ�ͷŸ���
	*/
	void updateRelease();

	/*
	*���»���,���ݶ�Ӧ��map
	*/
	void updateDraw();

	/*
	*�����Ƭ����
	*/
	void initDraw();


	/*
	*����ϸ�����ڼ���
	*/
	void setRefineLevel(unsigned int level);

	/*
	*���ñ߽�
	* xmin ymin zmin xmax ymax zmax
	*/
	void setBounds(float * bounds);

	/*
	*��ñ߽�
	* xmin ymin zmin xmax ymax zmax
	*/
	const float * getBounds();

	/*
	*���û״̬Ϊ�ر�
	*/
	void setActiveOff()
	{
		m_isActive = false;
	}

	/*
	*���û״̬Ϊ����
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
	\brief ������Ƭ��ʾΪ����Ƭ
	\param bool flag �����ж��Ƿ���Ҫɾ��֮ǰ���Ƶ�ͼԪ�� true--ɾ���� false--��ɾ�� 
	*/
	void updateSlices(std::list<KLOctreeNode*> nodelist, bool flag);

	
	float m_normal[3];
	float m_origin[3];
	float *PolygonBuffer;
	float *IntersectionBuffer;
	unsigned int m_refineLevel;
	float m_bounds[6];
	osg::ref_ptr<osg::Geode> m_highLightGeode;//��Ƭ�߿�ͼԪ
	osg::ref_ptr<osg::Geode> m_sliceGeode;//��ƬͼԪ
	//CUPsliceRefineThread * m_refineThread;
	//������Ƭ���� 0-x 1-y 2-z
	int m_direct;
	//�ж���Ƭ�Ƿ�
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
