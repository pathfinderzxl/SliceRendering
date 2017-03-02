#include <iostream>
#include "KL3DDisplayList.h"
#include "osg/Geometry"
#include <osgGA/CameraManipulator>


BEGIN_KLDISPLAY3D_NAMESPACE

class KLOctreeNode;
class KL3DVOctree;

KL3DDisplayList::KL3DDisplayList()
{
	m_MAXNUMDISPLAYLIST=100;
}

// 子块加载顺序
void KL3DDisplayList::setLoadOrder(KLOctreeNode *node, const osg::Vec3 cameraPosition, int order[8])
{
	KLOctreeNode *p = node->m_children[0];
	if (cameraPosition[0] >= p->m_maxSample && cameraPosition[1] >= p->m_maxCMP && cameraPosition[2] >= p->m_maxRecord)
	{
		order[0] = 7; order[1] = 3; order[
			
			2] = 5; order[3] = 6;
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

void KL3DDisplayList::updateDisplayList(KL3DVOctree *octree , const osg::Vec3 cameraPosition,osgViewer::Viewer* viewer)
{
	// 小数据没有生成八叉树，不需要更新列表
	if (octree == NULL)
		return ;
	//计算视景体的六个平面


	osg::Vec3d eye; osg::Vec3d center;
	osg::Vec3d up; 
	osg::ref_ptr<osg::Camera> camera = viewer->getCamera();
	camera->getViewMatrixAsLookAt(eye,center,up);

	//computeNearFar(viewer,octree->m_rootNode);
	calculateFrustumPlanes(viewer);

	std::list<KLOctreeNode *> tmpList;
	int num = 0, order[8];
	//测试裁剪
	int noCull = 0;
	//std::cout<<"p1=["<<octree->m_rootNode.m_minCMP<<","<<octree->m_rootNode.min
	(octree->m_rootNode)->m_isInFrustum = 1;
	if ((octree->m_rootNode)->m_isInFrustum)
	{
		m_DisplayNodeList.push_back(octree->m_rootNode);
	}
	/*std::cout<<"体块边界"<<std::endl;
	std::cout<<(octree->m_rootNode)->m_minCMP<<","<<(octree->m_rootNode)->m_minRecord<<","<<(octree->m_rootNode)->m_minSample<<std::endl;
	std::cout<<(octree->m_rootNode)->m_maxCMP<<","<<(octree->m_rootNode)->m_maxRecord<<","<<(octree->m_rootNode)->m_maxSample<<std::endl;*/
	std::list<KLOctreeNode *> tmpList2;
	m_DisplayNodeList.swap(tmpList2);
	KLOctreeNode *node = octree->m_rootNode;
	m_DisplayNodeList.push_back(node);

	// 从根节点开始，按层遍历八叉树各节点
	for (int loop = 0; loop <  octree->m_depth; ++loop)
	{

		// 离视点最近的体块分辨率最高
		// 视点与体块的距离达到某一阈值，该体块即为合适分辨率
		if(m_DisplayNodeList.size() == 0){continue;}
		//if (computeDistance(cameraPosition, m_DisplayNodeList.front()->m_center)
		//	>= m_DisplayNodeList.front()->m_diagnose * 5.0)
		//{
		//	/*std::cout<<"未裁剪应加载的节点数(体块分辨率合适)："<<noCull<<"\n";
		//	std::cout<<"裁剪后加载的节点数（体块分辨率合适）："<<num<<"\n";*/

		//	return ;
		//}

		num = m_DisplayNodeList.size();
		noCull = num;

		tmpList = m_DisplayNodeList;
		m_DisplayNodeList.clear();

		KLOctreeNode *firstNode = tmpList.front(), *tmpNode;
		std::list<KLOctreeNode *>::iterator itrNode;
		for (itrNode = tmpList.begin(); itrNode != tmpList.end(); ++itrNode)
		{
			tmpNode = *itrNode;

			// 视点位置与体块结点的位置关系（27种）：体块的6个面将空间分为27个部分，视点对应有27种位置
			// 可视体块与视点的27种位置关系，相应的不可见体块有27种情况
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
				tmpNode->m_visible = false;

				m_DisplayNodeList.push_back(tmpNode); // 不可见体块


			}
			else 
			{
				// 体块分辨率合适
				double distance = computeDistance(cameraPosition, tmpNode->m_center);
				if (distance >= tmpNode->m_diagnose * 5.0
					|| tmpNode->m_entropy == 0
					) 
				{
					m_DisplayNodeList.push_back(tmpNode);

				}
				else if (num - 1 + tmpNode->m_numChild <1000/*m_MAXNUMDISPLAYLIST*/) // 内存限制，是否超过最大体块数
				{
					// 体块分辨率小，内存充足，提高分辨率
					setLoadOrder(tmpNode, cameraPosition, order);
					for (int j=0; j<8; ++j)
					{
						if (tmpNode->m_children[order[j]])
						{
							m_DisplayNodeList.push_back(tmpNode->m_children[order[j]]);
						}
					}

#if 0//视景体裁剪
					num = num-1;
					noCull = noCull-1;
					if (tmpNode->m_isInFrustum == 1)//体块与视景体相交
					{
						for (int j = 0; j < 8; ++j)
							if (tmpNode->m_children[order[j]])
							{
								int type = isVolumeInFrustum(tmpNode->m_children[order[j]]);

								if(type>0)
								{
									tmpNode->m_children[order[j]]->m_isInFrustum = type;
									m_DisplayNodeList.push_back(tmpNode->m_children[order[j]]);
									num ++;
								}
								else
								{
									/*std::cout<<tmpNode->m_children[order[j]]->m_minCMP<<","<<tmpNode->m_children[order[j]]->m_minRecord<<","<<tmpNode->m_children[order[j]]->m_minSample<<std::endl;
									std::cout<<tmpNode->m_children[order[j]]->m_maxCMP<<","<<tmpNode->m_children[order[j]]->m_maxRecord<<","<<tmpNode->m_children[order[j]]->m_maxSample<<std::endl;*/
								}

							}
					}
					else if (tmpNode->m_isInFrustum == 2)//体块完全在视景体内
					{
						for (int j = 0; j < 8; ++j)
							if (tmpNode->m_children[order[j]])
							{
								tmpNode->m_children[order[j]]->m_isInFrustum = 2;//子块也完全在视景体内
								m_DisplayNodeList.push_back(tmpNode->m_children[order[j]]);
							}
							num += tmpNode->m_numChild;
					}

					//test
					noCull += tmpNode->m_numChild;
#endif
				}
				else 
				{
					// 体块分辨率小，内存达到上限，不再细化
					for (; itrNode != tmpList.end(); ++itrNode)
						m_DisplayNodeList.push_back(*itrNode);
					/*std::cout<<"未裁剪应加载的节点数(显示列表达到最大长度)："<<noCull<<"\n";
					std::cout<<"裁剪后加载的节点数(显示列表达到最大长度)："<<m_DisplayNodeList.size()<<"\n";*/
					return ;
				}
			}
		}
	}
	//test

}

void KL3DDisplayList::updateList(KL3DVOctree *octree, const osg::Vec3 cameraPosition, osgViewer::Viewer *viewer, int depth)
{
	// 小数据没有生成八叉树，不需要更新列表
	if (octree == NULL)
		return ;
	//计算视景体的六个平面


	osg::Vec3d eye; osg::Vec3d center;
	osg::Vec3d up; 
	osg::ref_ptr<osg::Camera> camera = viewer->getCamera();
	camera->getViewMatrixAsLookAt(eye,center,up);

	//computeNearFar(viewer,octree->m_rootNode);
	calculateFrustumPlanes(viewer);

	std::list<KLOctreeNode *> tmpList;
	int num = 0, order[8];
	//测试裁剪
	int noCull = 0;
	//std::cout<<"p1=["<<octree->m_rootNode.m_minCMP<<","<<octree->m_rootNode.min
	(octree->m_rootNode)->m_isInFrustum = 1;
	if ((octree->m_rootNode)->m_isInFrustum)
	{
		m_DisplayNodeList.push_back(octree->m_rootNode);
	}
	//std::cout<<"体块边界"<<std::endl;
	//std::cout<<(octree->m_rootNode)->m_minCMP<<","<<(octree->m_rootNode)->m_minRecord<<","<<(octree->m_rootNode)->m_minSample<<std::endl;
	//std::cout<<(octree->m_rootNode)->m_maxCMP<<","<<(octree->m_rootNode)->m_maxRecord<<","<<(octree->m_rootNode)->m_maxSample<<std::endl;
	std::list<KLOctreeNode *> tmpList2;
	m_DisplayNodeList.swap(tmpList2);
	KLOctreeNode *node = octree->m_rootNode;
	m_DisplayNodeList.push_back(node);

	// 从根节点开始，按层遍历八叉树各节点
	for (int loop = 0; loop < depth; ++loop)
	{

		// 离视点最近的体块分辨率最高
		// 视点与体块的距离达到某一阈值，该体块即为合适分辨率
		if(m_DisplayNodeList.size() == 0){continue;}
		if (computeDistance(cameraPosition, m_DisplayNodeList.front()->m_center)
			>= m_DisplayNodeList.front()->m_diagnose * 3.5
			)
		{
			return ;
		}

		num = m_DisplayNodeList.size();
		noCull = num;

		tmpList = m_DisplayNodeList;
		m_DisplayNodeList.clear();

		KLOctreeNode *firstNode = tmpList.front(), *tmpNode;
		std::list<KLOctreeNode *>::iterator itrNode;
		for (itrNode = tmpList.begin(); itrNode != tmpList.end(); ++itrNode)
		{
			tmpNode = *itrNode;

			// 视点位置与体块结点的位置关系（27种）：体块的6个面将空间分为27个部分，视点对应有27种位置
			// 可视体块与视点的27种位置关系，相应的不可见体块有27种情况
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
				tmpNode->m_visible = false;

				m_DisplayNodeList.push_back(tmpNode); // 不可见体块


			}
			else 
			{
				// 体块分辨率合适
				if (computeDistance(cameraPosition, tmpNode->m_center) >= tmpNode->m_diagnose * 3.5
					|| tmpNode->m_entropy < 0.5
					) 
				{
					cout<<tmpNode->m_entropy<<endl;
					m_DisplayNodeList.push_back(tmpNode);

				}
				else if (num - 1 + tmpNode->m_numChild <= m_MAXNUMDISPLAYLIST) // 内存限制，是否超过最大体块数
				{
					// 体块分辨率小，内存充足，提高分辨率
					setLoadOrder(tmpNode, cameraPosition, order);

					num = num-1;
					noCull = noCull-1;
					if (tmpNode->m_isInFrustum == 1)//体块与视景体相交
					{
						for (int j = 0; j < 8; ++j)
							if (tmpNode->m_children[order[j]])
							{
								int type = isVolumeInFrustum(tmpNode->m_children[order[j]]);


								if(type>0)
								{
									tmpNode->m_children[order[j]]->m_isInFrustum = type;
									m_DisplayNodeList.push_back(tmpNode->m_children[order[j]]);
									num ++;
								}
								else
								{
									//std::cout<<tmpNode->m_children[order[j]]->m_minCMP<<","<<tmpNode->m_children[order[j]]->m_minRecord<<","<<tmpNode->m_children[order[j]]->m_minSample<<std::endl;
									//std::cout<<tmpNode->m_children[order[j]]->m_maxCMP<<","<<tmpNode->m_children[order[j]]->m_maxRecord<<","<<tmpNode->m_children[order[j]]->m_maxSample<<std::endl;
								}

							}
					}
					else if (tmpNode->m_isInFrustum == 2)//体块完全在视景体内
					{
						for (int j = 0; j < 8; ++j)
							if (tmpNode->m_children[order[j]])
							{
								tmpNode->m_children[order[j]]->m_isInFrustum = 2;//子块也完全在视景体内
								m_DisplayNodeList.push_back(tmpNode->m_children[order[j]]);
							}
							num += tmpNode->m_numChild;
					}

					//test
					noCull += tmpNode->m_numChild;

				}
				else 
				{
					// 体块分辨率小，内存达到上限，不再细化
					for (; itrNode != tmpList.end(); ++itrNode)
						m_DisplayNodeList.push_back(*itrNode);


					std::cout<<"未裁剪应加载的节点数(显示列表达到最大长度)："<<noCull<<"\n";
					std::cout<<"裁剪后加载的节点数(显示列表达到最大长度)："<<m_DisplayNodeList.size()<<"\n";
					return ;
				}
			}
		}
	}
	//test
}




