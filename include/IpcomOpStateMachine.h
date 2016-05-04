#ifndef __IpcomOpStateMachine_h__
#define __IpcomOpStateMachine_h__

#include <IpcomTypes.h>

G_BEGIN_DECLS

typedef struct _IpcomOpStateMachine IpcomOpStateMachine;
typedef struct _IpcomOpState		IpcomOpState;

struct _IpcomOpState {
	IpcomProtocolOpContextStatus	nState;
	gint					nFinCode;
	IpcomOpStateMachine*	pSM;
};

typedef gint (*SMDoAction)(IpcomOpState *pOpState, IpcomProtocolOpContextTriggers trigger, gpointer data);
struct _IpcomOpStateMachine {
	SMDoAction actions[OPCONTEXT_STATUS_END][OPCONTEXT_TRIGGER_END];
};

typedef gint (*SMDoAction)(IpcomOpState *pOpState, IpcomProtocolOpContextTriggers trigger, gpointer data);

extern IpcomOpStateMachine SM_CLRO;	//Request Only / connectionless
extern IpcomOpStateMachine SM_CLRR;	//Request and Response / connectionless
extern IpcomOpStateMachine SM_CLNoti;	//Notification / connectionless

//Initialize all StateMachines
void IpcomOpStateMachineInit();

G_END_DECLS

#endif
