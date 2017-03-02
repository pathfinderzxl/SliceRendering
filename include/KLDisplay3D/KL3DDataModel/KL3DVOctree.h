/************************************************************************
Auther: xinbing
Date: 2013-9-22
Note:
************************************************************************/
#ifndef __KL3DDATA_KLOCTREE_H__
#define __KL3DDATA_KLOCTREE_H__

#include "KL3DDataModelMacro.h"
#include <string>
#include <QtCore/QString>
/*!
\brief 八叉树结构数据文件的文件头大小
\      单位B
*/
#define OCTREEHEADER 40

BEGIN_KLDISPLAY3D_NAMESPACE

class KLOctreeNode;
using namespace std;
class KL3DDATAMODEL_EXPORT KL3DVOctree
{
public:
	KL3DVOctree();
	//同过八叉树文件重构八叉树
	KL3DVOctree(QString fileName);
	~KL3DVOctree();

	/*!
	\brief 数据预处理，根据源数据文件建立八叉树结构，并根据八叉树结构处理
	\      源数据生成相应的八叉树结构数据文件，两个数据文件在同一目录下，后缀不同
	\note1 数据预处理时，对外的唯一接口。在此函数内部区分数据格式
	\note2 支持sgy和规则表单
	*/
	void dataPreprocessing();

	/*!
	\brief 恢复八叉树
	\param int depth--八叉树深度  int octreeSize--八叉树尺寸
	\      int minX, int maxX,
	\	   int minY, int maxY,
	\	   int minZ, int maxZ--八叉树三方向范围
	\note1  预处理时通过createOctree函数创建八叉树，并把关键信息写入八叉树结构数据文件
	\	    头部，这些信息就在此函数中使用
	\note2  渲染八叉树文件时，对外的唯一接口
	*/
	void recoverOctree( int depth, int octreeSize,
		int minX, int maxX,
		int minY, int maxY,
		int minZ, int maxZ );

	//在八叉树文件为参数的构造函数之后，根据文件重构八叉树
	void recoverOctree();
	//根据八叉树文件尾部的从根节点到叶子的location，给重构的八叉树结构各个节点的location赋值
	void readOctreeFileTail();
	/*!
	\brief 创建八叉树，由m_rootNode指向创建的八叉树的根节点
	\note1 使用前提：getSgyHeaderInfo()或getGridHeaderInfo()已被执行
	\note2 在dataPreprocessing()中被调用
	\note3 也可以单独使用。就目前需求来说，不需要单独调用此函数
	*/
	void createOctree();

	/*!
	\brief 设置源数据文件和生成的八叉树结构数据文件的全路径名
	\param sourceFile--源数据文件全路径名
	\      generateFile--生成的八叉树结构数据文件的全路径名
	\note  generateFile = NULL 用于只生成八叉树结构不进行预处理的情况
	*/
	void setAbsoluteFileName(  QString sourceFile, QString generateFile = "" );

	QString getOctreeFileName(){return m_octreeFileName;}

	//获得属性值的最大值
	int getMaxValue(){return m_maxValue;}
	int getBlockSize(){return m_BLOCKSIZE;}

	void initialEntropy(string path);

//受保护的成员函数
protected:
	/*!
	\brief 计算三个方向的边界值
	\note1  边界值的解释参见int m_limitSample, m_limitCMP, m_limitRecord;
	\note2  在createOctree函数中被调用
	*/
	void computeLimit();

	/*!
	\brief 在createOctree()中被调用，是递归函数
	\param KLOctreeNode *&node--指向新创建的八叉树节点
	\      int depth--此节点算起的八叉树剩余深度
	\	   int level--此节点所在的层级
	\	   int tileSize--此节点的瓦片尺寸
	\	   int minX, minY, minZ, maxX, maxY, maxZ--此节点三方向的标号范围
	*/
	void completeOctreeNode( KLOctreeNode *&node, int depth,
		int level, int tileSize,
		int minX, int maxX,
		int minY, int maxY,
		int minZ, int maxZ );

	/**
	\brief 对八叉树节点补充信息
	\note  堂兄弟间建立连接、找到最左叶子、统计叶子节点数目
	*/
	void setTileInfo();

	/*!释放八叉树空间*/
	void Delete( KLOctreeNode* rootNode );

	/*!
	\brief 创建一个瓦片节点，并且为其设置基本信息
	\param KLOctreeNode *&node--指向新建的节点
	\	   int depth--此节点算起的八叉树剩余深度
	\	   int level--此节点所在的层级
	\	   int minX, maxX, minY, maxY, minZ, maxZ--此节点三方向的标号范围
	\note  在completeOctreeNode函数中被调用
	*/
	void newTileNode( KLOctreeNode *&node, int depth, int level,
		                       int minX, int maxX,
							   int minY, int maxY,
							   int minZ, int maxZ, int _idx );

