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

/*==================================================================================================
                            LOCAL CONSTANT DEFINITION
==================================================================================================*/

/*==================================================================================================
                            LOCAL TYPES DEFINITION
==================================================================================================*/
typedef enum
{
	M2M_KEY_ACTION_RELEASE,
	M2M_KEY_ACTION_PRESS,
	M2M_KEY_ACTION_HOLD
} M2M_KEY_ACTION_T;

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

/*==================================================================================================
                            GLOBAL VARIABLES
==================================================================================================*/
extern UINT32 hw_count;
/*==================================================================================================
                            LOCAL FUNCTIONS IMPLEMENTATION
==================================================================================================*/

/*==================================================================================================
                            GLOBAL FUNCTIONS IMPLEMENTATION
==================================================================================================*/

/* =================================================================================================
 *
 * DESCRIPTION:     Handles wake up event set by m2m_rtc_set_alarm function. It is responsibility of the programmer to
 *					write, within the M2M_onWakeup callback, the code to fulfill the requirements of the M2M application.
 *
 * PARAMETERS:      None.
 *
 * RETURNS:         None.
 *
 * PRE-CONDITIONS:  None.
 *
 * POST-CONDITIONS: None.
 *
 * IMPORTANT NOTES: This function runs on the modem task on it's main loop (no M2M task) and hence should not run complex
 *					operations.
 * ============================================================================================== */
 
void M2M_onWakeup(void)
{

	/* write code fulfilling the requirements of the M2M project */

}

/* =================================================================================================
 *
 * DESCRIPTION:     Handles GPIO interrupt event set by m2m_hw_gpio_int_enable function.It is responsibility of the
 *					programmer to write, within the M2M_onInterrupt callback, the code to fulfill the requirements
 *					of the M2M application.
 *
 * PARAMETERS:      gpio: the GPIO port number that has generated the interrupt.
 *
 * RETURNS:         None.
 *
 * PRE-CONDITIONS:  None.
 *
 * POST-CONDITIONS: None.
 *
 * IMPORTANT NOTES: This function runs on the modem task (no M2M tasks) and hence must not run any complex/heavy code.
 * ============================================================================================== */
 
void M2M_onInterrupt(INT32 gpio)
{

  /* write code fulfilling the requirements of the M2M project */

}

/* =================================================================================================
 *
 * DESCRIPTION:     On Hardware Timers expiration the M2M_onHWTimer is automatically started. "timer_id"
 *					identifies the expired timer. It is responsibility of the programmer to write, within
 *					the M2M_onHWTimer function, the code to fulfill the requirements of the M2M application.
 *
 * PARAMETERS:      None.
 *
 * RETURNS:         None.
 *
 * PRE-CONDITIONS:  None.
 *
 * POST-CONDITIONS: None.
 *
 * IMPORTANT NOTES: This function runs on the modem task and hence must not run any complex/heavy code.
 * ============================================================================================== */

void M2M_onHWTimer ( INT32 timer_id )
{
	if (timer_id == 7)
	{
		hw_count++;
		m2m_hw_timer_start(7,10); /*call hw timer 7 callback after 1 ms*/
	}
  /* Write code fulfilling the requirements of the M2M project */

}
/* =================================================================================================
 *
 * DESCRIPTION:     Handles of Usb cable event. Input could be 1 = USB_CABLE_ATTACHED or 
 *                  0 = USB_CABLE_DETACHED. It is responsibility of the programmer to write,
 *					        within the M2M_onUSbCableEvent callback, the code to fulfill the requirements
 *					        of the M2M application. Do not write heavy code inside as it is a callback
 *
 * PARAMETERS:      usb_cable_event: USB_CABLE_ATTACHED or USB_CABLE_DETACHED
 *
 * RETURNS:         None.
 *
 * PRE-CONDITIONS:  None.
 *
 * POST-CONDITIONS: None.
 *
 * IMPORTANT NOTES: This function runs on the modem task (no M2M tasks) and hence must not run any complex/heavy code.
 * ============================================================================================== */
 
void M2M_onUSbCableEvent(UINT32 usb_cable_event)
{

 /* Write code fulfilling the requirements of the M2M project */
 
}

/* =================================================================================================
 *
 * DESCRIPTION:    THIS FUNCTION IS NOT IMPLEMENTED, DO NOT USE IT
 *
 * PARAMETERS:      /
 *
 * RETURNS:         /
 *
 * PRE-CONDITIONS:  /
 *
 * POST-CONDITIONS: /
 *
 * IMPORTANT NOTES: /
 *
 * ============================================================================================== */

void M2M_onI2CEvent (M2M_I2C_EVENT event, UINT16 sda_pin, UINT16 scl_pin, UINT32 address, UINT16 reg_addr, UINT16 *buffer, UINT32 length)
{

  /* THIS FUNCTION IS NOT IMPLEMENTED, DO NOT USE IT */
  
}

/* =================================================================================================
 *
 * DESCRIPTION:     Handles key events. No API is needed to enable the execution of the M2M_onKeyEvent (…)
 *                  callback function. Only actions on the "ON" key wake up the M2M_onKeyEvent() callback.
 *
 * PARAMETERS:      val1: action on the key: pressed, released and hold. Actions on the key are defined
 *						  by M2M_KEY_ACTION_T enum
 *                  val2: key code
 *
 * RETURNS:         None.
 *
 * PRE-CONDITIONS:  None.
 *
 * POST-CONDITIONS: None.
 *
 * IMPORTANT NOTES: This function runs on the modem task and hence must not run any complex/heavy code.
 *
 * ============================================================================================== */
void M2M_onKeyEvent(INT32 val1, INT32 val2)
{

	/* write code fulfilling the requirements of the M2M project */
}


