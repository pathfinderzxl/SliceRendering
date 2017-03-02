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
	//数据适配器支持单顶点多属性，索引只有xy方向，z值作为一个属性存在
	/*
	\ brief  获取顶点数量
	\ return 
	\ param  	
	*/
	virtual unsigned int getVertexCount()=0;
	/*
	\ brief  获取三角形数量
	\ return 
	\ param  	
	*/
	virtual unsigned int getTriangleCount()=0;
	/*
	\ brief  获取顶点坐标
	\ return void
	\ param  unsigned int idx 顶点编号
	、param  顶点坐标 x，y
	*/
	virtual void getVertexCoord(unsigned int idx , double& x ,double& y)=0;
	/*
	\ brief  获取三角形顶点索引
	\ return void
	\ param  三角形的编号idx
	\ param	 逆时针三角形顶点索引,v0,v1,v2
	*/
	virtual void getTriangleIndex(unsigned int idx,unsigned int& v0,unsigned int& v1,unsigned int&v2)=0;
	/*
	\ brief  获取给定索引点的属性值
	\ return void
	\ param  顶点索引idx，属性索引proidx
	*/
	virtual double getValue(unsigned int Idx,unsigned int proIdx=0)=0;
};
END_KLDISPLAY3D_NAMESPACE


#endif