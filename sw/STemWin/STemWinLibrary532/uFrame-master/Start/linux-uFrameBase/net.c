/*****************************************************************************/
/*��̵����ɷ����޹�˾                                     ��Ȩ��2008-2015   */
/*****************************************************************************/
/* ��Դ���뼰������ĵ�Ϊ���������̵����ɷ����޹�˾�������У�δ��������    */
/* �ɲ��������޸Ļ򷢲�������׷����صķ������Ρ�                          */
/*                                                                           */
/*                      ���������̹ɷ����޹�˾                             */
/*                      www.xjgc.com                                         */
/*                      (0374) 321 2924                                      */
/*                                                                           */
/*****************************************************************************/


/******************************************************************************
    ��Ŀ����	 ��  SGE800���������ն�ƽ̨
    �ļ���		 ��  net.c
    ����       		 ��  ���ļ�����������ģ��ӿ�
    �汾       		 ��  0.1
    ����       		 ��  ����
    ��������   	 ��  2009.09
******************************************************************************/

//������ͷ�ļ�
#include "private/config.h"

//ģ�����ÿ���
#ifdef CFG_NET_MODULE

//����ͷ�ļ�
#include "private/debug.h"

//��������ͷ�ļ�

//C��ͷ�ļ�
#include <stdio.h>						//printf
#include <fcntl.h>						//open
#include <unistd.h>						//read,write
#include <string.h>						//bzero
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>  
#include <net/route.h>


//�ṩ���û���ͷ�ļ�
#include "include/error.h"
#include "include/net.h"

/*************************************************
  ��̬ȫ�ֱ�������
*************************************************/
/*************************************************
  �ṹ���Ͷ���
*************************************************/
//�������˽ṹ��
static struct{
	struct sockaddr_in client_sockaddr;
	int sockfd;                          
	int client_fd[CFG_NET_BACKLOG];    //����ͻ����׽���
	u64 client_state;                  //�׽���ʹ��״̬
	socklen_t sin_size;
}server_net_info;

//�ͻ���
static struct{
	int sockfd[CFG_NET_BACKLOG];
}client_net_info;

/******************************************************************************
*	����:	net_ip_set
*	����:	����IP
*	����:	ipaddr			-	ip��ַ
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	������Ч

*	˵��:	�޸�ip�����½�������
******************************************************************************/
int net_ip_set(char *ipaddr)
{
	struct sockaddr_in sin;   
  	struct ifreq ifr;   
  	int fd = 0;
  	int ret = 0;
    //struct hostent *host;
	/*
	if ((host = gethostbyname(ipaddr)) == NULL){    //��ַ�����
		ret = -ERR_INVAL;
		goto error;
	}
	*/
    const char *ifname = "eth0";                     //�豸��
  	
  	memset (&ifr, 0 ,sizeof(struct ifreq));          //��ʼ���ṹ
    if (ipaddr == NULL){
    	ret = -ERR_INVAL;   
    	goto error;
    }   
      
  	fd = socket (AF_INET, SOCK_DGRAM, 0);   
  	if (fd == -1)   
  	{   
  		ret = -ERR_SYS;
  		goto error;   
 	 }   
  	strncpy (ifr.ifr_name, ifname, IFNAMSIZ);        //���ýӿ�����
  	//ifr.ifr_name[IFNAMSIZ - 1] = 0;   
  	memset(&sin, 0, sizeof(sin));   
  	sin.sin_family = AF_INET;   
    //sin.sin_addr = *((struct in_addr *)host->h_addr);
  	sin.sin_addr.s_addr = inet_addr(ipaddr);
    memset(&(sin.sin_zero), 0, 8);                   //���
    memcpy(&ifr.ifr_addr, &sin,sizeof(sin)); 
      
  	if(ioctl (fd, SIOCSIFADDR, &ifr) < 0)            //����
  	{   
  		ret = -ERR_SYS;
  		goto error;
  	}   
    
 	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;   
  	if(ioctl (fd, SIOCSIFFLAGS,&ifr) < 0)     
  	{   
  		ret = -ERR_SYS;
  		goto error;   
  	}
 error:
 	close (fd);
 	return(ret);     
}

