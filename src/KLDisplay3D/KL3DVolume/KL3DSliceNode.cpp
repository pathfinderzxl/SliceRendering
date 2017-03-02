#include "KL3DSliceNode.h"
#include <osg/Texture3D>
#include <iostream>
#include <osg/LineWidth>
#include <vector>

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include "KL3DDataModel/KLOctreeNode.h"
#include "KL3DSliceSet.h"

BEGIN_KLDISPLAY3D_NAMESPACE
int KL3DSliceNode::m_activeNum = 0;
KL3DSliceNode::KL3DSliceNode(void)
{
	PolygonBuffer = new float[36];
	IntersectionBuffer = new float[12];
	m_highLightGeode = new osg::Geode;
	m_sliceGeode = new osg::Geode;
	this->addChild(m_highLightGeode);
	this->addChild(m_sliceGeode);

	m_isActive = false;
	m_isInteractive = false;
}


KL3DSliceNode::~KL3DSliceNode(void)
{
	if (PolygonBuffer != NULL)
	{
		delete []PolygonBuffer;
		PolygonBuffer = NULL;
	}
	
	if (NULL != IntersectionBuffer)
	{
		delete []IntersectionBuffer;
		IntersectionBuffer = NULL;
	}
	
}


int KL3DSliceNode::getDirection()
{
	return m_direct;
}

void KL3DSliceNode::setNormal(float x,float y,float z)
{
	float norm[3] = {x,y,z};
	setNormal(norm);
}
void KL3DSliceNode::setNormal(float * norm)
{
	if(!norm)
		return;
	for (int i = 0;i < 3;i++)
	{
		m_normal[i] = norm[i];
		if (norm[i] == 1)
		{
			m_direct = i;
		}
	}
}


void KL3DSliceNode::setOrigin(float x,float y,float z)
{
	m_origin[0] = x;
	m_origin[1] = y;
	m_origin[2] = z;
}
void KL3DSliceNode::setOrigin(float * orig)
{
	if(!orig)
		return;
	for (int i = 0;i < 3;i++)
	{
		m_origin[i] = orig[i];
	}
}

void KL3DSliceNode::updateDrag()
{
	//m_refineThread->stopRefine();//不进行细化
	m_isInteractive = true;
	updateDraw();
	
	list<KLOctreeNode*> nodelist = m_pDataManager->getDisplayList()->getBasicSliceNodeList();
	updateSlices(nodelist, false);
}

void KL3DSliceNode::updateRelease()
{
	/*m_pDataManager->getDisplayList()
	->generateRefineSliceNodeList(m_pDataManager->getOctree(), 
	this->m_direct, m_origin[m_direct], 3);*/

	m_pDataManager->getDisplayList()
		->refineSliceNodeList(m_pDataManager->getOctree(),
								this->m_direct,
								m_pDataManager->getViewer(), m_origin[m_direct]);

	list<KLOctreeNode*> nodelist = m_pDataManager->getDisplayList()->getRefineSliceNodeList();
	m_activeNum++;
	updateSlices(nodelist, true);

	//m_refineThread->startRefine(m_direct,m_origin[m_direct]);//唤醒细化线程
	//updateDraw();
	//
	//	bool flag = true;
	//	bool drawFlag = false;
	//	map<KLOctreeNode*, osg::ref_ptr<osg::StateSet>> tmpMap;
	//	while (m_refineThread->getState())
	//	{
	//		{
	//			OpenThreads::ScopedLock<OpenThreads::Mutex> 
	//				mutlock(m_refineThread->m_refineMapListMutex);
	//			if (m_refineThread->m_refineMapList.size() > 0)
	//			{
	//				tmpMap = m_refineThread->m_refineMapList.front();
	//				m_refineThread->m_refineMapList.pop_front();
	//				drawFlag = true;
	//			}
	//			
	//		}

	//		if(drawFlag)
	//			updateSlices(tmpMap, flag);
	//		tmpMap.clear();
	//		flag = false;
	//		drawFlag = false;
	//	}


}

void KL3DSliceNode::initDraw()
{
	updateDraw();
	list<KLOctreeNode*> nodelist = m_pDataManager->getDisplayList()->getBasicSliceNodeList();
	updateSlices(nodelist, false);
}

