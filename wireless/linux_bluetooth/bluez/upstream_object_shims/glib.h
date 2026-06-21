#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_GLIB_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_GLIB_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef int gboolean;
typedef unsigned int guint;
typedef uint16_t guint16;
typedef uint32_t guint32;
typedef uint64_t guint64;
typedef int gint;
typedef size_t gsize;
typedef ssize_t gssize;
typedef void *gpointer;
typedef const void *gconstpointer;
typedef char gchar;
typedef void (*GDestroyNotify)(gpointer data);
typedef void (*GFunc)(gpointer data, gpointer user_data);
typedef gboolean (*GSourceFunc)(gpointer data);

typedef struct _GError
{
  gint code;
  gchar *message;
} GError;

typedef struct _GSList
{
  gpointer data;
  struct _GSList *next;
} GSList;

typedef struct _GList
{
  gpointer data;
  struct _GList *next;
  struct _GList *prev;
} GList;

typedef struct _GIOChannel
{
  int fd;
} GIOChannel;

typedef struct _GKeyFile
{
  int dummy;
} GKeyFile;

typedef struct _GHashTable
{
  int dummy;
} GHashTable;

typedef struct _GTimer
{
  int dummy;
} GTimer;

typedef guint (*GHashFunc)(gconstpointer key);
typedef gboolean (*GEqualFunc)(gconstpointer a, gconstpointer b);

#ifndef TRUE
#  define TRUE 1
#endif
#ifndef FALSE
#  define FALSE 0
#endif

#define G_IO_IN  0x01
#define G_IO_OUT 0x04
#define G_IO_ERR 0x08
#define G_IO_HUP 0x10
#define G_IO_NVAL 0x20

typedef unsigned int GIOCondition;
typedef gboolean (*GIOFunc)(GIOChannel *source, GIOCondition condition,
                            gpointer data);

static inline gpointer g_malloc0(size_t size)
{
  return calloc(1, size);
}

static inline gpointer g_malloc(size_t size)
{
  return malloc(size);
}

static inline void g_free(gpointer data)
{
  free(data);
}

static inline void g_error_free(GError *error)
{
  if (error != NULL)
    {
      free(error->message);
      free(error);
    }
}

static inline gpointer g_memdup2(gconstpointer mem, gsize byte_size)
{
  void *copy;

  if (mem == NULL || byte_size == 0)
    {
      return NULL;
    }

  copy = malloc(byte_size);
  if (copy == NULL)
    {
      return NULL;
    }

  memcpy(copy, mem, byte_size);
  return copy;
}

static inline GKeyFile *g_key_file_new(void)
{
  return calloc(1, sizeof(GKeyFile));
}

static inline void g_key_file_free(GKeyFile *key_file)
{
  g_free(key_file);
}

static inline gboolean g_key_file_load_from_file(GKeyFile *key_file,
                                                 const gchar *file,
                                                 guint flags,
                                                 GError **error)
{
  (void)key_file;
  (void)file;
  (void)flags;
  if (error != NULL)
    {
      *error = NULL;
    }

  return FALSE;
}

static inline gchar **g_key_file_get_groups(GKeyFile *key_file,
                                            gsize *length)
{
  (void)key_file;
  if (length != NULL)
    {
      *length = 0;
    }

  return NULL;
}

static inline gchar *g_key_file_get_string(GKeyFile *key_file,
                                           const gchar *group_name,
                                           const gchar *key,
                                           GError **error)
{
  (void)key_file;
  (void)group_name;
  (void)key;
  if (error != NULL)
    {
      *error = NULL;
    }

  return NULL;
}

static inline gboolean g_key_file_get_boolean(GKeyFile *key_file,
                                              const gchar *group_name,
                                              const gchar *key,
                                              GError **error)
{
  (void)key_file;
  (void)group_name;
  (void)key;
  if (error != NULL)
    {
      *error = NULL;
    }

  return FALSE;
}

static inline gboolean g_key_file_has_group(GKeyFile *key_file,
                                            const gchar *group_name)
{
  (void)key_file;
  (void)group_name;
  return FALSE;
}

static inline gint g_key_file_get_integer(GKeyFile *key_file,
                                          const gchar *group_name,
                                          const gchar *key,
                                          GError **error)
{
  (void)key_file;
  (void)group_name;
  (void)key;
  if (error != NULL)
    {
      *error = NULL;
    }

  return 0;
}

static inline guint64 g_key_file_get_uint64(GKeyFile *key_file,
                                            const gchar *group_name,
                                            const gchar *key,
                                            GError **error)
{
  (void)key_file;
  (void)group_name;
  (void)key;
  if (error != NULL)
    {
      *error = NULL;
    }

  return 0;
}