/******************************************************************************
*	����:	net_gateway_set
*	����:	��������
*	����:	gateway			-	���ص�ַ
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	������Ч

*	˵��:	�޸����غ����½�������
******************************************************************************/
int net_gateway_set(char *gateway)
{
    int ret;
    struct rtentry rt;
    struct sockaddr_in sin;
    int skfd = -1;
    struct hostent *host;
    
    if   (gateway == NULL){
    	ret = -ERR_INVAL;   
    	goto error;
    }   
    
    memset (&rt, 0, sizeof (struct rtentry));                     //�ṹ��ʼ��
    memset (&sin, 0, sizeof (struct sockaddr_in));
    
    if ((host = gethostbyname(gateway)) == NULL){                 //��ַ�����
		ret = -ERR_INVAL;
		goto error;
	}
	sin.sin_family = AF_INET;   
    sin.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(sin.sin_zero),0,8);                                  //���
    
    memcpy (&rt.rt_gateway, &sin, sizeof(struct sockaddr_in));    //���ýӿ�����
    ((struct sockaddr_in *) &rt.rt_dst)->sin_family = AF_INET;
    ((struct sockaddr_in *) &rt.rt_genmask)->sin_family = AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    skfd = socket (AF_INET, SOCK_DGRAM, 0);
    if (skfd < 0)
    {
    	ret = -ERR_SYS;
    	goto error;
    }
    
    if (ioctl (skfd, SIOCADDRT, &rt) < 0)                         // ����ϵͳgaetway 
    {
    	ret = -ERR_SYS;
    	goto error;
    }
    ret = 0;
 error:
    close (skfd);
    return (ret);
   
}

/******************************************************************************
*	����:	net_mask_set
*	����:	����mask
*	����:	mask			-	mask���������ַ
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	������Ч

*	˵��:	�޸ĺ����½�������
******************************************************************************/
int net_mask_set(char *mask)
{
	struct sockaddr_in sin;    
  	struct ifreq ifr;   
  	int fd = 0;
  	int ret = 0;
    struct hostent *host;
	
	if ((host = gethostbyname(mask)) == NULL){               //��ַ�����
		ret = -ERR_INVAL;
		goto error;
	}
    const char *ifname = "eth0";                             //�豸��
  	
  	memset(&ifr, 0 ,sizeof(struct ifreq));   
    if (ifname == NULL){
    	ret = -ERR_SYS;   
    	goto error;
    }
    if (mask == NULL){
    	ret = -ERR_INVAL;   
    	goto error;
    }   
      
  	fd = socket(AF_INET, SOCK_DGRAM, 0);   
  	if (fd == -1)   
  	{   
  		ret = -ERR_SYS;
  		goto error;   
 	 }   
  	strncpy (ifr.ifr_name, ifname, IFNAMSIZ);                  //���ýӿ�����
  	//ifr.ifr_name[IFNAMSIZ - 1] = 0;   
  	memset (&sin, 0, sizeof(sin));   
  	sin.sin_family = AF_INET;   
    sin.sin_addr = *((struct in_addr *)host->h_addr);
    memset (&(sin.sin_zero),0,8);                              //��ֵ
    memcpy (&ifr.ifr_addr,&sin,sizeof(sin)); 
      
  	if (ioctl (fd, SIOCSIFNETMASK, &ifr) < 0)                 //����mark
  	{   
  		ret = -ERR_SYS;
  		goto error;
  	}   
    
 	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;   
  	if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
  	{   
  		ret = -ERR_SYS;
  		goto error;   
  	}
 error:
 	close (fd);
 	return(ret);     
}
/*******************************�ͻ���IAP����ʵ��*********************************************/

/******************************************************************************
*	����:	net_client_init
*	����:	��ʼ���ͻ���
*	����:	timout			-	����ģʽ�½��շ��ͳ�ʱʱ��
			num				-	�ͻ��˺�
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����

*	˵��:	��
******************************************************************************/
int net_client_init(u8 num, u16 timeout)
{
	int ret;
	struct timeval tout;

	//����socket
	if ((client_net_info.sockfd[num] = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		ret = -ERR_SYS;
		goto error;
	}
	if (timeout != 0xffff){
		//���������������ա����ͳ�ʱʱ��
		tout.tv_sec = timeout;
		tout.tv_usec = 0;
		//���÷��ͳ�ʱ
		ret = setsockopt(client_net_info.sockfd[num], SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout));
		if (ret)
		{
			ret = -ERR_SYS;
			goto error;
		}
		//���ý��ճ�ʱ
		ret = setsockopt(client_net_info.sockfd[num], SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout));
		if (ret)
		{
			ret = -ERR_SYS;
			goto error;
		}
	}

	ret = 0;
error:
	return (ret);
}

