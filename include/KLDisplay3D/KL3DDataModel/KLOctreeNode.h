/************************************************************************
Auther: xinbing
Date: 2013-9-22
Note:
************************************************************************/
#ifndef __KL3DDATA_KLOCTREENODE_H__
#define __KL3DDATA_KLOCTREENODE_H__

#include "KL3DDataModelMacro.h"

/*��׼�����������ϵĲ�������Ŀ
\128--8M
\64--1M
*/
//#define BLOCKSIZE 128
#define BLOCKSIZE 64
BEGIN_KLDISPLAY3D_NAMESPACE

/*!
\�˲����ڵ㣬һ���ڵ����һ���ֿ�
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
	\ָ���ұߵ��ֵ�
	*/
	KLOctreeNode *m_brother;

	/**
	\ȡֵ0~depth
	*/
	int m_level;

	/**
	\2��n���ݣ�n��0��depth
	\��ʾ�����壺�ڲ���ֵ������£��˿�Ĳ���������Ҷ�Ӳ��������ı��������������ϱ���һ����
	*/
	int m_tileStep;
	
	/**
	\����������ʵ�ʵĲ������ȣ�������ֵ�Ͳ���ֵ�������
	*/
	float m_sampleDistance[3];

	/**
	\true--��׼�� false--�����׼��
	*/
	bool m_full;

	/**
	\�󲿷ֿ���BLOCKSIZE,���ڵ㼰�丽���ڵ���С
	*/
	int m_tileSize;

	int m_minSample, m_maxSample;
	int m_minCMP,    m_maxCMP;
	int m_minRecord, m_maxRecord;
	int m_sampleNum, m_CMPNum, m_recordNum;

	/**
	\true--Ҷ�� false--��Ҷ��
	*/
	bool m_emptyChild;

	/**
	\�ǿպ��ӽڵ���Ŀ��0~8��
	*/
	int m_numChild;

	/**
	\ÿ���ֿ����������
	*/
	double m_center[3];

	/**
	\ÿ���ֿ�İ˸����������
	*/
	double m_vertices[8][3];

	/**
	\�Խ��ߵĳ���
	*/
	double m_diagnose;

	/**
	\�ڵ��Ӧ��С����Ԥ����֮����ļ��е�λ��,��λ4B
	*/
	unsigned long long int m_location;//sizeof(unsigned long long int)=8

	/**
	\�ڵ�������ļ��е�λ��	
	*/
	unsigned long int m_entropyLocation;

	/**
	\brief ��ʶ���ڰ˲����е�λ��
	\note  Ŀǰ���������������ơ�
	\      ���KL3DVOctree::setTileInfo()����ע�͵��Ĳ���
	*/
	char *m_wei;

	bool m_visible;

	/**
	\brief ��ǽ���Ƿ����Ӿ�����
	\note  0�������ȫ���Ӿ�����
	\	   1��������Ӿ����ཻ
	\	   2�������ȫ���Ӿ�����
	*/
	int m_isInFrustum;

	/*!�ڵ�����*/
	int m_iNodeIdx;

	/*!�ڵ����*/
	double m_entropy;
};

END_KLDISPLAY3D_NAMESPACE
#endif
