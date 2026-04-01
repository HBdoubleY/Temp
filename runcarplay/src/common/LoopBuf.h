#ifndef LOOPBUF_H
#define LOOPBUF_H
#include "apptypedef.h"

class CLoopBuf {
public:
	CLoopBuf();
	virtual ~CLoopBuf();

	//��������

	//������
	bool Init(WORD wBufSize);
	void PutToBuf(BYTE* pbBuf, WORD wLen);
	WORD RxFromBuf(BYTE* pbRxBuf, WORD wBufSize);
	void DeleteFromBuf(WORD wLen);
	WORD GetBufLen();
	void ClrBuf();

protected:
	//��������
	WORD m_wBufSize;
	WORD m_wRxHead;
	WORD m_wRxTail;
	BYTE* m_pbLoopBuf;    //ѭ��������
};

#endif //LOOPBUF_H
