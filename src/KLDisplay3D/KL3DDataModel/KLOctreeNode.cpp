#include "KLOctreeNode.h"

namespace KLSeis{
namespace KLDisplay3D{

//------------------------------------
KLOctreeNode::KLOctreeNode()
{
	m_parent = NULL;
	m_children[0] = m_children[1] = m_children[2] = m_children[3] = NULL;
	m_children[4] = m_children[5] = m_children[6] = m_children[7] = NULL;
	m_brother = NULL;

	m_numChild = 0;
	m_emptyChild = true;	
	m_sampleDistance[0] = m_sampleDistance[1] = m_sampleDistance[2] = 0;
	m_tileStep = 0;
	m_tileSize = BLOCKSIZE;
	m_level = 0;

	m_minSample = m_maxSample = m_sampleNum = -1;
	m_minCMP    = m_maxCMP    = m_CMPNum    = -1;
	m_minRecord = m_maxRecord = m_recordNum = -1;

	m_center[0] = m_center[1] = m_center[2] = -1;
	m_diagnose = 0;
	m_location = -1;
}

//-----------------------------------
KLOctreeNode::~KLOctreeNode()
{
	m_parent = NULL;
	m_brother = NULL;
	for ( int i = 0; i < 8; ++i )
	{
		m_children[i] = NULL;
	}

	/*if ( m_wei )
	{
	delete []m_wei;
	m_wei = NULL;
	}*/
}

void KLOctreeNode::getNodeBounds(float * bounds)
{
	if (bounds == NULL)
	{
		return;
	}
	bounds[0] = m_minSample;
	bounds[1] = m_minCMP;
	bounds[2] = m_minRecord;
	bounds[3] = m_maxSample;
	bounds[4] = m_maxCMP;
	bounds[5] = m_maxRecord;
}

}//end namespace KLDisplay3D
}//end namespace xin
