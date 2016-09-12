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

typedef enum {
	OPCONTEXT_STATUS_START = 0,
	OPCONTEXT_STATUS_NONE = 0,
	OPCONTEXT_STATUS_REQUEST_SENT,
	OPCONTEXT_STATUS_ACK_RECV,
	OPCONTEXT_STATUS_PROCESS_RESPONSE,
	OPCONTEXT_STATUS_PROCESS_REQUEST,
	OPCONTEXT_STATUS_RESPONSE_SENT,				// 5
	OPCONTEXT_STATUS_NOTIFICATION_SENT,
	OPCONTEXT_STATUS_FINALIZE,		//will be destroyed
	OPCONTEXT_STATUS_END,
} IpcmdOpContextState;

typedef enum {
	OPCONTEXT_TRIGGER_START = 0,
	OPCONTEXT_TRIGGER_SEND_REQUEST = 0,
	OPCONTEXT_TRIGGER_SEND_RESPONSE,
	OPCONTEXT_TRIGGER_RECV_REQUEST,
	OPCONTEXT_TRIGGER_RECV_RESPONSE,
	OPCONTEXT_TRIGGER_RECV_ACK,
	OPCONTEXT_TRIGGER_PROCESS_DONE,				// 5
	OPCONTEXT_TRIGGER_SEND_NOTIFICATION,
	OPCONTEXT_TRIGGER_WFA_EXPIRED,
	OPCONTEXT_TRIGGER_WFR_EXPIRED,
	OPCONTEXT_TRIGGER_FINALIZE,
	OPCONTEXT_TRIGGER_END,						// 10
} IpcmdOpContextTriggers;

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
	IpcmdOpContextState		state_;
	gint					fin_code_;
	IpcmdOpStateMachine*	pSM;
};

typedef gint (*SMDoAction)(IpcomOpState *pOpState, IpcmdOpContextTriggers trigger, gpointer data);
struct _IpcomOpStateMachine {
	SMDoAction actions[OPCONTEXT_STATUS_END][OPCONTEXT_TRIGGER_END];
};

typedef gint (*SMDoAction)(IpcomOpState *pOpState, IpcmdOpContextTriggers trigger, gpointer data);

/*
 * REQUEST: message from the client to the server invoking a operation.
 * RESPONSE: A message from the server to the client transporting the result of a operation invocation.
 */
extern IpcmdOpStateMachine SM_CLRO;		//Request Only / connectionless
extern IpcmdOpStateMachine SM_CLRR;		//Request and Response / connectionless
extern IpcmdOpStateMachine SM_CLNoti;	//Notification / connectionless

//Initialize all StateMachines
void IpcomOpStateMachineInit();

G_END_DECLS

#endif /* INCLUDE_IPCMDOPSTATEMACHINE_H_ */