void KL3DSliceNode::updateSlices(std::list<KLOctreeNode *> nodelist, bool flag)
{
	//m_pDataManager->getTextureObjecCache()->m_pTexObjManagerThread->setNodeListInThread(nodelist);
	m_sliceGeode->removeDrawables(0, m_sliceGeode->getNumDrawables());
	std::list<KLOctreeNode*>::iterator itr = nodelist.begin();
	
	float bounds[6];//用于计算多边形顶点
	double n1 = 0, n2 = 0;
	//获取切片组
	m_pSliceSet->getSliceNodes(m_sliceNodeVec);

	for (; itr!=nodelist.end(); ++itr)
	{
		KLOctreeNode *node = *itr;
		node->getNodeBounds(bounds);

		if (m_origin[m_direct] < bounds[m_direct] || m_origin[m_direct] > bounds[m_direct+3])
		{
			continue;
		}

		
		osg::ref_ptr<osg::Vec3Array> verteces = new osg::Vec3Array;
		osg::ref_ptr<osg::Vec3Array> texArray = new osg::Vec3Array;
		

		computeBigPolygon(verteces, texArray, bounds);
		if (verteces->size()<3)
		{
			continue;
		}

		osg::ref_ptr<osg::Geometry> geom2 = new osg::Geometry;
		osg::ref_ptr<osg::StateSet> state = geom2->getOrCreateStateSet();
		//osg::ref_ptr<osg::Texture3D> texture3D = m_pDataManager->getNodeTextureFromMap(node);
		//osg::Texture3D* texture3D = m_pDataManager->getNodeTextureFromMap(node);
		//osg::ref_ptr<osg::Texture3D> texture3D = m_pDataManager->createTexture(node);
		osg::ref_ptr<osg::Texture3D> texture3D = m_pDataManager->getTextureFromMap(node, nodelist);
		texture3D->m_interactivNum = m_activeNum;
		if(flag)
		{
			osg::TextureCacheRecord *texRecord = texture3D->m_pTextureCacheRecord;
			map<osg::Texture3D*, osg::TextureItem*> stableArea = texRecord->m_stableArea;
			map<osg::Texture3D*, osg::TextureItem*> activeArea = texRecord->m_activeArea;
			map<osg::Texture3D*, osg::TextureItem*> unuseArea = texRecord->m_unuseArea;
			map<osg::Texture3D*, osg::TextureItem*>::iterator itr1, itr2, itr3;
			itr1 = stableArea.find(texture3D);
			itr2 = activeArea.find(texture3D);
			itr3 = unuseArea.find(texture3D);
			//cout<<stableArea.size()<<" "<<activeArea.size()<<" "<<unuseArea.size()<<endl;
			if (itr1!=stableArea.end()
				||itr2!=activeArea.end()
				||itr3!=unuseArea.end())
			{
				n1++;
			}
			else
			{
				n2++;
			}


		}
		
		texture3D->m_isInteractive = m_isInteractive;
		std::vector<KL3DSliceNode*>::iterator itr;
		std::vector<float> tmpVec;
		//cout<<texture3D->m_distance.empty()<<"  "<<texture3D->m_distance.size()<<endl;
		{
			//OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(texture3D->m_distanceMutex);
			/*if (texture3D->m_distance.size()>0)
			{
				(texture3D->m_distance).clear();
			}*/
		}
		
		m_pSliceSet->getSliceNodes(m_sliceNodeVec);
		//计算体块到每个切片的距离
		for (itr=m_sliceNodeVec.begin(); itr!=m_sliceNodeVec.end(); ++itr)
		{
			int x = m_sliceNodeVec.size();
			KL3DSliceNode *tmpSliceNode = *itr;
			int direct = tmpSliceNode->getDirection();
			float mid = (bounds[direct] + bounds[direct+3])/2;
			float distance = abs(mid - tmpSliceNode->getOrigin()[direct]);
			//cout<<distance<<endl;
			if (tmpSliceNode->isSliceActive())
			{
				//OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(texture3D->m_distanceMutex);
				//texture3D->m_distance.push_back(distance);
				texture3D->m_distanceA[0] = distance;
			}
			else
			{
				tmpVec.push_back(distance);
			}
		}
		for (int i=0; i<tmpVec.size(); ++i)
		{
			//OpenThreads::ScopedLock<OpenThreads::Mutex> mutlock(texture3D->m_distanceMutex);
			//texture3D->m_distance.push_back(tmpVec[i]);
			texture3D->m_distanceA[i+1] = tmpVec[i];
		}
		texture3D->m_level = node->m_level;

		/*if (m_pDataManager->getViewer()->getCamera()->getGraphicsContext() 
			&& texture3D->getTextureObject(0) == NULL)
		{
			texture3D->apply(*(m_pDataManager->getViewer()->getCamera()
				->getGraphicsContext()->getState()));
		}
*/
		

		state->setTextureAttributeAndModes(0, texture3D.get());
		osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode();
		polygonMode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
		state->setAttribute( polygonMode.get(), 
								osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );

		m_sliceGeode->addDrawable(geom2);
		geom2->setVertexArray(verteces);
		geom2->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN,0,verteces->size()));
		geom2->setTexCoordArray(0,texArray);
		//geom2->setStateSet(state);
		osg::ref_ptr<osg::Geometry> geomPartSlice = new osg::Geometry;
		m_sliceGeode->addDrawable(geomPartSlice);
		geomPartSlice->setVertexArray(verteces);
		geomPartSlice->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,0,verteces->size()));
	}	
	m_isInteractive = false;
	if(flag)
		cout<<"TextureObj Hit : "<<n1/(n1+n2)<<endl;
	/*cout<<"AreaSize:"<<tex3d->m_pTextureCacheRecord->m_stableArea.size()<<" "
					<<tex3d->m_pTextureCacheRecord->m_activeArea.size()<<" "
					<<tex3d->m_pTextureCacheRecord->m_unuseArea.size()<<"\n";*/
}
void KL3DSliceNode::updateDraw()
{
	m_highLightGeode->removeDrawables(0,m_highLightGeode->getNumDrawables());	

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	m_highLightGeode->addDrawable(geom);

	osg::ref_ptr<osg::Vec3Array> verteces = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> texArray = new osg::Vec3Array;
	computeBigPolygon(verteces,texArray,m_bounds);

	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array;
	colorArray->push_back(osg::Vec4(1,1,1,1));
	geom->setColorArray(colorArray);
	geom->setColorBinding(osg::Geometry::BIND_OVERALL);
	geom->setVertexArray(verteces);
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,0,verteces->size()));
	osg::StateSet* stateset = geom->getOrCreateStateSet();
	stateset->setAttributeAndModes(new osg::LineWidth(3),osg::StateAttribute::ON);
	stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED);

	//updateSlices(nodeStateMap);
}

