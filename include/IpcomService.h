#ifndef __IpcomService_h__
#define __IpcomService_h__

#include <glib.h>
#include <IpcomTypes.h>

typedef	IpcomMessage*(*IpcomCreateMessage)(IpcomService *service, gint opId, gint opType, void *data);

typedef enum {
	IPCOM_SERVICE_SERVER = 1,
	IPCOM_SERVICE_CLIENT = 2
} IpcomServiceMode;
/*
typedef enum {
	IPCOM_SERVICEID_TELEMATICS	= 0xA1,
	IPCOM_SERVICEID_PHONE		= 0xA2,
	IPCOM_SERVICEID_CONNECTIVITY	= 0xA3,
	IPCOM_SERVICEID_COMMON_PHONE_TELEMATICS	= 0xA7,
	IPCOM_SERVICEID_COMMON_ALL	= 0xA8,
	IPCOM_SERVICEID_POSITIONING	= 0xA9,
	IPCOM_SERVICEID_DIAGNOSTIC_MANAGEMENT	=	0xAA
} IpcomServiceId;
*/
struct _IpcomService {
	IpcomServiceMode	mode;
	guint16				serviceId;
	IpcomProtocol	*pProto;
	IpcomTransport		*pTransport;

	IpcomCreateMessage	createMessage;
};

//ServiceServer
struct _IpcomServiceServer {
	struct _IpcomService	_ServiceCommon;
	//processMessage()
	//listen()
	//onNewConnection()
};
typedef struct _IpcomServiceServer IpcomServiceServer;

//ServiceClient
struct _IpcomServiceClient {
	struct _IpcomService	_ServiceCommon;
	const gchar				*pServerAddress;
	guint					nServerPort;
	IpcomConnection			*connection;
	//connect()
	//send()
};
typedef struct _IpcomServiceClient IpcomServiceClient;

typedef gint (*IpcomServiceClientRecvCallback)(void *reply_data, void *userdata);

gboolean IpcomServiceClientConnect(IpcomServiceClient *client);

#endif
