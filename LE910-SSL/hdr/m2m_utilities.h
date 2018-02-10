
#ifndef	_M2M_UTILITIES_H
#define _M2M_UTILITIES_H

#include "m2m_log_defs.h"
/* This file provides utility interfaces available to the M2M application. */


/* Log level enumeration */
typedef enum {
	M2M_DEBUG_MORE, /* Prints every message, adds a "DEBUG" at the beginning of the message*/
	M2M_DEBUG,	/* Prints most of the messages, adds a "DEBUG" at the beginning of the message*/
	M2M_VERB,	/* print source-file name, line of the instruction and task ID*/
	M2M_INFO, 	/* print the message only, without any additional info */
	M2M_WARN,   /* like VERB, but adds a "WARNING" at the beginning of the message. */
	M2M_ERROR,  /* like VERB, but adds an "ERROR" at the beginning of the message */
	M2M_NONE    /* Do not print anything */
} M2M_LOG_LEVEL;


/* PUBLIC functions*/

UINT32 getUptime(void);
UINT32 getHWUptime(void);
int uptimeAsString(UINT32 uptime, char * string);

void setM2MLogLevel(M2M_LOG_LEVEL level);
M2M_LOG_LEVEL getM2MLogLevel(void);


int waitRegistration(UINT32 timeout);
int waitGPRSRegistration(UINT32 timeout);

/* Message formatting function. It will print on UART or USB according with the defined output stream (see #define USBLOG above)*/
/* According to log level, prints additional information */
void _M2M_LogFormat ( M2M_LOG_LEVEL level, const char* file, int line, const CHAR *fmt, ... );


#define M2M_LOG_ERROR(a...) 		_M2M_LogFormat(M2M_ERROR, __FILE__, __LINE__, a)
#define M2M_LOG_WARN(a...)  		_M2M_LogFormat(M2M_WARN, __FILE__, __LINE__, a)
#define M2M_LOG_INFO(a...)  		_M2M_LogFormat(M2M_INFO, NULL, 0,  a)
#define M2M_LOG_VERB(a...)   		_M2M_LogFormat(M2M_VERB, __FILE__, __LINE__, a)
#define M2M_LOG_DEBUG(a...)   		_M2M_LogFormat(M2M_DEBUG, __FILE__, __LINE__, a)
#define M2M_LOG_DEBUG_MORE(a...)   	_M2M_LogFormat(M2M_DEBUG_MORE, __FILE__, __LINE__, a)


/* INTERNAL functions*/

/* Returns the file name from the complete path */
const char* fileTitle(const char* path);

/* Outputs the given data to the UART directly. */
void _printToUart(const char *message);

/* Outputs the given data to the specified USB channel  directly. */
void _printToUSB(M2M_USB_CH ch, const char *message);


#if M2M_LOG_OUTPUT == M2M_USB_LOG
	#ifndef M2M_USB_CHANNEL
		#define M2M_USB_CHANNEL USB_CH0  /*Set USB0 as default*/
	#endif /*USB_CHANNEL*/
	#define _M2M_logBase(a)	_printToUSB(M2M_USB_CHANNEL, a)
#else
	#define _M2M_logBase(a)	_printToUart(a)
#endif


char *NetEventString(M2M_NETWORK_EVENT event);
#endif  /* _M2M_FS_API_H */