void KL3DDisplayList::calculateFrustumPlanes(osgViewer::Viewer *viewer)
{
	osg::Vec3dArray* vertexes = calculatePoints(viewer);
	int x = vertexes->size();
	osg::Vec3dArray::iterator vItr = vertexes->begin();
	const osg::Vec3d v0 = *vItr++;
	const osg::Vec3d v1 = *vItr++;
	const osg::Vec3d v2 = *vItr++;
	const osg::Vec3d v3 = *vItr++;
	const osg::Vec3d v4 = *vItr++;
	const osg::Vec3d v5 = *vItr++;
	const osg::Vec3d v6 = *vItr++;
	const osg::Vec3d v7 = *vItr;


	osg::Vec3d v40 = v0-v4;
	osg::Vec3d v45 = v5-v4;
	osg::Vec3d normal0 = v40^v45;
	normal0.normalize();
	frustumPlanes[0][0] = normal0.x();
	frustumPlanes[0][1] = normal0.y();
	frustumPlanes[0][2] = normal0.z();
	frustumPlanes[0][3] = -normal0*v1;


	osg::Vec3d v26 = v6-v2;
	osg::Vec3d v23 = v3-v2;
	osg::Vec3d normal1 = v26^v23;
	normal1.normalize();
	frustumPlanes[1][0] = normal1.x();
	frustumPlanes[1][1] = normal1.y();
	frustumPlanes[1][2] = normal1.z();
	frustumPlanes[1][3] = -normal1*v7;


	osg::Vec3d v37 = v7-v3;
	osg::Vec3d v31 = v1-v3;
	osg::Vec3d normal2 = v37^v31;
	normal2.normalize();
	frustumPlanes[2][0] = normal2.x();
	frustumPlanes[2][1] = normal2.y();
	frustumPlanes[2][2] = normal2.z();
	frustumPlanes[2][3] = -normal2*v5;


	osg::Vec3d v46 = v6-v4;
	v40 = v0-v4;
	osg::Vec3d normal3 = v46^v40;
	normal3.normalize();
	frustumPlanes[3][0] = normal3.x();
	frustumPlanes[3][1] = normal3.y();
	frustumPlanes[3][2] = normal3.z();
	frustumPlanes[3][3] = -normal3*v2;

	//前后
	/*osg::Vec3d v10 = v0-v1;
	osg::Vec3d v13 = v3-v1;
	osg::Vec3d normal4 = v10^v13;
	normal4.normalize();
	frustumPlanes[4][0] = normal4.x();
	frustumPlanes[4][1] = normal4.y();
	frustumPlanes[4][2] = normal4.z();
	frustumPlanes[4][3] = -normal4*v2;


	osg::Vec3d v64 = v4-v6;
	osg::Vec3d v67 = v7-v6;
	osg::Vec3d normal5 = v64^v67;
	normal5.normalize();
	frustumPlanes[5][0] = normal5.x();
	frustumPlanes[5][1] = normal5.y();
	frustumPlanes[5][2] = normal5.z();
	frustumPlanes[5][3] = -normal5*v5;
	*/
	
}


