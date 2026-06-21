#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_QUEUE_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_QUEUE_H

#include <stdbool.h>
#include <stdlib.h>

typedef void (*queue_destroy_func_t)(void *data);
typedef void (*queue_foreach_func_t)(void *data, void *user_data);
typedef bool (*queue_match_func_t)(const void *data, const void *match_data);

struct queue_entry
{
  void *data;
  struct queue_entry *next;
};

struct queue
{
  struct queue_entry *head;
  struct queue_entry *tail;
};

static inline struct queue *queue_new(void)
{
  return calloc(1, sizeof(struct queue));
}

static inline void queue_destroy(struct queue *queue,
                                 queue_destroy_func_t destroy)
{
  struct queue_entry *entry;

  if (queue == NULL)
    {
      return;
    }

  entry = queue->head;
  while (entry != NULL)
    {
      struct queue_entry *next = entry->next;

      if (destroy != NULL)
        {
          destroy(entry->data);
        }

      free(entry);
      entry = next;
    }

  free(queue);
}

static inline bool queue_push_tail(struct queue *queue, void *data)
{
  struct queue_entry *entry;

  if (queue == NULL)
    {
      return false;
    }

  entry = calloc(1, sizeof(*entry));
  if (entry == NULL)
    {
      return false;
    }

  entry->data = data;

  if (queue->tail != NULL)
    {
      queue->tail->next = entry;
    }
  else
    {
      queue->head = entry;
    }

  queue->tail = entry;
  return true;
}

static inline bool queue_push_head(struct queue *queue, void *data)
{
  struct queue_entry *entry;

  if (queue == NULL)
    {
      return false;
    }

  entry = calloc(1, sizeof(*entry));
  if (entry == NULL)
    {
      return false;
    }

  entry->data = data;
  entry->next = queue->head;
  queue->head = entry;

  if (queue->tail == NULL)
    {
      queue->tail = entry;
    }

  return true;
}

static inline void *queue_pop_head(struct queue *queue)
{
  struct queue_entry *entry;
  void *data;

  if (queue == NULL || queue->head == NULL)
    {
      return NULL;
    }

  entry = queue->head;
  data = entry->data;
  queue->head = entry->next;

  if (queue->tail == entry)
    {
      queue->tail = NULL;
    }

  free(entry);
  return data;
}

static inline bool queue_remove(struct queue *queue, void *data)
{
  struct queue_entry *entry;
  struct queue_entry *prev = NULL;

  if (queue == NULL)
    {
      return false;
    }

  entry = queue->head;
  while (entry != NULL)
    {
      if (entry->data == data)
        {
          if (prev != NULL)
            {
              prev->next = entry->next;
            }
          else
            {
              queue->head = entry->next;
            }

          if (queue->tail == entry)
            {
              queue->tail = prev;
            }

          free(entry);
          return true;
        }

      prev = entry;
      entry = entry->next;
    }

  return false;
}

static inline void *queue_find(struct queue *queue, queue_match_func_t match,
                               const void *match_data)
{
  struct queue_entry *entry;

  if (queue == NULL || match == NULL)
    {
      return NULL;
    }

  for (entry = queue->head; entry != NULL; entry = entry->next)
    {
      if (match(entry->data, match_data))
        {
          return entry->data;
        }
    }

  return NULL;
}

static inline unsigned int queue_remove_all(struct queue *queue,
                                            queue_match_func_t match,
                                            const void *match_data,
                                            queue_destroy_func_t destroy)
{
  struct queue_entry *entry;
  struct queue_entry *prev = NULL;
  unsigned int count = 0;

  if (queue == NULL)
    {
      return 0;
    }

  entry = queue->head;
  while (entry != NULL)
    {
      struct queue_entry *next = entry->next;
      bool matched = match == NULL || match(entry->data, match_data);

      if (matched)
        {
          if (prev != NULL)
            {
              prev->next = next;
            }
          else
            {
              queue->head = next;
            }

          if (queue->tail == entry)
            {
              queue->tail = prev;
            }

          if (destroy != NULL)
            {
              destroy(entry->data);
            }

          free(entry);
          count++;
        }
      else
        {
          prev = entry;
        }

      entry = next;
    }

  return count;
}

static inline void queue_foreach(struct queue *queue, queue_foreach_func_t func,
                                 void *user_data)
{
  struct queue_entry *entry;

  if (queue == NULL || func == NULL)
    {
      return;
    }

  for (entry = queue->head; entry != NULL; entry = entry->next)
    {
      func(entry->data, user_data);
    }
}

static inline const struct queue_entry *queue_get_entries(struct queue *queue)
{
  return queue == NULL ? NULL : queue->head;
}

static inline bool queue_isempty(struct queue *queue)
{
  return queue == NULL || queue->head == NULL;
}

static inline unsigned int queue_length(struct queue *queue)
{
  unsigned int count = 0;
  struct queue_entry *entry;

  if (queue == NULL)
    {
      return 0;
    }

  for (entry = queue->head; entry != NULL; entry = entry->next)
    {
      count++;
    }

  return count;
}

static inline void *queue_remove_if(struct queue *queue,
                                    queue_match_func_t match,
                                    const void *match_data)
{
  struct queue_entry *entry;
  struct queue_entry *prev = NULL;

  if (queue == NULL || match == NULL)
    {
      return NULL;
    }

  entry = queue->head;
  while (entry != NULL)
    {
      if (match(entry->data, match_data))
        {
          void *data = entry->data;

          if (prev != NULL)
            {
              prev->next = entry->next;
            }
          else
            {
              queue->head = entry->next;
            }

          if (queue->tail == entry)
            {
              queue->tail = prev;
            }

          free(entry);
          return data;
        }

      prev = entry;
      entry = entry->next;
    }

  return NULL;
}
#endif
