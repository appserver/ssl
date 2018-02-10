/*==================================================================================================
                            INCLUDE FILES
==================================================================================================*/

#include <stdio.h>
#include <string.h>

#include "m2m_type.h"
#include "m2m_clock_api.h"
#include "m2m_fs_api.h"
#include "m2m_hw_api.h"
#include "m2m_os_api.h"
#include "m2m_os_lock_api.h"
#include "m2m_socket_api.h"
#include "m2m_timer_api.h"
#include "m2m_sms_api.h"
#include "m2m_network_api.h"
#include "m2m_utilities.h"


/*==================================================================================================
                            LOCAL CONSTANT DEFINITION
==================================================================================================*/
char APN[50]= "INTERNETG"; // user need to configure the SIM operator before running
char SERVER[80] = "APPZONE.TELIT.COM";
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =//
#define TRUE 1
#define FALSE 0
/*==================================================================================================
                            LOCAL TYPES DEFINITION
==================================================================================================*/

/*==================================================================================================
                            LOCAL FUNCTION PROTOTYPES
==================================================================================================*/
static INT32 hw_uart_read_cb ( M2M_T_HW_UART_HANDLE handle, CHAR *buffer, INT32 len );
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =//
int activate_PDP(void);
int get_IP(void);
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =//
int create_socket(void);
int connect_socket(void);
void socket_close(void);
//= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =//
int send_data(void);
/*==================================================================================================
                            GLOBAL FUNCTIONS PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                            LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                            LOCAL VARIABLES
==================================================================================================*/
M2M_T_HW_UART_HANDLE global_uart = M2M_HW_UART_HANDLE_INVALID;
char* IP;
int firstTimeCleanBuffer = TRUE;
char globalSendStr[1024] = "";
M2M_SOCKET_BSD_SOCKET Socketfd;
M2M_SOCKET_BSD_SOCKADDR_IN StSockAddr;
/*==================================================================================================
                            GLOBAL VARIABLES
==================================================================================================*/

/*==================================================================================================
                            LOCAL FUNCTIONS IMPLEMENTATION
==================================================================================================*/
/* Call back UART */
static INT32 hw_uart_read_cb ( M2M_T_HW_UART_HANDLE handle, CHAR *buffer, INT32 len )
{
	CHAR serRxStr[ 128 ];
	int sent;
	int i;

	strncpy( serRxStr, buffer, len );

	serRxStr[len]= 0;

	m2m_hw_uart_write(global_uart, serRxStr, strlen(serRxStr), &sent);

	for(i=0; i < len ; i++)
	{
		if (serRxStr[i] == 13)
		{
			serRxStr[i] = 0;
			strcat (globalSendStr,serRxStr);
			send_data();
			globalSendStr[0] = '\0';
			return M2M_HW_UART_RESULT_SUCCESS;
		}

	}

	strcat (globalSendStr,serRxStr);

	return M2M_HW_UART_RESULT_SUCCESS;
}

/* send data (globalSendStr) */
int send_data()
{
	char sendbuf[1024];
	char rcvbuf[1024];
	char tempStr[512] = "";

	int count;
	M2M_LOG_INFO("\r\nSending Row...\r\n");

	//Server handle letters
	for (count=0; globalSendStr[count]!= '\0'; count++)
	{
		  if (globalSendStr[count] == ' ')
			  strcat(tempStr,"%20");
		  else if (globalSendStr[count] == '#')
			  strcat(tempStr,"%23");
		  else if (globalSendStr[count] == '$')
			  strcat(tempStr,"%24");
		  else if (globalSendStr[count] == '%')
			  strcat(tempStr,"%25");
		  else if (globalSendStr[count] == '^')
			  strcat(tempStr,"%5E");
		  else if (globalSendStr[count] == '&')
			  strcat(tempStr,"%26");
		  else	sprintf(tempStr,"%s%c",tempStr,globalSendStr[count]);
	}


    strcpy(globalSendStr,tempStr);

	if(!create_socket())
	{
		M2M_LOG_ERROR("Can't create socket.\r\n");
	}
	else if(!connect_socket())
	{
		M2M_LOG_INFO("Can't connect socket.\r\n");
	}

	//Sending String to server
	//Based on Telit-AppZone server files
	sprintf(sendbuf, "GET /Demo/GETserver/report.php?text=%s HTTP/1.1\r\n",globalSendStr);
	m2m_socket_bsd_send(Socketfd, sendbuf, strlen(sendbuf), 0);
	sprintf(sendbuf, "Host: %s\r\n",SERVER);
	m2m_socket_bsd_send(Socketfd, sendbuf, strlen(sendbuf), 0);
	sprintf(sendbuf, "\r\n\r\n\r\n\r\n");
	m2m_socket_bsd_send(Socketfd, sendbuf, strlen(sendbuf), 0);
	m2m_socket_bsd_send_buf_size(Socketfd);

	//Result from server
	memset(rcvbuf,0,sizeof(rcvbuf));
	m2m_socket_bsd_recv(Socketfd, rcvbuf, 1023, 0);

	//check for server response - 200 OK
	if (!strstr(rcvbuf,"200 OK"))
	{
		M2M_LOG_INFO("\n\rServer ERROR\n\rData doesn't sent to the server: %s\r",globalSendStr);

		return FALSE;
	}

	M2M_LOG_INFO("\n\rData Sent To Server: %s\r\nPlease go to 'http://appzone.telit.com/Demo/GETserver' to see the row.\r\nPlease enter data to send..\r",globalSendStr);
	return TRUE;
}

