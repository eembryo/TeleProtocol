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
typedef enum _IpcmdOpCtxTriggers 	IpcmdOpCtxTriggers;
typedef enum _OpContextStates		OpContextStates;

enum _IpcmdOpCtxTriggers {
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
	kIpcmdTriggerSendNoti,
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

enum _OpContextStates {
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
	kOpContextStateNotiSent,
	/* The end of State */
	kOpContextStateEnd,
};

typedef enum {
	OPCONTEXT_FINCODE_NORMAL = 0,
	OPCONTEXT_FINCODE_RECEIVE_ERROR_MESSAGE,
	OPCONTEXT_FINCODE_UNKNOWN_FAILURE,
	OPCONTEXT_FINCODE_TRANSMIT_FAILED,
	OPCONTEXT_FINCODE_NO_RESPONSE,
	OPCONTEXT_FINCODE_APP_ERROR,					// Error is happened while application processes the operation.
	OPCONTEXT_FINCODE_CANCELLED,
	OPCONTEXT_FINCODE_EXCEED_MAX_RETRIES,			// = 5
	OPCONTEXT_FINCODE_INVALID_STATE,
} IpcmdOpCtxFinCode;

struct _IpcmdOpState {
	enum _OpContextStates	state_;
	IpcmdOpStateMachine*	pSM;
	gint					fin_code_;
};

typedef gint (*SMDoAction)(IpcmdOpState *op_state, enum _IpcmdOpCtxTriggers trigger, gconstpointer data);
struct _IpcmdOpStateMachine {
	SMDoAction actions[kOpContextStateEnd][kIpcmdTriggerEnd];
};

/* connection-less (such as UDP) state machines */
extern IpcmdOpStateMachine SM_CL_Request;
extern IpcmdOpStateMachine SM_CL_Notreq;
extern IpcmdOpStateMachine SM_CL_Setnor;
extern IpcmdOpStateMachine SM_CL_Setreq;
extern IpcmdOpStateMachine SM_CL_Noti;

G_END_DECLS

#endif /* INCLUDE_IPCMDOPSTATEMACHINE_H_ */
