/*
 * =====================================================================================
 *
 *       Filename:  libserial.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2011年03月10日 09时23分25秒 UTC
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:
 *        Company:
 *
 * =====================================================================================
 */
#include "SerialLocal.h"
#include "../link/typedefine.h"
#include "../link/myprintf.h";
#include "Publics.h"

#pragma pack(1)

//报文接收结构
typedef struct{
	uchar	stx;
	uchar	node;
	uchar	serialno;
	uchar	cla;
	uchar	ins;
	uchar	p1;
	uchar	p2;
	uchar	tm[7];
	ushort	len;//Hi
} TPKGHEADER_RECV, *LPTPKGHEADER_RECV;

//报文发送结构
typedef struct{
	uchar	stx;
	uchar	node;
	uchar	serialno;
	uchar	cla;
	uchar	ins;
	ushort	len;//Hi
} TPKGHEADER_SEND, *LPTPKGHEADER_SEND;

#pragma pack()

#define ERR_RECV_TIMEOUT -1 //接收超时
#define ERR_RECV_FULL -2 //数据接收缓冲区满
#define ERR_RECV_PKGCRC -3 //数据校验错
#define ERR_RECV_CONTINUE -6 //接收未完成，继续
#define ERR_RECV_TIMEOUT_NEED_RESET -8 //接收超时,需要复位

#define CLA_FILE_COUNT 3 //文件类可用函数数量
#define CLA_BUSSINE_COUNT 3 //文件类可用函数数量

#ifdef _WIN32
const int speed_arr[] = {CBR_128000, CBR_115200, CBR_57600, CBR_38400, CBR_19200, CBR_9600, CBR_4800, CBR_2400, CBR_1200, CBR_300, 0};
#else
const int speed_arr[] = {B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300,B0};
#endif
const int name_arr[] = {230400, 115200, 57600, 38400,  19200,  9600, 4800, 2400, 1200,  300, 0};

static int g_fd=-1;
static int g_fd_ble=-1;

unsigned short int pkg_crc16_local(const uchar *ptr, unsigned int len)
{
	static unsigned short int const crc_ta[16]=
	{
		0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
		0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef
	};

    ushort crc;
    uchar da;


    crc=0;
    while (len--!=0)
    {
        da=crc>>12;
        crc<<=4;
        crc^=crc_ta[da^(*ptr>>4)];

        da=crc>>12;
        crc<<=4;
        crc^=crc_ta[da^(*ptr&0x0f)];
        ptr++;
    }
    return(crc);
}

//===================================================
//函数:listenpackchk
//功能:对上位机下发的数据进行校验，当数据包合法时，对数据内容进行处理并回传
//参数:
//     入口参数:len-接收的数据总长度, data-接收的数据内容
//     出口参数:当返回值=0时，参数有效，len-当前有完整数据包长度(不含校验字), data-数据内容
//返回值:
//     0-数据有效
//     1-数据包头错
//     -2 - 长度不足
//     -3 - 校验错
//====================================================
int listenpackchk_local(unsigned int nlen, uchar *buffer)
{
//	    uchar *pbuf=(uchar *)buffer;
//	    if (pbuf[nlen-1]==0x0d)
//	        return(0);
//	    else
//	        return(1);
    LPTPKGHEADER_RECV lpbuf=(LPTPKGHEADER_RECV)buffer;
    //uint headerlen_rx=sizeof(TPKGHEADER_RECV);
    //uchar *pcontent_rx=(uchar *)(lpbuf+headerlen_rx);

    uint pkglen;
    ushort crc1, crc2;
    int nret;


    do
    {

        if (nlen<sizeof(TPKGHEADER_RECV))
        {
            //如果当前接收的长度小于报文长度则数据包还未接收完整
            nret = ERR_RECV_CONTINUE;
            break;
        }


        pkglen = lpbuf->len;

        if (nlen>=(pkglen+18))
        {

            crc2 = pkg_crc16_local(buffer,pkglen+16); //校验数据从长度(2byte) 开始到校验码(1byte) 前(pklen+2-1)
            crc1 = buffer[pkglen+17];
            crc1<<=8;
            crc1 += buffer[pkglen+16];

            if (crc1==crc2)
            {
                nret = 0;
            }
            else
            {
                nret = ERR_RECV_PKGCRC;
            }
        }
        else
        {
            nret = ERR_RECV_CONTINUE;
        }


    }
    while (0);

    return(nret);

}


