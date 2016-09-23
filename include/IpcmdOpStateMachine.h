/*
 * IpcmdOpStateMachine.h
 *
 *  Created on: Sep 9, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDOPSTATEMACHINE_H_
#define INCLUDE_IPCMDOPSTATEMACHINE_H_

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
	kIpcmdTriggerRecvResp,
	kIpcmdTriggerSendResp,
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
} IpcmdOpCtxFinCode;

struct _IpcmdOpState {
	enum _OpContextState	state_;
	gint					fin_code_;
	IpcmdOpStateMachine*	pSM;
};

typedef gint (*SMDoAction)(IpcmdOpState *op_state, enum _IpcmdOpCtxTrigger trigger, gconstpointer data);
struct _IpcmdOpStateMachine {
	SMDoAction actions[kOpContextStateEnd][kIpcmdTriggerEnd];
};

/* connection-less (such as UDP) state machines */
extern IpcmdOpStateMachine SM_CL_Request;
/*
extern IpcmdOpStateMachine SM_CL_SetRequestNoReturn;
extern IpcmdOpStateMachine SM_CL_SetRequest;
extern IpcmdOpStateMachine SM_CL_NotificationRequest;
extern IpcmdOpStateMachine SM_CL_Notification;
*/
//Initialize all StateMachines
void IpcomOpStateMachineInit();

G_END_DECLS

#endif /* INCLUDE_IPCMDOPSTATEMACHINE_H_ */
