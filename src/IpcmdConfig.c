/*
 * IpcmdConfig.c
 *
 *  Created on: Sep 23, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdConfig.h"
#include <glib.h>
#include <stdio.h>
#include <string.h>

static IpcmdConfig *ipcmd_config_instance = NULL;
static void IpcmdConfigInit(IpcmdConfig *config);

#define KEY_STRING(KEYNAME) _KeyString_##KEYNAME
const gchar *_KeyString_DEFAULT_TIMEOUT_WFA = "DEFAULT_TIMEOUT_WFA";
const gchar *_KeyString_INCREASE_TIMER_VALUE_WFA = "INCREASE_TIMER_VALUE_WFA";
const gchar *_KeyString_NUMBER_OF_RETRIES_WFA = "NUMBER_OF_RETRIES_WFA";
const gchar *_KeyString_DEFAULT_TIMEOUT_WFR = "DEFAULT_TIMEOUT_WFR";
const gchar *_KeyString_INCREASE_TIMER_VALUE_WFR = "INCREASE_TIMER_VALUE_WFR";
const gchar *_KeyString_NUMBER_OF_RETRIES_WFR = "NUMBER_OF_RETRIES_WFR";
const gchar *_KeyString_MAX_CONCURRENT_MESSAGES = "MAX_CONCURRENT_MESSAGES";

// initial default value
#define IPCMDCONF_INIT_DEFAULT_TIMEOUT_WFA			500		//milliseconds
#define IPCMDCONF_INIT_INCREASE_TIMER_VALUE_WFA		1.5
#define IPCMDCONF_INIT_NUMBER_OF_RETRIES_WFA		6
#define IPCMDCONF_INIT_DEFAULT_TIMEOUT_WFR			1000	//milliseconds
#define IPCMDCONF_INIT_INCREASE_TIMER_VALUE_WFR		2
#define IPCMDCONF_INIT_NUMBER_OF_RETRIES_WFR		2
#define IPCMDCONF_INIT_MAX_CONCURRENT_MESSAGES		10

static void
_DumpConfiguredSignalParams(const IpcmdConfigSignalParams *params)
{
	g_print ("%s = %d\n", KEY_STRING(DEFAULT_TIMEOUT_WFA), params->defaultTimeoutWFA);
	g_print ("%s = %.2f\n", KEY_STRING(INCREASE_TIMER_VALUE_WFA), params->increaseTimerValueWFA);
	g_print ("%s = %d\n", KEY_STRING(NUMBER_OF_RETRIES_WFA), params->numberOfRetriesWFA);
	g_print ("%s = %d\n", KEY_STRING(DEFAULT_TIMEOUT_WFR), params->defaultTimeoutWFR);
	g_print ("%s = %.2f\n", KEY_STRING(INCREASE_TIMER_VALUE_WFR), params->increaseTimerValueWFR);
	g_print ("%s = %d\n", KEY_STRING(NUMBER_OF_RETRIES_WFR), params->numberOfRetriesWFR);
}
static void
_DumpConfiguredSettings()
{
	IpcmdConfig *config = IpcmdConfigGetInstance();

	g_print ("[DEFAULT]\n");
	g_print ("%s = %d\n", KEY_STRING(MAX_CONCURRENT_MESSAGES), config->maxConcurrentMessages);
	_DumpConfiguredSignalParams (&config->defaultSignalParams);

	{
		GHashTableIter iter;
		gpointer key, value;
		g_hash_table_iter_init (&iter, config->signalSpecificParams);
		while (g_hash_table_iter_next (&iter, &key, &value)) {
			IpcmdConfigSignalId *signal_id = (IpcmdConfigSignalId *)key;
			g_print ("\n[SIGNAL_0x%.4x_0x%.4x]\n", signal_id->service_id, signal_id->operation_id);
			_DumpConfiguredSignalParams ((const IpcmdConfigSignalParams *)value);
		}
	}
}

static guint
_SignalHashFunc (gconstpointer key)
{
	const IpcmdConfigSignalId *signal_id = (const IpcmdConfigSignalId *)key;
	return ((signal_id->service_id&0xFFFF)<<16) + (signal_id->operation_id&0xFFFF);
}
static gboolean
_SignalEqualFunc (gconstpointer a, gconstpointer b)
{
	return memcmp(a,b,sizeof(IpcmdConfigSignalId)) == 0 ? TRUE : FALSE;
}

IpcmdConfig	*
IpcmdConfigGetInstance()
{
	if (!ipcmd_config_instance) {
		ipcmd_config_instance = g_malloc0(sizeof(struct _IpcmdConfig));
		ipcmd_config_instance->signalSpecificParams = g_hash_table_new_full (_SignalHashFunc, _SignalEqualFunc, g_free, g_free);
		IpcmdConfigInit(ipcmd_config_instance);
	}

	return ipcmd_config_instance;
}

static gboolean
_HandleSignalSpecificParams(IpcmdConfig *config, GKeyFile *key_file, const gchar *group, const IpcmdConfigSignalId *signal_id)
{
	IpcmdConfigSignalParams *params = (IpcmdConfigSignalParams *)g_hash_table_lookup (config->signalSpecificParams, signal_id);
	GError *gerror = NULL;
	IpcmdConfigSignalId		*new_id = NULL;
	IpcmdConfigSignalParams	*new_params = NULL;
	guint64	ivalue;
	gdouble	dvalue;

	// if signal_id does not exist in hashtable, create new one.
	if (!params) {
		new_id = g_malloc0(sizeof(IpcmdConfigSignalId));
		new_params = g_malloc0(sizeof(IpcmdConfigSignalParams));
		if (!new_id || !new_params) goto _HandleSignalSpecificParams_error;

		*new_id = *signal_id;
		*new_params = config->defaultSignalParams;
		g_hash_table_insert (config->signalSpecificParams, new_id, new_params);

		params = new_params;
	}

	ivalue = g_key_file_get_uint64 (key_file, group, KEY_STRING (DEFAULT_TIMEOUT_WFA), &gerror);
	if (gerror) {g_error_free (gerror); gerror = NULL;}
	else params->defaultTimeoutWFA = ivalue;
	dvalue = g_key_file_get_double (key_file, group, KEY_STRING (INCREASE_TIMER_VALUE_WFA), &gerror);
	if (gerror) {g_error_free (gerror); gerror = NULL;}
	else params->increaseTimerValueWFA = dvalue;
	ivalue = g_key_file_get_uint64 (key_file, group, KEY_STRING (NUMBER_OF_RETRIES_WFA), &gerror);
	if (gerror) {g_error_free (gerror); gerror = NULL;}
	else params->numberOfRetriesWFA = ivalue;
	ivalue = g_key_file_get_uint64 (key_file, group, KEY_STRING (DEFAULT_TIMEOUT_WFR), &gerror);
	if (gerror) {g_error_free (gerror); gerror = NULL;}
	else params->defaultTimeoutWFR = ivalue;
	dvalue = g_key_file_get_double (key_file, group, KEY_STRING (INCREASE_TIMER_VALUE_WFR), &gerror);
	if (gerror) {g_error_free (gerror); gerror = NULL;}
	else params->increaseTimerValueWFR = dvalue;
	ivalue = g_key_file_get_uint64 (key_file, group, KEY_STRING (NUMBER_OF_RETRIES_WFR), &gerror);
	if (gerror) {g_error_free (gerror); gerror = NULL;}
	else params->numberOfRetriesWFR = ivalue;

	return TRUE;

	_HandleSignalSpecificParams_error:
	if (new_id) g_free (new_id);
	if (new_params) g_free (new_params);

	return FALSE;
}

gboolean
IpcmdConfigLoadFromFile(IpcmdConfig *config, const gchar *fname)
{
	GError *gerror = NULL;
	GKeyFile *key_file = g_key_file_new();

	if (!key_file) {
		g_warning("%s: Not enough memory", __func__);
		return FALSE;
	}

	if (!g_key_file_load_from_file (key_file, fname, G_KEY_FILE_NONE, &gerror))
		goto _IpcmdConfigLoadFromFile_error;

	// Read DEFAULT section
	if (g_key_file_has_group (key_file, "DEFAULT")) {
		guint64	ivalue;
		gdouble	dvalue;

		ivalue = g_key_file_get_uint64 (key_file, "DEFAULT", KEY_STRING (DEFAULT_TIMEOUT_WFA), &gerror);
		if (gerror) {g_error_free (gerror);	gerror = NULL;}
		else config->defaultSignalParams.defaultTimeoutWFA = ivalue;
		dvalue = g_key_file_get_double (key_file, "DEFAULT", KEY_STRING (INCREASE_TIMER_VALUE_WFA), &gerror);
		if (gerror) {g_error_free (gerror); gerror = NULL;}
		else config->defaultSignalParams.increaseTimerValueWFA = dvalue;
		ivalue = g_key_file_get_uint64 (key_file, "DEFAULT", KEY_STRING (NUMBER_OF_RETRIES_WFA), &gerror);
		if (gerror) {g_error_free (gerror);gerror = NULL;}
		else config->defaultSignalParams.numberOfRetriesWFA = ivalue;

		ivalue = g_key_file_get_uint64 (key_file, "DEFAULT", KEY_STRING (DEFAULT_TIMEOUT_WFR), &gerror);
		if (gerror) {g_error_free (gerror); gerror = NULL; }
		else config->defaultSignalParams.defaultTimeoutWFR = ivalue;
		dvalue = g_key_file_get_double (key_file, "DEFAULT", KEY_STRING (INCREASE_TIMER_VALUE_WFR), &gerror);
		if (gerror) {g_error_free (gerror);gerror = NULL;}
		else config->defaultSignalParams.increaseTimerValueWFR = dvalue;
		ivalue = g_key_file_get_uint64 (key_file, "DEFAULT", KEY_STRING (NUMBER_OF_RETRIES_WFR), &gerror);
		if (gerror) {g_error_free (gerror); gerror = NULL;}
		else config->defaultSignalParams.numberOfRetriesWFR = ivalue;

		ivalue = g_key_file_get_uint64 (key_file, "DEFAULT", KEY_STRING (MAX_CONCURRENT_MESSAGES), &gerror);
		if (gerror) {g_error_free (gerror);gerror = NULL;}
		else config->maxConcurrentMessages = ivalue;
	}

	{
		gchar **groups;
		gchar **gp;
		IpcmdConfigSignalId signal_id;
		guint service_id, operation_id;

		groups = g_key_file_get_groups (key_file, NULL);
		for (gp = groups; *gp != NULL; gp++) {
			if (g_str_has_prefix (*gp, "SIGNAL_")) {	// IPCMD signal specific section
				sscanf (*gp, "SIGNAL_0x%x_0x%x", &service_id, &operation_id);
				signal_id.service_id = (guint16)service_id; signal_id.operation_id = (guint16)operation_id;
				_HandleSignalSpecificParams (config, key_file, *gp, &signal_id);
			}
		}
		if (groups) g_strfreev (groups);
	}
	if (key_file) g_key_file_free (key_file);

	return TRUE;

	_IpcmdConfigLoadFromFile_error:
	if (gerror) {
		g_warning ("%s: %s", __func__, gerror->message);
		g_error_free (gerror);
	}
	if (key_file) g_key_file_free (key_file);
	return FALSE;
}

/* @fn: IpcmdConfigInit
 * @brief: initialize IpcmdConfig with default value
 */
