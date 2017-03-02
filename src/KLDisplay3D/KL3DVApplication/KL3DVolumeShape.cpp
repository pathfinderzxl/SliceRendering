
#include "KL3DVolumeShape.h"
#include <iostream>
#include <fstream>
#include "osgViewer/Viewer"
#include "osgViewer/ViewerEventHandlers"
#include "osgGA/TrackballManipulator"

#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/lexical_cast.hpp"

#include "KLGetColor.h"

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DVolumeShape::KL3DVolumeShape()
{
	m_pRoot = new osg::Switch;
	m_viewer = NULL;
	m_volume = new KL3DVolumeNode(); 
	m_pRoot->addChild(m_volume);
	m_pEventState = new KL3DVolumeEventState();
}	
KL3DVolumeShape::KL3DVolumeShape(KL3DVolumeDataManager *dataManager)
{
	m_pRoot = new osg::Switch;
	m_viewer = NULL;
	m_volume = new KL3DVolumeNode(); 
	m_pRoot->addChild(m_volume);
	m_dataManager = dataManager;
	m_pEventState = new KL3DVolumeEventState();
}

KL3DVolumeShape::~KL3DVolumeShape()
{
	/*if (m_pRoot)
	{
	delete m_pRoot;
	m_pRoot = 0;
	}
	if (m_volume)
	{
	delete m_volume;
	m_volume = 0;
	}*/
	if (m_pEventState)
	{
		delete m_pEventState;
		m_pEventState = 0;
	}
}
//bool KL3DVolumeShape::initData(std::stringoctreeFileName)
//{
//	if (octreeFileName == NULL || m_viewer == NULL)
//		return false;
//
//	if (!testFile(octreeFileName))
//		return false;
//
//	
//
//	onDataChanged();
//
//	return true;
//}
bool KL3DVolumeShape::initData()
{
	if(m_viewer == NULL || m_dataManager->getDataAdapter()==NULL)
		return false;
	m_volume->reinit();
	
	onDataChanged();
	
	return true;
}

bool KL3DVolumeShape::testFile(std::string filename)
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
bool KL3DVolumeShape::onDataChanged()
{
	//数据发生变化,m_volume的相关属性要全部更新
	if (!m_dataManager->getDataAdapter())
		return false;
	//清除上一次数据的绘制图元
	m_volume->removeChildren(0,m_volume->getNumChildren());

	VolumeHeader volumeHeader = m_dataManager->getDataAdapter()->getVolumeHeader();
	double bounds[6];
	m_dataManager->getDataAdapter()->getVolumeBoundsXYZ(bounds);
	osg::Vec3 vecBounds[2] = {
		osg::Vec3(bounds[0],bounds[2],bounds[4]),
		osg::Vec3(bounds[1],bounds[3],bounds[5])
	};
	m_volume->setVolumeBounds(vecBounds);	

	//小数据 
	if (m_dataManager->getVolumeData())
	{
		m_volume->setVolumeHeader(volumeHeader);
		m_volume->setVolumeData(m_dataManager->getVolumeData());
	}
	//大数据
	else if(m_dataManager->getCache())
		
	{	
		if(m_dataManager->getOctree())
			m_volume->setOctree(m_dataManager->getOctree());
		
		m_volume->setCache(m_dataManager->getCache());
		m_volume->setDisplayList(new KL3DDisplayList());
		m_volume->setOctreeNodeList(m_dataManager->getOctree()->m_rootNode);
		
	}
	m_volume->setFunction(this->pfn);
	m_viewer->addEventHandler(new KL3DVolumeEventHandler(m_pEventState));
	m_volume->setUpdateCallback(new KL3DVolumeUpdateCallback(m_viewer, m_pEventState));
	m_volume->initialize();

	return true;
	
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