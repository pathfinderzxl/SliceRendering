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

// 体结点初始化
void KL3DVolume::initialize()
{
	int bounds[6], dim[3]; 
	double spacing[3];
	OctreeNode *node;

	std::list<OctreeNode *>::iterator itrNode;
	for (itrNode = m_octreeNodeList.begin(); itrNode != m_octreeNodeList.end(); ++itrNode)
	{
		node = *itrNode;

		// 将体块作为叶节点加到体结点中
		osg::Geode *block = new osg::Geode();
		osg::StateSet *state = createState(node);
		block->setStateSet(state);

		// 体块六个边界
		bounds[0] = node->_minSample;
		bounds[1] = node->_maxSample;
		bounds[2] = node->_minCMP;
		bounds[3] = node->_maxCMP;
		bounds[4] = node->_minRecord;
		bounds[5] = node->_maxRecord;

		// 体块三个方向的采样点数
		dim[0] = node->_sampleNum;
		dim[1] = node->_CMPNum;
		dim[2] = node->_recordNum;	 

		// 采样间隔
		spacing[0] = node->_sampleDistance[0];
		spacing[1] = node->_sampleDistance[1];
		spacing[2] = node->_sampleDistance[2];		

		// 体块由与视线垂直的多边形切片组成，每个切片对应一个drawable
		for (int i = 0; i < dim[1]; ++i)
		{
			osg::Geometry* polygon = new osg::Geometry();
			block->addDrawable(polygon);

			// 多边形顶点纹理坐标
			osg::Vec3Array* tcoords = new osg::Vec3Array(4);
			(*tcoords)[0].set(0.0f, (float)i / (dim[1] - 1), 0.0f);
			(*tcoords)[3].set(0.0f, (float)i / (dim[1] - 1), 1.0f);		
			(*tcoords)[2].set(1.0f, (float)i / (dim[1] - 1), 1.0f);
			(*tcoords)[1].set(1.0f, (float)i / (dim[1] - 1), 0.0f);		
			polygon->setTexCoordArray(0, tcoords);

			// 多边形顶点坐标
			osg::Vec3Array* polygonVertices = new osg::Vec3Array;
			polygonVertices->push_back(osg::Vec3(bounds[0], (float)(i * spacing[1] + bounds[2]), bounds[4]));
			polygonVertices->push_back(osg::Vec3(bounds[1], (float)(i * spacing[1] + bounds[2]), bounds[4]));
			polygonVertices->push_back(osg::Vec3(bounds[1], (float)(i * spacing[1] + bounds[2]), bounds[5]));
			polygonVertices->push_back(osg::Vec3(bounds[0], (float)(i * spacing[1] + bounds[2]), bounds[5]));
			polygon->setVertexArray(polygonVertices);

			// 多边形由TRIANGLE_FAN构成
			osg::DrawElementsUInt* polygonPS = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_FAN, 0);
			polygonPS->push_back(0);
			polygonPS->push_back(1);
			polygonPS->push_back(2);
			polygonPS->push_back(3);
			polygon->addPrimitiveSet(polygonPS);
		}

		// 将体块对应叶节点加到体结点中
		addChild(block);		
	}
}

// 计算两点间的距离 
double computeDistance(osg::Vec3d point1, double *point2)
{
	return sqrt((point1[0] - point2[0]) * (point1[0] - point2[0])
		+ (point1[1] - point2[1]) * (point1[1] - point2[1])
		+ (point1[2] - point2[2]) * (point1[2] - point2[2]));
}

// 子块加载顺序
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

