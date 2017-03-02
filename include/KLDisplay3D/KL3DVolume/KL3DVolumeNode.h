#ifndef __KL3DVOLUME_KL3DVOLUME_H__
#define __KL3DVOLUME_KL3DVOLUME_H__

#include "boost/function.hpp"
#include "KL3DVolumeMacro.h"

#include "osg/Group"
#include "osg/Vec3d"
#include "osg/StateSet"
#include "osgViewer/Viewer"
#include "osg/NodeCallback"
#include "osg/NodeVisitor"
#include "osgUtil/CullVisitor"
#include <osgGA/GUIEventHandler>
#include <osgGA/GUIActionAdapter>
#include <osgGA/GUIEventAdapter>


#include <list>


#include "KL3DDataModel/KL3DVOctree.h"
#include "KL3DVolumeBlockCache.h"
//#include "KL3DDataModel/KL3DVTestSgyDataAdapter.h"
#include "KL3DDataModel/KL3DVolumeDataAdapter.h"
#include "KL3DDisplayList.h"


BEGIN_KLDISPLAY3D_NAMESPACE

//#define MAXDISPLAYLISTSIZE (MAXBLOCKNUM / 4)

typedef boost::function<osg::Vec4(float val)> getColorByValueFunc;
class KL3DVOctree;
class KL3DVolumeBlockCache;
class KLOctreeNode;
class KL3DVTestSgyDataAdapter;
struct VolumeHeader;

using namespace std;
class KL3DVOLUME_EXPORT KL3DVolumeNode : public osg::Group
{
public:
	KL3DVolumeNode();
	
protected:
	~KL3DVolumeNode();

public:
	enum
	{
		WHOLEVOLUME,
		SUBVOLUME
	};

	static int texInCache;
	
	//重新初始化成员变量,
	//同一个shape当数据变化之后，需要将之前的node绘制所生成的东西放弃掉，所以重新初始化成员变量（内容与构造函数一致）
	void reinit();
	/*!
	\brief 初始化三维体
	*/
	void initialize();

	/*!
	\brief 获取体包围盒的中心点坐标
	*/
	osg::Vec3d getVolumeCenter()
	{
		osg::Vec3d center;
		center[0] = 0.5 * (m_volumeBounds[0][0] + m_volumeBounds[1][0]);
		center[1] = 0.5 * (m_volumeBounds[0][1] + m_volumeBounds[1][1]);
		center[2] = 0.5 * (m_volumeBounds[0][2] + m_volumeBounds[1][2]);
		return center;
	}

	/*!
	\brief 获取体球形包围盒的半径
	*/
	double getVolumeRadius()
	{
		return sqrt(0.25 * ((m_volumeBounds[1][0] - m_volumeBounds[0][0]) * (m_volumeBounds[1][0] - m_volumeBounds[0][0])
			+ (m_volumeBounds[1][1] - m_volumeBounds[0][1]) * (m_volumeBounds[1][1] - m_volumeBounds[0][1])
			+ (m_volumeBounds[1][2] - m_volumeBounds[0][2]) * (m_volumeBounds[1][2] - m_volumeBounds[0][2])));
	}
	/*!
	\brief 根据视点位置，指定某一结点的孩子结点的加载顺序
	\param node 八叉树结点
	\      cameraPosition 视点位置
	\      order 得到的加载顺序，相应孩子结点的编号（0-7）
	\note  体块与体块之间有遮挡关系，体绘制时按照从后向前的顺序绘制各体块
	*/
	void setLoadOrder(KLOctreeNode *node, osg::Vec3 cameraPosition, int order[8]);

	/*!
	\brief 根据视点位置，更新显示列表
	\note 体块结点分为可见结点和不可见结点，不可见结点保持最低分辨率;
	\     可视结点离视点越近，分辨率越低
	*/
	void updateNodeList(osg::Vec3 cameraPosition);

