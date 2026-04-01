// Queue.cpp: implementation of the CQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Queue.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*OS_SEM OSSemCreate(WORD count);
 OS_SEM OSSemCreate(WORD count, WORD max);
 OS_SEM NewPeriodicSemaphore(DWORD dwMilliseconds);
 void OSSemDel(OS_SEM sem);
 void OSSemPend(OS_SEM sem);
 WORD OSSemPend(OS_SEM sem, DWORD timeout);
 void OSSemPost(OS_SEM sem);
 */

CQueue::CQueue() {
}

CQueue::~CQueue() {
	RemoveAll();
	delete[] m_pvMsgs;
}

bool CQueue::Init(int nMaxMsgs) {
	m_nMaxMsgs = nMaxMsgs;
	m_pvMsgs = new void*[m_nMaxMsgs];

	if (m_pvMsgs == NULL) {
		//Debug(1, ("CQueue::Init : critical error : sys out of memory !!!!!!!!!!!\r\n"));
		return false;
	}

	for (int i = 0; i < m_nMaxMsgs; i++)
			{
		m_pvMsgs[i] = NULL;
	}
	m_nFirst = m_nLast = 0;
	m_nMsgNum = 0;

	m_hsemSpace = OSSemCreate(m_nMaxMsgs, m_nMaxMsgs);
	m_hsemMail = OSSemCreate(0, m_nMaxMsgs);
	m_hmtxQ = OSSemCreate(1);

	return true;
}

void CQueue::RemoveAll() {
	while (Remove(1) != NULL) {
		;                      
	}
}

bool CQueue::Append(void* pvMsg, DWORD dwMilliseconds) {
	if (OSSemPend(m_hsemSpace, dwMilliseconds) != SYS_ERR_OK) {                                    
		//Debug(DB_QUEUE, ("CQueue::Append(): no space to append Queue %p. \r\n", this));
		return false;
	}

	OSSemPend(m_hmtxQ);      

	m_pvMsgs[m_nLast++] = pvMsg;  

	if (m_nLast == m_nMaxMsgs)    
			{
		m_nLast = 0;
	}

	m_nMsgNum++;

	OSSemPost(m_hsemMail);   
	OSSemPost(m_hmtxQ);     

	return true;
}

void* CQueue::Remove(DWORD dwMilliseconds) {
	if (OSSemPend(m_hsemMail, dwMilliseconds) != SYS_ERR_OK) 
	{
		//Debug(OP_DEBUG, ("CQueue::Remove(): no msg to remove from Queue %p. \r\n", this));
		return NULL;
	}

	OSSemPend(m_hmtxQ);       

	void* pvMsg;
	pvMsg = m_pvMsgs[m_nFirst];    
	m_pvMsgs[m_nFirst] = NULL;

	m_nFirst++;                    
	if (m_nFirst == m_nMaxMsgs)     
			{
		m_nFirst = 0;
	}

	m_nMsgNum--;

	OSSemPost(m_hsemSpace); 
	OSSemPost(m_hmtxQ);     

	return pvMsg;
}

