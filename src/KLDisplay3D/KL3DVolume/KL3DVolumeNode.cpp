
#include "KL3DVolumeNode.h"
#include "qfile.h"
#include "qdatastream.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Vec3d>
#include <osg/Texture3D>
#include <osg/PolygonMode>

#include <iostream>
#include "float.h"
#include <omp.h>

#include "KL3DDataModel/KL3DVOctree.h"


BEGIN_KLDISPLAY3D_NAMESPACE


	int KL3DVolumeNode::texInCache = 0;

KL3DVolumeNode::KL3DVolumeNode()
{
	m_volumeData = NULL;
	//m_viewer = NULL;
	m_octree = NULL;
	m_cache = NULL;


	m_volumeBounds[0] = osg::Vec3(0.0, 0.0, 0.0);
	m_volumeBounds[1] = osg::Vec3(1.0, 1.0, 1.0);

	m_bufferSize = 0;
	m_intersectionBuffer = NULL;
	m_polygonBuffer = NULL;
	m_numOfPolygon = 0;
	m_sampleDistance[0] = m_sampleDistance[1] = m_sampleDistance[2] = 1.0;

	m_colorMapChanged = true;

	this->pfn = NULL;
}
KL3DVolumeNode::~KL3DVolumeNode()
{
	if (m_intersectionBuffer)
	{
		delete[]m_intersectionBuffer;
	}
	if (m_polygonBuffer)
	{
		delete[]m_polygonBuffer;
	}
}
void KL3DVolumeNode:: reinit()
{
	m_volumeData = NULL;
	//m_viewer = NULL;
	m_octree = NULL;
	m_cache = NULL;


	m_volumeBounds[0] = osg::Vec3(0.0, 0.0, 0.0);
	m_volumeBounds[1] = osg::Vec3(1.0, 1.0, 1.0);

	m_bufferSize = 0;
	m_intersectionBuffer = NULL;
	m_polygonBuffer = NULL;
	m_numOfPolygon = 0;
	m_sampleDistance[0] = m_sampleDistance[1] = m_sampleDistance[2] = 1.0;

	m_colorMapChanged = true;

	this->pfn = NULL;
}

double computeDistance(const osg::Vec3d &point1, double *point2)
{
	return sqrt((point1[0] - point2[0]) * (point1[0] - point2[0])
		+ (point1[1] - point2[1]) * (point1[1] - point2[1])
		+ (point1[2] - point2[2]) * (point1[2] - point2[2]));
}

// �ӿ����˳��
void KL3DVolumeNode::setLoadOrder(KLOctreeNode *node, osg::Vec3 cameraPosition, int order[8])
{
	KLOctreeNode *p = node->m_children[0];
	if (cameraPosition[0] >= p->m_maxSample && cameraPosition[1] >= p->m_maxCMP && cameraPosition[2] >= p->m_maxRecord)
	{
		order[0] = 7; order[1] = 3; order[2] = 5; order[3] = 6;
		order[4] = 1; order[5] = 2; order[6] = 4; order[7] = 0;
	}
	else if (cameraPosition[0] < p->m_maxSample && cameraPosition[1] >= p->m_maxCMP && cameraPosition[2] >= p->m_maxRecord)
	{
		order[0] = 6; order[1] = 2; order[2] = 4; order[3] = 7;
		order[4] = 0; order[5] = 3; order[6] = 5; order[7] = 1;
	}
	else if (cameraPosition[0] >= p->m_maxSample && cameraPosition[1] < p->m_maxCMP && cameraPosition[2] >= p->m_maxRecord)
	{
		order[0] = 5; order[1] = 1; order[2] = 4; order[3] = 7;
		order[4] = 0; order[5] = 3; order[6] = 6; order[7] = 2;
	}
	else if (cameraPosition[0] < p->m_maxSample && cameraPosition[1] < p->m_maxCMP && cameraPosition[2] >= p->m_maxRecord)
	{
		order[0] = 4; order[1] = 0; order[2] = 5; order[3] = 6;
		order[4] = 1; order[5] = 2; order[6] = 7; order[7] = 3;
	}
	else if (cameraPosition[0] >= p->m_maxSample && cameraPosition[1] >= p->m_maxCMP && cameraPosition[2] < p->m_maxRecord)
	{
		order[0] = 3; order[1] = 1; order[2] = 2; order[3] = 7;
		order[4] = 0; order[5] = 5; order[6] = 6; order[7] = 4;
	}
	else if (cameraPosition[0] < p->m_maxSample && cameraPosition[1] >= p->m_maxCMP && cameraPosition[2] < p->m_maxRecord)
	{
		order[0] = 2; order[1] = 0; order[2] = 3; order[3] = 6;
		order[4] = 1; order[5] = 4; order[6] = 7; order[7] = 5;
	}
	else if (cameraPosition[0] >= p->m_maxSample && cameraPosition[1] < p->m_maxCMP && cameraPosition[2] < p->m_maxRecord)
	{
		order[0] = 1; order[1] = 0; order[2] = 3; order[3] = 5;
		order[4] = 2; order[5] = 4; order[6] = 7; order[7] = 6;
	}
	else if (cameraPosition[0] < p->m_maxSample && cameraPosition[1] < p->m_maxCMP && cameraPosition[2] < p->m_maxRecord)
	{
		order[0] = 0; order[1] = 1; order[2] = 2; order[3] = 4;
		order[4] = 3; order[5] = 5; order[6] = 6; order[7] = 7;
	}
}

// �����ӵ�ѡ����ʾ�б�
void KL3DVolumeNode::updateNodeList(osg::Vec3 cameraPosition)
{
	// С����û�����ɰ˲���������Ҫ�����б�
	if (m_octree == NULL)
		return ;

	std::list<KLOctreeNode *> tmpList;
	int num = 0, order[8];

	m_octreeNodeList.clear();
	KLOctreeNode *node = m_octree->m_rootNode;
	m_octreeNodeList.push_back(node);

	// �Ӹ��ڵ㿪ʼ����������˲������ڵ�
	for (int loop = 0; loop < m_octree->m_depth; ++loop)
	{
		num = m_octreeNodeList.size();

		// ���ӵ���������ֱ������
		// �ӵ������ľ���ﵽĳһ��ֵ������鼴Ϊ���ʷֱ���
		if (computeDistance(cameraPosition, m_octreeNodeList.front()->m_center)
			>= m_octreeNodeList.front()->m_diagnose * 10.5)
			return ;

		tmpList = m_octreeNodeList;
		m_octreeNodeList.clear();

		KLOctreeNode *firstNode = tmpList.front(), *tmpNode;
		std::list<KLOctreeNode *>::iterator itrNode;
		for (itrNode = tmpList.begin(); itrNode != tmpList.end(); ++itrNode)
		{
			tmpNode = *itrNode;

			// �ӵ�λ����������λ�ù�ϵ��27�֣�������6���潫�ռ��Ϊ27�����֣��ӵ��Ӧ��27��λ��
			// ����������ӵ��27��λ�ù�ϵ����Ӧ�Ĳ��ɼ������27�����
			if ((*itrNode)->m_level < loop
				|| ((cameraPosition[0] > node->m_maxSample && tmpNode->m_center[0] < firstNode->m_minSample 
				|| cameraPosition[0] < node->m_minSample && tmpNode->m_center[0] > firstNode->m_maxSample
				|| cameraPosition[0] <= node->m_maxSample && cameraPosition[0] >= node->m_minSample)
				&& (cameraPosition[1] > node->m_maxCMP && tmpNode->m_center[1] < firstNode->m_minCMP
				|| cameraPosition[1] < node->m_minCMP && tmpNode->m_center[1] > firstNode->m_maxCMP
				|| cameraPosition[1] <= node->m_maxCMP && cameraPosition[1] >= node->m_minCMP)
				&& (cameraPosition[2] > node->m_maxRecord && tmpNode->m_center[2] < firstNode->m_minRecord
				|| cameraPosition[2] < node->m_minRecord && tmpNode->m_center[2] > firstNode->m_maxRecord
				|| cameraPosition[2] <= node->m_maxRecord && cameraPosition[2] >= node->m_minRecord)))
			{
				m_octreeNodeList.push_back(tmpNode); // ���ɼ����
			}
			else 
			{
				// ���ֱ��ʺ���
				if (computeDistance(cameraPosition, tmpNode->m_center) >= tmpNode->m_diagnose * 1.5) 
					m_octreeNodeList.push_back(tmpNode);
				else if (num - 1 + tmpNode->m_numChild <= 100) // �ڴ����ƣ��Ƿ񳬹���������
				{
					// ���ֱ���С���ڴ���㣬��߷ֱ���
					setLoadOrder(tmpNode, cameraPosition, order);
					for (int j = 0; j < 8; ++j)
						if (tmpNode->m_children[order[j]])
							m_octreeNodeList.push_back(tmpNode->m_children[order[j]]);
					num += tmpNode->m_numChild;
				}
				else 
				{
					// ���ֱ���С���ڴ�ﵽ���ޣ�����ϸ��
					for (; itrNode != tmpList.end(); ++itrNode)
						m_octreeNodeList.push_back(*itrNode);
					return ;
				}
			}
		}
	}
}


