/********************************************************

********************************************************/
#include "KL3DVOctree.h"
#include <direct.h>
#include "qfile.h"
#include "qdatastream.h"
#include "QString.h"
#include <math.h>
#include <iomanip>
//#include <Windows.h>

#include <iostream>
#include <fstream>

BEGIN_KLDISPLAY3D_NAMESPACE

Octree::Octree(void)
{	
	_rootNode = NULL;
	_octreeFileName = "";
	_notEmptyNode = 0;
	_octreeSize = 0;
	_minSample = _maxSample = -1;
	_minCMP = _maxCMP = -1;
	_minRecord = _maxRecord = -1;
}

//Octree::Octree(const char* fileName)
//{
//	_fileName = fileName;
//	SegYInfo segYInfo(fileName);
//	_rootNode     = NULL;
//	_notEmptyNode = 0;
//	_minSample    = 1;
//	_maxSample    = segYInfo.GetSamplesPerTrace();
//	_minCMP       = segYInfo.GetMinCMP();
//	_maxCMP       = segYInfo.GetMaxCMP();
//	_minRecord    = segYInfo.GetMinRecord();
//	_maxRecord    = segYInfo.GetMaxRecord();
//	_octreeSize   = GetOctreeSize(_maxSample - _minSample, _maxCMP - _minCMP, _maxRecord - _minRecord);
//}

Octree::Octree(int minSample, int maxSample, int minCMP, int maxCMP, int minRecord, int maxRecord)
{
	_rootNode = NULL;
	_octreeFileName = "";
	_notEmptyNode = 0;
	_minSample = minSample;
	_maxSample = maxSample;
	_minCMP = minCMP;
	_maxCMP = maxCMP;
	_minRecord = minRecord;
	_maxRecord = maxRecord;
	_octreeSize = GetOctreeSize(_maxSample - _minSample, _maxCMP - _minCMP, _maxRecord - _minRecord);
}

Octree::Octree(int range[6])
{
	_rootNode = NULL;
	_octreeFileName = "";
	_notEmptyNode = 0;
	_minSample = range[0];
	_maxSample = range[1];
	_minCMP = range[2];
	_maxCMP = range[3];
	_minRecord = range[4];
	_maxRecord = range[5];
	_octreeSize = GetOctreeSize(_maxSample - _minSample, _maxCMP - _minCMP, _maxRecord - _minRecord);
}

Octree::Octree(std::string fileName)
{
	int bounds[6];
	std::fstream fs;

	_octreeFileName = fileName;
	// 数据文件名从KL3DVolume得到
	fs.open(fileName, std::ios::binary | std::ios::in);
	std::ios::sync_with_stdio(false);
	fs.seekg(std::ios::beg);
	fs.read((char *)bounds, 6 * sizeof(int));
	fs.close();

	_rootNode = NULL;
	//_octreeFileName = "";
	_notEmptyNode = 0;
	_minSample = bounds[0];
	_maxSample = bounds[1];
	_minCMP = bounds[2];
	_maxCMP = bounds[3];
	_minRecord = bounds[4];
	_maxRecord = bounds[5];
	_octreeSize = GetOctreeSize(_maxSample - _minSample, _maxCMP - _minCMP, _maxRecord - _minRecord);
}

Octree::~Octree(void)
{
	Delete(_rootNode);
}

int Max(int x, int y)
{
	return x >= y ? x : y;
}

// 满八叉树最底层采样间隔数，对原始地震体进行扩充，得到正方体
int Octree::GetOctreeSize(int sample, int CMP, int record)
{
	int size = BLOCKSIZE - 1; // 单个体块最大采样间隔数
	int maxSize = Max(Max(sample, CMP), record);

	while (size <= maxSize)
		size *= 2;

	return size;
}

void Octree::CreateOctree()
{
	m_depth = (int)(log((double)_octreeSize / (BLOCKSIZE - 1)) / log(2.0));
	//DWORD start_time = GetTickCount();
	CreateOctreeNode(_rootNode, m_depth, 0, _octreeSize,
						_minSample, _maxSample, 
						_minCMP   , _maxCMP   ,
						_minRecord, _maxRecord);
	//DWORD end_time = GetTickCount();
	//std::cout<<end_time - start_time<<"ms!"<<std::endl;
	SetTileInfo();
}

