#include "KL3DVolumeDataManager.h"
#include "KL3DOrthoSliceShape.h"

#include "KL3DSliceDragger.h"
#include "KL3DDataModel/KL3DVolumeDataAdapter.h"
#include "KL3DDataModel/KLOctreeNode.h"
#include "KL3DSliceNode.h"
#include "KL3DSliceDragger.h"

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DOrthoSliceShape::KL3DOrthoSliceShape(int direct, KL3DSliceSet* sliceset):m_sliceSet(sliceset)
{
	m_direct = direct;
	m_switch = new osg::Switch;
	m_switch->setAllChildrenOn();
	m_sliceNode = NULL;
	m_dataManager = NULL;
	m_sliceNode = new KL3DSliceNode;
	m_sliceNode->setSliceSet(m_sliceSet);
	m_sliceNode->setDirection(direct);
	
}


KL3DOrthoSliceShape::~KL3DOrthoSliceShape(void)
{
	delete m_sliceNode;
	m_sliceNode = NULL;
}


void KL3DOrthoSliceShape::initShape()
{
	
	initData();
	initSlice();

}

osg::ref_ptr<osg::Switch> KL3DOrthoSliceShape::getShapeNode()
{
	return m_switch;
}

KL3DSliceNode * KL3DOrthoSliceShape::getSliceNode()
{
	return m_sliceNode;
}
void KL3DOrthoSliceShape::initData()
{
	if (!m_dataManager)
	{
		return;
	}
	KL3DVolumeDataAdapter * adapter = m_dataManager->getDataAdapter();
	int vBound[3];
	vBound[0] = adapter->getVolumeHeader().numFirst;
	vBound[1] = adapter->getVolumeHeader().numSecond;
	vBound[2] = adapter->getVolumeHeader().numThird;

	for(int i = 0;i < 3;++i)
	{
		m_bounds[i] = 0;
	}
	m_bounds[3] = vBound[0];
	m_bounds[4] = vBound[1];
	m_bounds[5] = vBound[2];
	m_sliceNode->setBounds(m_bounds);

	//m_sliceNode->setBasicNodeMap(m_dataManager->getBasicMap());

	//std::list<KLOctreeNode *> nodeList;
	//nodeList.push_back(m_dataManager->getOctree()->getRootNode());//得到根节点，并生成相应级别的节点列表。根节点level为0.
	//KLOctreeNode * firstNode = nodeList.front();

	//while(firstNode->m_level < BASIC_LEVEL)
	//{
	//	 nodeList.pop_front();
	//	 //注意，有些节点不一定有八个子节点，需作判断
	//	 for (int i = 0;i < 8;i++)
	//	 {
	//		 if (firstNode->m_children[i])
	//		 {
	//			 nodeList.push_back(firstNode->m_children[i]);
	//		 }
	//	 }
	//	 firstNode = nodeList.front();
	//}
	//std::list<osg::ref_ptr<osg::StateSet> > stateList = m_dataManager->generateStateSetList(nodeList);
	//
	//std::list<KLOctreeNode *>::iterator nItr;
	//std::list<osg::ref_ptr<osg::StateSet> >::iterator sItr;
	//for (nItr = nodeList.begin(),sItr = stateList.begin();nItr != nodeList.end();nItr++,sItr++)
	//{
	//	m_sliceNode->getBasicNodeMap().insert(std::make_pair((*nItr),(*sItr)));
	//}
	//	m_sliceNode->getbBasicNodeMap().insert(std::make_pair(nodeList.front(),stateList.front()));
}

void KL3DOrthoSliceShape::initSlice()
{	
	float normal[3] = {0};
	normal[m_direct] = 1;

	float midX = (float)(m_bounds[3] - m_bounds[0])/2.0;
	float midY = (float)(m_bounds[4] - m_bounds[1])/2.0;
	float midZ = (float)(m_bounds[5] - m_bounds[2])/2.0;
	m_sliceNode->setDataManager(m_dataManager);
	m_sliceNode->setOrigin(midX,midY,midZ);
	m_sliceNode->setNormal(normal);

	osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
	osg::ref_ptr<KL3DSliceDragger> dragger = new KL3DSliceDragger;
	dragger->setSlice(m_sliceNode);
	dragger->addTransformUpdating(transform/*,osgManipulator::DraggerTransformCallback::HANDLE_TRANSLATE_IN_LINE*/);

	dragger->setHandleEvents(true);
	dragger->setStateSet(this->getShapeNode()->getStateSet());
	m_switch->addChild(dragger);
	//	m_switch->addChild(m_sliceNode);

	

	m_sliceNode->initDraw();



	//modified
	//sliceRefineUpdateCallback *sliceRefineCbk = new sliceRefineUpdateCallback();
	//m_sliceNode->setUpdateCallback(sliceRefineCbk);

	//这个thread有内存泄漏
	//CUPsliceRefineThread * thread = new CUPsliceRefineThread();
	//m_sliceNode->setRefineSliceThread(thread);
	//thread->setDataManager(m_dataManager);
	//thread->setNodeStateMap(m_dataManager->getBasicMap());
	//thread->setRefinedNodeStateMap(m_sliceNode->getRefinedNodeMap());
	//thread->startThread();

	//test
	//m_sliceNode->setDataManager(m_dataManager);
	//sliceRefineCbk->setViewer(m_viewer);
}

void KL3DOrthoSliceShape::setDirect(int d)
{
	if (d!=0 && d!=1 && d !=2 )//非法值，默认x方向
	{
		m_direct = 0;
	}
	else m_direct = d;
	switch (d)
	{
	case 0:
		m_sliceNode->setNormal(1,0,0);
		break;
	case 1:
		m_sliceNode->setNormal(0,1,0);
		break;
	case 2:
		m_sliceNode->setNormal(0,0,1);
		break;
	default:
		m_sliceNode->setNormal(1,0,0);
	}
}

END_KLDISPLAY3D_NAMESPACE