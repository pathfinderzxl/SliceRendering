
#include "float.h"
#include "qfile.h"
#include "qdatastream.h"
#include "KL3DVolumeDataManager.h"
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Vec3d>
#include <osg/Texture3D>

#include "KL3DDataModel/KL3DVOctree.h"


#include <omp.h>

BEGIN_KLDISPLAY3D_NAMESPACE
using namespace std;
KL3DVolumeDataManager::KL3DVolumeDataManager()
{
	m_volumeData = NULL;
	m_octree = NULL;
	m_cache = NULL;
	m_cacheSize = 200 ;
	//m_predataAdapter = NULL;
	m_preOctreeFileName = "";
	m_preVolumeData = NULL;
	m_dataAdapter = NULL;
	//m_texObjectCache = NULL;

}	
KL3DVolumeDataManager::~KL3DVolumeDataManager()
{
	if (m_octree)
		m_octree->~KL3DVOctree();////////////////////////////////////////////////////////////////
		//m_octree->~KLCreateOctree();
	if(m_cache)
		m_cache->~KL3DVolumeBlockCache();
	if(m_volumeData)
		delete[] m_volumeData;
}
bool KL3DVolumeDataManager::initData(KL3DVolumeDataAdapter* dataAdapter)
{
	if ( NULL == dataAdapter)
		return false;
	m_dataAdapter = dataAdapter;
	m_displayList->generateBasicSliceNodeList(m_octree, 2);

	OnDataChanged();
	return true;
}
bool KL3DVolumeDataManager::dataHasChanged()
{
	if (!m_preVolumeData && "" == m_preOctreeFileName)
		return true;

	if (m_preVolumeData != m_dataAdapter->getVolumeData())//前后两个小数据、NULL和小数据
		return true;
		
	if (m_preOctreeFileName!= m_dataAdapter->getOctreeFileName())//前后是两个大数据、NULL和大数据
		return true;

	if (m_preVolumeData && ""!=m_dataAdapter->getOctreeFileName())//前小后大
		return true;

	if (""!= m_preOctreeFileName && m_dataAdapter->getVolumeData())//前大后小
		return true;

	
	return false;
}
bool KL3DVolumeDataManager::OnDataChanged()
{
	if (!m_dataAdapter)
		return false;

	if (!m_dataAdapter->excuteVolumeData())
		return false;

	if(dataHasChanged())//数据变化的条件############################################################
	{
		if( m_preVolumeData)
			resetVolumeData();
		
		if(m_cache)
			resetCache();

		if(m_octree)
			resetOctree();

		if (m_dataAdapter->getVolumeData())//小数据
		{
			m_volumeData = m_dataAdapter->getVolumeData();
		}
		else if( "" != m_dataAdapter->getOctreeFileName())//八叉树文件不空，大数据
		{			
			m_octree = new KL3DVOctree(m_dataAdapter->getOctreeFileName());
			
			m_octree->recoverOctree();//根据八叉树文件重新创建八叉树，构建内存缓存

			m_octree->initialEntropy("E:/volData/pwave2000x38G.entropy");
			//m_octree->initialEntropy("F:\\PostGraduate\\Projects\\KLSeis\\vol_dat\\new_oct\\entropy\\pwave1000x4G.entropy");			
			//m_octree->initialEntropy("F:\\PostGraduate\\Projects\\KLSeis\\vol_dat\\new_oct\\entropy\\pwave500x500M.entropy");

			buildCache(m_octree,m_cacheSize);
		}
		//m_predataAdapter = m_dataAdapter;//////////////////////////////////////////////////////////////
		{
			if (m_dataAdapter->getVolumeData())
				m_preVolumeData = m_dataAdapter->getVolumeData();
			
			if( m_octree &&"" != m_dataAdapter->getOctreeFileName())
				m_preOctreeFileName = m_dataAdapter->getOctreeFileName();
		}
	
	return true;
}
void KL3DVolumeDataManager::buildCache(KL3DVOctree * octree, int cachesize)
{

	m_cache = new KL3DVolumeBlockCache();
	m_cache->setCacheSize(cachesize);
	m_cache->initCache(octree);

	//m_texObjectCache = new TextureObjectCache(this);
	//m_texObjectCache->setDataManager(this);
}
void KL3DVolumeDataManager::resetOctree()
{
	if(m_octree)
	{
		m_octree->~KL3DVOctree();
		//m_octree->~KLCreateOctree();
	}
	m_octree = NULL;
}
void KL3DVolumeDataManager::resetCache()
{
	if(m_cache)
	{
		m_cache->~KL3DVolumeBlockCache();
	}
	m_cache = NULL;
}
void KL3DVolumeDataManager::resetVolumeData()
{
	if(m_volumeData)
	{
		delete[] m_volumeData;
	}
	m_volumeData = NULL;

}

osg::ref_ptr<osg::StateSet> KL3DVolumeDataManager::createState(int dim[3], float *data)
{
	int blockSize = m_octree->getBlockSize();
	float * dataCache=new float[dim[0]*dim[1]*dim[2]];
	float* pdataCache=dataCache;
	bool isFull = dim[0] == blockSize && dim[1] == blockSize && dim[2] == blockSize;
	if (!isFull)
	{
		for (int k = 0; k < blockSize; k++)
		{
			for (int j = 0; j < blockSize; j++)
			{
				for (int i = 0; i < blockSize; i++)
				{
					if (-2 != *data)
					{
						*pdataCache=*data;
						pdataCache++;
					} 
					data++;
				}
			}
		}
		pdataCache = dataCache;
	}
	else
	{
		pdataCache = data;
	}

	const int *order = m_volHeader.orderXYZ;

	//转换后的 体块三个方向的采样点数
	int dimtrans[3] = {dim[ order[0]],dim[ order[1]],dim[ order[2]]};

	if (dim[ order[0]] == 1 &&dim[ order[1]] == 1 && dim[ order[2]] == 1)
	{
		std::cout<<"hehe-error:KL3DVolumeNode.createState"<<std::endl;
		return NULL;
	}

	// 三维纹理数据
	osg::ref_ptr<osg::Image> image_3d = new osg::Image;
	image_3d->allocateImage(dimtrans[0], dimtrans[1], dimtrans[2], GL_RGBA, GL_FLOAT);
	image_3d->setInternalTextureFormat(GL_RGBA);
	image_3d->setOrigin(osg::Image::BOTTOM_LEFT);

	osg::Vec4 *ptr = (osg::Vec4 *)image_3d->data();

	float val;
	osg::Vec4 color;


	int podr[3],pos;

	int sliceSizeFS = dim[0] * dim[1];
	int sliceSizeXY = dimtrans[0]*dimtrans[1];

	int processNum = omp_get_num_procs();


	//#pragma omp parallel for private(podr, pos, val, color)

	for (podr[0] = 0; podr[0] < dimtrans[2]; ++podr[0])//z
	{
		for (podr[1] = 0; podr[1] < dimtrans[1]; ++podr[1])//y
		{
			for (podr[2] = 0; podr[2] < dimtrans[0]; ++podr[2])//x
			{
				//三个方向上的偏移累加
				int totalidxFST = 0;
				int totalidxXYZ = podr[0]*sliceSizeXY + podr[1] * dimtrans[0] + podr[2];

				for (int m = 0;m < 3;m++) 
				{
					if (order[m] == 2)
					{
						totalidxFST += podr[2-m] * sliceSizeFS;
					}
					else if (order[m] == 1)
					{
						totalidxFST += podr[2-m] * dim[0];
					}
					else
					{
						totalidxFST += podr[2-m];
					}
				}
				val = pdataCache[totalidxFST];
				if (val == -2)
				{
					cout<<val;
				}
				//pfn ? color = pfn(val) : color = osg::Vec4(1.0,1.0,1.0,1.0);

				val = val /4001.0;
				color = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);

				ptr[totalidxXYZ] = color;
				/*val = *data++;
				val = val /4000.0;
				*ptr++ = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);*/

			}
		}


	}
	//for (int i=0; i < processNum; ++i)
	//{
	//	for (podr[0] = 0; podr[0] < blockNum; ++podr[0])
	//	{
	//		pos = i * blockNum;
	//		for (podr[1] = 0; podr[1] < dimtrans[1]; ++podr[1])
	//		{
	//			for (podr[2] = 0; podr[2] < dimtrans[0]; ++podr[2])
	//			{
	//				//三个方向上的偏移累加
	//				int totalidxFST = 0;
	//				int totalidxXYZ = (podr[0] + pos)*sliceSizeXY + podr[1] * dimtrans[0] + podr[2];
	//				
	//				for (int m = 0;m < 3;m++) 
	//				{
	//					if (order[m] == 2)
	//					{
	//						totalidxFST += podr[2-m] * sliceSizeFS;
	//						if (m == 2)
	//							totalidxFST += pos * sliceSizeFS;
	//					}
	//					else if (order[m] == 1)
	//					{
	//						totalidxFST += podr[2-m] * dim[0];
	//						if(m == 2)
	//							totalidxFST += pos * dim[0];
	//					}
	//					else
	//					{
	//						totalidxFST += podr[2-m];
	//						if(m == 2)
	//							totalidxFST += pos;
	//					}
	//				}
	//				val = data[totalidxFST];

	//				//pfn ? color = pfn(val) : color = osg::Vec4(1.0,1.0,1.0,1.0);

	//				val = val /4001.0;
	//				color = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);

	//				ptr[totalidxXYZ] = color;
	//				/*val = *data++;
	//				val = val /4000.0;
	//				*ptr++ = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);*/
	//				
	//			}
	//		}

	//	}
	//}



	osg::ref_ptr<osg::Texture3D>texture3D = new osg::Texture3D;
	texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
	texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
	texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
	texture3D->setResizeNonPowerOfTwoHint(false);  // 纹理支持非2的整次幂
	texture3D->setImage(image_3d);

	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
	stateset->setTextureAttributeAndModes(0, texture3D);//,osg::StateAttribute::ON

	// 从stateset得到三维纹理属性,用于响应色表变化
	//osg::Texture3D *tex = static_cast<osg::Texture3D *>(stateset->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
	//osg::Image *image = tex->getImage();
	//setImageData(image, data);
	osg::ref_ptr<osg::PolygonMode> polygonMode = new osg::PolygonMode();
	polygonMode->setMode( osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
	stateset->setAttribute( polygonMode.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON );
	return stateset;
}


osg::ref_ptr<osg::Texture3D> KL3DVolumeDataManager::createTexture(int dim[3], float *data)
{
	int blockSize = m_octree->getBlockSize();
	float * dataCache=new float[dim[0]*dim[1]*dim[2]];
	float* pdataCache=dataCache;
	bool isFull = dim[0] == blockSize && dim[1] == blockSize && dim[2] == blockSize;
	if (!isFull)
	{
		for (int k = 0; k < blockSize; k++)
		{
			for (int j = 0; j < blockSize; j++)
			{
				for (int i = 0; i < blockSize; i++)
				{
					if (-2 != *data)
					{
						*pdataCache=*data;
						pdataCache++;
					} 
					data++;
				}
			}
		}
		pdataCache = dataCache;
	}
	else
	{
		pdataCache = data;
	}

	const int *order = m_volHeader.orderXYZ;

	//转换后的 体块三个方向的采样点数
	int dimtrans[3] = {dim[ order[0]],dim[ order[1]],dim[ order[2]]};

	if (dim[ order[0]] == 1 &&dim[ order[1]] == 1 && dim[ order[2]] == 1)
	{
		std::cout<<"hehe-error:KL3DVolumeNode.createState"<<std::endl;
		return NULL;
	}

	// 三维纹理数据
	osg::ref_ptr<osg::Image> image_3d = new osg::Image;
	image_3d->allocateImage(dimtrans[0], dimtrans[1], dimtrans[2], GL_RGBA, GL_FLOAT);
	image_3d->setInternalTextureFormat(GL_RGBA);
	image_3d->setOrigin(osg::Image::BOTTOM_LEFT);

	osg::Vec4 *ptr = (osg::Vec4 *)image_3d->data();

	float val;
	osg::Vec4 color;


	int podr[3],pos;

	int sliceSizeFS = dim[0] * dim[1];
	int sliceSizeXY = dimtrans[0]*dimtrans[1];

	int processNum = omp_get_num_procs();


	//#pragma omp parallel for private(podr, pos, val, color)

	for (podr[0] = 0; podr[0] < dimtrans[2]; ++podr[0])//z
	{
		for (podr[1] = 0; podr[1] < dimtrans[1]; ++podr[1])//y
		{
			for (podr[2] = 0; podr[2] < dimtrans[0]; ++podr[2])//x
			{
				//三个方向上的偏移累加
				int totalidxFST = 0;
				int totalidxXYZ = podr[0]*sliceSizeXY + podr[1] * dimtrans[0] + podr[2];

				for (int m = 0;m < 3;m++) 
				{
					if (order[m] == 2)
					{
						totalidxFST += podr[2-m] * sliceSizeFS;
					}
					else if (order[m] == 1)
					{
						totalidxFST += podr[2-m] * dim[0];
					}
					else
					{
						totalidxFST += podr[2-m];
					}
				}
				val = pdataCache[totalidxFST];
				if (val == -2)
				{
					cout<<val;
				}

				val = val /4001.0;
				color = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);

				ptr[totalidxXYZ] = color;

			}
		}


	}
	//for (int i=0; i < processNum; ++i)
	//{
	//	for (podr[0] = 0; podr[0] < blockNum; ++podr[0])
	//	{
	//		pos = i * blockNum;
	//		for (podr[1] = 0; podr[1] < dimtrans[1]; ++podr[1])
	//		{
	//			for (podr[2] = 0; podr[2] < dimtrans[0]; ++podr[2])
	//			{
	//				//三个方向上的偏移累加
	//				int totalidxFST = 0;
	//				int totalidxXYZ = (podr[0] + pos)*sliceSizeXY + podr[1] * dimtrans[0] + podr[2];
	//				
	//				for (int m = 0;m < 3;m++) 
	//				{
	//					if (order[m] == 2)
	//					{
	//						totalidxFST += podr[2-m] * sliceSizeFS;
	//						if (m == 2)
	//							totalidxFST += pos * sliceSizeFS;
	//					}
	//					else if (order[m] == 1)
	//					{
	//						totalidxFST += podr[2-m] * dim[0];
	//						if(m == 2)
	//							totalidxFST += pos * dim[0];
	//					}
	//					else
	//					{
	//						totalidxFST += podr[2-m];
	//						if(m == 2)
	//							totalidxFST += pos;
	//					}
	//				}
	//				val = data[totalidxFST];

	//				//pfn ? color = pfn(val) : color = osg::Vec4(1.0,1.0,1.0,1.0);

	//				val = val /4001.0;
	//				color = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);

	//				ptr[totalidxXYZ] = color;
	//				/*val = *data++;
	//				val = val /4000.0;
	//				*ptr++ = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);*/
	//				
	//			}
	//		}

	//	}
	//}



	osg::ref_ptr<osg::Texture3D>texture3D = new osg::Texture3D;
	texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
	texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
	texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
	texture3D->setResizeNonPowerOfTwoHint(false);  // 纹理支持非2的整次幂
	texture3D->setImage(image_3d);
	texture3D->setTextureWidth(image_3d->s());
	texture3D->setTextureHeight(image_3d->t());
	texture3D->setTextureDepth(image_3d->r());
	int x = texture3D->referenceCount();
	return texture3D;
}