/*
计算视景体的八个顶点
*/
osg::ref_ptr<osg::Vec3dArray> KL3DDisplayList::calculatePoints(osgViewer::Viewer *viewer)
{
	osg::ref_ptr<osgGA::CameraManipulator> cm = viewer->getCameraManipulator();
	osg::ref_ptr<osg::Camera> camera = viewer->getCamera();
	/*视景体计算*/
	//视点位置和视线的方向向量
	osg::Vec3d eye,center,up,right,sightDirection;
	//近平面和远平面到视点的距离，以及两平面的宽、高,视野角度
	double nearDist,farDist,nearH,farH,nearW,farW,fovy,ratio;

	//((osgViewer::Renderer*)camera->getRenderer())->getSceneView(0)->setComputeNearFarMode(osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR);

	camera->getViewMatrixAsLookAt(eye,center,up);
	camera->getProjectionMatrixAsPerspective(fovy,ratio,nearDist,farDist);
	//计算近、远平面的高和宽
	nearH = 2*tan(fovy/2/180*3.1415926)*nearDist;
	nearW = nearH*ratio;
	farH = 2*tan(fovy/2/180*3.1415926)*farDist;
	farW = farH*ratio;

	/*计算视景体的八个顶点*/
	osg::ref_ptr<osg::Vec3dArray> vertexes = new osg::Vec3dArray;
	osg::Vec3d ftl,ftr,fbl,fbr,ntl,ntr,nbl,nbr;
	sightDirection = center-eye;
	sightDirection.normalize();

	right = sightDirection^up;
	right.normalize();

	//近平面的中心点
	osg::Vec3d nearCenter = eye+sightDirection*nearDist;
	ntl = nearCenter-right*(nearW/2)+up*(nearH/2);
	//std::cout<<"p1=["<<ntl.x()<<","<<ntl.y()<<","<<ntl.z()<<"]"<<std::endl;
	vertexes->push_back(ntl);

	nbl = nearCenter-right*(nearW/2)-up*(nearH/2);
	//std::cout<<"p2=["<<nbl.x()<<","<<nbl.y()<<","<<nbl.z()<<"]"<<std::endl;
	vertexes->push_back(nbl);

	ntr = nearCenter+right*(nearW/2)+up*(nearH/2);
	//std::cout<<"p3=["<<ntr.x()<<","<<ntr.y()<<","<<ntr.z()<<"]"<<std::endl;
	vertexes->push_back(ntr);

	nbr = nearCenter+right*(nearW/2)-up*(nearH/2);
	//std::cout<<"p4=["<<nbr.x()<<","<<nbr.y()<<","<<nbr.z()<<"]"<<std::endl;
	vertexes->push_back(nbr);

	//远平面中心点
	osg::Vec3d farCenter = eye+sightDirection*farDist;
	ftl = farCenter-right*(farW/2)+up*(farH/2);
	//std::cout<<"p5=["<<ftl.x()<<","<<ftl.y()<<","<<ftl.z()<<"]"<<std::endl;
	vertexes->push_back(ftl);

	fbl = farCenter-right*(farW/2)-up*(farH/2);
	//std::cout<<"p6=["<<fbl.x()<<","<<fbl.y()<<","<<fbl.z()<<"]"<<std::endl;
	vertexes->push_back(fbl);

	ftr = farCenter+right*(farW/2)+up*(farH/2);
	//std::cout<<"p7=["<<ftr.x()<<","<<ftr.y()<<","<<ftr.z()<<"]"<<std::endl;
	vertexes->push_back(ftr);

	fbr = farCenter+right*(farW/2)-up*(farH/2);
	//std::cout<<"p8=["<<fbr.x()<<","<<fbr.y()<<","<<fbr.z()<<"]"<<std::endl;
	vertexes->push_back(fbr);
	//std::cout<<"===============================\n";

	/*vertexes->ref();
	if(eye.y()-(-1497.7154541015625)<1)return vertexes;

	frustumGroup->removeChildren(0,frustumGroup->getNumChildren());
	osg::ref_ptr<osg::Geode> geode1 = new osg::Geode;
	geode1->removeDrawables(0,geode1->getNumDrawables());
	osg::ref_ptr<osg::Geometry> g1 = new osg::Geometry;
	geode1->addDrawable(g1);

	g1->setVertexArray(vertexes);

	osg::DrawElementsUInt* line = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP, 0);
	line->push_back(0);
	line->push_back(1);
	line->push_back(3);
	line->push_back(2);
	g1->addPrimitiveSet(line);
	frustumGroup->addChild(geode1);


	osg::ref_ptr<osg::Geometry> g2 = new osg::Geometry;
	geode1->addDrawable(g2);
	g2->setVertexArray(vertexes);
	osg::ref_ptr<osg::DrawElementsUInt> l2 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP,0);
	l2->push_back(2);
	l2->push_back(6);
	l2->push_back(7);
	l2->push_back(3);
	g2->addPrimitiveSet(l2);

	osg::ref_ptr<osg::Geometry> g3 = new osg::Geometry;
	geode1->addDrawable(g3);
	g3->setVertexArray(vertexes);
	osg::ref_ptr<osg::DrawElementsUInt> l3 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP,0);
	l3->push_back(6);
	l3->push_back(4);
	l3->push_back(5);
	l3->push_back(7);
	g3->addPrimitiveSet(l3);


	osg::ref_ptr<osg::Geometry> g4 = new osg::Geometry;

	geode1->addDrawable(g4);
	g4->setVertexArray(vertexes);
	osg::ref_ptr<osg::DrawElementsUInt> l4 = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP,0);
	l4->push_back(0);
	l4->push_back(1);
	l4->push_back(5);
	l4->push_back(4);
	g4->addPrimitiveSet(l4);


	osg::ref_ptr<osg::Geometry> g5 = new osg::Geometry;
	geode1->addDrawable(g5);
	osg::ref_ptr<osg::DrawElementsUInt> l5 = new osg::DrawElementsUInt;
	g5->setVertexArray(vertexes);
	l5->push_back(0);
	l5->push_back(2);
	l5->push_back(6);
	l5->push_back(4);
	g5->addPrimitiveSet(l5);


	osg::ref_ptr<osg::Geometry> g6 = new osg::Geometry;
	geode1->addDrawable(g6);
	osg::ref_ptr<osg::DrawElementsUInt> l6 = new osg::DrawElementsUInt;
	g6->setVertexArray(vertexes);
	l6->push_back(1);
	l6->push_back(3);
	l6->push_back(2);
	l6->push_back(5);
	g6->addPrimitiveSet(l6);*/


	vertexes->ref();
	return vertexes;


}