/* Activate PDP */
int activate_PDP()
{
	int res = 0;

    res = m2m_pdp_activate(APN, NULL, NULL);

	if (M2M_PDP_STATE_FAILURE == m2m_pdp_get_status())
	{
	    res = m2m_pdp_activate(APN, NULL, NULL);

	    if (M2M_PDP_STATE_FAILURE == res)
		      return FALSE;

	    else
	    	return TRUE;
	}

	else
		return TRUE;
}

/* Get IP */
int get_IP(void)
{
	int i;
	char buf[512];

	memset(buf, 0, 512);

	for (i=0 ; i < 20 ; i++) //minute
	{
		IP = m2m_socket_bsd_addr_str(m2m_pdp_get_my_ip());


		if (strcmp(IP, "0.0.0.0"))
			return TRUE;
		else
		{
			if( (i==1) ||(i==4) || (i==9) || (i==13) || (i==17))
				activate_PDP();
		}

		m2m_os_sleep_ms(3000);
	}

	return FALSE;
}

/* Create socket */
int create_socket()
{
	if (Socketfd != 0)
		socket_close();

	Socketfd = m2m_socket_bsd_socket(M2M_SOCKET_BSD_PF_INET, M2M_SOCKET_BSD_SOCK_STREAM, M2M_SOCKET_BSD_IPPROTO_TCP);

	if (M2M_SOCKET_BSD_INVALID_SOCKET == Socketfd)
	        	return FALSE;

	return TRUE;
}

/* Connect socket */
int connect_socket()
{
	memset(&StSockAddr, 0, sizeof(struct M2M_SOCKET_BSD_SOCKADDR_IN));

	StSockAddr.sin_family = M2M_SOCKET_BSD_PF_INET;
	StSockAddr.sin_port = m2m_socket_bsd_htons(80);
	StSockAddr.sin_addr.s_addr = m2m_socket_bsd_get_host_by_name((char*)SERVER);

	if (M2M_SOCKET_BSD_INVALID_SOCKET == m2m_socket_bsd_connect(Socketfd, (const struct M2M_SOCKET_BSD_SOCKADDR *)&StSockAddr, sizeof(struct M2M_SOCKET_BSD_SOCKADDR_IN)))
	{
		m2m_socket_bsd_close(Socketfd);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

/* Close socket */
void socket_close()
{
	m2m_socket_bsd_shutdown(Socketfd, M2M_SOCKET_BSD_SHUT_RDWR);
	m2m_socket_bsd_close(Socketfd);
	Socketfd = 0;
}

/*==================================================================================================
                            GLOBAL FUNCTIONS IMPLEMENTATION
==================================================================================================*/

/* =================================================================================================
 *
 * DESCRIPTION:     Handles events sent to process 1
 *
 * PARAMETERS:      type:	event id
 *                  param1: addition info
 *                  param2: addition info
 *
 * RETURNS:         None.
 *
 * PRE-CONDITIONS:  None.
 *
 * POST-CONDITIONS: None.
 *
 * IMPORTANT NOTES: This process has the highest priority of all 10 user tasks.
 * 					This function runs on the user task #1 (on it's main loop).
 *					Running complex code here will block other events coming to this task.
 * ============================================================================================== */

INT32 M2M_msgProc1(INT32 type, INT32 param1, INT32 param2)
{


	M2M_LOG_INFO("Welcome to UART-to-Server Application!\r\n", 1);
	M2M_LOG_INFO("Getting IP...\r\n", 1);

	if(get_IP())
		M2M_LOG_INFO("IP Received: %s\r\n", IP);


    global_uart = m2m_hw_uart_open();


	//Define UART Call Back
	m2m_hw_uart_ioctl(global_uart, M2M_HW_UART_IO_BLOCKING_SET, (INT32)M2M_HW_UART_IO_BLOCKING_RELEASE);
	m2m_hw_uart_ioctl (global_uart, M2M_HW_UART_IO_RCV_FUNC, (INT32) hw_uart_read_cb);

	M2M_LOG_INFO("Please enter your data (To send row, please press ENTER key):\r\n");

	/* just to avoid warnings */
	return 0;
}


void M2M_msgProcCompl ( INT8 procId, INT32 type, INT32 result )
{
  /* write code fulfilling the requirements of the M2M project */
}
  