int libserial_save_set_local(struct termios *save_tios)
{
#ifdef _WIN32

#else
	if (tcgetattr(g_fd, save_tios) < 0)
        return -1;
#endif
    return 0;
}

int libserial_save_set_local_ble(struct termios *save_tios)
{
#ifdef _WIN32

#else
	if (tcgetattr(g_fd, save_tios) < 0)
        return -1;
#endif
    return 0;
}

int libserial_re_set_local(struct termios *save_tios)
{
#ifdef _WIN32

#else
	if (tcsetattr(g_fd, TCSANOW, save_tios) < 0)
        return -1;
#endif
    return 0;
}

int libserial_re_set_local_ble(struct termios *save_tios)
{
#ifdef _WIN32

#else
	if (tcsetattr(g_fd_ble, TCSANOW, save_tios) < 0)
        return -1;
#endif
    return 0;
}

//=================================================================================================
// 串口初始化函数。
// dev：串口设备名，串口1是"/dev/ttyS0"，串口2是"/dev/ttyS1"，依此类推。
// speed：串口波特率，可以是230400, 115200, 57600, 38400,  19200,  9600, 4800, 2400, 1200, 300, 0。
// parity：奇偶校验。值为'N','E','O','S'。
// databits：数据位，值是5、6、7或者8。
// stopbits：停止位，值是1或者2。
// hwf：硬件流控制。1为打开，0为关闭。
// swf：软件流控制。1为打开，0为关闭。
//==================================================================================================
int libserial_init_local(char *dev, int speed, int parity, int databits, int stopbits, int hwf, int swf)
{
	HS_LOG("dev=%s   \n",dev);

    int i	= 0;
	int ret = 0;

#ifdef _WIN32

#else
    struct termios tty;

    g_fd = open(dev, O_RDWR | O_NOCTTY);

	//dbg_formatvar("g_fd=%d",g_fd);

	HS_LOG("g_fd=%d   \n",g_fd);

    if (g_fd == -1)
    {
		dbg_formatvar("-1");
		
		HS_LOG(" g_fd %d dev=%s error \n",g_fd,dev);
        return -1;
    }

    ret = tcgetattr(g_fd, &tty);

    if (ret<0)
    {
    
		HS_LOG("tcgetattr ret %d error \n",ret);
		dbg_formatvar("-2");
        close(g_fd);
        g_fd = -1;
        return -1;
    }
    //设置波特率
    for (i=0; i<(int)(sizeof(speed_arr)/sizeof(int)); i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&tty, speed_arr[i]);
            cfsetospeed(&tty, speed_arr[i]);
            break;
        }
        if (name_arr[i] == 0)
        {
//            printf("speed %d is not support,set to 9600\n",speed);
            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);
        }
    }

    //设置数据位
    switch (databits)
    {
    case 5:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
        break;
    case 6:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
        break;
    case 7:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
        break;
    case 8:
    default:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        break;
    }

    //设置停止位
    if (stopbits == 2)
        tty.c_cflag |= CSTOPB;
    else
        tty.c_cflag &= ~CSTOPB;

    //设置奇偶校验
    switch (parity)
    {
        //无奇偶校验
    case 'n':
    case 'N':
        tty.c_cflag &= ~PARENB;   /* Clear parity enable */
        tty.c_iflag &= ~INPCK;    /* Enable parity checking */
        break;
        //奇校验
    case 'o':
    case 'O':
        tty.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
        tty.c_iflag |= INPCK;             /* Disable parity checking */
        break;
        //偶校验
    case 'e':
    case 'E':
        tty.c_cflag |= PARENB;    /* Enable parity */
        tty.c_cflag &= ~PARODD;   /* 转换为偶效验*/
        tty.c_iflag |= INPCK;     /* Disable parity checking */
        break;
        //等效于“无奇偶校验”
    case 'S':
    case 's':  /*as no parity*/
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        break;
    default:
        tty.c_cflag &= ~PARENB;   /* Clear parity enable */
        tty.c_iflag &= ~INPCK;    /* Enable parity checking */
        break;
    }

    //设置硬件流控制
    if (hwf)
        tty.c_cflag |= CRTSCTS;
    else
        tty.c_cflag &= ~CRTSCTS;

    /*if (hwf) */
    /*tty.c_cflag |= CIREN;*/

    //设置软件流控制
    if (swf)
        tty.c_iflag |= IXON | IXOFF;
    else
        tty.c_iflag &= ~(IXON|IXOFF|IXANY);

    //设置为RAW模式
    tty.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC |
                     IXANY | IXON | IXOFF | INPCK | ISTRIP);
    tty.c_iflag |= (BRKINT | IGNPAR);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(XCASE|ECHONL|NOFLSH);
    tty.c_lflag &= ~(ICANON | ISIG | ECHO);
    tty.c_cflag |= (CLOCAL | CREAD);

    //tty.c_cc[VTIME] = 1;   /*（非阻塞模式）操作超时，单位为0.1秒*/
    //tty.c_cc[VMIN] = 1;   /* 1为阻塞模式，0为非阻塞模式*/
    tty.c_cc[VTIME] = 2;   /*（非阻塞模式）操作超时，单位为0.1秒*/
    tty.c_cc[VMIN] = 0;   /* 1为阻塞模式，0为非阻塞模式*/

    //设置串口属性
    ret = tcsetattr(g_fd, TCSANOW, &tty);

    if (ret<0)
    {
    
		HS_LOG("tcsetattr  ret %d error \n",ret);
		dbg_formatvar("-3");
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    tcflush(g_fd, TCIOFLUSH);
#endif

    return g_fd;
}