int KL3DDisplayList::isVolumeInFrustum(KLOctreeNode* node)
{

	if (isSphereInFrustum(node))
	{//体块完全在视景体内
		return 2;
	}
	for (int p=0;p<4;p++)
	{

		if(frustumPlanes[p][0]*node->m_minSample
			+frustumPlanes[p][1]*node->m_minCMP
			+frustumPlanes[p][2]*node->m_minRecord
			+frustumPlanes[p][3]>0)
		{
			continue;
		}
		if (frustumPlanes[p][0]*node->m_maxSample
			+frustumPlanes[p][1]*node->m_minCMP
			+frustumPlanes[p][2]*node->m_minRecord
			+frustumPlanes[p][3]>0)
		{
			continue;
		}
		if (frustumPlanes[p][0]*node->m_minSample
			+frustumPlanes[p][1]*node->m_maxCMP
			+frustumPlanes[p][2]*node->m_minRecord
			+frustumPlanes[p][3]>0)
		{
			continue;
		}
		if (frustumPlanes[p][0]*node->m_maxSample
			+frustumPlanes[p][1]*node->m_maxCMP
			+frustumPlanes[p][2]*node->m_minRecord
			+frustumPlanes[p][3]>0)
		{
			continue;
		}
		if (frustumPlanes[p][0]*node->m_minSample
			+frustumPlanes[p][1]*node->m_minCMP
			+frustumPlanes[p][2]*node->m_maxRecord
			+frustumPlanes[p][3]>0)
		{
			continue;
		}
		if (frustumPlanes[p][0]*node->m_maxSample
			+frustumPlanes[p][1]*node->m_minCMP
			+frustumPlanes[p][2]*node->m_maxRecord
			+frustumPlanes[p][3]>0)
		{
			continue;
		}

		if (frustumPlanes[p][0]*node->m_minSample
			+frustumPlanes[p][1]*node->m_maxCMP
			+frustumPlanes[p][2]*node->m_maxRecord
			+frustumPlanes[p][3]>0)
		{
			continue;
		}

		if (frustumPlanes[p][0]*node->m_maxSample
			+frustumPlanes[p][1]*node->m_maxCMP
			+frustumPlanes[p][2]*node->m_maxRecord
			+frustumPlanes[p][3]>0)
		{
			continue; 
		}
		return 0;
	}
	/*=========================================*/

	//int totalNum = 0;
	//for (int p=0;p<6;p++)
	//{
	//	int num = 0;
	//	if(frustumPlanes[p][0]*node->m_minCMP
	//		+frustumPlanes[p][1]*node->m_minRecord
	//		+frustumPlanes[p][2]*node->m_minSample
	//		+frustumPlanes[p][3]>0)
	//	{
	//		num++;
	//	}
	//	if (frustumPlanes[p][0]*node->m_maxCMP
	//		+frustumPlanes[p][1]*node->m_minRecord
	//		+frustumPlanes[p][2]*node->m_minSample
	//		+frustumPlanes[p][3]>0)
	//	{
	//		num++;
	//	}
	//	if (frustumPlanes[p][0]*node->m_minCMP
	//		+frustumPlanes[p][1]*node->m_maxRecord
	//		+frustumPlanes[p][2]*node->m_minSample
	//		+frustumPlanes[p][3]>0)
	//	{
	//		num++;
	//	}
	//	if (frustumPlanes[p][0]*node->m_maxCMP
	//		+frustumPlanes[p][1]*node->m_maxRecord
	//		+frustumPlanes[p][2]*node->m_minSample
	//		+frustumPlanes[p][3]>0)
	//	{
	//		num++;
	//	}
	//	if (frustumPlanes[p][0]*node->m_minCMP
	//		+frustumPlanes[p][1]*node->m_minRecord
	//		+frustumPlanes[p][2]*node->m_maxSample
	//		+frustumPlanes[p][3]>0)
	//	{
	//		num++;
	//	}
	//	if (frustumPlanes[p][0]*node->m_maxCMP
	//		+frustumPlanes[p][1]*node->m_minRecord
	//		+frustumPlanes[p][2]*node->m_maxSample
	//		+frustumPlanes[p][3]>0)
	//	{
	//		num++;
	//	}

	//	if (frustumPlanes[p][0]*node->m_minCMP
	//		+frustumPlanes[p][1]*node->m_maxRecord
	//		+frustumPlanes[p][2]*node->m_maxSample
	//		+frustumPlanes[p][3]>0)
	//	{
	//		num++;
	//	}

	//	if (frustumPlanes[p][0]*node->m_maxCMP
	//		+frustumPlanes[p][1]*node->m_maxRecord
	//		+frustumPlanes[p][2]*node->m_maxSample
	//		+frustumPlanes[p][3]>0)
	//	{
	//		num++;
	//	}
	//
	//	if (num==0)
	//	{
	//		return 0;//完全在视景体外
	//	}
	//	else{
	//		totalNum += num;
	//	}
	//	
	//}


	return 1;//与视景体相交
}


