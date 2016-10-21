/*
 * IpcmdConfig.c
 *
 *  Created on: Sep 23, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdConfig.h"
#include <glib.h>

static IpcmdConfig *ipcmd_config_instance = NULL;
static void IpcmdConfigInit(IpcmdConfig *config);

IpcmdConfig	*
IpcmdConfigGetInstance()
{
	if (!ipcmd_config_instance) {
		ipcmd_config_instance = g_malloc0(sizeof(struct _IpcmdConfig));
		IpcmdConfigInit(ipcmd_config_instance);
	}

	return ipcmd_config_instance;
}

void
IpcmdConfigLoadFromFile(gchar *filename)
{
}

static void
IpcmdConfigInit(IpcmdConfig *config)
{
#ifdef USE_PCL_LOCALCONFIG
#define PCL_LOCALCONFIG_DBID	0x50
#define PCL_READ_BUF_SIZE		4096
    gchar readBuf[PCL_READ_BUF_SIZE] = {0};
    gint ret;

    pclInitLibrary(aAppId, PCL_SHUTDOWN_TYPE_NONE);

    ret = pclKeyReadData(PCL_LOCALCONFIG_DBID, aResourceId, 0, 0, readBuf, PCL_READ_BUF_SIZE);

     pclLifecycleSet(PCL_SHUTDOWN);

    pclDeinitLibrary();
#else	//!USE_PCL_LOCALCONFIG
	/* REQPROD 346925: Default WFA value
	 * The default acknowledgment timeout value shall be named, defaultTimeoutWFA.
	 * This timer shall be set to 500 milliseconds as default value.
	 */
	config->defaultTimeoutWFA = 500;
	/* REQPROD 346978: Default WFR value
	 * The default response timeout value shall be named defaultTimeoutWFR.
	 * This timer shall be set to 1 second as default value.
	 */
	config->defaultTimeoutWFR = 1000;

	/* no specification about increaseTimerValue and numberOfRetries. follow the example in REQPROD 360604. */
	config->increaseTimerValueWFA = 1.5;
	config->numberOfRetriesWFA = 6;
	config->increaseTimerValueWFR = 2;
	config->numberOfRetriesWFR = 2;

	/* REQPROD 347045/MAIN;3 : Minimum concurrent message sequences */
	config->maximumConcurrentMessages = 10;
#endif
}
