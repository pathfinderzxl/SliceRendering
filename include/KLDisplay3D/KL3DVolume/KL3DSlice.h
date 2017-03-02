///************************************************************************
//Auther: yxl
//Date: 2014-11-11
//Note: xinbing
//************************************************************************/
//#ifndef __KL3DVOLUME_KL3DSLICE__
//#define __KL3DVOLUME_KL3DSLICE__
//
//#include <osg/Geode>
//#include <osg/Geometry>
//#include "KL3DVolumeMacro.h"
//
//BEGIN_KLDISPLAY3D_NAMESPACE
//
///*!
//\brief 三维空间中的单切片类
//*/
//class KL3DVOLUME_EXPORT KL3DSlice : public osg::Geode
//{
//public:
//	KL3DSlice();
//	~KL3DSlice();
//
//	
//	void setStateSet(osg::StateSet * stateset);
//
//	void On();
//	void highLightOn();
//	void highLightOff();
//	void updateSlice();
//
//	/*!
//	\brief 设置切片包围盒
//	*/
//	void setBounds( double *bou );
//	const double* getBounds();
//
//	//平面的点法式方程-begin
//
//	/*!
//	\brief 设置切片平面的法向
//	*/
//	void setSliceNormal( double x, double y, double z );
//	void getSliceNormal( double* );
//
//	/*!
//	\brief 设置切片平面上的一个点
//	*/
//	void setSlicePoint( double x, double y, double z );
//	void getSlicePoint( double* );
//
//	//平面的点法式方程-end
//
//private:
//	void updatePosition();
//	void updateTexture();
//
//	void generateGeometry();
//
//	void computeTexCoord();
//	void computeEasyTexCoord();
//
//	/*!
//	\brief 计算切片的顶点坐标（通用）
//	*/
//	void computeVertices();
//
//	/*!
//	\brief 计算切片的顶点坐标（垂直坐标轴）
//	\note1 此时切片垂直坐标轴，m_isPerpendicular = true
//	*/
//	void computeEasyVertices();
//
//	void generateHighLight();
//
//	/*!
//	\brief 切片对应的几何平面
//	*/
//	osg::ref_ptr<osg::Geometry>  m_planeGeometry;
//
//	/*!
//	\brief 切片几何图形的顶点坐标数组
//	\note1 顶点按顺时针存放
//	*/
//	osg::ref_ptr<osg::Vec3Array> m_vertices;
//	
//	/*!
//	\brief 切片几何图形的顶点对应的纹理坐标
//	\note1 顶点按顺时针存放
//	*/
//	osg::ref_ptr<osg::Vec3Array> m_texCoord;
//
//	osg::ref_ptr<osg::Geometry> _geomHighLight;
//
//	osg::ref_ptr<osg::Vec4Array> _color;	
//
//	osg::ref_ptr<osg::Vec4Array> _highLightColorArray;
//
//	osg::Vec4 * _highLightColor;
//
//	bool _isActive;
//
//	float *_sliceData;
//
//	/*!
//	\brief 切片的包围盒
//	\note1 是一个长方体
//	\note2 分别存储xmin xmax ymin ymax zmin zmax
//	*/
//	double  m_bounds[6];
//
//	//平面的点法式方程-begin
//
//	/*!
//	\brief 切片平面的法向
//	*/
//	double  m_normal[3];
//
//	/*!
//	\brief 切片平面上的一个点
//	*/
//	double  m_point[3];
//
//	//平面的点法式方程-end
//
//	/*!
//	\brief 标示切片平面是否垂直坐标轴
//	\note1 true - 垂直坐标轴 false - 不垂直坐标轴
//	*/
//	bool  m_isPerpendicular;
//};
//
//END_KLDISPLAY3D_NAMESPACE
//
//#endif//__KL3DVOLUME_KL3DSLICE__
