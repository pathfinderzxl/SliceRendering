/************************************************************************
Auther: xinbing
Date: 2013-9-22
Note:
************************************************************************/
#ifndef __KL3DDATA_KLOCTREENODE_H__
#define __KL3DDATA_KLOCTREENODE_H__

#include "KL3DDataModelMacro.h"

/*标准块三个方向上的采样点数目
\128--8M
\64--1M
*/
//#define BLOCKSIZE 128
#define BLOCKSIZE 64
BEGIN_KLDISPLAY3D_NAMESPACE

/*!
\八叉树节点，一个节点代表一个分块
*/
class KL3DDATAMODEL_EXPORT KLOctreeNode
{
public:
	KLOctreeNode();
	~KLOctreeNode();

	void getNodeBounds(float * bounds);

public:
	KLOctreeNode *m_parent;
	KLOctreeNode *m_children[8];

	/**
	\指向右边的兄弟
	*/
	KLOctreeNode *m_brother;

	/**
	\取值0~depth
	*/
	int m_level;

	/**
	\2的n次幂，n从0到depth
	\表示的意义：在不插值的情况下，此块的采样步长对叶子采样步长的倍数（三个方向上倍数一样）
	*/
	int m_tileStep;
	
	/**
	\三个方向上实际的采样长度，包括插值和不插值两种情况
	*/
	float m_sampleDistance[3];

	/**
	\true--标准块 false--不足标准块
	*/
	bool m_full;

	/**
	\大部分块是BLOCKSIZE,根节点及其附近节点略小
	*/
	int m_tileSize;

	int m_minSample, m_maxSample;
	int m_minCMP,    m_maxCMP;
	int m_minRecord, m_maxRecord;
	int m_sampleNum, m_CMPNum, m_recordNum;

	/**
	\true--叶子 false--非叶子
	*/
	bool m_emptyChild;

	/**
	\非空孩子节点数目（0~8）
	*/
	int m_numChild;

	/**
	\每个分块的中心坐标
	*/
	double m_center[3];

	/**
	\每个分块的八个顶点的坐标
	*/
	double m_vertices[8][3];

	/**
	\对角线的长度
	*/
	double m_diagnose;

	/**
	\节点对应的小块在预处理之后的文件中的位置,单位4B
	*/
	unsigned long long int m_location;//sizeof(unsigned long long int)=8

	/**
	\节点的熵在文件中的位置	
	*/
	unsigned long int m_entropyLocation;

	/**
	\brief 标识块在八叉树中的位置
	\note  目前不做处理，留待完善。
	\      详见KL3DVOctree::setTileInfo()函数注释掉的部分
	*/
	char *m_wei;

	bool m_visible;

	/**
	\brief 标记结点是否在视景体内
	\note  0：结点完全在视景体外
	\	   1：结点与视景体相交
	\	   2：结点完全在视景体内
	*/
	int m_isInFrustum;

	/*!节点索引*/
	int m_iNodeIdx;

	/*!节点的熵*/
	double m_entropy;
};

END_KLDISPLAY3D_NAMESPACE
#endif
