#ifndef __KL3DWIREBOXNODE_H__
#define __KL3DWIREBOXNODE_H__

#include "KL3DVolumeDataManager.h"
#include <osg/Vec3>
#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PrimitiveSet>



BEGIN_KLDISPLAY3D_NAMESPACE

class KL3DVOLUME_EXPORT KL3DWireBoxNode : public osg::Geode
{
public:
	KL3DWireBoxNode(void);
	virtual ~KL3DWireBoxNode(void);

	void setBounds(osg::Vec3 & minB,osg::Vec3 & maxB);

private:
	void generateBox();
};

END_KLDISPLAY3D_NAMESPACE
#endif