int libserial_init_local_ble(char *dev, int speed, int parity, int databits, int stopbits, int hwf, int swf)
{

	HS_LOG("dev=%s   \n",dev);

    int i	= 0;
	int ret = 0;

#ifdef _WIN32

#else
    struct termios tty;

    g_fd_ble = open(dev, O_RDWR | O_NOCTTY);

    if (g_fd_ble == -1)
    {
		dbg_formatvar("-1");
        return -1;
    }

    ret = tcgetattr(g_fd_ble, &tty);

    if (ret<0)
    {
		dbg_formatvar("-2");
        close(g_fd_ble);
        g_fd = -1;
        return -1;
    }
    //设置波特率
    for (i=0; i<(int)(sizeof(speed_arr)/sizeof(int)); i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&tty, speed_arr[i]);
            cfsetospeed(&tty, speed_arr[i]);
            break;
        }
        if (name_arr[i] == 0)
        {
//            printf("speed %d is not support,set to 9600\n",speed);
            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);
        }
    }

    //设置数据位
    switch (databits)
    {
    case 5:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
        break;
    case 6:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
        break;
    case 7:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
        break;
    case 8:
    default:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        break;
    }

    //设置停止位
    if (stopbits == 2)
        tty.c_cflag |= CSTOPB;
    else
        tty.c_cflag &= ~CSTOPB;

    //设置奇偶校验
    switch (parity)
    {
        //无奇偶校验
    case 'n':
    case 'N':
        tty.c_cflag &= ~PARENB;   /* Clear parity enable */
        tty.c_iflag &= ~INPCK;    /* Enable parity checking */
        break;
        //奇校验
    case 'o':
    case 'O':
        tty.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
        tty.c_iflag |= INPCK;             /* Disable parity checking */
        break;
        //偶校验
    case 'e':
    case 'E':
        tty.c_cflag |= PARENB;    /* Enable parity */
        tty.c_cflag &= ~PARODD;   /* 转换为偶效验*/
        tty.c_iflag |= INPCK;     /* Disable parity checking */
        break;
        //等效于“无奇偶校验”
    case 'S':
    case 's':  /*as no parity*/
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        break;
    default:
        tty.c_cflag &= ~PARENB;   /* Clear parity enable */
        tty.c_iflag &= ~INPCK;    /* Enable parity checking */
        break;
    }

    //设置硬件流控制
    if (hwf)
        tty.c_cflag |= CRTSCTS;
    else
        tty.c_cflag &= ~CRTSCTS;

    /*if (hwf) */
    /*tty.c_cflag |= CIREN;*/

    //设置软件流控制
    if (swf)
        tty.c_iflag |= IXON | IXOFF;
    else
        tty.c_iflag &= ~(IXON|IXOFF|IXANY);

    //设置为RAW模式
    tty.c_iflag &= ~(IGNBRK | IGNCR | INLCR | ICRNL | IUCLC |
                     IXANY | IXON | IXOFF | INPCK | ISTRIP);
    tty.c_iflag |= (BRKINT | IGNPAR);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(XCASE|ECHONL|NOFLSH);
    tty.c_lflag &= ~(ICANON | ISIG | ECHO);
    tty.c_cflag |= (CLOCAL | CREAD);

    //tty.c_cc[VTIME] = 1;   /*（非阻塞模式）操作超时，单位为0.1秒*/
    //tty.c_cc[VMIN] = 1;   /* 1为阻塞模式，0为非阻塞模式*/
    tty.c_cc[VTIME] = 2;   /*（非阻塞模式）操作超时，单位为0.1秒*/
    tty.c_cc[VMIN] = 0;   /* 1为阻塞模式，0为非阻塞模式*/

    //设置串口属性
    ret = tcsetattr(g_fd_ble, TCSANOW, &tty);

    if (ret<0)
    {
		dbg_formatvar("-3");
        close(g_fd_ble);
        g_fd = -1;
        return -1;
    }

    tcflush(g_fd_ble, TCIOFLUSH);
