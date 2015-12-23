#include <stdio.h>
#include <glib.h>

#ifdef DEBUG
#define DPRINT(fmt, args...) g_fprintf(stdout, "%s(): " fmt, __func__, ##args)
#define DWARN(fmt, args...) g_fprintf(stdout, "WARNING: %s(): " fmt, __func__, ##args)
#define DFUNCTION_START	DPRINT("__function_start__\n")
#else
#define DPRINT(fmt, args...)
#define DWARN(fmt, args...)
#define DFUNCTION_START
#endif

#define DERROR(fmt, args...) g_fprintf(stdout, "ERROR: %s(): " fmt, __func__, ##args)
