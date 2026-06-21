#ifndef BLUEZ_UPSTREAM_OBJECT_SHIM_BLUETOOTH_HCI_LIB_H
#define BLUEZ_UPSTREAM_OBJECT_SHIM_BLUETOOTH_HCI_LIB_H

static inline int hci_get_route(const void *bdaddr)
{
  (void)bdaddr;
  return 0;
}

#endif /* BLUEZ_UPSTREAM_OBJECT_SHIM_BLUETOOTH_HCI_LIB_H */