#endif

    return g_fd_ble;
}


int libserial_init_test(char *dev, int speed, int parity, int databits, int stopbits, int hwf, int swf)
{

    int i	= 0;
	int ret = 0;

#ifdef _WIN32

#else
    struct termios tty;

    ret = tcgetattr(g_fd_ble, &tty);

    if (ret<0)
    {
		dbg_formatvar("-2");
        close(g_fd_ble);
        g_fd = -1;
        return -1;
    }
    //设置波特率
    for (i=0; i<(int)(sizeof(speed_arr)/sizeof(int)); i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&tty, speed_arr[i]);
            cfsetospeed(&tty, speed_arr[i]);
            break;
        }
        if (name_arr[i] == 0)
        {
//            printf("speed %d is not support,set to 9600\n",speed);
            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);
        }
    }

    //设置数据位
    switch (databits)
    {
    case 5:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS5;
        break;
    case 6:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS6;
        break;
    case 7:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS7;
        break;
    case 8:
    default:
        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
        break;
    }

    //设置停止位
    if (stopbits == 2)
        tty.c_cflag |= CSTOPB;
    else
        tty.c_cflag &= ~CSTOPB;

    //设置奇偶校验
    switch (parity)
    {
        //无奇偶校验
    case 'n':
    case 'N':
        tty.c_cflag &= ~PARENB;   /* Clear parity enable */
        tty.c_iflag &= ~INPCK;    /* Enable parity checking */
        break;
        //奇校验
    case 'o':
    case 'O':
        tty.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
        tty.c_iflag |= INPCK;             /* Disable parity checking */
        break;
        //偶校验
    case 'e':
    case 'E':
        tty.c_cflag |= PARENB;    /* Enable parity */
        tty.c_cflag &= ~PARODD;   /* 转换为偶效验*/
        tty.c_iflag |= INPCK;     /* Disable parity checking */
        break;
        //等效于“无奇偶校验”
    case 'S':
    case 's':  /*as no parity*/
        tty.c_cflag &= ~PARENB;
        tty.c_cflag &= ~CSTOPB;
        break;
    default:
        tty.c_cflag &= ~PARENB;   /* Clear parity enable */
        tty.c_iflag &= ~INPCK;    /* Enable parity checking */
        break;
    }