osg::Texture3D* KL3DVolumeDataManager::createTextureP(int dim[3], float *data) 
{
	int blockSize = m_octree->getBlockSize();
	float * dataCache=new float[dim[0]*dim[1]*dim[2]];
	float* pdataCache=dataCache;
	bool isFull = dim[0] == blockSize && dim[1] == blockSize && dim[2] == blockSize;
	if (!isFull)
	{
		for (int k = 0; k < blockSize; k++)
		{
			for (int j = 0; j < blockSize; j++)
			{
				for (int i = 0; i < blockSize; i++)
				{
					if (-2 != *data)
					{
						*pdataCache=*data;
						pdataCache++;
					} 
					data++;
				}
			}
		}
		pdataCache = dataCache;
	}
	else
	{
		pdataCache = data;
	}

	const int *order = m_volHeader.orderXYZ;

	//转换后的 体块三个方向的采样点数
	int dimtrans[3] = {dim[ order[0]],dim[ order[1]],dim[ order[2]]};

	if (dim[ order[0]] == 1 &&dim[ order[1]] == 1 && dim[ order[2]] == 1)
	{
		std::cout<<"hehe-error:KL3DVolumeNode.createState"<<std::endl;
		return NULL;
	}

	// 三维纹理数据
	osg::ref_ptr<osg::Image> image_3d = new osg::Image;
	image_3d->allocateImage(dimtrans[0], dimtrans[1], dimtrans[2], GL_RGBA, GL_FLOAT);
	image_3d->setInternalTextureFormat(GL_RGBA);
	image_3d->setOrigin(osg::Image::BOTTOM_LEFT);

	osg::Vec4 *ptr = (osg::Vec4 *)image_3d->data();

	float val;
	osg::Vec4 color;


	int podr[3],pos;

	int sliceSizeFS = dim[0] * dim[1];
	int sliceSizeXY = dimtrans[0]*dimtrans[1];

	int processNum = omp_get_num_procs();


	//#pragma omp parallel for private(podr, pos, val, color)

	for (podr[0] = 0; podr[0] < dimtrans[2]; ++podr[0])//z
	{
		for (podr[1] = 0; podr[1] < dimtrans[1]; ++podr[1])//y
		{
			for (podr[2] = 0; podr[2] < dimtrans[0]; ++podr[2])//x
			{
				//三个方向上的偏移累加
				int totalidxFST = 0;
				int totalidxXYZ = podr[0]*sliceSizeXY + podr[1] * dimtrans[0] + podr[2];

				for (int m = 0;m < 3;m++) 
				{
					if (order[m] == 2)
					{
						totalidxFST += podr[2-m] * sliceSizeFS;
					}
					else if (order[m] == 1)
					{
						totalidxFST += podr[2-m] * dim[0];
					}
					else
					{
						totalidxFST += podr[2-m];
					}
				}
				val = pdataCache[totalidxFST];
				if (val == -2)
				{
					cout<<val;
				}

				val = val /4001.0;
				color = osg::Vec4(val, 1.0 - val, 1.0 - val * 0.5, 0.0);

				ptr[totalidxXYZ] = color;

			}
		}


	}

	osg::Texture3D* texture3D = new osg::Texture3D;
	texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
	texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
	texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
	texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
	texture3D->setResizeNonPowerOfTwoHint(false);  // 纹理支持非2的整次幂
	texture3D->setImage(image_3d);
	texture3D->setTextureWidth(image_3d->s());
	texture3D->setTextureHeight(image_3d->t());
	texture3D->setTextureDepth(image_3d->r());
	int x = texture3D->referenceCount();
	return texture3D;
}

