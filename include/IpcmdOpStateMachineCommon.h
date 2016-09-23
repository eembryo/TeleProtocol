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
	static int fn_name(IpcmdOpState *op_state, IpcmdOpCtxTrigger trigger, gconstpointer data)

/*******************************************
 * Not determined action. program should not be here.
 * @ op_state :
 * @ trigger :
 * @ data : NULL
 *
 *******************************************/
DECLARE_SM_ENTRY(DoAction_NotDetermined)
{
	g_error("Not determined action.\n");
	return -1;
}

/*******************************************
 * The trigger is acceptable, but will be ignored.
 * @ op_state :
 * @ trigger :
 * @ data : NULL
 *
 *******************************************/
DECLARE_SM_ENTRY(DoAction_Ignore)
{
	//silently ignore
	return op_state->state_;
}

#endif /* INCLUDE_IPCMDOPSTATEMACHINECOMMON_H_ */
