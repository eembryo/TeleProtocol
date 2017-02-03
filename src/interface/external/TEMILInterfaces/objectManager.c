/*
 * Generated by gdbus-codegen 2.32.4. DO NOT EDIT.
 *
 * The license of this code is the same as for the source it was derived from.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "objectManager.h"

#include <string.h>
#ifdef G_OS_UNIX
#  include <gio/gunixfdlist.h>
#endif

typedef struct
{
  GDBusArgInfo parent_struct;
  gboolean use_gvariant;
} _ExtendedGDBusArgInfo;

typedef struct
{
  GDBusMethodInfo parent_struct;
  const gchar *signal_name;
  gboolean pass_fdlist;
} _ExtendedGDBusMethodInfo;

typedef struct
{
  GDBusSignalInfo parent_struct;
  const gchar *signal_name;
} _ExtendedGDBusSignalInfo;

typedef struct
{
  GDBusPropertyInfo parent_struct;
  const gchar *hyphen_name;
  gboolean use_gvariant;
} _ExtendedGDBusPropertyInfo;

typedef struct
{
  GDBusInterfaceInfo parent_struct;
  const gchar *hyphen_name;
} _ExtendedGDBusInterfaceInfo;

typedef struct
{
  const _ExtendedGDBusPropertyInfo *info;
  guint prop_id;
  GValue orig_value; /* the value before the change */
} ChangedProperty;

static void
_changed_property_free (ChangedProperty *data)
{
  g_value_unset (&data->orig_value);
  g_free (data);
}

static gboolean
_g_strv_equal0 (gchar **a, gchar **b)
{
  gboolean ret = FALSE;
  guint n;
  if (a == NULL && b == NULL)
    {
      ret = TRUE;
      goto out;
    }
  if (a == NULL || b == NULL)
    goto out;
  if (g_strv_length (a) != g_strv_length (b))
    goto out;
  for (n = 0; a[n] != NULL; n++)
    if (g_strcmp0 (a[n], b[n]) != 0)
      goto out;
  ret = TRUE;
out:
  return ret;
}

static gboolean
_g_variant_equal0 (GVariant *a, GVariant *b)
{
  gboolean ret = FALSE;
  if (a == NULL && b == NULL)
    {
      ret = TRUE;
      goto out;
    }
  if (a == NULL || b == NULL)
    goto out;
  ret = g_variant_equal (a, b);
out:
  return ret;
}

G_GNUC_UNUSED static gboolean
_g_value_equal (const GValue *a, const GValue *b)
{
  gboolean ret = FALSE;
  g_assert (G_VALUE_TYPE (a) == G_VALUE_TYPE (b));
  switch (G_VALUE_TYPE (a))
    {
      case G_TYPE_BOOLEAN:
        ret = (g_value_get_boolean (a) == g_value_get_boolean (b));
        break;
      case G_TYPE_UCHAR:
        ret = (g_value_get_uchar (a) == g_value_get_uchar (b));
        break;
      case G_TYPE_INT:
        ret = (g_value_get_int (a) == g_value_get_int (b));
        break;
      case G_TYPE_UINT:
        ret = (g_value_get_uint (a) == g_value_get_uint (b));
        break;
      case G_TYPE_INT64:
        ret = (g_value_get_int64 (a) == g_value_get_int64 (b));
        break;
      case G_TYPE_UINT64:
        ret = (g_value_get_uint64 (a) == g_value_get_uint64 (b));
        break;
      case G_TYPE_DOUBLE:
        {
          /* Avoid -Wfloat-equal warnings by doing a direct bit compare */
          gdouble da = g_value_get_double (a);
          gdouble db = g_value_get_double (b);
          ret = memcmp (&da, &db, sizeof (gdouble)) == 0;
        }
        break;
      case G_TYPE_STRING:
        ret = (g_strcmp0 (g_value_get_string (a), g_value_get_string (b)) == 0);
        break;
      case G_TYPE_VARIANT:
        ret = _g_variant_equal0 (g_value_get_variant (a), g_value_get_variant (b));
        break;
      default:
        if (G_VALUE_TYPE (a) == G_TYPE_STRV)
          ret = _g_strv_equal0 (g_value_get_boxed (a), g_value_get_boxed (b));
        else
          g_critical ("_g_value_equal() does not handle type %s", g_type_name (G_VALUE_TYPE (a)));
        break;
    }
  return ret;
}

