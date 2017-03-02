#include <osg/Switch>
#include <osg/Geometry>
#include "osg/Group"
#include <iostream>

class KLVolume: public osg::Group 
{
public:
	KLVolume()
	{
		std::cout<<"init KLVolume : "<<std::endl;
	}
	void update(std::string words)
	{
		std::cout<<"KLVolume say "<<words<<std::endl;
	}

};
class KLSlice: public osg::Group 
{
public:
	KLSlice()
	{
		
			std::cout<<"init KLSlice : "<<std::endl;
		
	}
	void update(std::string words)
	{
		std::cout<<"KLSlice say "<<words<<std::endl;
	}

};
class KLSurface: public osg::Group 
{
public:
	KLSurface()
	{
		std::cout<<"init KLSurface : "<<std::endl;
	}

	void update(std::string words)
	{
		std::cout<<"KLSurface say "<<words<<std::endl;
	}

};
class KLVolumeS: public osg::Switch 
{
public:
	KLVolumeS()
	{
		m_root = new osg::Switch();
		m_volume= new KLVolume();
		m_root->addChild(m_volume);
		std::cout<<"init KLVolumeSwitch : "<<std::endl;
	}
	void update(std::string words)
	{
		std::cout<<"KLVolumeSwitch say "<<words<<std::endl;
	}
	osg::Switch* m_root;
	osg::ref_ptr<KLVolume> m_volume;

};
class KLROOT: public osg::Group
{
public:KLROOT()
	   {
		   m_volumeSwitch = new KLVolumeS();
		   std::cout<<"init KLROOT : "<<std::endl;
	   }
	   void update(std::string words)
	   {
		   std::cout<<"KLROOT say "<<words<<std::endl;
		   m_volumeSwitch->update(words);
		  /* int num =this->getNumChildren();
		  osg::ref_ptr<KLVolume> volume= (KLVolume*)this->getChild(0);
		  volume->update(words);
		  osg::ref_ptr<KLSurface> surface= (KLSurface*)this->getChild(1);
		  surface->update(words);
		  osg::ref_ptr<KLSlice> slice= (KLSlice*)this->getChild(2);
		  slice->update(words);*/
	   }
	   osg::ref_ptr<KLVolumeS> m_volumeSwitch;
		
};
int main2()
{
	osg::ref_ptr<KLROOT> Root_Node= new KLROOT();
	/*osg::ref_ptr<osg::Switch> child1 = new osg::Switch();
	osg::ref_ptr<osg::Switch> child2 = new osg::Switch();
	osg::ref_ptr<osg::Switch> child3 = new osg::Switch();
	Root_Node->addChild(child1);
	Root_Node->addChild(child2);
	Root_Node->addChild(child3);
	osg::ref_ptr<KLVolume> volume = new KLVolume();
	child1->addChild(volume);
	osg::ref_ptr<KLSurface> surface = new KLSurface();
	child2->addChild(surface);
	osg::ref_ptr<KLSlice> slice = new KLSlice();
	child3->addChild(slice);
	child2->setValue(0,false);*/
	Root_Node->update("update");


	system("pause");
	return 0;
}
