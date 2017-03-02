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
	//std::cout<<"读取sgy文件信息开始..."<<std::endl;

	QFile file( m_fileName );
	QDataStream in( &file );
	in.setByteOrder( QDataStream::LittleEndian );
	if( !file.open( QIODevice::ReadOnly ) ) 
	{
		std::cout<<"can't open file : "<<m_fileName<<std::endl;
		return ;
	}

	/*地震波向地下传递时，每隔一定时间（us）进行一次采样，此时间即为sampleInterval。
	\ 目前先不用
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

	//得到各维度采样点数
	m_sampleNum = m_maxSample - m_minSample;
	m_cmpNum = m_maxCMP - m_minCMP;
	m_recordNum = m_maxRecord - m_minRecord;
	//得到各维度采样点数

	//得到采样点的取值范围
	float temp;
	m_minValue = 0;
	m_maxValue = 0;
	file.seek(3600);//跳过3600B的卷头
	for (int i = 0; i <= (this->m_recordNum - 1); ++i)
	{
		for (int j=0;j <= (this->m_cmpNum - 1); ++j)
		{
			file.seek(file.pos() + 240);//跳过每一个cmp的道头
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
	//得到采样点的取值范围

	//目前先设置成自定义
	m_origin[0] = 12.5;
	m_origin[1] = 3.4;
	m_origin[2] = 10;
	m_sampleInterval = 0.5;
	m_cmpInterval = 1.5;
	m_recordInterval = 1.5;

	//DWORD end_time = GetTickCount();
	//std::cout<<"读取sgy文件信息结束.用时："<<start_time-end_time<<"ms"<<std::endl;

	/*std::cout
			<<"每道采样点数："<<m_samplesPerTrace<<std::endl
			<<"每条测线道数："<<m_tracesPerRecord<<std::endl
			<<"测线总数    ："<<m_totalRecords<<std::endl
			<<"采样间隔    ："<<m_sampleInterval<<"us"<<std::endl
			<<"数据编码格式："<<m_dataCodeType<<std::endl
			<<"数据道总数  ："<<m_totalTraces<<std::endl			
			<<"测线区间    ："<<m_minRecord<<"--"<<m_maxRecord<<std::endl			
			<<"CMP区间     ："<<m_minCMP<<"--"<<m_maxCMP<<std::endl;*/
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
