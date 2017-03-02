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
	// 计算两点间的距离 
	double  computeDistance(const osg::Vec3d &point1, double *point2)
	{
		return sqrt((point1[0] - point2[0]) * (point1[0] - point2[0])
			+ (point1[1] - point2[1]) * (point1[1] - point2[1])
			+ (point1[2] - point2[2]) * (point1[2] - point2[2]));
	}
	/*
		brief
		通过根节点和视点的距离生成显示列表
	*/
	void updateDisplayList(KL3DVOctree *octree , const osg::Vec3 cameraPosition, osgViewer::Viewer* viewer);
	
	/*
		brief
		通过根节点和视点的距离，生成显示列表,并且限制列表中分辨率最高体块的级别。
		用于交互时简化体模型，提高交互效率
	*/
	void updateList(KL3DVOctree *octree, const osg::Vec3 cameraPosition, osgViewer::Viewer *viewer, int depth);
	/*!
	\brief 根据视点位置，指定某一结点的孩子结点的加载顺序
	\param node 八叉树结点
	\      cameraPosition 视点位置
	\      order 得到的加载顺序，相应孩子结点的编号（0-7）
	\note  体块与体块之间有遮挡关系，体绘制时按照从后向前的顺序绘制各体块
	*/
	void setLoadOrder(KLOctreeNode *node,const osg::Vec3 cameraPosition, int order[8]);

	std::list<KLOctreeNode *> getDisplayNodeList()
	{
		return m_DisplayNodeList;
	}

	//设置显示体块列表长度的阈值
	void setDisplayListThreshold(int num){m_MAXNUMDISPLAYLIST = num;}
	int getDisplayListThreshold(){return m_MAXNUMDISPLAYLIST;}

	std::list<KLOctreeNode *> getBasicSliceNodeList(){return m_basicSliceNodeList;}
	std::list<KLOctreeNode *> getRefineSliceNodeList(){return m_refineSliceNodeList;}

	void generateBasicSliceNodeList(KL3DVOctree *octree, int basicLevel);
	void generateBasicSliceNodeList();
	
	/*!
	\brief 
	\ direct -- 方向
	\ depth -- 网格深度
	\ refineLevel -- 细化级别
	*/
	void generateRefineSliceNodeList(KL3DVOctree *octree, int direct, int depth, int refineLevel);

	void refineSliceNodeList(KL3DVOctree *octree, int direct, osgViewer::Viewer *viewer, int depth);

	//list<KLOctreeNode*> refineSliceNode(int octreeDepth,KLOctreeNode* node, int direct, osgViewer::Viewer *viewer, int depth);
protected:
	/*计算视景体的六个平面*/
	void calculateFrustumPlanes(osgViewer::Viewer *viewer);


	/*
		\brife:判断体块是否在视景体内
		\note：对于视景体的六个平面，分别判断体块的八个顶点是否在每个平面的视景体外的一侧，
			  如果所有顶点在某个平面的视景体外的一侧，则表明体块在视景体外
		\return:
				0:体块完全在视景体外；
				1:体块与视景体相交；
				2:体块完全在视景体内；
	*/
	int isVolumeInFrustum(KLOctreeNode* node);
	osg::ref_ptr<osg::Vec3dArray> calculatePoints(osgViewer::Viewer *viwwer);
	/*
	\brief:判断体块块的包围球是否完全在视景体内
	*/
	bool isSphereInFrustum(KLOctreeNode* node); 

	//void computeNearFar(osgViewer::Viewer *viewer,KLOctreeNode *root);
	osg::ref_ptr<osg::Group> frustumGroup ;


private:
	std::list<KLOctreeNode *> m_DisplayNodeList;
	std::list<KLOctreeNode *> m_basicSliceNodeList;
	std::list<KLOctreeNode *> m_refineSliceNodeList;
	int m_MAXNUMDISPLAYLIST;


	/*视景体六个平面方程的参数*/
	float frustumPlanes[6][4];
};


END_KLDISPLAY3D_NAMESPACE
#endif