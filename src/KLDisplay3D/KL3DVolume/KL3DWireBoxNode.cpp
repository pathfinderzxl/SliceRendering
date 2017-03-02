#include "KL3DWireBoxNode.h"

BEGIN_KLDISPLAY3D_NAMESPACE

KL3DWireBoxNode::KL3DWireBoxNode(void)
{
}


KL3DWireBoxNode::~KL3DWireBoxNode(void)
{
}


void KL3DWireBoxNode::setBounds(osg::Vec3 & minB,osg::Vec3 & maxB)
{


	osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array;
	colorArray->push_back(osg::Vec4(0,0,1,1));
	colorArray->push_back(osg::Vec4(0,0,1,1));
	colorArray->push_back(osg::Vec4(0,0,1,1));
	colorArray->push_back(osg::Vec4(0,0,1,1));
	colorArray->push_back(osg::Vec4(0,0,1,1));
	colorArray->push_back(osg::Vec4(0,0,1,1));
	colorArray->push_back(osg::Vec4(0,0,1,1));
	colorArray->push_back(osg::Vec4(0,0,1,1));

	osg::ref_ptr<osg::Vec3Array> verArray = new osg::Vec3Array;
	verArray->push_back(osg::Vec3(minB[0],minB[1],minB[2]));
	verArray->push_back(osg::Vec3(maxB[0],minB[1],minB[2]));
	verArray->push_back(osg::Vec3(maxB[0],minB[1],maxB[2]));
	verArray->push_back(osg::Vec3(minB[0],minB[1],maxB[2]));

	verArray->push_back(osg::Vec3(minB[0],maxB[1],minB[2]));
	verArray->push_back(osg::Vec3(maxB[0],maxB[1],minB[2]));
	verArray->push_back(osg::Vec3(maxB[0],maxB[1],maxB[2]));
	verArray->push_back(osg::Vec3(minB[0],maxB[1],maxB[2]));

	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
	geom->setVertexArray(verArray);
	osg::DrawElementsUInt * bottomDrawElement = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP,0);
	bottomDrawElement->push_back(0);
	bottomDrawElement->push_back(1);
	bottomDrawElement->push_back(2);
	bottomDrawElement->push_back(3);
	geom->addPrimitiveSet(bottomDrawElement);

	osg::DrawElementsUInt * topDrawElement = new osg::DrawElementsUInt(osg::PrimitiveSet::LINE_LOOP,0);
	topDrawElement->push_back(4);
	topDrawElement->push_back(5);
	topDrawElement->push_back(6);
	topDrawElement->push_back(7);
	geom->addPrimitiveSet(topDrawElement);

	osg::DrawElementsUInt * linesDrawElement 
		= new osg::DrawElementsUInt(osg::PrimitiveSet::LINES,0);
	linesDrawElement->push_back(0);
	linesDrawElement->push_back(4);
	linesDrawElement->push_back(1);
	linesDrawElement->push_back(5);
	linesDrawElement->push_back(2);
	linesDrawElement->push_back(6);
	linesDrawElement->push_back(3);
	linesDrawElement->push_back(7);
	geom->addPrimitiveSet(linesDrawElement);
	geom->setColorArray(colorArray);
	//geom->setColorBinding(osg::Geometry::BIND_OVERALL);
	this->addDrawable(geom);
}
void KL3DWireBoxNode::generateBox()
{

}

END_KLDISPLAY3D_NAMESPACE