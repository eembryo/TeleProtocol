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

typedef struct _IpcmdConfig {
	gint	defaultTimeoutWFA;		//milliseconds
	gfloat	increaseTimerValueWFA;
	gint	numberOfRetriesWFA;
	gint	defaultTimeoutWFR;		//milliseconds
	gfloat	increaseTimerValueWFR;
	gint	numberOfRetriesWFR;
	guint16	maximumConcurrentMessages;
} IpcmdConfig;

IpcmdConfig	*IpcmdConfigGetInstance();
void		IpcmdConfigLoadFromFile(gchar *filename);

G_END_DECLS

#endif /* INCLUDE_IPCMDCONFIG_H_ */
