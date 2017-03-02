////#pragma once
//
//#ifndef __KL3DVOLUME_KLTHREECROSSSLICE__
//#define __KL3DVOLUME_KLTHREECROSSSLICE__
//#include <osg/Group>
//#include <osg/MatrixTransform>
//#include <osg/Texture3D>
//#include <osg/Image>
//#include <osg/TexEnv>
//#include "KLDisplay3D/KL3DDataModel/KL3DVolumeDataAdapter.h"
//#include "KL3DVolumeMacro.h"
//BEGIN_KLDISPLAY3D_NAMESPACE
//
//class KL3DSlice;
//
//class KL3DVOLUME_EXPORT KL3DThreeCrossSlice :public osg::Group
//{
//public:
//	KL3DThreeCrossSlice(void);
//	~KL3DThreeCrossSlice(void);
//
//	template<class T>
//	void setVolumeData(KL3DVolumeDataAdapter<T> * dataAdapter);
//
//	void On();
//private:
//	template<class T>
//	void generateStateSet(KL3DVolumeDataAdapter<T> * dataAdapter);
//
//	void initSlices();
//
//	osg::ref_ptr<osg::StateSet> _stateSet;
//
//	osg::ref_ptr<KL3DSlice> _sliceXLine;
//	osg::ref_ptr<KL3DSlice> _sliceYLine;
//	osg::ref_ptr<KL3DSlice> _sliceZLine;
//
//	double * m_bounds;
//};
//
////
//template<class T>
//void KL3DThreeCrossSlice::setVolumeData(KL3DVolumeDataAdapter<T> * dataAdapter)
//{
//	//获取体数据
//	dataAdapter->excuteVolumeData();
//	//const int * pBound = dataAdapter->getBounds();
//
//	/*for(int i = 0;i < 6;++i)
//	{
//	_bounds[i] = pBound[i];
//	}*/
//
//	//测试-xinbing
//	m_bounds[0] = 0; m_bounds[1] = 64;
//	m_bounds[2] = 0; m_bounds[3] = 64;
//	m_bounds[4] = 0; m_bounds[5] = 64;
//
//	generateStateSet(dataAdapter);
//	initSlices();
//}
//
////
//template<class T>
//void KL3DThreeCrossSlice::generateStateSet(KL3DVolumeDataAdapter<T> * dataAdapter)
//{
//	struct VolumeHeader<T> header = dataAdapter->getVolumeHeader();
//	int numSampleX = header.numSampleX;
//	int numSampleY = header.numSampleY;
//	int numSampleZ = header.numSampleZ;
//	int totalSamples = numSampleX * numSampleY * numSampleZ;
//
//	osg::ref_ptr<osg::Image> image = new osg::Image;
//	image->allocateImage(numSampleZ,numSampleY,numSampleX,GL_RGBA,GL_UNSIGNED_BYTE);
//	image->setInternalTextureFormat(GL_RGBA);
//	osg::Vec4b *ptr = (osg::Vec4b *)image->data();
//
//	float val;
//	float maxVal = 4*4*3;
//	const T*  pdata = dataAdapter->getVolumeData();
//	//color mapping
//	for( int i = 0; i < totalSamples; ++i)
//	{
//		val = pdata[i];
//		val = val/maxVal * 255.0;
//		//*ptr ++= osg::Vec4b(val,255.0-val,255.0-val*0.5,255.0);
//		*ptr ++= osg::Vec4b(val,255-val,255-val*0.5,255);
//	}
//
//	osg::ref_ptr<osg::Texture3D> texture = new osg::Texture3D;
//	texture->setResizeNonPowerOfTwoHint(false);
//	texture->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::NEAREST);
//	texture->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::NEAREST);
//	texture->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
//	texture->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
//	texture->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
//	texture->setImage(image);
//
//	osg::ref_ptr<osg::TexEnv> texEnv = new osg::TexEnv;  
//	texEnv->setMode(osg::TexEnv::DECAL);  
//	_stateSet->setTextureAttribute(0,texEnv.get());
//	_stateSet->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
//}
//
//END_KLDISPLAY3D_NAMESPACE
//#endif