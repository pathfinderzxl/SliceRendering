#include <iostream>
#include "osgViewer/Viewer"
#include "osgViewer/ViewerEventHandlers"
#include "osgGA/TrackballManipulator"

#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/lexical_cast.hpp"

#include "KL3DIsoSurfaceShape.h"

BEGIN_KLDISPLAY3D_NAMESPACE
KL3DIsoSurfaceShape::KL3DIsoSurfaceShape()
{
	m_pRoot = new osg::Switch;
	m_viewer = NULL;
	m_isosurface = new KL3DIsoSurfaceNode(); 
	m_pRoot->addChild(m_isosurface);

}
KL3DIsoSurfaceShape::KL3DIsoSurfaceShape(KL3DVolumeDataManager* dataManager)
{
	m_pRoot = new osg::Switch;
	m_viewer = NULL;
	m_isosurface = new KL3DIsoSurfaceNode(); 
	m_pRoot->addChild(m_isosurface);
	m_dataManager = dataManager;
}


bool KL3DIsoSurfaceShape::initData()
{
	if( m_dataManager->getDataAdapter()==NULL || m_viewer == NULL)
		return false;
	m_isosurface->reinit();
	onDataChanged();
	return true;
	
}


bool KL3DIsoSurfaceShape::testFile(char *filename)
{
	std::fstream ftest;
	ftest.open(filename,std::ios::in | std::ios::binary);
	if(ftest.is_open())
	{
		ftest.close();
		return true;
	}
	return false;
}
bool KL3DIsoSurfaceShape::onDataChanged()
{
	if (!m_dataManager->getDataAdapter())
		return false;
	
	//清除上一次数据的绘制图元
	m_isosurface->removeDrawables(0,m_isosurface->getNumDrawables());

	m_isosurface->setIsoValue(m_IsoValue);

	
	///小数据
	if (m_dataManager->getVolumeData())
	{
		m_isosurface->setVolumeHeader( m_dataManager->getDataAdapter()->getVolumeHeader());
		m_isosurface->setVolumeData(m_dataManager->getVolumeData());

	}
	else if(m_dataManager->getCache())//大数据
	{
		if(m_dataManager->getOctree())
			m_isosurface->setOctree(m_dataManager->getOctree());

		m_isosurface->setCache(m_dataManager->getCache());
		m_isosurface->setDisplayList(new KL3DDisplayList());
		m_isosurface->setOctreeNodeList(m_dataManager->getOctree()->m_rootNode);

	}
	/**************************************************************************/

	
	m_isosurface->setFunction(this->pfn);
	m_isosurface->setUpdateCallback(new KL3DIsoSurfaceUpdateCallback(m_viewer));

	m_isosurface->initIsoSurface();
	return true;
}

void KL3DIsoSurfaceShape::onColorMapChanged()
{
	m_isosurface->resetTexture();
}
END_KLDISPLAY3D_NAMESPACE