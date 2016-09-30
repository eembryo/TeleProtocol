/*
 * IpcmdOpStateMahcineCommon.c
 *
 *  Created on: Sep 29, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdOpStateMachineCommon.h"
#include <glib.h>

gint
DoAction_NotDetermined(IpcmdOpState *op_state, enum _IpcmdOpCtxTriggers trigger, gconstpointer data)
{
	g_warning("Not determined action.(state:%d, trigger:%d)", op_state->state_, trigger);
	return -1;
}
gint
DoAction_Ignore(IpcmdOpState *op_state, enum _IpcmdOpCtxTriggers trigger, gconstpointer data)
{
	//silently ignore
	return op_state->state_;
}