/* ------------------------------------------------------------------------
 * Code for Object, ObjectProxy and ObjectSkeleton
 * ------------------------------------------------------------------------
 */

/**
 * SECTION:Object
 * @title: Object
 * @short_description: Specialized GDBusObject types
 *
 * This section contains the #Object, #ObjectProxy, and #ObjectSkeleton types which make it easier to work with objects implementing generated types for D-Bus interfaces.
 */

/**
 * Object:
 *
 * The #Object type is a specialized container of interfaces.
 */

/**
 * ObjectIface:
 * @parent_iface: The parent interface.
 *
 * Virtual table for the #Object interface.
 */

static void
object_default_init (ObjectIface *iface)
{
}

typedef ObjectIface ObjectInterface;
G_DEFINE_INTERFACE_WITH_CODE (Object, object, G_TYPE_OBJECT, g_type_interface_add_prerequisite (g_define_type_id, G_TYPE_DBUS_OBJECT));



static void
object_notify (GDBusObject *object, GDBusInterface *interface)
{
  g_object_notify (G_OBJECT (object), ((_ExtendedGDBusInterfaceInfo *) g_dbus_interface_get_info (interface))->hyphen_name);
}

/**
 * ObjectProxy:
 *
 * The #ObjectProxy structure contains only private data and should only be accessed using the provided API.
 */

/**
 * ObjectProxyClass:
 * @parent_class: The parent class.
 *
 * Class structure for #ObjectProxy.
 */

static void
object_proxy__object_iface_init (ObjectIface *iface)
{
}

static void
object_proxy__g_dbus_object_iface_init (GDBusObjectIface *iface)
{
  iface->interface_added = object_notify;
  iface->interface_removed = object_notify;
}


G_DEFINE_TYPE_WITH_CODE (ObjectProxy, object_proxy, G_TYPE_DBUS_OBJECT_PROXY,
                         G_IMPLEMENT_INTERFACE (TYPE_OBJECT, object_proxy__object_iface_init)
                         G_IMPLEMENT_INTERFACE (G_TYPE_DBUS_OBJECT, object_proxy__g_dbus_object_iface_init));

static void
object_proxy_init (ObjectProxy *object)
{
}

static void
object_proxy_set_property (GObject      *gobject,
  guint         prop_id,
  const GValue *value,
  GParamSpec   *pspec)
{
  G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
}

static void
object_proxy_get_property (GObject      *gobject,
  guint         prop_id,
  GValue       *value,
  GParamSpec   *pspec)
{
  ObjectProxy *object = OBJECT_PROXY (gobject);
  GDBusInterface *interface;

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
  }
}

static void
object_proxy_class_init (ObjectProxyClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = object_proxy_set_property;
  gobject_class->get_property = object_proxy_get_property;

}

/**
 * object_proxy_new:
 * @connection: A #GDBusConnection.
 * @object_path: An object path.
 *
 * Creates a new proxy object.
 *
 * Returns: (transfer full): The proxy object.
 */
ObjectProxy *
object_proxy_new (GDBusConnection *connection,
  const gchar *object_path)
{
  g_return_val_if_fail (G_IS_DBUS_CONNECTION (connection), NULL);
  g_return_val_if_fail (g_variant_is_object_path (object_path), NULL);
  return OBJECT_PROXY (g_object_new (TYPE_OBJECT_PROXY, "g-connection", connection, "g-object-path", object_path, NULL));
}

/**
 * ObjectSkeleton:
 *
 * The #ObjectSkeleton structure contains only private data and should only be accessed using the provided API.
 */

/**
 * ObjectSkeletonClass:
 * @parent_class: The parent class.
 *
 * Class structure for #ObjectSkeleton.
 */

static void
object_skeleton__object_iface_init (ObjectIface *iface)
{
}


