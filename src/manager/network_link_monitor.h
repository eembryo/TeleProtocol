#include <glib.h>
#include <gio/gio.h>
#include <string>
#include <xcallservice_logger.h>

#ifdef DEBUG_ENABLED
#include "xcall_debug.h"
#endif

class NetworkLinkEventListener {
	public:
		virtual void notifyDelLinkEvent() = 0;

	private:
};

class NetworkLinkMonitor {
	public:
		NetworkLinkMonitor();
		virtual ~NetworkLinkMonitor();

		bool setListener(const char *ifc_name, NetworkLinkEventListener *listener);
		GSocket *getGSocket() {return mSocket;}

	protected:
		static gboolean readLinkEvent(GSocket *socket, GIOCondition condition, gpointer user_data);
		bool init();

	private:
		std::string	mNetifcName;
		GMainContext	*mMainContext;
		GSource	*mSocketReadSource;
		GSocket	*mSocket;
		NetworkLinkEventListener *mListener;
		gint mSockfd;
};