bool KL3DVolumeNode::checkStatesetList(KLOctreeNode *node)
{

	std::list<KLOctreeNode *>::iterator itrnode;
	itrnode = find(m_nodeInStatesetCache.begin(), m_nodeInStatesetCache.end(), node);
	if (itrnode == m_nodeInStatesetCache.end())
	{
		return false;
	}
	else 
	{
		// ����ڻ����У�������Ƶ���β�� LRU
		m_nodeInStatesetCache.erase(itrnode);
		m_nodeInStatesetCache.push_back(node);
		return true;
	}
}

osg::ref_ptr<osg::StateSet> KL3DVolumeNode::getStateSet(KLOctreeNode *node, int dim[3])
{
	osg::ref_ptr<osg::StateSet> state = new osg::StateSet;

	if (checkStatesetList(node))
	{
		state = m_mapNodeAndStateset[node];

		// ����
		//++texInCache;
	}
	else 
	{
		float *data = m_cache->getNodeDataFromMap(node);
		state = createState(dim, data);
		if (m_nodeInStatesetCache.size() >= 200)
		{
			std::map<KLOctreeNode *, osg::StateSet *>::iterator itrMap;
			std::list<KLOctreeNode *>::iterator itrNode;
			KLOctreeNode *tmpNode;
			osg::StateSet *tmpState;

			bool flag = TRUE;
			while (flag)
			{
				tmpNode = m_nodeInStatesetCache.front();
				itrMap = m_mapNodeAndStateset.find(tmpNode);
				m_nodeInStatesetCache.pop_front();
				itrNode = find(m_octreeNodeList.begin(), m_octreeNodeList.end(), tmpNode);
				if (itrNode == m_octreeNodeList.end())
				{
					tmpState = itrMap->second;
					tmpState->unref();
					m_mapNodeAndStateset.erase(itrMap);
					flag = FALSE;
					break ;
				}
				m_nodeInStatesetCache.push_back(tmpNode);
			}
		}
		m_nodeInStatesetCache.push_back(node);
		m_mapNodeAndStateset.insert(std::make_pair(node, state));
		state->ref();//�������ü���,���������������һ��ָ��������
		//state->re
	}

	return state.get();
}
// �����ʼ��
void KL3DVolumeNode::initialize()
{
	if (m_volumeData)//С����
	{
		osg::Geode *vol = new osg::Geode;
		int dim[3] =  { m_volHeader.numFirst,m_volHeader.numSecond,m_volHeader.numThird};
		vol->setStateSet(createState(dim, m_volumeData));
		const int *order = m_volHeader.orderXYZ;

		//ת����� �����������Ĳ�������
		int dimtrans[3] = {dim[ order[0]],dim[ order[1]],dim[ order[2]]};
		int sampleDistancetrans[3] = {m_sampleDistance[order[0]],m_sampleDistance[order[1]],m_sampleDistance[order[2]]};
		for (int i = 0; i < dimtrans[1]; ++i)
		{
			osg::Geometry* polygon = new osg::Geometry();
			vol->addDrawable(polygon);

			// ����ζ�����������
			osg::Vec3Array* tcoords = new osg::Vec3Array(4);
			(*tcoords)[0].set(0.0f, (float)i / (dimtrans[1] - 1), 0.0f);
			(*tcoords)[1].set(1.0f, (float)i / (dimtrans[1] - 1), 0.0f);		
			(*tcoords)[2].set(1.0f, (float)i / (dimtrans[1] - 1), 1.0f);
			(*tcoords)[3].set(0.0f, (float)i / (dimtrans[1] - 1), 1.0f);		
			polygon->setTexCoordArray(0, tcoords);

			// ����ζ�������
			osg::Vec3Array* polygonVertices = new osg::Vec3Array;
			polygonVertices->push_back(osg::Vec3(
				m_volumeBounds[0][0], (float)(i * sampleDistancetrans[1] + m_volumeBounds[0][1]), m_volumeBounds[0][2]));
			polygonVertices->push_back(osg::Vec3(
				m_volumeBounds[1][0], (float)(i * sampleDistancetrans[1] + m_volumeBounds[0][1]), m_volumeBounds[0][2]));
			polygonVertices->push_back(osg::Vec3(
				m_volumeBounds[1][0], (float)(i * sampleDistancetrans[1] + m_volumeBounds[0][1]), m_volumeBounds[1][2]));
			polygonVertices->push_back(osg::Vec3(
				m_volumeBounds[0][0], (float)(i * sampleDistancetrans[1] + m_volumeBounds[0][1]), m_volumeBounds[1][2]));
			polygon->setVertexArray(polygonVertices);

			// �������TRIANGLE_FAN����
			osg::DrawElementsUInt* polygonPS = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_FAN, 0);
			polygonPS->push_back(0);
			polygonPS->push_back(1);
			polygonPS->push_back(2);
			polygonPS->push_back(3);
			polygon->addPrimitiveSet(polygonPS);
		}

		addChild(vol);

		return ;
	}
	//�����ݳ�ʼ��
	KLOctreeNode *node;

	std::list<KLOctreeNode *>::iterator itrNode;
	for (itrNode = m_octreeNodeList.begin(); itrNode != m_octreeNodeList.end(); ++itrNode)
	{
		node = *itrNode;
		const int *order = m_volHeader.orderXYZ;
		double temp_bounds[6];
		getVolumeNodeBoundsXYZ(node,temp_bounds);

		osg::Vec3 bounds[2] = {
			osg::Vec3(temp_bounds[0], temp_bounds[2], temp_bounds[4]),
			osg::Vec3(temp_bounds[1], temp_bounds[3], temp_bounds[5])
		};

		m_sampleDistance[0] = node->m_sampleDistance[0]; 
		m_sampleDistance[1] = node->m_sampleDistance[1];
		m_sampleDistance[2] = node->m_sampleDistance[2];
		float sampleDistancetrans[3] = {m_sampleDistance[order[0]],m_sampleDistance[order[1]],m_sampleDistance[order[2]]};

		// �����������Ĳ�������
		int dim[3];

		dim[0] = node->m_sampleNum;
		dim[1] = node->m_CMPNum;
		dim[2] = node->m_recordNum;

		//ת����� �����������Ĳ�������
		int dimtrans[3] = {dim[ order[0]],dim[ order[1]],dim[ order[2]]};


		osg::ref_ptr<osg::StateSet> state = getStateSet(node, dim);

		// �������ΪҶ�ڵ�ӵ�������
		osg::Geode *block = new osg::Geode();
		block->setStateSet(state);

		// ����������ߴ�ֱ�Ķ������Ƭ��ɣ�ÿ����Ƭ��Ӧһ��drawable
		for (int i = 0; i < dimtrans[1]; ++i)
		{
			osg::Geometry* polygon = new osg::Geometry();
			block->addDrawable(polygon);

			// ����ζ�����������
			osg::Vec3Array* tcoords = new osg::Vec3Array(4);
			(*tcoords)[0].set(0.0f, (float)i / (dimtrans[1] - 1), 0.0f);
			(*tcoords)[1].set(1.0f, (float)i / (dimtrans[1] - 1), 0.0f);		
			(*tcoords)[2].set(1.0f, (float)i / (dimtrans[1] - 1), 1.0f);
			(*tcoords)[3].set(0.0f, (float)i / (dimtrans[1] - 1), 1.0f);	
			polygon->setTexCoordArray(0, tcoords);

			// ����ζ�������
			osg::Vec3Array* polygonVertices = new osg::Vec3Array;
			polygonVertices->push_back(osg::Vec3(
				bounds[0][0], (float)(i * sampleDistancetrans[1] + bounds[0][1]), bounds[0][2]));
			polygonVertices->push_back(osg::Vec3(
				bounds[1][0], (float)(i * sampleDistancetrans[1] + bounds[0][1]), bounds[0][2]));
			polygonVertices->push_back(osg::Vec3(
				bounds[1][0], (float)(i * sampleDistancetrans[1] + bounds[0][1]), bounds[1][2]));
			polygonVertices->push_back(osg::Vec3(
				bounds[0][0], (float)(i * sampleDistancetrans[1] + bounds[0][1]), bounds[1][2]));
			polygon->setVertexArray(polygonVertices);

			// �������TRIANGLE_FAN����
			osg::DrawElementsUInt* polygonPS = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_FAN, 0);
			polygonPS->push_back(0);
			polygonPS->push_back(1);
			polygonPS->push_back(2);
			polygonPS->push_back(3);
			polygon->addPrimitiveSet(polygonPS);
		}

		// ������ӦҶ�ڵ�ӵ�������
		addChild(block);		
	}
}