static void
IpcmdConfigInit(IpcmdConfig *config)
{
	g_hash_table_remove_all (config->signalSpecificParams);

	/* REQPROD 346925: Default WFA value
	 * The default acknowledgment timeout value shall be named, defaultTimeoutWFA.
	 * This timer shall be set to 500 milliseconds as default value.
	 */
	config->defaultSignalParams.defaultTimeoutWFA = IPCMDCONF_INIT_DEFAULT_TIMEOUT_WFA;
	/* REQPROD 346978: Default WFR value
	 * The default response timeout value shall be named defaultTimeoutWFR.
	 * This timer shall be set to 1 second as default value.
	 */
	config->defaultSignalParams.defaultTimeoutWFR = IPCMDCONF_INIT_DEFAULT_TIMEOUT_WFR;

	/* no specification about increaseTimerValue and numberOfRetries. follow the example in REQPROD 360604. */
	config->defaultSignalParams.increaseTimerValueWFA = IPCMDCONF_INIT_INCREASE_TIMER_VALUE_WFA;
	config->defaultSignalParams.numberOfRetriesWFA = IPCMDCONF_INIT_NUMBER_OF_RETRIES_WFA;
	config->defaultSignalParams.increaseTimerValueWFR = IPCMDCONF_INIT_INCREASE_TIMER_VALUE_WFR;
	config->defaultSignalParams.numberOfRetriesWFR = IPCMDCONF_INIT_NUMBER_OF_RETRIES_WFR;

	/* REQPROD 347045/MAIN;3 : Minimum concurrent message sequences */
	config->maxConcurrentMessages = IPCMDCONF_INIT_MAX_CONCURRENT_MESSAGES;
}

const IpcmdConfigSignalParams *
IpcmdConfigGetSignalParams (IpcmdConfig *config, guint16 service_id, guint16 operation_id)
{
	IpcmdConfigSignalId signal_id = {
			.service_id = service_id,
			.operation_id = operation_id
	};
	IpcmdConfigSignalParams *params = g_hash_table_lookup (config->signalSpecificParams, &signal_id);

	return params ? params : &config->defaultSignalParams;
}

void __attribute__((constructor))
_ConfigInit ()
{
	IpcmdConfig *config = IpcmdConfigGetInstance();
	const gchar *conffile_path = "/etc/libipcmd.conf";

	if (!IpcmdConfigLoadFromFile (config, conffile_path)) {
		g_warning ("Failed to read configuration file: %s", conffile_path);
	}
	_DumpConfiguredSettings ();
}
