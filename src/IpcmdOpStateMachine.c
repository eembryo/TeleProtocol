/*
 * IpcmdOpStateMachine.c
 *
 *  Created on: Sep 19, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOpStateMachine.h"

extern void IpcmdOpStateMachine_CLREQUEST_Init(IpcmdOpStateMachine *SM);
//extern IpcmdOpStateMachine_CLSetRequestNoReturn_Init(IpcmdOpStateMachine *SM);
//extern IpcmdOpStateMachine_CLSetRequest_Init(IpcmdOpStateMachine *SM);
//extern IpcmdOpStateMachine_CLSetNotificationRequest_Init(IpcmdOpStateMachine *SM);
//extern IpcmdOpStateMachine_CLSetNotification_Init(IpcmdOpStateMachine *SM);

IpcmdOpStateMachine SM_CL_Request;
//IpcmdOpStateMachine SM_CL_SetRequestNoReturn;
//IpcmdOpStateMachine SM_CL_SetRequest;
//IpcmdOpStateMachine SM_CL_NotificationRequest;
//IpcmdOpStateMachine SM_CL_Notification;

void IpcomOpStateMachineInit()
{
	IpcmdOpStateMachine_CLREQUEST_Init(&SM_CL_Request);
	//IpcmdOpStateMachine_CLSetRequestNoReturn_Init(&SM_CL_SetRequestNoReturn);
	//IpcmdOpStateMachine_CLSetRequest_Init(&SM_CL_SetRequest);
	//IpcmdOpStateMachine_CLSetNotificationRequest_Init(&SM_CL_NotificationRequest);
	//IpcmdOpStateMachine_CLSetNotification_Init(&SM_CL_Notification);
}