static void
object_skeleton__g_dbus_object_iface_init (GDBusObjectIface *iface)
{
  iface->interface_added = object_notify;
  iface->interface_removed = object_notify;
}

G_DEFINE_TYPE_WITH_CODE (ObjectSkeleton, object_skeleton, G_TYPE_DBUS_OBJECT_SKELETON,
                         G_IMPLEMENT_INTERFACE (TYPE_OBJECT, object_skeleton__object_iface_init)
                         G_IMPLEMENT_INTERFACE (G_TYPE_DBUS_OBJECT, object_skeleton__g_dbus_object_iface_init));

static void
object_skeleton_init (ObjectSkeleton *object)
{
}

static void
object_skeleton_set_property (GObject      *gobject,
  guint         prop_id,
  const GValue *value,
  GParamSpec   *pspec)
{
  ObjectSkeleton *object = OBJECT_SKELETON (gobject);
  GDBusInterfaceSkeleton *interface;

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
  }
}

static void
object_skeleton_get_property (GObject      *gobject,
  guint         prop_id,
  GValue       *value,
  GParamSpec   *pspec)
{
  ObjectSkeleton *object = OBJECT_SKELETON (gobject);
  GDBusInterface *interface;

  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
      break;
  }
}

static void
object_skeleton_class_init (ObjectSkeletonClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = object_skeleton_set_property;
  gobject_class->get_property = object_skeleton_get_property;

}

/**
 * object_skeleton_new:
 * @object_path: An object path.
 *
 * Creates a new skeleton object.
 *
 * Returns: (transfer full): The skeleton object.
 */
ObjectSkeleton *
object_skeleton_new (const gchar *object_path)
{
  g_return_val_if_fail (g_variant_is_object_path (object_path), NULL);
  return OBJECT_SKELETON (g_object_new (TYPE_OBJECT_SKELETON, "g-object-path", object_path, NULL));
}


/* ------------------------------------------------------------------------
 * Code for ObjectManager client
 * ------------------------------------------------------------------------
 */

/**
 * SECTION:ObjectManagerClient
 * @title: ObjectManagerClient
 * @short_description: Generated GDBusObjectManagerClient type
 *
 * This section contains a #GDBusObjectManagerClient that uses object_manager_client_get_proxy_type() as the #GDBusProxyTypeFunc.
 */

/**
 * ObjectManagerClient:
 *
 * The #ObjectManagerClient structure contains only private data and should only be accessed using the provided API.
 */

/**
 * ObjectManagerClientClass:
 * @parent_class: The parent class.
 *
 * Class structure for #ObjectManagerClient.
 */

G_DEFINE_TYPE (ObjectManagerClient, object_manager_client, G_TYPE_DBUS_OBJECT_MANAGER_CLIENT);

static void
object_manager_client_init (ObjectManagerClient *manager)
{
}

static void
object_manager_client_class_init (ObjectManagerClientClass *klass)
{
}

/**
 * object_manager_client_get_proxy_type:
 * @manager: A #GDBusObjectManagerClient.
 * @object_path: The object path of the remote object (unused).
 * @interface_name: (allow-none): Interface name of the remote object or %NULL to get the object proxy #GType.
 * @user_data: User data (unused).
 *
 * A #GDBusProxyTypeFunc that maps @interface_name to the generated #GDBusObjectProxy<!-- -->- and #GDBusProxy<!-- -->-derived types.
 *
 * Returns: A #GDBusProxy<!-- -->-derived #GType if @interface_name is not %NULL, otherwise the #GType for #ObjectProxy.
 */
GType
object_manager_client_get_proxy_type (GDBusObjectManagerClient *manager, const gchar *object_path, const gchar *interface_name, gpointer user_data)
{
  static gsize once_init_value = 0;
  static GHashTable *lookup_hash;
  GType ret;

  if (interface_name == NULL)
    return TYPE_OBJECT_PROXY;
  if (g_once_init_enter (&once_init_value))
    {
      lookup_hash = g_hash_table_new (g_str_hash, g_str_equal);
      g_once_init_leave (&once_init_value, 1);
    }
  ret = (GType) GPOINTER_TO_SIZE (g_hash_table_lookup (lookup_hash, interface_name));
  if (ret == (GType) 0)
    ret = G_TYPE_DBUS_PROXY;
  return ret;
}

