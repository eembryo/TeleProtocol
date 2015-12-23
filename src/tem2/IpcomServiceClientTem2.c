#include <glib.h>

#include <IpcomService.h>
#include <IpcomMessage.h>
#include <IpcomProtocol.h>
#include <IpcomTransport.h>
#include <IpcomTransportUDPv4.h>

#define TEM2SERVER_IP_ADDRESS	"127.0.0.1"
#define TEM2SERVER_PORT_NUMBER	50000
#define TEM2CLIENT_IP_ADDRESS	"127.0.0.1"
#define TEM2CLIENT_PORT_NUMBER	50001

#define SERVICEID_TELEMATICS	0xA1

typedef enum {
	TEM2_OPID_TELEMATIC_SETTINGS 	= 0x0104,
	TEM2_OPID_POSITION_DATA			= 0x0105,
	TEM2_OPID_SUBSCRIPTION_ACTIVATION	= 0x0107,
	TEM2_OPID_SENDTOCAR_CONFIRMATION	= 0x010E,
	TEM2_OPID_SVTSTATUS				= 0x0102,
} Tem2OpId;


static IpcomMessage *
Tem2ServiceCreateMessage(IpcomService *service, gint opId, gint opType, void *data)
{
	IpcomMessage *new = IpcomMessageNew(0);
	g_assert(new);
	VCCPDUHeader *pHeader = new->vccpdu_ptr;

	/*
	 * struct _VCCPDUHeader {
	 * 		 guint16		serviceID;
	 *		 guint16		operationID;
	 *		 guint32		length;
	 *		 guint32		serviceHandleID;
	 *		 guint8		proto_version;
	 *		 guint8		opType;
	 *		 guint8		dataType;
	 *		 guint8		proc:1;
	 *		 guint8		reserved:7;}
	 */
	pHeader->serviceID = g_htons(service->serviceId);
	pHeader->operationID = g_htons(TEM2_OPID_TELEMATIC_SETTINGS);
	pHeader->length = g_htonl(8 + 0);	//senderhandleid + protocol_version + optype + datatype + reserved + payload
	pHeader->proto_version = 0x03;
	pHeader->opType = IPPROTO_OPTYPE_REQUEST;
	pHeader->senderHandleId = BUILD_SENDERHANDLEID(pHeader->serviceID, pHeader->operationID, pHeader->opType, 0);

	return new;
}

void Tem2ServiceInit(IpcomServiceClient *client)
{
	gboolean value;

	client->_ServiceCommon.mode = IPCOM_SERVICE_CLIENT;
	client->_ServiceCommon.serviceId = SERVICEID_TELEMATICS;
	client->_ServiceCommon.pProto = IpcomProtocolGetInstance();
	g_assert(client->_ServiceCommon.pProto);
	client->_ServiceCommon.createMessage = Tem2ServiceCreateMessage;
	client->_ServiceCommon.pTransport = IpcomTransportUDPv4New();
	g_assert(client->_ServiceCommon.pTransport);
	//bind UDP socket
	value = client->_ServiceCommon.pTransport->bind(
			client->_ServiceCommon.pTransport, TEM2CLIENT_IP_ADDRESS, TEM2CLIENT_PORT_NUMBER);
	g_assert(value);
	client->pServerAddress = TEM2SERVER_IP_ADDRESS;
	client->nServerPort = TEM2SERVER_PORT_NUMBER;
	//IpcomTransportUDPv4AttachGlibContext(UDPV4TRANSPORT_FROM_TRANSPORT(client->_ServiceCommon.pTransport, context);
}

IpcomServiceClient *
Tem2ClientCreate()
{
	IpcomServiceClient *tem2client;

	tem2client = g_malloc0(sizeof(IpcomServiceClient));
	g_assert(tem2client);

	Tem2ServiceInit(tem2client);
	return tem2client;
}

gboolean
Tem2ClientSendDummyMessage(IpcomServiceClient *client)
{
	if (!client->connection) { //create new connection
		IpcomConnection *conn;
		conn = client->_ServiceCommon.pTransport->connect(
				client->_ServiceCommon.pTransport, client->pServerAddress, client->nServerPort);
		if (conn == NULL) return FALSE;
		client->connection = conn;
	}
	IpcomMessage *mesg = Tem2ServiceCreateMessage(&client->_ServiceCommon, TEM2_OPID_TELEMATIC_SETTINGS, IPPROTO_OPTYPE_REQUEST, NULL);
	IpcomProtocolSendMessage(client->_ServiceCommon.pProto, client->connection, mesg, NULL, NULL);

	return TRUE;
}
gint main()
{
	GMainContext *context;
	GMainLoop	*main_loop;

	IpcomServiceClient *client = Tem2ClientCreate();
	Tem2ClientSendDummyMessage(client);
	/*
	context = g_main_context_new();
	main_loop = g_main_loop_new(context, FALSE);
	g_main_context_unref(context);

	g_main_loop_run (main_loop);
*/
	return 0;
}
