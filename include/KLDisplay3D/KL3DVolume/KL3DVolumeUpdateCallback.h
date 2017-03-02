//
//#pragma once
//
//#include <osg/NodeCallback>
//#include <osgViewer/Viewer>
//
//#include "KL3DVolume.h"
//#include "KL3DVolumeBlockCache.h"
//
//class KL3DVolumeUpdateCallback : public osg::NodeCallback
//{
//public:
//
//	KL3DVolumeUpdateCallback(osgViewer::Viewer *viewer, osg::Vec3d eye/*, KL3DVolumeBlockCache *cache*/)
//		: m_viewer(viewer), preEye(eye)/*, m_cache(cache)*/
//	{
//	}
//
//	void SetPreEye(osg::Vec3d eye)
//	{
//		preEye = eye;
//	}
//	
//	virtual void operator()(osg::Node *node, osg::NodeVisitor *nv)
//	{
//		osgGA::CameraManipulator *cm = m_viewer->getCameraManipulator();
//		osg::Camera *camera = m_viewer->getCamera();
//		osg::Vec3d cameraPosition;
//		osg::Matrix matrix1 = cm->getMatrix();
//		cameraPosition = osg::Vec3d(0.0, 0.0, 0.0) * matrix1;
//
//		if (!(fabs(cameraPosition[0] - preEye[0]) < 0.001
//			&& fabs(cameraPosition[1] - preEye[1]) < 0.001
//			&& fabs(cameraPosition[2] - preEye[2]) < 0.001)
//			) 
//		{
//			KL3DVolume *volume = (KL3DVolume *)node;
//			volume->UpdateVolume(cameraPosition);
//			//m_cache->setNodesIn(volume->getnidelist);
//		}
//
//		preEye = cameraPosition;
//		
//		//traverse(node, nv);
//	}
//
//private:
//
//	osgViewer::Viewer *m_viewer;
//
//	//KL3DVolumeBlockCache *m_cache;
//
//	osg::Vec3d preEye;
//	osg::Vec3d preCenter;
//	osg::Vec3d preUp;
//};