// 根据视点选择显示列表
void KL3DVolume::updateNodeList(osg::Vec3d cameraPosition)
{
	std::list<OctreeNode *> tmpList;
	int num = 0, order[8];

	m_octreeNodeList.clear();
	OctreeNode *node = m_octree->_rootNode;
	m_octreeNodeList.push_back(node);

	// 从根节点开始，按层遍历八叉树各节点
	for (int loop = 0; loop < m_octree->m_depth; ++loop)
	{
		num = m_octreeNodeList.size();

		// 离视点最近的体块分辨率最高
		// 视点与体块的距离达到某一阈值，该体块即为合适分辨率
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

			// 视点位置与体块结点的位置关系（27种）：体块的6个面将空间分为27个部分，视点对应有27种位置
			// 可视体块与视点的27种位置关系，相应的不可见体块有27种情况
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
				m_octreeNodeList.push_back(tmpNode); // 不可见体块
			}
			else 
			{
				// 体块分辨率合适
				if (computeDistance(cameraPosition, tmpNode->_center) >= tmpNode->_diagnose * 1.5) 
					m_octreeNodeList.push_back(tmpNode);
				else if (num - 1 + tmpNode->_notEmptyChild <= 500) // 内存限制，是否超过最大体块数
				{
					// 体块分辨率小，内存充足，提高分辨率
					setLoadOrder(tmpNode, cameraPosition, order);
					for (int j = 0; j < 8; ++j)
						if (tmpNode->_children[order[j]])
							m_octreeNodeList.push_back(tmpNode->_children[order[j]]);
					num += tmpNode->_notEmptyChild;
				}
				else 
				{
					// 体块分辨率小，内存达到上限，不再细化
					for (; itrNode != tmpList.end(); ++itrNode)
						m_octreeNodeList.push_back(*itrNode);
				}
			}
		}
	}
}

