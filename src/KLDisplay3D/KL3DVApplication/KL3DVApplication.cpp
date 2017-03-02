#include "KL3DVolume/KL3DVolumeDataManager.h"
#include "KL3DVolumeShape.h"
//#include "KL3DIsoSurfaceShape.h"
#include "osgViewer/Viewer"
#include "KLGetColor.h"
#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "KL3DVolume/KL3DBaseShape.h"
#include "KL3DVolume/KL3DOrthoSliceShape.h"
#include "KL3DVolume/KL3DSliceNode.h"
#include "KL3DVolume/KL3DSliceSet.h"
#include "KL3DVolume/KL3DWireBoxNode.h"

#include "osgViewer/ViewerEventHandlers"
#include "osgUtil/Optimizer"

using namespace KLSeis::KLDisplay3D;

int KLGetColor::flag = 0;

int main()
{
	osgViewer::Viewer *viewer = new osgViewer::Viewer;	
		
	KLGetColor getColor;
	getColorByValueFunc pfn = boost::bind(&KLGetColor::getColorByValue1, &getColor, _1);

	//QString sourceFileName = "E:/volData/pwave2000x38G.sgy";
	QString sourceFileName = "F:/PostGraduate/Projects/KLSeis/vol_dat/new_oct/pwave1000x4G.sgy";
	//QString sourceFileName = "F:/PostGraduate/Projects/KLSeis/vol_dat/new_oct/pwave500x500M.sgy";
	KL3DVTestSgyDataAdapter *volDataAdapter = new KL3DVTestSgyDataAdapter(sourceFileName);

	KL3DVolumeDataManager* dataManager = new KL3DVolumeDataManager();
	KL3DDisplayList *displayList = new KL3DDisplayList();
	dataManager->initData(volDataAdapter);

	dataManager->setDisplayList(displayList);
	//dataManager->setCacheSize(100);//在初始化数据之前调用才有效，否则会是默认值50块

	if (!dataManager->initData(volDataAdapter))
		return 0;  
	
	KL3DSliceSet *sliceSet = new KL3DSliceSet();

	KL3DOrthoSliceShape *orthoSlice0 = new KL3DOrthoSliceShape(0, sliceSet);
	orthoSlice0->setDataManager(dataManager);
	orthoSlice0->setViewer(viewer);
	

	KL3DOrthoSliceShape *orthoSlice1 = new KL3DOrthoSliceShape(1, sliceSet);
	orthoSlice1->setDataManager(dataManager);
	orthoSlice1->setViewer(viewer);


	KL3DOrthoSliceShape *orthoSlice2 = new KL3DOrthoSliceShape(2, sliceSet);
	orthoSlice2->setDataManager(dataManager);
	orthoSlice2->setViewer(viewer);



	sliceSet->addSlice(orthoSlice0);
	sliceSet->addSlice(orthoSlice1);
	sliceSet->addSlice(orthoSlice2);

	orthoSlice0->initShape();
	orthoSlice1->initShape();
	orthoSlice2->initShape();

	double *bounds = new double[6];
	int vBound[3];
	vBound[0] = volDataAdapter->getVolumeHeader().numFirst;
	vBound[1] = volDataAdapter->getVolumeHeader().numSecond;
	vBound[2] = volDataAdapter->getVolumeHeader().numThird;

	for(int i = 0;i < 3;++i)
	{
		bounds[i] = 0;
	}
	bounds[3] = vBound[0];
	bounds[4] = vBound[1];
	bounds[5] = vBound[2];

	osg::ref_ptr<KL3DWireBoxNode> bNode = new KL3DWireBoxNode();
	osg::Vec3 v1(bounds[0], bounds[1], bounds[2]);
	osg::Vec3 v2(bounds[3], bounds[4], bounds[5]);
	bNode->setBounds(v1, v2);

	osg::ref_ptr<osg::Group> root = new osg::Group();
	sliceSet->addToGroup(root);
	root->addChild(bNode);

	osgUtil::Optimizer optimizer;
	optimizer.optimize(root);

	viewer->setSceneData(root);
	viewer->addEventHandler(new osgViewer::StatsHandler);
	viewer->setLightingMode(osg::View::LightingMode::NO_LIGHT);
	viewer->setUpViewInWindow(100, 100, 800, 600);
	viewer->realize();
	return viewer->run();

	
	
}