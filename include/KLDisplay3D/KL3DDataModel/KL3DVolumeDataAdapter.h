
#ifndef __KL3DDATAMODEL_KL3DVOLUMEDATAADAPTER__
#define __KL3DDATAMODEL_KL3DVOLUMEDATAADAPTER__

#include <string>
#include "KL3DDataModelMacro.h"
#include "KL3DVOctree.h"
//typedef long long int INT64
const int FloatByte_he = sizeof(float);
BEGIN_KLDISPLAY3D_NAMESPACE

/*!
\brief �����ݵ�״̬��Ϣ
\note1 ���������ȴ洢˳��first-second-third
\note2 ͨ�����������������ϵĲ�������Ŀ������������ֵ����������(float)
\<br>  ���Լ���������ݴ�С����λB����������ֵ��Χ�ھ���ΪС
\<br>  ���ݴ���������Ϊ�����ݴ���
*/
struct VolumeHeader
{
	//First����Ĳ�������Ŀ
	int    numFirst;
	//Second����Ĳ�������Ŀ
	int    numSecond;
	//Third����Ĳ�������Ŀ
	int    numThird;
	
	//First����Ĳ������
	double   interFirst;
	//Second����Ĳ������
	double   interSecond;
	//Third����Ĳ������
	double   interThird;

	//First����Ĳ�������ĵ�λ����
	std::string   nameFirst;
	//Second����Ĳ�������ĵ�λ����
	std::string   nameSecond;
	//Third����Ĳ�������ĵ�λ����
	std::string   nameThird;

	//�����ļ�������ֵ����Сֵ
	float    minVal;
	//�����ļ�������ֵ�����ֵ
	float    maxVal;

	//��һ�������������
	double   origin[3];

	//�������а����û�ָ����˳����0��1��2��λ��0��ʾx�ᣬλ��1��ʾy�ᣬλ��2��ʾz��
	//���磬orderXYZ[3] = {1,2,0}second��Ϊx�ᣨCMP����Ϊx�ᣩ��third��Ϊy�ᣨRecord��Ϊy�ᣩ��first��sample����Ϊz��
	int orderXYZ[3];

	//���ݵ�ԭ�ļ��������ݿ��ܴ����С��
	QString sourceFileName;

	//������ת���ɵĶ�Ӧ�İ˲����ļ���
	QString octreeFileName;

	//ͨ��������ֵ�жϳ�������С���Ǵ���������
	int DataSize;
	/*enum
	{
	BIG,
	SMALL
	};*/

	VolumeHeader()
	{
		numFirst = 1;
		numSecond = 1;
		numThird = 1;
		
		interFirst = 1;
		interSecond = 1;
		interThird = 1;

		nameFirst = "s"; //��
		nameSecond = "m"; //��
		nameThird = "m"; //��

		minVal = 0.0f;
		maxVal = 0.0f;
		origin[0] = origin[1] = origin[2] = 0;
		//��ʼֵ��x���Ӧsample��
		orderXYZ[0] = 0;orderXYZ[1] = 1;orderXYZ[2] = 2;
		/*orderXYZ[0] = 1;orderXYZ[1] = 2;orderXYZ[2] = 0;*/
	
		sourceFileName = "";
		octreeFileName = "";
		
		DataSize = -1; //ö�����ʹ�0��ʼ��ţ���ʼ��ʹ�ø���
	}
	
};

/*!
\brief ��ά������������
\note1 ��Ӧ�ÿ�����ʹ��ʱҪ�������ಢʵ�ִ��麯��
*/
class KL3DDATAMODEL_EXPORT KL3DVolumeDataAdapter
{
public:
	KL3DVolumeDataAdapter();
	virtual ~KL3DVolumeDataAdapter();

	//
	enum
	{
		WHOLE, //���Ի�ȡ������
		PART   //ֻ���Ի�ȡ��Ƭ
	};
	int AdapterType;

	//���ڱ�������Ǵ����ݻ���С����
	enum
	{
		BIG,
		SMALL
	};
	/*!
	\brief ���������ṩ����
	\note1 ����������Ҫ������ʱ���ô˺�������
	*/
	virtual bool excuteVolumeData();
	
	// ����ļ��Ƿ����
	bool testFile(char *fileName);
	/*!
	\brief ���������ṩ�����ݻ�����Ϣ
	*/
	struct VolumeHeader getVolumeHeader()
	{
		return m_volHeader;
	}

	/*!
	\brief ���������ṩ�洢�������ڴ��ָ��
	*/
	float*  getVolumeData()
	{
		return m_volumeData;
	}
	void  setVolumeData(float* data)
	{
		 m_volumeData = data;
	}
	void setSourceFilename(QString sourceFileName)
	{
		m_sourceFileName = sourceFileName;
	}
	/*!
	\brief ���������ṩ�˲����ļ�
	*/
	QString  getOctreeFileName()
	{
		return  m_octreeFileName;
	}
	void setOctreeFileName(QString fileName)
	{
		m_octreeFileName  = fileName;
	}
	//�����ṩ����ϵ�и�������������Сֵ
	//����֮ǰ����ȷ��requestinformation�����Ѿ�����
	void getVolumeBoundsXYZ(double* bounds);

protected:
	/*!
	\brief ��ȡ�����ļ�������Ϣ�������ݵ���ֵ
	\note1 �ѻ�ȡ����Щ��Ϣ��ŵ�struct VolumeHeader m_volHeader��
	*/
	virtual bool requestInformation() = 0;

	/*!
	\brief ���մ洢˳���ȡһ������
	\param char* �洢���ݵ��ڴ�ռ�ָ��
	\<br>  length ������ݵĳ��ȣ���λB
	\<br>  pos ��ȡ�Ŀ�ʼλ�ã���λB
	\note1 pos=0����ʾ�ӵ�һ�������㿪ʼ��
	\note2 WHOLE == AdapterTypeʱ����Ҫʵ�ִ˺���
	*/
	virtual bool read( char*, long long int length, long long int pos ) = 0;

	/*!
	\brief ��ȡ��Ƭ����
	\param int slice - ȡֵ0��1��2���ֱ����first-second-third��
	\param int pos - ȡֵ0 - num����ʾ�ڼ�����Ƭ
	\param char* - �洢��Ƭ����
	*/
	virtual bool readSliceData( int slice, int pos, char* ) = 0;

	/*!
	\brief ��ȡָ��λ�õ�����ֵ
	\param int x, int y, int z ���������ϵĲ���������
	\note1 ������0��ʼ���
	*/
	//virtual float getValue( int x, int y, int z ) = 0;

	/*
	\ brief  �������ݵ���ֵ�����Զ�����趨(����ͷ����ֵ������)
	\ return 
	\ param  	long long int
	*/
	void setVolumeDataThreshold(long long int);
	
	

protected:
	//�洢�����ݵĻ�����Ϣ
	struct VolumeHeader   m_volHeader;

	//��������С����ʱ�����ڴ���λ��ָ��
	float    *m_volumeData;
	bool	isRead;
	//�����ݵ���ֵ��������ֵ����������
	long long int  m_volumeDataThreshold;

	
	QString m_sourceFileName;
	QString m_octreeFileName;
};


//const int VolMax = 209715200; //209715200B = 200MB;

END_KLDISPLAY3D_NAMESPACE

#endif //__KL3DDATAMODEL_KL3DVOLUMEDATAADAPTER__