// ��ά����£����¼�����ʾ�б���������
void KL3DVolumeNode::updateVolume(osg::Vec3 cameraPosition, osgViewer::Viewer* viewer, int depth)
{
	//�ӵ㷢���仯��С������ʾ����
	if (m_volumeData)
	{
		osg::Geode* vol = (osg::Geode *)this->getChild(0);

		if (m_colorMapChanged)//���ɫ�����仯������Ҫ����״̬
		{
			int dim[3] = { m_volHeader.numFirst,m_volHeader.numSecond,m_volHeader.numThird};
			vol->setStateSet(createState(dim, m_volumeData));
			m_colorMapChanged = false;
		}
		////С���ݵķ�Χ��ֱ���ɶ������ת��ķ�Χ
		computePolygon(cameraPosition, m_volumeBounds);
		vol->removeDrawables(0, vol->getNumDrawables());

		for (int i = 0; i < m_numOfPolygon; ++i)
		{
			osg::Vec3Array* polygonVertices = new osg::Vec3Array;
			osg::Vec3Array* tcoords = new osg::Vec3Array;

			float *ptr = m_polygonBuffer + 36 * i;
			int j;

			// �õ���Ƭ��Ӧ������������������
			for (j = 0; j < 6; ++j)
			{
				if ( ptr[0] < 0.0 )
					break ;

				polygonVertices->push_back(osg::Vec3(ptr[3], ptr[4], ptr[5]));
				tcoords->push_back(osg::Vec3(ptr[0], ptr[1], ptr[2]));

				ptr += 6;
			}

			if (j < 3)
				continue ;

			osg::ref_ptr<osg::Geometry> polygon = new osg::Geometry();
			vol->addDrawable(polygon);

			polygon->setVertexArray(polygonVertices);
			polygon->setTexCoordArray(0, tcoords);

			osg::DrawElementsUInt* polygonPS = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_FAN, 0);
			// j��Ӧ��Ƭ�����Ľ������
			for (int k = 0; k < j; ++k)
			{
				polygonPS->push_back(k);
			}
			polygon->addPrimitiveSet(polygonPS);
		}

		return ;
	}
	//��������ʾ����
	removeChildren(0, this->getNumChildren());

	// ������ʾ�б�
	if ( depth == 0)
	{
		m_displayList->updateDisplayList(m_octree,cameraPosition, viewer);
		m_octreeNodeList = m_displayList->getDisplayNodeList();
	}
	else
	{
		m_displayList->updateList(m_octree,cameraPosition, viewer,depth);
		m_octreeNodeList = m_displayList->getDisplayNodeList();
	}



	/*m_octreeNodeList.push_back(m_octree->m_rootNode);*/
	//std::cout<<"��ʾ�б�ĳ��ȣ�"<<m_octreeNodeList.size()<<std::endl;



	// ���µ���ʾ�б������̣߳�Ԥ�����������
	m_cache->setNodesIn(m_octreeNodeList);



	/*osg::GraphicsCostEstimator *gce = new osg::GraphicsCostEstimator;
	osg::GeometryCostEstimator *gce1 = new osg::GeometryCostEstimator;
	osg::TextureCostEstimator *tce = new osg::TextureCostEstimator;

	int numPoly = 0, numVert = 0;
	osg::Timer_t time_cp = 0, time_loadNode = 0, time_transTex = 0;
	osg::Timer_t tmp, start;
	osg::Timer *t = new osg::Timer;
	double time_compileNode = 0, time_tex = 0.0, time_geom = 0.0;
	start = t->tick();*/
	const int *order = m_volHeader.orderXYZ;
	KLOctreeNode *node;
	std::list<KLOctreeNode *>::iterator itrNode;

	double getStateTime = 0;
	double computePolygonTime = 0;
	double createGeometryTime = 0;
	double polygonNum = 0;
	osg::Timer timer;
	cout<<m_octreeNodeList.size()<<endl;
	for (itrNode = m_octreeNodeList.begin(); itrNode != m_octreeNodeList.end(); ++itrNode)
	{
		node = *itrNode;

		m_sampleDistance[0] = node->m_sampleDistance[0]; 
		m_sampleDistance[1] = node->m_sampleDistance[1];
		m_sampleDistance[2] = node->m_sampleDistance[2];
		float sampleDistancetrans[3] = {m_sampleDistance[order[0]],m_sampleDistance[order[1]],m_sampleDistance[order[2]]};

		osg::ref_ptr<osg::Geode> block = new osg::Geode();
		//������ÿ�������Ҫÿһ�鶼���¼������֮��ÿ����ķ�Χ
		double temp_bounds[6];
		getVolumeNodeBoundsXYZ(node,temp_bounds);

		osg::Vec3 bounds[2] = {
			osg::Vec3(temp_bounds[0], temp_bounds[2], temp_bounds[4]),
			osg::Vec3(temp_bounds[1], temp_bounds[3], temp_bounds[5])
		};


		// �����������Ĳ�������
		int dim[3];
		dim[0] = node->m_sampleNum;
		dim[1] = node->m_CMPNum;
		dim[2] = node->m_recordNum;

		/*tmp = t->tick();*/
		/*float *data = m_cache->getNodeDataFromMap(node);
		block->setStateSet(createState(dim, data));*/
		/*time_transTex += t->tick() - tmp;*/
		double s1 = timer.time_m();
		osg::ref_ptr<osg::StateSet> state = getStateSet(node, dim);
		block->setStateSet(state);

		osg::Texture *texture = dynamic_cast<osg::Texture*>(
			(state->getTextureAttribute(0, osg::StateAttribute::TEXTURE)));

		texMap.insert(std::make_pair(texture, 1));

		getStateTime += (timer.time_m() - s1);

		// ��������
		/*tmp = t->tick();*/
		double s2 = timer.time_m();
		computePolygon(cameraPosition, bounds);
		computePolygonTime += (timer.time_m() - s2);
		/*time_cp += t->tick() - tmp;*/

		/*tmp = t->tick();
		numPoly += m_numOfPolygon;*/
		double s3 = timer.time_m();
		polygonNum += m_numOfPolygon;

		for (int i = 0; i < m_numOfPolygon; ++i)
		{
			osg::ref_ptr<osg::Vec3Array> polygonVertices = new osg::Vec3Array;
			osg::ref_ptr<osg::Vec3Array> tcoords = new osg::Vec3Array;

			float *ptr = m_polygonBuffer + 36 * i;
			int j;

			// �õ���Ƭ��Ӧ������������������
			for (j = 0; j < 6; ++j)
			{
				if ( ptr[0] < 0.0 )
					break ;

				polygonVertices->push_back(osg::Vec3(ptr[3], ptr[4], ptr[5]));
				tcoords->push_back(osg::Vec3(ptr[0], ptr[1], ptr[2]));

				ptr += 6;
			}

			if (j < 3)
				continue ;

			osg::ref_ptr<osg::Geometry> polygon = new osg::Geometry();
			block->addDrawable(polygon);

			/*numVert += polygonVertices->size();*/
			polygon->setVertexArray(polygonVertices);
			polygon->setTexCoordArray(0, tcoords);

			osg::ref_ptr<osg::DrawElementsUInt> polygonPS = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_FAN, 0);
			// j��Ӧ��Ƭ�����Ľ������
			for (int k = 0; k < j; ++k)
			{
				polygonPS->push_back(k);
			}
			polygon->addPrimitiveSet(polygonPS);

			/*osg::CostPair compileCost = gce1->estimateCompileCost(polygon);

			time_geom +=compileCost.first + compileCost.second;*/
		}
		createGeometryTime += (timer.time_m() - s3);
		/*time_loadNode += t->tick() - tmp;*/

		osg::ref_ptr<osg::Geode> fraBlock = new osg::Geode;
		osg::Vec3 v0 = bounds[0];
		osg::Vec3 v1 = osg::Vec3(bounds[1][0], bounds[0][1], bounds[0][2]);
		osg::Vec3 v2 = osg::Vec3(bounds[1][0], bounds[0][1], bounds[1][2]);
		osg::Vec3 v3 = osg::Vec3(bounds[0][0], bounds[0][1], bounds[1][2]);
		osg::Vec3 v4 = osg::Vec3(bounds[0][0], bounds[1][1], bounds[0][2]);
		osg::Vec3 v5 = osg::Vec3(bounds[1][0], bounds[1][1], bounds[0][2]);
		osg::Vec3 v6 = bounds[1];
		osg::Vec3 v7 = osg::Vec3(bounds[0][0], bounds[1][1], bounds[1][2]);

		osg::ref_ptr<osg::DrawElementsUInt> ps = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP, 0);
		osg::ref_ptr<osg::Geometry> plan0 = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> va0 = new osg::Vec3Array;
		va0->push_back(v0);
		va0->push_back(v1);
		va0->push_back(v2);
		va0->push_back(v3);
		plan0->setVertexArray(va0);
		for (int i=0; i<4; ++i)
		{
			ps->push_back(i);
		}
		plan0->addPrimitiveSet(ps);
		fraBlock->addDrawable(plan0);

		osg::ref_ptr<osg::Geometry> plan1 = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> va1 = new osg::Vec3Array;
		va1->push_back(v4);
		va1->push_back(v5);
		va1->push_back(v6);
		va1->push_back(v7);
		plan1->setVertexArray(va1);
		plan1->addPrimitiveSet(ps);
		fraBlock->addDrawable(plan1);

		osg::ref_ptr<osg::Geometry> plan2 = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> va2 = new osg::Vec3Array;
		va2->push_back(v3);
		va2->push_back(v2);
		va2->push_back(v6);
		va2->push_back(v7);
		plan2->setVertexArray(va2);
		plan2->addPrimitiveSet(ps);
		fraBlock->addDrawable(plan2);

		osg::ref_ptr<osg::Geometry> plan3 = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> va3 = new osg::Vec3Array;
		va3->push_back(v0);
		va3->push_back(v1);
		va3->push_back(v5);
		va3->push_back(v4);
		plan3->setVertexArray(va3);
		plan3->addPrimitiveSet(ps);
		fraBlock->addDrawable(plan3);

		osg::ref_ptr<osg::Geometry> plan4 = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> va4 = new osg::Vec3Array;
		va4->push_back(v0);
		va4->push_back(v4);
		va4->push_back(v7);
		va4->push_back(v3);
		plan4->setVertexArray(va4);
		plan4->addPrimitiveSet(ps);
		fraBlock->addDrawable(plan4);

		osg::ref_ptr<osg::Geometry> plan5 = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> va5 = new osg::Vec3Array;
		va5->push_back(v1);
		va5->push_back(v5);
		va5->push_back(v6);
		va5->push_back(v2);
		plan5->setVertexArray(va5);
		plan5->addPrimitiveSet(ps);
		fraBlock->addDrawable(plan5);

		addChild(fraBlock);
		addChild(block);
		/*osg::CostPair compileCost = tce->estimateCompileCost((const osg::Texture *)(block->getStateSet()->getTextureAttribute(0, osg::StateAttribute::TEXTURE)));
		time_tex += compileCost.first + compileCost.second;*/
	}

	/*osg::CostPair compileCost = gce->estimateCompileCost(this);
	time_compileNode = compileCost.first + compileCost.second;

	std::cout<<std::endl;
	std::cout<<"��ʾ�б��С��"<<m_octreeNodeList.size()<<std::endl;
	std::cout<<"�������Σ�"<<time_cp / 1000000.0<<std::endl;
	std::cout<<"���������"<<numVert<<std::endl;
	std::cout<<"����θ�����"<<numPoly<<std::endl;
	std::cout<<"���ض���Σ�"<<time_loadNode / 1000000.0<<std::endl;
	std::cout<<"�����������ɣ�"<<time_transTex / 1000000.0<<std::endl;
	std::cout<<"Geometry����"<<time_geom<<std::endl;
	std::cout<<"Texture����"<<time_tex<<std::endl;
	std::cout<<"����ƣ�"<<time_compileNode<<std::endl;
	std::cout<<std::endl;
	*/
	/*cout<<"polygonNum:"<<polygonNum<<endl;
	cout<<"getStateTime:"<<getStateTime<<endl;
	cout<<"computePolyognTime:"<<computePolygonTime<<endl;
	cout<<"createGeomtryTime:"<<createGeometryTime<<endl;
	cout<<"=====================================\n";*/
	m_colorMapChanged = false;
}
void KL3DVolumeNode::updateSubVolume(osg::Vec3 cameraPosition, int subBounds[6])
{
	if (m_volumeData)
	{
		osg::Geode *vol = (osg::Geode *)this->getChild(0);
		//vol->setStateSet(createState(m_volumeBounds, m_volumeData));
		computePolygon(cameraPosition, m_volumeBounds);
		vol->removeDrawables(0, vol->getNumDrawables());

		for (int i = 0; i < m_numOfPolygon; ++i)
		{
			osg::Vec3Array* polygonVertices = new osg::Vec3Array;
			osg::Vec3Array* tcoords = new osg::Vec3Array;

			float *ptr = m_polygonBuffer + 36 * i;
			int j;

			// �õ���Ƭ��Ӧ������������������
			for (j = 0; j < 6; ++j)
			{
				if ( ptr[0] < 0.0 )
					break ;

				polygonVertices->push_back(osg::Vec3(ptr[3], ptr[4], ptr[5]));
				tcoords->push_back(osg::Vec3(ptr[0], ptr[1], ptr[2]));

				ptr += 6;
			}

			if (j < 3)
				continue ;

			osg::Geometry* polygon = new osg::Geometry();
			vol->addDrawable(polygon);

			polygon->setVertexArray(polygonVertices);
			polygon->setTexCoordArray(0, tcoords);

			osg::DrawElementsUInt* polygonPS = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_FAN, 0);
			// j��Ӧ��Ƭ�����Ľ������
			for (int k = 0; k < j; ++k)
			{
				polygonPS->push_back(k);
			}
			polygon->addPrimitiveSet(polygonPS);
		}

		return ;
	}
}