#endif

    return g_fd_ble;
}

int libserial_close_local(void)
{
    int status;

    status = close(g_fd);
    g_fd = -1;

    return(status);
}

int libserial_close_local_ble(void)
{
    int status;

    status = close(g_fd_ble);
    g_fd_ble = -1;

    return(status);
}

void libserial_chang_baud_local(int speed)
{
    int i	= 0;

#ifdef _WIN32

#else
    struct termios tty;

    // 取得当前串口参数
    tcgetattr(g_fd, &tty);

    //设置波特率
    for (i= 0;  i < (int)(sizeof(speed_arr) / sizeof(int));  i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&tty, speed_arr[i]); // 输入波特率
            cfsetospeed(&tty, speed_arr[i]); // 输出波特率
            break;
        }
        if (name_arr[i] == 0)
        {
            printf("speed %d is not support,set to 115200\n",speed);
            cfsetispeed(&tty, B115200);
            cfsetospeed(&tty, B115200);
        }
    }

    // 设置新的串口参数
    tcsetattr(g_fd,TCSANOW,&tty);
#endif
}


void libserial_chang_baud_local_ble(int speed)
{
    int i	= 0;

#ifdef _WIN32

#else
    struct termios tty;

    // 取得当前串口参数
    tcgetattr(g_fd_ble, &tty);

    //设置波特率
    for (i= 0;  i < (int)(sizeof(speed_arr) / sizeof(int));  i++)
    {
        if (speed == name_arr[i])
        {
            cfsetispeed(&tty, speed_arr[i]); // 输入波特率
            cfsetospeed(&tty, speed_arr[i]); // 输出波特率
            break;
        }
        if (name_arr[i] == 0)
        {
            printf("speed %d is not support,set to 115200\n",speed);
            cfsetispeed(&tty, B115200);
            cfsetospeed(&tty, B115200);
        }
    }

    // 设置新的串口参数
    tcsetattr(g_fd_ble,TCSANOW,&tty);
#endif
}

void libserial_null_read_local(void)
{
	int rc			= 0;
	char tmp[128]	= {0};

#ifdef _WIN32

#else
    struct termios options;
    struct termios old;

    libserial_save_set_local(&old);

    // 设置超时时间
    tcgetattr(g_fd, &options);
    options.c_cc[VTIME]=1;
    tcsetattr(g_fd, TCSANOW, &options);

    while (1)
    {
        rc = read(g_fd, tmp, sizeof(tmp));
        //if (rc != 1) break;
        if (rc <= 0) break;
    }

    libserial_re_set_local(&old);
#endif
}

void libserial_null_read_local_ble(void)
{
	int rc			= 0;
	char tmp[128]	= {0};

#ifdef _WIN32

#else
    struct termios options;
    struct termios old;

    libserial_save_set_local_ble(&old);

    // 设置超时时间
    tcgetattr(g_fd_ble, &options);
    options.c_cc[VTIME]=1;
    tcsetattr(g_fd_ble, TCSANOW, &options);

    while (1)
    {
        rc = read(g_fd_ble, tmp, sizeof(tmp));
        if (rc <= 0) break;
		//g_Record.log_buffer("read last data from the buf before the real readdata:",(uchar *)tmp,rc);
		//g_Record.log_buffer("get data from bluetooth:", bufTemp,bufTemplen);
    }

    libserial_re_set_local_ble(&old);
#endif
}

