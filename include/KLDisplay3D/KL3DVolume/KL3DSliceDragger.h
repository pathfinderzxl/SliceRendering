#ifndef __KL3DSLICEDRAGGER_H__
#define __KL3DSLICEDRAGGER_H__


#include "KL3DSliceNode.h"
#include <osgManipulator/Translate1DDragger>

using namespace osgManipulator;

BEGIN_KLDISPLAY3D_NAMESPACE
class KL3DVOLUME_EXPORT KL3DSliceDragger : public osgManipulator::Translate1DDragger
{
public:
	KL3DSliceDragger(void);
	virtual ~KL3DSliceDragger(void);

	virtual bool handle(const PointerInfo& pi, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us);

	void setSlice(KL3DSliceNode * slice);

private:
	void setDirection(float direct[3]);
	void setupDefaultGeometry();

	osg::Vec3d					_orig;
	osg::Vec3					_origNow;
	KL3DSliceNode*				_slice;
	static KL3DSliceNode*		_activeSlice;
};

END_KLDISPLAY3D_NAMESPACE
#endif