// ��ȡ������ݣ�������ά����


//osg::ref_ptr<osg::StateSet> KL3DVolumeNode::createState(int dim[3], float *data)
//{
//	// ��ά��������
//	osg::Image* image_3d = new osg::Image;
//	image_3d->allocateImage(dim[0], dim[1], dim[2], GL_RGBA, GL_FLOAT);
//	image_3d->setInternalTextureFormat(GL_RGBA);
//	image_3d->setOrigin(osg::Image::BOTTOM_LEFT);
//
//	osg::Vec4 *ptr = (osg::Vec4 *)image_3d->data();
//	int s, t, r;
//	float val;
//
//	// �ӻ����еõ��������
//	
//	for (r = 0; r < dim[2]; ++r)
//	{
//		for (t = 0; t < dim[1]; ++t)
//		{
//			for (s = 0; s < dim[0]; ++s)
//			{
//				
//				val = *data++;
//				val = val /4000.0;
//				*ptr++ = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);
//				//*ptr = pfn(val * 255.f);
//				//ptr++;
//			}
//		}
//	}
//	
//
//	osg::Texture3D *texture3D = new osg::Texture3D;
//	texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
//	texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
//	texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
//	texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
//	texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
//	texture3D->setResizeNonPowerOfTwoHint(false);  // ����֧�ַ�2��������
//	texture3D->setImage(image_3d);
//
//	osg::StateSet* stateset = new osg::StateSet;
//	stateset->setTextureAttributeAndModes(0, texture3D);//,osg::StateAttribute::ON
//
//	// ��stateset�õ���ά��������,������Ӧɫ��仯
//	//osg::Texture3D *tex = static_cast<osg::Texture3D *>(stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
//	//osg::Image *image = tex->getImage();
//	//setImageData(image, data);
//
//	return stateset;
//}
osg::ref_ptr<osg::StateSet> KL3DVolumeNode::createState(int dim[3], float *data)
{
	int blockSize = m_octree->getBlockSize();
	float * dataCache=new float[dim[0]*dim[1]*dim[2]];
	float* pdataCache=dataCache;
	bool isFull = dim[0] == blockSize && dim[1] == blockSize && dim[2] == blockSize;
	if (!isFull)
	{
		for (int k = 0; k < blockSize; k++)
		{
			for (int j = 0; j < blockSize; j++)
			{
				for (int i = 0; i < blockSize; i++)
				{
					if (-2 != *data)
					{
						*pdataCache=*data;
						pdataCache++;
					} 
					data++;
				}
			}
		}
		pdataCache = dataCache;
	}
	else
	{
		pdataCache = data;
	}

	const int *order = m_volHeader.orderXYZ;

	//ת����� �����������Ĳ�������
	int dimtrans[3] = {dim[ order[0]],dim[ order[1]],dim[ order[2]]};

	if (dim[ order[0]] == 1 &&dim[ order[1]] == 1 && dim[ order[2]] == 1)
	{
		std::cout<<"hehe-error:KL3DVolumeNode.createState"<<std::endl;
		return NULL;
	}

	// ��ά��������
	osg::ref_ptr<osg::Image> image_3d = new osg::Image;
	image_3d->allocateImage(dimtrans[0], dimtrans[1], dimtrans[2], GL_RGBA, GL_FLOAT);
	image_3d->setInternalTextureFormat(GL_RGBA);
	image_3d->setOrigin(osg::Image::BOTTOM_LEFT);

	osg::Vec4 *ptr = (osg::Vec4 *)image_3d->data();

	float val;
	osg::Vec4 color;


	int podr[3],pos;

	int sliceSizeFS = dim[0] * dim[1];
	int sliceSizeXY = dimtrans[0]*dimtrans[1];

	int processNum = omp_get_num_procs();


	//#pragma omp parallel for private(podr, pos, val, color)

	for (podr[0] = 0; podr[0] < dimtrans[2]; ++podr[0])//z
	{
		for (podr[1] = 0; podr[1] < dimtrans[1]; ++podr[1])//y
		{
			for (podr[2] = 0; podr[2] < dimtrans[0]; ++podr[2])//x
			{
				//���������ϵ�ƫ���ۼ�
				int totalidxFST = 0;
				int totalidxXYZ = podr[0]*sliceSizeXY + podr[1] * dimtrans[0] + podr[2];

				for (int m = 0;m < 3;m++) 
				{
					if (order[m] == 2)
					{
						totalidxFST += podr[2-m] * sliceSizeFS;
					}
					else if (order[m] == 1)
					{
						totalidxFST += podr[2-m] * dim[0];
					}
					else
					{
						totalidxFST += podr[2-m];
					}
				}
				val = pdataCache[totalidxFST];
				if (val == -2)
				{
					cout<<val;
				}
				//pfn ? color = pfn(val) : color = osg::Vec4(1.0,1.0,1.0,1.0);

				val = val /4001.0;
				color = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);

				ptr[totalidxXYZ] = color;
				/*val = *data++;
				val = val /4000.0;
				*ptr++ = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);*/

			}
		}


	}
	//for (int i=0; i < processNum; ++i)
	//{
	//	for (podr[0] = 0; podr[0] < blockNum; ++podr[0])
	//	{
	//		pos = i * blockNum;
	//		for (podr[1] = 0; podr[1] < dimtrans[1]; ++podr[1])
	//		{
	//			for (podr[2] = 0; podr[2] < dimtrans[0]; ++podr[2])
	//			{
	//				//���������ϵ�ƫ���ۼ�
	//				int totalidxFST = 0;
	//				int totalidxXYZ = (podr[0] + pos)*sliceSizeXY + podr[1] * dimtrans[0] + podr[2];
	//				
	//				for (int m = 0;m < 3;m++) 
	//				{
	//					if (order[m] == 2)
	//					{
	//						totalidxFST += podr[2-m] * sliceSizeFS;
	//						if (m == 2)
	//							totalidxFST += pos * sliceSizeFS;
	//					}
	//					else if (order[m] == 1)
	//					{
	//						totalidxFST += podr[2-m] * dim[0];
	//						if(m == 2)
	//							totalidxFST += pos * dim[0];
	//					}
	//					else
	//					{
	//						totalidxFST += podr[2-m];
	//						if(m == 2)
	//							totalidxFST += pos;
	//					}
	//				}
	//				val = data[totalidxFST];

	//				//pfn ? color = pfn(val) : color = osg::Vec4(1.0,1.0,1.0,1.0);

	//				val = val /4001.0;
	//				color = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);

	//				ptr[totalidxXYZ] = color;
	//				/*val = *data++;
	//				val = val /4000.0;
	//				*ptr++ = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);*/
	//				
	//			}
	//		}

	//	}
	//}
	m_colorMapChanged = false;



	osg::ref_ptr<osg::Texture3D>texture3D = new osg::Texture3D;
	texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
	texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
	texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
	texture3D->setResizeNonPowerOfTwoHint(false);  // ����֧�ַ�2��������
	texture3D->setImage(image_3d);

	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
	stateset->setTextureAttributeAndModes(0, texture3D);//,osg::StateAttribute::ON

	// ��stateset�õ���ά��������,������Ӧɫ��仯
	//osg::Texture3D *tex = static_cast<osg::Texture3D *>(stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
	//osg::Image *image = tex->getImage();
	//setImageData(image, data);
	osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode();
	polygonMode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
	stateset->setAttribute( polygonMode.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
	return stateset;
}

