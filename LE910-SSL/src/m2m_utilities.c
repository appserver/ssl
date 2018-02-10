/*=================================================================
#Telit Extensions
#
#Copyright © 2016, Telit Communications S.p.A.
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
#Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#
#Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in
#the documentation and/or other materials provided with the distribution.
#
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS``AS IS'' AND ANY EXPRESS OR IMPLIED
#WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
#DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#POSSIBILITY OF SUCH DAMAGE.
#
==============================================================*/

/*==================================================================================================
                            INCLUDE FILES
==================================================================================================*/

#include <stdarg.h>
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
#include "m2m_ssl_api.h"
#include "m2m_sms_api.h"
#include "m2m_network_api.h"

#include "m2m_utilities.h"

/*==================================================================================================
                            LOCAL CONSTANT DEFINITION
==================================================================================================*/

/*==================================================================================================
                            LOCAL TYPES DEFINITION
==================================================================================================*/

/*==================================================================================================
                            LOCAL FUNCTION PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                            GLOBAL FUNCTIONS PROTOTYPES
==================================================================================================*/

/*==================================================================================================
                            LOCAL MACROS
==================================================================================================*/

/*==================================================================================================
                            LOCAL VARIABLES
==================================================================================================*/
M2M_T_OS_LOCK lock_handle = NULL;
M2M_T_OS_LOCK USBlock_handle[USB_INSTANCES];

M2M_LOG_LEVEL GLOBAL_LOG_LEVEL = M2M_INFO; //default value
char netEventoutput[64];

/*==================================================================================================
                            GLOBAL VARIABLES
==================================================================================================*/
UINT32 hw_count = 0;
/*==================================================================================================
                            LOCAL FUNCTIONS IMPLEMENTATION
==================================================================================================*/

/*==================================================================================================
                            GLOBAL FUNCTIONS IMPLEMENTATION
==================================================================================================*/

void setM2MLogLevel(M2M_LOG_LEVEL level){
	GLOBAL_LOG_LEVEL = level;
	m2m_hw_timer_start(7,10); /*call hw timer 7 callback after 1 ms*/
}

M2M_LOG_LEVEL getM2MLogLevel(void){
	return GLOBAL_LOG_LEVEL;
}

const char* fileTitle(const char* path)
{
    const char* p = path;

    while (*p) {
        if (*p == '/' || *p == '\\') {
            return p + 1;
        }

        p++;
    }

    return path;
}

UINT32 getUptime(void)
{
	UINT32 uptime;
	uptime = m2m_os_retrieve_clock() * 100; //milliseconds, 100ms granularity
	return uptime;
}

UINT32 getHWUptime(void)
{
	return hw_count;
}

int uptimeAsString(UINT32 uptime, char * string)
{
	if (!string){
		return -1;
	}
	return sprintf(string, "%6u.%03u", uptime / 1000, uptime %1000 );
}

//##################################################################################################################################
/**
 *  \brief Waits until cellular registration (blocking, no timeout)
 *	\param [in] timeout: the timeout in milliseconds (granularity: 100ms ) after which the function will fail if a registration was not occurred.
 *	\return 1 if the module registered before the timeout expiration, 0 otherwise
 *
 */
//##################################################################################################################################
int waitRegistration(UINT32 timeout)
{

	M2M_T_NETWORK_REG_STATUS_INFO reg_info;
	UINT32 timer = 0;
	while(1)
	{
		//first check the timeout
		if(timer >= timeout){
			M2M_LOG_ERROR("cannot get a network registration!\n");
			return 0;
		}
		if(m2m_network_get_reg_status(&reg_info)){
			M2M_LOG_VERB("Reg status: %d\n", reg_info.status);
			if (reg_info.status == 1 || reg_info.status == 5)
			{
				return 1;
			}
			m2m_os_sleep_ms(2000);
			timer +=2000;
		}

	}
}


#if 0 /*not available in 2G*/
//##################################################################################################################################
/**
 *  \brief Waits until cellular GPRS registration (blocking, no timeout)
 *	\param [in] timeout: the timeout in milliseconds (granularity: 100ms ) after which the function will fail if a registration was not occurred.
 *	\return 1 if the module registered before the timeout expiration, 0 otherwise
 *
 */
