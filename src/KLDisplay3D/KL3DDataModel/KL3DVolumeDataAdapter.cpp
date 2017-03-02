
#include "KL3DVolumeDataAdapter.h"
#include <fstream>
#include <iostream>

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DVolumeDataAdapter::KL3DVolumeDataAdapter()
{
	AdapterType = WHOLE;
	
	m_volumeData = NULL;
	isRead = false;
	m_volumeDataThreshold = 419430400;
	m_octreeFileName = "";
}

//
KL3DVolumeDataAdapter::~KL3DVolumeDataAdapter()
{
	/*if ( NULL != m_volumeData )
	{
	delete []m_volumeData;
	m_volumeData = NULL;
	}*/
	/*if (NULL != m_octree || NULL != m_octreeFileName)
	{
	m_octree->~KL3DVOctree();

	}*/
}
void KL3DVolumeDataAdapter::setVolumeDataThreshold(long long int threshold)
{
	if (threshold <= 0)
	{
		std::cout<<"hehe-error:"
			<<"KL3DVolumeDataAdapter.setVolumeDataThreshold()"<<std::endl;
		return;
	}
	m_volumeDataThreshold = threshold;
	//m_volHeader.DataThreshold = threshold;

}
bool KL3DVolumeDataAdapter::testFile(char *filename)
{
	std::fstream ftest;
	ftest.open(filename,std::ios::in | std::ios::binary);
	if(ftest.is_open())
	{
		ftest.close();
		return true;
	}
	return false;
}
//
bool KL3DVolumeDataAdapter::excuteVolumeData()
{
	bool boolValue = requestInformation();
	if ( false == boolValue )
	{
		std::cout<<"hehe-error:"
			<<"KL3DVolumeDataAdapter.excuteVolumeData()"<<std::endl;
		return false;
	}

	if ( PART == AdapterType )
	{
		m_volumeData = NULL;
		return true;
	}
	if ( WHOLE != AdapterType )
	{
		std::cout<<"hehe-error:"
			<<"KL3DVolumeDataAdapter.excuteVolumeData()"<<std::endl;
		return false;
	}

	long long int numSam = m_volHeader.numFirst * m_volHeader.numSecond *m_volHeader.numThird;
	long long int volumeSize = sizeof(float) * numSam;

	if ( m_volHeader.DataSize == SMALL )
	{	
		//�����С������Ҫ���ڴ��п��ٿռ䣬�����ݶ���
		m_volumeData = new float[ numSam ];
		read( (char*)m_volumeData, volumeSize, 0 );
		isRead = true;
		return true;
	}

	if ( m_volHeader.DataSize == BIG )
	{
		//����Ǵ�������Ҫ��ԭ�ļ�ת���ɰ˲����ļ�,���˲����ļ��Ѿ����ھ�����ת����
		//���˰˲����ļ�֮�󣬻���Ҫ���˲����ļ��ع�����Ӧ�İ˲����ṹ

		//�˲����ļ�������ʱ
		if ( "" == m_volHeader.octreeFileName)
		{
			//����Ԥ�������ﲻ���ǣ�Ĭ��һ�����ж�Ӧ�˲����ļ�
			std::cout<<"hehe-error: octree file is not exist!"<<std::endl;
			return false;
		}
		else//�˲����ļ�����ʱ
		{
			m_octreeFileName = m_volHeader.octreeFileName;
			return true;
		}

	}

	return false;
}

void KL3DVolumeDataAdapter::getVolumeBoundsXYZ(double* bounds)
{
	if ( NULL == bounds)
	{
		std::cout<<"hehe-error:KL3DVolumeDataAdapter.getVolumeBoundsXYZ"<<std::endl;
		return;
	}
	if ( m_volHeader.numFirst == 1 && m_volHeader.numSecond == 1 && m_volHeader.numThird == 1)
	{
		std::cout<<"hehe-error:KL3DVolumeDataAdapter.getVolumeBoundsXYZ"<<std::endl;
		return;
	}
	double temp_bound[3] = {
		m_volHeader.numFirst * m_volHeader.interFirst,
		m_volHeader.numSecond * m_volHeader.interSecond,
		m_volHeader.numThird * m_volHeader.interThird
	};

	int j = m_volHeader.orderXYZ[0];
	bounds[0] = m_volHeader.origin[0];
	bounds[1] = m_volHeader.origin[0]+temp_bound[j]-1;
		j = m_volHeader.orderXYZ[1];
	bounds[2] = m_volHeader.origin[1];
	bounds[3] = m_volHeader.origin[1]+temp_bound[j]-1;
		j = m_volHeader.orderXYZ[2];
	bounds[4] = m_volHeader.origin[2];
	bounds[5] = m_volHeader.origin[2]+temp_bound[j]-1;

}
END_KLDISPLAY3D_NAMESPACE
