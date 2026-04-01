#ifndef FRMQUE_H
#define FRMQUE_H
#include "apptypedef.h"
#include "sysarch.h"

class CFrmQue {
public:
	CFrmQue();
	virtual ~CFrmQue();
	bool Init(WORD wMaxFrms, WORD wFrmSize);
	bool Append(BYTE* pbFrm, WORD wLen, DWORD dwMilliseconds);
	WORD Remove(BYTE* pbFrm, DWORD dwMilliseconds);
	void RemoveAll();
	int GetMsgNum() {
		return m_wFrmNum;
	}
	;
	bool IsFull() {
		return m_wFrmNum == m_wMaxFrms;
	}
	;

protected:
	OS_SEM m_hsemMail;
	OS_SEM m_hsemSpace;
	OS_SEM m_hmtxQ;

	WORD m_wFrmSize;        //ÿ֡����ܻ�����ֽ���
	WORD m_wMaxFrms;        //����ܻ����֡����
	WORD m_wFrmNum;         //ʵ�ʻ����֡����
	WORD m_wFirst;
	WORD m_wLast;

	BYTE* m_pbFrms;        //�������һ������֡
	WORD* m_pwFrmBytes;    //ÿ��֡��ʵ���ֽ���
};

#endif //FRMQUE_H

