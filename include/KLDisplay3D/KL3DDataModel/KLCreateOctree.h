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
	\brief ��createOctree()�б����ã��ǵݹ麯��
	\param KLOctreeNode *&node--ָ���´����İ˲����ڵ�
	\      int depth--�˽ڵ�����İ˲���ʣ�����
	\	   int level--�˽ڵ����ڵĲ㼶
	\	   int tileSize--�˽ڵ����Ƭ�ߴ�
	\	   int minX, minY, minZ, maxX, maxY, maxZ--�˽ڵ�������ı�ŷ�Χ
	*/
	void completeOctreeNode(KLOctreeNode *&node, int depth,
		int level, int tileSize,
		int minX, int maxX,
		int minY, int maxY,
		int minZ, int maxZ);

	/*!
	\brief ����һ����Ƭ�ڵ㣬����Ϊ�����û�����Ϣ
	\param KLOctreeNode *&node--ָ���½��Ľڵ�
	\	   int depth--�˽ڵ�����İ˲���ʣ�����
	\	   int level--�˽ڵ����ڵĲ㼶
	\	   int minX, maxX, minY, maxY, minZ, maxZ--�˽ڵ�������ı�ŷ�Χ
	\note  ��completeOctreeNode�����б�����
	*/
	void newTileNode(KLOctreeNode *&node, int depth, int level,
		int minX, int maxX,
		int minY, int maxY,
		int minZ, int maxZ);

	/**
	\brief �԰˲����ڵ㲹����Ϣ
	\note  ���ֵܼ佨�����ӡ��ҵ�����Ҷ�ӡ�ͳ��Ҷ�ӽڵ���Ŀ
	*/
	void setTileInfo();

	void readOctreeFileTail();

private:

	//�˲����ļ��б���ֵ����Сֵ�����ֵ�Ϳ�ֵ
	float m_minValue, m_maxValue, m_nullValue;

	/*!Դ�������������ϵĲ�������ʼ�ͽ�����ţ�minֵĬ��0*/
	int m_minX, m_minY, m_minZ;
	int m_maxX, m_maxY, m_maxZ;

	/*!Դ�������������ϵĲ������
	\  y��z�����ϵ�λ���ף�x�����ϵ�λ���룬����Ҫ��x�������и�����ϵ��
	\  ��֤��ʾ����ӽ������塣
	\  m_gridSizey��m_gridSizezʵ�ʲ��������m_gridSizex�ǳ��Ա���ϵ��֮���
	\  �������
	*/
	double m_gridSizex, m_gridSizey, m_gridSizez;

private:

	int m_BLOCKSIZE;

	/**
	\�����˲���ʱ����������������ϵ������
	*/
	int m_octreeSize;

	/**
	\�˲�������ȴ�0����
	*/
	int m_depth;

	/**
	\Ҷ�ӽڵ����Ŀ
	*/
	int m_numYezi;

	/**
	\�˲����Ľڵ���Ŀ
	*/
	int m_nodeNum;

	/**
	\�����˲�����m_rootNodeָ�򴴽��İ˲���
	*/
	KLOctreeNode* m_rootNode;

	/**
	\ָ��Ҷ�Ӳ�����˵�Ҷ��
	*/
	KLOctreeNode *m_zuoYezi;

private:
	QFile* m_pFileIn;
};
END_KLDISPLAY3D_NAMESPACE
#endif
