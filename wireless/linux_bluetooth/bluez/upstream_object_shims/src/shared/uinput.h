#ifndef BLUEZ_UPSTREAM_OBJECT_SHIMS_SRC_SHARED_UINPUT_H
#define BLUEZ_UPSTREAM_OBJECT_SHIMS_SRC_SHARED_UINPUT_H

#include <stdbool.h>
#include <stdint.h>

#include "bluetooth/bluetooth.h"

struct input_id;

struct bt_uinput {
	int fd;
};

struct bt_uinput_key_map {
	const char *name;
	unsigned int code;
	uint16_t uinput;
};

typedef void (*bt_uinput_debug_func_t)(const char *str, void *user_data);

static inline struct bt_uinput *bt_uinput_new(const char *name,
					const char *suffix, const bdaddr_t *addr,
					const struct input_id *dev_id)
{
	static struct bt_uinput uinput;

	(void) name;
	(void) suffix;
	(void) addr;
	(void) dev_id;
	uinput.fd = -1;
	return &uinput;
}

static inline void bt_uinput_set_debug(struct bt_uinput *uinput,
					bt_uinput_debug_func_t debug_func,
					void *user_data)
{
	(void) uinput;
	(void) debug_func;
	(void) user_data;
}

static inline int bt_uinput_create(struct bt_uinput *uinput,
					const struct bt_uinput_key_map *key_map)
{
	(void) uinput;
	(void) key_map;
	return 0;
}

static inline void bt_uinput_destroy(struct bt_uinput *uinput)
{
	(void) uinput;
}

static inline void bt_uinput_send_key(struct bt_uinput *uinput,
					uint16_t key, bool pressed)
{
	(void) uinput;
	(void) key;
	(void) pressed;
}

#endif