void libserial_set_rts_local(int st)
{
    int linedata	= 0;

#ifdef _WIN32

#else
	ioctl(g_fd, TIOCMGET, &linedata);

    if (st)
    {
        linedata |= TIOCM_RTS;
    }
    else
    {
        linedata &= ~TIOCM_RTS;
    }

    ioctl(g_fd, TIOCMSET, &linedata);
    usleep(2);

#endif
}

void libserial_set_rts_local_ble(int st)
{
    int linedata	= 0;

#ifdef _WIN32

#else
	ioctl(g_fd_ble, TIOCMGET, &linedata);

    if (st)
    {
        linedata |= TIOCM_RTS;
    }
    else
    {
        linedata &= ~TIOCM_RTS;
    }

    ioctl(g_fd_ble, TIOCMSET, &linedata);
    usleep(2);

#endif
}


int libserial_send_package_local(uchar *buf, const int buflen)
{

    unsigned short crc16;

    // 刷新读写数据
    crc16 = pkg_crc16_local(buf, buflen-2);
    memcpy((char *)(buf+(buflen-2)), (char *)&crc16, 2);
    /*tcflush(fd, TCIOFLUSH);*/
    int realwrite=write(g_fd, buf, buflen);
    if (realwrite==buflen)
    	{
    		return 0;
    	}
    	else
		    return -1;
}

int libserial_send_package_local_ble(uchar *buf, const int buflen)
{

    unsigned short crc16;

    // 刷新读写数据
    //crc16 = pkg_crc16_local(buf, buflen-2);
    //memcpy((char *)(buf+(buflen-2)), (char *)&crc16, 2);
    /*tcflush(fd, TCIOFLUSH);*/
	//dbg_formatvar("libserial_send_package_local_ble");
    int realwrite=write(g_fd_ble, buf, buflen);
	dbg_formatvar("realwrite:%d",realwrite);
	dbg_dumpmemory("send data with bluetooth ,datas is:", buf, buflen);
	//g_Record.log_out(0, level_error,"send data length is :%ld",realwrite);
	g_Record.log_buffer("send data with bluetooth ,datas is:", buf, buflen);
    if (realwrite==buflen)
    	{
    		return 0;
    	}
    	else
		    return -1;
}

