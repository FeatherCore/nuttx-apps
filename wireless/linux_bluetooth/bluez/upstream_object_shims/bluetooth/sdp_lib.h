#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_SDP_LIB_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_SDP_LIB_H

#include "bluetooth/sdp.h"
#include <stdlib.h>

static inline sdp_record_t *sdp_record_alloc(void)
{
  sdp_record_t *record = calloc(1, sizeof(*record));

  if (record != NULL)
    {
      record->handle = 1;
    }

  return record;
}

static inline void sdp_record_free(sdp_record_t *record)
{
  free(record);
}

static inline void sdp_uuid16_create(uuid_t *uuid, uint16_t value)
{
  if (uuid != NULL)
    {
      uuid->type = SDP_UUID16;
      uuid->value.uuid16 = value;
    }
}

static inline sdp_list_t *sdp_list_append(sdp_list_t *list, void *data)
{
  sdp_list_t *node = calloc(1, sizeof(*node));
  sdp_list_t *tail;

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

static inline sdp_list_t *sdp_list_insert_sorted(sdp_list_t *list,
                                                 void *data,
                                                 int (*cmp)(const void *,
                                                            const void *))
{
  sdp_list_t *node = calloc(1, sizeof(*node));
  sdp_list_t *cur;
  sdp_list_t *prev = NULL;

  if (node == NULL)
    {
      return list;
    }

  node->data = data;

  for (cur = list; cur != NULL; cur = cur->next)
    {
      if (cmp == NULL || cmp(data, cur->data) <= 0)
        {
          break;
        }

      prev = cur;
    }

  node->next = cur;
  if (prev != NULL)
    {
      prev->next = node;
      return list;
    }

  return node;
}

static inline sdp_list_t *sdp_list_find(sdp_list_t *list, const void *data,
                                        int (*cmp)(const void *,
                                                   const void *))
{
  for (; list != NULL; list = list->next)
    {
      if (cmp == NULL)
        {
          if (list->data == data)
            {
              return list;
            }
        }
      else if (cmp(list->data, data) == 0)
        {
          return list;
        }
    }

  return NULL;
}

static inline sdp_list_t *sdp_list_remove(sdp_list_t *list, void *data)
{
  sdp_list_t *cur = list;
  sdp_list_t *prev = NULL;

  while (cur != NULL)
    {
      if (cur->data == data)
        {
          sdp_list_t *next = cur->next;

          if (prev != NULL)
            {
              prev->next = next;
            }
          else
            {
              list = next;
            }

          free(cur);
          return list;
        }

      prev = cur;
      cur = cur->next;
    }

  return list;
}

static inline void sdp_set_browse_groups(sdp_record_t *record,
                                         sdp_list_t *root)
{
  if (record != NULL)
    {
      record->pattern = root;
    }
}

static inline void sdp_set_service_classes(sdp_record_t *record,
                                           sdp_list_t *classes)
{
  if (record != NULL)
    {
      record->svclass = classes;
    }
}

static inline void sdp_set_profile_descs(sdp_record_t *record,
                                         sdp_list_t *descs)
{
  (void)record;
  (void)descs;
}

static inline void sdp_set_access_protos(sdp_record_t *record,
                                         sdp_list_t *protos)
{
  (void)record;
  (void)protos;
}

static inline sdp_data_t *sdp_data_alloc(uint8_t dtd, const void *value)
{
  sdp_data_t *data = calloc(1, sizeof(*data));

  if (data != NULL)
    {
      data->dtd = dtd;
      if (value != NULL)
        {
          data->val.uint16 = *(const uint16_t *)value;
        }
    }

  return data;
}

static inline void sdp_attr_add(sdp_record_t *record, uint16_t attr,
                                sdp_data_t *data)
{
  (void)attr;
  if (record != NULL)
    {
      record->attrlist = sdp_list_append(record->attrlist, data);
    }
}

static inline void sdp_attr_replace(sdp_record_t *record, uint16_t attr,
                                    sdp_data_t *data)
{
  sdp_attr_add(record, attr, data);
}

static inline void sdp_attr_add_new(sdp_record_t *record, uint16_t attr,
                                    uint8_t dtd, const void *value)
{
  sdp_attr_add(record, attr, sdp_data_alloc(dtd, value));
}

static inline void sdp_set_info_attr(sdp_record_t *record, const char *name,
                                     const char *provider,
                                     const char *description)
{
  (void)record;
  (void)name;
  (void)provider;
  (void)description;
}

static inline int sdp_get_access_protos(const sdp_record_t *rec,
                                        sdp_list_t **protos)
{
  (void)rec;
  if (protos != NULL)
    {
      *protos = NULL;
    }

  return -1;
}

static inline sdp_data_t *sdp_seq_alloc(void **dtds, void **values, int len)
{
  (void)dtds;
  (void)values;
  (void)len;
  return sdp_data_alloc(SDP_UINT16, NULL);
}

static inline sdp_data_t *sdp_data_get(sdp_record_t *record, uint16_t attr)
{
  (void)attr;
  if (record == NULL || record->attrlist == NULL)
    {
      return NULL;
    }

  return record->attrlist->data;
}

static inline void sdp_pattern_add_uuid(sdp_record_t *record,
                                        const uuid_t *uuid)
{
  if (record != NULL)
    {
      record->pattern = sdp_list_append(record->pattern, (void *)uuid);
    }
}

static inline int sdp_extract_seqtype(const uint8_t *pdata, int size,
                                      uint8_t *dtd, int *seqlen)
{
  (void)pdata;
  if (dtd != NULL)
    {
      *dtd = 0;
    }

  if (seqlen != NULL)
    {
      *seqlen = size > 0 ? size : 0;
    }

  return size > 0 ? 1 : 0;
}

static inline sdp_data_t *sdp_extract_attr(const uint8_t *pdata, int size,
                                           int *extracted,
                                           sdp_record_t *record)
{
  (void)pdata;
  (void)record;

  if (extracted != NULL)
    {
      *extracted = size > 0 ? 1 : 0;
    }

  return sdp_data_alloc(SDP_UINT16, NULL);
}

static inline void sdp_data_free(sdp_data_t *data)
{
  free(data);
}

static inline void sdp_print_service_attr(sdp_list_t *attrlist)
{
  (void)attrlist;
}

static inline uint16_t get_be16(const void *ptr)
{
  const uint8_t *p = ptr;

  return ((uint16_t)p[0] << 8) | p[1];
}

static inline uint32_t get_be32(const void *ptr)
{
  const uint8_t *p = ptr;

  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) | p[3];
}

