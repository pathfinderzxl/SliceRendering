#pragma  once
#ifndef __KL3DVOLUME_KL3DDISPLAYLIST_H__
#define __KL3DVOLUME_KL3DDISPLAYLIST_H__

#include "KL3DVolumeMacro.h"
#include "osg/Vec3d"
#include <list>
#include <osgViewer/Viewer>

#include "KL3DDataModel/KL3DVOctree.h"
#include "KL3DDataModel/KLOctreeNode.h"

BEGIN_KLDISPLAY3D_NAMESPACE
class KLOctreeNode;
class KL3DVOctree;
class KL3DVOLUME_EXPORT KL3DDisplayList 
{
public:
	KL3DDisplayList();
	
	~KL3DDisplayList();
	// ���������ľ��� 
	double  computeDistance(const osg::Vec3d &point1, double *point2)
	{
		return sqrt((point1[0] - point2[0]) * (point1[0] - point2[0])
			+ (point1[1] - point2[1]) * (point1[1] - point2[1])
			+ (point1[2] - point2[2]) * (point1[2] - point2[2]));
	}
	/*
		brief
		ͨ�����ڵ���ӵ�ľ���������ʾ�б�
	*/
	void updateDisplayList(KL3DVOctree *octree , const osg::Vec3 cameraPosition, osgViewer::Viewer* viewer);
	
	/*
		brief
		ͨ�����ڵ���ӵ�ľ��룬������ʾ�б�,���������б��зֱ���������ļ���
		���ڽ���ʱ����ģ�ͣ���߽���Ч��
	*/
	void updateList(KL3DVOctree *octree, const osg::Vec3 cameraPosition, osgViewer::Viewer *viewer, int depth);
	/*!
	\brief �����ӵ�λ�ã�ָ��ĳһ���ĺ��ӽ��ļ���˳��
	\param node �˲������
	\      cameraPosition �ӵ�λ��
	\      order �õ��ļ���˳����Ӧ���ӽ��ı�ţ�0-7��
	\note  ��������֮�����ڵ���ϵ�������ʱ���մӺ���ǰ��˳����Ƹ����
	*/
	void setLoadOrder(KLOctreeNode *node,const osg::Vec3 cameraPosition, int order[8]);

	std::list<KLOctreeNode *> getDisplayNodeList()
	{
		return m_DisplayNodeList;
	}

	//������ʾ����б��ȵ���ֵ
	void setDisplayListThreshold(int num){m_MAXNUMDISPLAYLIST = num;}
	int getDisplayListThreshold(){return m_MAXNUMDISPLAYLIST;}

	std::list<KLOctreeNode *> getBasicSliceNodeList(){return m_basicSliceNodeList;}
	std::list<KLOctreeNode *> getRefineSliceNodeList(){return m_refineSliceNodeList;}

	void generateBasicSliceNodeList(KL3DVOctree *octree, int basicLevel);
	void generateBasicSliceNodeList();
	
	/*!
	\brief 
	\ direct -- ����
	\ depth -- �������
	\ refineLevel -- ϸ������
	*/
	void generateRefineSliceNodeList(KL3DVOctree *octree, int direct, int depth, int refineLevel);

	void refineSliceNodeList(KL3DVOctree *octree, int direct, osgViewer::Viewer *viewer, int depth);

	//list<KLOctreeNode*> refineSliceNode(int octreeDepth,KLOctreeNode* node, int direct, osgViewer::Viewer *viewer, int depth);
protected:
	/*�����Ӿ��������ƽ��*/
	void calculateFrustumPlanes(osgViewer::Viewer *viewer);


	/*
		\brife:�ж�����Ƿ����Ӿ�����
		\note�������Ӿ��������ƽ�棬�ֱ��ж����İ˸������Ƿ���ÿ��ƽ����Ӿ������һ�࣬
			  ������ж�����ĳ��ƽ����Ӿ������һ�࣬�����������Ӿ�����
		\return:
				0:�����ȫ���Ӿ����⣻
				1:������Ӿ����ཻ��
				2:�����ȫ���Ӿ����ڣ�
	*/
	int isVolumeInFrustum(KLOctreeNode* node);
	osg::ref_ptr<osg::Vec3dArray> calculatePoints(osgViewer::Viewer *viwwer);
	/*
	\brief:�ж�����İ�Χ���Ƿ���ȫ���Ӿ�����
	*/
	bool isSphereInFrustum(KLOctreeNode* node); 

	//void computeNearFar(osgViewer::Viewer *viewer,KLOctreeNode *root);
	osg::ref_ptr<osg::Group> frustumGroup ;


private:
	std::list<KLOctreeNode *> m_DisplayNodeList;
	std::list<KLOctreeNode *> m_basicSliceNodeList;
	std::list<KLOctreeNode *> m_refineSliceNodeList;
	int m_MAXNUMDISPLAYLIST;


	/*�Ӿ�������ƽ�淽�̵Ĳ���*/
	float frustumPlanes[6][4];
};


END_KLDISPLAY3D_NAMESPACE
#endif