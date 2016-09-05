/*
 * IpcmdClient.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDCLIENT_H_
#define INCLUDE_IPCMDCLIENT_H_

typedef struct _IpcmdClient IpcmdClient;

struct _IpcmdOperationCallback {
	gint		(*cb_func)(OpHandle handle, gconstpointer data_from_caller, gpointer data_from_callee);
	// On destroying IpcmdOperationCallback, cb_destroy() is called to free cb_data.
	// If it is NULL, g_free() is used instead.
	gint		(*cb_destroy)(gpointer *cb_data);
	gpointer	cb_data;
};
typedef struct _IpcmdOperationCallback IpcmdOperationCallback;

#endif /* INCLUDE_IPCMDCLIENT_H_ */
