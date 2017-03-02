#include "TextureItem.h"


BEGIN_KLDISPLAY3D_NAMESPACE
TextureItem::TextureItem()
{
}

TextureItem::TextureItem(osg::ref_ptr<osg::Texture3D> tex3D, 
						 osg::ref_ptr<osg::Texture::TextureObject> texObj, 
						 unsigned int priority, unsigned int state
						 ):m_pTexture(tex3D), m_pTexObj(texObj), m_iPriority(priority),m_iState(state)
{
	
}

TextureItem::~TextureItem()
{
}

END_KLDISPLAY3D_NAMESPACE