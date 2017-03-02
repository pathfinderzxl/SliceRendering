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
\brief �˲����ṹ�����ļ����ļ�ͷ��С
\      ��λB
*/
#define OCTREEHEADER 40

BEGIN_KLDISPLAY3D_NAMESPACE

class KLOctreeNode;
using namespace std;
class KL3DDATAMODEL_EXPORT KL3DVOctree
{
public:
	KL3DVOctree();
	//ͬ���˲����ļ��ع��˲���
	KL3DVOctree(QString fileName);
	~KL3DVOctree();

	/*!
	\brief ����Ԥ��������Դ�����ļ������˲����ṹ�������ݰ˲����ṹ����
	\      Դ����������Ӧ�İ˲����ṹ�����ļ������������ļ���ͬһĿ¼�£���׺��ͬ
	\note1 ����Ԥ����ʱ�������Ψһ�ӿڡ��ڴ˺����ڲ��������ݸ�ʽ
	\note2 ֧��sgy�͹����
	*/
	void dataPreprocessing();

	/*!
	\brief �ָ��˲���
	\param int depth--�˲������  int octreeSize--�˲����ߴ�
	\      int minX, int maxX,
	\	   int minY, int maxY,
	\	   int minZ, int maxZ--�˲���������Χ
	\note1  Ԥ����ʱͨ��createOctree���������˲��������ѹؼ���Ϣд��˲����ṹ�����ļ�
	\	    ͷ������Щ��Ϣ���ڴ˺�����ʹ��
	\note2  ��Ⱦ�˲����ļ�ʱ�������Ψһ�ӿ�
	*/
	void recoverOctree( int depth, int octreeSize,
		int minX, int maxX,
		int minY, int maxY,
		int minZ, int maxZ );

	//�ڰ˲����ļ�Ϊ�����Ĺ��캯��֮�󣬸����ļ��ع��˲���
	void recoverOctree();
	//���ݰ˲����ļ�β���ĴӸ��ڵ㵽Ҷ�ӵ�location�����ع��İ˲����ṹ�����ڵ��location��ֵ
	void readOctreeFileTail();
	/*!
	\brief �����˲�������m_rootNodeָ�򴴽��İ˲����ĸ��ڵ�
	\note1 ʹ��ǰ�᣺getSgyHeaderInfo()��getGridHeaderInfo()�ѱ�ִ��
	\note2 ��dataPreprocessing()�б�����
	\note3 Ҳ���Ե���ʹ�á���Ŀǰ������˵������Ҫ�������ô˺���
	*/
	void createOctree();

	/*!
	\brief ����Դ�����ļ������ɵİ˲����ṹ�����ļ���ȫ·����
	\param sourceFile--Դ�����ļ�ȫ·����
	\      generateFile--���ɵİ˲����ṹ�����ļ���ȫ·����
	\note  generateFile = NULL ����ֻ���ɰ˲����ṹ������Ԥ��������
	*/
	void setAbsoluteFileName(  QString sourceFile, QString generateFile = "" );

	QString getOctreeFileName(){return m_octreeFileName;}

	//�������ֵ�����ֵ
	int getMaxValue(){return m_maxValue;}
	int getBlockSize(){return m_BLOCKSIZE;}

	void initialEntropy(string path);

//�ܱ����ĳ�Ա����
protected:
	/*!
	\brief ������������ı߽�ֵ
	\note1  �߽�ֵ�Ľ��Ͳμ�int m_limitSample, m_limitCMP, m_limitRecord;
	\note2  ��createOctree�����б�����
	*/
	void computeLimit();

	/*!
	\brief ��createOctree()�б����ã��ǵݹ麯��
	\param KLOctreeNode *&node--ָ���´����İ˲����ڵ�
	\      int depth--�˽ڵ�����İ˲���ʣ�����
	\	   int level--�˽ڵ����ڵĲ㼶
	\	   int tileSize--�˽ڵ����Ƭ�ߴ�
	\	   int minX, minY, minZ, maxX, maxY, maxZ--�˽ڵ�������ı�ŷ�Χ
	*/
	void completeOctreeNode( KLOctreeNode *&node, int depth,
		int level, int tileSize,
		int minX, int maxX,
		int minY, int maxY,
		int minZ, int maxZ );

	/**
	\brief �԰˲����ڵ㲹����Ϣ
	\note  ���ֵܼ佨�����ӡ��ҵ�����Ҷ�ӡ�ͳ��Ҷ�ӽڵ���Ŀ
	*/
	void setTileInfo();

	/*!�ͷŰ˲����ռ�*/
	void Delete( KLOctreeNode* rootNode );

	/*!
	\brief ����һ����Ƭ�ڵ㣬����Ϊ�����û�����Ϣ
	\param KLOctreeNode *&node--ָ���½��Ľڵ�
	\	   int depth--�˽ڵ�����İ˲���ʣ�����
	\	   int level--�˽ڵ����ڵĲ㼶
	\	   int minX, maxX, minY, maxY, minZ, maxZ--�˽ڵ�������ı�ŷ�Χ
	\note  ��completeOctreeNode�����б�����
	*/
	void newTileNode( KLOctreeNode *&node, int depth, int level,
		                       int minX, int maxX,
							   int minY, int maxY,
							   int minZ, int maxZ, int _idx );