bool KL3DDisplayList::isSphereInFrustum(KLOctreeNode* node)
{
	/*包围球的半径*/
	double radius = 0.5*sqrt(double((node->m_maxSample-node->m_minSample)*(node->m_maxSample-node->m_minSample)
		+(node->m_maxCMP-node->m_minCMP)*(node->m_maxCMP-node->m_minCMP)
		+(node->m_maxRecord-node->m_minRecord)*(node->m_maxRecord-node->m_minRecord)));
	/*
	判断体块的中心到每个平面的距离，如果小于radius或者体块的中心点在视景体外则可以断定体块的包围球不完全在视景体内。
	*/
	for (int i=0;i<4;i++)
	{
		double d = frustumPlanes[i][0]*node->m_center[0]
		+frustumPlanes[i][1]*node->m_center[1]
		+frustumPlanes[i][2]*node->m_center[2]
		+frustumPlanes[i][3];
		if(d<radius)
		{
			return false;
		}
		//计算包围球的中心到平面的距离
		/*double distence = abs(frustumPlanes[i][0]*node->m_center[0]
		+frustumPlanes[i][1]*node->m_center[1]
		+frustumPlanes[i][2]*node->m_center[2]
		+frustumPlanes[i][3])/sqrt(frustumPlanes[i][0]*frustumPlanes[i][0]
		+frustumPlanes[i][1]*frustumPlanes[i][1]
		+frustumPlanes[i][2]*frustumPlanes[i][2]);


		if (distence<radius)
		{
		return false;
		}*/
	}
	return true;
}