// 三维体更新，重新计算显示列表、计算多边形
void KL3DVolume::updateVolume(osg::Vec3d cameraPosition)
{
	removeChildren(0, this->getNumChildren());

	// 更新显示列表
	updateNodeList(cameraPosition);
	//std::cout<<"VolumeBlockNum:"<<m_octreeNodeList.size()<<std::endl;

	// 将新的显示列表传给子线程，预加载体块数据
	m_cache->setNodesIn(m_octreeNodeList);

	OctreeNode *node;
	std::list<OctreeNode *>::iterator itrNode;
	for (itrNode = m_octreeNodeList.begin(); itrNode != m_octreeNodeList.end(); ++itrNode)
	{
		node = *itrNode;

		// 计算多边形
		computePolygon(cameraPosition, node);

		osg::Geode *block = new osg::Geode();

		for (int i = 0; i < m_numOfPolygon; ++i)
		{
			osg::Vec3Array* polygonVertices = new osg::Vec3Array;
			osg::Vec3Array* tcoords = new osg::Vec3Array;

			float *ptr = m_polygonBuffer + 36 * i;
			int j;

			// 得到切片对应顶点坐标与纹理坐标
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
			// j对应切片与体块的交点个数
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

// 读取体块数据，生成三维纹理
osg::StateSet* KL3DVolume::createState(OctreeNode *node)
{
	// 三维采样点数
	int dim[3];
	dim[0] = node->_sampleNum;
	dim[1] = node->_CMPNum;
	dim[2] = node->_recordNum;

	// 三维纹理数据
	osg::Image* image_3d = new osg::Image;
	image_3d->allocateImage(dim[0], dim[1], dim[2], GL_RGBA, GL_FLOAT);
	image_3d->setInternalTextureFormat(GL_RGBA);

	osg::Vec4 *ptr = (osg::Vec4 *)image_3d->data();
	int s, t, r;
	float val;

	// 从缓存中得到体块数据
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
	texture3D->setResizeNonPowerOfTwoHint(false);  // 纹理支持非2的整次幂
	texture3D->setImage(image_3d);

	osg::StateSet* stateset = new osg::StateSet;
	stateset->setTextureAttributeAndModes(0, texture3D);//,osg::StateAttribute::ON

	// 从stateset得到三维纹理属性,用于响应色表变化
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

	// 过视点与视线方向垂直的平面方程
	plane[0] = center[0] - cameraPosition[0];
	plane[1] = center[1] - cameraPosition[1];
	plane[2] = center[2] - cameraPosition[2];

	plane.normalize();

	plane[3] = -(plane[0] * cameraPosition[0] + plane[1] * cameraPosition[1] + plane[2] * cameraPosition[2]);

	// 体块顶点到过视点平面的距离
	double minDistance = DBL_MAX;
	double maxDistance = -DBL_MAX;

	// 体块的八个顶点
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
				// 顶点到平面的距离
				double d =
					plane[0] * (*vertices)[idx][0] +
					plane[1] * (*vertices)[idx][1] +
					plane[2] * (*vertices)[idx][2] +
					plane[3];

				idx++;

				// 求最值
				minDistance = d < minDistance ? d : minDistance;
				maxDistance = d > maxDistance ? d : maxDistance;
			}
		}
	}

	float spacing[3];
	spacing[0] = spacing[1] = spacing[2] = 1.0;

	double offset = 0.333 * 0.5 * (spacing[0] + spacing[1] + spacing[2]);

	minDistance = minDistance < offset ? offset : minDistance;

	// 相邻多边形的距离
	double stepSize = node->_tileStep;

	// 多边形个数
	int numPolys = static_cast<int>((maxDistance - minDistance) / static_cast<double>(stepSize));

	// 内存检测
	if (m_bufferSize < numPolys)
	{
		delete [] m_polygonBuffer;
		delete [] m_intersectionBuffer;

		m_bufferSize = numPolys;

		m_polygonBuffer = new float [36 * m_bufferSize];
		m_intersectionBuffer = new float [12 * m_bufferSize];
	}

	m_numOfPolygon = numPolys;

	// 计算平面与体块12条边的交点
	int lines[12][2] = {
		{0, 1}, {1, 3}, {2, 3},
		{0, 2}, {4, 5}, {5, 7},
		{6, 7}, {4, 6},	{0, 4},
		{1, 5}, {3, 7}, {2, 6}
	};// 用顶点编号表示12条边

	float *iptr, *pptr;
	osg::Vec3 line;
	double d = maxDistance, planeDotLineOrigin, planeDotLine, t, increment;

	for (i = 0; i < 12; i++)
	{			
		// 边所在直线的方向
		line[0] = (*vertices)[lines[i][1]][0] - (*vertices)[lines[i][0]][0];
		line[1] = (*vertices)[lines[i][1]][1] - (*vertices)[lines[i][0]][1];
		line[2] = (*vertices)[lines[i][1]][2] - (*vertices)[lines[i][0]][2];			

		iptr = m_intersectionBuffer + i;

		planeDotLineOrigin = plane * (*vertices)[lines[i][0]] - plane[3]; // vec4d *重载 +plane[3]
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

	// 通过相邻边查找交点
	int neighborLines[12][6] = {
		{1, 2, 3, 4,  8,  9}, {0, 2, 3, 5, 9, 10},
		{0, 1, 3, 6, 10, 11}, {0, 1, 2, 7, 8, 11},
		{0, 5, 6, 7,  8,  9}, {1, 4, 6, 7, 9, 10},
		{2, 4, 5, 7, 10, 11}, {3, 4, 5, 6, 8, 11},
		{0, 3, 4, 7,  9, 11}, {0, 1, 4, 5, 8, 10},
		{1, 2, 5, 6,  9, 11}, {2, 3, 6, 7, 8, 10}
	};

	// 前3项表示边的起点的纹理坐标，第4项表示边上前3项中发生变化的项的标号
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
		// 查找多边形与体块的相交边
		start = 0;

		while (start < 12 && iptr[start] == -1.0)
		{
			start++;
		}

		if (start == 12)
		{
			pptr[0] = -1.0; // 不相交，没有交点
		}
		else
		{
			int current = start;
			int previous = -1;
			int errFlag = 0;

			idx   = 0;

			// idx表示交点个数，一个平面最多与体块的6条边相交
			while (idx < 6 && !errFlag && (idx == 0 || current != start))
			{
				double t = iptr[current];

				// 多边形纹理坐标
				*(pptr + idx * 6)     = tCoord[current][0];
				*(pptr + idx * 6 + 1) = tCoord[current][1];
				*(pptr + idx * 6 + 2) = tCoord[current][2];

				int coord = static_cast<int>(tCoord[current][3]);
				*(pptr + idx * 6 + coord) = t;

				// 多边形顶点坐标
				*(pptr + idx * 6 + 3) = static_cast<float>((*vertices)[lines[current][0]][0] +
					t * ((*vertices)[lines[current][1]][0] - (*vertices)[lines[current][0]][0]));
				*(pptr + idx * 6 + 4) = static_cast<float>((*vertices)[lines[current][0]][1] + 
					t * ((*vertices)[lines[current][1]][1] - (*vertices)[lines[current][0]][1]));
				*(pptr + idx * 6 + 5) = static_cast<float>((*vertices)[lines[current][0]][2] + 
					t * ((*vertices)[lines[current][1]][2] - (*vertices)[lines[current][0]][2]));

				idx++;

				// 查找下一条未计算的相交边
				j = 0;
				while (j < 6
					&&	(*(m_intersectionBuffer + i * 12 + neighborLines[current][j]) < 0
					|| neighborLines[current][j] == previous)) 
				{
					j++;
				}

				if ( j >= 6 ) // 相交边超过6，报错
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
