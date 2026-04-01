
#include "DHCryptlib.h"
#include <string.h>
#include <apptypedef.h>
#include <comm_uart.h>
struct Dev_Frame{	
	BYTE bHead1;

	BYTE bLen11;    
	BYTE bLen12;
	BYTE bLen21;
	BYTE bLen22;

	BYTE bHead2;
	BYTE  bCmd;
	BYTE  bDataLen;
	BYTE  bdata[2]; 

};

int MameDevFrm(BYTE bCmd,  BYTE* pbNr, BYTE bLen,  BYTE* pbSend)
{
   
	BYTE CS = 0;
	int i = 0;  
   
	pbSend[0] = 0x68;
	pbSend[1] = bLen +10;
	pbSend[2] = 0;
	pbSend[3] = bLen +10;
	pbSend[4] = 0;
	pbSend[5] = 0x68;

	pbSend[6] = bCmd;

	pbSend[7] = bLen;
	for (i = 0; i < bLen; i++)
		pbSend[8+i] = pbNr[i];

	for (i = 0; i < 8 + bLen; i++)
		CS += pbSend[i];

	pbSend[8 + bLen] = CS;
	pbSend[9 + bLen] = 0x16;
    
	return 10 + bLen;

}

BYTE GetSum( BYTE *data, unsigned int len )
{
    BYTE sum = 0, i;
    
    for( i = 0; i < len; i++ )
    {       
		sum += *(data++);
    }
    
    return sum;
}

int GetSeries(BYTE* pbSeries)// 此处需要改写，得到设备系列号
{
   BYTE bSeries[16];
   memset(bSeries,  0, 16);// 此处需要改写，得到设备系列号
   Getflash_id(pbSeries);
   return 16;
}


void SetAuthoKey(BYTE* pbAuthoKey , BYTE bLen)//设置授权码，此处需要改下保存在设备
{
	int fd = -1;
	system("touch /opt/work/app/license");
	fd = open("/opt/work/app/license",O_RDWR);
	if(fd > 0){
		write(fd,pbAuthoKey,bLen);
	}
	close(fd);
	system("sync");
}

//设置授权码，此处需要改下保存在设备
int xcb_check_validation(char* path)
{
	int fd = -1;
	BYTE bAuthoKey[128];
	//system("touch /opt/work/app/license");
	fd = open("/opt/work/app/license",O_RDONLY);
	if(fd > 0){
		read(fd,bAuthoKey,24);
		close(fd);
		if(JugeAuthKey(bAuthoKey,24)){
			return 1;
		}
	}
	return -1;
}

BOOL JugeAuthKey( BYTE* pbAuthoKey , BYTE bLen)//判断授权码是否合法
{
	unsigned char bKey[16]={0x11, 0x22,0x33, 0x44, 0x55, 0x66,0x77, 0x88, 0x11, 0x22,0x33, 0x44, 0x55, 0x66,0x77, 0x88};//授权加密key, 这个固定，跟授权服务器一致就可以。
	BYTE sPlain[16];
	BYTE bSeries[16];
	GetSeries(bSeries);
	aes_unwrap_byte(bKey,  pbAuthoKey, bLen,  sPlain);
	if (memcmp(sPlain, bSeries, bLen-8) == 0)
		return true;

	return false;
}

int Dev_Receive(BYTE* pbBuf, BYTE bLen)//接收上位机帧接口
{	
	struct Dev_Frame* pFrame = NULL;
	BYTE bSeries[16];
	BYTE bCipherSeries[16+8];
	BYTE bTx[200];
	BYTE bTxLen = 0;
	BYTE bData[2];
	BYTE bKey[16]={0,0,0,0,0,0,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33};//加密系列号的KEY
    int iSeriesLen = 0;
	printf("pbBuf[0]= %c,pbBuf[1]= %c,pbBuf[2]= %c\n",pbBuf[0],pbBuf[1],pbBuf[2]);
	while(pbBuf[0]==0xfe && bLen>0)
	{
		pbBuf++; bLen--;
	}

	if (pbBuf[1] != bLen || pbBuf[3] != bLen){//长度不对
		printf("bLen err !!!! \n");
		return -1;
	}

	if (GetSum(pbBuf, bLen-2) != pbBuf[bLen-2]){//校验不对
		printf("crc err !!!! \n");
		return -1;
	}

	bData[0] = 0;

	pFrame= (struct Dev_Frame*)pbBuf;
	if ( pFrame->bCmd == 0x11)//读系列号
	{
		memcpy(bKey, &pFrame->bdata[0], 6);//动态Key
		iSeriesLen  = GetSeries(&bSeries[0]);//取得设备序列号
		aes_wrap_byte(bKey, bSeries, iSeriesLen,  bCipherSeries);
		bTxLen = MameDevFrm(0x91,  bCipherSeries, iSeriesLen+8,  bTx);
		if(bTxLen > 0)
		{
			SendFrm(bTx,bTxLen);
		}
		return 0;
	}
	else if ( pFrame->bCmd == 0x14)//设置授权码
	{
		
		if(JugeAuthKey(&pFrame->bdata[0] , pFrame->bDataLen) == true)
		{
			SetAuthoKey(&pFrame->bdata[0] , pFrame->bDataLen);//把下发来的授权码保持起来
			bTxLen = MameDevFrm(0x94,  bData, 1,  bTx);
			//需要改下，通过串口发送出去
		
			SendFrm(bTx,bTxLen);
			
			return 1;
		}
		bTxLen = MameDevFrm(0xD4,  bData, 1,  bTx);
		//需要改下，通过串口发送出去
		
		SendFrm(bTx,bTxLen);	
		
	}
	return 0;
}