void KL3DDisplayList::generateBasicSliceNodeList(KL3DVOctree *octree, int basicLevel)
{
	if (NULL == octree)
	{
		return;
	}

	m_basicSliceNodeList.push_back(octree->m_rootNode);
	KLOctreeNode *tmpNode = m_basicSliceNodeList.front();

	while (tmpNode->m_level <= basicLevel)
	{
		m_basicSliceNodeList.pop_front();
		for (int i=0; i<8; ++i)
		{
			if (tmpNode->m_children[i])
			{
				m_basicSliceNodeList.push_back(tmpNode->m_children[i]);
			}
		}

		tmpNode = m_basicSliceNodeList.front();
	}
}

void KL3DDisplayList::generateRefineSliceNodeList(KL3DVOctree *octree, int direct, int depth, int refineLevel)
{
	if (NULL == octree)
	{
		return;
	}

	m_refineSliceNodeList.clear();
	m_refineSliceNodeList.push_back(octree->m_rootNode);
	list<KLOctreeNode*> tmpList;
	
	float bounds[6];
	for (int level=0; level<=refineLevel; ++level)
	{
		m_refineSliceNodeList.swap(tmpList);
		m_refineSliceNodeList.clear();
		list<KLOctreeNode*>::iterator itrNode = tmpList.begin();
		for (; itrNode!=tmpList.end(); ++itrNode)
		{
			KLOctreeNode *node = *itrNode;
			node->getNodeBounds(bounds);
			if (depth < bounds[direct] || depth > bounds[direct+3])
			{
				continue;
			}
			
			if (node->m_entropy < 0.1)
			{
				m_refineSliceNodeList.push_back(node);
				continue;
			}

			for (int i=0; i<8; ++i)
			{
				if (NULL == node->m_children[i])
				{
					continue;
				}

				node->m_children[i]->getNodeBounds(bounds);
				if (depth >= bounds[direct] || depth <= bounds[direct+3])
				{
					m_refineSliceNodeList.push_back(node->m_children[i]);
				}
			}

		}

		

	}

}

