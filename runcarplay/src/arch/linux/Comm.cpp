// Comm.cpp: implementation of the CComm class.
//
//////////////////////////////////////////////////////////////////////

#include "Comm.h"
//#include "Uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "sysapi.h"
#include "sysdebug.h"
//#include "board.h"
//#include "Info.h"
//#include "FaConst.h"

CComm::CComm() {
	m_wPort = 100;          //m_pCSIO = NULL;
	m_dwTimeouts = 1000;        //ms
	m_dwBaudRate = B9600;       //B9600;		//CBR_9600;     //ly modify
	m_bByteSize = 8;
	m_bStopBits = ONESTOPBIT;   //ly modify
	m_bParity = NOPARITY;       //ly modify

	Comm_fd = -1;               //ly add
	m_fOpen = false;
	CreateSemaphore();
}

CComm::~CComm() {
	//CloseDev();
	DeleteSemaphore();
}

void CComm::CreateSemaphore(void) {
	m_semRead = OSSemCreate(1);
	m_semWrite = OSSemCreate(1);
	//printf("already new create Read_sem & Write_sem!\r\n");
}

void CComm::DeleteSemaphore(void) {
	OSSemDel(m_semRead);
	OSSemDel(m_semWrite);
	//printf("already free Read_sem & Write_sem!\r\n");
}

void CComm::Config(WORD wPort, DWORD dwBaudRate, BYTE bByteSize, BYTE bStopBits, BYTE bParity) {
	m_wPort = wPort;
	m_dwBaudRate = dwBaudRate;
	m_bByteSize = bByteSize;
	m_bStopBits = bStopBits;
	m_bParity = bParity;
}

void CComm::Config(TCommPara& CommPara) {
	m_wPort = CommPara.wPort;
	m_dwBaudRate = CommPara.dwBaudRate;
	m_bByteSize = CommPara.bByteSize;
	m_bStopBits = CommPara.bStopBits;
	m_bParity = CommPara.bParity;
}

bool CComm::GetCommPara(TCommPara* pCommPara) {
	if (IsOpen()) {
		pCommPara->wPort = m_wPort;
		pCommPara->dwBaudRate = m_dwBaudRate;
		pCommPara->bByteSize = m_bByteSize;
		pCommPara->bStopBits = m_bStopBits;
		pCommPara->bParity = m_bParity;
		return true;
	} else {
		return false;
	}
}

int CComm::OpenDev(const char *Dev) {
	int fd = open(Dev, O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY, 0);

	if (fd < 0) {
		Debug(DEBUG_CRITICAL, ("Can't Open Serial Port %s\r\n", Dev));
		return -1;
	} else {
		return fd;
	}
}

bool CComm::Close() {
	bool fRet = false;

	if (Comm_fd < 0) {
		return true;
	}

	int ret = close(Comm_fd);
	if (ret < 0) {
		Debug(DEBUG_CRITICAL, ("Close Serial Port Fail!\r\n"));
	} else {
		Comm_fd = -1;
		fRet = true;
	}

	m_fOpen = false;
	return fRet;
}

DWORD CComm::ReadFunc(unsigned char *buf, INT16U len, DWORD dwTimeouts) {
	int rval = 0;
	int iCnt = -1;
	bool fRecvd = false;

	WORD wInterV;

	if (m_dwBaudRate < B1200) {
		wInterV = 200;
	} else if (m_dwBaudRate < B9600) {
		wInterV = 150;
	} else {
		wInterV = 100;
	}

	if (!m_fOpen) {
		Debug(DEBUG_CRITICAL, ("CComm::ReadFunc : read fail, port have not opened!\r\n"));
		return 0;
	}
	OSSemPend(m_semRead);

	DWORD dwTick = GetTick();
	do {
		iCnt = read(Comm_fd, buf + rval, len - rval);
		if (iCnt > 0) {
			rval += iCnt;
			fRecvd = true;
		}
		if (fRecvd && iCnt <= 0) {
			iCnt = read(Comm_fd, buf + rval, len - rval);
			if (iCnt <= 0) {
				break;
			}
			rval += iCnt;
		}
		Sleep(wInterV);
	} while (GetTick() - dwTick < dwTimeouts);

	OSSemPost(m_semRead);

	return rval;
}

DWORD CComm::WriteFunc(unsigned char *buf, int Txlen) {
	int n, written = 0;

	if (!m_fOpen) {
		Debug(DEBUG_CRITICAL, ("CComm::WriteFunc : write fail, port have not opened!\r\n"));
		return -1;
	}
	OSSemPend(m_semWrite);
	while (written < Txlen) {
		n = write(Comm_fd, buf + written, Txlen - written);
		if (n == -1) {
			Debug(DEBUG_CRITICAL, ("CComm::WriteFunc : Wirte data error(%d) uart_%d!\r\n", errno, m_wPort));
			Close();
			Open();
			OSSemPost(m_semWrite);
			return -1;
		}
		written += n;
	}

	//OSSemPost(m_semWrite);
	OSSemPost(m_semWrite);
	return written;
}

