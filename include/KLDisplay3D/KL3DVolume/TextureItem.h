#ifndef __TEXTUREITEM_H__
#define __TEXTUREITEM_H__

#include "KL3DVolumeMacro.h"
#include<osg/Texture3D>
#include "KL3DDataModel/KLOctreeNode.h"

BEGIN_KLDISPLAY3D_NAMESPACE

class KL3DVOLUME_EXPORT TextureItem
{

public:
	TextureItem(void);
	TextureItem(osg::ref_ptr<osg::Texture3D> tex3D, 
				osg::ref_ptr<osg::Texture::TextureObject> texObj,
				unsigned int priority,
				unsigned int sate);
	virtual ~TextureItem(void);

	osg::ref_ptr<osg::Texture3D> m_pTexture;
	osg::ref_ptr<osg::Texture::TextureObject> m_pTexObj;
	unsigned int m_iPriority;
	unsigned int m_iState;

};

END_KLDISPLAY3D_NAMESPACE
#endif