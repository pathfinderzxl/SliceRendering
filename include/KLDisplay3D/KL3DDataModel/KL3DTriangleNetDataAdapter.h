#pragma once
#ifndef __KL3DDATAMODEL_KL3DTRIANGLENETDATAADAPTER_H__
#define __KL3DDATAMODEL_KL3DTRIANGLENETDATAADAPTER_H__

#include <vector>
#include "KL3DDataModelMacro.h"

BEGIN_KLDISPLAY3D_NAMESPACE

class KL3DDATAMODEL_EXPORT KL3DTriangleNetDataAdapter
{
public:
	KL3DTriangleNetDataAdapter();
	virtual ~KL3DTriangleNetDataAdapter();
	//����������֧�ֵ���������ԣ�����ֻ��xy����zֵ��Ϊһ�����Դ���
	/*
	\ brief  ��ȡ��������
	\ return 
	\ param  	
	*/
	virtual unsigned int getVertexCount()=0;
	/*
	\ brief  ��ȡ����������
	\ return 
	\ param  	
	*/
	virtual unsigned int getTriangleCount()=0;
	/*
	\ brief  ��ȡ��������
	\ return void
	\ param  unsigned int idx ������
	��param  �������� x��y
	*/
	virtual void getVertexCoord(unsigned int idx , double& x ,double& y)=0;
	/*
	\ brief  ��ȡ�����ζ�������
	\ return void
	\ param  �����εı��idx
	\ param	 ��ʱ�������ζ�������,v0,v1,v2
	*/
	virtual void getTriangleIndex(unsigned int idx,unsigned int& v0,unsigned int& v1,unsigned int&v2)=0;
	/*
	\ brief  ��ȡ���������������ֵ
	\ return void
	\ param  ��������idx����������proidx
	*/
	virtual double getValue(unsigned int Idx,unsigned int proIdx=0)=0;
};
END_KLDISPLAY3D_NAMESPACE


#endif