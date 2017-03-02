#include <iostream>

#include "KLOctreeNode.h"
#include "QFile.h"
#include "QDataStream.h"

#include "KLCreateOctree.h"
BEGIN_KLDISPLAY3D_NAMESPACE
using namespace std;

KLCreateOctree::KLCreateOctree(QString octreeFileName)
{
	m_pFileIn =new QFile(octreeFileName);
	QDataStream in(m_pFileIn);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);
	in.setByteOrder(QDataStream::LittleEndian);
	if (!m_pFileIn->open(QIODevice::ReadOnly))
	{
		m_rootNode = NULL;
		cout << "Can't ope file: " << octreeFileName.toStdString() << endl;
		return;
	}

	int info[2];
	float value[2];
	int bounds[6];

	m_pFileIn->seek(0);
	m_pFileIn->read((char*)info, 2 * sizeof(int));
	m_pFileIn->read((char*)bounds, 6 * sizeof(int));
	m_pFileIn->read((char*)value, 2 * sizeof(float));
	//fileIn->close();

	m_rootNode = NULL;
	m_zuoYezi = NULL;

	m_nodeNum = 0;

	m_depth		 = info[0];
	m_octreeSize = info[1];

	m_minX = bounds[0];
	m_maxX = bounds[1];
	m_minY = bounds[2];
	m_maxY = bounds[3];
	m_minZ = bounds[4];
	m_maxZ = bounds[5];

	m_minValue = value[0];
	m_maxValue = value[1];

	m_BLOCKSIZE = 128;
}

KLCreateOctree::~KLCreateOctree()
{
	Delete(m_rootNode);
	m_rootNode = NULL;
	m_zuoYezi = NULL;
}

void KLCreateOctree::setInterFirst(int inter)
{
	this->m_gridSizex = inter;
}

void KLCreateOctree::setInterSecond(int inter)
{
	this->m_gridSizey = inter;
}

void KLCreateOctree::setInterThird(int inter)
{
	this->m_gridSizez = inter;
}

void KLCreateOctree::recoverOctree()
{
	cout << "建立八叉树..." << endl;
	completeOctreeNode(m_rootNode, m_depth, 0, m_octreeSize,
		m_minX, m_maxX,
		m_minY, m_maxY,
		m_minZ, m_maxZ);

	setTileInfo();

	cout << "读取文件尾..." << endl;
	readOctreeFileTail();
}