static inline void g_key_file_set_uint64(GKeyFile *key_file,
                                         const gchar *group_name,
                                         const gchar *key,
                                         guint64 value)
{
  (void)key_file;
  (void)group_name;
  (void)key;
  (void)value;
}

static inline gchar **g_key_file_get_string_list(GKeyFile *key_file,
                                                 const gchar *group_name,
                                                 const gchar *key,
                                                 gsize *length,
                                                 GError **error)
{
  (void)key_file;
  (void)group_name;
  (void)key;
  if (length != NULL)
    {
      *length = 0;
    }
  if (error != NULL)
    {
      *error = NULL;
    }

  return NULL;
}

static inline gchar *g_strdup(const gchar *str)
{
  if (str == NULL)
    {
      return NULL;
    }

  return strdup(str);
}

static inline gboolean g_utf8_validate(const char *str, gssize max_len,
                                       const char **end)
{
  const char *cursor = str;
  gssize remaining = max_len;

  if (str == NULL)
    {
      if (end != NULL)
        {
          *end = NULL;
        }

      return FALSE;
    }

  while (*cursor != '\0' && (max_len < 0 || remaining-- > 0))
    {
      cursor++;
    }

  if (end != NULL)
    {
      *end = cursor;
    }

  return TRUE;
}

static inline gchar *g_strconcat(const gchar *string1, ...)
{
  va_list ap;
  const gchar *part;
  size_t length = 0;
  gchar *result;
  gchar *cursor;

  if (string1 == NULL)
    {
      return NULL;
    }

  va_start(ap, string1);
  length = strlen(string1);
  while ((part = va_arg(ap, const gchar *)) != NULL)
    {
      length += strlen(part);
    }
  va_end(ap);

  result = g_malloc(length + 1);
  if (result == NULL)
    {
      return NULL;
    }

  cursor = result;
  strcpy(cursor, string1);
  cursor += strlen(cursor);

  va_start(ap, string1);
  while ((part = va_arg(ap, const gchar *)) != NULL)
    {
      strcpy(cursor, part);
      cursor += strlen(part);
    }
  va_end(ap);

  return result;
}

static inline gchar *g_strdelimit(gchar *string,
                                  const gchar *delimiters,
                                  gchar new_delimiter)
{
  gchar *cursor;

  if (string == NULL)
    {
      return NULL;
    }

  if (delimiters == NULL)
    {
      delimiters = "_-|> <.";
    }

  for (cursor = string; *cursor != '\0'; cursor++)
    {
      if (strchr(delimiters, *cursor) != NULL)
        {
          *cursor = new_delimiter;
        }
    }

  return string;
}

static inline gchar **g_strsplit(const gchar *string,
                                 const gchar *delimiter,
                                 gint max_tokens)
{
  gchar **vector;

  (void)delimiter;
  (void)max_tokens;

  vector = (gchar **)g_malloc0(sizeof(gchar *) * 2);
  if (vector == NULL)
    {
      return NULL;
    }

  vector[0] = g_strdup(string == NULL ? "" : string);
  vector[1] = NULL;
  return vector;
}

static inline int g_strcmp0(const char *a, const char *b)
{
  if (a == NULL || b == NULL)
    {
      return a == b ? 0 : (a == NULL ? -1 : 1);
    }

  return strcmp(a, b);
}

static inline gboolean g_str_equal(gconstpointer a, gconstpointer b)
{
  return g_strcmp0(a, b) == 0;
}

static inline guint g_str_hash(gconstpointer key)
{
  const unsigned char *str = key;
  guint hash = 5381;

  if (str == NULL)
    {
      return 0;
    }

  while (*str != '\0')
    {
      hash = ((hash << 5) + hash) + *str++;
    }

  return hash;
}

static inline GHashTable *g_hash_table_new_full(GHashFunc hash_func,
                                                GEqualFunc key_equal_func,
                                                GDestroyNotify key_destroy,
                                                GDestroyNotify value_destroy)
{
  (void)hash_func;
  (void)key_equal_func;
  (void)key_destroy;
  (void)value_destroy;
  return calloc(1, sizeof(GHashTable));
}

static inline void g_hash_table_unref(GHashTable *table)
{
  g_free(table);
}

static inline gboolean g_hash_table_insert(GHashTable *table, gpointer key,
                                           gpointer value)
{
  (void)table;
  (void)key;
  (void)value;
  return TRUE;
}

static inline gboolean g_hash_table_add(GHashTable *table, gpointer key)
{
  return g_hash_table_insert(table, key, key);
}

static inline void g_hash_table_destroy(GHashTable *table)
{
  g_hash_table_unref(table);
}

static inline gpointer g_hash_table_lookup(GHashTable *table,
                                           gconstpointer key)
{
  (void)table;
  (void)key;
  return NULL;
}

static inline GList *g_hash_table_get_keys(GHashTable *table)
{
  (void)table;
  return NULL;
}