int uart_fd = -1;
void openPort(int port) {
	if (port == 1) {
		uart_fd = open("/dev/ttyS1", O_RDWR | O_NDELAY, 0);
		if (uart_fd < 0) {
			printf("Open comm 1 error \n");
		}
		comm_uart uart;
		uart.set_parity(uart_fd, 8, 1, 'n');
		uart.set_speed(uart_fd, B19200);
	}
}

void closePort(int port) {
//#ifdef SYS_LINUX
	
	if (port == 1) {
		close(uart_fd); 
	}
//#endif
}

int SendFrm(unsigned char data[], unsigned int dataSize) {
	int frmSize = 0;
	unsigned char buff[256 + 4];
	if (dataSize > 0) {
		memcpy(&buff[frmSize], data, dataSize);
		frmSize += dataSize;
	}
	if (1) {
		printf("fh->mz: ");
		for (int i = 0; i < frmSize; i++) {
			printf("%02X ", buff[i]);
		}
		printf("\n");
	}

	if (uart_fd < 0) {
		return false;
	}
	write(uart_fd, buff, frmSize);
	return true;
}

int Getflash_id(BYTE *buf)
{
   int fd;
   //memset(buf,0,sizeof(buf));
   fd = open("/proc/device-tree/spi-nor-controller@1F000000/spi-nor@0/unique-id0", O_RDONLY); 
   if (fd < 0) {  
        perror("Failed to open /proc/device-tree/spi-nor-controller@1F000000/spi-nor@0/unique-id0!");  
        return -1;
   }
   if (read(fd, buf, 4) < 0) {  
        perror("Failed to read 0value!");  
        return -1;  
   } 
   close(fd); 



   fd = open("/proc/device-tree/spi-nor-controller@1F000000/spi-nor@0/unique-id1", O_RDONLY); 
   if (fd < 0) {  
        perror("Failed to open /proc/device-tree/spi-nor-controller@1F000000/spi-nor@0/unique-id1!");  
        return -1;
   }
   if (read(fd, &buf[4], 4) < 0) {  
        perror("Failed to read 1value!");  
        return -1;  
   }
   close(fd);


   fd = open("/proc/device-tree/spi-nor-controller@1F000000/spi-nor@0/unique-id2", O_RDONLY); 
   if (fd < 0) {  
        perror("Failed to open /proc/device-tree/spi-nor-controller@1F000000/spi-nor@0/unique-id2!");  
        return -1;
   }
   if (read(fd, &buf[8], 4) < 0) {  
        perror("Failed to read 2value!");  
        return -1;  
   }
   close(fd);


   fd = open("/proc/device-tree/spi-nor-controller@1F000000/spi-nor@0/unique-id3", O_RDONLY); 
   if (fd < 0) {  
        perror("Failed to open /proc/device-tree/spi-nor-controller@1F000000/spi-nor@0/unique-id3!");  
        return -1;
   }
   if (read(fd, &buf[12], 4) < 0) {  
        perror("Failed to read 3value!");  
        return -1;  
   }
   close(fd);

   for(int i = 0; i <4 ;i++){
	   printf("flash_id0_buf[%d]= %x\n",i,buf[i]);
   }

   for(int i = 4; i <8 ;i++){
	   printf("flash_id1_buf[%d]= %x\n",i,buf[i]);
   }

   for(int i = 8; i <12 ;i++){
	   printf("flash_id2_buf[%d]= %x\n",i,buf[i]);
   }

   for(int i = 12; i <16 ;i++){
	   printf("flash_id3_buf[%d]= %x\n",i,buf[i]);
   }
   return 0;
}

int aes_is_file_exist(char *path){

	int fRet = -1;
	int len = 0;
	INT8U tmpbBuf[512];

	if (uart_fd < 0) {
		return -1;
	}

	do{
		int tmplen = read(uart_fd, tmpbBuf+len, 512 - 10);
		if (tmplen <= 0) {
			break;
		}
		len += tmplen;
		usleep(100*1000);
	}while(1);

	if (len <= 0) {
		return -1;
	}

	if (1) {
		printf("mz->fh: ");
		for (int i = 0; i < len; i++) {
			printf("%02X ", tmpbBuf[i]);
		}
		printf("\n");
	}

	if (len > 0)
	{
		fRet = Dev_Receive(tmpbBuf, len);
	}
	return fRet;
}


int RcvBlock(char* pbBlock, int nLen){
	return 0;
}