	/*!
	\brief 根据视点位置更新体
	*/
	void updateVolume(osg::Vec3 cameraPosition,osgViewer::Viewer* viewer, int depth);
	/*!
	\brief 根据视点位置和给定空间范围更新子体
	*/
	void updateSubVolume(osg::Vec3 cameraPosition, int subBounds[6]);
	/*!
	\brief 体数据对应的八叉树结构
	*/
	void setOctree(KL3DVOctree *octree)
	{
		m_octree = octree;
	}
	void setDisplayList(KL3DDisplayList* displayList)
	{
		m_displayList = displayList;
	}
	void setOctreeNodeList(KLOctreeNode *node)
	{
		m_octreeNodeList.push_back(node); 
	}
	

	void setVolumeBounds(osg::Vec3 bounds[2])
	{
		m_volumeBounds[0] = bounds[0];
		m_volumeBounds[1] = bounds[1];
	}

	void setVolumeBounds( VolumeHeader  volumeHeader)
	{

		m_volumeBounds[0] = osg::Vec3(volumeHeader.origin[0], volumeHeader.origin[1], volumeHeader.origin[2]);
		m_volumeBounds[1][0] = m_volumeBounds[0][0] + volumeHeader.interFirst * (volumeHeader.numFirst - 1);
		m_volumeBounds[1][1] = m_volumeBounds[0][1] + volumeHeader.interSecond * (volumeHeader.numSecond - 1);
		m_volumeBounds[1][2] = m_volumeBounds[0][2] + volumeHeader.interThird * (volumeHeader.numThird - 1);
	}
	void setVolumeHeader( VolumeHeader  volumeHeader)
	{
		m_volHeader = volumeHeader;
	}
	float* getVolumeData()
	{
		return m_volumeData;
	}

	void setVolumeData(float *data)
	{
		m_volumeData = data;
	}
	KL3DVolumeBlockCache *getCache()
	{
		return m_cache;
	}
	void setCache(KL3DVolumeBlockCache *cache)
	{
		m_cache = cache;
	}

	void buildCache(KL3DVOctree *octree,int cachesize)
	{//重新开辟
		
		m_cache = new KL3DVolumeBlockCache();
		m_cache->setCacheSize(cachesize);
		m_cache->initCache(octree);
		m_octreeNodeList.push_back(octree->m_rootNode);
		
	}

	
	// 色表
	void setFunction(getColorByValueFunc pfn){this->pfn = pfn;}

	// 色表变化，重新生成纹理
	void resetTexture() {m_colorMapChanged = true;}

	// 判断色表是否变化
	bool isColorMapChanged(){return m_colorMapChanged;}

	//根据获得旋转后的体节点的坐标范围
	void getVolumeNodeBoundsXYZ(KLOctreeNode *node,double* bounds);
protected:

	/*!
	\brief 为体块设置状态,包括读取体块数据、生成三维纹理,体块对应八叉树结点
	\	   现在兼容小数据的纹理生成。data指针为内存指针而已
	\	   还包括实现坐标轴旋转切换也都在本函数中实现
	\param  dim[3]是原始三个维度的采样点数
	*/
	osg::ref_ptr<osg::StateSet> createState(int dim[3], float *data);

	
	void computePolygon(osg::Vec3 cameraPosition, osg::Vec3 bounds[2]);

	// 查询stateset缓冲结点列表
	bool checkStatesetList(KLOctreeNode *node);

	// 获取结点状态
	osg::ref_ptr<osg::StateSet> getStateSet(KLOctreeNode *node, int dim[3]);
protected:
	//色表
	getColorByValueFunc pfn;
	
	// 体块数据的缓存管理对象
	KL3DVolumeBlockCache* m_cache;

	
	// 三维体的范围
	osg::Vec3 m_volumeBounds[2];

	double m_volBounds[6];

	// 显示列表
	std::list<KLOctreeNode *> m_octreeNodeList;
	//根据视点更新生成显示列表的对象
	KL3DDisplayList* m_displayList;
	

	//数据状态信息
	struct VolumeHeader m_volHeader;

	// 体数据对应的八叉树结构
	KL3DVOctree *m_octree;
	
	// 小数据时，直接将数据写入内存的地址
	float *m_volumeData;

	// 辅助计算多边形切片
	int m_bufferSize;
	float *m_polygonBuffer;
	float *m_intersectionBuffer;
	int m_numOfPolygon;
	double m_sampleDistance[3];

