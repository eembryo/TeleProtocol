/*
 * IpcmdOpStateMachine.c
 *
 *  Created on: Sep 19, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOpStateMachine.h"

extern void IpcmdOpStateMachine_CLREQUEST_Init(IpcmdOpStateMachine *SM);
extern void IpcmdOpStateMachine_CLSetreq_Init(IpcmdOpStateMachine *SM);
extern void IpcmdOpStateMachine_CLSetnor_Init(IpcmdOpStateMachine *SM);
extern void IpcmdOpStateMachine_CLNotreq_Init(IpcmdOpStateMachine *SM);
extern void IpcmdOpStateMachine_CLNoti_Init(IpcmdOpStateMachine *SM);


IpcmdOpStateMachine SM_CL_Request;
IpcmdOpStateMachine SM_CL_Setnor;
IpcmdOpStateMachine SM_CL_Notreq;
IpcmdOpStateMachine SM_CL_Setreq;
IpcmdOpStateMachine SM_CL_Noti;

void IpcmdOpStateMachineInit() __attribute__((constructor));

void IpcmdOpStateMachineInit()
{
	IpcmdOpStateMachine_CLREQUEST_Init(&SM_CL_Request);
	IpcmdOpStateMachine_CLSetnor_Init(&SM_CL_Setnor);
	IpcmdOpStateMachine_CLNotreq_Init(&SM_CL_Notreq);
	IpcmdOpStateMachine_CLSetreq_Init(&SM_CL_Setreq);
	IpcmdOpStateMachine_CLNoti_Init(&SM_CL_Noti);
}