bool CComm::SetComm(DWORD whichbaud, BYTE databits, BYTE stopbits, BYTE parity) {
	//1 stop bit,no parity, 8 bit data
	struct termios tty;

	if (tcgetattr(Comm_fd, &tty) != 0) {
		Debug(DEBUG_CRITICAL, ("CComm::SetComm : tcgetattr fail!\r\n"));
		return false;
	}

	tty.c_iflag = IGNBRK | IGNPAR;
	tty.c_oflag = 0;
	tty.c_lflag = 0;
	tty.c_line = 0;
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;
	//	CS8: 8n1
	//	CLOCAL:
	//	CREAD:
	//tty.c_cflag = CS8|CREAD|CLOCAL;//|HUPCL;

	//tty.c_cflag &= ~CSIZE;    //ly add 2006-06-10 18:09
	switch (whichbaud) {
	case B115200:
		break;
	case B57600:
		break;
	case B38400:
		break;
	case B19200:
		break;
	case B9600:
		break;
	case B4800:
		break;
	case B2400:
		break;
	case B1200:
		break;
	case B300:
		break;
	default:
		Debug(DEBUG_CRITICAL, ("CComm::SetComm : Baud %d is not support. use default baud B9600\r\n", whichbaud));
		whichbaud = B9600;
		break;      // default is 9600
	}
	//tty.c_cflag |= whichbaud;

	tcflush(Comm_fd, TCIOFLUSH);
	cfsetispeed(&tty, whichbaud);
	cfsetospeed(&tty, whichbaud);
	tcflush(Comm_fd, TCIOFLUSH);

	//tty.c_ispeed = whichbaud;
	//tty.c_ospeed = whichbaud;

	tty.c_cflag &= ~CSIZE;  //ly add 2006-06-10 18:09
	switch (databits) {
	case 7:
		tty.c_cflag |= CS7;
		break;
	case 8:
		tty.c_cflag |= CS8;
		break;
	default:
		Debug(DEBUG_CRITICAL, ("CComm::SetComm : Unsupported data size!\r\n"));
		return false;
	}

	switch (parity) {
	case NOPARITY:          //0
		tty.c_cflag &= ~PARENB;                 // Clear parity enable
		tty.c_iflag &= ~INPCK;                  // Disnable parity checking
		break;
	case ODDPARITY:         //1
		tty.c_cflag |= (PARODD | PARENB);
		tty.c_iflag |= INPCK;                   // Enable parity checking
		break;
	case EVENPARITY:        //2
		tty.c_cflag |= PARENB;                  // Enable parity
		tty.c_cflag &= ~PARODD;
		tty.c_iflag |= INPCK;                   // Enable parity checking
		break;
	case 'S':
	case 's':      //as no parity
		tty.c_cflag &= ~PARENB;
		tty.c_cflag &= ~CSTOPB;
		break;
	default:
		Debug(DEBUG_CRITICAL, ("CComm::SetComm : Unsupported parity!\r\n"));
		return false;
	}

	switch (stopbits) {
	case 1:
		tty.c_cflag &= ~CSTOPB;
		break;
	case 2:
		tty.c_cflag |= CSTOPB;
		break;
	default:
		Debug(DEBUG_CRITICAL, ("CComm::SetComm : Unsupported stop bits!\r\n"));
		return false;
	}
	// Set input parity option
	if (parity != 'n') {
		tty.c_iflag |= INPCK;
	}
	//tty.c_cc[VTIME] = 150; // 15 seconds
	//tty.c_cc[VMIN] = 0;

	tcflush(Comm_fd, TCIFLUSH); // Update the tty and do it NOW

	// TCSANOW
	// TCSAFLUSH
	if (tcsetattr(Comm_fd, TCSANOW, &tty) != 0) {
		Debug(DEBUG_CRITICAL, ("CComm::SetComm : tcsetattr fail!\r\n"));
		return false;
	}

	m_dwBaudRate = whichbaud;
	m_bByteSize = databits;
	m_bStopBits = stopbits;
	m_bParity = parity;

	return true;
}

int CComm::GetCommFd() {
	return Comm_fd;
}

bool CComm::Open(unsigned short wPort, unsigned long dwBaudRate, unsigned char bByteSize, unsigned char bStopBits,
		unsigned char bParity) {
	const char *dev[] = { "/dev/carPLUS0", "/dev/carPLUS1", "/dev/carPLUS2", "/dev/carPLUS3", "/dev/ttyS5",
			"/dev/ttyS6", "/dev/ttyUSB5", "/dev/ttyS0", "/dev/mux0", "/dev/mux1", "/dev/mux2", };

	m_fOpen = false;

	if (wPort > 11) {
		return false;
	}

	Comm_fd = OpenDev(dev[wPort]);
	if (Comm_fd < 0) {
		return false;
	}

	m_fOpen = SetComm(dwBaudRate, bByteSize, bStopBits, bParity);

	if (m_fOpen) {
		m_wPort = wPort;
	}

	return m_fOpen;
}

bool CComm::Open() {
	const char *dev[] = { "/dev/carPLUS0", "/dev/carPLUS1", "/dev/carPLUS2", "/dev/carPLUS3", "/dev/ttyS5",
			"/dev/ttyS6", "/dev/ttyUSB5", "/dev/ttyS0", "/dev/mux0", "/dev/mux1", "/dev/mux2", };

	if (m_wPort > 11) {
		return false;
	}
	m_fOpen = false;

	Comm_fd = OpenDev(dev[m_wPort]);
	if (Comm_fd < 0) {
		return false;
	}

	m_fOpen = SetComm(m_dwBaudRate, m_bByteSize, m_bStopBits, m_bParity);

	return m_fOpen;
}

bool CComm::SetBaudRate(unsigned long dwBaudRate) {
	if (m_fOpen == false) {
		return false;
	}

	return SetComm(dwBaudRate, m_bByteSize, m_bStopBits, m_bParity);    //ly add
}