	/*!
	\brief 获取源数据文件(m_sourceFileName)的文件头信息
	\note1  通过此函数获取建立八叉树需要的相关信息，屏蔽了源数据文件的格式
	\	   对于不同格式的源数据文件，要重写此函数
	\	   (备注：此处针对sgy数据文件)
	\note2  在createOctree()函数中被调用
	*/
	//virtual void excuteSourceFileHeader();


private:
	/*!
	\brief 获取sgy文件的头部信息
	\note  在processSgy()中被调用
	*/
	void getSgyHeaderInfo();

	/*!
	\brief 获取规则表单的头部信息
	\note  在processGrid()中被调用
	*/
	void getGridHeaderInfo();

	/*!
	\brief sgy数据预处理
	\note  在dataPreprocessing()中被调用
	*/
	void processSgy();

	/*!
	\brief 规则表单预处理
	\note  在dataPreprocessing()中被调用
	*/
	void processGrid();
	

//公有成员变量
public:
	/**
	\叶子节点的数目
	*/
	int m_numYezi;

	/**
	\八叉树的节点数目
	*/
	int m_nodeNum;

	/**
	\创建八叉树后，m_rootNode指向创建的八叉树
	*/
	KLOctreeNode *m_rootNode;

	/**
	\八叉树的深度从0算起
	*/
	int m_depth;

	/**
	\指向叶子层最左端的叶子
	*/
	KLOctreeNode *m_zuoYezi;
	
	/**
	\创建八叉树时计算出
	*/
	int m_octreeSize;	

	/*!源数据三个方向上的采样点起始和结束标号，min值默认0*/
	int m_minX, m_minY, m_minZ;
	int m_maxX, m_maxY, m_maxZ;

	/*!源数据三个方向上的采样点数目*/
	int m_numx, m_numy, m_numz;

	/*!源数据第一个采样点的坐标*/
	double m_originx, m_originy, m_originz;

	/*!源数据三个方向上的采样间隔
	\  y、z方向上单位是米，x方向上单位是秒，所以要在x方向上有个倍乘系数
	\  保证显示的体接近立方体。
	\  m_gridSizey、m_gridSizez实际采样间隔，m_gridSizex是乘以倍乘系数之后的
	\  采样间隔，m_gridSizexFact是x方向实际采样间隔, m_ratiox是x方向倍乘系数
	\
	*/
	double m_gridSizex, m_gridSizey, m_gridSizez, m_gridSizexFact, m_ratiox;
	

//私有成员变量
private:

	int m_BLOCKSIZE;
	/*!
	\m_sourceFileName--源数据文件全路径名
	\m_octreeFileName--生成的八叉树结构数据文件全路径名
	*/
	QString m_sourceFileName, m_octreeFileName;

	/*!
	\brief 变量记录的是：边界值到max值之间的部分整体向前扩展后的新的边界值
	\	   在computeLimit函数中计算出
	\note  三个方向上的边界值，由octreeSize除以2的n次幂得到，0到边界值之间的部分可以
	\	   组成完全八叉树，边界值到max值之间的部分整体向前扩展，以致这两部分在建树时
	\	   虽然在数据上有交叉，但是边界值固定不变，从而，子节点的边界与父节点的边界完全相同。
	*/
	int m_limitSample, m_limitCMP, m_limitRecord;//向前扩展后的新的边界值
	int m_oldSample, m_oldCMP, m_oldRecord;//没扩展前的边界值

	/*!
	\brief 是否使用边界值，true使用，false不使用
	\note1 构造函数中置true
	\	   在某个方向上第一次划分成两段区域时，才用到limit值，以后不再使用
	\note2 此判断标准存在漏洞。比如，1层是0246块，当进入下一层时，需要在x方向上划分，
	\	   所以这四个块都要用到m_limitSample值，但是此判断标准只能让0块使用这个limit值，
	\	   剩余的246块就使用不了这个limit值了。
	\note3 新的判断标准：由父块进入子块时，以x方向为例，若minx < m_limitSample < maxx，
	\	   则使用此limit值
	*/
	bool flag_limitx, flag_limity, flag_limitz;

	//八叉树文件中标量值的最小值和最大值
	float m_minValue, m_maxValue;

};

END_KLDISPLAY3D_NAMESPACE
#endif
