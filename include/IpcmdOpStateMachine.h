/*
 * IpcmdOpStateMachine.h
 *
 *  Created on: Sep 9, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDOPSTATEMACHINE_H_
#define INCLUDE_IPCMDOPSTATEMACHINE_H_

#include "IpcmdOperationContext.h"
#include <glib.h>

G_BEGIN_DECLS

typedef struct _IpcmdOpStateMachine IpcmdOpStateMachine;
typedef struct _IpcmdOpState		IpcmdOpState;
typedef enum _IpcmdOpCtxTrigger 	IpcmdOpCtxTrigger;
typedef enum _OpContextState		OpContextState;

enum _IpcmdOpCtxTrigger {
	kIpcmdTriggerStart = 0,
	/* operation type specific triggers */
	kIpcmdTriggerRecvRequest=0,
	kIpcmdTriggerRecvSetnor,
	kIpcmdTriggerRecvSetreq,
	kIpcmdTriggerRecvNotreq,
	kIpcmdTriggerSendRequest,
	kIpcmdTriggerSendSetnor,
	kIpcmdTriggerSendSetreq,
	kIpcmdTriggerSendNotreq,
	kIpcmdTriggerSendNotification,
	/* common triggers */
	kIpcmdTriggerRecvAck,
	kIpcmdTriggerRecvError,
	kIpcmdTriggerStartAppProcess,
	kIpcmdTriggerCompletedAppProcess,
	kIpcmdTriggerWFATimeout,
	kIpcmdTriggerWFRTimeout,
	/* The end of triggers */
	kIpcmdTriggerEnd,
};

enum _OpContextState {
	kOpContextStateStart = 0,
	/* common state */
	kOpContextStateIdle = 0,
	kOpContextStateAckRecv,
	kOpContextStateRespRecv,
	kOpContextStateRespSent,
	kOpContextStateAppProcess,
	kOpContextStateErrorSent,
	kOpContextStateFinalize,
	/* operation type specific */
	kOpContextStateReqSent,
	kOpContextStateReqRecv,
	kOpContextStateSetnorSent,
	kOpContextStateSetnorRecv,
	kOpContextStateNotreqSent,
	kOpContextStateNotreqRecv,
	kOpContextStateSetreqSent,
	kOpContextStateSetreqRecv,
	/* The end of State */
	kOpContextStateEnd,
};

typedef enum {
	OPCONTEXT_FINCODE_NORMAL = 0,
	OPCONTEXT_FINCODE_UNKNOWN_FAILURE,
	OPCONTEXT_FINCODE_TRANSMIT_FAILED,
	OPCONTEXT_FINCODE_NO_RESPONSE,
	OPCONTEXT_FINCODE_APP_ERROR,					// Error is happened while application processes the operation.
	OPCONTEXT_FINCODE_CANCELLED,
	OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES,			// = 5
	OPCONTEXT_FINCODE_PEER_NOT_OK,					// Peer application reports an error
	OPCONTEXT_FINCODE_PEER_SERVICEID_NOT_AVAILABLE,
	OPCONTEXT_FINCODE_PEER_OPERATIONID_NOT_AVAILABLE,
	OPCONTEXT_FINCODE_PEER_INVALID_PROTOCOL_VERSION,
	OPCONTEXT_FINCODE_PEER_PROCESSING,
	OPCONTEXT_FINCODE_PEER_INVALID_LENGTH,
	OPCONTEXT_FINCODE_PEER_APPLICATION_ERROR,
	OPCONTEXT_FINCODE_PEER_BUSY,
} IpcomOpContextFinCode;

struct _IpcomOpState {
	enum _OpContextState	state_;
	gint					fin_code_;
	IpcmdOpStateMachine*	pSM;
};

typedef gint (*SMDoAction)(IpcomOpState *op_state, enum _IpcmdOpCtxTrigger trigger, gpointer data);
struct _IpcomOpStateMachine {
	SMDoAction actions[kOpContextStateEnd][kIpcmdTriggerEnd];
};

/* connection-less (such as UDP) state machines */
extern IpcmdOpStateMachine SM_CL_Request;
extern IpcmdOpStateMachine SM_CL_SetRequestNoReturn;
extern IpcmdOpStateMachine SM_CL_SetRequest;
extern IpcmdOpStateMachine SM_CL_NotificationRequest;
extern IpcmdOpStateMachine SM_CL_Notification;

#if 0
/*
 * REQUEST: message from the client to the server invoking a operation.
 * RESPONSE: A message from the server to the client transporting the result of a operation invocation.
 */
extern IpcmdOpStateMachine SM_CLRO;		//Request Only / connectionless
extern IpcmdOpStateMachine SM_CLRR;		//Request and Response / connectionless
extern IpcmdOpStateMachine SM_CLNoti;	//Notification / connectionless
#endif

//Initialize all StateMachines
void IpcomOpStateMachineInit();

G_END_DECLS

#endif /* INCLUDE_IPCMDOPSTATEMACHINE_H_ */