/**
 * object_manager_client_new:
 * @connection: A #GDBusConnection.
 * @flags: Flags from the #GDBusObjectManagerClientFlags enumeration.
 * @name: (allow-none): A bus name (well-known or unique) or %NULL if @connection is not a message bus connection.
 * @object_path: An object path.
 * @cancellable: (allow-none): A #GCancellable or %NULL.
 * @callback: A #GAsyncReadyCallback to call when the request is satisfied.
 * @user_data: User data to pass to @callback.
 *
 * Asynchronously creates #GDBusObjectManagerClient using object_manager_client_get_proxy_type() as the #GDBusProxyTypeFunc. See g_dbus_object_manager_client_new() for more details.
 *
 * When the operation is finished, @callback will be invoked in the <link linkend="g-main-context-push-thread-default">thread-default main loop</link> of the thread you are calling this method from.
 * You can then call object_manager_client_new_finish() to get the result of the operation.
 *
 * See object_manager_client_new_sync() for the synchronous, blocking version of this constructor.
 */
void
object_manager_client_new (
    GDBusConnection        *connection,
    GDBusObjectManagerClientFlags  flags,
    const gchar            *name,
    const gchar            *object_path,
    GCancellable           *cancellable,
    GAsyncReadyCallback     callback,
    gpointer                user_data)
{
  g_async_initable_new_async (TYPE_OBJECT_MANAGER_CLIENT, G_PRIORITY_DEFAULT, cancellable, callback, user_data, "flags", flags, "name", name, "connection", connection, "object-path", object_path, "get-proxy-type-func", object_manager_client_get_proxy_type, NULL);
}

/**
 * object_manager_client_new_finish:
 * @res: The #GAsyncResult obtained from the #GAsyncReadyCallback passed to object_manager_client_new().
 * @error: Return location for error or %NULL
 *
 * Finishes an operation started with object_manager_client_new().
 *
 * Returns: (transfer full) (type ObjectManagerClient): The constructed object manager client or %NULL if @error is set.
 */
GDBusObjectManager *
object_manager_client_new_finish (
    GAsyncResult        *res,
    GError             **error)
{
  GObject *ret;
  GObject *source_object;
  source_object = g_async_result_get_source_object (res);
  ret = g_async_initable_new_finish (G_ASYNC_INITABLE (source_object), res, error);
  g_object_unref (source_object);
  if (ret != NULL)
    return G_DBUS_OBJECT_MANAGER (ret);
  else
    return NULL;
}

/**
 * object_manager_client_new_sync:
 * @connection: A #GDBusConnection.
 * @flags: Flags from the #GDBusObjectManagerClientFlags enumeration.
 * @name: (allow-none): A bus name (well-known or unique) or %NULL if @connection is not a message bus connection.
 * @object_path: An object path.
 * @cancellable: (allow-none): A #GCancellable or %NULL.
 * @error: Return location for error or %NULL
 *
 * Synchronously creates #GDBusObjectManagerClient using object_manager_client_get_proxy_type() as the #GDBusProxyTypeFunc. See g_dbus_object_manager_client_new_sync() for more details.
 *
 * The calling thread is blocked until a reply is received.
 *
 * See object_manager_client_new() for the asynchronous version of this constructor.
 *
 * Returns: (transfer full) (type ObjectManagerClient): The constructed object manager client or %NULL if @error is set.
 */
GDBusObjectManager *
object_manager_client_new_sync (
    GDBusConnection        *connection,
    GDBusObjectManagerClientFlags  flags,
    const gchar            *name,
    const gchar            *object_path,
    GCancellable           *cancellable,
    GError                **error)
{
  GInitable *ret;
  ret = g_initable_new (TYPE_OBJECT_MANAGER_CLIENT, cancellable, error, "flags", flags, "name", name, "connection", connection, "object-path", object_path, "get-proxy-type-func", object_manager_client_get_proxy_type, NULL);
  if (ret != NULL)
    return G_DBUS_OBJECT_MANAGER (ret);
  else
    return NULL;
}


