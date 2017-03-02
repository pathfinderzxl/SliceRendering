#include <iostream>
#include "qfile.h"
#include "qdatastream.h"
#include "KLSgyHeader.h"

namespace KLSeis{
namespace KLDisplay3D{

using std::cout;
using std::endl;

//----------------------------------------------
KLSgyHeader::KLSgyHeader( const char* fileName )
{
	m_samplesPerTrace = 0;
	m_tracesPerRecord = 0;
	m_totalRecords    = 0;
	m_totalTraces     = 0;
	m_fileSize		  = 0;
	m_dataCodeType	  = 0;
	m_sampleInterval  = 0;
	m_minSample = m_maxSample = 0;
	m_minCMP    = m_maxCMP    = 0;
	m_minRecord = m_maxRecord = 0;
	m_fileName = fileName;
}

//-------------------------
KLSgyHeader::~KLSgyHeader()
{
	m_fileName = NULL;
}

//-------------------------------
void KLSgyHeader::excuteSgyInfo()
{
	//DWORD start_time = GetTickCount();
	//std::cout<<"��ȡsgy�ļ���Ϣ��ʼ..."<<std::endl;

	QFile file( m_fileName );
	QDataStream in( &file );
	in.setByteOrder( QDataStream::LittleEndian );
	if( !file.open( QIODevice::ReadOnly ) ) 
	{
		std::cout<<"can't open file : "<<m_fileName<<std::endl;
		return ;
	}

	/*��������´���ʱ��ÿ��һ��ʱ�䣨us������һ�β�������ʱ�伴ΪsampleInterval��
	\ Ŀǰ�Ȳ���
	*/
	unsigned short sampleInterval;
	file.seek(3216);
	in>>sampleInterval;
	
	file.seek(3220);
	in>>m_samplesPerTrace;
	
	file.seek(3224);
	in>>m_dataCodeType;

	unsigned int lengthOfTrace = m_samplesPerTrace * 4 + 240;
	m_fileSize = file.size();
	m_totalTraces = ( m_fileSize - 3600 ) / lengthOfTrace ;

	file.seek(3608);
	in>>m_minRecord;

	file.seek(3620);
	in>>m_minCMP;

	file.seek( 3608 + (m_totalTraces - 1)*static_cast<unsigned long long>(lengthOfTrace) );
	in>>m_maxRecord;

	file.seek( 3620 + (m_totalTraces - 1)*static_cast<unsigned long long>(lengthOfTrace) );
	in>>m_maxCMP;

	file.close();

	m_tracesPerRecord = m_maxCMP - m_minCMP + 1;
	m_totalRecords = m_maxRecord - m_minRecord + 1;

	m_minSample = 0;
	m_maxSample = m_samplesPerTrace - 1;

	//�õ���ά�Ȳ�������
	m_sampleNum = m_maxSample - m_minSample;
	m_cmpNum = m_maxCMP - m_minCMP;
	m_recordNum = m_maxRecord - m_minRecord;
	//�õ���ά�Ȳ�������

	//�õ��������ȡֵ��Χ
	float temp;
	m_minValue = 0;
	m_maxValue = 0;
	file.seek(3600);//����3600B�ľ�ͷ
	for (int i = 0; i <= (this->m_recordNum - 1); ++i)
	{
		for (int j=0;j <= (this->m_cmpNum - 1); ++j)
		{
			file.seek(file.pos() + 240);//����ÿһ��cmp�ĵ�ͷ
			for (int k=0; k <= (this->m_sampleNum - 1); ++k)
			{
				in>>temp;
				if (m_minValue > temp)
					m_minValue = temp;
				if (m_maxValue < temp)
					m_maxValue = temp;
			}
		}
	}
	//�õ��������ȡֵ��Χ

	//Ŀǰ�����ó��Զ���
	m_origin[0] = 12.5;
	m_origin[1] = 3.4;
	m_origin[2] = 10;
	m_sampleInterval = 0.5;
	m_cmpInterval = 1.5;
	m_recordInterval = 1.5;

	//DWORD end_time = GetTickCount();
	//std::cout<<"��ȡsgy�ļ���Ϣ����.��ʱ��"<<start_time-end_time<<"ms"<<std::endl;

	/*std::cout
			<<"ÿ������������"<<m_samplesPerTrace<<std::endl
			<<"ÿ�����ߵ�����"<<m_tracesPerRecord<<std::endl
			<<"��������    ��"<<m_totalRecords<<std::endl
			<<"�������    ��"<<m_sampleInterval<<"us"<<std::endl
			<<"���ݱ����ʽ��"<<m_dataCodeType<<std::endl
			<<"���ݵ�����  ��"<<m_totalTraces<<std::endl			
			<<"��������    ��"<<m_minRecord<<"--"<<m_maxRecord<<std::endl			
			<<"CMP����     ��"<<m_minCMP<<"--"<<m_maxCMP<<std::endl;*/
}

//---------------------------------
void KLSgyHeader::outputFileHeader()
{
	cout<<"File: "<<m_fileName<<endl;
	cout<<"NumSample: "<<m_samplesPerTrace
		<<"   NumCMP: "<<m_tracesPerRecord
		<<"   NumRecord: "<<m_totalRecords<<endl;
	cout<<"IntervalSample: "<<m_sampleInterval<<"s"
		<<"   IntervalCMP: "<<m_cmpInterval<<"m"
		<<"   IntervalRecord: "<<m_recordInterval<<"m"<<endl;
	cout<<"SampleMin: "<<m_minSample
		<<"   CMPMin: "<<m_minCMP
		<<"   RecordMin: "<<m_minRecord<<endl;
	cout<<"Origin(s,c,r): ("<<m_origin[0]<<", "
		<<m_origin[1]<<", "<<m_origin[2]<<")"<<endl;
}

}//end namespace KLSgy
}//end namespace xin