void  Octree::CreateOctreeNode(OctreeNode *&node, int depth, int level, int tileSize,
								int minSample, int maxSample,
								int minCMP   , int maxCMP   ,
								int minRecord, int maxRecord)
{
	if (depth >= 0)
	{
		node = new OctreeNode();
		++_notEmptyNode;

		node->_tileStep = tileSize / (BLOCKSIZE - 1);
		node->_level = level;

		node->_minSample = minSample;// > this->_minSample ? minSample - node->_tileStep : minSample;
		node->_maxSample = maxSample;// < this->_maxSample ? maxSample + 1 : maxSample;
		node->_minCMP = minCMP;// > this->_minCMP ? minCMP - node->_tileStep : minCMP;
		node->_maxCMP = maxCMP;// < this->_maxCMP ? maxCMP + 1 : maxCMP;
		node->_minRecord = minRecord;// > this->_minRecord ? minRecord - node->_tileStep : minRecord;
		node->_maxRecord = maxRecord;// < this->_maxRecord ? maxRecord + 1 : maxRecord;

		node->_diagnose = sqrt((double)((node->_maxSample - node->_minSample) * (node->_maxSample - node->_minSample)
									  + (node->_maxCMP - node->_minCMP ) * (node->_maxCMP - node->_minCMP )
								      + (node->_maxRecord - node->_minRecord) * (node->_maxRecord - node->_minRecord)));

		node->_center[0] = (node->_minSample + node->_maxSample)/ 2.0;
		node->_center[1] = (node->_minCMP + node->_maxCMP )/ 2.0;
		node->_center[2] = (node->_minRecord + node->_maxRecord)/ 2.0;

		//---xinbing-s
		double volBounds[6];
		volBounds[0] = node->_minSample; volBounds[1] = node->_maxSample;
		volBounds[2] = node->_minCMP; volBounds[3] = node->_maxCMP;
		volBounds[4] = node->_minRecord; volBounds[5] = node->_maxRecord;
		int idx = 0;
		for ( int k = 0; k < 2; k++ )
		{
			for ( int j = 0; j < 2; j++ )
			{
				for ( int i = 0; i < 2; i++ )
				{
					node->m_vertices[idx][2] = volBounds[4+k];
					node->m_vertices[idx][1] = volBounds[2+j];
					node->m_vertices[idx][0] = volBounds[i];
					idx++;
				}
			}
		}
		//---xinbing-e

		node->_sampleNum = ceil((node->_maxSample - node->_minSample) / (double)node->_tileStep) + 1;
		node->_CMPNum = ceil((node->_maxCMP - node->_minCMP ) / (double)node->_tileStep) + 1;
		node->_recordNum = ceil((node->_maxRecord - node->_minRecord) / (double)node->_tileStep) + 1;

		node->_sampleDistance[0] = (node->_maxSample - node->_minSample) / (double)(node->_sampleNum - 1);
		node->_sampleDistance[1] = (node->_maxCMP - node->_minCMP) / (double)(node->_CMPNum - 1);
		node->_sampleDistance[2] = (node->_maxRecord - node->_minRecord) / (double)(node->_recordNum - 1);

		if (depth > 0)
		{
			--depth;
			++level;
			int sampleSize = maxSample - minSample;	
			int CMPSize = maxCMP - minCMP;
			int recordSize = maxRecord - minRecord;
			tileSize /= 2;

			if (sampleSize <= tileSize && CMPSize <= tileSize && recordSize <= tileSize)
			{
				CreateOctreeNode(node->_children[0], depth, level, tileSize,
									minSample, maxSample,
									minCMP, maxCMP,
									minRecord, maxRecord);
				++node->_notEmptyChild;
			}
			else if (sampleSize > tileSize && CMPSize <= tileSize && recordSize <= tileSize)
			{
				CreateOctreeNode(node->_children[0], depth, level, tileSize,
									minSample, minSample + tileSize,
									minCMP, maxCMP,
									minRecord, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[1], depth, level, tileSize,
									minSample + tileSize, maxSample,
									minCMP, maxCMP,
									minRecord, maxRecord);
				++node->_notEmptyChild;
			}
			else if (sampleSize <= tileSize && CMPSize > tileSize && recordSize <= tileSize)
			{
				CreateOctreeNode(node->_children[0], depth, level, tileSize,
									minSample, maxSample,
									minCMP, minCMP + tileSize,
									minRecord, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[2], depth, level, tileSize,
									minSample, maxSample,
									minCMP + tileSize, maxCMP   ,
									minRecord, maxRecord);
				++node->_notEmptyChild;
			}
			else if (sampleSize <= tileSize && CMPSize <= tileSize && recordSize > tileSize)
			{
				CreateOctreeNode(node->_children[0], depth, level, tileSize,
									minSample, maxSample,
									minCMP, maxCMP,
									minRecord, minRecord + tileSize);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[4], depth, level, tileSize,
									minSample, maxSample,
									minCMP, maxCMP,
									minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
			}
			else if (sampleSize > tileSize && CMPSize > tileSize && recordSize <= tileSize)
			{
				CreateOctreeNode(node->_children[0], depth, level, tileSize,
									minSample, minSample + tileSize,
									minCMP, minCMP    + tileSize,
									minRecord, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[1], depth, level, tileSize,
									minSample + tileSize, maxSample,
									minCMP, minCMP + tileSize,
									minRecord, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[2], depth, level, tileSize,
									minSample, minSample + tileSize,
									minCMP + tileSize, maxCMP,
									minRecord, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[3], depth, level, tileSize,
									minSample + tileSize, maxSample,
									minCMP + tileSize, maxCMP,
									minRecord, maxRecord);
				++node->_notEmptyChild;
			}			
			else if (sampleSize > tileSize && CMPSize <= tileSize && recordSize > tileSize)
			{
				CreateOctreeNode(node->_children[0], depth, level, tileSize,
									minSample, minSample + tileSize,
									minCMP, maxCMP,
									minRecord, minRecord + tileSize);
				++node->_notEmptyChild ;
				CreateOctreeNode(node->_children[1], depth, level, tileSize,
									minSample + tileSize, maxSample,
									minCMP, maxCMP,
									minRecord, minRecord + tileSize);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[4], depth, level, tileSize,
									minSample, minSample + tileSize,
									minCMP, maxCMP,
									minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[5], depth, level, tileSize,
									minSample + tileSize, maxSample,
									minCMP, maxCMP   ,
									minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
			}
			else if (sampleSize <= tileSize	&& CMPSize > tileSize && recordSize > tileSize)
			{				
				CreateOctreeNode(node->_children[0], depth, level, tileSize,
									minSample, maxSample,
									minCMP, minCMP + tileSize,
									minRecord, minRecord + tileSize);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[2], depth, level, tileSize,
					minSample, maxSample,
					minCMP + tileSize, maxCMP,
					minRecord, minRecord + tileSize);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[4], depth, level, tileSize,
									minSample, maxSample,
									minCMP, minCMP + tileSize,
									minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[6], depth, level, tileSize,
									minSample, maxSample,
									minCMP + tileSize, maxCMP   ,
									minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
			}
			else if (sampleSize > tileSize && CMPSize > tileSize && recordSize > tileSize)
			{
				CreateOctreeNode(node->_children[0], depth , level, tileSize,
									minSample, minSample + tileSize,
									minCMP, minCMP+ tileSize,
									minRecord, minRecord + tileSize);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[1], depth, level, tileSize,
					minSample + tileSize, maxSample,
					minCMP, minCMP    + tileSize,
					minRecord, minRecord + tileSize);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[2], depth, level, tileSize,
					minSample, minSample + tileSize,
					minCMP + tileSize, maxCMP,
					minRecord, minRecord + tileSize);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[3], depth, level, tileSize,
					minSample + tileSize, maxSample,
					minCMP + tileSize, maxCMP,
					minRecord, minRecord + tileSize);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[4], depth, level, tileSize,
									minSample, minSample + tileSize,
									minCMP, minCMP + tileSize,
									minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[5], depth, level, tileSize,
					minSample + tileSize, maxSample,
					minCMP, minCMP + tileSize,
					minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[6], depth, level, tileSize,
									minSample, minSample + tileSize,
									minCMP + tileSize, maxCMP,
									minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
				CreateOctreeNode(node->_children[7], depth, level, tileSize,
									minSample + tileSize, maxSample,
									minCMP + tileSize, maxCMP   ,
									minRecord + tileSize, maxRecord);
				++node->_notEmptyChild;
			}
		}
		if (level == 0)
			node->_parent = NULL;
		for (int i = 0; i < 8; ++i)
			if (node->_children[i])
				node->_children[i]->_parent = node;
	}
}