// 
void KL3DVolumeNode::computePolygon(osg::Vec3 cameraPosition, osg::Vec3 bounds[2])
{
	osg::Vec4d plane;
	float center[3];
	//int bounds[6] = {
	//	node->m_minSample, node->m_maxSample,
	//	node->m_minCMP, node->m_maxCMP,
	//	node->m_minRecord, node->m_maxRecord
	//};

	int dim[3];
	//dim[0] = node->_sampleNum;
	//dim[1] = node->_CMPNum;
	//dim[2] = node->_recordNum;
	dim[0] = bounds[1][0] - bounds[0][0] + 1;
	dim[1] = bounds[1][1] - bounds[0][1] + 1;
	dim[2] = bounds[1][2] - bounds[0][2] + 1;

	//center[0] = (bounds[0] + bounds[1]) / 2.0;
	//center[1] = (bounds[2] + bounds[3]) / 2.0;
	//center[2] = (bounds[4] + bounds[5]) / 2.0;
	center[0] = (bounds[0][0] + bounds[1][0]) / 2.0;
	center[1] = (bounds[0][1] + bounds[1][1]) / 2.0;
	center[2] = (bounds[0][2] + bounds[1][2]) / 2.0;


	// ���ӵ������߷���ֱ��ƽ�淽��
	plane[0] = center[0] - cameraPosition[0];
	plane[1] = center[1] - cameraPosition[1];
	plane[2] = center[2] - cameraPosition[2];

	plane.normalize();
	//����һ���ӵ㴦�����ߴ�ֱ��ƽ��
	plane[3] = -(plane[0] * cameraPosition[0] + plane[1] * cameraPosition[1] + plane[2] * cameraPosition[2]);

	// ��鶥�㵽���ӵ�ƽ��ľ���
	double minDistance = DBL_MAX;
	double maxDistance = -DBL_MAX;

	// ���İ˸�����
	int i, j, k;
	osg::Vec3Array *vertices = new osg::Vec3Array;

	int idx = 0;

	for ( k = 0; k < 2; k++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( i = 0; i < 2; i++ )
			{
				vertices->push_back(osg::Vec3d(bounds[i][0], bounds[j][1], bounds[k][2]));
				// ���㵽ƽ��ľ��룬���ӵ㴦�����ߴ�ֱ��ƽ���붥�㴦�����ߴ�ֱ��ƽ�棨������ƽ�У�֮��ľ��루ƽ��ķ��򳤶�Ϊ1��
				double d =
					plane[0] * (*vertices)[idx][0] +
					plane[1] * (*vertices)[idx][1] +
					plane[2] * (*vertices)[idx][2] +
					plane[3];

				idx++;

				// ����ֵ
				minDistance = d < minDistance ? d : minDistance;
				maxDistance = d > maxDistance ? d : maxDistance;
			}
		}
	}

	float spacing[3];
	spacing[0] = spacing[1] = spacing[2] = 1.0;

	double offset = 0.333 * 0.5 * (spacing[0] + spacing[1] + spacing[2]);

	minDistance = minDistance < offset ? offset : minDistance;

	// ���ڶ���εľ���
	double stepSize = (m_sampleDistance[0] + m_sampleDistance[1] + m_sampleDistance[2]) / 6.0;

	// ����θ���
	int numPolys = static_cast<int>((maxDistance - minDistance) / static_cast<double>(stepSize));

	// �ڴ���
	if (m_bufferSize < numPolys)
	{
		delete [] m_polygonBuffer;
		delete [] m_intersectionBuffer;

		m_bufferSize = numPolys;

		m_polygonBuffer = new float [36 * m_bufferSize];
		m_intersectionBuffer = new float [12 * m_bufferSize];
	}

	m_numOfPolygon = numPolys;

	// ����ƽ�������12���ߵĽ���
	int lines[12][2] = {
		{0, 1}, {1, 3}, {2, 3},
		{0, 2}, {4, 5}, {5, 7},
		{6, 7}, {4, 6},	{0, 4},
		{1, 5}, {3, 7}, {2, 6}
	};// �ö����ű�ʾ12����

	float *iptr, *pptr;
	osg::Vec3 line;
	double d = maxDistance, planeDotLineOrigin, planeDotLine, t, increment;

	for (i = 0; i < 12; i++)
	{			
		// ������ֱ�ߵķ���

		line[0] = (*vertices)[lines[i][1]][0] - (*vertices)[lines[i][0]][0];
		line[1] = (*vertices)[lines[i][1]][1] - (*vertices)[lines[i][0]][1];
		line[2] = (*vertices)[lines[i][1]][2] - (*vertices)[lines[i][0]][2];			

		iptr = m_intersectionBuffer + i;

		planeDotLineOrigin = plane * (*vertices)[lines[i][0]]- plane[3] ; // vec4d *vec3�������⣨3ά��4ά��£�Զ����1����Ӧ����ˣ�
		//���¶��plane[3]������ҪУ��
		planeDotLine       = plane * line - plane[3];

		if (planeDotLine != 0.0)
		{
			//��ǰ�߶ε���㣨���㣩�������ߴ�ֱ�浽��Զ���������ߴ�ֱ��֮�����/�߶����������߷����ͶӰ����
			t = (d - planeDotLineOrigin - plane[3]) / planeDotLine;//
			increment = -stepSize / planeDotLine;//���߷����ϵĲ�������/��ǰ�������߷����ϵ�ͶӰ����
			//���������������ÿһ����ͶӰ��������ռ�ı���
			////////////////////////////////////////////////////////////////////////////////////�˴����޸�
		}
		else
		{
			t         = -1.0;
			increment =  0.0;
		}

		for ( j = 0; j < numPolys; j++ )
		{
			*iptr = t > 0.0 && t < 1.0 ? t : -1.0;

			t += increment;
			iptr += 12;
		}//�����ÿһ������ϵ�tֵ������ǰ����֮��ı��ϵ�tֵ����-1����ǰ����tֵΪ0~1֮�䣩������t��0~1֮��
		 //����ζ�ŵ�ǰ�����м�������ν���
	}//������ÿһ�����

	// ͨ�����ڱ߲��ҽ���
	int neighborLines[12][6] = {
		{1, 2, 3, 4,  8,  9}, {0, 2, 3, 5, 9, 10},
		{0, 1, 3, 6, 10, 11}, {0, 1, 2, 7, 8, 11},
		{0, 5, 6, 7,  8,  9}, {1, 4, 6, 7, 9, 10},
		{2, 4, 5, 7, 10, 11}, {3, 4, 5, 6, 8, 11},
		{0, 3, 4, 7,  9, 11}, {0, 1, 4, 5, 8, 10},
		{1, 2, 5, 6,  9, 11}, {2, 3, 6, 7, 8, 10}
	};
	// ͨ�����ڱ߲��ҽ���
	//int neighborLines[12][11] = {
	//	{  1, 2, 3, 4, 5, 6 ,7, 8,  9,10,11}, {0 ,  2, 3, 4, 5, 6 ,7, 8,  9,10,11},
	//	{0,1,    3, 4, 5, 6 ,7, 8,  9,10,11}, {0,1, 2,    4, 5, 6 ,7, 8,  9,10,11},
	//	{0,1, 2, 3,    5, 6 ,7, 8,  9,10,11}, {0,1, 2, 3, 4,    6 ,7, 8,  9,10,11},
	//	{0,1, 2, 3, 4, 5,    7, 8,  9,10,11}, {0,1, 2, 3, 4, 5, 6 ,   8,  9,10,11},
	//	{0,1, 2, 3, 4, 5, 6 ,7,     9,10,11}, {0,1, 2, 3, 4, 5, 6 ,7, 8,    10,11},
	//	{0,1, 2, 3, 4, 5, 6 ,7, 8,  9,   11}, {0,1, 2, 3, 4, 5, 6 ,7, 8,  9,10   }
	//};
	// ǰ3���ʾ�ߵ������������꣬��4���ʾ����ǰ3���з����仯����ı��
	float tCoord[12][4] = {
		{0, 0, 0, 0}, {1, 0, 0, 1}, {0, 1, 0, 0}, {0, 0, 0, 1},
		{0, 0, 1, 0}, {1, 0, 1, 1}, {0, 1, 1, 0}, {0, 0, 1, 1},
		{0, 0, 0, 2}, {1, 0, 0, 2}, {1, 1, 0, 2}, {0, 1, 0, 2}
	};

	//for (i = 0; i < 12; i++)
	//{
	//	tCoord[i][0] = tCoord[i][0] ? 1.0 : 0.0;
	//	tCoord[i][1] = tCoord[i][1] ? 1.0 : 0.0;
	//	tCoord[i][2] = tCoord[i][2] ? 1.0 : 0.0;
	//}

	iptr = m_intersectionBuffer;
	pptr = m_polygonBuffer;

	int start = 0; 
	for (i = 0; i < numPolys; i++)
	{
		// ���Ҷ�����������ཻ��
		start = 0;

		while (start < 12 && iptr[start] == -1.0)
		{
			start++;
		}

		if (start == 12)
		{
			pptr[0] = -1.0; // ���ཻ��û�н���
		}
		else
		{
			int current = start;
			int previous = -1;
			int errFlag = 0;

			idx   = 0;

			// idx��ʾ���������һ��ƽ�����������6�����ཻ
			while (idx < 6 && !errFlag && (idx == 0 || current != start))
			{
				double t = iptr[current];

				// �������������
				*(pptr + idx * 6)     = tCoord[current][0];
				*(pptr + idx * 6 + 1) = tCoord[current][1];
				*(pptr + idx * 6 + 2) = tCoord[current][2];

				int coord = static_cast<int>(tCoord[current][3]);
				*(pptr + idx * 6 + coord) = t;

				// ����ζ�������
				*(pptr + idx * 6 + 3) = static_cast<float>((*vertices)[lines[current][0]][0] +
					t * ((*vertices)[lines[current][1]][0] - (*vertices)[lines[current][0]][0]));
				*(pptr + idx * 6 + 4) = static_cast<float>((*vertices)[lines[current][0]][1] + 
					t * ((*vertices)[lines[current][1]][1] - (*vertices)[lines[current][0]][1]));
				*(pptr + idx * 6 + 5) = static_cast<float>((*vertices)[lines[current][0]][2] + 
					t * ((*vertices)[lines[current][1]][2] - (*vertices)[lines[current][0]][2]));

				idx++;

				// ������һ��δ������ཻ��
				j = 0;
				while (j < 6
					&&	(*(m_intersectionBuffer + i * 12 + neighborLines[current][j]) < 0
					|| neighborLines[current][j] == previous)) 
				{
					j++;
				}

				if ( j >= 6 ) // �ཻ�߳���6������
				{
					errFlag = 1;
				}
				else
				{
					previous = current;
					current = neighborLines[current][j];
				}
			}//while

			if (idx < 6)
			{
				*(pptr + idx * 6) = -1;
			}
		}//���㵱ǰ�������߽�������������ʵ������

		iptr += 12;
		pptr += 36;
	}//���������еĶ����
}
//void KL3DVolumeNode::computePolygon(osg::Vec3 cameraPosition, osg::Vec3 bounds[2])
//{
//	osg::Vec4d plane;
//	float center[3];
//	//int bounds[6] = {
//	//	node->m_minSample, node->m_maxSample,
//	//	node->m_minCMP, node->m_maxCMP,
//	//	node->m_minRecord, node->m_maxRecord
//	//};
//
//	int dim[3];
//	//dim[0] = node->_sampleNum;
//	//dim[1] = node->_CMPNum;
//	//dim[2] = node->_recordNum;
//	dim[0] = bounds[1][0] - bounds[0][0] + 1;
//	dim[1] = bounds[1][1] - bounds[0][1] + 1;
//	dim[2] = bounds[1][2] - bounds[0][2] + 1;
//
//	//center[0] = (bounds[0] + bounds[1]) / 2.0;
//	//center[1] = (bounds[2] + bounds[3]) / 2.0;
//	//center[2] = (bounds[4] + bounds[5]) / 2.0;
//	center[0] = (bounds[0][0] + bounds[1][0]) / 2.0;
//	center[1] = (bounds[0][1] + bounds[1][1]) / 2.0;
//	center[2] = (bounds[0][2] + bounds[1][2]) / 2.0;
//
//
//	// ���ӵ������߷���ֱ��ƽ�淽��
//	plane[0] = center[0] - cameraPosition[0];
//	plane[1] = center[1] - cameraPosition[1];
//	plane[2] = center[2] - cameraPosition[2];
//
//	plane.normalize();
//	//����һ���ӵ㴦�����ߴ�ֱ��ƽ��
//	plane[3] = -(plane[0] * cameraPosition[0] + plane[1] * cameraPosition[1] + plane[2] * cameraPosition[2]);
//
//	// ��鶥�㵽���ӵ�ƽ��ľ���
//	double minDistance = DBL_MAX;
//	double maxDistance = -DBL_MAX;
//
//	// ���İ˸�����
//	int i, j, k;
//	osg::Vec3Array *vertices = new osg::Vec3Array;
//
//	int idx = 0;
//
//	for ( k = 0; k < 2; k++ )
//	{
//		for ( j = 0; j < 2; j++ )
//		{
//			for ( i = 0; i < 2; i++ )
//			{
//				vertices->push_back(osg::Vec3d(bounds[i][0], bounds[j][1], bounds[k][2]));
//				// ���㵽ƽ��ľ��룬���ӵ㴦�����ߴ�ֱ��ƽ���붥�㴦�����ߴ�ֱ��ƽ�棨������ƽ�У�֮��ľ���
//				double d =
//					plane[0] * (*vertices)[idx][0] +
//					plane[1] * (*vertices)[idx][1] +
//					plane[2] * (*vertices)[idx][2] +
//					plane[3];
//
//				idx++;
//
//				// ����ֵ
//				minDistance = d < minDistance ? d : minDistance;
//				maxDistance = d > maxDistance ? d : maxDistance;
//			}
//		}
//	}
//
//	float spacing[3];
//	spacing[0] = spacing[1] = spacing[2] = 1.0;
//
//	double offset = 0.333 * 0.5 * (spacing[0] + spacing[1] + spacing[2]);
//
//	minDistance = minDistance < offset ? offset : minDistance;
//
//	// ���ڶ���εľ���
//	double stepSize = (m_sampleDistance[0] + m_sampleDistance[1] + m_sampleDistance[2]) / 3.0;
//
//	// ����θ���
//	int numPolys = static_cast<int>((maxDistance - minDistance) / static_cast<double>(stepSize));
//
//	// �ڴ���
//	if (m_bufferSize < numPolys)
//	{
//		delete [] m_polygonBuffer;
//		delete [] m_intersectionBuffer;
//
//		m_bufferSize = numPolys;
//
//		m_polygonBuffer = new float [36 * m_bufferSize];
//		m_intersectionBuffer = new float [12 * m_bufferSize];
//	}
//
//	m_numOfPolygon = numPolys;
//
//	// ����ƽ�������12���ߵĽ���
//	int lines[12][2] = {
//		{0, 1}, {1, 3}, {2, 3},
//		{0, 2}, {4, 5}, {5, 7},
//		{6, 7}, {4, 6},	{0, 4},
//		{1, 5}, {3, 7}, {2, 6}
//	};// �ö����ű�ʾ12����
//
//	float *iptr, *pptr;
//	osg::Vec3 line;
//	double d = maxDistance, planeDotLineOrigin, planeDotLine, t, increment;
//
//	for (i = 0; i < 12; i++)
//	{			
//		// ������ֱ�ߵķ���
//		line[0] = (*vertices)[lines[i][1]][0] - (*vertices)[lines[i][0]][0];
//		line[1] = (*vertices)[lines[i][1]][1] - (*vertices)[lines[i][0]][1];
//		line[2] = (*vertices)[lines[i][1]][2] - (*vertices)[lines[i][0]][2];			
//
//		iptr = m_intersectionBuffer + i;
//
//		planeDotLineOrigin = plane * (*vertices)[lines[i][0]] - plane[3]; // vec4d *���� +plane[3]
//		planeDotLine       = plane * line - plane[3];
//
//		if (planeDotLine != 0.0)
//		{
//			t = (d - planeDotLineOrigin - plane[3]) / planeDotLine;
//			increment = -stepSize / planeDotLine;
//		}
//		else
//		{
//			t         = -1.0;
//			increment =  0.0;
//		}
//
//		for ( j = 0; j < numPolys; j++ )
//		{
//			*iptr = t > 0.0 && t < 1.0 ? t : -1.0;
//
//			t += increment;
//			iptr += 12;
//		}
//	}
//
//	// ͨ�����ڱ߲��ҽ���
//	int neighborLines[12][6] = {
//		{1, 2, 3, 4,  8,  9}, {0, 2, 3, 5, 9, 10},
//		{0, 1, 3, 6, 10, 11}, {0, 1, 2, 7, 8, 11},
//		{0, 5, 6, 7,  8,  9}, {1, 4, 6, 7, 9, 10},
//		{2, 4, 5, 7, 10, 11}, {3, 4, 5, 6, 8, 11},
//		{0, 3, 4, 7,  9, 11}, {0, 1, 4, 5, 8, 10},
//		{1, 2, 5, 6,  9, 11}, {2, 3, 6, 7, 8, 10}
//	};
//
//	// ǰ3���ʾ�ߵ������������꣬��4���ʾ����ǰ3���з����仯����ı��
//	float tCoord[12][4] = {
//		{0, 0, 0, 0}, {1, 0, 0, 1}, {0, 1, 0, 0}, {0, 0, 0, 1},
//		{0, 0, 1, 0}, {1, 0, 1, 1}, {0, 1, 1, 0}, {0, 0, 1, 1},
//		{0, 0, 0, 2}, {1, 0, 0, 2}, {1, 1, 0, 2}, {0, 1, 0, 2}
//	};
//
//	//for (i = 0; i < 12; i++)
//	//{
//	//	tCoord[i][0] = tCoord[i][0] ? 1.0 : 0.0;
//	//	tCoord[i][1] = tCoord[i][1] ? 1.0 : 0.0;
//	//	tCoord[i][2] = tCoord[i][2] ? 1.0 : 0.0;
//	//}
//
//	iptr = m_intersectionBuffer;
//	pptr = m_polygonBuffer;
//
//	int start = 0; 
//	for (i = 0; i < numPolys; i++)
//	{
//		// ���Ҷ�����������ཻ��
//		start = 0;
//
//		while (start < 12 && iptr[start] == -1.0)
//		{
//			start++;
//		}
//
//		if (start == 12)
//		{
//			pptr[0] = -1.0; // ���ཻ��û�н���
//		}
//		else
//		{
//			int current = start;
//			int previous = -1;
//			int errFlag = 0;
//
//			idx   = 0;
//
//			// idx��ʾ���������һ��ƽ�����������6�����ཻ
//			while (idx < 6 && !errFlag && (idx == 0 || current != start))
//			{
//				double t = iptr[current];
//
//				// �������������
//				*(pptr + idx * 6)     = tCoord[current][0];
//				*(pptr + idx * 6 + 1) = tCoord[current][1];
//				*(pptr + idx * 6 + 2) = tCoord[current][2];
//
//				int coord = static_cast<int>(tCoord[current][3]);
//				*(pptr + idx * 6 + coord) = t;
//
//				// ����ζ�������
//				*(pptr + idx * 6 + 3) = static_cast<float>((*vertices)[lines[current][0]][0] +
//					t * ((*vertices)[lines[current][1]][0] - (*vertices)[lines[current][0]][0]));
//				*(pptr + idx * 6 + 4) = static_cast<float>((*vertices)[lines[current][0]][1] + 
//					t * ((*vertices)[lines[current][1]][1] - (*vertices)[lines[current][0]][1]));
//				*(pptr + idx * 6 + 5) = static_cast<float>((*vertices)[lines[current][0]][2] + 
//					t * ((*vertices)[lines[current][1]][2] - (*vertices)[lines[current][0]][2]));
//
//				idx++;
//
//				// ������һ��δ������ཻ��
//				j = 0;
//				while (j < 6
//					&&	(*(m_intersectionBuffer + i * 12 + neighborLines[current][j]) < 0
//					|| neighborLines[current][j] == previous)) 
//				{
//					j++;
//				}
//
//				if ( j >= 6 ) // �ཻ�߳���6������
//				{
//					errFlag = 1;
//				}
//				else
//				{
//					previous = current;
//					current = neighborLines[current][j];
//				}
//			}
//
//			if (idx < 6)
//			{
//				*(pptr + idx * 6) = -1;
//			}
//		}
//
//		iptr += 12;
//		pptr += 36;
//	}
//}