osg::ref_ptr<osg::Texture3D> KL3DVolumeDataManager::createTexture()
{
	osg::ref_ptr<osg::Texture3D> tex3D = new osg::Texture3D;
	osg::ref_ptr<osg::Image> img = new osg::Image;
	img->allocateImage(1, 1, 1, GL_RGBA, GL_FLOAT);
	*(osg::Vec4*)img->data() = osg::Vec4(1, 0, 0, 1);
	tex3D->setImage(img);
	return tex3D;
}


osg::ref_ptr<osg::Texture3D> KL3DVolumeDataManager::createTexture(KLOctreeNode* node)
{
	float *data = m_cache->getNodeDataFromMap(node);
	int dim[3];

	dim[0] = node->m_sampleNum;
	dim[1] = node->m_CMPNum;
	dim[2] = node->m_recordNum;

	return createTexture(dim, data);
	//return createTexture();
}


osg::ref_ptr<osg::Texture3D> KL3DVolumeDataManager::getNodeTextureFromMap(KLOctreeNode *node)
{
	return NULL;

	//if (NULL == node)
	//{
	//	return NULL;
	//}

	////std::map<KLOctreeNode*, osg::ref_ptr<osg::Texture3D>>::iterator itrMap = m_nodeTextureMap.find(node);
	//std::map<KLOctreeNode*, osg::Texture3D*>::iterator itrMap = m_nodeTextureMap.find(node);
	//
	//
	////osg::Texture3D* texture3D = NULL;
	//if (m_nodeTextureMap.end() == itrMap)
	//{
	//	float *data = m_cache->getNodeDataFromMap(node);
	//	int dim[3];

	//	dim[0] = node->m_sampleNum;
	//	dim[1] = node->m_CMPNum;
	//	dim[2] = node->m_recordNum;

	//	texture3D = createTexture(dim, data);
	//	int x = texture3D->referenceCount();
	//	//texture3D = createTexture();
	//	if (m_nodeTextureMap.size() >= 200)
	//	{
	//		//std::map<KLOctreeNode *, osg::ref_ptr<osg::Texture3D>>::iterator itrMap;
	//		std::map<KLOctreeNode *, osg::Texture3D*>::iterator itrMap;
	//		std::list<KLOctreeNode *>::iterator itrNode;
	//		osg::Texture3D *tmpTex;
	//		//osg::ref_ptr<osg::Texture3D> tmpTex;
	//		KLOctreeNode *tmpNode;
	//		list<KLOctreeNode*> nodelist = m_displayList->getRefineSliceNodeList();
	//		for(itrMap=m_nodeTextureMap.begin(); itrMap!=m_nodeTextureMap.end(); ++itrMap)
	//		{
	//			tmpNode = itrMap->first;
	//			itrNode = find(nodelist.begin(), nodelist.end(), tmpNode);
	//			
	//			if (itrNode == nodelist.end())
	//			{
	//				tmpTex = itrMap->second;
	//				if(tmpTex->getTextureObject(0) != NULL) 
	//					continue;
	//				tmpTex->unref();
	//				m_nodeTextureMap.erase(itrMap);
	//				
	//				break;
	//			}
	//		}
	//	}
	//	m_nodeTextureMap.insert(std::make_pair(node, texture3D.get()));	
	//	texture3D->ref();//增加引用计数,表明这个对象还有另一个指针引用它
	//}
	//else
	//{
	//	texture3D = itrMap->second;
	//}

	////return texture3D.get();
	//return texture3D;

}