	/*!
	\brief ��ȡԴ�����ļ�(m_sourceFileName)���ļ�ͷ��Ϣ
	\note1  ͨ���˺�����ȡ�����˲�����Ҫ�������Ϣ��������Դ�����ļ��ĸ�ʽ
	\	   ���ڲ�ͬ��ʽ��Դ�����ļ���Ҫ��д�˺���
	\	   (��ע���˴����sgy�����ļ�)
	\note2  ��createOctree()�����б�����
	*/
	//virtual void excuteSourceFileHeader();


private:
	/*!
	\brief ��ȡsgy�ļ���ͷ����Ϣ
	\note  ��processSgy()�б�����
	*/
	void getSgyHeaderInfo();

	/*!
	\brief ��ȡ�������ͷ����Ϣ
	\note  ��processGrid()�б�����
	*/
	void getGridHeaderInfo();

	/*!
	\brief sgy����Ԥ����
	\note  ��dataPreprocessing()�б�����
	*/
	void processSgy();

	/*!
	\brief �����Ԥ����
	\note  ��dataPreprocessing()�б�����
	*/
	void processGrid();
	

//���г�Ա����
public:
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
	KLOctreeNode *m_rootNode;

	/**
	\�˲�������ȴ�0����
	*/
	int m_depth;

	/**
	\ָ��Ҷ�Ӳ�����˵�Ҷ��
	*/
	KLOctreeNode *m_zuoYezi;
	
	/**
	\�����˲���ʱ�����
	*/
	int m_octreeSize;	

	/*!Դ�������������ϵĲ�������ʼ�ͽ�����ţ�minֵĬ��0*/
	int m_minX, m_minY, m_minZ;
	int m_maxX, m_maxY, m_maxZ;

	/*!Դ�������������ϵĲ�������Ŀ*/
	int m_numx, m_numy, m_numz;

	/*!Դ���ݵ�һ�������������*/
	double m_originx, m_originy, m_originz;

	/*!Դ�������������ϵĲ������
	\  y��z�����ϵ�λ���ף�x�����ϵ�λ���룬����Ҫ��x�������и�����ϵ��
	\  ��֤��ʾ����ӽ������塣
	\  m_gridSizey��m_gridSizezʵ�ʲ��������m_gridSizex�ǳ��Ա���ϵ��֮���
	\  ���������m_gridSizexFact��x����ʵ�ʲ������, m_ratiox��x���򱶳�ϵ��
	\
	*/
	double m_gridSizex, m_gridSizey, m_gridSizez, m_gridSizexFact, m_ratiox;
	

//˽�г�Ա����
private:

	int m_BLOCKSIZE;
	/*!
	\m_sourceFileName--Դ�����ļ�ȫ·����
	\m_octreeFileName--���ɵİ˲����ṹ�����ļ�ȫ·����
	*/
	QString m_sourceFileName, m_octreeFileName;

	/*!
	\brief ������¼���ǣ��߽�ֵ��maxֵ֮��Ĳ���������ǰ��չ����µı߽�ֵ
	\	   ��computeLimit�����м����
	\note  ���������ϵı߽�ֵ����octreeSize����2��n���ݵõ���0���߽�ֵ֮��Ĳ��ֿ���
	\	   �����ȫ�˲������߽�ֵ��maxֵ֮��Ĳ���������ǰ��չ���������������ڽ���ʱ
	\	   ��Ȼ���������н��棬���Ǳ߽�ֵ�̶����䣬�Ӷ����ӽڵ�ı߽��븸�ڵ�ı߽���ȫ��ͬ��
	*/
	int m_limitSample, m_limitCMP, m_limitRecord;//��ǰ��չ����µı߽�ֵ
	int m_oldSample, m_oldCMP, m_oldRecord;//û��չǰ�ı߽�ֵ

	/*!
	\brief �Ƿ�ʹ�ñ߽�ֵ��trueʹ�ã�false��ʹ��
	\note1 ���캯������true
	\	   ��ĳ�������ϵ�һ�λ��ֳ���������ʱ�����õ�limitֵ���Ժ���ʹ��
	\note2 ���жϱ�׼����©�������磬1����0246�飬��������һ��ʱ����Ҫ��x�����ϻ��֣�
	\	   �������ĸ��鶼Ҫ�õ�m_limitSampleֵ�����Ǵ��жϱ�׼ֻ����0��ʹ�����limitֵ��
	\	   ʣ���246���ʹ�ò������limitֵ�ˡ�
	\note3 �µ��жϱ�׼���ɸ�������ӿ�ʱ����x����Ϊ������minx < m_limitSample < maxx��
	\	   ��ʹ�ô�limitֵ
	*/
	bool flag_limitx, flag_limity, flag_limitz;

	//�˲����ļ��б���ֵ����Сֵ�����ֵ
	float m_minValue, m_maxValue;

};

END_KLDISPLAY3D_NAMESPACE
#endif