static inline GTimer *g_timer_new(void)
{
  return calloc(1, sizeof(GTimer));
}

static inline void g_timer_destroy(GTimer *timer)
{
  g_free(timer);
}

static inline void g_timer_start(GTimer *timer)
{
  (void)timer;
}

static inline double g_timer_elapsed(GTimer *timer, unsigned long *microseconds)
{
  (void)timer;
  if (microseconds != NULL)
    {
      *microseconds = 0;
    }

  return 0.0;
}

#define GUINT_TO_POINTER(u) ((gpointer)(uintptr_t)(u))
#define GPOINTER_TO_UINT(p) ((guint)(uintptr_t)(p))
#define GINT_TO_POINTER(i) ((gpointer)(intptr_t)(i))
#define GPOINTER_TO_INT(p) ((gint)(intptr_t)(p))
#define UINT_TO_PTR(u) GUINT_TO_POINTER(u)
#define PTR_TO_UINT(p) GPOINTER_TO_UINT(p)
#define INT_TO_PTR(i) GINT_TO_POINTER(i)
#define PTR_TO_INT(p) GPOINTER_TO_INT(p)

static inline void g_clear_error(GError **error)
{
  if (error != NULL && *error != NULL)
    {
      g_error_free(*error);
      *error = NULL;
    }
}

static inline void g_io_channel_set_close_on_unref(GIOChannel *channel,
                                                   gboolean do_close)
{
  (void)channel;
  (void)do_close;
}

static inline gboolean g_str_has_prefix(const char *str, const char *prefix)
{
  size_t prefix_len;

  if (str == NULL || prefix == NULL)
    {
      return FALSE;
    }

  prefix_len = strlen(prefix);
  return strncmp(str, prefix, prefix_len) == 0;
}

static inline gboolean g_hash_table_replace(GHashTable *table, gpointer key,
                                            gpointer value)
{
  return g_hash_table_insert(table, key, value);
}

#define g_new0(type, n) ((type *)g_malloc0(sizeof(type) * (n)))
#define g_new(type, n) ((type *)g_malloc(sizeof(type) * (n)))

static inline GSList *g_slist_next(GSList *list)
{
  return list == NULL ? NULL : list->next;
}

static inline guint g_slist_length(GSList *list)
{
  guint length = 0;

  for (; list != NULL; list = list->next)
    {
      length++;
    }

  return length;
}

static inline GSList *g_slist_append(GSList *list, gpointer data)
{
  GSList *node = g_new0(GSList, 1);
  GSList *tail;

  if (node == NULL)
    {
      return list;
    }

  node->data = data;

  if (list == NULL)
    {
      return node;
    }

  for (tail = list; tail->next != NULL; tail = tail->next)
    {
    }

  tail->next = node;
  return list;
}

static inline gchar *g_strdup_printf(const gchar *format, ...)
{
  char stack[256];
  va_list ap;
  int n;

  va_start(ap, format);
  n = vsnprintf(stack, sizeof(stack), format, ap);
  va_end(ap);

  if (n < 0)
    {
      return NULL;
    }

  if ((size_t)n < sizeof(stack))
    {
      return g_strdup(stack);
    }

  return NULL;
}

static inline GSList *g_slist_prepend(GSList *list, gpointer data)
{
  GSList *node = g_new0(GSList, 1);

  if (node == NULL)
    {
      return list;
    }

  node->data = data;
  node->next = list;
  return node;
}

static inline GSList *g_slist_remove(GSList *list, gconstpointer data)
{
  GSList *node = list;
  GSList *prev = NULL;

  while (node != NULL)
    {
      if (node->data == data)
        {
          if (prev != NULL)
            {
              prev->next = node->next;
            }
          else
            {
              list = node->next;
            }

          free(node);
          break;
        }

      prev = node;
      node = node->next;
    }

  return list;
}

static inline GSList *g_slist_find(GSList *list, gconstpointer data)
{
  while (list != NULL)
    {
      if (list->data == data)
        {
          return list;
        }

      list = list->next;
    }

  return NULL;
}

static inline GSList *g_slist_find_custom(GSList *list, gconstpointer data,
                                          int (*func)(gconstpointer a,
                                                      gconstpointer b))
{
  while (list != NULL)
    {
      if (func != NULL && func(list->data, data) == 0)
        {
          return list;
        }

      list = list->next;
    }

  return NULL;
}

static inline void g_slist_foreach(GSList *list, GFunc func,
                                   gpointer user_data)
{
  while (list != NULL)
    {
      GSList *next = list->next;
      func(list->data, user_data);
      list = next;
    }
}

static inline void g_slist_free_full(GSList *list, GDestroyNotify free_func)
{
  while (list != NULL)
    {
      GSList *next = list->next;

      if (free_func != NULL)
        {
          free_func(list->data);
        }

      free(list);
      list = next;
    }
}