/**
 * object_manager_client_new_for_bus:
 * @bus_type: A #GBusType.
 * @flags: Flags from the #GDBusObjectManagerClientFlags enumeration.
 * @name: A bus name (well-known or unique).
 * @object_path: An object path.
 * @cancellable: (allow-none): A #GCancellable or %NULL.
 * @callback: A #GAsyncReadyCallback to call when the request is satisfied.
 * @user_data: User data to pass to @callback.
 *
 * Like object_manager_client_new() but takes a #GBusType instead of a #GDBusConnection.
 *
 * When the operation is finished, @callback will be invoked in the <link linkend="g-main-context-push-thread-default">thread-default main loop</link> of the thread you are calling this method from.
 * You can then call object_manager_client_new_for_bus_finish() to get the result of the operation.
 *
 * See object_manager_client_new_for_bus_sync() for the synchronous, blocking version of this constructor.
 */
void
object_manager_client_new_for_bus (
    GBusType                bus_type,
    GDBusObjectManagerClientFlags  flags,
    const gchar            *name,
    const gchar            *object_path,
    GCancellable           *cancellable,
    GAsyncReadyCallback     callback,
    gpointer                user_data)
{
  g_async_initable_new_async (TYPE_OBJECT_MANAGER_CLIENT, G_PRIORITY_DEFAULT, cancellable, callback, user_data, "flags", flags, "name", name, "bus-type", bus_type, "object-path", object_path, "get-proxy-type-func", object_manager_client_get_proxy_type, NULL);
}

/**
 * object_manager_client_new_for_bus_finish:
 * @res: The #GAsyncResult obtained from the #GAsyncReadyCallback passed to object_manager_client_new_for_bus().
 * @error: Return location for error or %NULL
 *
 * Finishes an operation started with object_manager_client_new_for_bus().
 *
 * Returns: (transfer full) (type ObjectManagerClient): The constructed object manager client or %NULL if @error is set.
 */
GDBusObjectManager *
object_manager_client_new_for_bus_finish (
    GAsyncResult        *res,
    GError             **error)
{
  GObject *ret;
  GObject *source_object;
  source_object = g_async_result_get_source_object (res);
  ret = g_async_initable_new_finish (G_ASYNC_INITABLE (source_object), res, error);
  g_object_unref (source_object);
  if (ret != NULL)
    return G_DBUS_OBJECT_MANAGER (ret);
  else
    return NULL;
}

/**
 * object_manager_client_new_for_bus_sync:
 * @bus_type: A #GBusType.
 * @flags: Flags from the #GDBusObjectManagerClientFlags enumeration.
 * @name: A bus name (well-known or unique).
 * @object_path: An object path.
 * @cancellable: (allow-none): A #GCancellable or %NULL.
 * @error: Return location for error or %NULL
 *
 * Like object_manager_client_new_sync() but takes a #GBusType instead of a #GDBusConnection.
 *
 * The calling thread is blocked until a reply is received.
 *
 * See object_manager_client_new_for_bus() for the asynchronous version of this constructor.
 *
 * Returns: (transfer full) (type ObjectManagerClient): The constructed object manager client or %NULL if @error is set.
 */
GDBusObjectManager *
object_manager_client_new_for_bus_sync (
    GBusType                bus_type,
    GDBusObjectManagerClientFlags  flags,
    const gchar            *name,
    const gchar            *object_path,
    GCancellable           *cancellable,
    GError                **error)
{
  GInitable *ret;
  ret = g_initable_new (TYPE_OBJECT_MANAGER_CLIENT, cancellable, error, "flags", flags, "name", name, "bus-type", bus_type, "object-path", object_path, "get-proxy-type-func", object_manager_client_get_proxy_type, NULL);
  if (ret != NULL)
    return G_DBUS_OBJECT_MANAGER (ret);
  else
    return NULL;
}


