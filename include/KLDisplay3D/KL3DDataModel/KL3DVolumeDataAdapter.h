
#ifndef __KL3DDATAMODEL_KL3DVOLUMEDATAADAPTER__
#define __KL3DDATAMODEL_KL3DVOLUMEDATAADAPTER__

#include <string>
#include "KL3DDataModelMacro.h"
#include "KL3DVOctree.h"
//typedef long long int INT64
const int FloatByte_he = sizeof(float);
BEGIN_KLDISPLAY3D_NAMESPACE

/*!
\brief 体数据的状态信息
\note1 体数据优先存储顺序first-second-third
\note2 通过体数据三个方向上的采样点数目和体数据属性值的数据类型(float)
\<br>  可以计算出体数据大小（单位B），若在阈值范围内就作为小
\<br>  数据处理，否则作为大数据处理
*/
struct VolumeHeader
{
	//First方向的采样点数目
	int    numFirst;
	//Second方向的采样点数目
	int    numSecond;
	//Third方向的采样点数目
	int    numThird;
	
	//First方向的采样间隔
	double   interFirst;
	//Second方向的采样间隔
	double   interSecond;
	//Third方向的采样间隔
	double   interThird;

	//First方向的采样间隔的单位名称
	std::string   nameFirst;
	//Second方向的采样间隔的单位名称
	std::string   nameSecond;
	//Third方向的采样间隔的单位名称
	std::string   nameThird;

	//数据文件中属性值的最小值
	float    minVal;
	//数据文件中属性值的最大值
	float    maxVal;

	//第一个采样点的坐标
	double   origin[3];

	//此数组中按照用户指定的顺序存放0、1、2，位置0表示x轴，位置1表示y轴，位置2表示z轴
	//例如，orderXYZ[3] = {1,2,0}second轴为x轴（CMP轴作为x轴），third轴为y轴（Record轴为y轴），first（sample）轴为z轴
	int orderXYZ[3];

	//数据的原文件名（数据可能大可能小）
	QString sourceFileName;

	//大数据转化成的对应的八叉树文件名
	QString octreeFileName;

	//通过数据阈值判断出数据是小还是大后做出标记
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

		nameFirst = "s"; //秒
		nameSecond = "m"; //米
		nameThird = "m"; //米

		minVal = 0.0f;
		maxVal = 0.0f;
		origin[0] = origin[1] = origin[2] = 0;
		//初始值，x轴对应sample轴
		orderXYZ[0] = 0;orderXYZ[1] = 1;orderXYZ[2] = 2;
		/*orderXYZ[0] = 1;orderXYZ[1] = 2;orderXYZ[2] = 0;*/
	
		sourceFileName = "";
		octreeFileName = "";
		
		DataSize = -1; //枚举类型从0开始编号，初始化使用负数
	}
	
};

/*!
\brief 三维体数据适配器
\note1 各应用课题在使用时要派生子类并实现纯虚函数
*/
class KL3DDATAMODEL_EXPORT KL3DVolumeDataAdapter
{
public:
	KL3DVolumeDataAdapter();
	virtual ~KL3DVolumeDataAdapter();

	//
	enum
	{
		WHOLE, //可以获取整个体
		PART   //只可以获取切片
	};
	int AdapterType;

	//用于标记数据是大数据还是小数据
	enum
	{
		BIG,
		SMALL
	};
	/*!
	\brief 给绘制类提供数据
	\note1 绘制类在需要体数据时调用此函数即可
	*/
	virtual bool excuteVolumeData();
	
	// 检查文件是否存在
	bool testFile(char *fileName);
	/*!
	\brief 给绘制类提供体数据基本信息
	*/
	struct VolumeHeader getVolumeHeader()
	{
		return m_volHeader;
	}

	/*!
	\brief 给绘制类提供存储体数据内存的指针
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
	\brief 给绘制类提供八叉树文件
	*/
	QString  getOctreeFileName()
	{
		return  m_octreeFileName;
	}
	void setOctreeFileName(QString fileName)
	{
		m_octreeFileName  = fileName;
	}
	//对外提供坐标系中各个方向的最大最小值
	//调用之前必须确保requestinformation（）已经调用
	void getVolumeBoundsXYZ(double* bounds);

protected:
	/*!
	\brief 获取数据文件基本信息及体数据的阈值
	\note1 把获取的这些信息存放到struct VolumeHeader m_volHeader中
	*/
	virtual bool requestInformation() = 0;

	/*!
	\brief 按照存储顺序读取一段数据
	\param char* 存储数据的内存空间指针
	\<br>  length 这段数据的长度，单位B
	\<br>  pos 读取的开始位置，单位B
	\note1 pos=0，表示从第一个采样点开始读
	\note2 WHOLE == AdapterType时才需要实现此函数
	*/
	virtual bool read( char*, long long int length, long long int pos ) = 0;

	/*!
	\brief 读取切片数据
	\param int slice - 取值0、1、2，分别代表first-second-third轴
	\param int pos - 取值0 - num，表示第几个切片
	\param char* - 存储切片数据
	*/
	virtual bool readSliceData( int slice, int pos, char* ) = 0;

	/*!
	\brief 获取指定位置的属性值
	\param int x, int y, int z 三个方向上的采样点索引
	\note1 索引从0开始编号
	*/
	//virtual float getValue( int x, int y, int z ) = 0;

	/*
	\ brief  对体数据的阈值进行自定义的设定(数据头中阈值会重置)
	\ return 
	\ param  	long long int
	*/
	void setVolumeDataThreshold(long long int);
	
	

protected:
	//存储体数据的基本信息
	struct VolumeHeader   m_volHeader;

	//体数据是小数据时读入内存后的位置指针
	float    *m_volumeData;
	bool	isRead;
	//体数据的阈值，超过此值当做大数据
	long long int  m_volumeDataThreshold;

	
	QString m_sourceFileName;
	QString m_octreeFileName;
};


//const int VolMax = 209715200; //209715200B = 200MB;

END_KLDISPLAY3D_NAMESPACE

#endif //__KL3DDATAMODEL_KL3DVOLUMEDATAADAPTER__