static inline void put_be16(uint16_t value, void *ptr)
{
  uint8_t *p = ptr;

  p[0] = value >> 8;
  p[1] = value;
}

static inline void put_be32(uint32_t value, void *ptr)
{
  uint8_t *p = ptr;

  p[0] = value >> 24;
  p[1] = value >> 16;
  p[2] = value >> 8;
  p[3] = value;
}

static inline sdp_record_t *sdp_extract_pdu(const uint8_t *pdata, int size,
                                            int *scanned)
{
  sdp_record_t *record;

  (void)pdata;

  if (scanned != NULL)
    {
      *scanned = size;
    }

  record = sdp_record_alloc();
  if (record != NULL)
    {
      uuid_t *uuid = calloc(1, sizeof(*uuid));

      if (uuid != NULL)
        {
          sdp_uuid16_create(uuid, AUDIO_SINK_SVCLASS_ID);
          record->svclass = sdp_list_append(record->svclass, uuid);
        }
    }

  return record;
}

static inline int sdp_get_service_classes(const sdp_record_t *rec,
                                          sdp_list_t **classes)
{
  if (classes == NULL)
    {
      return -1;
    }

  *classes = rec == NULL ? NULL : rec->svclass;
  return *classes == NULL ? -1 : 0;
}

static inline sdp_data_t *sdp_get_proto_desc(sdp_list_t *list,
                                             uint16_t proto)
{
  (void)list;
  (void)proto;
  return NULL;
}

static inline void sdp_list_foreach(sdp_list_t *list, sdp_list_func_t func,
                                    void *user_data)
{
  while (list != NULL)
    {
      sdp_list_t *next = list->next;

      if (func != NULL)
        {
          func(list->data, user_data);
        }

      list = next;
    }
}

static inline void sdp_list_free(sdp_list_t *list, void (*free_func)(void *))
{
  while (list != NULL)
    {
      sdp_list_t *next = list->next;

      if (free_func != NULL)
        {
          free_func(list->data);
        }

      list = next;
    }
}
#endif
