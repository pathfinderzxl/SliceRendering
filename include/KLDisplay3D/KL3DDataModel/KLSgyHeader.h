/************************************************************************
Auther: xinbing
Date: 2013-10-10
Note:
************************************************************************/
#ifndef __KLSGY_KLSGYHEADER_H__
#define __KLSGY_KLSGYHEADER_H__

#include "KL3DDataModelMacro.h"

namespace KLSeis{
namespace KLDisplay3D{

/*!
\brief 读取sgy数据文件相关信息用于建八叉树
*/
class KL3DDATAMODEL_EXPORT KLSgyHeader
{
public:
	/*!
	\const char* fileName--sgy数据文件全路径名
	*/
	KLSgyHeader( const char* fileName );
	~KLSgyHeader();

	/**
	\获得sgy数据文件的信息
	*/
	void excuteSgyInfo();

	/*文件头信息输出到屏幕上*/
	void outputFileHeader();

	/*!向类外提供sgy相关信息------begin-----*/

	int getSamplesPerTrace() { return (int)m_samplesPerTrace; }
	int getTracesPerRecord() { return (int)m_tracesPerRecord; }
	int getTotalRecords() { return (int)m_totalRecords; }

	double getMinValue(){return m_minValue;}
	double getMaxValue(){return m_maxValue;}

	double getOriginX() { return m_origin[0]; }
	double getOriginY() { return m_origin[1]; }
	double getOriginZ() { return m_origin[2]; }

	double getSampleInterval() { return m_sampleInterval; }
	double getCMPInterval() { return m_cmpInterval; }
	double getRecordInterval() { return m_recordInterval; }

	/*!向类外提供sgy相关信息------end-----*/



private:
	/**
	\每一道（trace）采样点数目
	*/
	unsigned short m_samplesPerTrace;

	/**
	\CMP数目：每条测线中的道数目
	*/
	unsigned short m_tracesPerRecord;

	/**
	\测线数目
	*/
	unsigned short m_totalRecords;

	/**
	\总道数。通过文件大小和每道的大小求出
	*/
	unsigned int m_totalTraces;

	/**
	\sgy文件大小B
	*/
	unsigned long long m_fileSize;

	/**
	\数据编码格式。1：4字节IBM浮点（标准） 2:4字节整型（标准）
	\3：2字节整型（标准） 5：4字节IEEE浮点（非标准）
	\8：1字节整型（非标准）
	\在计算机内部使用的是IEEE浮点，转换格式
	*/
	unsigned short m_dataCodeType;
	
	unsigned short m_minSample, m_minCMP, m_minRecord;
	unsigned short m_maxSample, m_maxCMP, m_maxRecord;
	unsigned short m_sampleNum, m_cmpNum, m_recordNum;
	/**
	\sgy文件名
	*/
	const char* m_fileName;

	/*!
	\brief 第一个像素点的坐标
	\note  目前在sgy数据文件中没有发现此属性
	\	   默认定义为(12.5, 3.4, 10)
	*/
	double m_origin[3];

	/*!
	\brief 三个方向采样间隔
	\note  地震波向地下传递时，每隔一定时间（us）进行一次采样，此时间即为sampleInterval。
	\	   目前在sgy数据文件中没有发现cmp和record方向采样间隔属性(一般单位是米)
	\	   此处三个方向上的采样间隔属性使用默认值0.5s, 1.5m, 1.5m
	*/
	double m_sampleInterval, m_cmpInterval, m_recordInterval;

	/*!
	\brief 标量值的最小值和最大值
	\note  目前在sgy数据文件中没有发现此属性
	\	   目前不使用
	*/
	float m_minValue, m_maxValue;
};

}//end namespace KLSgy
}//end namespace xin
#endif
