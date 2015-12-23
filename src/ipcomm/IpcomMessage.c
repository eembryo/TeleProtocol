#include <stdio.h>

#include <IpcomMessage.h>
#include <ref_count.h>
#include <dprint.h>

static inline void
_IpcomMessageFree(struct ref *r)
{
	IpcomMessage *pMsg = container_of(r, IpcomMessage, _ref);
	r->count = -1;		//mark that this object is already freed.
	g_free(pMsg);
}

IpcomMessage *
IpcomMessageNew(guint16 size)
{
	const gchar *error;
	IpcomMessage *pMsg;
	guint16	alloc_size = (size ? size : MAX_PACKET_SIZE);

	pMsg = (IpcomMessage *) g_malloc(sizeof(IpcomMessage) + alloc_size);
	if (pMsg == NULL) {
		error = "Memory is not enough.";
		goto message_alloc_failed;
	}
	pMsg->actual_size = alloc_size;
	pMsg->vccpdu_ptr = (struct _VCCPDUHeader *)pMsg->message;
	pMsg->body_ptr = pMsg->message + sizeof(struct _VCCPDUHeader);

	ref_init(&pMsg->_ref, _IpcomMessageFree);

	return IpcomMessageGet(pMsg);

message_alloc_failed:
	DWARN("%s\n", error);

	return NULL;
}


IpcomMessage *
IpcomMessageGet(IpcomMessage *mesg)
{
	if(get_ref_count(&mesg->_ref) < 0) {
		DERROR("ipcom_message has invalid reference count(%d).", get_ref_count(&mesg->_ref));
		g_assert(FALSE);
	}
	ref_inc(&mesg->_ref);

	return mesg;
}

void
IpcomMessagePut(IpcomMessage *mesg)
{
	if(get_ref_count(&mesg->_ref) < 0) {
		DERROR("ipcom_message has invalid reference count(%d).", get_ref_count(&mesg->_ref));
		g_assert(FALSE);
	}
	ref_dec(&mesg->_ref);
}