osg::StateSet* KL3DVolumeDataManager::getStateFromMap(KLOctreeNode* node)
{
	return NULL;
}

osg::ref_ptr<osg::Texture3D> KL3DVolumeDataManager::getTextureFromMap( KLOctreeNode* node, list<KLOctreeNode*>& worksetList )
{
	list<KLOctreeNode*>::iterator itr = find(m_nodeTexInCache.begin(), m_nodeTexInCache.end(), node);
	if (itr != m_nodeTexInCache.end())
	{
		m_nodeTexInCache.erase(itr);
		m_nodeTexInCache.push_back(node);
		return m_nodeTextureMap[node];
	}
	else
	{
		float *data = m_cache->getNodeDataFromMap(node);
		int dim[3];

		dim[0] = node->m_sampleNum;
		dim[1] = node->m_CMPNum;
		dim[2] = node->m_recordNum;
		osg::ref_ptr<osg::Texture3D> tex3d = createTexture(dim, data);
		if (m_nodeTexInCache.size()>=200)
		{
			list<KLOctreeNode*>::iterator itr = m_nodeTexInCache.begin();
			list<KLOctreeNode*>::iterator tmpItr = find(worksetList.begin(), worksetList.end(), *itr);
			while (tmpItr != worksetList.end())
			{
				++itr;
				tmpItr = find(worksetList.begin(), worksetList.end(), *itr);
			}
			m_nodeTextureMap.erase(*itr);
			m_nodeTexInCache.erase(itr);
			m_nodeTexInCache.push_back(node);
			m_nodeTextureMap.insert(make_pair(node, tex3d));
		}
		return tex3d;
	}
}

END_KLDISPLAY3D_NAMESPACE