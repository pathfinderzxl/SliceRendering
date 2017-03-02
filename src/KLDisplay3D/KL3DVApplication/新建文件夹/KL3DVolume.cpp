#include "KL3DVolume.h"
#include "KL3DVolumeBlockCache.h"
#include "KL3DDataModel/KL3DVOctree.h"
//#include "KLGetColor.h"

#include "qfile.h"
#include "qdatastream.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Vec3d>
#include <osg/Texture3D>

#include <iostream>

#include "float.h"

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DVolume::KL3DVolume(Octree *octree, osgViewer::Viewer *viewer, getColorByValueFunc pfn)
{
	m_viewer = viewer;
	m_octree = octree;
	m_cache = new KL3DVolumeBlockCache(octree);
	//m_cache->setFileName(m_octree->getOctreeFileName());

	m_volumeBounds[0][0] = octree->_minSample;
	m_volumeBounds[0][1] = octree->_minCMP;
	m_volumeBounds[0][2] = octree->_minRecord;
	m_volumeBounds[1][0] = octree->_maxSample;
	m_volumeBounds[1][1] = octree->_maxCMP;
	m_volumeBounds[1][2] = octree->_maxRecord;

	m_bufferSize = 0;
	m_intersectionBuffer = NULL;
	m_polygonBuffer = NULL;
	m_numOfPolygon = 0;

	m_octreeNodeList.push_back(octree->_rootNode);

	m_colorMapChanged = true;

	this->pfn = pfn;
	initialize();
}

