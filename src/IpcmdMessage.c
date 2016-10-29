/*
 * IpcmdMessage.c
 *
 *  Created on: Aug 31, 2016
 *      Author: hyotiger
 */

#include "../include/IpcmdMessage.h"
#include "../include/reference.h"
#include "../include/IpcmdHost.h"
#include <glib.h>
#include <string.h>

static inline void
_IpcmdMessageFree(struct ref *r)
{
	IpcmdMessage *pMsg = container_of(r, IpcmdMessage, _ref);
	r->count = -1;		//mark that this object is already freed.

	//g_debug("Free IpcmdMessage (SHID=0x%.04x, OpType=0x%x)", IpcmdMessageGetVCCPDUSenderHandleID(pMsg), IpcmdMessageGetVCCPDUOpType(pMsg));

	if (pMsg->payload_ptr < (gpointer)pMsg || pMsg->payload_ptr > ((gpointer)pMsg->vccpdu_ptr + pMsg->actual_size)) {
		g_free(pMsg->payload_ptr);
	}

	if (pMsg->origin_addr) g_object_unref(pMsg->origin_addr);
	if (pMsg->origin_host) IpcmdHostUnref(pMsg->origin_host);
	g_free(pMsg);
}

gboolean
IpcmdMessageInit(IpcmdMessage *mesg) {
	mesg->vccpdu_ptr = (struct _VCCPDUHeader *)mesg->message;
	mesg->payload_ptr = (gpointer)mesg->vccpdu_ptr + sizeof(struct _VCCPDUHeader);
	mesg->mesg_length = VCCPDUHEADER_SIZE;
	mesg->origin_addr = NULL;
	mesg->origin_host = NULL;
	ref_init(&mesg->_ref, _IpcmdMessageFree);

	return TRUE;
}

IpcmdMessage *
IpcmdMessageNew(guint16 size)
{
	const gchar *error = NULL;
	struct _IpcmdMessage *pMsg;
	guint16	alloc_size;

	//For compatibility
	if (size == 0) alloc_size = IPCMD_MESSAGE_MAX_SIZE;
	//
	else alloc_size = (size < IPCMD_MESSAGE_MIN_SIZE ? IPCMD_MESSAGE_MIN_SIZE : size);

	g_assert(alloc_size <= IPCMD_MESSAGE_MAX_SIZE);

	pMsg = (IpcmdMessage *) g_malloc0(sizeof(IpcmdMessage) + alloc_size);
	if (pMsg == NULL) {
		error = "Memory is not enough.";
		goto message_alloc_failed;
	}
	pMsg->actual_size = alloc_size;
	IpcmdMessageInit(pMsg);

	return IpcmdMessageRef(pMsg);

message_alloc_failed:
	if (error) g_warning("%s", error);

	return NULL;
}


IpcmdMessage *
IpcmdMessageRef(IpcmdMessage *mesg)
{
	if(get_ref_count(&mesg->_ref) < 0) {
		g_error("ipcom_message has invalid reference count(%d).", get_ref_count(&mesg->_ref));
	}
	ref_inc(&mesg->_ref);

	return mesg;
}

void
IpcmdMessageUnref(IpcmdMessage *mesg)
{
	if(get_ref_count(&mesg->_ref) < 0) {
		g_error("ipcom_message has invalid reference count(%d).", get_ref_count(&mesg->_ref));
	}
	ref_dec(&mesg->_ref);
}

gboolean
IpcmdMessageCopyToPayloadBuffer(IpcmdMessage *mesg, gpointer src, guint32 length) {
	guint max_payload_size = mesg->actual_size-VCCPDUHEADER_SIZE;

	if (length > max_payload_size) //if payload size is bigger than allocated memory
		return FALSE;

	mesg->payload_ptr = (gpointer)mesg->vccpdu_ptr + sizeof(struct _VCCPDUHeader);
	memcpy(mesg->payload_ptr, src, length);
	IpcmdMessageSetPayloadLength(mesg, length);

	return TRUE;
}

void
IpcmdMessageSetOriginSockAddress(IpcmdMessage *mesg, GSocketAddress *paddr)
{
    if (mesg->origin_addr) g_object_unref(paddr);

    mesg->origin_addr = g_object_ref(paddr);
}

gboolean
IpcmdMessageCopyOriginInetAddressString(IpcmdMessage *mesg, gpointer buf, gsize buf_len)
{
	gchar*          addr_str;

	if (!mesg->origin_host) return FALSE;
	if (mesg->origin_host->host_type_ != IPCMD_HOSTLINK_UDPv4)
		return FALSE;

	{
		IpcmdUdpv4Host *udp_host = (IpcmdUdpv4Host*)mesg->origin_host;
		addr_str = g_inet_address_to_string(g_inet_socket_address_get_address(udp_host->inet_sockaddr_));
		g_strlcpy(buf, addr_str, buf_len);
		g_free(addr_str);
	}

	return TRUE;
}

void
IpcmdMessageSetOriginHost(IpcmdMessage *mesg, IpcmdHost *origin)
{
	mesg->origin_host = IpcmdHostRef(origin);
}