void KL3DSliceNode::computeBigPolygon(osg::Vec3Array * verAry,osg::Vec3Array * texAry,float * b)
{
	//每次使polygonBuffer和IntersectionBuffer复位,并且清空定点序列
	memset(IntersectionBuffer,0,36);
	memset(PolygonBuffer,0,12);
	if(!verAry->empty())
		verAry->clear();
	if(!texAry->empty())
		texAry->clear();
	float *bounds = b;
	float pNormal[3];
	float pOrigin[3];
	for (int i = 0;i < 3; ++i)
	{
		pNormal[i] = m_normal[i];
		pOrigin[i] = m_origin[i];
	}

	osg::Vec3f plane(pNormal[0],pNormal[1],pNormal[2]);

	int i, j, k;
	osg::Vec3d vertices[8];
	int idx = 0;

	for ( k = 0; k < 2; k++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( i = 0; i < 2; i++ )
			{
				vertices[idx][2] = bounds[2+3*k];
				vertices[idx][1] = bounds[1+3*j];
				vertices[idx][0] = bounds[0+3*i];
				idx++;
			}
		}
	}
	int lines[12][2] = 
	{ {0,1}, {1,3}, {2,3}, {0,2},
	{4,5}, {5,7}, {6,7}, {4,6},
	{0,4}, {1,5}, {3,7}, {2,6} };

	double t;
	osg::Vec3d line;
	for ( i = 0; i < 12; i++ )
	{

		line[0] = vertices[lines[i][1]][0] - vertices[lines[i][0]][0];
		line[1] = vertices[lines[i][1]][1] - vertices[lines[i][0]][1];
		line[2] = vertices[lines[i][1]][2] - vertices[lines[i][0]][2];

		float planeDotLine = pNormal[0] * line[0] + pNormal[1] * line[1] +pNormal[2] * line[2];
		float planeDotOrigin = pNormal[0] *(pOrigin[0] - vertices[lines[i][0]][0])
			+pNormal[1] *(pOrigin[1] - vertices[lines[i][0]][1])
			+pNormal[2] *(pOrigin[2] - vertices[lines[i][0]][2]);

		if (planeDotLine !=0.0)
		{
			t = planeDotOrigin /planeDotLine;//注意，这没绝对值！！
		}
		else
		{
			t = -1.0;
		}
		IntersectionBuffer[i] = (t > 0.0 && t < 1.0)?(t):(-1.0);
	}

	int neighborLines[12][6] = 
	{ {  1,  2,  3,  4,  8,  9}, {  0,  2,  3,  5,  9, 10},
	{  0,  1,  3,  6, 10, 11}, {  0,  1,  2,  7,  8, 11},
	{  0,  5,  6,  7,  8,  9}, {  1,  4,  6,  7,  9, 10},
	{  2,  4,  5,  7, 10, 11}, {  3,  4,  5,  6,  8, 11},
	{  0,  3,  4,  7,  9, 11}, {  0,  1,  4,  5,  8, 10},
	{  1,  2,  5,  6,  9, 11}, {  2,  3,  6,  7,  8, 10} };

	float tCoord[12][4] = {
		{0, 0, 0, 0}, {1, 0, 0, 1}, {0, 1, 0, 0}, {0, 0, 0, 1},
		{0, 0, 1, 0}, {1, 0, 1, 1}, {0, 1, 1, 0}, {0, 0, 1, 1},
		{0, 0, 0, 2}, {1, 0, 0, 2}, {1, 1, 0, 2}, {0, 1, 0, 2}};

		for (i = 0; i < 12; i++)
		{
			tCoord[i][0] = tCoord[i][0] ? 1.0 : 0.0;
			tCoord[i][1] = tCoord[i][1] ? 1.0 : 0.0;
			tCoord[i][2] = tCoord[i][2] ? 1.0 : 0.0;
		}
		float *iptr = IntersectionBuffer;
		float *pptr = PolygonBuffer;

		int start = 0;
		//int current;
		while ( start < 12 && IntersectionBuffer[start] == -1.0 )//找到第一个相交的节点
		{
			start++;
		}
		if ( start == 12 )
		{
			pptr[0] = -1.0;
		}
		else
		{
			int current = start;
			int previous = -1;
			int errFlag = 0;

			idx   = 0;

			while ( idx < 6 && !errFlag && ( idx == 0 || current != start) )
			{
				double t = iptr[current];

				osg::Vec3 vert;
				osg::Vec3 texcoord;
				*(pptr + idx * 6)     = tCoord[current][0];
				*(pptr + idx * 6 + 1) = tCoord[current][1];
				*(pptr + idx * 6 + 2) = tCoord[current][2];

				int coord = static_cast<int>(tCoord[current][3]);
				*(pptr + idx * 6 + coord) = t;

				*(pptr + idx*6 + 3) = (	vertices[lines[current][0]][0] + 
					t*(vertices[lines[current][1]][0] - vertices[lines[current][0]][0]));

				*(pptr + idx*6 + 4) = (	vertices[lines[current][0]][1] + 
					t*(vertices[lines[current][1]][1] - vertices[lines[current][0]][1]));

				*(pptr + idx*6 + 5) = (	vertices[lines[current][0]][2] + 
					t*(vertices[lines[current][1]][2] - vertices[lines[current][0]][2]));

				vert.set(*(pptr + idx*6 + 3),*(pptr + idx*6 + 4),*(pptr + idx*6 + 5));
				verAry->push_back(vert);

				texcoord.set(*(pptr + idx*6 + 0),*(pptr + idx*6 + 1),*(pptr + idx*6 + 2));
				texAry->push_back(texcoord);

				idx++;
				j = 0;

				while ( j < 6 &&
					(*(IntersectionBuffer + 
					neighborLines[current][j]) < 0 || 
					neighborLines[current][j] == previous) ) 
				{
					j++;
				}

				if ( j >= 6 )
				{
					errFlag = 1;
				}
				else
				{
					previous = current;
					current = neighborLines[current][j];
				}
			}

			if ( idx < 6 )
			{
				*(pptr + idx*3) = -1;
			}
		}
}

