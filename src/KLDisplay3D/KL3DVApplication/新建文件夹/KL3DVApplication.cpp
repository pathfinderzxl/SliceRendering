
#include "KL3DVolumeShape.h"
#include "osgViewer/Viewer"
#include "KLGetColor.h"

using namespace KLSeis::KLDisplay3D;

int KLGetColor::flag = 0;

int main()
{
	osgViewer::Viewer *viewer = new osgViewer::Viewer;

	char *sourceFileName = "G:/voldata/pwave500x500M.sgy";

	KL3DVolumeShape *volumeShape = new KL3DVolumeShape(viewer, sourceFileName);

	viewer->setSceneData(volumeShape->getRoot());

	viewer->realize();
	viewer->run();

	return 0;
}