void Octree::SetTileInfo()
{
	std::list<OctreeNode*> OctreeList;
	OctreeNode* p;
	OctreeList.push_back(_rootNode);
	p = OctreeList.front();
	int location = 6, capacity = 0, tileID = 0;
	while (p)
	{
		//p->_tileID = tileID;
		location += capacity;
		p->_location = location;
		capacity = p->_sampleNum * p->_CMPNum * p->_recordNum;

		//p->m_bytes = capacity * 4;

		/*std::cout
		<<p->_location<<" "<<p->_notEmptyChild<<" "<<p->_tileStep<<" "			
		<<p->_minSample<<" "<<p->_minCMP<<" "<<p->_minRecord<<" "
		<<p->_sampleNum<<" "<<p->_CMPNum<<" "<<p->_recordNum<<" "
		<<std::setprecision(5)
		<<p->_sampleDistance[0]<<" "<<p->_sampleDistance[1]<<" "<<p->_sampleDistance[2]<<" "
		<<p->_center[0]<<" "<<p->_center[1]<<" "<<p->_center[2]<<" "<<std::endl;*/

		for (int i = 0; i < 8; ++i)
			if (p->_children[i])
				OctreeList.push_back(p->_children[i]);

		OctreeList.erase(OctreeList.begin());
		if (!OctreeList.empty())
			p = OctreeList.front();
		else 
			break ;
	}
}

