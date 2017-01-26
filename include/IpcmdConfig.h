/*
 * IpcmdConfig.h
 *
 *  Created on: Sep 22, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDCONFIG_H_
#define INCLUDE_IPCMDCONFIG_H_

#include <glib.h>

G_BEGIN_DECLS

typedef struct {
	guint16	service_id;
	guint16	operation_id;
} __attribute__ ((packed)) IpcmdConfigSignalId;

typedef struct {
	gint	defaultTimeoutWFA;		//milliseconds
	gfloat	increaseTimerValueWFA;
	gint	numberOfRetriesWFA;
	gint	defaultTimeoutWFR;		//milliseconds
	gfloat	increaseTimerValueWFR;
	gint	numberOfRetriesWFR;
} IpcmdConfigSignalParams ;

typedef struct _IpcmdConfig {
	IpcmdConfigSignalParams	defaultSignalParams;
	guint16		maxConcurrentMessages;
	GHashTable	*signalSpecificParams;	// signal specific parameters
										// key: IpcmdConfigSignalId, value: IpcmdConfigSignalParams
} IpcmdConfig;

IpcmdConfig	*IpcmdConfigGetInstance();
//gboolean	IpcmdConfigLoadFromFile(IpcmdConfig *config, const gchar *fname);
const IpcmdConfigSignalParams *IpcmdConfigGetSignalParams (IpcmdConfig *config, guint16 service_id, guint16 operation_id);

G_END_DECLS

#endif /* INCLUDE_IPCMDCONFIG_H_ */
