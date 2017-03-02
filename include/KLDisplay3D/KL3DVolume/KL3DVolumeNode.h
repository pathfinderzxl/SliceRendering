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
	
	//���³�ʼ����Ա����,
	//ͬһ��shape�����ݱ仯֮����Ҫ��֮ǰ��node���������ɵĶ������������������³�ʼ����Ա�����������빹�캯��һ�£�
	void reinit();
	/*!
	\brief ��ʼ����ά��
	*/
	void initialize();

	/*!
	\brief ��ȡ���Χ�е����ĵ�����
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
	\brief ��ȡ�����ΰ�Χ�еİ뾶
	*/
	double getVolumeRadius()
	{
		return sqrt(0.25 * ((m_volumeBounds[1][0] - m_volumeBounds[0][0]) * (m_volumeBounds[1][0] - m_volumeBounds[0][0])
			+ (m_volumeBounds[1][1] - m_volumeBounds[0][1]) * (m_volumeBounds[1][1] - m_volumeBounds[0][1])
			+ (m_volumeBounds[1][2] - m_volumeBounds[0][2]) * (m_volumeBounds[1][2] - m_volumeBounds[0][2])));
	}
	/*!
	\brief �����ӵ�λ�ã�ָ��ĳһ���ĺ��ӽ��ļ���˳��
	\param node �˲������
	\      cameraPosition �ӵ�λ��
	\      order �õ��ļ���˳����Ӧ���ӽ��ı�ţ�0-7��
	\note  ��������֮�����ڵ���ϵ�������ʱ���մӺ���ǰ��˳����Ƹ����
	*/
	void setLoadOrder(KLOctreeNode *node, osg::Vec3 cameraPosition, int order[8]);

	/*!
	\brief �����ӵ�λ�ã�������ʾ�б�
	\note ������Ϊ�ɼ����Ͳ��ɼ���㣬���ɼ���㱣����ͷֱ���;
	\     ���ӽ�����ӵ�Խ�����ֱ���Խ��
	*/
	void updateNodeList(osg::Vec3 cameraPosition);

	/*!
	\brief �����ӵ�λ�ø�����
	*/
	void updateVolume(osg::Vec3 cameraPosition,osgViewer::Viewer* viewer, int depth);
	/*!
	\brief �����ӵ�λ�ú͸����ռ䷶Χ��������
	*/
	void updateSubVolume(osg::Vec3 cameraPosition, int subBounds[6]);
	/*!
	\brief �����ݶ�Ӧ�İ˲����ṹ
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
	{//���¿���
		
		m_cache = new KL3DVolumeBlockCache();
		m_cache->setCacheSize(cachesize);
		m_cache->initCache(octree);
		m_octreeNodeList.push_back(octree->m_rootNode);
		
	}

	
	// ɫ��
	void setFunction(getColorByValueFunc pfn){this->pfn = pfn;}

	// ɫ��仯��������������
	void resetTexture() {m_colorMapChanged = true;}

	// �ж�ɫ���Ƿ�仯
	bool isColorMapChanged(){return m_colorMapChanged;}

	//���ݻ����ת�����ڵ�����귶Χ
	void getVolumeNodeBoundsXYZ(KLOctreeNode *node,double* bounds);
protected:

	/*!
	\brief Ϊ�������״̬,������ȡ������ݡ�������ά����,����Ӧ�˲������
	\	   ���ڼ���С���ݵ��������ɡ�dataָ��Ϊ�ڴ�ָ�����
	\	   ������ʵ����������ת�л�Ҳ���ڱ�������ʵ��
	\param  dim[3]��ԭʼ����ά�ȵĲ�������
	*/
	osg::ref_ptr<osg::StateSet> createState(int dim[3], float *data);

	
	void computePolygon(osg::Vec3 cameraPosition, osg::Vec3 bounds[2]);

	// ��ѯstateset�������б�
	bool checkStatesetList(KLOctreeNode *node);

	// ��ȡ���״̬
	osg::ref_ptr<osg::StateSet> getStateSet(KLOctreeNode *node, int dim[3]);
protected:
	//ɫ��
	getColorByValueFunc pfn;
	
	// ������ݵĻ���������
	KL3DVolumeBlockCache* m_cache;

	
	// ��ά��ķ�Χ
	osg::Vec3 m_volumeBounds[2];

	double m_volBounds[6];

	// ��ʾ�б�
	std::list<KLOctreeNode *> m_octreeNodeList;
	//�����ӵ����������ʾ�б�Ķ���
	KL3DDisplayList* m_displayList;
	

	//����״̬��Ϣ
	struct VolumeHeader m_volHeader;

	// �����ݶ�Ӧ�İ˲����ṹ
	KL3DVOctree *m_octree;
	
	// С����ʱ��ֱ�ӽ�����д���ڴ�ĵ�ַ
	float *m_volumeData;

	// ��������������Ƭ
	int m_bufferSize;
	float *m_polygonBuffer;
	float *m_intersectionBuffer;
	int m_numOfPolygon;
	double m_sampleDistance[3];

	// ��ʶɫ��仯
	bool m_colorMapChanged;

	// stateset������
	std::list<KLOctreeNode *> m_nodeInStatesetCache;
	std::map<KLOctreeNode *, osg::StateSet *> m_mapNodeAndStateset;
public:
	map<osg::Texture*, int> texMap;
};

/*!
\breif ��ڵ㽻����Ϣ
*/
class KL3DVOLUME_EXPORT KL3DVolumeEventState
{
public:
	KL3DVolumeEventState():m_bDetailFlag(false){}
	~KL3DVolumeEventState(){}
	bool getDetailFlag(){return m_bDetailFlag;}
	void setDetailFlag(bool _flag){m_bDetailFlag = _flag;}
private:
	//���Ƿ�ϸ��
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

		
		// �ӵ��ɫ�����仯��������
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