//##################################################################################################################################
int waitGPRSRegistration(UINT32 timeout)
{

	M2M_T_NETWORK_GREG_STATUS_INFO g_reg_info;
	UINT32 timer = 0;
	while(1)
	{
		//first check the timeout
		if(timer >= timeout){
			M2M_LOG_ERROR("cannot get a network registration!\n");
			return 0;
		}
		if(m2m_network_get_gprs_reg_status(&g_reg_info)){
			M2M_LOG_VERB("Reg status: %d\n", g_reg_info.gprs_status);
			if (g_reg_info.gprs_status == 1 || g_reg_info.gprs_status == 5)
			{
				return 1;
			}
			m2m_os_sleep_ms(2000);
			timer +=2000;
		}

	}
}

#endif

//##################################################################################################################################
/**
 *  \brief Prints on the defined stream (main UART or USB channel, see m2m_utilities.h)
 *
 *	\param [in] level: Logging level. see LOG_LEVEL enum
 *	\param [in] file: source file path to add to the output if log is verbose
 *	\param [in] line: source file line to add to the output if log is verbose
 *  \param [in] fmt : parameters to prints
 *  \param [in] ...  : ...
 *  \return no returned value: if unsuccessfully for any reason, it does not print anything
 *
 */
//##################################################################################################################################
void _M2M_LogFormat(M2M_LOG_LEVEL level, const char* file, int line, const CHAR *fmt, ...)
{

	INT32	sent;
	va_list arg;
	//CHAR	buf[750];
	/* FabioPi 2016-06-13 Changed to dynamic allocation*/

	CHAR *buf = NULL;
	INT32   offset = 0;
	UINT32 now = getHWUptime();
	int bufSize = 1024;
	buf = m2m_os_mem_alloc(sizeof(CHAR) * bufSize);
	if (!buf) return;

	memset(buf,0,bufSize);



	/* If the selected log level is high enough create the message and print it */
	if (level >= getM2MLogLevel()) {
		switch(level) {
		case M2M_DEBUG_MORE:
			offset = sprintf(buf, "%5u.%03u %10s - %15s:%-4d {%d} ",
					now / 1000, now % 1000,
					"++DEBUG+",
					fileTitle(file), line,
					m2m_os_get_current_task_id());
			break;
		case M2M_DEBUG:
			offset = sprintf(buf, "%5u.%03u %10s - %15s:%-4d {%d} ",
					now / 1000, now % 1000,
					"++DEBUG",
					fileTitle(file), line,
					m2m_os_get_current_task_id());
			break;
		case M2M_VERB:
			offset = sprintf(buf, "%5u.%03u %10s - %15s:%-4d {%d} ",
					now / 1000, now % 1000,
					"--VERB",
					fileTitle(file), line,
					m2m_os_get_current_task_id());
			break;

		case M2M_INFO:
			offset = 0;
			break;

		case M2M_WARN:
			offset = sprintf(buf, "%5u.%03u %10s - %15s:%-4d {%d} ",
					now / 1000, now % 1000,
					"##WARNING",
					fileTitle(file), line,
					m2m_os_get_current_task_id());
			break;

		case M2M_ERROR:
			offset = sprintf(buf, "%5u.%03u %10s - %15s:%-4d {%d} ",
					now / 1000, now % 1000,
					"**ERROR",
					fileTitle(file), line,
					m2m_os_get_current_task_id());
			break;

		default:
			break;
		}

		va_start(arg, fmt);
		vsnprintf(buf + offset, bufSize-offset, fmt, arg);
		va_end(arg);

		/* Print the message on the selected output stream */
		_M2M_logBase(buf);

		m2m_os_mem_free(buf);
		buf = NULL;
	}

}




//##################################################################################################################################
/**
 *  \brief Print directly on the main UART
 *
 *	\param [in] message: the string to print
 *  \return no returned value: if unsuccessfully for any reason, it does not print anything
 *
 */