void KL3DSliceNode::setRefineLevel(unsigned int level)
{
	m_refineLevel = level;
}

void KL3DSliceNode::setSliceSet(KL3DSliceSet* sliceset)
{
	this->m_pSliceSet = sliceset;
	m_pSliceSet->getSliceNodes(m_sliceNodeVec);
}

float * KL3DSliceNode::getNormal()
{
	return m_normal;
}
float * KL3DSliceNode::getOrigin()
{
	return m_origin;
}




void KL3DSliceNode::setBounds(float * bounds)
{
	if (bounds == NULL)
	{
		std::cout<<"ERROR--CUPSliceNode--setBounds---bounds = NULL"<<std::endl;
	}
	for (int i = 0;i < 6;i++)
	{
		m_bounds[i] = bounds[i];
	}
}

const float * KL3DSliceNode::getBounds()
{
	return m_bounds;
}

osg::Geode* KL3DSliceNode::drawSlice2Refine( std::list<KLOctreeNode*> nodelist )
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	std::list<KLOctreeNode*>::iterator itr = nodelist.begin();
	
	float bounds[6];//用于计算多边形顶点
	double n1 = 0, n2 = 0;
	//获取切片组
	m_pSliceSet->getSliceNodes(m_sliceNodeVec);

	for (; itr!=nodelist.end(); ++itr)
	{
		KLOctreeNode *node = *itr;
		node->getNodeBounds(bounds);

		if (m_origin[m_direct] < bounds[m_direct] || m_origin[m_direct] > bounds[m_direct+3])
		{
			continue;
		}

		
		osg::ref_ptr<osg::Vec3Array> verteces = new osg::Vec3Array;
		osg::ref_ptr<osg::Vec3Array> texArray = new osg::Vec3Array;
		

		computeBigPolygon(verteces, texArray, bounds);
		if (verteces->size()<3)
		{
			continue;
		}

		osg::ref_ptr<osg::Geometry> geom2 = new osg::Geometry;
		osg::ref_ptr<osg::StateSet> state = geom2->getOrCreateStateSet();
		osg::ref_ptr<osg::Texture3D> texture3D = m_pDataManager->getNodeTextureFromMap(node);
		texture3D->m_interactivNum = m_activeNum;
		//if(flag)
		{
			osg::TextureCacheRecord *texRecord = texture3D->m_pTextureCacheRecord;
			map<osg::Texture3D*, osg::TextureItem*> stableArea = texRecord->m_stableArea;
			map<osg::Texture3D*, osg::TextureItem*> activeArea = texRecord->m_activeArea;
			map<osg::Texture3D*, osg::TextureItem*> unuseArea = texRecord->m_unuseArea;
			map<osg::Texture3D*, osg::TextureItem*>::iterator itr1, itr2, itr3;
			itr1 = stableArea.find(texture3D);
			itr2 = activeArea.find(texture3D);
			itr3 = unuseArea.find(texture3D);
			//cout<<stableArea.size()<<" "<<activeArea.size()<<" "<<unuseArea.size()<<endl;
			if (itr1!=stableArea.end()
				||itr2!=activeArea.end()
				||itr3!=unuseArea.end())
			{
				n1++;
			}
			else
			{
				n2++;
			}


		}
		
		texture3D->m_isInteractive = m_isInteractive;
		std::vector<KL3DSliceNode*>::iterator itr;
		std::vector<float> tmpVec;
		/*{
			OpenThreads::ScopedLock<OpenThreads::Mutex>(texture3D->m_distanceMutex);
			texture3D->m_distance.clear();
		}*/
		m_pSliceSet->getSliceNodes(m_sliceNodeVec);
		//计算体块到每个切片的距离
		for (itr=m_sliceNodeVec.begin(); itr!=m_sliceNodeVec.end(); ++itr)
		{
			int x = m_sliceNodeVec.size();
			KL3DSliceNode *tmpSliceNode = *itr;
			int direct = tmpSliceNode->getDirection();
			int mid = (bounds[direct] + bounds[direct+3])/2;
			int distance = abs(mid - tmpSliceNode->getOrigin()[direct]);
			//cout<<distance<<endl;
			if (tmpSliceNode->isSliceActive())
			{
				OpenThreads::ScopedLock<OpenThreads::Mutex>(texture3D->m_distanceMutex);
				//texture3D->m_distance.push_back(distance);
				texture3D->m_distanceA[0] = distance;
			}
			else
			{
				tmpVec.push_back(distance);
			}
		}
		for (int i=0; i<tmpVec.size(); ++i)
		{
			OpenThreads::ScopedLock<OpenThreads::Mutex>(texture3D->m_distanceMutex);
			//texture3D->m_distance.push_back(tmpVec[i]);
			texture3D->m_distanceA[i+1] = tmpVec[i];
		}
		texture3D->m_level = node->m_level;	

		state->setTextureAttributeAndModes(0, texture3D);
		osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode();
		polygonMode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
		state->setAttribute( polygonMode.get(), 
								osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );

		geode->addDrawable(geom2);
		geom2->setVertexArray(verteces);
		geom2->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN,0,verteces->size()));
		geom2->setTexCoordArray(0,texArray);
		//geom2->setStateSet(state);
		osg::ref_ptr<osg::Geometry> geomPartSlice = new osg::Geometry;
		geode->addDrawable(geomPartSlice);
		geomPartSlice->setVertexArray(verteces);
		geomPartSlice->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,0,verteces->size()));
	}	
	m_isInteractive = false;
	return geode.get();
}

END_KLDISPLAY3D_NAMESPACE