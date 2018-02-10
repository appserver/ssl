/*
 * m2m_log_defs.h
 *
 *  Created on: 11/ago/2016
 *      Author: FabioPi
 */
#ifndef HDR_M2M_LOG_DEFS_H_
#define HDR_M2M_LOG_DEFS_H_

#define M2M_UART_LOG 		1
#define M2M_USB_LOG			3


	/* =========================================================================
	 *
	 *  Set LOG_OUTPUT to
	 * - M2M_UART_LOG to print logs on UART
	 *
	 * - M2M_USB_LOG to print logs on USB.
	 * 		M2M_USB_CHANNEL can be defined. Please refer to M2M_USB_CH enum.
	 * 		If not defined, it defaults to USB_CH0
	 *
	 ===========================================================================*/
	#define M2M_LOG_OUTPUT M2M_UART_LOG
	//#define M2M_LOG_OUTPUT M2M_USB_LOG


	#if M2M_LOG_OUTPUT == M2M_USB_LOG
	/* =========================================================================
	 *
	 *	   If USB is used, change the USB_CHANNEL define according to the project
	 *	   (see M2M_USB_CH enumeration). Defaults to USB0.
	 *
	 ===========================================================================*/
	#define M2M_USB_CHANNEL USB_CH0
	#endif

#endif /* HDR_M2M_LOG_DEFS_H_ */