void KLCreateOctree::completeOctreeNode(KLOctreeNode *&node, int depth,
	int level, int tileSize,
	int minX, int maxX,
	int minY, int maxY,
	int minZ, int maxZ)
{
	//depth控制递归次数
	if (depth < 0)
		return;

	//创建八叉树节点并计数
	newTileNode(node, depth, level, minX, maxX, minY, maxY, minZ, maxZ);
	++m_nodeNum;

	if (depth == 0)
		return;

	/*****进入node的子节点*****/

	--depth; ++level; tileSize /= 2;
	int sampleSize = maxX - minX;
	int CMPSize = maxY - minY;
	int recordSize = maxZ - minZ;

	if (sampleSize <= tileSize && CMPSize <= tileSize && recordSize <= tileSize)
	{//0
		node->m_numChild = 1;
		completeOctreeNode(node->m_children[0], depth, level, tileSize,
			minX, maxX,
			minY, maxY,
			minZ, maxZ);
		node->m_children[0]->m_brother = NULL;
	}
	else if (sampleSize > tileSize && CMPSize <= tileSize && recordSize <= tileSize)
	{//01
		node->m_numChild = 2;

		completeOctreeNode(node->m_children[0], depth, level, tileSize,
			minX, minX + tileSize,
			minY, maxY,
			minZ, maxZ);

		completeOctreeNode(node->m_children[1], depth, level, tileSize,
			minX + tileSize, maxX,
			minY, maxY,
			minZ, maxZ);

		//指向兄弟
		node->m_children[0]->m_brother = node->m_children[1];
	}
	else if (sampleSize <= tileSize && CMPSize > tileSize && recordSize <= tileSize)
	{//02
		node->m_numChild = 2;

		completeOctreeNode(node->m_children[0], depth, level, tileSize,
			minX, maxX,
			minY, minY + tileSize,
			minZ, maxZ);

		completeOctreeNode(node->m_children[2], depth, level, tileSize,
			minX, maxX,
			minY + tileSize, maxY,
			minZ, maxZ);

		node->m_children[0]->m_brother = node->m_children[2];
	}
	else if (sampleSize <= tileSize && CMPSize <= tileSize && recordSize > tileSize)
	{//04
		node->m_numChild = 2;

		completeOctreeNode(node->m_children[0], depth, level, tileSize,
			minX, maxX,
			minY, maxY,
			minZ, minZ + tileSize);

		completeOctreeNode(node->m_children[4], depth, level, tileSize,
			minX, maxX,
			minY, maxY,
			minZ + tileSize, maxZ);

		node->m_children[0]->m_brother = node->m_children[4];
	}
	else if (sampleSize > tileSize && CMPSize > tileSize && recordSize <= tileSize)
	{//0123
		node->m_numChild = 4;

		completeOctreeNode(node->m_children[0], depth, level, tileSize,
			minX, minX + tileSize,
			minY, minY + tileSize,
			minZ, maxZ);
		completeOctreeNode(node->m_children[1], depth, level, tileSize,
			minX + tileSize, maxX,
			minY, minY + tileSize,
			minZ, maxZ);
		completeOctreeNode(node->m_children[2], depth, level, tileSize,
			minX, minX + tileSize,
			minY + tileSize, maxY,
			minZ, maxZ);
		completeOctreeNode(node->m_children[3], depth, level, tileSize,
			minX + tileSize, maxX,
			minY + tileSize, maxY,
			minZ, maxZ);

		node->m_children[0]->m_brother = node->m_children[1];
		node->m_children[1]->m_brother = node->m_children[2];
		node->m_children[2]->m_brother = node->m_children[3];
	}
	else if (sampleSize > tileSize && CMPSize <= tileSize && recordSize > tileSize)
	{//0145
		node->m_numChild = 4;

		completeOctreeNode(node->m_children[0], depth, level, tileSize,
			minX, minX + tileSize,
			minY, maxY,
			minZ, minZ + tileSize);
		completeOctreeNode(node->m_children[1], depth, level, tileSize,
			minX + tileSize, maxX,
			minY, maxY,
			minZ, minZ + tileSize);
		completeOctreeNode(node->m_children[4], depth, level, tileSize,
			minX, minX + tileSize,
			minY, maxY,
			minZ + tileSize, maxZ);
		completeOctreeNode(node->m_children[5], depth, level, tileSize,
			minX + tileSize, maxX,
			minY, maxY,
			minZ + tileSize, maxZ);

		node->m_children[0]->m_brother = node->m_children[1];
		node->m_children[1]->m_brother = node->m_children[4];
		node->m_children[4]->m_brother = node->m_children[5];
	}
	else if (sampleSize <= tileSize	&& CMPSize > tileSize && recordSize > tileSize)
	{//	0246
		node->m_numChild = 4;

		completeOctreeNode(node->m_children[0], depth, level, tileSize,
			minX, maxX,
			minY, minY + tileSize,
			minZ, minZ + tileSize);
		completeOctreeNode(node->m_children[2], depth, level, tileSize,
			minX, maxX,
			minY + tileSize, maxY,
			minZ, minZ + tileSize);
		completeOctreeNode(node->m_children[4], depth, level, tileSize,
			minX, maxX,
			minY, minY + tileSize,
			minZ + tileSize, maxZ);
		completeOctreeNode(node->m_children[6], depth, level, tileSize,
			minX, maxX,
			minY + tileSize, maxY,
			minZ + tileSize, maxZ);

		node->m_children[0]->m_brother = node->m_children[2];
		node->m_children[2]->m_brother = node->m_children[4];
		node->m_children[4]->m_brother = node->m_children[6];
	}
	else if (sampleSize > tileSize && CMPSize > tileSize && recordSize > tileSize)
	{//01234567
		node->m_numChild = 8;

		completeOctreeNode(node->m_children[0], depth, level, tileSize,
			minX, minX + tileSize,
			minY, minY + tileSize,
			minZ, minZ + tileSize);
		completeOctreeNode(node->m_children[1], depth, level, tileSize,
			minX + tileSize, maxX,
			minY, minY + tileSize,
			minZ, minZ + tileSize);
		completeOctreeNode(node->m_children[2], depth, level, tileSize,
			minX, minX + tileSize,
			minY + tileSize, maxY,
			minZ, minZ + tileSize);
		completeOctreeNode(node->m_children[3], depth, level, tileSize,
			minX + tileSize, maxX,
			minY + tileSize, maxY,
			minZ, minZ + tileSize);
		completeOctreeNode(node->m_children[4], depth, level, tileSize,
			minX, minX + tileSize,
			minY, minY + tileSize,
			minZ + tileSize, maxZ);
		completeOctreeNode(node->m_children[5], depth, level, tileSize,
			minX + tileSize, maxX,
			minY, minY + tileSize,
			minZ + tileSize, maxZ);
		completeOctreeNode(node->m_children[6], depth, level, tileSize,
			minX, minX + tileSize,
			minY + tileSize, maxY,
			minZ + tileSize, maxZ);
		completeOctreeNode(node->m_children[7], depth, level, tileSize,
			minX + tileSize, maxX,
			minY + tileSize, maxY,
			minZ + tileSize, maxZ);

		node->m_children[0]->m_brother = node->m_children[1];
		node->m_children[1]->m_brother = node->m_children[2];
		node->m_children[2]->m_brother = node->m_children[3];
		node->m_children[3]->m_brother = node->m_children[4];
		node->m_children[4]->m_brother = node->m_children[5];
		node->m_children[5]->m_brother = node->m_children[6];
		node->m_children[6]->m_brother = node->m_children[7];
	}

	//子节点的父节点指针指向相应的父节点
	for (int i = 0; i < 8; ++i)
	{
		if (node->m_children[i])
		{
			node->m_children[i]->m_parent = node;
		}
	}
}