/******************************************************************************
*	����:	net_client_connect
*	����:	�ͻ������������������
*	����:	ip				-	������IP��ַ
			port			-	�������˿ں�
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	�ӿڲ������ô���
 		    -ERR_DISCONNECT	-	������δ��

*	˵��:	��
******************************************************************************/
int net_client_connect(u8 num, char *ip,u16 port)
{
	int ret = 0;
	struct sockaddr_in serv_addr;
	struct hostent *host;
	
	if (num >= CFG_NET_BACKLOG){
		ret = -ERR_INVAL;
		return ret;
	}

	if ((host = gethostbyname(ip)) == NULL){    //��ַ�����
		ret = -ERR_INVAL;
		goto error;
	}
	//���ò���
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	memset(&(serv_addr.sin_zero),0,8);          //��server_sockaddr.sin_zero���ڵ��ڴ�����Ϊ0
	
	//����Է������˵�����
	if (connect(client_net_info.sockfd[num], (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1){
		NETPRINTF("connect fail!\n");
		ret = -ERR_DISCONNECT;
		goto error;
	}
	else{
		NETPRINTF("connect!\n");
	}
	
error:
	return (ret);
}

/******************************************************************************
*	����:	net_client_receive
*	����:	�ͻ��˽�������
*	����:	buf				-	���ջ�������ַ
            len				-	���ջ�������С
            length			-	ʵ�ʽ��մ�С�������
            flag			-	�������������ԣ�0��ʾ������0xffff��ʾ������
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	�ӿڲ������ô���
 		    -ERR_TIMEOUT	-	���ӳ�ʱ
 		    -ERR_DISCONNECT	-	����Ͽ�

*	˵��:	��
******************************************************************************/
int net_client_receive(u8 num, u8 *buf, u16 max_length, u16 *length, u8 flag)
{
	int recvbytes,ret;
	int errnum;
	
	if (num >= CFG_NET_BACKLOG){
		ret = -ERR_INVAL;
		return ret;
	}

	if (flag == NET_NONBLOCK){
		recvbytes = recv(client_net_info.sockfd[num], buf, max_length, MSG_DONTWAIT);    //����������
	}else if (flag == NET_BLOCK){
 		recvbytes = recv(client_net_info.sockfd[num], buf, max_length, 0);               //��������
	}else{
		ret = -ERR_INVAL;
		goto error;
	}

 	if (recvbytes != -1){
 		memcpy(length, &recvbytes, 2);						//���ݳ���2���ֽ�
 		ret = 0;
    }else{
    	errnum = errno;
		NETPRINTF("error=%d\n",errnum);
    	if (errnum == EAGAIN){
    		ret = -ERR_TIMEOUT;
    		goto error;
    	}else{
    		ret = -ERR_SYS;
    		goto error;
    	}
    }
    ret = 0;
error:
	return(ret);
}

/******************************************************************************
*	����:	net_client_send
*	����:	�ͻ��˷�������
*	����:	buf				-	���ͻ�������ַ
            length			-	�����ֽ���
*	����:	>0				-	�ɹ����͵��ֽ���
			-ERR_INVAL		-	�ӿڲ������ô���
 		    -ERR_TIMEOUT	-	���ӳ�ʱ
 		    -ERR_DISCONNECT	-	����Ͽ�

*	˵��:	��
******************************************************************************/
int net_client_send(u8 num, u8 *buf,u16 length)
{
	int ret;
	int errnum;
	if (length == 0){
		ret = -ERR_INVAL;
		goto error;
	}
	if (num >= CFG_NET_BACKLOG){
			ret = -ERR_INVAL;
			return ret;
	}

	ret = send(client_net_info.sockfd[num], buf, length, 0);
	if (ret == -1){
		errnum = errno;
		if ((errnum == EPIPE) || (errnum == EDESTADDRREQ)){
			ret = -ERR_DISCONNECT;
		}else{
			NETPRINTF("error=%d\n",errnum);
			ret = -ERR_TIMEOUT;
		}
		goto error;
	}
error:
	return(ret);
}


/******************************************************************************
*	����:	net_client_disconnect
*	����:	�ͻ��˶Ͽ�����
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
*	˵��:	��
******************************************************************************/
int net_client_disconnect(u8 num)
{
	int ref,ret;
	int errnum;
	ref = close(client_net_info.sockfd[num]);
	if (ref == 0){
		NETPRINTF("disconnect net success!\n");
	}else{
		errnum = errno;
		if(errnum == EBADF){
			NETPRINTF("net disconnect already!\n");	
			ret = -ERR_BUSY;
			goto error;
		}else{
			ret = -ERR_SYS;
			goto error;
		}
	}
	ret = 0;
error:
	return(ret);
}

/*******************************��������API����ʵ��***********************************/
/******************************************************************************
*	����:	net_server_init
*	����:	��ʼ����������
*	����:	port			-	�������˿ں�
			timout			-	����ģʽ�·���/����/�������ӳ�ʱ
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����

*	˵��:	��
******************************************************************************/
int net_server_init(u16 port, u16 timeout)
{
	int ret,i;
	int optval = 1; 
	struct timeval tout;
	
	struct sockaddr_in server_sockaddr;
	server_net_info.client_state = 0;                                         //��ʼ���׽���ʹ��״̬1��ʾ���׽���û��
	for (i = 0;i < CFG_NET_MAXSIZE; i++){
		server_net_info.client_fd[i] = 0;
	}
	
	if ((server_net_info.sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1){       //AF_INET--IPv4Э��
		NETPRINTF("opening socket failed!");
		ret = -ERR_SYS;
		goto error;
	}else{ 
		NETPRINTF("socket success!");
		server_sockaddr.sin_family = AF_INET;
		server_sockaddr.sin_port = htons(port);
		server_sockaddr.sin_addr.s_addr = INADDR_ANY;                         //INADDR_ANY ��ʾ���м��������������
		memset(&(server_sockaddr.sin_zero), 0, 8);
		
		if (timeout != 0xffff){
			//���������������ա����ͳ�ʱʱ��
			tout.tv_sec = timeout;
			tout.tv_usec = 0;
			//���÷��ͳ�ʱ
			ret = setsockopt(server_net_info.sockfd, SOL_SOCKET, SO_SNDTIMEO, &tout, sizeof(tout));
			if (ret)
			{
				ret = -ERR_SYS;
				goto error;
			}
			//���ý��ճ�ʱ
			ret = setsockopt(server_net_info.sockfd, SOL_SOCKET, SO_RCVTIMEO, &tout, sizeof(tout));
			if (ret)
			{
				ret = -ERR_SYS;
				goto error;
			}
		}
		//���ö˿ڸ���
  		if (setsockopt(server_net_info.sockfd, SOL_SOCKET, SO_REUSEADDR,(char *)&optval,sizeof(optval)) !=  0){   
        	NETPRINTF("error!setsockopt failed!\n");
        	ret = -ERR_INVAL;
        	goto error;   
          }   

		if (bind(server_net_info.sockfd, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr)) == -1){
			NETPRINTF("blind failed!\n");
			ret = -ERR_INVAL;
			goto error;
		}else{ 
			NETPRINTF("blind success!\n");
		}
		if(listen(server_net_info.sockfd, CFG_NET_BACKLOG) == -1){         //�����ȴ�����
    		NETPRINTF("listen failed!\n");
    		ret = -ERR_SYS;
    		goto error;
		}
	}
	ret = 0;
	return (ret);
 error:
  	close(server_net_info.sockfd);
  	return (ret);
}

/******************************************************************************
*	����:	net_server_listen
*	����:	����������
*	����:	id 				-	���ؼ��������׽��ֺ�
			mode			-	��NET_LISTEN_SELECT�������˵��߳�ʵ�ַ�ʽ��ͬʱ�����������׽��ֺ������ӿͻ��˵Ķ���׽���
							-	NET_LISTEN:�����������׽��֣�����ģʽ��id�ӿ���Ч
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	�ӿڴ���

*	˵��:	�������˵��߳�ʵ�ַ�ʽid����Oxff��ʾ���µ�����Ӧ���ý������Ӻ�����
			id��������Ϊ�׽��ֺ�Ϊid�����������ݣ�Ӧ���÷��������պ���
******************************************************************************/
int net_server_listen(u8 *id, u8 mode)
{
	int ref,ret;
	int i,max_fd;
	fd_set testfds;                                    //select �׽��ֶ���   
	
	if (mode == NET_LISTEN_SELECT)
	{
    	FD_ZERO (&testfds);
    	FD_SET (server_net_info.sockfd, &testfds);         //�������״̬�ּ�
    	max_fd = server_net_info.sockfd;                   //8λ״̬λ
    	for (i = 0; i < CFG_NET_MAXSIZE; i ++){
    		if (server_net_info.client_state & (1 << i)){   //�ж�״̬λ
    			FD_SET(server_net_info.client_fd[i], &testfds);
    		}
    		if (max_fd < server_net_info.client_fd[i]){
    			max_fd = server_net_info.client_fd[i];
    		}
    	}
    	
    	ref = select (max_fd+1, &testfds, NULL, NULL, NULL);
    	if(ref < 0){
    		ret = -ERR_SYS;
    		goto error;
    	}else{
    		if (FD_ISSET (server_net_info.sockfd, &testfds)){
    			*id = 0xff;        							//id����Oxff��ʾ���µ�����Ӧ���ý������Ӻ���
    			NETPRINTF("new connect\n");
    		}else{
    			for (i = 0;i < CFG_NET_MAXSIZE;i++){
    				if (FD_ISSET(server_net_info.client_fd[i], &testfds)){
    					*id = i;
    				}
    			}
    		}
    	}
	}else if (mode == NET_LISTEN){
		ref = listen(server_net_info.sockfd, CFG_NET_BACKLOG);
		if (ref < 0){
			ret = -ERR_SYS;
    		goto error;
		}
	}else{
		ret = -ERR_INVAL;
		goto error;
	}
	
	ret = 0;
error:
	return (ret);
}

/******************************************************************************
*	����:	net_server_connect
*	����:	��ͻ��˽���������
*	����:	id 				-	���ӷ�����׽��ֺţ����أ�
*	����:	0				-	�ɹ�
*	����:	0				-	�ɹ�
			-ERR_TIMEOUT	-	���ӳ�ʱ
			-ERR_SYS		-	ϵͳ����
*	˵��:	��
******************************************************************************/
int net_server_connect(u8 *id)
{
	int ret,i;
	u8 ud = 0;
	int errnum;
	int socket_fd_temp;
	
	if (server_net_info.client_state == (1<<CFG_NET_MAXSIZE)-1)
	{
		ret = -ERR_BUSY;
		return(ret);
	}
	NETPRINTF("server waiting**********\n");
	//�ͻ��˺ͷ������Ͽ�֮�������accept
	if((socket_fd_temp = accept(server_net_info.sockfd, \
	(struct sockaddr *)&server_net_info.client_sockaddr, &server_net_info.sin_size)) == -1){
		errnum = errno;
		if (errnum == EAGAIN){
			NETPRINTF("timeout!\n");
			ret = -ERR_TIMEOUT;
			goto error;
		}else{
			NETPRINTF("accept failed!\n");
			ret = -ERR_SYS;
			goto error;
		}
	}
	i = 0;
	while(i < CFG_NET_MAXSIZE){                        //�Զ������׽���
		if((server_net_info.client_state & (1 << i)) == 0){        //��λ״̬λ
			*id = i;
			ud = i;
			server_net_info.client_state |= (1 << i);	//��λ״̬λ
			i = CFG_NET_MAXSIZE;
		}else{
			if (i == CFG_NET_MAXSIZE - 1){
				ret = -ERR_BUSY;
				return(ret);
			}
		}
		i ++;
	}

	server_net_info.client_fd[ud]  = socket_fd_temp;
	ret = 0;
	return (ret);
error:
	server_net_info.client_state &= ~(1<< ud);   //��λ״̬λ
	return(ret);
}

/******************************************************************************
*	����:	net_server_receive
*	����:	���������ܺ���
*	����:	id 				-	����������Ľ��յ����ݵ��׽��ֺ�
			buf				- 	�������ݻ�����
			max_length		-	�������ݻ����������
            length			-	ʵ�ʷ������ݵĴ�С
            flag	      	-	flag=0Ϊ������ʽ����ʱʱ���ڳ�ʼ�����裩��0xffffΪ������
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	�ӿڴ���
			-ERR_DISCONNECT	-	���ӶϿ�
 			-ERR_TIMEOUT	-	��ʱ

*	˵��:	��
******************************************************************************/
int net_server_receive(u8 id, u8 *buf, u16 max_length,u16 *length, u8 flag)
{
	int ret,recvbytes;
	int errnum;
	
	if (id >=CFG_NET_MAXSIZE){
		ret = -ERR_INVAL;
		goto error;
	}
	if (max_length == 0){
		ret = -ERR_INVAL;
		goto error;
	}
	if (flag == NET_BLOCK){     						//������ʽ
		recvbytes = recv(server_net_info.client_fd[id], buf, max_length, 0);
		if(recvbytes != -1){
			memcpy(length, &recvbytes, 2);     			//�������ݳ���
			ret = 0;
		}else{
			errnum = errno;
			if ((errnum == EBADF) || (errnum == ENOTCONN)){
				ret = -ERR_DISCONNECT;
				goto error;
			}else if (errnum == EAGAIN){
				ret = -ERR_TIMEOUT;
				goto error;
			}else{
				ret = -ERR_SYS;
				goto error;
			}
		}
	}
	else if(flag == NET_NONBLOCK){					//��������ʽ
		recvbytes = recv(server_net_info.client_fd[id], buf, max_length, MSG_DONTWAIT);
		if(recvbytes != -1){
			memcpy(length, &recvbytes, 2);     		//�������ݳ���
			ret = 0;
		}else{
			errnum = errno;
			if ((errnum == EBADF) || (errnum ==  ENOTCONN)){
				ret = -ERR_DISCONNECT;
				goto error;
			}else if (errnum == EAGAIN){
				ret = -ERR_TIMEOUT;
				goto error;
			}else{
				ret = -ERR_SYS;
				goto error;
			}
		}
	}
	else{
		ret = -ERR_INVAL;
		goto error;
	}

    ret = 0;
error:
	return(ret);
}

/******************************************************************************
*	����:	net_server_send
*	����:	���������ͺ���
*	����:	id 				-	����Ҫ�����������ӵ��׽��ֺ�
			buf				- 	�������ݻ�����
            length			-	�������ݵĴ�С
*	����:	>0				-	�ɹ����͵��ֽ���
			-ERR_INVAL		-	�ӿڴ���
			-ERR_SYS		-	ϵͳ����
			-ERR_DISCONNECT	-	���ӶϿ�
 			-ERR_TIMEOUT	-	��ʱ

*	˵��:	��
******************************************************************************/
int net_server_send(u8 id, u8 *buf, u16 length)
{
	int ret;
	int errnum;
	if (id >= CFG_NET_MAXSIZE){
		ret = -ERR_INVAL;
		goto error;
	}
	if (length == 0){
		ret = -ERR_INVAL;
		goto error;
	}
	ret = send(server_net_info.client_fd[id], buf, length, MSG_NOSIGNAL);
	if (ret == -1){
		NETPRINTF("send data fail!\n");
		errnum = errno;
		if ((errnum == EPIPE) || (errnum == EDESTADDRREQ)){
			ret = -ERR_DISCONNECT;
		}else{
			ret = -ERR_SYS;
		}
		goto error;
	}
error:
	return(ret);
}

/******************************************************************************
*	����:	net_server_disconnect
*	����:	�������Ͽ�����
*	����:	id 				-	����Ҫ�����������ӵ��׽��ֺ�
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			-ERR_INVAL		-	�ӿڴ���
			-ERR_BUSY		-	�ѶϿ�

*	˵��:	��
******************************************************************************/
int net_server_disconnect(u8 id)
{
	int ref,ret;
	int errnum;
	
	if (id >= CFG_NET_MAXSIZE){
		ret = -ERR_INVAL;
		goto error;
	}
	ref = close(server_net_info.client_fd[id]);
	if (ref == 0){
		NETPRINTF("disconnect net success!\n");
		server_net_info.client_state &= ~(1 << id);      //����״̬λ
	}else{
		errnum = errno;
		if (errnum == EBADF){
			NETPRINTF("net disconnect already!\n");	
			ret = -ERR_BUSY;
			goto error;
		}else{
			ret = -ERR_SYS;
			goto error;
		}
	}
	
	ret = 0;
error:
	return(ret);
}

/******************************************************************************
*	����:	net_server_close
*	����:	�������ر�
*	����:	��
*	����:	0				-	�ɹ�
			-ERR_SYS		-	ϵͳ����
			��ERR_BUSY		-	�ѹر�

*	˵��:	��
******************************************************************************/
int net_server_close()
{
	int ref,ret;
	int errnum;
	
	ref = close(server_net_info.sockfd);
	if (ref == 0){
		NETPRINTF("disconnect net success!\n");
	}else{
		errnum = errno;
		if (errnum == EBADF){
			NETPRINTF("net disconnect already!\n");	
			ret = -ERR_BUSY;
			goto error;
		}else{
			ret = -ERR_SYS;
			goto error;
		}
	}
	ret = 0;
error:
	return ret;
}

#endif      /* CFG_NET_MODULE */
