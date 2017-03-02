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
\brief ��ȡsgy�����ļ������Ϣ���ڽ��˲���
*/
class KL3DDATAMODEL_EXPORT KLSgyHeader
{
public:
	/*!
	\const char* fileName--sgy�����ļ�ȫ·����
	*/
	KLSgyHeader( const char* fileName );
	~KLSgyHeader();

	/**
	\���sgy�����ļ�����Ϣ
	*/
	void excuteSgyInfo();

	/*�ļ�ͷ��Ϣ�������Ļ��*/
	void outputFileHeader();

	/*!�������ṩsgy�����Ϣ------begin-----*/

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

	/*!�������ṩsgy�����Ϣ------end-----*/



private:
	/**
	\ÿһ����trace����������Ŀ
	*/
	unsigned short m_samplesPerTrace;

	/**
	\CMP��Ŀ��ÿ�������еĵ���Ŀ
	*/
	unsigned short m_tracesPerRecord;

	/**
	\������Ŀ
	*/
	unsigned short m_totalRecords;

	/**
	\�ܵ�����ͨ���ļ���С��ÿ���Ĵ�С���
	*/
	unsigned int m_totalTraces;

	/**
	\sgy�ļ���СB
	*/
	unsigned long long m_fileSize;

	/**
	\���ݱ����ʽ��1��4�ֽ�IBM���㣨��׼�� 2:4�ֽ����ͣ���׼��
	\3��2�ֽ����ͣ���׼�� 5��4�ֽ�IEEE���㣨�Ǳ�׼��
	\8��1�ֽ����ͣ��Ǳ�׼��
	\�ڼ�����ڲ�ʹ�õ���IEEE���㣬ת����ʽ
	*/
	unsigned short m_dataCodeType;
	
	unsigned short m_minSample, m_minCMP, m_minRecord;
	unsigned short m_maxSample, m_maxCMP, m_maxRecord;
	unsigned short m_sampleNum, m_cmpNum, m_recordNum;
	/**
	\sgy�ļ���
	*/
	const char* m_fileName;

	/*!
	\brief ��һ�����ص������
	\note  Ŀǰ��sgy�����ļ���û�з��ִ�����
	\	   Ĭ�϶���Ϊ(12.5, 3.4, 10)
	*/
	double m_origin[3];

	/*!
	\brief ��������������
	\note  ��������´���ʱ��ÿ��һ��ʱ�䣨us������һ�β�������ʱ�伴ΪsampleInterval��
	\	   Ŀǰ��sgy�����ļ���û�з���cmp��record��������������(һ�㵥λ����)
	\	   �˴����������ϵĲ����������ʹ��Ĭ��ֵ0.5s, 1.5m, 1.5m
	*/
	double m_sampleInterval, m_cmpInterval, m_recordInterval;

	/*!
	\brief ����ֵ����Сֵ�����ֵ
	\note  Ŀǰ��sgy�����ļ���û�з��ִ�����
	\	   Ŀǰ��ʹ��
	*/
	float m_minValue, m_maxValue;
};

}//end namespace KLSgy
}//end namespace xin
#endif
