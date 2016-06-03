// @@@LICENSE
//
// Copyright (C) 2015, LG Electronics, All Right Reserved.
//
// No part of this source code may be communicated, distributed, reproduced
// or transmitted in any form or by any means, electronic or mechanical or
// otherwise, for any purpose, without the prior written permission of
// LG Electronics.
//
//
// design/author : hyobeom1.lee@lge.com
// date   : 12/30/2015
// Desc   :
//
// LICENSE@@@

#include <stdio.h>

#include <IpcomMessage.h>
#include <ref_count.h>
#include <dprint.h>
#include <string.h>

static inline void
_IpcomMessageFree(struct ref *r)
{
	IpcomMessage *pMsg = container_of(r, IpcomMessage, _ref);
	r->count = -1;		//mark that this object is already freed.

	DPRINT("Free IpcomMessage (SenderHandleID: 0x%x, OpType: 0x%x)\n", IpcomMessageGetVCCPDUSenderHandleID(pMsg), IpcomMessageGetVCCPDUOpType(pMsg));

	if (pMsg->payload_ptr < (gpointer)pMsg || pMsg->payload_ptr > ((gpointer)pMsg->vccpdu_ptr + pMsg->actual_size)) {
		g_free(pMsg->payload_ptr);
	}

	if (pMsg->origin_addr) g_object_unref(pMsg->origin_addr);

	g_free(pMsg);
}

gboolean
IpcomMessageInit(IpcomMessage *mesg) {
	mesg->vccpdu_ptr = (struct _VCCPDUHeader *)mesg->message;
	mesg->payload_ptr = (gpointer)mesg->vccpdu_ptr + sizeof(struct _VCCPDUHeader);
	mesg->mesg_length = VCCPDUHEADER_SIZE;
	mesg->origin_addr = NULL;
	ref_init(&mesg->_ref, _IpcomMessageFree);

	return TRUE;
}

IpcomMessage *
IpcomMessageNew(guint16 size)
{
	const gchar *error = NULL;
	struct _IpcomMessage *pMsg;
	guint16	alloc_size;

	//For compatibility
	if (size == 0) alloc_size = IPCOM_MESSAGE_MAX_SIZE;
	//
	else alloc_size = (size < IPCOM_MESSAGE_MIN_SIZE ? IPCOM_MESSAGE_MIN_SIZE : size);

	g_assert(alloc_size <= IPCOM_MESSAGE_MAX_SIZE);

	pMsg = (IpcomMessage *) g_malloc0(sizeof(IpcomMessage) + alloc_size);
	if (pMsg == NULL) {
		error = "Memory is not enough.";
		goto message_alloc_failed;
	}
	pMsg->actual_size = alloc_size;
	IpcomMessageInit(pMsg);

	return IpcomMessageRef(pMsg);

message_alloc_failed:
	if (error) DWARN("%s\n", error);

	return NULL;
}


IpcomMessage *
IpcomMessageRef(IpcomMessage *mesg)
{
	if(get_ref_count(&mesg->_ref) < 0) {
		DERROR("ipcom_message has invalid reference count(%d).", get_ref_count(&mesg->_ref));
		g_assert(FALSE);
	}
	ref_inc(&mesg->_ref);

	return mesg;
}

void
IpcomMessageUnref(IpcomMessage *mesg)
{
	if(get_ref_count(&mesg->_ref) < 0) {
		DERROR("ipcom_message has invalid reference count(%d).", get_ref_count(&mesg->_ref));
		g_assert(FALSE);
	}
	ref_dec(&mesg->_ref);
}
gboolean
IpcomMessageCopyToPayloadBuffer(IpcomMessage *mesg, gpointer src, guint32 length) {
	guint MaxPayloadSize = mesg->actual_size-VCCPDUHEADER_SIZE;

	if (length > MaxPayloadSize) return FALSE;	//payload size is bigger than allocated memory

	mesg->payload_ptr = (gpointer)mesg->vccpdu_ptr + sizeof(struct _VCCPDUHeader);
	memcpy(mesg->payload_ptr, src, length);
	IpcomMessageSetPayloadLength(mesg, length);

	return TRUE;
}

void
IpcomMessageSetOriginSockAddress(IpcomMessage *mesg, GSocketAddress *paddr)
{
    if (mesg->origin_addr) g_object_unref(paddr);

    mesg->origin_addr = g_object_ref(paddr);
}
/*
gchar*
IpcomMessageGetOriginInetAddressString(IpcomMessage *mesg) {
    GInetAddress *addr;

    if (!mesg->origin_addr) return NULL;

    addr = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(mesg->origin_addr));

    return g_inet_address_to_string(addr);
}
*/
gboolean
IpcomMessageCopyOriginInetAddressString(IpcomMessage *mesg, gpointer buf, gsize buf_len)
{
    gchar*          addr_str;

    if (!mesg->origin_addr) return FALSE;

    addr_str = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(mesg->origin_addr)));
    g_strlcpy(buf, addr_str, buf_len);
    g_free(addr_str);

    return TRUE;
}
guint16
IpcomMessageGetOriginInetPort(IpcomMessage *mesg) {
    if (!mesg->origin_addr) return 0;

    return g_inet_socket_address_get_port(G_INET_SOCKET_ADDRESS(mesg->origin_addr));
}

