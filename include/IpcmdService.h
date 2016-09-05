/*
 * IpcmdService.h
 *
 *  Created on: Aug 30, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDSERVICE_H_
#define INCLUDE_IPCMDSERVICE_H_

typedef struct _IpcmdService IpcmdService;

struct _IpcmdService {
	guint16		service_id_;

	void		(*ExecOperation)(OpHandle handle, const IpcmdOperation *opeartion);
};


#endif /* INCLUDE_IPCMDSERVICE_H_ */