int libserial_recv_package_local(int maxrecv, int timeout, uchar *buf)
{
    int rc = 0/*,record = 0*/;
    //int i = 0;
    int sumlen=0, pos=0/*, len=0*/;
    int maxlimite=maxrecv;//最大接收长度，防止缓冲溢出
    //uchar c = 0;
    int result = -1;
	uchar type =0;
	char cardType[2+1];	 //卡类型

#ifdef _WIN32

#else
	struct termios old, curr;
    libserial_save_set_local(&old);

    tcgetattr(g_fd, &curr);
    if (timeout<=0)
    	curr.c_cc[VTIME] = 1; //100ms
    else
    	curr.c_cc[VTIME] = timeout; //100ms
    tcsetattr(g_fd, TCSANOW, &curr);

	//dbg_formatvar("g_fd:%d",g_fd);

    while (1)
    {

        rc = read(g_fd, buf+pos, maxlimite); // 接收字符
		
        if (rc < 1)
        {
            // 超时
			//dbg_formatvar("ERR_RECV_TIMEOUT \n");
            result = ERR_RECV_TIMEOUT;
            break;
        }
		else{
			HS_LOG(" rc=%d g_fd=%d\n",rc,g_fd);
		}

        sumlen += rc;
        pos += rc;
        maxlimite -= rc;
		dbg_formatvar("rc:%d",rc);
		dbg_formatvar("sumlen:%d last info=%x",sumlen,buf[sumlen - 1]);
		dbg_dumpmemory("buf:",buf,sumlen);

		if(buf[sumlen - 1] == 0x0D){			
			dbg_formatvar("ERR_RECV_TIMEOUT 0x0D exit \n");
			//break;
		}

		/*

		if(sumlen >= 181)
		{
			if(memcmp(buf, "CSMG", 4) != 0)
			{
				//二维码不合法
				result = ERR_RECV_PKGCRC;
				return result;
				//break;
			}
			type = buf[54];
			memcpy(cardType, buf + 55, 2);
			dbg_formatvar("cardType = %s",cardType);

			if((type == '2') || (type == '4'))
			{
				if(sumlen >= 181)
					break;
			}

			if((type == '3'))
			{
				if(memcmp(cardType, "00", 2) == 0)
				{
					if(sumlen >= 185)
						break;
				}

				if(memcmp(cardType, "01", 2) == 0)//电子后付费储值票
				{
					if(sumlen >= (170 - 2))
						break;
				}

				if(memcmp(cardType, "02", 2) == 0)//电子预付费储值票
				{
					if(sumlen >= (162 - 2))
						break;
				}

			}

			if((type == '1'))
			{
				if(memcmp(cardType, "01", 2) == 0)//电子后付费储值票
				{
					if(sumlen >= (131 - 2))
						break;
				}

				if(memcmp(cardType, "02", 2) == 0)//电子预付费储值票
				{
					if(sumlen >= (141 - 2))
						break;
				}
			}

		}*/

        if ( maxlimite <= 0 )
        {
            result = ERR_RECV_FULL;			
			dbg_formatvar("maxlimite \n");
            break;
        }


        //result = -1;

    }

	//dbg_formatvar("ERR_RECV_TIMEOUT \n");

	//dbg_dumpmemory("buf:",buf,sumlen);

    libserial_re_set_local(&old);
#endif

    return sumlen;
}



