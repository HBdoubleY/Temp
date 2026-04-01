// Queue.h: interface for the CQueue class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __CQUEUE_H__
#define __CQUEUE_H__
#include "sysarch.h"

class CQueue {
public:
	CQueue();
	virtual ~CQueue();
	bool Init(int nMaxMsgs);
	bool Append(void* pvMsg, DWORD dwMilliseconds);
	void* Remove(DWORD dwMilliseconds);
	void RemoveAll();
	int GetMsgNum() {
		return m_nMsgNum;
	}
	;
	bool IsFull() {
		return m_nMsgNum == m_nMaxMsgs;
	}
	;

protected:
	int m_nFirst;
	int m_nLast;
	int m_nMsgNum;
	void** m_pvMsgs;
	OS_SEM m_hsemMail;
	OS_SEM m_hsemSpace;
	OS_SEM m_hmtxQ;

	int m_nMaxMsgs;
};

#endif //CQUEUE_H

