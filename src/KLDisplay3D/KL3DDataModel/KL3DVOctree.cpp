#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <QtCore/QTime>

#include "QtCore/qfile.h"
#include "QtCore/qdatastream.h"
#include "KLOctreeNode.h"
#include "KLSgyHeader.h"
//#include "KLData/KLCommon/KLByteOrder.h"
//#include "KLData/KLCommon/KLDataConvertFloat.h"
//#include "KLData/KLCommon/KLSystemByteOrder.h"
//#include "KLData/KLGridTable/KLGridData3D.h"
#include "KL3DVOctree.h"
namespace KLSeis{
namespace KLDisplay3D{

using namespace std;
using std::vector;

//------------------------
KL3DVOctree::KL3DVOctree()
{
	m_nodeNum = 0;
	flag_limitx = true;
	flag_limity = true;
	flag_limitz = true;
}

//---------------------------------
KL3DVOctree::~KL3DVOctree()
{
	Delete( m_rootNode );

	m_rootNode = NULL;
	m_zuoYezi = NULL;
	m_sourceFileName = "";
	m_octreeFileName = "";
}

//---------------------------------------------
void KL3DVOctree::Delete( KLOctreeNode* node )
{
	if( node == NULL )
		return;

	for ( int i = 0; i < 8; ++i )
	{
		Delete( node->m_children[i] );
	}

	delete node;
}

//---------------------------
void KL3DVOctree::computeLimit()
{
	/*!
	\执行此函数的前提：m_octreeSize已知
	*/

	int temp, size;
	
	size = m_octreeSize;
	temp = m_maxX - m_minX;
	while ( size > temp )
		size /= 2;

	m_oldSample = size;
	m_limitSample = size - ( ( BLOCKSIZE-1 ) - ( m_maxX - size ) % ( BLOCKSIZE-1 ) );

	size = m_octreeSize;
	temp = m_maxY - m_minY;
	while ( size > temp )
		size /= 2;

	m_oldCMP = size;
	m_limitCMP = size - ( ( BLOCKSIZE-1 ) - ( m_maxY - size ) % ( BLOCKSIZE-1 ) );

	size = m_octreeSize;
	temp = m_maxZ - m_minZ;
	while ( size > temp )
		size /= 2;

	m_oldRecord = size;
	m_limitRecord = size - ( ( BLOCKSIZE-1 ) - ( m_maxZ - size ) % ( BLOCKSIZE-1 ) );
}

//-----------------------------
void KL3DVOctree::createOctree()
{
	//计算八叉树的尺寸时，注意采样点数目和步长的关系，差1
	int size    = BLOCKSIZE - 1;
	int maxSize = (m_maxX-m_minX) > (m_maxY-m_minY) ? (m_maxX-m_minX) : (m_maxY-m_minY);
	maxSize = maxSize > (m_maxZ-m_minZ) ? maxSize : (m_maxZ-m_minZ);

	while ( size < maxSize )
		size *= 2;

	m_octreeSize = size;
	m_depth = (int)(log(m_octreeSize/(BLOCKSIZE-1)*1.0) / log(2.0));

	computeLimit();

	completeOctreeNode( m_rootNode, m_depth, 0, m_octreeSize,
	m_minX, m_maxX, 
	m_minY, m_maxY,
	m_minZ, m_maxZ );

	setTileInfo();


}

void KL3DVOctree::initialEntropy(string path)
{
	fstream fs(path, ios::in | ios::binary);
	if (fs.is_open())
	{
		list<KLOctreeNode*> nodeList, tmpList;
		tmpList.push_back(m_rootNode);
		double *h = new double[1];
		for(int loop=0; loop<=m_depth; ++loop)
		{
			nodeList.swap(tmpList);
			tmpList.clear();
			list<KLOctreeNode*>::iterator itr = nodeList.begin();
			for (; itr!=nodeList.end(); ++itr)
			{
				KLOctreeNode *tmpNode = *itr;
				/*将子节点加入tmpList*/
				for (int i=0; i<8; ++i)
				{
					if (tmpNode->m_children[i])
					{
						tmpList.push_back(tmpNode->m_children[i]);
					}
				}
				fs.seekg(tmpNode->m_entropyLocation*sizeof(double), ios::beg);
				bool b = fs.is_open();
				fs.read((char*)h, sizeof(double));
				//cout<<h[0]<<" "<<endl;
				tmpNode->m_entropy = h[0];
			}

		}

		delete []h;
	}

	fs.close();
}

//------------------------------------------------------
void KL3DVOctree::recoverOctree( int depth, int octreeSize,
	int minX, int maxX,
	int minY, int maxY,
	int minZ, int maxZ )
{
	m_octreeSize = octreeSize;
	m_depth = depth;

	m_minX = minX; m_minY = minY; m_minZ = minZ;
	m_maxX = maxX; m_maxY = maxY; m_maxZ = maxZ;

	computeLimit();

	completeOctreeNode( m_rootNode, depth, 0, octreeSize, minX, maxX, minY, maxY, minZ, maxZ );
	setTileInfo();
}

//--------------------------------------------------------
void KL3DVOctree::newTileNode( KLOctreeNode *&node, int depth, int level,
								     int minX, int maxX,
									 int minY, int maxY,
									 int minZ, int maxZ, int _idx )
{

	node = new KLOctreeNode();
	//设置节点的索引
	node->m_iNodeIdx = _idx;
	//设置节点对应体块的熵在文件中的位置
	node->m_entropyLocation = (node->m_iNodeIdx) * sizeof(double);


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
	node->m_sampleNum = (int)floor((maxX - minX)*1.0 / node->m_tileStep) + 1;
	node->m_CMPNum = (int)floor((maxY - minY)*1.0 / node->m_tileStep) + 1;
	node->m_recordNum = (int)floor((maxZ - minZ)*1.0 / node->m_tileStep) + 1;

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

//-------------------------------------------------------------------
void KL3DVOctree::completeOctreeNode( KLOctreeNode *&node, int depth,
			int level, int tileSize,
			int minX, int maxX,
			int minY, int maxY,
			int minZ, int maxZ )
{
	//depth控制递归次数
	if (depth < 0)
		return;

	//创建八叉树节点并计数
	newTileNode(node, depth, level, minX, maxX, minY, maxY, minZ, maxZ, m_nodeNum);
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

//--------------------------------------
void KL3DVOctree::setTileInfo()
{
	//找到最左叶子
	KLOctreeNode* node = m_rootNode;
	while ( node->m_children[0] )
		node = node->m_children[0];
	m_zuoYezi = node;


	/*!采用自根向叶子的层序遍历方法*/

	std::list<KLOctreeNode*> nodeList;	
	nodeList.push_back( m_rootNode );
	node = nodeList.front();
//	node->m_wei = new char('0');

	while ( node )
	{
		//堂兄弟间建立连接
		if ( !node->m_brother && node->m_parent && node->m_parent->m_brother )
			node->m_brother = node->m_parent->m_brother->m_children[0];


		//确定节点的位置
		for (int i = 0; i < 8; i ++)
		{
			if (node->m_children[i])
			{
				/*node->m_children[i]->m_wei = new char[node->m_level+2];

				char *up, *down;
				up = node->m_wei;
				down = node->m_children[i]->m_wei;
				for ( int j = 0; j < node->m_level+1; j ++ )
				{
				*down = *up;
				down++;
				up++;
				}
				*down=48+i;*/

				nodeList.push_back( node->m_children[i] );
			}
		}

		nodeList.erase( nodeList.begin() );
		if (!nodeList.empty())
			node = nodeList.front();
		else 
			break ;
	}

	//统计叶子节点数目
	m_numYezi = 0;
	node = m_zuoYezi;
	while ( node )
	{
		++ m_numYezi;
		node = node->m_brother;
	}
}

//--------------------------------
void KL3DVOctree::dataPreprocessing()
{
	//规则表单预处理
	//processGrid();
	//sgy数据预处理
	processSgy();
}

//-------------------------------------
void KL3DVOctree::processSgy()
{
	getSgyHeaderInfo();
	createOctree();

	/*!针对sgy数据的预处理*/

	cout<<"预处理开始..."<<std::endl;
	QTime timer;
	timer.start();

	/*只读方式打开sgy数据文件，in类似文件里的指针，用于读取文件里的数据---begin*/
	QFile fileIn( m_sourceFileName );
	QDataStream in( &fileIn );
	in.setFloatingPointPrecision( QDataStream::SinglePrecision );
	in.setByteOrder( QDataStream::LittleEndian );
	if ( !fileIn.open(QIODevice::ReadOnly) )
	{
		cout<<"can't open file : "<<m_sourceFileName.toStdString()<<std::endl;
		return;
	}
	/*正确打开sgy数据文件---end*/

	/*以只写方式建立文件，用于存储处理的sgy数据---begin*/
	QFile fileOut( m_octreeFileName );
	fileOut.close();
	QDataStream out(&fileOut);
	out.setFloatingPointPrecision(QDataStream::SinglePrecision);
	out.setByteOrder(QDataStream::LittleEndian);
	if( !fileOut.open(QIODevice::WriteOnly) )
	{
		cout<<"can't create file : "<<m_octreeFileName.toStdString()<<std::endl;
		return;
	}
	/*以只写方式建立文件，用于存储处理的sgy数据---end*/

	//判断八叉树的建立过程是否出错
	if ( m_zuoYezi == NULL )
	{
		cout<<"建树时出错."<<std::endl;
		return;
	}

	/*!
	\在八叉树结构的数据文件开头写入恢复八叉树结构所需要的属性
	\包括：int depth--八叉树深度 int octreeSize--八叉树尺寸
	\	   int minX, int maxX,
	\	   int minY, int maxY,
	\	   int minZ, int maxZ--八叉树三方向上范围
	*/
	//此时location值从10开始
	out<<m_depth<<m_octreeSize<<m_minX<<m_maxX<<m_minY<<m_maxY<<m_minZ<<m_maxZ<<m_minValue<<m_maxValue;

	/*********************开始算法******************************/

	//一个trace的长度，单位B，包含道头。在sample方向就是trace
	int lengthOfTrace = 240 + 4 * m_numx;
	//一个record的长度，单位B。也就是在一个record中，所有trace的长度之和
	long long int lengthOfRecord = m_numy * lengthOfTrace;

	/*盛放三维数组，包括64-8-1出来的标准块和后续的非标准块
	**由于存在非标准块所以不用vector< float(*)[dian1][dian1] > gradVector;*/
	vector< vector<vector<vector<float>>> > gradVector;

	/*location：块在新文件里的位置，单位4B
	**capacity：一个块的大小，单位4B，辅助location的生成*/
	unsigned long long int location = OCTREEHEADER / 4, capacity=0;

	/*dian1：一个标准块在一个方向上的采样点数目
	**dian8：八个标准块组成的一个大块，在一个方向上的采样点数目
	**dian64:64个标准块组成的一个大块，在一个方向上的采样点数目*/
	const int dian64 = 4*BLOCKSIZE - 3;
	const int dian8  = 2*BLOCKSIZE - 1;
	const int dian1  = BLOCKSIZE;

	/*kuai1：一个标准块的采样点数目
	**kuai8：八个标准块组成的一个大块，拥有的采样点数目
	**kuia64:64个标准块组成的一个大块，拥有的采样点数目*/
	const int kuai64 = dian64*dian64*dian64;
	const int kuai8  = dian8*dian8*dian8;
	const int kuai1  = dian1*dian1*dian1;

	/*!
	\brief 开辟64个标准块组成的一个大块的存储空间,每个方向509个点（503.0526M）
	\note  开辟大空间时，vector速度太慢。使用new-delete组合比较好
	*/
	/*vector< vector< vector<float> > >
		blockValue64( dian64, vector< vector<float> >(dian64, vector<float>(dian64) ) );*/
	float (*blockValue64)[dian64][dian64] = new float[ dian64 ][ dian64 ][ dian64 ];

	//开辟8个标准块组成的一个大块的存储空间,每个方向255个点（63.2529M）
	/*vector< vector< vector<float> > >
		blockValue8( dian8, vector< vector<float> >(dian64, vector<float>(dian64) ) );*/
	float (*blockValue8)[ dian8 ][ dian8 ] = new float[ dian8 ][ dian8 ][ dian8 ];

	/*开辟一个标准块存储空间，在64-8-1模式中用于写入文件时用
	**BLOCKSIZE为64时，一个标准块1M
	**BLOCKSIZE为128时，一个标准块8M*/
	float *blockValue = new float[ kuai1 ];


	//读取64块时cmp方向的指针跳跃
	unsigned long long index1 = ( m_numx - (4*BLOCKSIZE - 3) )*4 + 240;
	//读取64块时record方向的指针跳跃
	unsigned long long index2 = ( m_numy - (4*BLOCKSIZE - 3) )*lengthOfTrace + index1;

	KLOctreeNode *yeNode = m_zuoYezi;
	/*=-=-按照64-8-1模式处理叶子节点，并且把1处的节点保存在gradvector中备用---begin*/

	/*!
	\brief 进入64-8-1模式的条件
	\note 当xyz每个方向上都存在不少于4块时，才能进入
	*/
	//每个方向上存在多少个4块
	int countx, county,countz;
	countx = m_oldSample / (dian64-1);//完全八叉树对应的4块
	countx += (int)( (m_maxX-m_limitSample)*1.0/(dian64-1) );//扩展部分对应的4块
	county = m_oldCMP / (dian64-1);
	county += (int)( (m_maxY-m_limitCMP)*1.0/(dian64-1) );
	countz = m_oldRecord / (dian64-1);
	countz += (int)( (m_maxZ-m_limitRecord)*1.0/(dian64-1) );
	//64-8-1模式循环次数
	int cishu = countx*county*countz;

	//当存在64块时就进行处理
	while( cishu != 0 && yeNode )
	{
		//若此64块对应的爷爷节点不是同一个，则跳出循环
		KLOctreeNode *temp = yeNode;
		for ( int i = 0; i < 63; ++i )
		{
			temp = temp->m_brother;
		}//此时temp指向这64个节点的最后一个节点
		if ( yeNode->m_parent->m_parent != temp->m_parent->m_parent )
		{
			break;
		}

		//确定三个方向的范围,写入时使用
		int minRecord = yeNode->m_minRecord;
		int minCMP	  = yeNode->m_minCMP;
		int minSample = yeNode->m_minSample;

		//定位64小块的开始位置
		long long index =(yeNode->m_minRecord - m_minZ) * lengthOfRecord
					    +(yeNode->m_minCMP    - m_minY) * lengthOfTrace
					    +(yeNode->m_minSample - m_minX) * 4 + 3840;

		long long int x = index - index1;
		for ( int j = 0; j < dian64; ++j )
		{
			for ( int i = 0; i < dian64; ++i )
			{
				x += index1;
				fileIn.seek( x );//务必使第一次的x值为index
				fileIn.read( (char*)(&blockValue64[j][i][0]), dian64*4 );
				x += dian64*4;
			}
			x += index2;
			x -= index1;
		}//64小块对应的一大块数据全部读进了blockValue64中

		for (int k = 0; k < dian8; k ++)
		{
			for ( int j = 0; j < dian8; j ++ )
			{
				for ( int i = 0; i < dian8; i ++ )
				{
					blockValue8[k][j][i] = blockValue64[ k*2 ][ j*2 ][ i*2 ];
				}
			}
		}//8块父节点对应的一大块全部放在了blockValue8中

		//开辟1个完整块的存储空间（1M\8M）
		//不使用float (*blockValue1)[ dian1 ][ dian1 ] = new float[ dian1 ][ dian1 ][ dian1 ];
		//因为要push到gradVector中
		vector<vector<vector<float>>> blockValue1(dian1,vector<vector<float>>(dian1,vector<float>(dian1)));

		for (int k = 0; k < dian1; k ++)
		{
			for ( int j = 0; j < dian1; j ++ )
			{
				for ( int i = 0; i < dian1; i ++ )
				{
					blockValue1[k][j][i] = blockValue8[ k*2 ][ j*2 ][ i*2 ];
				}
			}
		}//一块父节点对应的数据全部放在了blockValue1中

		//把上面得到的这块父节点保存下来
		gradVector.push_back( blockValue1 );

		//写64块
		KLOctreeNode *tempNode = yeNode;
		for ( int i = 0; i < 64; ++i )
		{
			tempNode->m_location = location;
			capacity = tempNode->m_sampleNum * tempNode->m_CMPNum * tempNode->m_recordNum;
			location += capacity;

			int r = tempNode->m_minRecord - minRecord;
			int c = tempNode->m_minCMP    - minCMP;
			int s = tempNode->m_minSample - minSample;

			float* temp_blockValue = blockValue;
			for (int k = 0; k < dian1; ++k )
			{
				for ( int j = 0; j < dian1; ++j )
				{
					for ( int i = 0; i < dian1; ++i )
					{
						*temp_blockValue = blockValue64[ r+k ][ c+j ][ s+i ];
						++temp_blockValue;
					}
				}
			}
			//写入文件
			fileOut.write( (char*)blockValue, 4*kuai1 );

			tempNode = tempNode->m_brother;
		}//64块写完

		//写8块
		tempNode = yeNode->m_parent;
		for ( int i = 0; i < 8; ++i )
		{
			tempNode->m_location = location;
			capacity = tempNode->m_sampleNum * tempNode->m_CMPNum * tempNode->m_recordNum;
			location += capacity;

			int r = static_cast<int>( ( tempNode->m_minRecord - minRecord )/2.0 );
			int c = static_cast<int>( ( tempNode->m_minCMP    - minCMP )/2.0 );
			int s = static_cast<int>( ( tempNode->m_minSample - minSample )/2.0 );

			float* temp_blockValue = blockValue;
			for (int k = 0; k < dian1; k ++)
			{
				for ( int j = 0; j < dian1; j ++ )
				{
					for ( int i = 0; i < dian1; i ++ )
					{
						*temp_blockValue = blockValue8[ r+k ][ c+j ][ s+i ];
						++temp_blockValue;
					}
				}
			}
			fileOut.write( (char*)blockValue, 4*kuai1 );

			tempNode = tempNode->m_brother;
		}//8块写完

		//写1块
		tempNode = yeNode->m_parent->m_parent;
		tempNode->m_location = location;
		capacity = tempNode->m_sampleNum * tempNode->m_CMPNum * tempNode->m_recordNum;
		location += capacity;

		float* temp_blockValue = blockValue;
		for (int k = 0; k < dian1; k ++)
		{
			for ( int j = 0; j < dian1; j ++ )
			{
				for ( int i = 0; i < dian1; i ++ )
				{
					*temp_blockValue = blockValue1[ k ][ j ][ i ];
					++temp_blockValue;
				}
			}
		}
		fileOut.write( (char*)blockValue, 4*kuai1 );		

		//如果有，继续处理后续的64块
		for ( int i = 0; i < 64; ++i )
		{
			yeNode = yeNode->m_brother;
		}

		--cishu;
	}//64-8-1模式处理完毕

	//释放开辟的空间
	delete []blockValue64;
	delete []blockValue8;
	delete []blockValue;


	/*=-=-此时一种情况是跳出循环后还有叶子，已经不适应64-8-1模式；
	=-=-=-另一种情况是没有进入循环。
	=-=-=-这个时候叶子节点进行n-1模式，其中n取值8\4\2\1
	=-=-=-同时父节点保存在parentVector中备用---begin*/

	//暂存一个标准块，用完释放
	float *blockValuek = new float[ kuai1 ];

	//存放叶子节点的父亲节点，由这些父亲节点得到爷爷节点，放到gradVector中
	vector< vector<vector<vector<float>>> > parentVector;
	//指向parentVector中的第一个节点
	KLOctreeNode* parentFirstNode = yeNode->m_parent;

	while( yeNode )
	{
		//先找出同属于一个父节点的n个叶子节点
		KLOctreeNode *node = yeNode;
		int num=0; //计数叶子节点
		while ( node && node->m_parent == yeNode->m_parent )
		{
			++num;
			node = node->m_brother;
		}
		node = yeNode;
		for ( int i = 0; i < num-1; ++i )
		{
			node = node->m_brother;
		}

		//读入这num个块的整块
		int minSample = yeNode->m_minSample;
		int minCMP	  = yeNode->m_minCMP;
		int minRecord = yeNode->m_minRecord;
		int maxSample = node->m_maxSample;
		int maxCMP	  = node->m_maxCMP;
		int maxRecord = node->m_maxRecord;
		//开辟包含这num块的整个大块的空间，用完释放
		int m = maxRecord-minRecord+1;
		int n = maxCMP-minCMP+1;
		int p = maxSample-minSample+1;
		vector<vector<vector<float>>> blockValueN(m,vector<vector<float>>(n,vector<float>(p)));
		//float (*blockValueN)[dian64][dian64] = new float[ m ][ n ][ p ];

		//定位
		unsigned long long index  = ( minRecord - m_minZ) * lengthOfRecord
			+ ( minCMP    - m_minY   ) * lengthOfTrace
			+ ( minSample - m_minX) * 4 + 3840;

		//读取n块时cmp方向的指针跳跃
		index1 = ( m_numx - p )*4 + 240;
		//读取n块时record方向的指针跳跃
		index2 = ( m_numy - n )*lengthOfTrace + index1;
		unsigned long long int x = index - index1;//index1--cmp方向的跳跃
		//从sgy文件读取num块叶子
		for ( int k = 0; k < m; k ++ )
		{//record
			for ( int j = 0; j < n; j ++ )
			{//cmp
				x += index1;
				fileIn.seek( x );//务必使第一次的x值为index
				fileIn.read( (char*)(&blockValueN[k][j][0]), p*4 );
				x += p*4;
			}
			x += index2;
			x -= index1;
		}//num块读完了

		//从blockValueN中得到这num小块并写入文件
		KLOctreeNode *tempNode = yeNode;
		for ( int i = 0; i < num; i ++ )
		{
			tempNode->m_location = location;
			capacity = tempNode->m_sampleNum * tempNode->m_CMPNum * tempNode->m_recordNum;
			location += capacity;

			int r = tempNode->m_minRecord - minRecord;
			int c = tempNode->m_minCMP    - minCMP;
			int s = tempNode->m_minSample - minSample;

			float* temp_blockValue = blockValuek;
			for (int k = 0; k < dian1; k ++)
			{
				for ( int j = 0; j < dian1; j ++ )
				{
					for ( int i = 0; i < dian1; i ++ )
					{
						*temp_blockValue = blockValueN[ r+k ][ c+j ][ s+i ];
						++temp_blockValue;
					}
				}
			}
			//写入文件
			fileOut.write( (char*)blockValuek, 4*kuai1 );
			tempNode = tempNode->m_brother;
		}//num块写完

		//开辟父节点块的存储空间，由这些父节点块得到爷爷节点时，才能释放
		/*从blockValueN中得到父节点块，并写入文件----begin*/
		//指向yeNode的parent
		KLOctreeNode* parent = yeNode->m_parent;
		int m1 = parent->m_recordNum;
		int n1 = parent->m_CMPNum;
		int p1 = parent->m_sampleNum;
		//存放父节点块，从blockValueN中得到数据
		vector<vector<vector<float>>> blockValue(m1,vector<vector<float>>(n1,vector<float>(p1)));

		if (fabs(parent->m_tileStep - parent->m_sampleDistance[0]) <= 0.0001
			&& fabs(parent->m_tileStep - parent->m_sampleDistance[1]) <= 0.0001
			&& fabs(parent->m_tileStep - parent->m_sampleDistance[2]) <= 0.0001)
		{//不需要插值
			//从blockValueN中获得父节点块
			for (int k = 0; k < m1; k ++)
			{
				for ( int j = 0; j < n1; j ++ )
				{
					for ( int i = 0; i < p1; i ++ )
					{
						blockValue[k][j][i] = blockValueN[ k*2 ][ j*2 ][ i*2 ];
					}
				}
			}//获得父节点完毕
		}
		else
		{
			double wSample, wCMP, wRecord;
			float  A, B, C, D, E, F, G, H, value1;

			//插值法
			for (int i = 0; i < parent->m_recordNum; ++i)
			{
				float temp1 = i * parent->m_sampleDistance[2];
				//record方向定位
				int l_record = floor( temp1 / yeNode->m_tileStep );
				int r_record = ceil( temp1 / yeNode->m_tileStep );
				wRecord = ( temp1 - l_record*yeNode->m_tileStep )/yeNode->m_tileStep;

				for (int j = 0; j < parent->m_CMPNum; ++j)
				{
					temp1 = j * parent->m_sampleDistance[1];
					//cmp方向定位
					int l_cmp = floor( temp1 / yeNode->m_tileStep );
					int r_cmp = ceil( temp1 / yeNode->m_tileStep );
					wCMP = ( temp1 - l_cmp*yeNode->m_tileStep )/yeNode->m_tileStep;

					for (int k = 0; k < parent->m_sampleNum; ++k)
					{
						temp1 = k * parent->m_sampleDistance[0];
						//sample方向定位
						int l_sample = floor( temp1 / yeNode->m_tileStep );
						int r_sample = ceil( temp1 / yeNode->m_tileStep );
						wSample = ( temp1 - l_sample*yeNode->m_tileStep )/yeNode->m_tileStep;
						//A-H这8个值是:需要插值出的点的周围最近的8个点的值
						A = blockValueN[l_record][l_cmp][l_sample];
						B = blockValueN[l_record][l_cmp][r_sample];
						C = blockValueN[l_record][r_cmp][l_sample];
						D = blockValueN[l_record][r_cmp][r_sample];
						E = blockValueN[r_record][l_cmp][l_sample];
						F = blockValueN[r_record][l_cmp][r_sample];
						G = blockValueN[r_record][r_cmp][l_sample];
						H = blockValueN[r_record][r_cmp][r_sample];

						value1 = 
							(1.0 - wRecord) * (1.0 - wCMP) * (1.0 - wSample) * A +
							(1.0 - wRecord) * (1.0 - wCMP) *        wSample  * B +
							(1.0 - wRecord) *        wCMP  * (1.0 - wSample) * C +
							(1.0 - wRecord) *        wCMP  *        wSample  * D +
							wRecord  * (1.0 - wCMP) * (1.0 - wSample) * E +
							wRecord  * (1.0 - wCMP) *        wSample  * F +
							wRecord  * (      wCMP) * (1.0 - wSample) * G +
							wRecord  * (      wCMP) *        wSample  * H;

						blockValue[i][j][k] = value1;
					}
				}
			} //父节点块插值结束

		}//end else

		//暂存此父节点块，用于获得爷爷节点
		parentVector.push_back( blockValue );
		parent->m_location = location;
		capacity = parent->m_sampleNum * parent->m_CMPNum * parent->m_recordNum;
		location += capacity;

		float* xie = new float[m1*n1*p1];
		float *tr = xie;
		for ( int i = 0; i < m1; ++i )
		{
			for ( int j = 0; j < n1; ++j )
			{
				for ( int k = 0; k < p1; ++k)
				{
					*tr = blockValue[i][j][k];
					++tr;
				}
				//fileOut.write( (char*)(&(blockValue[i][j][0])), p*4 );
				//问题：通过vector写入的时候老是漏写入数据，不知道原因，待查。
			}
		}
		fileOut.write( (char*)xie, m1*n1*p1*4 );
		delete []xie;
		//从blockValueN中得到父节点块，并写入文件----end

		//释放内存
		vector<vector<vector<float> > > temp;
		blockValueN.swap( temp );

		//继续处理下一个num块
		for ( int i = 0; i < num; ++i )
		{
			yeNode = yeNode->m_brother;
		}

	}//end while( yeNode )

	//释放暂存一块的空间
	delete []blockValuek;
	/*=-=-跳出n-1模式，至此叶子节点全部处理完毕---end*/

	/*=-=-经过n-1模式得到的parent层的所有的块都在parentVector中，要从中得到grad层块
		=-=-=-并放入gradVector中----begin*/
	KLOctreeNode* node = parentFirstNode;
	while ( node )
	{
		/*找到属于一个节点的n个子节点进行处理----begin*/

		//计数节点个数
		int num = 0;
		KLOctreeNode* temp = node;
		while ( temp && node->m_parent == temp->m_parent )
		{
			++num;
			temp = temp->m_brother;
		}	

		//指向node的parent
		KLOctreeNode* parent = node->m_parent;
		//开辟node->m_parent的存储空间
		int m = parent->m_recordNum;
		int n = parent->m_CMPNum;
		int p = parent->m_sampleNum;
		//存放node->m_parent块
		vector<vector<vector<float>>> blockValue(m,vector<vector<float>>(n,vector<float>(p)));
		
		/*加上这个条件之后，在8块生成一块的情况下，会导致本来不用插值却进入插值代码，但实际上并
		\没有插值。待完善*/
		bool flag01 = ( m == BLOCKSIZE && n == BLOCKSIZE && p == BLOCKSIZE );

		if (fabs(parent->m_tileStep - parent->m_sampleDistance[0]) <= 0.0001
				&& fabs(parent->m_tileStep - parent->m_sampleDistance[1]) <= 0.0001
				&& fabs(parent->m_tileStep - parent->m_sampleDistance[2]) <= 0.0001
				&& flag01 )
		{
			//此时parent节点不需要插值，则生成这个parent节点的node节点应该是8个标准块
			if ( num != 8 )
			{
				//此时先输出错误，若不是8，再后续处理
				cout<<"num != 8，由parentVector生成父节点出错."<<std::endl;
			}

			//num是8才能往下进行

			//不同的块，s0,c0,r0有不同的取值
			int s0,c0,r0;
			//暂存数据
			vector<float> tempvalue1;
			for ( int t = 0; t < num; ++t )
			{
				switch(t)
				{
				case 0: { r0 = 0; c0 = 0; s0 = 0; }break;
				case 1: { r0 = 0; c0 = 0; s0 = 1; }break;
				case 2: { r0 = 0; c0 = 1; s0 = 0; }break;
				case 3: { r0 = 0; c0 = 1; s0 = 1; }break;
				case 4: { r0 = 1; c0 = 0; s0 = 0; }break;
				case 5: { r0 = 1; c0 = 0; s0 = 1; }break;
				case 6: { r0 = 1; c0 = 1; s0 = 0; }break;
				case 7: { r0 = 1; c0 = 1; s0 = 1; }break;
				}
				
				//把一块中属于父节点的数据暂存到tempvalue1中
				for ( int i = r0; i < dian1; i += 2 )
				{
					for ( int j = c0; j < dian1; j += 2 )
					{
						for ( int k = s0; k < dian1; k += 2 )
						{
							tempvalue1.push_back( parentVector[t][i][j][k] );
						}
					}
				}//把一块中属于父节点的数据暂存到tempvalue1中，完毕

				//暂存在tempvalue1中的数据放到blockValue中
				vector<float>::iterator iter = tempvalue1.begin();
				for ( int i = 0; i < dian1/2; ++i )
				{
					for ( int j = 0; j < dian1/2; ++j )
					{
						for ( int k = 0; k < dian1/2; ++k )
						{
							//不同的块，数据放的位置不同
							switch (t)
							{
							case 0 :
								blockValue[i][j][k] = *iter; break;
							case 1 :
								blockValue[i][j][k+dian1/2] = *iter; break;
							case 2 :
								blockValue[i][j+dian1/2][k] = *iter; break;
							case 3 :
								blockValue[i][j+dian1/2][k+dian1/2] = *iter; break;
							case 4 :
								blockValue[i+dian1/2][j][k] = *iter; break;
							case 5 :
								blockValue[i+dian1/2][j][k+dian1/2] = *iter; break;
							case 6 :
								blockValue[i+dian1/2][j+dian1/2][k] = *iter; break;
							case 7 :
								blockValue[i+dian1/2][j+dian1/2][k+dian1/2] = *iter; break;
							}

							++iter;
						}
					}
				}//暂存在tempvalue1中的数据放到blockValue中，完毕

				//清空tempvalue1
				tempvalue1.erase( tempvalue1.begin(), tempvalue1.end() );
			}//由这num块得到了父节点的数据并放入到blockValue中，完毕
		}
		else
		{//需要插值
			double wSample, wCMP, wRecord;
				float  A, B, C, D, E, F, G, H, value1;

				//插值法
				for (int i = 0; i < parent->m_recordNum; ++i)
				{
					int l_record, r_record;
					int sign_record;//标识在哪个块上,0：在0块上，1：在1块上
					float temp1 = i * parent->m_sampleDistance[2];

					/*确定sign_record、l_record、r_record这三个值--begin*/
					if ( temp1 <= ( node->m_maxRecord - node->m_minRecord) )
					{//在node块上
						l_record = floor( temp1 / node->m_sampleDistance[2] );
						r_record = ceil( temp1 / node->m_sampleDistance[2] );
						wRecord = ( temp1 - l_record*node->m_sampleDistance[2] )/node->m_sampleDistance[2];
						sign_record = 0;
					}
					else
					{//在node的下一个块上
						KLOctreeNode* temN;
						switch( num )
						{
						case 8: temN = node->m_brother->m_brother->m_brother->m_brother;break;
						case 4: temN = node->m_brother->m_brother;break;
						case 2: temN = node->m_brother;break;
						}
						
						temp1 -= ( temN->m_minRecord - node->m_minRecord);
						l_record = floor( temp1 / temN->m_sampleDistance[2] );
						r_record = ceil( temp1 / temN->m_sampleDistance[2] );
						wRecord = ( temp1 - l_record*temN->m_sampleDistance[2] )/temN->m_sampleDistance[2];
						sign_record = 1;
					}
					/*确定sign_record、l_record、r_record这三个值--end*/

					for (int j = 0; j < parent->m_CMPNum; ++j)
					{
						int l_cmp, r_cmp;
						int sign_cmp;//标识在哪个块上,0：在0块上，1：在2块上
						temp1 = j * parent->m_sampleDistance[1];
						
						/*确定sign_cmp、l_cmp、r_cmp这三个值--begin*/
						if ( temp1 <= ( node->m_maxCMP - node->m_minCMP) )
						{//在node块上
							l_cmp = floor( temp1 / node->m_sampleDistance[1] );
							r_cmp = ceil( temp1 / node->m_sampleDistance[1] );
							wCMP = ( temp1 - l_cmp*node->m_sampleDistance[1] )/node->m_sampleDistance[1];
							sign_cmp = 0;
						} 
						else
						{//在node的下一个块上
							KLOctreeNode* temN;
							switch( num )
							{
							case 8: temN = node->m_brother->m_brother;break;
							case 4: {
										if (parent->m_children[0]&&parent->m_children[1]&&parent->m_children[2]&&
											parent->m_children[3])//parent->m_sampleNum > parent->m_recordNum
											temN = node->m_brother->m_brother;
										else
											temN = node->m_brother;
									}break;
							case 2: temN = node->m_brother;break;
							}

							temp1 -= ( temN->m_minCMP - node->m_minCMP);
							l_cmp = floor( temp1 / temN->m_sampleDistance[1] );
							r_cmp = ceil( temp1 / temN->m_sampleDistance[1] );
							wCMP = ( temp1 - l_cmp*temN->m_sampleDistance[1] )/temN->m_sampleDistance[1];
							sign_cmp = 1;
						}					
						/*确定sign_cmp、l_cmp、r_cmp这三个值--end*/

						for (int k = 0; k < parent->m_sampleNum; ++k)
						{
							int l_sample, r_sample;
							int sign_sample;//标识在哪个块上,0：在0块上，1：在4块上
							temp1 = k * parent->m_sampleDistance[0];
							
							/*确定sign_sample、l_sample、r_sample这三个值--begin*/
							if ( temp1 <= ( node->m_maxSample - node->m_minSample) )
							{//在node块上
								l_sample = floor( temp1 / node->m_sampleDistance[0] );
								r_sample = ceil( temp1 / node->m_sampleDistance[0] );
								wSample = ( temp1 - l_sample*node->m_sampleDistance[0] )/node->m_sampleDistance[0];
								sign_sample = 0;
							} 
							else
							{//在node的下一个块上
									KLOctreeNode* tempnode2 = node->m_brother;
									temp1 -= ( tempnode2->m_minSample - node->m_minSample);
									l_sample = floor( temp1 / tempnode2->m_sampleDistance[0] );
									r_sample = ceil( temp1 / tempnode2->m_sampleDistance[0] );
									wSample = ( temp1 - l_sample*tempnode2->m_sampleDistance[0] )/tempnode2->m_sampleDistance[0];
									sign_sample = 1;
							}					
							/*确定sign_sample、l_sample、r_sample这三个值--end*/

							//A-H这8个值是:需要插值出的点的周围最近的8个点的值
							int tt;
							if ( sign_record==0 && sign_cmp==0 && sign_sample==0 )
							{
								tt = 0;//在0块上
							}
							else if ( sign_record==1 && sign_cmp==0 && sign_sample==0 )
							{
								switch(num)
								{
								case 8 : tt = 4; break;
								case 4 : tt = 2; break;
								case 2 : tt = 1; break;
								}
							}
							else if ( sign_record==0 && sign_cmp==1 && sign_sample==0 )
							{
								switch(num)
								{
								case 8 : tt = 2; break;
								case 2 : tt = 1; break;
								case 4 : {
									if( parent->m_children[0]&&parent->m_children[1]&&parent->m_children[2]&&
										parent->m_children[3] )
												tt = 2;
											else
												tt = 1;
										 }break;
								}
							}
							else if ( sign_record==1 && sign_cmp==1 && sign_sample==0 )
							{
								switch(num)
								{
								case 8 : tt = 6; break;
								case 4 : tt = 3; break;
								}
							}
							else if ( sign_record==0 && sign_cmp==0 && sign_sample==1 )
							{
								tt = 1;//在4块上
							}
							else if ( sign_record==1 && sign_cmp==0 && sign_sample==1 )
							{
								switch(num)
								{
								case 8 : tt = 5; break;
								case 4 : tt = 3; break;
								}
							}
							else if ( sign_record==0 && sign_cmp==1 && sign_sample==1 )
							{
								tt = 3;//在6块上
							}
							else if ( sign_record==1 && sign_cmp==1 && sign_sample==1 )
							{
								tt = 7;//在7块上
							}

							A = parentVector[tt][l_record][l_cmp][l_sample];
							B = parentVector[tt][l_record][l_cmp][r_sample];
							C = parentVector[tt][l_record][r_cmp][l_sample];
							D = parentVector[tt][l_record][r_cmp][r_sample];
							E = parentVector[tt][r_record][l_cmp][l_sample];
							F = parentVector[tt][r_record][l_cmp][r_sample];
							G = parentVector[tt][r_record][r_cmp][l_sample];
							H = parentVector[tt][r_record][r_cmp][r_sample];
							
							value1 = 
									(1.0 - wRecord) * (1.0 - wCMP) * (1.0 - wSample) * A +
									(1.0 - wRecord) * (1.0 - wCMP) *        wSample  * B +
									(1.0 - wRecord) *        wCMP  * (1.0 - wSample) * C +
									(1.0 - wRecord) *        wCMP  *        wSample  * D +
									wRecord  * (1.0 - wCMP) * (1.0 - wSample) * E +
									wRecord  * (1.0 - wCMP) *        wSample  * F +
									wRecord  * (      wCMP) * (1.0 - wSample) * G +
									wRecord  * (      wCMP) *        wSample  * H;

							blockValue[i][j][k] = value1;
						}
					}
				} //父节点块插值结束
		}//不管node->parent块是否需要插值，已经从子节点中把数据放到blockValue中

		gradVector.push_back( blockValue );//把node->parent放入gradVector中
		//把node->parent块写入文件
		parent->m_location = location;
		capacity = parent->m_sampleNum * parent->m_CMPNum * parent->m_recordNum;
		location += capacity;

		float* xie = new float[m*n*p];
		float *tr = xie;
		for ( int i = 0; i < m; ++i )
		{
			for ( int j = 0; j < n; ++j )
			{
				for ( int k = 0; k < p; ++k)
				{
					*tr = blockValue[i][j][k];
					++tr;
				}
				//fileOut.write( (char*)(&(blockValue[i][j][0])), p*4 );
				//问题：通过vector写入的时候老是漏写入数据，不知道原因，待查。
			}
		}
		fileOut.write( (char*)xie, m*n*p*4 );
		delete []xie;

		//fileOut.write( (char*)(&(blockValue[0][0][0])), capacity );//一块写完

		/*写入文件并且放入gradVector中完毕----end*/

		//把parentVector中前num个块清除掉
		parentVector.erase( parentVector.begin(), parentVector.begin()+num );

		//继续后续处理
		for ( int i = 0; i < num; ++i )
		{
			node = node->m_brother;
		}
	}//endwhile，此时都放在gradVector中
	//释放内存
	vector<vector<vector<vector<float> > > > temp;
	parentVector.swap(temp);
	/*=-=-从parentVector中得到了父节点块并放入了gradVector中---end*/

	/*先不转移，因为数据量很大时，转移会导致内存分配问题，待完善*/
	//vector< vector<vector<vector<float>>> > childVector(gradVector);
	//gradVector.swap( vector< vector<vector<vector<float>>> >() );//gradVector不再使用了

	/*!全部需要的块都放在了childVector中，再从这个vector中一层一层
	\向上层获得块，直到根节点。此算法要求树的深度至少3层
	\所以childVector中一定有数据*/

	if ( gradVector.empty() )
	{
		cout<<"xinbing-error: gradVector is empty."<<std::endl;
		return;
		//gradVector为空，depth可能不到3层，或者生成gradVector时出错
	}

	/*gradVector不空才能往下进行*/

	/*如果gradVector中只有一块，说明这是根节点，那么就不用处理了，释放掉这个vector空间
	\这个算法就算结束了，分块后的数据全部写入外存，八叉树也已经建好*/

	//zuoyi指向gradVector中第一个节点
	KLOctreeNode* zuoyi = m_zuoYezi->m_parent->m_parent;
	//用来存放父亲层节点
	vector< vector<vector<vector<float> > > > pregradVector;

	while( gradVector.size() != 1 )
	{
		/*从gradVector中获得父节点块放入pregradVector中，并写入文件----begin*/

		KLOctreeNode* node = zuoyi;
		while ( node )
		{
			/*找到属于一个节点的n个子节点进行处理，把处理之后的这个节点保存并写入文件----begin*/

			//计数节点个数
			int num = 0;
			KLOctreeNode* temp = node;
			while ( temp && node->m_parent == temp->m_parent )
			{
				++num;
				temp = temp->m_brother;
			}

			//指向node的parent
			KLOctreeNode* parent = node->m_parent;
			//开辟node->m_parent的存储空间
			int m = parent->m_recordNum;
			int n = parent->m_CMPNum;
			int p = parent->m_sampleNum;
			//存放node->m_parent块
			vector<vector<vector<float> > > blockValue(m,vector<vector<float> >(n,vector<float>(p)));
			
			/*加上这个条件之后，在8块生成一块的情况下，会导致本来不用插值却进入插值代码，但实际上并
			\没有插值。待完善*/
			bool flag01 = ( m == BLOCKSIZE && n == BLOCKSIZE && p == BLOCKSIZE );

			if ( fabs(parent->m_tileStep - parent->m_sampleDistance[0]) <= 0.0001
				 && fabs(parent->m_tileStep - parent->m_sampleDistance[1]) <= 0.0001
				 && fabs(parent->m_tileStep - parent->m_sampleDistance[2]) <= 0.0001
				 && flag01 )
			{
				//此时parent节点不需要插值，则生成这个parent节点的node节点应该是8个标准块

				if ( num != 8 )
				{
					//此时先输出错误，若不是8，再后续处理
					cout<<"num != 8，由parentVector生成父节点出错."<<std::endl;
				}
				
				//num是8才能往下进行

				//不同的块，s0,c0,r0有不同的取值
				int s0,c0,r0;

				//暂存数据
				vector<float> tempvalue1;
				for ( int t = 0; t < num; ++t )
				{
					switch(t)
					{
					case 0: { r0 = 0; c0 = 0; s0 = 0; }break;
					case 1: { r0 = 0; c0 = 0; s0 = 1; }break;
					case 2: { r0 = 0; c0 = 1; s0 = 0; }break;
					case 3: { r0 = 0; c0 = 1; s0 = 1; }break;
					case 4: { r0 = 1; c0 = 0; s0 = 0; }break;
					case 5: { r0 = 1; c0 = 0; s0 = 1; }break;
					case 6: { r0 = 1; c0 = 1; s0 = 0; }break;
					case 7: { r0 = 1; c0 = 1; s0 = 1; }break;
					}
					
					//把一块中属于父节点的数据暂存到tempvalue1中
					for ( int i = r0; i < dian1; i += 2 )
					{
						for ( int j = c0; j < dian1; j += 2 )
						{
							for ( int k = s0; k < dian1; k += 2 )
							{
								tempvalue1.push_back( gradVector[t][i][j][k] );
							}
						}
					}//把一块中属于父节点的数据暂存到tempvalue1中，完毕

					//暂存在tempvalue1中的数据放到blockValue中
					vector<float>::iterator iter = tempvalue1.begin();
					for ( int i = 0; i < dian1/2; ++i )
					{
						for ( int j = 0; j < dian1/2; ++j )
						{
							for ( int k = 0; k < dian1/2; ++k )
							{
								//不同的块，数据放的位置不同
								switch (t)
								{
								case 0 :
									blockValue[i][j][k] = *iter; break;
								case 1 :
									blockValue[i][j][k+dian1/2] = *iter; break;
								case 2 :
									blockValue[i][j+dian1/2][k] = *iter; break;
								case 3 :
									blockValue[i][j+dian1/2][k+dian1/2] = *iter; break;
								case 4 :
									blockValue[i+dian1/2][j][k] = *iter; break;
								case 5 :
									blockValue[i+dian1/2][j][k+dian1/2] = *iter; break;
								case 6 :
									blockValue[i+dian1/2][j+dian1/2][k] = *iter; break;
								case 7 :
									blockValue[i+dian1/2][j+dian1/2][k+dian1/2] = *iter; break;
								}

								++iter;
							}
						}
					}//暂存在tempvalue1中的数据放到blockValue中，完毕

					//清空tempvalue1
					tempvalue1.erase( tempvalue1.begin(), tempvalue1.end() );
				}//由这num块得到了父节点的数据并放入到blockValue中，完毕
			}
			else
			{//需要插值
				double wSample, wCMP, wRecord;
				float  A, B, C, D, E, F, G, H, value1;

				//插值法
				for (int i = 0; i < parent->m_recordNum; ++i)
				{
					int l_record, r_record;
					int sign_record;//标识在哪个块上,0：在0块上，1：在1块上
					float temp1 = i * parent->m_sampleDistance[2];

					/*确定sign_record、l_record、r_record这三个值--begin*/
					if ( temp1 <= ( node->m_maxRecord - node->m_minRecord) )
					{//在node块上
						l_record = floor( temp1 / node->m_sampleDistance[2] );
						r_record = ceil( temp1 / node->m_sampleDistance[2] );
						wRecord = ( temp1 - l_record*node->m_sampleDistance[2] )/node->m_sampleDistance[2];
						sign_record = 0;
					}
					else
					{//在node的下一个块上
						KLOctreeNode* temN;
						switch( num )
						{
						case 8: temN = node->m_brother->m_brother->m_brother->m_brother;break;
						case 4: temN = node->m_brother->m_brother;break;
						case 2: temN = node->m_brother;break;
						}
						
						temp1 -= ( temN->m_minRecord - node->m_minRecord);
						l_record = floor( temp1 / temN->m_sampleDistance[2] );
						r_record = ceil( temp1 / temN->m_sampleDistance[2] );
						wRecord = ( temp1 - l_record*temN->m_sampleDistance[2] )/temN->m_sampleDistance[2];
						sign_record = 1;
					}
					/*确定sign_record、l_record、r_record这三个值--end*/

					for (int j = 0; j < parent->m_CMPNum; ++j)
					{
						int l_cmp, r_cmp;
						int sign_cmp;//标识在哪个块上,0：在0块上，1：在2块上
						temp1 = j * parent->m_sampleDistance[1];
						
						/*确定sign_cmp、l_cmp、r_cmp这三个值--begin*/
						if ( temp1 <= ( node->m_maxCMP - node->m_minCMP) )
						{//在node块上
							l_cmp = floor( temp1 / node->m_sampleDistance[1] );
							r_cmp = ceil( temp1 / node->m_sampleDistance[1] );
							wCMP = ( temp1 - l_cmp*node->m_sampleDistance[1] )/node->m_sampleDistance[1];
							sign_cmp = 0;
						} 
						else
						{//在node的下一个块上
							KLOctreeNode* temN;
							switch( num )
							{
							case 8: temN = node->m_brother->m_brother;break;
							case 4: {
										if (parent->m_children[0]&&parent->m_children[1]&&
											parent->m_children[2]&&parent->m_children[3])
											temN = node->m_brother->m_brother;
										else
											temN = node->m_brother;
									}break;
							case 2: temN = node->m_brother;break;
							}

							temp1 -= ( temN->m_minCMP - node->m_minCMP);
							l_cmp = floor( temp1 / temN->m_sampleDistance[1] );
							r_cmp = ceil( temp1 / temN->m_sampleDistance[1] );
							wCMP = ( temp1 - l_cmp*temN->m_sampleDistance[1] )/temN->m_sampleDistance[1];
							sign_cmp = 1;
						}					
						/*确定sign_cmp、l_cmp、r_cmp这三个值--end*/

						for (int k = 0; k < parent->m_sampleNum; ++k)
						{
							int l_sample, r_sample;
							int sign_sample;//标识在哪个块上,0：在0块上，1：在4块上
							temp1 = k * parent->m_sampleDistance[0];
							
							/*确定sign_sample、l_sample、r_sample这三个值--begin*/
							if ( temp1 <= ( node->m_maxSample - node->m_minSample) )
							{//在node块上
								l_sample = floor( temp1 / node->m_sampleDistance[0] );
								r_sample = ceil( temp1 / node->m_sampleDistance[0] );
								wSample = ( temp1 - l_sample*node->m_sampleDistance[0] )/node->m_sampleDistance[0];
								sign_sample = 0;
							} 
							else
							{//在node的下一个块上
									KLOctreeNode* tempnode2 = node->m_brother;
									temp1 -= ( tempnode2->m_minSample - node->m_minSample);
									l_sample = floor( temp1 / tempnode2->m_sampleDistance[0] );
									r_sample = ceil( temp1 / tempnode2->m_sampleDistance[0] );
									wSample = ( temp1 - l_sample*tempnode2->m_sampleDistance[0] )/tempnode2->m_sampleDistance[0];
									sign_sample = 1;
							}					
							/*确定sign_sample、l_sample、r_sample这三个值--end*/

							//A-H这8个值是:需要插值出的点的周围最近的8个点的值
							int tt;
							if ( sign_record==0 && sign_cmp==0 && sign_sample==0 )
							{
								tt = 0;//在0块上
							}
							else if ( sign_record==1 && sign_cmp==0 && sign_sample==0 )
							{
								switch(num)
								{
								case 8 : tt = 4; break;
								case 4 : tt = 2; break;
								case 2 : tt = 1; break;
								}
							}
							else if ( sign_record==0 && sign_cmp==1 && sign_sample==0 )
							{
								switch(num)
								{
								case 8 : tt = 2; break;
								case 2 : tt = 1; break;
								case 4 : {
									if( parent->m_children[0]&&parent->m_children[1]&&
										parent->m_children[2]&&parent->m_children[3] )
										tt = 2;
									else
										tt = 1;
										 }break;
								}
							}
							else if ( sign_record==1 && sign_cmp==1 && sign_sample==0 )
							{
								switch(num)
								{
								case 8 : tt = 6; break;
								case 4 : tt = 3; break;
								}
							}
							else if ( sign_record==0 && sign_cmp==0 && sign_sample==1 )
							{
								tt = 1;//在4块上
							}
							else if ( sign_record==1 && sign_cmp==0 && sign_sample==1 )
							{
								switch(num)
								{
								case 8 : tt = 5; break;
								case 4 : tt = 3; break;
								}
							}
							else if ( sign_record==0 && sign_cmp==1 && sign_sample==1 )
							{
								tt = 3;//在6块上
							}
							else if ( sign_record==1 && sign_cmp==1 && sign_sample==1 )
							{
								tt = 7;//在7块上
							}

							A = gradVector[tt][l_record][l_cmp][l_sample];
							B = gradVector[tt][l_record][l_cmp][r_sample];
							C = gradVector[tt][l_record][r_cmp][l_sample];
							D = gradVector[tt][l_record][r_cmp][r_sample];
							E = gradVector[tt][r_record][l_cmp][l_sample];
							F = gradVector[tt][r_record][l_cmp][r_sample];
							G = gradVector[tt][r_record][r_cmp][l_sample];
							H = gradVector[tt][r_record][r_cmp][r_sample];
							
							value1 = 
									(1.0 - wRecord) * (1.0 - wCMP) * (1.0 - wSample) * A +
									(1.0 - wRecord) * (1.0 - wCMP) *        wSample  * B +
									(1.0 - wRecord) *        wCMP  * (1.0 - wSample) * C +
									(1.0 - wRecord) *        wCMP  *        wSample  * D +
									wRecord  * (1.0 - wCMP) * (1.0 - wSample) * E +
									wRecord  * (1.0 - wCMP) *        wSample  * F +
									wRecord  * (      wCMP) * (1.0 - wSample) * G +
									wRecord  * (      wCMP) *        wSample  * H;

							blockValue[i][j][k] = value1;
						}
					}
				} //父节点块插值结束
			}//不管node->parent块是否需要插值，已经从子节点中把数据放到blockValue中
			
			pregradVector.push_back( blockValue );//把node->parent放入pregradVector中
			//把node->parent块写入文件
			parent->m_location = location;
			capacity = parent->m_sampleNum * parent->m_CMPNum * parent->m_recordNum;
			location += capacity;

			float* xie = new float[m*n*p];
			float *tr = xie;
			for ( int i = 0; i < m; ++i )
			{
				for ( int j = 0; j < n; ++j )
				{
					for ( int k = 0; k < p; ++k)
					{
						*tr = blockValue[i][j][k];
						++tr;
					}
					//fileOut.write( (char*)(&(blockValue[i][j][0])), p*4 );
					//问题：通过vector写入的时候老是漏写入数据，不知道原因，待查。
				}
			}
			fileOut.write( (char*)xie, m*n*p*4 );
			delete []xie;

			/*找到属于一个节点的n个子节点进行处理，把处理之后的这个节点保存并写入文件----end*/

			//把gradVector中前num个块清除掉
			gradVector.erase( gradVector.begin(), gradVector.begin()+num );

			//继续后续处理
			for ( int i = 0; i < num; ++i )
			{
				node = node->m_brother;
			}
		}//endwhile，此时都放在pregradVector中

		//释放内存
		vector<vector<vector<vector<float> > > > temp1;
		gradVector.swap( temp1 );
		/*从gradVector中获得父节点块放入pregradVector中，并写入文件----end*/

		//把pregradVector中的数据放到gradVector中，进行循环
		gradVector.swap( pregradVector );
		//清空pregradVector，备用
		vector< vector<vector<vector<float> > > > temp2;
		pregradVector.swap( temp2 );

		zuoyi = zuoyi->m_parent;//zuoyi指向父节点
	}//算法完毕，清空gradVector
		
	vector< vector<vector<vector<float> > > > temp12;
	gradVector.swap( temp12 );

	//=-=-=-=-=-=-=-=-=-=-算法完毕=-=-=-=-=-=-=-=-=-=-=-=-

	/*把八叉树节点的location值写入文件尾部，从根向叶子层序遍历的顺序写入--begin*/
	unsigned long long int *locationV = new unsigned long long int[ m_nodeNum ];
	unsigned long long int *temLV = locationV;
	KLOctreeNode *nodeTemp = m_rootNode;
	while ( nodeTemp )
	{
		//从根向叶子层序遍历写入location
		KLOctreeNode *node = nodeTemp;
		while( node )
		{
			*temLV = node->m_location;
			++temLV;
			node = node->m_brother;
		}
		nodeTemp = nodeTemp->m_children[0];
	}
	
	//fileOut.write( (char*)(&(locatoinVec[0])), locatoinVec.size()*sizeof(int) );
	fileOut.write( (char*)locationV, m_nodeNum*sizeof(unsigned long long int) );
	delete []locationV;
	/*把八叉树节点的location值写入文件尾部，从根向叶子层序遍历的顺序写入--end*/

	/*数据预处理完毕，关闭sgy数据文件和生成的分块的数据文件*/
	fileIn.close();
	fileOut.close();
	cout<<"预处理结束."<<std::endl;
	double dElapsed1 = timer.elapsed();
	std::cout<<dElapsed1<<"ms"<<std::endl<<dElapsed1/60000<<"min"<<std::endl;
}

//--------------------------
void KL3DVOctree::processGrid()
{
	//和processSgy()不同之处：使用规则表单的读取接口
}

//------------------------------------------------------------------------------------
void KL3DVOctree::setAbsoluteFileName( QString sourceFile, QString generateFile )
{
	m_sourceFileName = sourceFile;
	m_octreeFileName = generateFile;
}

//-------------------------------------
void KL3DVOctree::getSgyHeaderInfo()
{
	/*为了便于处理，对源数据相关信息进行了简化*/

	KLSgyHeader *sgyHeader =
		new KLSgyHeader( m_sourceFileName.toStdString().data() );
	sgyHeader->excuteSgyInfo();

	m_minValue = sgyHeader->getMinValue();
	m_maxValue = sgyHeader->getMaxValue();

	m_originx = sgyHeader->getOriginX();
	m_originy = sgyHeader->getOriginY();
	m_originz = sgyHeader->getOriginZ();

	m_numx = sgyHeader->getSamplesPerTrace();
	m_numy = sgyHeader->getTracesPerRecord();
	m_numz = sgyHeader->getTotalRecords();

	//min值默认为0，在sgy中实际上不是
	m_minX = m_minY = m_minZ = 0;
	m_maxX = m_numx - 1 + m_minX;
	m_maxY = m_numy - 1 + m_minY;
	m_maxZ = m_numz - 1 + m_minZ;

	m_gridSizexFact = sgyHeader->getSampleInterval();
	m_gridSizey = sgyHeader->getCMPInterval();
	m_gridSizez = sgyHeader->getRecordInterval();

	m_ratiox = ( m_gridSizey*(m_numy-1)+m_gridSizez*(m_numz-1) )/2/m_gridSizexFact/(m_numz-1);
	m_gridSizex = m_gridSizexFact*m_ratiox;
}

KL3DVOctree::KL3DVOctree(QString fileName)
{
	int info[3];
	float value[2];
	int bounds[6];
	float gridSizes[3];
	m_octreeFileName = fileName;
	std::ifstream fs;
	fs.open(fileName.toStdString(), std::ios::binary);
	ios::sync_with_stdio(false);
	if (!fs.is_open())
	{
		cout<<"can't open file\n";
		return;

	}


	fs.seekg(ios::beg);
	fs.read((char*)info, 3 * sizeof(int));
	fs.read((char*)bounds, 6 * sizeof(int));
	fs.read((char*)value, 2 * sizeof(float));
	fs.read((char*)gridSizes, 3 * sizeof(float));
	//fileIn->close();

	m_rootNode = NULL;
	m_zuoYezi = NULL;

	m_nodeNum = 0;

	m_BLOCKSIZE	 = info[0];
	m_depth		 = info[1];
	m_octreeSize = info[2];

	m_minX = bounds[0];
	m_maxX = bounds[1];
	m_minY = bounds[2];
	m_maxY = bounds[3];
	m_minZ = bounds[4];
	m_maxZ = bounds[5];

	m_minValue = value[0];
	m_maxValue = value[1];

	/*m_gridSizex = gridSizes[0];
	m_gridSizey = gridSizes[1];
	m_gridSizez = gridSizes[2];*/
	m_gridSizex = 1.0;
	m_gridSizey = 1.0;
	m_gridSizez = 1.0;
	
}

void KL3DVOctree::readOctreeFileTail()
{
	QFile fileIn( m_octreeFileName );
	QDataStream in( &fileIn );
	in.setFloatingPointPrecision( QDataStream::SinglePrecision );
	in.setByteOrder( QDataStream::LittleEndian );
	if ( !fileIn.open(QIODevice::ReadOnly) )
	{
		cout<<"can't open file : "<<m_octreeFileName.toStdString()<<std::endl;
		return;
	}

	//定位尾部
	unsigned long long int fileSize = fileIn.size();
	unsigned long long int tailSize = sizeof( unsigned long long int )*this->m_nodeNum;
	fileIn.seek( fileSize - tailSize );
	unsigned long long int *locationV = new unsigned long long int[ this->m_nodeNum ];
	fileIn.read( (char*)locationV, tailSize );

	unsigned long long int *temLV = locationV;
	KLOctreeNode *nodeTemp = this->m_rootNode;
	while ( nodeTemp )
	{
		//从根向叶子层序遍历写入location
		KLOctreeNode *node = nodeTemp;
		while( node )
		{
			node->m_location = *temLV;
			++temLV;
			node = node->m_brother;
		}
		nodeTemp = nodeTemp->m_children[0];
	}

	delete []locationV;

	fileIn.close();
}

void KL3DVOctree::recoverOctree()
{
	computeLimit();

	completeOctreeNode( m_rootNode, m_depth, 0, m_octreeSize,
		m_minX, m_maxX, 
		m_minY, m_maxY,
		m_minZ, m_maxZ );

	setTileInfo();
	readOctreeFileTail();
}
}//end namespace
}//end namespace xin
