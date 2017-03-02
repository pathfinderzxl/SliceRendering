#include "SliceRefineThread.h"

BEGIN_KLDISPLAY3D_NAMESPACE

SliceRefineThread::SliceRefineThread(void)
{
}


SliceRefineThread::~SliceRefineThread(void)
{
}

void SliceRefineThread::run()
{
	while (!isCanceled)
	{
		list<KLOctreeNode*>::iterator itr = m_basicNodeList.begin();
		for (; itr!=m_basicNodeList.end(); ++itr)
		{

		}
	}
	
}

void SliceRefineThread::setBasicNodeList( list<KLOctreeNode*> nodeList, float pos)
{
	m_pos = pos;
	m_basicNodeList.clear();
	list<KLOctreeNode*>::iterator itr = nodeList.begin();
	for (; itr!=nodeList.end(); ++itr)
	{
		m_basicNodeList.push_back(*itr);
	}
}

END_KLDISPLAY3D_NAMESPACE