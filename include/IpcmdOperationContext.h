/*
 * IpcmdOperationContext.h
 *
 *  Created on: Aug 31, 2016
 *      Author: hyotiger
 */

#ifndef INCLUDE_IPCMDOPERATIONCONTEXT_H_
#define INCLUDE_IPCMDOPERATIONCONTEXT_H_

#include <glib.h>

G_BEGIN_DECLS

typedef struct _IpcmdOperationContext IpcmdOperationContext;
typedef struct _IpcmdOperationContextId IpcmdOperationContextId;

inline gboolean
IpcmdOperationContextIdIsMatch(const IpcmdOperationContextId *a, const IpcmdOperationContextId *b)
{
	return a->channel_id_ == b->channel_id_ && a->sender_handle_id_ == b->sender_handle_id_ ? TRUE : FALSE;
}

G_END_DECLS

#endif /* INCLUDE_IPCMDOPERATIONCONTEXT_H_ */