void KLCreateOctree::newTileNode(KLOctreeNode *&node, int depth, int level,
	int minX, int maxX,
	int minY, int maxY,
	int minZ, int maxZ)
{
	node = new KLOctreeNode();

	if (depth != 0)
	{
		node->m_emptyChild = false;  //非叶子
	}
	else
	{
		node->m_emptyChild = true;  //叶子
	}

	//由depth计算采样步长，叶子步长看作为1，往上走逐层加倍
	node->m_tileStep = (int)(pow(2.0, depth));
	node->m_level = level;

	node->m_minSample = minX; node->m_maxSample = maxX;
	node->m_minCMP = minY; node->m_maxCMP = maxY;
	node->m_minRecord = minZ; node->m_maxRecord = maxZ;

	node->m_diagnose = sqrt(static_cast<double>(
		(maxX - minX) * m_gridSizex * (maxX - minX) * m_gridSizex
		+ (maxY - minY) * m_gridSizey * (maxY - minY) * m_gridSizey
		+ (maxZ - minZ) * m_gridSizez * (maxZ - minZ) * m_gridSizez));

	node->m_center[0] = (minX + maxX) * m_gridSizex / 2.0;
	node->m_center[1] = (minY + maxY) * m_gridSizey / 2.0;
	node->m_center[2] = (minZ + maxZ) * m_gridSizez / 2.0;

	node->m_center[0] = (minX + maxX) * m_gridSizex / 2.0;
	node->m_center[1] = (minY + maxY) * m_gridSizey / 2.0;
	node->m_center[2] = (minZ + maxZ) * m_gridSizez / 2.0;

	double volBounds[6];
	int idx = 0, i, j, k;
	volBounds[0] = minX;	volBounds[1] = maxX;
	volBounds[2] = minY;	volBounds[3] = maxY;
	volBounds[4] = minZ;	volBounds[5] = maxZ;
	for (k = 0; k < 2; k++)
	{
		for (j = 0; j < 2; j++)
		{
			for (i = 0; i < 2; i++)
			{
				node->m_vertices[idx][2] = volBounds[4 + k];
				node->m_vertices[idx][1] = volBounds[2 + j];
				node->m_vertices[idx][0] = volBounds[i];
				idx++;
			}
		}
	}

	//采用向上取整：当按照tileStep除不尽的时候，采样点数目多了一个，所以使得
	//distance值比tileStep略小
	node->m_sampleNum = (int)ceil((maxX - minX)*1.0 / node->m_tileStep) + 1;
	node->m_CMPNum = (int)ceil((maxY - minY)*1.0 / node->m_tileStep) + 1;
	node->m_recordNum = (int)ceil((maxZ - minZ)*1.0 / node->m_tileStep) + 1;

	//判断是否为BLOCKSIZE的标准块
	if (node->m_sampleNum == BLOCKSIZE && node->m_CMPNum == BLOCKSIZE &&
		node->m_recordNum == BLOCKSIZE)
	{
		node->m_full = true;
		node->m_tileSize = BLOCKSIZE;
	}
	else
	{
		node->m_full = false;
		node->m_tileSize = -1;
	}

	//实际采样间距
	node->m_sampleDistance[0] = static_cast<float>(node->m_tileStep * m_gridSizex);
	node->m_sampleDistance[1] = static_cast<float>(node->m_tileStep * m_gridSizey);
	node->m_sampleDistance[2] = static_cast<float>(node->m_tileStep * m_gridSizez);
}

