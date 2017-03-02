

#ifndef __KL3DDATAMODEL_KL3DVTESTSGYDATAADAPTER_H__
#define __KL3DDATAMODEL_KL3DVTESTSGYDATAADAPTER_H__

#include "KL3DVolumeDataAdapter.h"

BEGIN_KLDISPLAY3D_NAMESPACE

class KL3DDATAMODEL_EXPORT KL3DVTestSgyDataAdapter : public KL3DVolumeDataAdapter
{
public:
	KL3DVTestSgyDataAdapter(QString sourceFileName);
	//KL3DVTestSgyDataAdapter( std::stringoctreeFileName);
	virtual ~KL3DVTestSgyDataAdapter();

	/*void setSourceFilename(std::string sourceFileName)
	{
		m_sourceFileName = sourceFileName;
	}*/
	//bool excuteVolumeData();

protected:
	bool requestInformation();
	bool read(char*, long long int length, long long int pos);
	bool readSliceData(int slice, int pos, char*){return true;}
	
	/*
	\ brief     通过原文件名查找对应八叉树文件名
	\ return    bool
	\ param  	sourceFileNames
	*/
	bool Find_Files(std::string sourceFileNames);


};

END_KLDISPLAY3D_NAMESPACE

#endif //__KL3DDATAMODEL_KL3DVTESTSGYDATAADAPTER_H__