void KL3DDisplayList::refineSliceNodeList(KL3DVOctree *octree, int direct, osgViewer::Viewer *viewer, int depth)
{
	if (NULL == octree)
	{
		return;
	}

	osgGA::CameraManipulator *cm = viewer->getCameraManipulator();
	osg::Vec3d cameraPosition;
	osg::Matrix matrix1 = cm->getMatrix();
	cameraPosition = osg::Vec3d(0.0, 0.0, 0.0) * matrix1;

	int order[8];
	int d = 0;
	m_refineSliceNodeList.clear();
	m_refineSliceNodeList.push_back(octree->m_rootNode);
	list<KLOctreeNode*> tmpList;

	float bounds[6];
	for (int level=0; level<octree->m_depth; ++level)
	{
		m_refineSliceNodeList.swap(tmpList);
		m_refineSliceNodeList.clear();
		
		int nodeNum = tmpList.size();
		if (nodeNum >= m_MAXNUMDISPLAYLIST)
		{
			m_refineSliceNodeList.swap(tmpList);
			return;
		}

		list<KLOctreeNode*>::iterator itrNode = tmpList.begin();
		for (; itrNode!=tmpList.end(); ++itrNode)
		{
			--nodeNum;
			KLOctreeNode *node = *itrNode;

			if (node->m_entropy < 0.1)
			{
				m_refineSliceNodeList.push_back(node);
				continue;
			}

			for (int i=0; i<8; ++i)
			{
				order[i] = i;
			}
			if (node->m_level<octree->m_depth)
			{
				setLoadOrder(node, cameraPosition, order);
			}
			int n = 0;
			for (int i=0; i<8; ++i)
			{
				if (NULL == node->m_children[i])
				{
					continue;
				}

				node->m_children[i]->getNodeBounds(bounds);
				if (depth >= bounds[direct] && depth <= bounds[direct+3])
				{
					//m_refineSliceNodeList.push_back(node->m_children[i]);
					++n;
				}
			}
			if (m_refineSliceNodeList.size() + n + nodeNum <= m_MAXNUMDISPLAYLIST)
			{
				for (int i=0; i<8; ++i)
				{
					if (NULL == node->m_children[order[i]])
					{
						continue;
					}

					node->m_children[order[i]]->getNodeBounds(bounds);
					if (depth >= bounds[direct] && depth <= bounds[direct+3])
					{
						m_refineSliceNodeList.push_back(node->m_children[order[i]]);
						if (node->m_children[order[i]]->m_level > d)
						{
							d = node->m_children[order[i]]->m_level;
						}
					}
				}
			}
			else
			{
				m_refineSliceNodeList.push_back(node);
				if (node->m_level > d)
				{
					d = node->m_level;
				}
			}

		}
	}
	cout<<"level: "<<d<<endl;
}

