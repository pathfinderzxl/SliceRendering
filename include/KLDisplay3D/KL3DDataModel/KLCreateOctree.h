#ifndef __KL3DDATA_KLCREATEOCTREE_H__
#define __KL3DDATA_KLCREATEOCTREE_H__

class KLOctreeNode;
class QString;
BEGIN_KLDISPLAY3D_NAMESPACE
class KL3DDATAMODEL_EXPORT KLCreateOctree
{
public:
	KLCreateOctree(QString octreeFileName);
	~KLCreateOctree();

	void setInterFirst(int inter);
	void setInterSecond(int inter);
	void setInterThird(int inter);
	
	void recoverOctree();

	KLOctreeNode* getRootNode();

	void Delete(KLOctreeNode* node);

private:
	/*!
	\brief 在createOctree()中被调用，是递归函数
	\param KLOctreeNode *&node--指向新创建的八叉树节点
	\      int depth--此节点算起的八叉树剩余深度
	\	   int level--此节点所在的层级
	\	   int tileSize--此节点的瓦片尺寸
	\	   int minX, minY, minZ, maxX, maxY, maxZ--此节点三方向的标号范围
	*/
	void completeOctreeNode(KLOctreeNode *&node, int depth,
		int level, int tileSize,
		int minX, int maxX,
		int minY, int maxY,
		int minZ, int maxZ);

	/*!
	\brief 创建一个瓦片节点，并且为其设置基本信息
	\param KLOctreeNode *&node--指向新建的节点
	\	   int depth--此节点算起的八叉树剩余深度
	\	   int level--此节点所在的层级
	\	   int minX, maxX, minY, maxY, minZ, maxZ--此节点三方向的标号范围
	\note  在completeOctreeNode函数中被调用
	*/
	void newTileNode(KLOctreeNode *&node, int depth, int level,
		int minX, int maxX,
		int minY, int maxY,
		int minZ, int maxZ);

	/**
	\brief 对八叉树节点补充信息
	\note  堂兄弟间建立连接、找到最左叶子、统计叶子节点数目
	*/
	void setTileInfo();

	void readOctreeFileTail();

private:

	//八叉树文件中标量值的最小值，最大值和空值
	float m_minValue, m_maxValue, m_nullValue;

	/*!源数据三个方向上的采样点起始和结束标号，min值默认0*/
	int m_minX, m_minY, m_minZ;
	int m_maxX, m_maxY, m_maxZ;

	/*!源数据三个方向上的采样间隔
	\  y、z方向上单位是米，x方向上单位是秒，所以要在x方向上有个倍乘系数
	\  保证显示的体接近立方体。
	\  m_gridSizey、m_gridSizez实际采样间隔，m_gridSizex是乘以倍乘系数之后的
	\  采样间隔
	*/
	double m_gridSizex, m_gridSizey, m_gridSizez;

private:

	int m_BLOCKSIZE;

	/**
	\创建八叉树时计算出，三个方向上的最长点数
	*/
	int m_octreeSize;

	/**
	\八叉树的深度从0算起
	*/
	int m_depth;

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
	KLOctreeNode* m_rootNode;

	/**
	\指向叶子层最左端的叶子
	*/
	KLOctreeNode *m_zuoYezi;

private:
	QFile* m_pFileIn;
};
END_KLDISPLAY3D_NAMESPACE
#endif