static inline void g_slist_free(GSList *list)
{
  g_slist_free_full(list, NULL);
}

static inline gpointer g_list_nth_data(GList *list, guint n)
{
  while (list != NULL && n-- > 0)
    {
      list = list->next;
    }

  return list == NULL ? NULL : list->data;
}

static inline GList *g_list_remove(GList *list, gconstpointer data)
{
  GList *node = list;

  while (node != NULL)
    {
      if (node->data == data)
        {
          if (node->prev != NULL)
            {
              node->prev->next = node->next;
            }
          else
            {
              list = node->next;
            }

          if (node->next != NULL)
            {
              node->next->prev = node->prev;
            }

          free(node);
          break;
        }

      node = node->next;
    }

  return list;
}

static inline int g_io_channel_unix_get_fd(GIOChannel *channel)
{
  return channel == NULL ? -1 : channel->fd;
}

static inline GIOChannel *g_io_channel_unix_new(int fd)
{
  static GIOChannel channels[8];
  static unsigned int next;
  GIOChannel *channel = &channels[next++ % (sizeof(channels) /
                                            sizeof(channels[0]))];

  channel->fd = fd;
  return channel;
}

static inline GIOChannel *g_io_channel_ref(GIOChannel *channel)
{
  return channel;
}

static inline void g_io_channel_unref(GIOChannel *channel)
{
  (void)channel;
}

static inline void g_io_channel_shutdown(GIOChannel *channel,
                                         gboolean flush,
                                         GError **error)
{
  (void)channel;
  (void)flush;
  if (error != NULL)
    {
      *error = NULL;
    }
}

static inline guint g_io_add_watch(GIOChannel *channel, GIOCondition condition,
                                   GIOFunc func, gpointer data)
{
  (void)channel;
  (void)condition;
  (void)func;
  (void)data;
  return 1;
}

static inline gboolean g_source_remove(guint tag)
{
  (void)tag;
  return TRUE;
}

static inline guint g_idle_add(GSourceFunc function, gpointer data)
{
  (void)function;
  (void)data;
  return 1;
}

#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_GQUEUE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_GQUEUE_COMPAT

typedef struct _GQueue {
	GSList *head;
	GSList *tail;
	unsigned int length;
} GQueue;

static inline GQueue *g_queue_new(void)
{
	return (GQueue *) calloc(1, sizeof(GQueue));
}

static inline void g_queue_push_tail(GQueue *queue, gpointer data)
{
	GSList *node;

	if (!queue)
		return;

	node = (GSList *) calloc(1, sizeof(GSList));
	if (!node)
		return;

	node->data = data;

	if (queue->tail)
		queue->tail->next = node;
	else
		queue->head = node;

	queue->tail = node;
	queue->length++;
}

static inline gpointer g_queue_pop_head(GQueue *queue)
{
	GSList *node;
	gpointer data;

	if (!queue || !queue->head)
		return NULL;

	node = queue->head;
	data = node->data;
	queue->head = node->next;
	if (!queue->head)
		queue->tail = NULL;
	if (queue->length)
		queue->length--;
	free(node);

	return data;
}

static inline gboolean g_queue_is_empty(GQueue *queue)
{
	return !queue || !queue->head;
}

static inline void g_queue_foreach(GQueue *queue, GFunc func, gpointer user_data)
{
	GSList *node;

	if (!queue || !func)
		return;

	for (node = queue->head; node; node = node->next)
		func(node->data, user_data);
}

static inline void g_queue_free(GQueue *queue)
{
	while (queue && queue->head)
		(void) g_queue_pop_head(queue);
	free(queue);
}

#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_GSLIST_NTH_DATA_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_GSLIST_NTH_DATA_COMPAT

static inline gpointer g_slist_nth_data(GSList *list, unsigned int n)
{
	while (list && n) {
		list = list->next;
		n--;
	}

	return list ? list->data : NULL;
}

#endif

#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_GLIB_DEVICE_COMPAT
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_GLIB_DEVICE_COMPAT

typedef uint8_t guint8;
typedef int8_t gint8;
typedef int16_t gint16;
typedef gint (*GCompareFunc)(gconstpointer a, gconstpointer b);

#ifndef g_try_new0
#define g_try_new0(type, n) ((type *)g_malloc0(sizeof(type) * (n)))
#endif
#ifndef g_try_malloc0
#define g_try_malloc0(n) g_malloc0(n)
#endif
#ifndef g_renew
#define g_renew(type, mem, n) ((type *)realloc((mem), sizeof(type) * (n)))
#endif

static inline gboolean g_error_matches(const GError *error, int domain,
                                       int code)
{
	(void)domain;
	return error != NULL && error->code == code;
}

#endif
