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
//\brief ��ά�ռ��еĵ���Ƭ��
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
//	\brief ������Ƭ��Χ��
//	*/
//	void setBounds( double *bou );
//	const double* getBounds();
//
//	//ƽ��ĵ㷨ʽ����-begin
//
//	/*!
//	\brief ������Ƭƽ��ķ���
//	*/
//	void setSliceNormal( double x, double y, double z );
//	void getSliceNormal( double* );
//
//	/*!
//	\brief ������Ƭƽ���ϵ�һ����
//	*/
//	void setSlicePoint( double x, double y, double z );
//	void getSlicePoint( double* );
//
//	//ƽ��ĵ㷨ʽ����-end
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
//	\brief ������Ƭ�Ķ������꣨ͨ�ã�
//	*/
//	void computeVertices();
//
//	/*!
//	\brief ������Ƭ�Ķ������꣨��ֱ�����ᣩ
//	\note1 ��ʱ��Ƭ��ֱ�����ᣬm_isPerpendicular = true
//	*/
//	void computeEasyVertices();
//
//	void generateHighLight();
//
//	/*!
//	\brief ��Ƭ��Ӧ�ļ���ƽ��
//	*/
//	osg::ref_ptr<osg::Geometry>  m_planeGeometry;
//
//	/*!
//	\brief ��Ƭ����ͼ�εĶ�����������
//	\note1 ���㰴˳ʱ����
//	*/
//	osg::ref_ptr<osg::Vec3Array> m_vertices;
//	
//	/*!
//	\brief ��Ƭ����ͼ�εĶ����Ӧ����������
//	\note1 ���㰴˳ʱ����
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
//	\brief ��Ƭ�İ�Χ��
//	\note1 ��һ��������
//	\note2 �ֱ�洢xmin xmax ymin ymax zmin zmax
//	*/
//	double  m_bounds[6];
//
//	//ƽ��ĵ㷨ʽ����-begin
//
//	/*!
//	\brief ��Ƭƽ��ķ���
//	*/
//	double  m_normal[3];
//
//	/*!
//	\brief ��Ƭƽ���ϵ�һ����
//	*/
//	double  m_point[3];
//
//	//ƽ��ĵ㷨ʽ����-end
//
//	/*!
//	\brief ��ʾ��Ƭƽ���Ƿ�ֱ������
//	\note1 true - ��ֱ������ false - ����ֱ������
//	*/
//	bool  m_isPerpendicular;
//};
//
//END_KLDISPLAY3D_NAMESPACE
//
//#endif//__KL3DVOLUME_KL3DSLICE__
