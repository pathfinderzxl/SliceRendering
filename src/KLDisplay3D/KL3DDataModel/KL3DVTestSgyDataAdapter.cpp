

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>


#include "KL3DVTestSgyDataAdapter.h"
	//using namespace std;
	BEGIN_KLDISPLAY3D_NAMESPACE

	KL3DVTestSgyDataAdapter::KL3DVTestSgyDataAdapter(QString sourceFileName)
{
	this->m_sourceFileName =  sourceFileName;

	m_volHeader.sourceFileName =  sourceFileName;
	setVolumeDataThreshold(419430400);

}
//KL3DVTestSgyDataAdapter::KL3DVTestSgyDataAdapter( char *octreeFileName)
//{
//	m_octreeFileName = (char *)octreeFileName;
//
//	m_volHeader.octreeFileName = (char *)octreeFileName;
//	setVolumeDataThreshold(419430400);
//
//}

KL3DVTestSgyDataAdapter::~KL3DVTestSgyDataAdapter()
{
}

bool KL3DVTestSgyDataAdapter::requestInformation()
{
	std::ifstream fin;
	std::ios::sync_with_stdio(false);
	fin.open(m_sourceFileName.toStdString(), std::ios::in | std::ios::binary);
	if (!fin.is_open())
		return false;

	unsigned short int tmp;
	fin.seekg(3220);
	fin.read((char *)&tmp, 2);
	
	m_volHeader.numFirst = tmp;

	int lenTrace = m_volHeader.numFirst * 4 + 240;

	int minSecond, maxSecond, minThird, maxThird;
	fin.seekg(3608);
	fin.read((char *)&minThird, 4);
	fin.seekg(3620);
	fin.read((char *)&minSecond, 4);
	fin.seekg(8 - lenTrace, std::ios::end);
	fin.read((char *)&maxThird, 4);
	fin.seekg(20 - lenTrace, std::ios::end);
	fin.read((char *)&maxSecond, 4);
	m_volHeader.numThird = maxThird - minThird + 1;
	m_volHeader.numSecond = maxSecond - minSecond + 1;;
	//当是大数据时需要对数据大小进行区分以及原文件文件名赋值，
	//并查找八叉树文件，若没有值为空。
	long long int numSam = m_volHeader.numFirst * m_volHeader.numSecond *m_volHeader.numThird;
	long long int volumeSize = sizeof(float) * numSam;

	if (volumeSize <= m_volumeDataThreshold )
	{	
		m_volHeader.DataSize = SMALL;
	}

	if (volumeSize > m_volumeDataThreshold )
	{
		//大数据
		m_volHeader.DataSize = BIG;
		//先在原数据所在文件夹下查找八叉树文件，若存在将八叉树文件读入，反之则仍然为空
		if(Find_Files(m_volHeader.sourceFileName.toStdString()))
		{
			std::cout<<"存在对应八叉树文件::::::"<<m_volHeader.octreeFileName.toStdString()<<std::endl;
			std::cout<<"存在对应八叉树文件"<<std::endl;
			
		}

	}

	fin.close();

	return true;
}

bool KL3DVTestSgyDataAdapter::read(char *dst, long long int length, long long int pos)
{
	char *tmp = dst;
	std::fstream fin;
	fin.open(m_sourceFileName.toStdString(), std::ios::in | std::ios::binary);
	if (!fin.is_open())
		return false;

	for (int i = 0; i < m_volHeader.numSecond * m_volHeader.numThird; ++i)
	{
		fin.seekg(3840 + (240 + m_volHeader.numFirst * 4) * i);
		fin.read(tmp, m_volHeader.numFirst * 4);
		tmp += m_volHeader.numFirst * 4;
	}

	fin.close();

	return true;
}

bool KL3DVTestSgyDataAdapter::Find_Files(std::string sourceFilePaths)

{
	

	WIN32_FIND_DATAA wfd;
	char filename[256]="";
	
	char drive[_MAX_DRIVE];  
	char dir[_MAX_DIR];  
	char fname[_MAX_FNAME];  
	char ext[_MAX_EXT];

	_splitpath( sourceFilePaths.c_str(), drive, dir, fname, ext );  
	std::string sourcefileName;
	sourcefileName = strcat(fname,ext);

	_splitpath( sourceFilePaths.c_str(), drive, dir, fname, ext ); 
	std::string octreefileName;
	octreefileName = strcat(fname,".dat");

	_splitpath( sourceFilePaths.c_str(), drive, dir, fname, ext );  
	printf( "Path extracted with _splitpath:\n" );  
	printf( "  Drive: %s\n", drive );  
	printf( "  Dir: %s\n", dir );  
	
	
	strcat(filename,drive);
	strcat(filename,dir);
	strcat(filename,"\\*.*");

	std::string ss = filename;

	HANDLE hFile = FindFirstFileA(ss.c_str(),&wfd);

	if(INVALID_HANDLE_VALUE == hFile)

	{
		printf("搜索失败!");
		return false;

	}

	while(::FindNextFileA(hFile, &wfd))
	{
		// 过虑.和..
		if (strcmp(wfd.cFileName, sourcefileName.c_str()) == 0 )
		{
			std::cout<<wfd.cFileName<<std::endl;

			char filename2[256] ="";
			strcat(filename2,drive);
			strcat(filename2,dir);
			
			
			strcat(filename2,octreefileName.c_str());
			
			//这里使用strcpy而不是直接赋值，是为了防止目标指向来及内容
			//防止strcpy的目标需要是一个给定长度char[]
			if (!testFile(filename2))
			{
				std::cout<<"the octree file cannot open "<<std::endl;
					return false;
			}
				
			char octreeTemp[256];
			strcpy(octreeTemp,filename2);
			
			m_volHeader.octreeFileName = octreeTemp;
			
			return true;
		}
		continue;
	}
	return false;

}

END_KLDISPLAY3D_NAMESPACE