/*
list<KLOctreeNode*> KL3DDisplayList::refineSliceNode( int octreeDepth, KLOctreeNode* node, int direct, osgViewer::Viewer *viewer, int depth )
{

	list<KLOctreeNode*> nodeList;
	osgGA::CameraManipulator *cm = viewer->getCameraManipulator();
	osg::Vec3d cameraPosition;
	osg::Matrix matrix1 = cm->getMatrix();
	cameraPosition = osg::Vec3d(0.0, 0.0, 0.0) * matrix1;

	int order[8];

	nodeList.clear();
	nodeList.push_back(node);
	list<KLOctreeNode*> tmpList;

	float bounds[6];
	for (int level=0; level<octreeDepth; ++level)
	{
		nodeList.swap(tmpList);
		nodeList.clear();

		int nodeNum = tmpList.size();
		if (nodeNum >= m_MAXNUMDISPLAYLIST)
		{
			nodeList.swap(tmpList);
			return;
		}

		list<KLOctreeNode*>::iterator itrNode = tmpList.begin();
		for (; itrNode!=tmpList.end(); ++itrNode)
		{
			--nodeNum;
			KLOctreeNode *node = *itrNode;

			if (node->m_entropy < 0.1)
			{
				nodeList.push_back(node);
				continue;
			}

			for (int i=0; i<8; ++i)
			{
				order[i] = i;
			}
			if (node->m_level<octreeDepth)
			{
				setLoadOrder(node, cameraPosition, order);
			}
			int n = 0;
			for (int i=0; i<8; ++i)
			{
				if (NULL == node->m_children[i])
				{
					continue;
				}

				node->m_children[i]->getNodeBounds(bounds);
				if (depth >= bounds[direct] && depth <= bounds[direct+3])
				{
					//m_refineSliceNodeList.push_back(node->m_children[i]);
					++n;
				}
			}
			if (nodeList.size() + n + nodeNum <= m_MAXNUMDISPLAYLIST)
			{
				for (int i=0; i<8; ++i)
				{
					if (NULL == node->m_children[order[i]])
					{
						continue;
					}

					node->m_children[order[i]]->getNodeBounds(bounds);
					if (depth >= bounds[direct] && depth <= bounds[direct+3])
					{
						nodeList.push_back(node->m_children[order[i]]);
					}
				}
			}
			else
			{
				nodeList.push_back(node);
			}

		}



	}
	return nodeList;
}
*/
END_KLDISPLAY3D_NAMESPACE


