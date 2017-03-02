#ifndef __KL3DSLICESET_H__
#define __KL3DSLICESET_H__

#include <vector>

#include "KL3DSliceNode.h"
#include "KL3DBaseShape.h"
#include "KL3DOrthoSliceShape.h"

using namespace std;

BEGIN_KLDISPLAY3D_NAMESPACE

class KL3DVOLUME_EXPORT KL3DSliceSet
{
public:
	KL3DSliceSet(void);
	virtual ~KL3DSliceSet(void);
	/*
	*����Ƭ�����������Ƭ
	*/
	void addSlice(KL3DBaseShape * baseShape);

	/*
	*��ͼ�μ��ŵ�������
	*/
	void addToGroup(osg::ref_ptr<osg::Group> group);

	/*
	*���ؽڵ㼯������ڲ�����
	*/
	void getSliceNodes(std::vector<KL3DSliceNode *> &nodeList);
	void setDataManager(KL3DVolumeDataManager* dataManager){m_pDataManager = dataManager;}
private:
	std::vector<KL3DBaseShape *> m_baseShapeSet;
	KL3DVolumeDataManager *m_pDataManager;
};

END_KLDISPLAY3D_NAMESPACE
#endif
