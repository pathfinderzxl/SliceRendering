/************************************************************************
Auther: xinbing
Date: 2014-11-11
Note:
************************************************************************/
#ifndef __KL3DDATAMODELMACRO_H__
#define __KL3DDATAMODELMACRO_H__

#include "QtCore/qglobal.h"

#ifdef _COMPILING_KL3DDATAMODEL
#define KL3DDATAMODEL_EXPORT Q_DECL_EXPORT
#else
#define KL3DDATAMODEL_EXPORT Q_DECL_IMPORT
#endif

//ÃüÃû¿Õ¼ä
#define BEGIN_KLDISPLAY3D_NAMESPACE namespace KLSeis { namespace KLDisplay3D {
#define END_KLDISPLAY3D_NAMESPACE }}

#endif //__KL3DDATAMODELMACRO_H__
