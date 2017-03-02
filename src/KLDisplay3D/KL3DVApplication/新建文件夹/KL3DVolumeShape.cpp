
#include "KL3DVolumeShape.h"
#include "osgViewer/Viewer"
#include "osgViewer/ViewerEventHandlers"
#include "osgGA/TrackballManipulator"
#include "KLGetColor.h"

#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/lexical_cast.hpp"

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DVolumeShape::KL3DVolumeShape(osgViewer::Viewer *viewer, char *sourceFileName)
{
	m_pRoot = new osg::Switch;
	m_volumeDataAdapter = new KL3DVBigSgyDataAdapter(sourceFileName);
	m_volumeDataAdapter->process();
	Octree *octree = new Octree(m_volumeDataAdapter->getOctreeFileName());
	octree->CreateOctree();

	KLGetColor getColor;
	pfn = boost::bind(&KLGetColor::getColorByValue1, &getColor, _1);

	m_volume = new KL3DVolume(octree, viewer, pfn);
	// m_volume->setFunction(pfn);

	viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->setLightingMode(osg::View::LightingMode::NO_LIGHT);

	viewer->setUpViewInWindow(100, 80, 400, 300);

	osgGA::CameraManipulator *cm = new osgGA::TrackballManipulator;
	cm->setAutoComputeHomePosition(false);

	double radius = m_volume->getVolumeRadius();
	osg::Vec3 center = m_volume->getVolumeCenter();
	osg::Vec3 eye = center + osg::Vec3(0, -5 * radius, 0);

	cm->setHomePosition(eye, center, osg::Vec3(0.0, 0.0, 1.0), false);
	viewer->setCameraManipulator(cm);

	m_volume->setUpdateCallback(new KL3DVolumeUpdateCallback(viewer, eye));
	m_pRoot->addChild(m_volume);
}

void KL3DVolumeShape::onColorMapChanged()
{
	KLGetColor getColor;

	switch(KLGetColor::flag)
	{
	case 0:
		{
			pfn = boost::bind(&KLGetColor::getColorByValue1, &getColor, _1);
		}
		break ;
	case 1:
		{
			pfn = boost::bind(&KLGetColor::getColorByValue1, &getColor, _1);
		}
		break ;
	case 2:
		{
			pfn = boost::bind(&KLGetColor::getColorByValue1, &getColor, _1);
		}
		break ;
	}

	m_volume->setFunction(pfn);
	m_volume->resetTexture();
}

END_KLDISPLAY3D_NAMESPACE