void Octree::ReadWriteData()
{
	
	//std::cout<<"reading and writing data"<<std::endl;
	//DWORD start_time = GetTickCount();
	QFile fileIn(_sourceFileName.c_str());
	QDataStream in(&fileIn);
	in.setFloatingPointPrecision(QDataStream::SinglePrecision);
	in.setByteOrder(QDataStream::LittleEndian);
	if (!fileIn.open(QIODevice::ReadOnly))
	{
		//std::cout<<"can't open file : " + _fileName.toStdString()<<std::endl;
		return ;
	}
	QFile fileOut(_octreeFileName.c_str());
	fileOut.close();
	QDataStream out(&fileOut);
	out.setFloatingPointPrecision(QDataStream::SinglePrecision);
	out.setByteOrder(QDataStream::LittleEndian);
	if(!fileOut.open(QIODevice::WriteOnly))
	{
		//std::cout<<"can't open file " + fileOut.fileName().toStdString()<<std::endl;
		return ;
	}

	out<<_minSample<<_maxSample<<_minCMP<<_maxCMP<<_minRecord<<_maxRecord;

	float  value;
	double wSample, wCMP, wRecord;
	double fSample, fCMP, fRecord;
	double sampleRate[3];
	int    sample, CMP, record;
	float  A, B, C, D, E, F, G, H;
	
	int index1, index2, index3;
	int lengthOfTrace = 240 + 4 * _maxSample;
	int lengthOfRecord = (_maxCMP - _minCMP + 1) * lengthOfTrace;
	
	std::list<OctreeNode*> octreeList;
	OctreeNode* p;
	octreeList.push_back(_rootNode);
	p = octreeList.front();
	while (p)
	{		
		sampleRate[0] = p->_sampleDistance[0] / static_cast<double>(p->_tileStep);
		sampleRate[1] = p->_sampleDistance[1] / static_cast<double>(p->_tileStep);
		sampleRate[2] = p->_sampleDistance[2] / static_cast<double>(p->_tileStep);

		index1  = (p->_minRecord - _minRecord) * lengthOfRecord
				+ (p->_minCMP    - _minCMP   ) * lengthOfTrace
				+ (p->_minSample - _minSample) * 4 + 3840;

		if (fabs(p->_tileStep - p->_sampleDistance[0]) <= 0.0001
			&& fabs(p->_tileStep - p->_sampleDistance[1]) <= 0.0001
			&& fabs(p->_tileStep - p->_sampleDistance[2]) <= 0.0001)
		{
			for (int i = 0; i < p->_recordNum; ++i)
			{
				index2 = index1;
				for (int j = 0; j < p->_CMPNum; ++j)
				{
					index3 = index2;
					for (int k = 0; k < p->_sampleNum; ++k)
					{
						fileIn.seek(index3);
						in>>value;
						out<<value;
						index3 += 4 * p->_tileStep;
					}
					index2 += p->_tileStep * lengthOfTrace;
				}
				index1 += p->_tileStep * lengthOfRecord;
			}
		}
		else
		{
			for (int i = 0; i < p->_recordNum; ++i)
			{
				fRecord = i * sampleRate[2];
				//fz = (fz >= inputDimensions[2]-1)?(inputDimensions[2]-1.001):(fz);
				record  = floor(fRecord);
				wRecord = fRecord - record;
				for (int j = 0; j < p->_CMPNum; ++j)
				{
					fCMP = j * sampleRate[1];
					//fy = (fy >= inputDimensions[1]-1)?(inputDimensions[1]-1.001):(fy);
					CMP  = floor(fCMP);
					wCMP = fCMP - CMP;
					for (int k = 0; k < p->_sampleNum; ++k)
					{
						fSample = k * sampleRate[0];
						//fx = (fx >= inputDimensions[0]-1)?(inputDimensions[0]-1.001):(fx);
						sample  = floor(fSample);
						wSample = fSample - sample;
						index3 = index1 + record * p->_tileStep * lengthOfRecord
									+ CMP * p->_tileStep * lengthOfTrace
									+ sample * p->_tileStep * 4;
						fileIn.seek(index3);	
						in>>A;
						fileIn.seek(index3 + 4 * p->_tileStep);
						in>>B;
						fileIn.seek(index3 + lengthOfTrace);
						in>>C;
						fileIn.seek(index3 + 4 * p->_tileStep + lengthOfTrace);
						in>>D;

						index3 += lengthOfRecord;
						fileIn.seek(index3);
						in>>E;
						fileIn.seek(index3 + 4 * p->_tileStep);
						in>>F;
						fileIn.seek(index3 + lengthOfTrace);
						in>>G;
						fileIn.seek(index3 + 4 * p->_tileStep + lengthOfTrace);
						in>>H;

						value = 
								(1.0 - wRecord) * (1.0 - wCMP) * (1.0 - wSample) * A +
								(1.0 - wRecord) * (1.0 - wCMP) *        wSample  * B +
								(1.0 - wRecord) *        wCMP  * (1.0 - wSample) * C +
								(1.0 - wRecord) *        wCMP  *        wSample  * D +
									   wRecord  * (1.0 - wCMP) * (1.0 - wSample) * E +
									   wRecord  * (1.0 - wCMP) *        wSample  * F +
									   wRecord  * (      wCMP) * (1.0 - wSample) * G +
									   wRecord  * (      wCMP) *        wSample  * H;
						out<<value;
					}
				}
			}
		}
		for (int i = 0; i < 8; ++i)
			if (p->_children[i])
				octreeList.push_back(p->_children[i]);
		
		octreeList.erase(octreeList.begin());

		if (!octreeList.empty())
			p = octreeList.front();			
		else 
			break ;
	}
	fileOut.close();
	fileIn.close();
	//DWORD end_time = GetTickCount();
	//std::cout<<end_time - start_time<<"ms!"<<std::endl;
	//std::cout<<" end read and write data"<<std::endl;
}

void Octree::Delete(OctreeNode *p)
{
	if (p)
	{
		for (int i = 0; i < 8; ++i)
			Delete(p->_children[i]);
		delete p;
	}

}
END_KLDISPLAY3D_NAMESPACE