int libserial_recv_package_local_ble(ETBLEOPER operType,int maxrecv, int timeout, uchar *buf)
{
    int rc = 0/*,record = 0*/;
    //int i = 0;
    int sumlen=0, pos=0, rest=0, bufTemplen=0/*, len=0*/;
    int maxlimite=maxrecv;//最大接收长度，防止缓冲溢出
    //uchar c = 0;
    int result = -1;
	int count = 0;

	uint8_t msgType = 0xFF;

	int wishOnePackLength = 0;

	uchar bufTemp[1024] = {0};//用来存放每次接收的数据，以便处理
	uchar temp[1024] = {0};//用来处理过程数据，进行转换

	uchar errorType = 0x00;

	int i = 0;
	int j = 0;
	int m = 0;

	int length = 0;


#ifdef _WIN32

#else
	struct termios old, curr;
    libserial_save_set_local_ble(&old);

    tcgetattr(g_fd_ble, &curr);
    if (timeout<=0)
    	curr.c_cc[VTIME] = 1; //100ms
    else
    	curr.c_cc[VTIME] = timeout; 
    tcsetattr(g_fd_ble, TCSANOW, &curr);

	// rest-rc中还剩多少未处理
	// pos-buf指针
	// bufTemplen-待处理数据中一共剩余多少数据
	// rc本次读了多少数据
	//dbg_formatvar("g_fd_ble:%d",g_fd_ble);

    while (1)
    {
		//stopflag = false;
		if((operType == operBleTestOther)||(operType == operBleDownloadReset))//发送复位指令后，不需要等待返回
			return 0;

        rc = read(g_fd_ble, buf, maxlimite); // 接收字符
		//g_Record.log_buffer("ble scan begin", m_ticket_data.ble_macAddress,6);
		//g_Record.log_out(0, level_error,"*********************rc=%d*********************",rc);
		//dbg_formatvar("rc:%d",rc);

        if (rc < 1)
        {
            // 超时
			g_Record.log_out(0, level_error,"ERR_RECV_TIMEOUT");
			if(errorType == 0x61)
				result = ERR_RECV_TIMEOUT_NEED_RESET;
			else
				result = ERR_RECV_TIMEOUT;
            break;
        }

		//将读到的数据拷贝到待处理数据数组中
		memcpy(bufTemp + bufTemplen, buf, rc);
		bufTemplen += rc;//bufTemplen为bufTemp中还剩余的未处理数据

		dbg_formatvar("bufTemplen:%d",bufTemplen);
		dbg_dumpmemory("bufTemp----->",bufTemp,bufTemplen);
		g_Record.log_buffer("get data from bluetooth:", bufTemp,bufTemplen);

		if((operType == operBleTest))
			return 0;

		//一次交易完成报文为
		//FE 01 01   SCAN返回里有下发的广播号
		//FE 01 02   Connect成功
		//FE 01 03   discovery成功
		//FE 01 04   Write成功
		//FE 10 00 FF 30 31 30 30 30 30 30 36 31 30 31 31 30 30  收到回写结果通知


		if(bufTemplen >= 3)
		{

			for(j = 0; j < bufTemplen; )
			{
				if(bufTemp[j] != 0xFE)
				{
					j++;
				}
				else{
					for (m = 0;m < (bufTemplen - j); m++)
					{
						bufTemp[m] = bufTemp[j+m];
					}
					bufTemplen = bufTemplen - j;
					break;
				}
			}

			dbg_formatvar("bufTemplen:%d",bufTemplen);

			length = bufTemp[1];

			//注意这两种情况 (1)FE0161    FE1000  或者FE0171
			//               (2)FE0161FE1000
			//				 (3)FE0161FE0171
			if((bufTemplen == 3)&&(bufTemp[2] == 0x61))//(1)只收了三个，那么认为后面还有 还得去收
			{
				errorType = 0x61;
				continue;
			}

			if((bufTemplen > 3)&&(bufTemp[2] == 0x61))
			{
				errorType = 0x61;
				if(bufTemplen < 6)
					continue;

				length = bufTemp[4];
				if((length == 1)&&(bufTemp[5] == 0x71)){
					result = 71;
					break;
				}else if((length == 16)&&(bufTemp[5] == 0x00)){
					if((bufTemp[16] == 0x30) && ((bufTemp[17] == 0x30)))//返回成功
					{
						//返回00说明交易成功
						result = 0;
						break;
					}
					else
					{
						//返回非00，说明交易失败
						result = 64;
						break;
					}
				}else{
					result = 63;
					break;
				}
			}



			if(bufTemplen >= length + 2)//说明接收完成
			{
				errorType = bufTemp[2];
				if(length == 1)//失败交易
				{
					if(bufTemp[2] == 0x70){
						result = 70;
					}else if(bufTemp[2] == 0x71){
						result = 71;
					}else if(bufTemp[2] == 0x72){
						result = 72;
					}else if(bufTemp[2] == 0x74){
						result = 74;
					}else if(bufTemp[2] == 0x87){
						result = 87;
					}else if(bufTemp[2] == 0x88){
						result = 88;
					}else if(bufTemp[2] == 0x89){
						result = 89;
					}else if(bufTemp[2] == 0x8A){
						result = 138;
					}else if(bufTemp[2] == 0x8B){
						result = 139;
					}else if(bufTemp[2] == 0x6F){
						result = 111;
					}else{
						result = 63;
					}
					break;
				}else{
					if((bufTemp[16] == 0x30) && ((bufTemp[17] == 0x30)))//返回成功
					{
						//返回00说明交易成功
						result = 0;
						break;
					}
					else
					{
						//返回非00，说明交易失败
						result = 64;
						break;
					}
				}
			}else{
				continue;
			}
		}

        if ( maxlimite <= 0 )
        {
            result = ERR_RECV_FULL;
            break;
        }

    }

	//dbg_dumpmemory("buf:",buf,sumlen);

    libserial_re_set_local_ble(&old);
#endif

    return result;
}