	// 标识色表变化
	bool m_colorMapChanged;

	// stateset缓冲区
	std::list<KLOctreeNode *> m_nodeInStatesetCache;
	std::map<KLOctreeNode *, osg::StateSet *> m_mapNodeAndStateset;
public:
	map<osg::Texture*, int> texMap;
};

/*!
\breif 体节点交互信息
*/
class KL3DVOLUME_EXPORT KL3DVolumeEventState
{
public:
	KL3DVolumeEventState():m_bDetailFlag(false){}
	~KL3DVolumeEventState(){}
	bool getDetailFlag(){return m_bDetailFlag;}
	void setDetailFlag(bool _flag){m_bDetailFlag = _flag;}
private:
	//体是否细化
	bool m_bDetailFlag;
};

/*
	
*/
class KL3DVOLUME_EXPORT KL3DVolumeEventHandler : public osgGA::GUIEventHandler
{
public:
	KL3DVolumeEventHandler(KL3DVolumeEventState *_evnetState):m_pEventState(_evnetState){}
	virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
	{
		switch(ea.getEventType())
		{
		case(osgGA::GUIEventAdapter::KEYDOWN):
			{
				switch(ea.getKey())
				{
				case 'd':
				case 'D':
					{
						m_pEventState->setDetailFlag(true);
						return false;
						break;
					}
				default: 
					return false;
				}
			}
		default: 
			return false;
		}
	}

private:
	KL3DVolumeEventState *m_pEventState;
};




class KL3DVOLUME_EXPORT KL3DVolumeUpdateCallback : public osg::NodeCallback
{
public:

	KL3DVolumeUpdateCallback(osgViewer::Viewer *viewer, KL3DVolumeEventState *_eventState)	
		: m_viewer(viewer), m_pEventState(_eventState)
	{
	}

	void setPreEye(osg::Vec3d eye)
	{
		preEye = eye;
	}

	void operator()(osg::Node *node, osg::NodeVisitor *nv)
	{
		osgGA::CameraManipulator *cm = m_viewer->getCameraManipulator();
		//osg::Camera *camera = m_viewer->getCamera();
		osg::Vec3d cameraPosition;
		osg::Matrix matrix1 = cm->getMatrix();
		cameraPosition = osg::Vec3d(0.0, 0.0, 0.0) * matrix1;
		KL3DVolumeNode *volume = (KL3DVolumeNode *)node;

		
		// 视点或色表发生变化，更新体
		if (!(fabs(cameraPosition[0] - preEye[0]) < 0.001
			&& fabs(cameraPosition[1] - preEye[1]) < 0.001
			&& fabs(cameraPosition[2] - preEye[2]) < 0.001) || volume->isColorMapChanged()
			) 
		{
			osg::Matrix matrix2 = cm->getInverseMatrix();
			m_viewer->getCamera()->setViewMatrix(matrix2);
			volume->updateVolume(cameraPosition,m_viewer,0);
			map<osg::Texture*, int>::iterator itr = volume->texMap.begin();
			for (; itr!=volume->texMap.end(); ++itr)
			{
				osg::Texture *texture = itr->first;
				osg::Texture::TextureObject *to = (itr->first)->getTextureObject(0);
				 
				if (to)
				{
					itr->second = 1;
				}
				if(to == NULL){itr->second = 0;}
			}
		
			int n=0;
			for(itr=volume->texMap.begin(); itr!=volume->texMap.end(); ++itr)
			{
				if (itr->second ==1)
				{
					++n;
				}
			}
			cout<<"-------------------"<<n<<endl;
		}
		else
		{
			if (m_pEventState->getDetailFlag())
			{
				volume->updateVolume(cameraPosition,m_viewer,0);
				m_pEventState->setDetailFlag(false);
			}
			
		}
		preEye = cameraPosition;
	}

private:
	
	osgViewer::Viewer *m_viewer;

	osg::Vec3d preEye;
	osg::Vec3d preCenter;
	osg::Vec3d preUp;
	KL3DVolumeEventState *m_pEventState;
	
};

END_KLDISPLAY3D_NAMESPACE

#endif