// �����ʼ��
void KL3DVolume::initialize()
{
	int bounds[6], dim[3]; 
	double spacing[3];
	OctreeNode *node;

	std::list<OctreeNode *>::iterator itrNode;
	for (itrNode = m_octreeNodeList.begin(); itrNode != m_octreeNodeList.end(); ++itrNode)
	{
		node = *itrNode;

		// �������ΪҶ�ڵ�ӵ�������
		osg::Geode *block = new osg::Geode();
		osg::StateSet *state = createState(node);
		block->setStateSet(state);

		// ��������߽�
		bounds[0] = node->_minSample;
		bounds[1] = node->_maxSample;
		bounds[2] = node->_minCMP;
		bounds[3] = node->_maxCMP;
		bounds[4] = node->_minRecord;
		bounds[5] = node->_maxRecord;

		// �����������Ĳ�������
		dim[0] = node->_sampleNum;
		dim[1] = node->_CMPNum;
		dim[2] = node->_recordNum;	 

		// �������
		spacing[0] = node->_sampleDistance[0];
		spacing[1] = node->_sampleDistance[1];
		spacing[2] = node->_sampleDistance[2];		

		// ����������ߴ�ֱ�Ķ������Ƭ��ɣ�ÿ����Ƭ��Ӧһ��drawable
		for (int i = 0; i < dim[1]; ++i)
		{
			osg::Geometry* polygon = new osg::Geometry();
			block->addDrawable(polygon);

			// ����ζ�����������
			osg::Vec3Array* tcoords = new osg::Vec3Array(4);
			(*tcoords)[0].set(0.0f, (float)i / (dim[1] - 1), 0.0f);
			(*tcoords)[3].set(0.0f, (float)i / (dim[1] - 1), 1.0f);		
			(*tcoords)[2].set(1.0f, (float)i / (dim[1] - 1), 1.0f);
			(*tcoords)[1].set(1.0f, (float)i / (dim[1] - 1), 0.0f);		
			polygon->setTexCoordArray(0, tcoords);

			// ����ζ�������
			osg::Vec3Array* polygonVertices = new osg::Vec3Array;
			polygonVertices->push_back(osg::Vec3(bounds[0], (float)(i * spacing[1] + bounds[2]), bounds[4]));
			polygonVertices->push_back(osg::Vec3(bounds[1], (float)(i * spacing[1] + bounds[2]), bounds[4]));
			polygonVertices->push_back(osg::Vec3(bounds[1], (float)(i * spacing[1] + bounds[2]), bounds[5]));
			polygonVertices->push_back(osg::Vec3(bounds[0], (float)(i * spacing[1] + bounds[2]), bounds[5]));
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

// ���������ľ��� 
double computeDistance(osg::Vec3d point1, double *point2)
{
	return sqrt((point1[0] - point2[0]) * (point1[0] - point2[0])
		+ (point1[1] - point2[1]) * (point1[1] - point2[1])
		+ (point1[2] - point2[2]) * (point1[2] - point2[2]));
}

// �ӿ����˳��
void KL3DVolume::setLoadOrder(OctreeNode *node, osg::Vec3d cameraPosition, int order[8])
{
	OctreeNode *p = node->_children[0];
	if (cameraPosition[0] >= p->_maxSample && cameraPosition[1] >= p->_maxCMP && cameraPosition[2] >= p->_maxRecord)
	{
		order[0] = 7; order[1] = 3; order[2] = 5; order[3] = 6;
		order[4] = 1; order[5] = 2; order[6] = 4; order[7] = 0;
	}
	else if (cameraPosition[0] < p->_maxSample && cameraPosition[1] >= p->_maxCMP && cameraPosition[2] >= p->_maxRecord)
	{
		order[0] = 6; order[1] = 2; order[2] = 4; order[3] = 7;
		order[4] = 0; order[5] = 3; order[6] = 5; order[7] = 1;
	}
	else if (cameraPosition[0] >= p->_maxSample && cameraPosition[1] < p->_maxCMP && cameraPosition[2] >= p->_maxRecord)
	{
		order[0] = 5; order[1] = 1; order[2] = 4; order[3] = 7;
		order[4] = 0; order[5] = 3; order[6] = 6; order[7] = 2;
	}
	else if (cameraPosition[0] < p->_maxSample && cameraPosition[1] < p->_maxCMP && cameraPosition[2] >= p->_maxRecord)
	{
		order[0] = 4; order[1] = 0; order[2] = 5; order[3] = 6;
		order[4] = 1; order[5] = 2; order[6] = 7; order[7] = 3;
	}
	else if (cameraPosition[0] >= p->_maxSample && cameraPosition[1] >= p->_maxCMP && cameraPosition[2] < p->_maxRecord)
	{
		order[0] = 3; order[1] = 1; order[2] = 2; order[3] = 7;
		order[4] = 0; order[5] = 5; order[6] = 6; order[7] = 4;
	}
	else if (cameraPosition[0] < p->_maxSample && cameraPosition[1] >= p->_maxCMP && cameraPosition[2] < p->_maxRecord)
	{
		order[0] = 2; order[1] = 0; order[2] = 3; order[3] = 6;
		order[4] = 1; order[5] = 4; order[6] = 7; order[7] = 5;
	}
	else if (cameraPosition[0] >= p->_maxSample && cameraPosition[1] < p->_maxCMP && cameraPosition[2] < p->_maxRecord)
	{
		order[0] = 1; order[1] = 0; order[2] = 3; order[3] = 5;
		order[4] = 2; order[5] = 4; order[6] = 7; order[7] = 6;
	}
	else if (cameraPosition[0] < p->_maxSample && cameraPosition[1] < p->_maxCMP && cameraPosition[2] < p->_maxRecord)
	{
		order[0] = 0; order[1] = 1; order[2] = 2; order[3] = 4;
		order[4] = 3; order[5] = 5; order[6] = 6; order[7] = 7;
	}
}

// �����ӵ�ѡ����ʾ�б�
void KL3DVolume::updateNodeList(osg::Vec3d cameraPosition)
{
	std::list<OctreeNode *> tmpList;
	int num = 0, order[8];

	m_octreeNodeList.clear();
	OctreeNode *node = m_octree->_rootNode;
	m_octreeNodeList.push_back(node);

	// �Ӹ��ڵ㿪ʼ����������˲������ڵ�
	for (int loop = 0; loop < m_octree->m_depth; ++loop)
	{
		num = m_octreeNodeList.size();

		// ���ӵ���������ֱ������
		// �ӵ������ľ���ﵽĳһ��ֵ������鼴Ϊ���ʷֱ���
		if (computeDistance(cameraPosition, m_octreeNodeList.front()->_center)
			>= m_octreeNodeList.front()->_diagnose * 1.5)
			return ;

		tmpList = m_octreeNodeList;
		m_octreeNodeList.clear();

		OctreeNode *firstNode = tmpList.front(), *tmpNode;
		std::list<OctreeNode *>::iterator itrNode;
		for (itrNode = tmpList.begin(); itrNode != tmpList.end(); ++itrNode)
		{
			tmpNode = *itrNode;

			// �ӵ�λ����������λ�ù�ϵ��27�֣�������6���潫�ռ��Ϊ27�����֣��ӵ��Ӧ��27��λ��
			// ����������ӵ��27��λ�ù�ϵ����Ӧ�Ĳ��ɼ������27�����
			if ((*itrNode)->_level < loop
				|| ((cameraPosition[0] > node->_maxSample && tmpNode->_center[0] < firstNode->_minSample 
				|| cameraPosition[0] < node->_minSample && tmpNode->_center[0] > firstNode->_maxSample
				|| cameraPosition[0] <= node->_maxSample && cameraPosition[0] >= node->_minSample)
				&& (cameraPosition[1] > node->_maxCMP && tmpNode->_center[1] < firstNode->_minCMP
				|| cameraPosition[1] < node->_minCMP && tmpNode->_center[1] > firstNode->_maxCMP
				|| cameraPosition[1] <= node->_maxCMP && cameraPosition[1] >= node->_minCMP)
				&& (cameraPosition[2] > node->_maxRecord && tmpNode->_center[2] < firstNode->_minRecord
				|| cameraPosition[2] < node->_minRecord && tmpNode->_center[2] > firstNode->_maxRecord
				|| cameraPosition[2] <= node->_maxRecord && cameraPosition[2] >= node->_minRecord)))
			{
				m_octreeNodeList.push_back(tmpNode); // ���ɼ����
			}
			else 
			{
				// ���ֱ��ʺ���
				if (computeDistance(cameraPosition, tmpNode->_center) >= tmpNode->_diagnose * 1.5) 
					m_octreeNodeList.push_back(tmpNode);
				else if (num - 1 + tmpNode->_notEmptyChild <= 500) // �ڴ����ƣ��Ƿ񳬹���������
				{
					// ���ֱ���С���ڴ���㣬��߷ֱ���
					setLoadOrder(tmpNode, cameraPosition, order);
					for (int j = 0; j < 8; ++j)
						if (tmpNode->_children[order[j]])
							m_octreeNodeList.push_back(tmpNode->_children[order[j]]);
					num += tmpNode->_notEmptyChild;
				}
				else 
				{
					// ���ֱ���С���ڴ�ﵽ���ޣ�����ϸ��
					for (; itrNode != tmpList.end(); ++itrNode)
						m_octreeNodeList.push_back(*itrNode);
				}
			}
		}
	}
}

// ��ά����£����¼�����ʾ�б���������
void KL3DVolume::updateVolume(osg::Vec3d cameraPosition)
{
	removeChildren(0, this->getNumChildren());

	// ������ʾ�б�
	updateNodeList(cameraPosition);
	//std::cout<<"VolumeBlockNum:"<<m_octreeNodeList.size()<<std::endl;

	// ���µ���ʾ�б������̣߳�Ԥ�����������
	m_cache->setNodesIn(m_octreeNodeList);

	OctreeNode *node;
	std::list<OctreeNode *>::iterator itrNode;
	for (itrNode = m_octreeNodeList.begin(); itrNode != m_octreeNodeList.end(); ++itrNode)
	{
		node = *itrNode;

		// ��������
		computePolygon(cameraPosition, node);

		osg::Geode *block = new osg::Geode();

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
			block->addDrawable(polygon);

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
		addChild(block);

		block->setStateSet(createState(node));

		//std::cout<<block->getDrawableList().size()<<std::endl;
	}

	m_colorMapChanged = false;
}

//void KL3DVolume::setTextureImage(osg::Vec4 *image, float *data, int size)
//{
//	for (int i = 0; i < size; ++i)
//	{
//		// *image++ = osg::Vec4(*data, 255 - *data, 255 - *data * 0.5, 255);
//		*image = pfn(*data);
//	}
//}
//
//void KL3DVolume::resetTextureImage(OctreeNode *node)
//{
//	osg::Texture3D *tex = static_cast<osg::Texture3D *>(stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
//	osg::Image *image = tex->getImage();
//	setImageData(image, data);
//}

// ��ȡ������ݣ�������ά����
osg::StateSet* KL3DVolume::createState(OctreeNode *node)
{
	// ��ά��������
	int dim[3];
	dim[0] = node->_sampleNum;
	dim[1] = node->_CMPNum;
	dim[2] = node->_recordNum;

	// ��ά��������
	osg::Image* image_3d = new osg::Image;
	image_3d->allocateImage(dim[0], dim[1], dim[2], GL_RGBA, GL_FLOAT);
	image_3d->setInternalTextureFormat(GL_RGBA);

	osg::Vec4 *ptr = (osg::Vec4 *)image_3d->data();
	int s, t, r;
	float val;

	// �ӻ����еõ��������
	float *data = m_cache->getNodeDataFromMap(node);

	for (r = 0; r < dim[2]; ++r)
	{
		for (t = 0; t < dim[1]; ++t)
		{
			for (s = 0; s < dim[0]; ++s)
			{
				val = *data++;
				val = val / 4001.0;
				//*ptr++ = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 1);
				*ptr++ = pfn(val * 255.f);			
			}
		}
	}

	osg::Texture3D *texture3D = new osg::Texture3D;
	texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
	texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
	texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
	texture3D->setResizeNonPowerOfTwoHint(false);  // ����֧�ַ�2��������
	texture3D->setImage(image_3d);

	osg::StateSet* stateset = new osg::StateSet;
	stateset->setTextureAttributeAndModes(0, texture3D);//,osg::StateAttribute::ON

	// ��stateset�õ���ά��������,������Ӧɫ��仯
	//osg::Texture3D *tex = static_cast<osg::Texture3D *>(stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
	//osg::Image *image = tex->getImage();
	//setImageData(image, data);

	return stateset;
}

// 
void KL3DVolume::computePolygon(osg::Vec3d cameraPosition, KLSeis::KLDisplay3D::OctreeNode *node)
{
	osg::Vec4d plane;
	float center[3];
	int bounds[6] = {
		node->_minSample, node->_maxSample,
		node->_minCMP, node->_maxCMP,
		node->_minRecord, node->_maxRecord
	};

	int dim[3];
	dim[0] = node->_sampleNum;
	dim[1] = node->_CMPNum;
	dim[2] = node->_recordNum;

	center[0] = (bounds[0] + bounds[1]) / 2.0;
	center[1] = (bounds[2] + bounds[3]) / 2.0;
	center[2] = (bounds[4] + bounds[5]) / 2.0;

	// ���ӵ������߷���ֱ��ƽ�淽��
	plane[0] = center[0] - cameraPosition[0];
	plane[1] = center[1] - cameraPosition[1];
	plane[2] = center[2] - cameraPosition[2];

	plane.normalize();

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
				vertices->push_back(osg::Vec3d(bounds[i], bounds[2 + j], bounds[4 + k]));
				// ���㵽ƽ��ľ���
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
	double stepSize = node->_tileStep;

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

		planeDotLineOrigin = plane * (*vertices)[lines[i][0]] - plane[3]; // vec4d *���� +plane[3]
		planeDotLine       = plane * line - plane[3];

		if (planeDotLine != 0.0)
		{
			t = (d - planeDotLineOrigin - plane[3]) / planeDotLine;
			increment = -stepSize / planeDotLine;
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
		}
	}

	// ͨ�����ڱ߲��ҽ���
	int neighborLines[12][6] = {
		{1, 2, 3, 4,  8,  9}, {0, 2, 3, 5, 9, 10},
		{0, 1, 3, 6, 10, 11}, {0, 1, 2, 7, 8, 11},
		{0, 5, 6, 7,  8,  9}, {1, 4, 6, 7, 9, 10},
		{2, 4, 5, 7, 10, 11}, {3, 4, 5, 6, 8, 11},
		{0, 3, 4, 7,  9, 11}, {0, 1, 4, 5, 8, 10},
		{1, 2, 5, 6,  9, 11}, {2, 3, 6, 7, 8, 10}
	};

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
			}

			if (idx < 6)
			{
				*(pptr + idx * 6) = -1;
			}
		}

		iptr += 12;
		pptr += 36;
	}
}

END_KLDISPLAY3D_NAMESPACE