void KLCreateOctree::setTileInfo()
{
	//找到最左叶子
	KLOctreeNode* node = m_rootNode;
	while (node->m_children[0])
		node = node->m_children[0];
	m_zuoYezi = node;


	/*!采用自根向叶子的层序遍历方法*/

	std::list<KLOctreeNode*> nodeList;
	nodeList.push_back(m_rootNode);
	node = nodeList.front();

	while (node)
	{
		//堂兄弟间建立连接
		if (!node->m_brother && node->m_parent && node->m_parent->m_brother)
			node->m_brother = node->m_parent->m_brother->m_children[0];

		//确定节点的位置
		for (int i = 0; i < 8; i++)
		{
			if (node->m_children[i])
			{
				nodeList.push_back(node->m_children[i]);
			}
		}

		nodeList.erase(nodeList.begin());
		if (!nodeList.empty())
			node = nodeList.front();
		else
			break;
	}

	//统计叶子节点数目
	m_numYezi = 0;
	node = m_zuoYezi;
	while (node)
	{
		++m_numYezi;
		node = node->m_brother;
	}
}

void KLCreateOctree::readOctreeFileTail()
{
	unsigned long long int fileSize = m_pFileIn->size();
	unsigned long long int tailSize = sizeof(unsigned long long int) * this->m_nodeNum;
	m_pFileIn->seek(fileSize - tailSize);
	unsigned long long int *locationV = new unsigned long long int[this->m_nodeNum];
	m_pFileIn->read((char*)locationV, tailSize);

	unsigned long long int* temLV = locationV;
	KLOctreeNode * nodeTemp = this->m_rootNode;
	while (nodeTemp)
	{
		KLOctreeNode* node = nodeTemp;
		while (node)
		{
			node->m_location = *temLV;
			++temLV;
			node = node->m_brother;
		}
		nodeTemp = nodeTemp->m_children[0];
	}
	delete[]locationV;
	m_pFileIn->close();
}

KLOctreeNode* KLCreateOctree::getRootNode()
{
	return this->m_rootNode;
}

void KLCreateOctree::Delete(KLOctreeNode* node)
{
	if (node == NULL)
		return;

	for (int i = 0; i < 8; ++i)
	{
		Delete(node->m_children[i]);
	}

	delete node;
}

END_KLDISPLAY3D_NAMESPACE

