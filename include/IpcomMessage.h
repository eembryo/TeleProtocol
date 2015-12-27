#ifndef __IpcomMessage_h__
#define __IpcomMessage_h__

#include <glib.h>
#include <ref_count.h>
#include <IpcomTypes.h>

#define MAX_PACKET_SIZE 1472	//MTU - IP_HEADER_SIZE(20) - UDP_HEADER_SIZE(8)

#define BUILD_SENDERHANDLEID(service, operation, optype, seqNR)	(guint32)( (((seqNR%0xFF)<<8 + (optype%0xFF))<<8 + (operation%0xFF))<<8 + service%0xFF)

struct _VCCPDUHeader {
	guint16		serviceID;
	guint16		operationID;
	guint32		length;
	guint32 	senderHandleId;
	guint8		proto_version;
	guint8		opType;
	guint8		dataType;
	guint8		proc:1;
	guint8		reserved:7;
} __attribute__ ((packed));

struct _IpcomMessage {
	struct ref		_ref;
	struct _VCCPDUHeader	*vccpdu_ptr;
	const gchar		*body_ptr;
	guint16			length;			//the size of this message. It does not include the size of 'struct _IpcomMessage'
	guint16			actual_size;	//the size of allocated memory for this message. It does not include the size of 'struct _IpcomMessage'
	char			message[0];		//the start of raw message.
};

// if size equals to zero, maximum message size is allocated.
IpcomMessage *IpcomMessageNew(guint16 maxSize);

static inline VCCPDUHeader *IpcomMessageGetVCCPDUHeader(IpcomMessage *mesg) {
	return mesg->vccpdu_ptr;
};

static inline gboolean IpcomMessageInit(IpcomMessage *mesg, guint16 length) {
	if (mesg->actual_size < length) //too small to embed a packet with 'length' size.
		return FALSE;
	mesg->vccpdu_ptr = mesg->message;
	mesg->body_ptr = mesg->vccpdu_ptr + sizeof(struct _VCCPDUHeader);
	mesg->length = length;
	return TRUE;
};
IpcomMessage *IpcomMessageRef(IpcomMessage *message);
void IpcomMessageUnref(IpcomMessage *message);

#endif