void KL3DVolumeNode::getVolumeNodeBoundsXYZ(KLOctreeNode *node,double* bounds)
{
	if ( NULL == bounds)
	{
		std::cout<<"hehe-error:KL3DVolumeDataAdapter.getVolumeBoundsXYZ"<<std::endl;
		return;
	}
	/*if ( node.numFirst  == 1 && node.numSecond == 1 && node.numThird == 1)
	{
	std::cout<<"hehe-error:KL3DVolumeDataAdapter.getVolumeBoundsXYZ"<<std::endl;
	return;
	}*/
	//int temp_bound[3] = {node.numFirst ,	node.numSecond ,	node.numThird 	};
	double temp_bound[6] = {node->m_minSample,node->m_maxSample,
		node->m_minCMP   ,node->m_maxCMP,
		node->m_minRecord,node->m_maxRecord};

	int j = m_volHeader.orderXYZ[0];
	bounds[0] = temp_bound[j*2];
	bounds[1] =  temp_bound[j*2+1];
	j = m_volHeader.orderXYZ[1];
	bounds[2] = temp_bound[j*2];
	bounds[3] = temp_bound[j*2+1];
	j = m_volHeader.orderXYZ[2];
	bounds[4] = temp_bound[j*2];
	bounds[5] = temp_bound[j*2+1];


}

END_KLDISPLAY3D_NAMESPACE