//##################################################################################################################################
void _printToUart(const char *message)
{
	INT32	sent;
	M2M_T_HW_UART_HANDLE local_fd = M2M_HW_UART_HANDLE_INVALID;

	if (NULL == lock_handle) {
		lock_handle = m2m_os_lock_init(M2M_OS_LOCK_CS);
	}

	m2m_os_lock_lock(lock_handle);

	/* Get a UART handle first */
	local_fd = m2m_hw_uart_open();

	if (M2M_HW_UART_HANDLE_INVALID != local_fd) {
		m2m_hw_uart_write(local_fd, (char*) message, strlen(message), &sent);
		//m2m_hw_uart_write(local_fd, "\r\n", 2, &sent);

		/* in case of concurrency using m2m_hw_uart... comment the next API to avoid the closing */
		//m2m_hw_uart_close(local_fd);
	}

	m2m_os_lock_unlock(lock_handle);
}



//##################################################################################################################################
/**
 *  \brief Print as PrintToUart but using a specified USB channel
 *
 *  \param [in] ch: USB channel where to print
 *  \param [in] message : Message to print
 *  \return no returned value: if unsuccessfully for any reason, it does not print anything
 *
 *  \details Using channel:USB_CH_DEFAULT uses channel assigned to instance USER_USB_INSTANCE_0
 */
//##################################################################################################################################
 void _printToUSB (M2M_USB_CH ch, const char *message )
{
	INT32	sent;
	USER_USB_INSTANCE_T  UsbInstance;

	M2M_T_HW_USB_HANDLE local_USBfd = M2M_HW_USB_UART_HANDLE_INVALID_PORT;
	UsbInstance = m2m_hw_usb_get_instance(ch);
	if (UsbInstance >= USER_USB_INSTANCE_ERR)                  // no istance found
	{
		UsbInstance = m2m_hw_usb_get_instance(USB_CH_NONE);  // search for new istance
		if (UsbInstance >= USER_USB_INSTANCE_ERR)
			return;                                               // no istances available
	}

	if (NULL == USBlock_handle[UsbInstance])
	{
		USBlock_handle[UsbInstance] = m2m_os_lock_init(M2M_OS_LOCK_CS);
	}

	m2m_os_lock_lock(USBlock_handle[UsbInstance]);


	/* Get a USB handle first */
	if ( M2M_API_RESULT_SUCCESS != m2m_hw_usb_open(ch, &local_USBfd) )
	{
		//destroy lock related to USB that can not be opened
		m2m_os_lock_destroy(USBlock_handle[UsbInstance]);
		//free for later use, if any
		USBlock_handle[UsbInstance] = NULL;
		return;
	}

	(void)m2m_hw_usb_write ( local_USBfd, (char*) message, strlen(message), &sent );
	//(void)m2m_hw_usb_write(local_USBfd, "\r\n", 2, &sent);

	/* in case of concurrency using m2m_hw_usb... comment the next API to avoid the closing */
	(void)m2m_hw_usb_close(local_USBfd);

	m2m_os_lock_unlock(USBlock_handle[UsbInstance]);
}

 char *NetEventString(M2M_NETWORK_EVENT event)
 {
	 memset(netEventoutput,0,sizeof(netEventoutput));
	 switch (event)
	 	{
	 		// PDP related
	 		case M2M_SOCKET_EVENT_PDP_ACTIVE:
	 			sprintf(netEventoutput, "M2M_SOCKET_EVENT_PDP_ACTIVE");

	 			break;
	 		case M2M_SOCKET_EVENT_PDP_BREAK:
	 			sprintf(netEventoutput, "M2M_SOCKET_EVENT_PDP_BREAK");

	 			break;
	 		case M2M_SOCKET_EVENT_PDP_DEACTIVE:
	 			sprintf(netEventoutput, "M2M_SOCKET_EVENT_PDP_DEACTIVE");
	 			break;
	 		case M2M_SOCKET_EVENT_PDP_ACTIVATION_FAIL:
	 			sprintf(netEventoutput,"M2M_SOCKET_EVENT_PDP_ACTIVATION_FAIL!");
	 			break;
	 		case M2M_SOCKET_EVENT_PDP_IPV6_ACTIVE:
	 			sprintf(netEventoutput,"M2M_SOCKET_EVENT_PDP_IPV6_ACTIVE");
	 			break;
	 		default:
	 			sprintf(netEventoutput,"M2M NET EVENT: %d\n", event);
	 			break;
	 	}
	 return netEventoutput;
 }
