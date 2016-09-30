/*
 * IpcmdOpStateMachineCommon.h
 *
 *  Created on: Sep 22, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDOPSTATEMACHINECOMMON_H_
#define INCLUDE_IPCMDOPSTATEMACHINECOMMON_H_

#include "../include/IpcmdDeclare.h"
#include "../include/IpcmdOpStateMachine.h"

#define DECLARE_SM_ENTRY(fn_name) \
	static gint fn_name(IpcmdOpState *op_state, enum _IpcmdOpCtxTriggers trigger, gconstpointer data)

/*******************************************
 * @fn: DoAction_NotDetermined
 * Not determined action. program should not be here.
 * @ op_state :
 * @ trigger :
 * @ data : NULL
 *
 *******************************************/
extern gint DoAction_NotDetermined(IpcmdOpState *op_state, enum _IpcmdOpCtxTriggers trigger, gconstpointer data);
/*******************************************
 * @fn: DoAction_Ignore
 * The trigger is acceptable, but will be ignored.
 * @ op_state :
 * @ trigger :
 * @ data : NULL
 *
 *******************************************/
extern gint DoAction_Ignore(IpcmdOpState *op_state, enum _IpcmdOpCtxTriggers trigger, gconstpointer data);

#endif /* INCLUDE_IPCMDOPSTATEMACHINECOMMON_H_ */
