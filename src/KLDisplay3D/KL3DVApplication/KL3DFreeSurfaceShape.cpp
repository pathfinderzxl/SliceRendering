//#include <iostream>
//#include <osg/Image>
//#include <osg/MatrixTransform>
//#include <osg/TexEnv>
//#include "KL3DFreeSurfaceShape.h"
////#include "KL3DBase/Kl3DVScene.h"
//#include "KL3DDataModel/KL3DVOctree.h"
//#include "KL3DDataModel/KL3DTriangleNetDataAdapter.h"
//
//
//BEGIN_KLDISPLAY3D_NAMESPACE
//KL3DFreeSurfaceShape::KL3DFreeSurfaceShape()//(KLDisplay3D::KL3DVScene* pScene) :KL3DVisualObjectImpl(pScene)
//{
//	m_pRoot = new osg::Switch;
//	m_VolDataAdapter = NULL;
//	m_TriDataAdapter = NULL;
//	m_viewer = NULL;
//	m_FreeSurfaceNode = new KL3DFreeSurfaceNode();
//	m_pRoot->addChild(m_FreeSurfaceNode);
//	//addChild(m_pRoot);
//}
//KL3DFreeSurfaceShape::~KL3DFreeSurfaceShape()
//{
//
//}
//
//bool KL3DFreeSurfaceShape::initData(KL3DVolumeDataAdapter* volAdapter , KL3DTriangleNetDataAdapter* triAdapter)
//{
//	if(volAdapter == NULL || triAdapter == NULL)
//		return false;
//	m_VolDataAdapter = volAdapter;
//	m_TriDataAdapter = triAdapter;
//	onDataChanged();
//	return true;
//}
//
//bool KL3DFreeSurfaceShape::testFile(char*fileName)
//{
//	std::fstream ftest;
//	ftest.open(fileName,std::ios::in | std::ios::binary);
//	if(ftest.is_open())
//	{
//		ftest.close();
//		return true;
//	}
//	return false;
//}
//
//bool KL3DFreeSurfaceShape::onDataChanged()
//{
//	//小数据
//	if(m_VolDataAdapter && m_TriDataAdapter)
//		
//	{
//		if(!m_VolDataAdapter->excuteVolumeData()) return false;
//		VolumeHeader volumeHeader = m_VolDataAdapter->getVolumeHeader();
//		m_FreeSurfaceNode->setDataAdapter(m_VolDataAdapter , m_TriDataAdapter);
//		m_FreeSurfaceNode->setVolumeData(m_VolDataAdapter->getVolumeData());
//		m_FreeSurfaceNode->setVolumeHeader(volumeHeader);
//	}
//	//大数据
//	else
//	{
//		std::string name_temp(m_octreeFileName);
//		KL3DVOctree* octree = new KL3DVOctree(name_temp);
//		octree->CreateOctree();
//		m_FreeSurfaceNode->setOctree(octree);
//		m_FreeSurfaceNode->setDataAdapter(m_VolDataAdapter,m_TriDataAdapter);
//		m_FreeSurfaceNode->setOctreeNodeList(octree->_rootNode);
//		m_FreeSurfaceNode->setFileName(m_octreeFileName);
//
//		m_FreeSurfaceNode->buildCache(octree);
//	}
//	m_FreeSurfaceNode->setFunction(this->pfn);
//	m_FreeSurfaceNode->setUpdateCallback(new KL3DFreeSurfaceUpdateCallback(m_viewer));
//
//	m_FreeSurfaceNode->init();
//	return true;
//	
//}
//
//void KL3DFreeSurfaceShape::onColorMapChanged()
//{
//	m_FreeSurfaceNode->setFunction(pfn);
//	m_FreeSurfaceNode->resetTexture();
//}
//
//END_KLDISPLAY3D_NAMESPACE
//
//
