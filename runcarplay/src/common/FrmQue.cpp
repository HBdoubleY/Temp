#include "stdafx.h"
#include "FrmQue.h"

/////////////////////////////////////////////////////////////////////////
//CFrmQue

CFrmQue::CFrmQue() {
	m_pwFrmBytes = NULL;
	m_pbFrms = NULL;
}

CFrmQue::~CFrmQue() {
	if (m_pwFrmBytes != NULL) {
		delete[] m_pwFrmBytes;
	}

	if (m_pbFrms != NULL) {
		delete[] m_pbFrms;
	}
}

//����:��ʼ��
//����:@wMaxFrms ����ܻ����֡����
//	   @wFrmSize ÿ֡����ܻ�����ֽ���
bool CFrmQue::Init(WORD wMaxFrms, WORD wFrmSize) {
	m_wMaxFrms = wMaxFrms;      //����ܻ����֡����
	m_wFrmSize = wFrmSize;      //ÿ֡����ܻ�����ֽ���

	m_pwFrmBytes = new WORD[m_wMaxFrms];         //ÿ��֡��ʵ���ֽ���
	m_pbFrms = new BYTE[(DWORD) m_wFrmSize * m_wMaxFrms]; //�������һ������֡

	if (m_pwFrmBytes == NULL || m_pbFrms == NULL) {
		//Debug(1, ("CFrmQue::Init : critical error : sys out of memory !!!!!!!!!!!\r\n"));
		return false;
	}

	m_wFirst = m_wLast = 0;
	m_wFrmNum = 0;

	m_hsemSpace = OSSemCreate(m_wMaxFrms, m_wMaxFrms);
	m_hsemMail = OSSemCreate(0, m_wMaxFrms);
	m_hmtxQ = OSSemCreate(1);

	return true;
}

//������ɾ��������ȫ��ָ�룬���ͷ�����ռ�õĿռ�
void CFrmQue::RemoveAll() {
	while (Remove(NULL, 1) != 0) {
		;                           //LINUXƽ̨��0��ʾ���޵ȴ�,���Ա�����1
	}
}

bool CFrmQue::Append(BYTE* pbFrm, WORD wLen, DWORD dwMilliseconds) {
	if (OSSemPend(m_hsemSpace, dwMilliseconds) != SYS_ERR_OK) {                                 //�ȵȴ��������пռ���ʼ�
		//Debug(DB_QUEUE, ("CFrmQue::Append(): no space to append Queue %p. \r\n", this));
		return false;
	}

	OSSemPend(m_hmtxQ);        //ȡ�ö������ռ��

	memcpy(&m_pbFrms[(DWORD) m_wFrmSize * m_wLast], pbFrm, wLen);
	m_pwFrmBytes[m_wLast] = wLen;

	m_wLast++;
	if (m_wLast >= m_wMaxFrms)     //Խ��ѭ�����еı߽�
			{
		m_wLast = 0;
	}

	m_wFrmNum++;

	OSSemPost(m_hsemMail);   //��һ���µ���Ϣ�ˣ�
	OSSemPost(m_hmtxQ);      //�ͷŶ������ռ��

	return true;
}

WORD CFrmQue::Remove(BYTE* pbFrm, DWORD dwMilliseconds) {
	if (OSSemPend(m_hsemMail, dwMilliseconds) != SYS_ERR_OK) //�ȵȴ����������ʼ�
	{
		//Debug(OP_DEBUG, ("CFrmQue::Remove(): no msg to remove from Queue %p. \r\n", this));
		return 0;
	}

	OSSemPend(m_hmtxQ);            //ȡ�ö������ռ��

	WORD wLen = m_pwFrmBytes[m_wFirst];
	if (pbFrm != NULL) {
		memcpy(pbFrm, &m_pbFrms[(DWORD) m_wFrmSize * m_wFirst], wLen); //������Ϣ

	}
	m_wFirst++;                     //βָ��++����ʹ����msg==NULL��++��ʾ���
	if (m_wFirst >= m_wMaxFrms)     //Խ��ѭ�����еı߽�
			{
		m_wFirst = 0;
	}

	m_wFrmNum--;

	OSSemPost(m_hsemSpace);  //���µĿռ��ˣ�
	OSSemPost(m_hmtxQ);      //�ͷŶ������ռ��

	return wLen;
}

