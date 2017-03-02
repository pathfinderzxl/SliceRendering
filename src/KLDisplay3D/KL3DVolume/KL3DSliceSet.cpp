#include "KL3DSliceSet.h"

using namespace std;

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DSliceSet::KL3DSliceSet(void)
{
}


KL3DSliceSet::~KL3DSliceSet(void)
{
}


void KL3DSliceSet::addToGroup(osg::ref_ptr<osg::Group> group)
{
	vector<KL3DBaseShape *>::iterator itr;

	for (itr = m_baseShapeSet.begin();itr != m_baseShapeSet.end();++itr)
	{
		group->addChild((*itr)->getShapeNode());
	}
}



void KL3DSliceSet::addSlice(KL3DBaseShape * baseShape)
{
	m_baseShapeSet.push_back(baseShape);
}

void KL3DSliceSet::getSliceNodes(std::vector<KL3DSliceNode *> &nodeList)
{
	if (nodeList.size() > 0)
	{
		nodeList.clear();
	}
	std::vector<KL3DBaseShape *>::iterator itr;
	KL3DOrthoSliceShape * sliceShape;
	KL3DSliceNode * sliceNode;
	//debug 
	int i = 0;
	int direct = 0;
	for (itr = m_baseShapeSet.begin();itr != m_baseShapeSet.end();++itr)
	{
		sliceShape = dynamic_cast<KL3DOrthoSliceShape *> (*itr);
		if (sliceShape)
		{

			sliceNode = sliceShape->getSliceNode();
			direct = sliceNode->getDirection();
			nodeList.push_back(sliceNode);
			//std::cout<<"ÇÐÆ¬"<<++i<<":·½Ïò"<<direct<<" Î»ÖÃ"<<sliceNode->getOrigin()[direct]<<endl;
		}
	}
}

END_KLDISPLAY3D_NAMESPACE