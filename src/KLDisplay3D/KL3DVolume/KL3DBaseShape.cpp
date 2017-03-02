#include "KL3DBaseShape.h"

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DBaseShape::KL3DBaseShape(void)
{
	m_dataManager = NULL;
}


KL3DBaseShape::~KL3DBaseShape(void)
{
}

void KL3DBaseShape::initShape()
{

}
void KL3DBaseShape::setDataManager(KL3DVolumeDataManager * dataManager)
{
	if (dataManager)
	{
		this->m_dataManager = dataManager;
	}
}

END_KLDISPLAY3D_NAMESPACE