///*********************************************************************************
//*class name:	KL3DSlice
//*author name:	yxl
//*note:			note
//*********************************************************************************/
//#ifndef __KL3DVOLUME_KL3DSLICECONTAINER__
//#define __KL3DVOLUME_KL3DSLICECONTAINER__
//
////#pragma once
//#include <osg/Group>
//#include "KLDisplay3D/KL3DDataModel/KL3DVolumeDataAdapter.h"
//#include "KL3DVolumeMacro.h"
//BEGIN_KLDISPLAY3D_NAMESPACE
//
//class KL3DSlice;
//
//class KL3DSliceContainer :public osg::Group
//{
//public:
//	KL3DSliceContainer(void);
//	~KL3DSliceContainer(void);
//
//	template<class T>
//	void setVolumeData(KL3DVolumeDataAdapter<T> * dataAdapter);
//
//	void addMoveableSlice(float *pNormal,float *pOrigin);
//
//	void addStadySlice(float *pNormal,float *pOrigin);
//
//protected:
//	template<class T>
//	void generateStateSet(KL3DVolumeDataAdapter<T> * dataAdapter);
//private:
//	osg::ref_ptr<osg::StateSet> _stateSet;
//	double *_bounds;
//
//
//};
//END_KLDISPLAY3D_NAMESPACE
//#endif