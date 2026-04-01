#include "stdafx.h"
#include "string.h"
#include "LoopBuf.h"
#include "sysdebug.h"

////////////////////////////////////////////////////////////////////////////////////////////
//CLoopBuf

CLoopBuf::CLoopBuf() {
	m_pbLoopBuf = NULL;
}

CLoopBuf::~CLoopBuf() {
	if (m_pbLoopBuf != NULL) {
		delete[] m_pbLoopBuf;
	}
}

bool CLoopBuf::Init(WORD wBufSize) {
	m_wBufSize = wBufSize;
	m_pbLoopBuf = new BYTE[m_wBufSize];
	if (m_pbLoopBuf == NULL) {
		//Debug(DB_CRITICAL, ("CLoopBuf::Init: critical error : sys out of memory !!!!!!\r\n"));
		return false;
	}

	m_wRxHead = 0;
	m_wRxTail = 0;
	return true;
}

WORD CLoopBuf::GetBufLen() {
	if (m_wRxHead >= m_wRxTail) {
		return m_wRxHead - m_wRxTail;
	} else {
		return m_wRxHead + m_wBufSize - m_wRxTail;
	}
}

void CLoopBuf::PutToBuf(BYTE* pbBuf, WORD wLen) {
	WORD WBuffLen = 0;

	if (wLen > m_wBufSize) {
		//Debug(DB_FAPROTO, ("CLoopBuf::PutToLoopBuf over,wLen>m_wBufSize.\n"));
		return;
	}
	WBuffLen = (m_wRxHead + m_wBufSize - m_wRxTail) % m_wBufSize;
	if (wLen + WBuffLen > m_wBufSize) {
		//Debug(DB_FAPROTO, ("CLoopBuf::PutToLoopBuf over,(wLen+WBuffLen)>m_wBufSize.\n"));
		return;
	}

	if (m_wRxHead + wLen <= m_wBufSize) {
		memcpy(&m_pbLoopBuf[m_wRxHead], pbBuf, wLen);
	} else {
		memcpy(&m_pbLoopBuf[m_wRxHead], pbBuf, m_wBufSize - m_wRxHead);
		memcpy(m_pbLoopBuf, pbBuf + m_wBufSize - m_wRxHead, wLen - (m_wBufSize - m_wRxHead));
	}

	m_wRxHead += wLen;
	if (m_wRxHead >= m_wBufSize) {
		m_wRxHead -= m_wBufSize;
	}
}

WORD CLoopBuf::RxFromBuf(BYTE* pbRxBuf, WORD wBufSize) {
	WORD wRetLen;

	if (m_wRxHead != m_wRxTail)  
			{
		if (m_wRxHead > m_wRxTail) {
			wRetLen = m_wRxHead - m_wRxTail;
			if (wRetLen > wBufSize) {
				wRetLen = wBufSize;
			}

			memcpy(pbRxBuf, &m_pbLoopBuf[m_wRxTail], wRetLen);
			return wRetLen;
		} else {
			
			wRetLen = m_wBufSize - m_wRxTail;
			if (wRetLen >= wBufSize) {
				memcpy(pbRxBuf, &m_pbLoopBuf[m_wRxTail], wBufSize);
				return wBufSize;
			} else {
				memcpy(pbRxBuf, &m_pbLoopBuf[m_wRxTail], wRetLen);

				if (wRetLen + m_wRxHead > wBufSize) {
					memcpy(pbRxBuf + wRetLen, m_pbLoopBuf, wBufSize - wRetLen);
					return wBufSize;
				} else {
					memcpy(pbRxBuf + wRetLen, m_pbLoopBuf, m_wRxHead);
					return wRetLen + m_wRxHead;
				}
			}
		}
		
	} else {
		return 0;
	}
}

void CLoopBuf::DeleteFromBuf(WORD wLen) {
	WORD wLeft = GetBufLen();

	if (wLen > wLeft) {
		wLen = wLeft;
	}

	m_wRxTail = (m_wRxTail + wLen) % m_wBufSize;
}

void CLoopBuf::ClrBuf() {
	m_wRxHead = 0;
	m_wRxTail = 0;
}
