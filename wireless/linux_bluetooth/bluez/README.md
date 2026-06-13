# BlueZ userspace port for NuttX Linux Bluetooth

This directory is the apps-side home for the full BlueZ userspace port.

The upstream source is kept as a single authoritative tree at:

```text
/home/uan/Feather-develop-BT/third/bluez
```

and is exposed here through:

```text
upstream -> ../../../../../third/bluez
```

Porting rule:

- Keep Linux kernel Bluetooth protocol behavior under `nuttx/wireless/linux_bluetooth` and `nuttx/drivers/bluetooth`.
- Keep BlueZ userspace behavior under this `apps/wireless/linux_bluetooth/bluez` subtree.
- Do not make `btctl` the final demo driver.  `btctl` remains a low-level probe; sim demos should progressively move to BlueZ tools/daemon/profile entry points from this directory.

Current executable landing points:

```text
bluezbneptest
bluezmgmt
bluezbtmon
bluezhciioctl
bluezhciraw
bluezdaemon
```

`bluezbneptest` is the first BlueZ tool adapter.  It targets the network-profile
shape from `upstream/tools/bneptest.c`: connected L2CAP fd -> `BNEPCONNADD` ->
BNEP connlist/conninfo -> `BNEPCONNDEL` cleanup.

Next required BlueZ port stages:

```text
bluetoothd + mainloop/DBus shims
bluetoothctl or mgmt client tool over NuttX HCI/mgmt sockets
network profile / bneptest PAN demo replacing btctl-only setup
A2DP MediaEndpoint/MediaTransport source/sink demo
LE Audio BAP/PACS/ASCS/ISO demo
```

bluezbtmon
----------

`bluezbtmon` is the first BlueZ btmon-style adapter.  It validates the public
`AF_BLUETOOTH/SOCK_RAW/BTPROTO_HCI` monitor channel by binding
`HCI_CHANNEL_MONITOR`, then triggering a small mgmt control exchange and reading
monitor records from the monitor fd.  The hwsim gate is `bluez-btmon-monitor`.

Current public-fd path:

```text
bluezbtmon control
  -> socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)
  -> bind(HCI_CHANNEL_MONITOR)
  -> linux_bt_sockif.c
  -> linux_bt_upstream_hci_socket_open()
  -> upstream hci_sock.c
```

Current result:

```text
PASS bluez-btmon-monitor
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-btmon-monitor-public-hci-upstream-socket-final/bluez-btmon-monitor.ble1.log
```

The old global monitor/mgmt probe counters intentionally remain zero in this
gate; monitor records are delivered through the per-fd upstream HCI socket.

bluezhciioctl
-------------

`bluezhciioctl` is the first BlueZ hciconfig-style adapter.  It validates the
public `AF_BLUETOOTH/SOCK_RAW/BTPROTO_HCI` ioctl ABI with `HCIGETDEVLIST`,
`HCIGETDEVINFO`, `HCIDEVUP`, `HCIDEVRESTAT`, `HCIDEVRESET`, and `HCIDEVDOWN`.
The hwsim gate is `bluez-hciioctl-basic`.

Current public-fd path:

```text
bluezhciioctl basic
  -> socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)
  -> ioctl(HCIGETDEVLIST / HCIGETDEVINFO / HCIDEV*)
  -> linux_bt_sockif.c
  -> linux_bt_upstream_hci_socket_create()
  -> upstream hci_sock.c ioctl
```

Current result:

```text
PASS bluez-hciioctl-basic
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-hciioctl-public-hci-upstream-socket/bluez-hciioctl-basic.ble1.log
```

bluezhciraw
-----------

`bluezhciraw` is the first BlueZ hcitool-style raw HCI adapter.  It validates
`AF_BLUETOOTH/SOCK_RAW/BTPROTO_HCI` with `HCI_CHANNEL_RAW`, `HCI_FILTER`
set/get sockopts, raw `HCI_COMMAND_PKT` send, and `HCI_EVENT_PKT` command
complete receive.  The hwsim gate is `bluez-hciraw-command`.

Current public-fd path:

```text
bluezhciraw command
  -> socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)
  -> bind(HCI_CHANNEL_RAW / hci0)
  -> setsockopt/getsockopt(HCI_FILTER)
  -> send(HCI_COMMAND_PKT Read Local Version)
  -> recv(HCI_EVENT_PKT Command Complete)
  -> linux_bt_sockif.c
  -> linux_bt_upstream_hci_socket_open()
  -> upstream hci_sock.c
```

Current result:

```text
PASS bluez-hciraw-command
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-hciraw-public-hci-upstream-socket-final/bluez-hciraw-command.ble1.log
```

The old staging data-socket RX counter intentionally remains zero in this gate;
the command/event path is delivered through the per-fd upstream HCI raw socket.

bluezmgmt pair-noio
-------------------

`bluezmgmt pair-noio` validates a BlueZ btmgmt-style NoInputNoOutput pairing
flow over the public mgmt control socket.  It sends `MGMT_OP_SET_IO_CAPABILITY`
and `MGMT_OP_PAIR_DEVICE`, then reads the pairing event and pair-device command
complete from the same fd.  The hwsim gate is `bluez-mgmt-pair-noio`.

bluezmgmt user-confirm
----------------------

`bluezmgmt user-confirm` validates the DisplayYesNo user-confirm pairing path
over the public BlueZ/Linux mgmt control socket.  It sends
`MGMT_OP_PAIR_DEVICE`, observes `MGMT_EV_USER_CONFIRM_REQUEST`, replies with
`MGMT_OP_USER_CONFIRM_REPLY`, then receives `MGMT_EV_NEW_LONG_TERM_KEY` and the
final `Pair Device` command complete with `MGMT_STATUS_SUCCESS`.  The hwsim
gate is `bluez-mgmt-user-confirm`.  This still uses the `CONFIG_SIM_BTHWSIM`
shortcut in imported `net/bluetooth/mgmt.c`; it validates the public ABI and
event order, but not the full upstream SMP/security workqueue engine.

bluezmgmt user-confirm-neg
--------------------------

`bluezmgmt user-confirm-neg` validates the DisplayYesNo user-confirm reject
path over the public BlueZ/Linux mgmt control socket.  It sends
`MGMT_OP_PAIR_DEVICE`, observes `MGMT_EV_USER_CONFIRM_REQUEST`, replies with
`MGMT_OP_USER_CONFIRM_NEG_REPLY`, then receives the negative-reply command
complete with `MGMT_STATUS_SUCCESS` and the original `Pair Device` command
complete with `MGMT_STATUS_FAILED`.  The hwsim gate is
`bluez-mgmt-user-confirm-neg`.  This still uses the `CONFIG_SIM_BTHWSIM`
shortcut in imported `net/bluetooth/mgmt.c`; it validates the public ABI and
event order, but not the full upstream SMP/security workqueue engine.

bluezmgmt passkey
-----------------

`bluezmgmt passkey` validates the KeyboardOnly passkey pairing path over the
public BlueZ/Linux mgmt control socket.  It sends `MGMT_OP_PAIR_DEVICE`,
observes `MGMT_EV_USER_PASSKEY_REQUEST`, replies with an exact 11-byte
`MGMT_OP_USER_PASSKEY_REPLY` payload carrying passkey `123456`, then receives
`MGMT_EV_NEW_LONG_TERM_KEY` and the final `Pair Device` command complete with
`MGMT_STATUS_SUCCESS`.  The hwsim gate is `bluez-mgmt-passkey`.  This still uses
the `CONFIG_SIM_BTHWSIM` shortcut in imported `net/bluetooth/mgmt.c`; it
validates the public ABI and event order, but not the full upstream
SMP/security workqueue engine.

bluezmgmt passkey-neg
---------------------

`bluezmgmt passkey-neg` validates the KeyboardOnly passkey reject path over the
public BlueZ/Linux mgmt control socket.  It sends `MGMT_OP_PAIR_DEVICE`,
observes `MGMT_EV_USER_PASSKEY_REQUEST`, replies with
`MGMT_OP_USER_PASSKEY_NEG_REPLY`, then receives the negative-reply command
complete with `MGMT_STATUS_SUCCESS` and the original `Pair Device` command
complete with `MGMT_STATUS_FAILED`.  The hwsim gate is
`bluez-mgmt-passkey-neg`.  This still uses the `CONFIG_SIM_BTHWSIM` shortcut in
imported `net/bluetooth/mgmt.c`; it validates the public ABI and event order,
but not the full upstream SMP/security workqueue engine.

bluezmgmt cancel-pair
---------------------

`bluezmgmt cancel-pair` validates the BlueZ/Linux mgmt cancel-pair ABI over the
public control socket.  The current hwsim shim completes pairing immediately,
so a cancel request without a pending pairing is expected to return
`MGMT_STATUS_INVALID_PARAMS`, matching the staged-kernel behavior documented in
the linux_bluetooth port notes.  The hwsim gate is
`bluez-mgmt-cancel-pair`; it is an ABI/error-path gate, not proof that the full
upstream pending-pairing state machine has been ported.

bluezmgmt cancel-pair-pending
-----------------------------

`bluezmgmt cancel-pair-pending` validates the pending pairing cancel sequence
over the public BlueZ/Linux mgmt control socket.  It sets DisplayYesNo IO
capability, sends `MGMT_OP_PAIR_DEVICE`, observes
`MGMT_EV_USER_CONFIRM_REQUEST`, then sends `MGMT_OP_CANCEL_PAIR_DEVICE`.  The
expected event order is `Pair Device` command complete with
`MGMT_STATUS_CANCELLED`, followed by `Cancel Pair Device` command complete with
`MGMT_STATUS_SUCCESS`.  The hwsim gate is
`bluez-mgmt-cancel-pair-pending`.  This still uses the `CONFIG_SIM_BTHWSIM`
shortcut in imported `net/bluetooth/mgmt.c`; it validates the public ABI and
event order, but not the full upstream SMP/security workqueue engine.

bluezmgmt error-path
--------------------

BlueZ A2DP bidirectional transport update
-----------------------------------------

The A2DP MediaTransport-shaped path now has bidirectional hwsim coverage on the
same BT1/BT2 session.  The verified gates are:

```text
PASS bluez-a2dp-transport-bidir
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-bidir-validator/run-results.json

PASS bluez-a2dp-transport-bidir-teardown
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-bidir-teardown-validator/run-results.json
```

The bidirectional gate covers:

```text
BT1 source -> BT2 sink:
  AVDTP discover/getcap/setconfig/open/start
  MediaTransport Acquire -> L2CAP media write -> peer L2CAP media read -> Release

BT2 source -> BT1 sink:
  AVDTP discover/getcap/setconfig/open/start
  MediaTransport Acquire -> L2CAP media write -> peer L2CAP media read -> Release
```

The teardown gate adds suspend/close-stream transitions for both directions.
The receiver-side validator now requires the handle-based media fd path:

```text
upstream-l2cap-recv-handle: psm=0x0019 cid=0x0041 handle=0x0052 recv-ret=24
```

This is stronger than the earlier global probe evidence because it proves the
media payload is read through an acquired L2CAP transport handle, while the
AVDTP signaling path remains separate.

Boundary: this is still a BlueZ-profile-shaped adapter gate, not the full
`bluetoothd` audio plugin.  The remaining A2DP work is full daemon mainloop and
D-Bus object ownership, upstream AVDTP session ownership, controller-driven
L2CAP channel setup/error policy, codec policy beyond the current SBC gate, and
complete AVRCP/media control behavior.

BlueZ A2DP AVRCP control update
-------------------------------

The A2DP control side now has a first BlueZ MediaControl1-shaped AVRCP hwsim
gate:

```text
PASS bluez-a2dp-avrcp-control
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-control-final/run-results.json
```

The gate covers a minimal AVRCP PASS THROUGH PLAY transaction over the AVCTP
control channel:

```text
BT1 controller:
  org.bluez.MediaControl1.Play
  -> AVCTP control PSM 0x0017 / CID 0x0042
  -> AVRCP PASS THROUGH opid 0x44

BT2 target:
  receive payload 00 11 0e 00 48 7c 44 00
  -> send accepted response 02 11 0e 09 48 7c 44 00

BT1 controller:
  receive accepted response
```

This moves the registered `MediaControl1` / AVCTP / AVRCP evidence from static
lifecycle logging into an actual L2CAP control exchange.  The current hwsim
adapter uses separate short-lived TX/RX handles around the synthetic AVCTP
transaction to avoid the present sim channel-reuse limitation.

Boundary: this is not complete AVRCP.  Metadata, browsing PSM 0x001b,
notifications, player application settings, addressed-player ownership,
absolute volume, and full `bluetoothd` media-control D-Bus object integration
remain to be ported and verified.

BlueZ A2DP AVRCP browsing update
--------------------------------

The AVRCP side now also has first-pass browsing-channel hwsim coverage:

```text
PASS bluez-a2dp-avrcp-browsing
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-browsing/run-results.json
```

The gate uses the AVCTP browsing channel rather than the control channel:

```text
BT1 controller:
  MediaControl1 ListItems-shaped request
  -> AVCTP browsing PSM 0x001b / CID 0x0043
  -> AVRCP browsing GetFolderItems-shaped PDU 0x71
  -> payload 00 11 0e 00 71 00 00 00 01

BT2 target:
  receive GetFolderItems-shaped request
  -> response payload 02 11 0e 09 71 00 00 00 00

BT1 controller:
  receive browsing success response
```

This is intentionally small, but it proves that the BlueZ-shaped audio adapter
can exercise both AVRCP control PSM `0x0017` and browsing PSM `0x001b` through
the NuttX Linux-Bluetooth L2CAP/hwsim path.

Boundary: this is not full AVRCP browsing.  Real folder hierarchy, media
metadata, item attributes, player list ownership, notifications, addressed
player selection, and full `bluetoothd` AVRCP plugin behavior remain to be
ported.

BlueZ A2DP AVRCP notification update
------------------------------------

The A2DP AVRCP side now also has a focused RegisterNotification hwsim gate:

```text
PASS bluez-a2dp-avrcp-notification
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-notification/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-notify` on BT1 and
`bluezaudio avrcp-control target-notify-respond` on BT2.  It covers a
playback-status RegisterNotification exchange over AVCTP control PSM `0x0017`
/ CID `0x0042`:

```text
BT1 controller:
  org.bluez.MediaControl1 RegisterNotification playback-status
  -> payload 00 11 0e 00 31 00 00 05 01 00 00 00

BT2 target:
  receive playback-status notification request
  -> interim response 02 11 0e 0f 31 00 00 02 01 01

BT1 controller:
  receive interim playback-status=playing response
```

The validator requires both sides to show the BlueZ-shaped MediaControl1 path,
AVCTP/AVRCP metadata from `third/bluez/profiles/audio`, L2CAP handle
write/read evidence, and endpoint cleanup.

Boundary: this is not complete AVRCP notification support.  It covers one
playback-status interim notification exchange, not changed-event delivery,
multiple event IDs, addressed-player ownership, player application settings,
absolute volume, metadata, or the full `bluetoothd` AVRCP plugin mainloop.

BlueZ A2DP AVRCP absolute-volume update
---------------------------------------

The A2DP AVRCP side now has a focused SetAbsoluteVolume hwsim gate:

```text
PASS bluez-a2dp-avrcp-absolute-volume
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-absolute-volume/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-volume` on BT1 and
`bluezaudio avrcp-control target-volume-respond` on BT2.  It covers an AVRCP
SetAbsoluteVolume exchange over AVCTP control PSM `0x0017` / CID `0x0042`,
with BlueZ-shaped `org.bluez.MediaTransport1.Volume` evidence:

```text
BT1 controller:
  MediaTransport1.Volume = 64
  -> AVRCP SetAbsoluteVolume PDU 0x50
  -> payload 00 11 0e 00 50 00 00 01 40

BT2 target:
  receive SetAbsoluteVolume
  -> response payload 02 11 0e 09 50 00 00 01 40

BT1 controller:
  receive accepted volume=64 response
```

This extends the current AVRCP coverage from play, browsing, and notification
to the volume-control path that A2DP sinks/sources commonly expose through
BlueZ media transport objects.

Boundary: this is not complete absolute-volume support.  It does not yet cover
volume-changed notifications, volume range mapping across multiple transports,
controller/target role policy, persistence, or full `bluetoothd` AVRCP/media
transport ownership.

BlueZ A2DP AVRCP metadata update
--------------------------------

The A2DP AVRCP side now has a focused GetElementAttributes hwsim gate:

```text
PASS bluez-a2dp-avrcp-metadata
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-metadata/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-metadata` on BT1 and
`bluezaudio avrcp-control target-metadata-respond` on BT2.  It covers an AVRCP
GetElementAttributes exchange over AVCTP control PSM `0x0017` / CID `0x0042`,
with BlueZ-shaped `org.bluez.MediaPlayer1.Track` evidence:

```text
BT1 controller:
  MediaPlayer1.Track query
  -> AVRCP GetElementAttributes PDU 0x20
  -> payload 00 11 0e 00 20 00 00 09 ff ff ff ff ff ff ff ff 00

BT2 target:
  receive GetElementAttributes
  -> title attribute response for "Feather"
  -> payload 02 11 0e 09 20 00 00 10 01 00 00 00 01 00 6a 00 07 46 65 61 74 68 65 72

BT1 controller:
  receive metadata title=Feather response
```

This extends the current AVRCP coverage from transport control and volume to
the media metadata query path commonly surfaced through BlueZ media player
objects.

Boundary: this is not complete metadata support.  It covers one title
attribute response, not full player object lifetime, track change events,
multiple attributes, browsing item metadata, charset/string edge cases, or full
`bluetoothd` AVRCP player ownership.

BlueZ A2DP AVRCP player-settings-list update
--------------------------------------------

The A2DP AVRCP side now has a focused ListPlayerApplicationSettingAttributes
hwsim gate:

```text
PASS bluez-a2dp-avrcp-player-settings-list
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-player-settings-list/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-player-settings-list` on
BT1 and `bluezaudio avrcp-control target-player-settings-list-respond` on BT2.
It covers AVRCP ListPlayerApplicationSettingAttributes over AVCTP control PSM
`0x0017` / CID `0x0042`, with BlueZ-shaped
`org.bluez.MediaPlayer1.SupportedSettings` evidence:

```text
BT1 controller:
  query supported MediaPlayer1 settings
  -> AVRCP PDU 0x11
  -> payload 00 11 0e 00 11 00 00 00

BT2 target:
  receive supported-settings query
  -> repeat/shuffle attribute response
  -> payload 02 11 0e 09 11 00 00 03 02 02 03

BT1 controller:
  receive supported attributes repeat,shuffle
```

This fills in the discovery side before reading or setting player application
settings.

Boundary: this is not complete player-settings discovery.  It does not yet
cover supported value discovery, value text lookup, setting change
notifications, rejected/invalid attribute paths, or full `bluetoothd` AVRCP
player policy.

BlueZ A2DP AVRCP player-settings-values update
----------------------------------------------

The A2DP AVRCP side now has a focused ListPlayerApplicationSettingValues hwsim
gate:

```text
PASS bluez-a2dp-avrcp-player-settings-values
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-player-settings-values/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-player-settings-values`
on BT1 and `bluezaudio avrcp-control target-player-settings-values-respond` on
BT2.  It covers AVRCP ListPlayerApplicationSettingValues over AVCTP control PSM
`0x0017` / CID `0x0042`, with BlueZ-shaped
`org.bluez.MediaPlayer1.SupportedSettingValues` evidence:

```text
BT1 controller:
  query supported repeat values
  -> AVRCP PDU 0x12
  -> payload 00 11 0e 00 12 00 00 01 02

BT2 target:
  receive repeat value query
  -> off/single-track/all-tracks/group response
  -> payload 02 11 0e 09 12 00 00 05 04 01 02 03 04

BT1 controller:
  receive supported repeat values
```

This fills in the value-discovery step between supported setting attributes and
reading or setting the current setting values.

Boundary: this is not complete player-settings value support.  It currently
covers repeat values only, not shuffle/scan/equalizer value discovery, value
text lookup, setting change notifications, rejected/invalid attribute paths, or
full `bluetoothd` AVRCP player policy.

BlueZ A2DP AVRCP player-settings-value-text update
--------------------------------------------------

The A2DP AVRCP side now has a focused
GetPlayerApplicationSettingValueText hwsim gate:

```text
PASS bluez-a2dp-avrcp-player-settings-value-text
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-player-settings-value-text/run-results.json
```

The gate drives
`bluezaudio avrcp-control controller-player-settings-value-text` on BT1 and
`bluezaudio avrcp-control target-player-settings-value-text-respond` on BT2.
It covers AVRCP GetPlayerApplicationSettingValueText over AVCTP control PSM
`0x0017` / CID `0x0042`, with BlueZ-shaped
`org.bluez.MediaPlayer1.SettingValueText` evidence:

```text
BT1 controller:
  query repeat value text for off/single-track
  -> AVRCP PDU 0x16
  -> payload 00 11 0e 00 16 00 00 04 02 02 01 02

BT2 target:
  receive repeat value text query
  -> Off / Single Track response
  -> payload 02 11 0e 09 16 00 00 1a 02 01 00 6a 00 03 4f 66 66 02 00 6a 00 0c 53 69 6e 67 6c 65 20 54 72 61 63 6b

BT1 controller:
  receive value text response
```

This fills in the text lookup step after supported value discovery and before
user-facing player setting presentation.

Boundary: this is not complete player-settings text support.  It currently
covers two repeat value labels only, not all repeat/shuffle/scan/equalizer
texts, charset edge cases, localization, setting change notifications,
rejected/invalid value paths, or full `bluetoothd` AVRCP player policy.

BlueZ A2DP AVRCP player-settings-notification update
----------------------------------------------------

The A2DP AVRCP side now has a focused player application setting changed
notification hwsim gate:

```text
PASS bluez-a2dp-avrcp-player-settings-notification
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-player-settings-notification/run-results.json
```

The gate drives
`bluezaudio avrcp-control controller-player-settings-notify` on BT1 and
`bluezaudio avrcp-control target-player-settings-notify-respond` on BT2.  It
covers AVRCP RegisterNotification event `0x08` over AVCTP control PSM `0x0017`
/ CID `0x0042`, with BlueZ-shaped `org.bluez.MediaPlayer1.PropertiesChanged`
evidence:

```text
BT1 controller:
  register player application setting changed notification
  -> AVRCP PDU 0x31 event 0x08
  -> payload 00 11 0e 00 31 00 00 05 08 00 00 00

BT2 target:
  receive notification registration
  -> interim repeat=single-track shuffle=all-tracks response
  -> payload 02 11 0e 0f 31 00 00 06 08 02 02 02 03 02

BT1 controller:
  receive player settings changed interim notification
```

This complements the player settings discovery/read/write gates with an
event-path proof for settings changes.

Boundary: this is not complete changed-notification ownership.  It currently
covers one interim response, not re-registration after changed events,
unsolicited changed delivery after an actual set operation, multiple subscriber
handling, timeout/cancel behavior, or full `bluetoothd` AVRCP player policy.

BlueZ A2DP AVRCP player-settings-error update
---------------------------------------------

The A2DP AVRCP side now has a focused invalid-attribute rejection hwsim gate:

```text
PASS bluez-a2dp-avrcp-player-settings-error
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-player-settings-error/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-player-settings-error` on
BT1 and `bluezaudio avrcp-control target-player-settings-error-respond` on
BT2.  It covers an AVRCP GetCurrentPlayerApplicationSettingValue request for an
invalid attribute over AVCTP control PSM `0x0017` / CID `0x0042`, with
BlueZ-shaped `org.bluez.MediaPlayer1.InvalidArguments` evidence:

```text
BT1 controller:
  query invalid setting attribute 0x7f
  -> AVRCP PDU 0x13
  -> payload 00 11 0e 00 13 00 00 02 01 7f

BT2 target:
  receive invalid attribute request
  -> rejected response type 0x0a, status invalid-parameter
  -> payload 02 11 0e 0a 13 00 00 01 01

BT1 controller:
  receive rejected response
```

This adds a negative-path proof to the player-settings coverage, so the hwsim
matrix no longer validates only successful setting operations.

Boundary: this is not a complete AVRCP error matrix.  It covers one invalid
attribute rejection only, not all invalid PDU/state combinations, malformed
length handling, unsupported attributes/values across every player setting
command, or full `bluetoothd` error propagation.

BlueZ A2DP AVRCP addressed-player update
----------------------------------------

The A2DP AVRCP side now has a focused SetAddressedPlayer hwsim gate:

```text
PASS bluez-a2dp-avrcp-addressed-player
  FeatherCore/build/bt-hwsim-usecases-a2dp-batch-final/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-addressed-player` on BT1
and `bluezaudio avrcp-control target-addressed-player-respond` on BT2.  It
covers AVRCP SetAddressedPlayer over AVCTP control PSM `0x0017` / CID `0x0042`,
with BlueZ-shaped `org.bluez.MediaPlayer1.AddressedPlayer` evidence:

```text
BT1 controller:
  set addressed player id 0x0001
  -> AVRCP PDU 0x60
  -> payload 00 11 0e 00 60 00 00 02 00 01

BT2 target:
  receive addressed-player selection
  -> success response
  -> payload 02 11 0e 09 60 00 00 01 04

BT1 controller:
  receive addressed-player success response
```

This closes the current focused addressed-player gap in the A2DP AVRCP harness.

Boundary: this is not full addressed-player ownership.  It covers one successful
player selection, not invalid player id rejection, available-player list
management, addressed-player changed notification, browsing-player ownership,
or complete `bluetoothd` AVRCP player/session policy.

A2DP batch validation update
----------------------------

The current BT1/BT2 binaries were rebuilt and a single A2DP batch run passed:

```text
PASS A2DP batch
  FeatherCore/build/logs/build-bt1-a2dp-batch-final.log
  FeatherCore/build/logs/build-bt2-a2dp-batch-final.log
  FeatherCore/build/logs/run-a2dp-batch-final.log
  FeatherCore/build/bt-hwsim-usecases-a2dp-batch-final/run-results.json
```

The batch validates the main BlueZ-shaped A2DP/SBC/MediaTransport paths plus
the AVRCP focused gates in one run: SBC transport, extended signaling, abort,
reconnect, bidirectional MediaTransport, bidirectional teardown, play, browsing,
notification, absolute volume, metadata, player setting discovery, player
setting values, value text, player setting changed notification, invalid
attribute rejection, addressed player, player setting read, and player setting
set.

Boundary: this batch is stronger than the earlier one-gate-at-a-time evidence,
but it is still focused hwsim/profile-adapter coverage.  Full completion still
requires real `bluetoothd` audio/AVRCP plugin mainloop and ownership, real
D-Bus MediaEndpoint/MediaTransport/MediaPlayer object lifetimes, complete
upstream AVDTP/L2CAP session ownership, controller-driven L2CAP setup/error
policy, and broader codec/policy coverage.

BlueZ A2DP AVRCP player-settings update
---------------------------------------

The A2DP AVRCP side now has a focused PlayerApplicationSettings hwsim gate:

```text
PASS bluez-a2dp-avrcp-player-settings
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-player-settings/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-player-settings` on BT1
and `bluezaudio avrcp-control target-player-settings-respond` on BT2.  It
covers AVRCP GetCurrentPlayerApplicationSettingValue over AVCTP control PSM
`0x0017` / CID `0x0042`, with BlueZ-shaped `org.bluez.MediaPlayer1.Settings`
evidence:

```text
BT1 controller:
  MediaPlayer1.Settings query for repeat/shuffle
  -> AVRCP PDU 0x13
  -> payload 00 11 0e 00 13 00 00 03 02 02 03

BT2 target:
  receive repeat/shuffle query
  -> repeat=off shuffle=off response
  -> payload 02 11 0e 09 13 00 00 05 02 02 01 03 01

BT1 controller:
  receive accepted repeat=off shuffle=off response
```

This extends the current AVRCP coverage from metadata query to player
application settings, another common BlueZ media player control surface.

Boundary: this is not complete player-settings support.  It does not yet cover
setting enumeration, value text lookup, setting change notification, invalid
attribute/error paths, persistence, or full `bluetoothd` AVRCP player policy.

BlueZ A2DP AVRCP set-player-settings update
-------------------------------------------

The A2DP AVRCP side now has a focused SetPlayerApplicationSettingValue hwsim
gate:

```text
PASS bluez-a2dp-avrcp-player-settings-set
  FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-avrcp-player-settings-set/run-results.json
```

The gate drives `bluezaudio avrcp-control controller-player-settings-set` on
BT1 and `bluezaudio avrcp-control target-player-settings-set-respond` on BT2.
It covers AVRCP SetPlayerApplicationSettingValue over AVCTP control PSM
`0x0017` / CID `0x0042`, with BlueZ-shaped
`org.bluez.MediaPlayer1.Settings` evidence:

```text
BT1 controller:
  set MediaPlayer1.Settings repeat=single-track shuffle=all-tracks
  -> AVRCP PDU 0x14
  -> payload 00 11 0e 00 14 00 00 05 02 02 02 03 02

BT2 target:
  receive repeat/shuffle set request
  -> accepted response
  -> payload 02 11 0e 09 14 00 00 00

BT1 controller:
  receive accepted response
```

This complements the read-side `bluez-a2dp-avrcp-player-settings` gate by
validating the write/control direction for player application settings.

Boundary: this is not complete player-settings ownership.  It still does not
cover setting enumeration, supported value discovery, value text lookup, setting
change notifications, rejected/invalid attribute paths, persistence, or full
`bluetoothd` AVRCP player policy.

`bluezmgmt error-path` validates Linux mgmt error propagation over the public
control socket.  It sends an invalid `MGMT_OP_SET_IO_CAPABILITY` payload and
expects `MGMT_EV_CMD_STATUS` with `MGMT_STATUS_INVALID_PARAMS`.  The hwsim gate
is `bluez-mgmt-error-path`.

bluezdaemon device-policy
-------------------------

`bluezdaemon device-policy` validates a BlueZ daemon/device-shaped lifecycle
over a long-lived `AF_BLUETOOTH/SOCK_RAW/BTPROTO_HCI` control socket:

```text
READ_VERSION / READ_COMMANDS / READ_INDEX_LIST / READ_INFO
SET_POWERED / SET_CONNECTABLE / SET_DISCOVERABLE / SET_BONDABLE
SET_LE / SET_ADVERTISING
ADD_DEVICE -> GET_DEVICE_FLAGS -> SET_DEVICE_FLAGS -> GET_DEVICE_FLAGS
BLOCK_DEVICE -> UNBLOCK_DEVICE
SET_IO_CAPABILITY -> PAIR_DEVICE -> DEVICE_CONNECTED
UNPAIR_DEVICE -> REMOVE_DEVICE
```

The hwsim gate is `bluez-daemon-device-policy` and currently passes.  The path
enters through upstream Linux `hci_sock.c`; device-policy opcodes that are not
yet safe in upstream `mgmt.c` under the NuttX compat layer are completed by a
small Feather fast-path using upstream `mgmt_cmd_complete()` event formatting.

This is daemon-shaped ABI coverage, not a full `bluetoothd` port: DBus
adapter/device objects, persistent storage, agent/profile ownership, and the
complete upstream `mgmt.c` state machine remain future work.

bluezdaemon discovery-peer
--------------------------

`bluezdaemon discovery-peer` validates the next daemon-shaped discovery step:
one sim role advertises on the hwsim medium, while another role keeps a
long-lived BlueZ-style mgmt control fd open, sends `MGMT_OP_START_DISCOVERY`,
then receives both `MGMT_EV_DISCOVERING` and `MGMT_EV_DEVICE_FOUND` from that
same fd.

Current hwsim gate:

```text
bluez-daemon-discovery-peer
```

Current result:

```text
PASS bluez-daemon-discovery-peer
  PASS ble2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-discovery-peer-public-hci-upstream-socket-final/bluez-daemon-discovery-peer.ble2.log
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-discovery-peer-public-hci-upstream-socket-final/bluez-daemon-discovery-peer.ble1.log
```

Key evidence:

```text
bluez-daemon: event-loop label=discovery-start ... event=0x0013 ... opcode=0x0023 status=0x00
bluez-daemon: event-only label=device-found ret=36 event=0x0012 index=0x0000 len=30 expect-event=0x0012
bluez-daemon: discovery-peer complete
upstream-af: ... hci-mgmt-socket-recv=0 ...
```

This proves the current BlueZ apps-side mgmt shape can consume a peer discovery
event produced through the sim Bluetooth hwsim ADV medium.

The public BlueZ fd now enters a per-fd upstream HCI socket created through
`linux_bt_sockif.c` -> `linux_bt_upstream_hci_socket_open()` -> upstream
`hci_sock.c`.  The old global mgmt probe receive counter remains zero in this
gate, which is intentional evidence that the BlueZ-facing fd is no longer using
the `g_hci_mgmt_probe_socket` recv path.

Boundary: simple adapter settings and hwsim peer discovery still use small
Feather fast-paths inside upstream `hci_sock.c` to avoid the not-yet-complete
`mgmt.c`/`hci_sync.c` asynchronous controller path.  The full upstream Linux
`mgmt.c` discovery ownership, complete control-socket broadcast policy,
`bluetoothd` adapter object, D-Bus device object creation, and persistent
discovery policy are not complete yet.

bluezbneptest native BNEP fd handoff
------------------------------------

`bluezbneptest fd-handoff` now validates the BlueZ-side network-profile fd
handoff shape against imported Linux BNEP:

```text
socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_L2CAP)
  -> bind/connect
socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_BNEP)
  -> ioctl(BNEPGETSUPPFEAT)
  -> ioctl(BNEPCONNADD, connected_l2cap_fd)
  -> ioctl(BNEPGETCONNLIST / BNEPGETCONNINFO)
  -> ioctl(BNEPCONNDEL)
```

Current hwsim gate:

```text
bluez-bneptest-fd-handoff
```

Current result:

```text
PASS bluez-bneptest-fd-handoff
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-current/bluez-bneptest-fd-handoff.bt1.log
```

The same imported BNEP session/netdev path also passes the two-role PAN ping
gate through `btbneptest pan-up`:

```text
PASS bneptest-ping
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bneptest-ping-current/bneptest-ping.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bneptest-ping-current/bneptest-ping.bt2.log
```

Native-path evidence in the logs:

```text
bnep-staging-active=0
bnep-native-netdev-register=1
bnep-native-netdev-xmit=10
bnep-native-tx-frame=10
bnep-native-l2cap-rx=11
bnep-native-rx-frame=22
bnep-native-netif-rx=22
bnep-native-session-create=1
bnep-native-session-stop=1
```

This means the current PAN data path is no longer the early staging session
table.  It is:

```text
BlueZ-style fd handoff -> imported Linux BNEP ioctl/session/netdev
-> NuttX btn0 lower-half bridge -> hwsim ACL medium -> peer BNEP RX
```

The same native BNEP path also passes the current TCP/UDP throughput matrix:

```text
PASS bnep-iperf-tcp
PASS bnep-iperf-tcp-reverse
PASS bnep-iperf-udp
PASS bnep-iperf-udp-reverse
```

Current run manifest:

```text
FeatherCore/build/bt-hwsim-usecases-bnep-iperf-current/run-results.json
```

These gates require `btbneptest` fd handoff, `btn0`, iperf client/server
`Mbits/sec` output, `iperf exit` on the client side, and non-zero native BNEP
data-path counters such as:

```text
bnep-native-netdev-xmit
bnep-native-tx-frame-ok
bnep-native-l2cap-delivered
bnep-native-rx-frame-ok
bnep-native-netif-rx
```

Remaining BlueZ-side work is to replace the small `bluezbneptest` adapter with
`bluetoothd` network profile / DBus driven setup while preserving this same
kernel-facing BNEP ABI.

## BlueZ mgmt lifecycle gate

`bluezmgmt lifecycle` validates a BlueZ btmgmt-style lifecycle over the public
HCI management control socket:

```text
SET_IO_CAPABILITY -> PAIR_DEVICE -> DEVICE_CONNECTED
GET_CONN_INFO -> DISCONNECT -> DEVICE_DISCONNECTED
```

The hwsim gate is `bluez-mgmt-lifecycle`.  Current validation artifacts:

```text
FeatherCore/build/logs/build-ble1-bluez-mgmt-lifecycle.log
FeatherCore/build/logs/run-bluez-mgmt-lifecycle-current.log
FeatherCore/build/bt-hwsim-usecases-bluez-mgmt-lifecycle-current/run-results.json
```

This proves the apps-side BlueZ-style mgmt fd can drive pair/get-info/disconnect
through the NuttX Linux Bluetooth socket ABI.  The connected peer remains a sim
hwsim synthetic lifecycle state; full upstream Linux `mgmt.c`, `hci_conn`,
`hci_event`, and `smp.c` ownership is still a later alignment step.

## BlueZ-style BNEP reconnect/stress gates

`btbneptest` now covers repeated PAN bring-up/tear-down over the public
Bluetooth socket ABI.  The app opens an L2CAP socket, connects it, opens a BNEP
socket, and passes the connected L2CAP fd through BNEP `connadd` so the kernel
side creates `btn0`.

Current hwsim gates:

```text
PASS bneptest-reconnect
PASS bneptest-reconnect-stress
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-bneptest-reconnect-current.log
FeatherCore/build/logs/build-bt2-bneptest-reconnect-current.log
FeatherCore/build/logs/run-bneptest-reconnect-current.log
FeatherCore/build/logs/run-bneptest-reconnect-stress-current.log
FeatherCore/build/bt-hwsim-usecases-bneptest-reconnect-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bneptest-reconnect-stress-current/run-results.json
```

Key evidence:

```text
btbneptest: l2cap-socket fd=3
btbneptest: l2cap-bind ret=0
btbneptest: l2cap-connect ret=0
btbneptest: bnep-socket fd=4
btbneptest: bnep-connadd-fd ret=0 device=btn0
ping: 2 packets transmitted, 2 received, 0% packet loss
bnep-native-session-create/start/stop/terminate match reconnect count
bnep-staging-active=0 bnep-native-active=0 after teardown
```

This is still a BlueZ-style test tool flow, not the full BlueZ network profile
daemon.  It proves the fd handoff and BNEP netdev lifecycle expected by BlueZ's
network profile path, while the remaining alignment target is full upstream
Linux `bnep/core.c`, `sock.c`, and `netdev.c` session/thread ownership.

## Audio probe gates

The current sim image passes the audio-oriented hwsim probes:

```text
PASS a2dp
PASS a2dp-extended
PASS a2dp-media
PASS le-audio
```

Artifacts:

```text
FeatherCore/build/logs/run-a2dp-current.log
FeatherCore/build/logs/run-a2dp-extended-current.log
FeatherCore/build/logs/run-a2dp-media-current.log
FeatherCore/build/logs/run-le-audio-current.log
FeatherCore/build/bt-hwsim-usecases-a2dp-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-a2dp-extended-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-a2dp-media-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-le-audio-current/run-results.json
```

The A2DP probes validate synthetic AVDTP signaling and media payload movement over
L2CAP/VHCI/hwsim.  The LE Audio probe validates ISO socket bind/connect/send/recv
with a synthetic LC3-like frame.  These are useful BlueZ-facing protocol-shape
checks, but they are not yet full BlueZ daemon profile flows.  Remaining work is
to attach `bluetoothd` profile code, D-Bus media endpoint registration, AVDTP
transport acquire/release, and LE Audio BAP/CAP/PACS/ASCS semantics to the same
NuttX Bluetooth socket and hwsim paths.

## BlueZ-facing ABI matrix

Current verified BlueZ-derived app gates over the public NuttX Bluetooth socket
ABI:

```text
PASS bluez-mgmt-control
PASS bluez-mgmt-pair-noio
PASS bluez-mgmt-user-confirm
PASS bluez-mgmt-user-confirm-neg
PASS bluez-mgmt-passkey
PASS bluez-mgmt-passkey-neg
PASS bluez-mgmt-cancel-pair
PASS bluez-mgmt-cancel-pair-pending
PASS bluez-mgmt-lifecycle
PASS bluez-mgmt-error-path
PASS bluez-btmon-monitor
PASS bluez-hciioctl-basic
PASS bluez-hciraw-command
```

These apps exercise `AF_BLUETOOTH` HCI control, monitor, raw socket, and HCI ioctl
paths instead of calling internal `btctl upstream` probe APIs directly.

The latest fix restores Linux mgmt invalid-parameter behavior for
`MGMT_OP_SET_IO_CAPABILITY`: `io_capability=0xff` now returns
`MGMT_STATUS_INVALID_PARAMS(0x0d)` through the same BlueZ-style control fd.
`bluez-mgmt-cancel-pair` also verifies `MGMT_OP_CANCEL_PAIR_DEVICE` over that
public control fd.  Since the current shim has no pending pairing window, the
expected Linux-visible result is `MGMT_STATUS_INVALID_PARAMS(0x0d)`; this is an
ABI/error-path gate and not a claim that upstream pending-pair cancellation is
complete.
`bluez-mgmt-cancel-pair-pending` now validates the pending cancel event order:
`MGMT_EV_USER_CONFIRM_REQUEST`, `MGMT_OP_PAIR_DEVICE` completed with
`MGMT_STATUS_CANCELLED(0x10)`, and `MGMT_OP_CANCEL_PAIR_DEVICE` completed with
`MGMT_STATUS_SUCCESS(0x00)`.
`bluez-mgmt-user-confirm` validates the positive DisplayYesNo path:
`MGMT_EV_USER_CONFIRM_REQUEST`, `MGMT_OP_USER_CONFIRM_REPLY` completed with
`MGMT_STATUS_SUCCESS(0x00)`, `MGMT_EV_NEW_LONG_TERM_KEY`, and final
`MGMT_OP_PAIR_DEVICE` success.
`bluez-mgmt-user-confirm-neg` validates the DisplayYesNo rejection path:
`MGMT_EV_USER_CONFIRM_REQUEST`, `MGMT_OP_USER_CONFIRM_NEG_REPLY` accepted with
`MGMT_STATUS_SUCCESS(0x00)`, and final `MGMT_OP_PAIR_DEVICE` failed with
`MGMT_STATUS_FAILED(0x03)`.
`bluez-mgmt-passkey` validates the KeyboardOnly passkey path:
`MGMT_EV_USER_PASSKEY_REQUEST`, exact 11-byte `MGMT_OP_USER_PASSKEY_REPLY`,
`MGMT_EV_NEW_LONG_TERM_KEY`, and final `MGMT_OP_PAIR_DEVICE` success.
`bluez-mgmt-passkey-neg` validates the KeyboardOnly passkey rejection path:
`MGMT_EV_USER_PASSKEY_REQUEST`, `MGMT_OP_USER_PASSKEY_NEG_REPLY` accepted with
`MGMT_STATUS_SUCCESS(0x00)`, and final `MGMT_OP_PAIR_DEVICE` failed with
`MGMT_STATUS_FAILED(0x03)`.

Artifacts:

```text
FeatherCore/build/logs/build-ble1-bluez-mgmt-cancel-pair.log
FeatherCore/build/logs/build-ble1-bluez-mgmt-cancel-pair-pending.log
FeatherCore/build/logs/build-ble1-bluez-mgmt-user-confirm.log
FeatherCore/build/logs/build-ble1-bluez-mgmt-user-confirm-neg.log
FeatherCore/build/logs/build-ble1-bluez-mgmt-passkey.log
FeatherCore/build/logs/build-ble1-bluez-mgmt-passkey-neg.log
FeatherCore/build/logs/build-ble1-bluez-mgmt-error-path-current.log
FeatherCore/build/logs/run-bluez-mgmt-control-current.log
FeatherCore/build/logs/run-bluez-mgmt-pair-noio-current.log
FeatherCore/build/logs/run-bluez-mgmt-user-confirm.log
FeatherCore/build/logs/run-bluez-mgmt-user-confirm-neg.log
FeatherCore/build/logs/run-bluez-mgmt-passkey.log
FeatherCore/build/logs/run-bluez-mgmt-passkey-neg.log
FeatherCore/build/logs/run-bluez-mgmt-cancel-pair.log
FeatherCore/build/logs/run-bluez-mgmt-cancel-pair-pending.log
FeatherCore/build/logs/run-bluez-mgmt-lifecycle-current.log
FeatherCore/build/logs/run-bluez-mgmt-error-path-current.log
FeatherCore/build/bt-hwsim-usecases-bluez-mgmt-cancel-pair/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-mgmt-cancel-pair-pending/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-mgmt-user-confirm/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-mgmt-user-confirm-neg/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-mgmt-passkey/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-mgmt-passkey-neg/run-results.json
FeatherCore/build/logs/run-bluez-btmon-monitor-current.log
FeatherCore/build/logs/run-bluez-hciioctl-basic-current.log
FeatherCore/build/logs/run-bluez-hciraw-command-current.log
```

This is a working BlueZ-apps-to-NuttX-socket-to-Linux-BT/VHCI/hwsim path.  It is
not yet the full upstream BlueZ daemon/profile stack; the remaining work is to
move from focused tools to `bluetoothd`, D-Bus, and profile plugins while also
reducing kernel-side staging in favor of upstream Linux Bluetooth modules.

bluezdaemon pairing-matrix
--------------------------

`bluezdaemon pairing-matrix` keeps one BlueZ daemon-shaped mgmt control fd open
and validates the agent/pairing decisions that `bluetoothd` would normally
drive through `src/adapter.c` and `src/agent.c`:

```text
READ_VERSION / READ_COMMANDS / READ_INDEX_LIST / READ_INFO
SET_POWERED / SET_BONDABLE / SET_LE
DisplayYesNo pair -> USER_CONFIRM_REQUEST -> USER_CONFIRM_REPLY -> New LTK
DisplayYesNo pair -> USER_CONFIRM_REQUEST -> USER_CONFIRM_NEG_REPLY
KeyboardOnly pair -> USER_PASSKEY_REQUEST -> USER_PASSKEY_REPLY -> New LTK
KeyboardOnly pair -> USER_PASSKEY_REQUEST -> USER_PASSKEY_NEG_REPLY
DisplayYesNo pair -> USER_CONFIRM_REQUEST -> CANCEL_PAIR_DEVICE
```

The hwsim gate is `bluez-daemon-pairing-matrix`.  It validates Linux mgmt
opcode payload sizes, reply status, failed/rejected pair status, cancelled pair
status, and event ordering over the public BlueZ-facing control socket.  This
is still a `CONFIG_SIM_BTHWSIM` public ABI gate, not proof that the complete
upstream SMP/security workqueue or full `bluetoothd` D-Bus agent ownership has
been ported.

## Daemon-shaped mgmt smoke

`bluezdaemon` is the first daemon-shaped BlueZ adapter in this port.  It is not
the complete upstream `bluetoothd`, but it keeps a long-lived mgmt control fd and
drives the controller lifecycle in the same broad shape as BlueZ
`src/main.c`/`src/adapter.c`.

Current verified gate:

```text
PASS bluez-daemon-smoke
```

The smoke sequence covers:

```text
socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)
bind(HCI_CHANNEL_CONTROL)
READ_VERSION / READ_COMMANDS / READ_INDEX_LIST / READ_INFO
SET_POWERED / SET_CONNECTABLE / SET_DISCOVERABLE / SET_BONDABLE
SET_LE / SET_ADVERTISING / SET_BREDR
START_DISCOVERY / STOP_DISCOVERY
SET_IO_CAPABILITY / PAIR_DEVICE / GET_CONN_INFO / DISCONNECT
```

Artifacts:

```text
FeatherCore/build/logs/build-ble1-bluez-daemon-smoke-current.log
FeatherCore/build/logs/run-bluez-daemon-smoke-current.log
FeatherCore/build/bt-hwsim-usecases-bluez-daemon-smoke-current/run-results.json
```

Representative evidence:

```text
bluez-daemon: source=third/bluez/src/main.c+src/adapter.c style mode=smoke
bluez-daemon: hci-bind-control ret=0 errno=0
bluez-daemon: event-loop label=discovery-start ... event=0x0013
bluez-daemon: event-loop label=pair-connected ... event=0x000b
bluez-daemon: event-loop label=disconnect-complete ... event=0x000c
bluez-daemon: smoke complete
upstream-af: ... hci-mgmt-socket-cmd=17 ... hci-mgmt-socket-recv=28 ...
```

This gives the BlueZ side a stable daemon-style foothold for the next migration
step: attaching upstream `bluetoothd` mainloop/D-Bus/adapter/profile code to the
same NuttX Bluetooth socket ABI, then retiring the remaining tool-only probes.

## Daemon-shaped reconnect stress

`bluezdaemon reconnect-stress 3` keeps one mgmt control fd open and repeats the
pair/get-connection-info/disconnect lifecycle three times.

Current verified gate:

```text
PASS bluez-daemon-reconnect-stress
```

The validator requires three successful `Pair Device`, `Get Conn Info`, and
`Disconnect` command sequences, three `Device Connected` events, three
`Device Disconnected` events, and final `conn-hash acl=0 sco=0 le=0`.

Artifacts:

```text
FeatherCore/build/logs/build-ble1-bluez-daemon-reconnect-stress-current.log
FeatherCore/build/logs/run-bluez-daemon-reconnect-stress-current.log
FeatherCore/build/bt-hwsim-usecases-bluez-daemon-reconnect-stress-current/run-results.json
```

This is still a bounded daemon-shaped hwsim gate, but it gives the future
`bluetoothd` adapter/device port a stronger mgmt lifecycle baseline than a
single smoke run.

## BlueZ audio adapter gates

`bluezaudio` is the first audio profile-shaped adapter under the BlueZ apps
subtree.  It moves the synthetic A2DP media and LE Audio ISO payload gates from
the generic `btaudio` command toward BlueZ profile entry points.

Current verified gates:

```text
PASS bluez-a2dp-signaling
PASS bluez-a2dp-signaling-native
PASS bluez-a2dp-media
PASS bluez-a2dp-profile
PASS bluez-a2dp-transport
PASS bluez-a2dp-endpoint-transport
PASS bluez-a2dp-sbc-codec-transport
PASS bluez-a2dp-transport-reconnect
PASS bluez-a2dp-transport-bidir
PASS bluez-a2dp-transport-bidir-teardown
PASS bluez-le-audio
PASS bluez-le-audio-profile
PASS bluez-le-audio-broadcast-restart
PASS bluez-le-audio-transport
PASS bluez-le-audio-transport-reconnect
PASS bluez-le-audio-transport-bidir
PASS bluez-le-audio-transport-bidir-disable
PASS bluez-le-audio-transport-bidir-qos-update
PASS bluez-le-audio-transport-bidir-qos-reject
PASS bluez-le-audio-transport-bidir-qos-cancel
PASS bluez-le-audio-transport-bidir-release-reconfig
PASS bluez-le-audio-transport-bidir-reconnect
```

The A2DP signaling gate maps to the BlueZ AVDTP profile shape:

```text
bluezaudio a2dp-signal listen 0x0052
bluezaudio a2dp-signal auto-rsp-loop 1 7
bluezaudio a2dp-signal discover 2
bluezaudio a2dp-signal getcap 2
bluezaudio a2dp-signal setconfig 2
bluezaudio a2dp-signal open 2
bluezaudio a2dp-signal start 2
bluezaudio a2dp-signal suspend 2
bluezaudio a2dp-signal close-stream 2
```

Evidence includes:

```text
bluez-audio: source=third/bluez/profiles/audio/avdtp.c style profile=a2dp-signaling
upstream-l2cap-send: psm=0x0019 cid=0x0040 handle=0x0052
bluez-audio: a2dp auto-rsp signal=0x01 ... state=IDLE->DISCOVERED
bluez-audio: a2dp auto-rsp signal=0x07 ... state=OPEN->STREAMING
bluez-audio: a2dp auto-rsp-loop complete count=7
```

The native AVDTP signaling gate keeps the same BlueZ AVDTP request/response
shape, but enables imported Linux L2CAP receive for the signaling CID:

```text
bluezaudio a2dp-signal listen-native 0x0052
bluezaudio a2dp-signal auto-rsp-loop 1 7
bluezaudio a2dp-signal discover/getcap/setconfig/open/start/suspend/close-stream 2
```

Evidence includes:

```text
PASS bluez-a2dp-signaling-native
upstream-l2cap-native-control: enabled=1
bluez-audio: a2dp auto-rsp-loop complete count=7
l2cap-socket-recv=7
l2cap-socket-native-recv=7
l2cap-socket-native-attach-fail=0
l2cap-socket-native-recv-fail=0
```

The A2DP media gate maps to the BlueZ audio source/sink/transport shape:

```text
bluezaudio a2dp-source start 2
bluezaudio a2dp-sink start
bluezaudio a2dp-sink read
bluezaudio a2dp-sink stop
```

Evidence includes:

```text
bluez-audio: source=third/bluez/profiles/audio/a2dp.c style profile=a2dp-source
upstream-l2cap-send: psm=0x0019 cid=0x0041 handle=0x0052 payload-len=24
bluez-audio: source=third/bluez/profiles/audio/transport.c style profile=a2dp-sink command=read
upstream-l2cap-recv: recv-ret=24 ... payload=41 32 44 50 3a 53 42 43 ...
bluez-audio: a2dp sink media payload received
```

The profile-level A2DP gate combines signaling and media in one BT1/BT2
lifecycle:

```text
bluezaudio a2dp-signal discover/getcap/setconfig/open/start/suspend/close-stream 2
bluezaudio a2dp-source start 2
bluezaudio a2dp-sink read
```

Evidence includes:

```text
PASS bluez-a2dp-profile
bluez-audio: a2dp auto-rsp-loop complete count=7
upstream-l2cap-send: psm=0x0019 cid=0x0041 handle=0x0052 payload-len=24
upstream-l2cap-recv: recv-ret=24 ... payload=41 32 44 50 3a 53 42 43 ...
```

The A2DP transport gate adds the BlueZ `profiles/audio/transport.c`
MediaTransport Acquire/Release shape to the BR/EDR AVDTP media path:

```text
bluezaudio a2dp-signal discover/getcap/setconfig/open/start 2
bluezaudio media-transport a2dp-source-acquire-write-release 2
bluezaudio media-transport a2dp-sink-acquire-read-release 1
```

Evidence includes:

```text
PASS bluez-a2dp-transport
bluez-audio: source=third/bluez/profiles/audio/transport.c style profile=media-transport
bluez-audio: media transport acquire fd=l2cap ... read-mtu=672 write-mtu=672
upstream-l2cap-write: payload-len=24 send-ret=24
upstream-l2cap-recv: recv-ret=24 ... payload=41 32 44 50 3a 53 42 43 ...
bluez-audio: media transport release complete role=sink
l2cap-socket-native-recv=1
```

The endpoint transport gate adds the BlueZ `MediaEndpoint1` / codec selection
shape in front of the same AVDTP + MediaTransport data path:

```text
PASS bluez-a2dp-endpoint-transport
source endpoint: register -> SBC capabilities -> select config -> set configuration
sink endpoint: register -> SBC capabilities -> select config -> set configuration
AVDTP: discover/getcap/setconfig/open/start
transport: acquire L2CAP fd -> write/read A2DP:SBC synthetic payload -> release
teardown: suspend/close-stream -> clear configuration -> unregister endpoint
```

Evidence includes:

```text
bluez-audio: media endpoint register endpoint=/org/bluez/hci0/dev_feather/sep/source0 uuid=0000110a-0000-1000-8000-00805f9b34fb codec=sbc codec-id=0x00
bluez-audio: media endpoint register endpoint=/org/bluez/hci0/dev_feather/sep/sink0 uuid=0000110b-0000-1000-8000-00805f9b34fb codec=sbc codec-id=0x00
bluez-audio: media endpoint capabilities media-type=audio codec=sbc caps=ff ff 02 35
bluez-audio: source=third/bluez/profiles/audio/a2dp.c select-config role=source sampling=44100 channels=2 channel-mode=joint-stereo block-length=16 subbands=8 allocation=loudness bitpool=2..53
bluez-audio: source=third/bluez/profiles/audio/media.c set-configuration ... configuration=21 15 02 35
bluez-audio: media transport read complete payload=A2DP:SBC:synthetic-frame
bluez-audio: media endpoint clear complete role=source
bluez-audio: media endpoint clear complete role=sink
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-bluez-a2dp-endpoint-transport.log
FeatherCore/build/logs/build-bt2-bluez-a2dp-endpoint-transport.log
FeatherCore/build/logs/run-bluez-a2dp-endpoint-transport.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-endpoint-transport/run-results.json
```

The SBC codec transport gate keeps the same MediaEndpoint + AVDTP +
MediaTransport flow, but replaces the old ASCII media probe with a binary
SBC-shaped frame and validates the sink-side frame parser:

```text
PASS bluez-a2dp-sbc-codec-transport
source: PCM s16le 44.1 kHz stereo -> SBC-shaped frame
selected-config: 21 15 02 35
frame: 9c bd 35 00 10 20 30 40 50 60 70 80 90 a0 b0 c0 d0 e0 f0 00
sink: recv-ret=20 -> syncword/config/frame length check -> PCM checksum
```

Evidence includes:

```text
bluez-audio: source=third/bluez/client/player.c preset=a2dp_src/sbc codec=sbc codec-id=0x00 caps=ff ff 02 40 preset-count=6
bluez-audio: source=third/bluez/client/player.c preset=a2dp_sink/sbc codec=sbc codec-id=0x00 caps=ff ff 02 40 preset-count=6
bluez-audio: source=third/bluez/gdbus/object.c dbus object-manager path=/ interface=org.freedesktop.DBus.ObjectManager method=GetManagedObjects ...
bluez-audio: source=third/bluez/gdbus/object.c dbus signal=InterfacesAdded path=/org/bluez/hci0/dev_feather interface=org.bluez.MediaControl1 ...
bluez-audio: source=third/bluez/gdbus/object.c dbus signal=InterfacesAdded path=/org/bluez/hci0/dev_feather/sep/source0 interface=org.bluez.MediaEndpoint1 ...
bluez-audio: source=third/bluez/gdbus/object.c dbus signal=InterfacesAdded path=/org/bluez/hci0/dev_feather/fd/source0 interface=org.bluez.MediaTransport1 ...
bluez-audio: source=third/bluez/profiles/audio/source.c sdp register service=AudioSource uuid=0000110a-0000-1000-8000-00805f9b34fb psm=0x0019 ...
bluez-audio: source=third/bluez/profiles/audio/sink.c sdp register service=AudioSink uuid=0000110b-0000-1000-8000-00805f9b34fb psm=0x0019 ...
bluez-audio: source=third/bluez/profiles/audio/avctp.c control register psm=0x0017 browsing-psm=0x001b ...
bluez-audio: source=third/bluez/profiles/audio/avrcp.c sdp register controller uuid=0000110e-0000-1000-8000-00805f9b34fb target uuid=0000110c-0000-1000-8000-00805f9b34fb ...
bluez-audio: source=third/bluez/profiles/audio/media.c dbus owner=:1.feather adapter=/org/bluez/hci0 interface=org.bluez.Media1 method=RegisterEndpoint ...
bluez-audio: source=third/bluez/client/player.c dbus provider=bluezaudio interface=org.bluez.MediaEndpoint1 methods=SelectConfiguration,SetConfiguration,ClearConfiguration,Release
bluez-audio: source=third/bluez/profiles/audio/transport.c dbus export interface=org.bluez.MediaTransport1 ...
bluez-audio: codec-backend profile=a2dp codec=sbc backend=libsbc status=source-built required-source=third/sbc-2.0
bluez-audio: source=third/bluez/profiles/audio/a2dp-codecs.h selected-config=21 15 02 35 codec-id=0x00
bluez-audio: source=third/bluez/profiles/audio/a2dp-codecs.h sbc encode syncword=0x9c frame-header=bd 20 b9 samplerate=44100 channels=2 channel-mode=joint-stereo blocks=16 subbands=8 allocation=loudness bitpool=32 frame-len=77
upstream-l2cap-write: payload-len=77 send-ret=77
upstream-l2cap-recv: recv-ret=77 flags=0x0 payload=9c bd 20 b9 ...
bluez-audio: source=third/bluez/profiles/audio/transport.c dbus method=Acquire interface=org.bluez.MediaTransport1 ...
bluez-audio: source=third/bluez/profiles/audio/a2dp-codecs.h sbc decode syncword=0x9c frame-header=bd 20 b9 samplerate=44100 channels=2 channel-mode=joint-stereo blocks=16 subbands=8 allocation=loudness bitpool=32 frame-len=77
bluez-audio: source=third/sbc-2.0/sbc/sbc.c sbc_decode pcm-bytes=512 codesize=512 checksum=0x...
bluez-audio: source=third/bluez/profiles/audio/media.c pcm output format=s16le rate=44100 channels=2 pcm-bytes=512 checksum=0x...
bluez-audio: source=third/bluez/profiles/audio/media.c dbus method=ClearConfiguration ...
bluez-audio: source=third/bluez/profiles/audio/transport.c dbus unexport interface=org.bluez.MediaTransport1 ...
bluez-audio: source=third/bluez/gdbus/object.c dbus signal=InterfacesRemoved path=/org/bluez/hci0/dev_feather/fd/source0 interface=org.bluez.MediaTransport1 ...
bluez-audio: source=third/bluez/gdbus/object.c dbus signal=InterfacesRemoved path=/org/bluez/hci0/dev_feather/sep/source0 interface=org.bluez.MediaEndpoint1 ...
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-a2dp-sbc-teardown.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-teardown/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-sbc-reconnect-teardown.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-reconnect-teardown/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-sbc-extended.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-extended/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-sbc-abort.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-abort/run-results.json
```

Boundary: this is now a source-built libsbc frame transport gate aligned with
BlueZ SBC capability/preset metadata from `client/player.c` and
`profiles/audio/a2dp-codecs.h`.  It keeps the AVDTP selected configuration
(`21 15 02 35`) separate from the generated SBC media frame header, so the sink
validates actual SBC header fields instead of treating AVDTP config bytes as
audio frame bytes.  `CONFIG_LINUX_BLUEZ_AUDIO_LIBSBC=y` builds
`third/sbc-2.0/sbc/sbc.c` and the required primitive sources into
`bluezaudio`, and the gate must report
`backend=libsbc status=source-built required-source=third/sbc-2.0`.  The sink
now calls `sbc_decode` from the same recompiled source backend and records
decoded PCM bytes/codesize/checksum.  The gate also records BlueZ-shaped
`Media1.RegisterEndpoint`, `MediaEndpoint1` provider methods,
SDP AudioSource/AudioSink registration, AVCTP/AVRCP control registration,
ObjectManager `GetManagedObjects` plus `InterfacesAdded`/`InterfacesRemoved`,
`MediaTransport1` export/Acquire, `ClearConfiguration`, and transport unexport
ownership.  The same gate now runs the BT2 AVDTP responder through
`listen-native` and requires clean native attach evidence:
`l2cap-socket-native-recv>=6`, `l2cap-socket-native-attach-fail=0`, and
`l2cap-socket-native-recv-fail=0`.  BT1/source control sends also include
explicit `native-ret` logging, and the gate requires at least five AVDTP
control sends with `bind-ret=0` and native send evidence.  After media
delivery, the sink reopens a native-control responder in `STREAMING` state and
must acknowledge `SUSPEND` as `STREAMING->OPEN` and `CLOSE` as `OPEN->IDLE`.
This still does not complete all of A2DP: the remaining boundary is running the
full `bluetoothd` main loop/object manager instead of this command-driven
profile adapter, plus full upstream L2CAP/AVDTP session ownership beyond the
current hwsim command-driven path.

The reconnect gate `bluez-a2dp-sbc-codec-reconnect` repeats the same path twice
in one BT1/BT2 hwsim session.  It requires two ObjectManager-visible endpoint
lifecycles, ten source AVDTP control sends with native evidence, two
source-built SBC frame writes, two sink-side `sbc_decode` passes, two endpoint
clears, two native `SUSPEND/CLOSE` teardown transitions, and BT2
`l2cap-socket-native-recv>=16` with zero native attach/recv failures.

The extended gate `bluez-a2dp-sbc-codec-extended` adds
`GetAllCapabilities`, `GetConfiguration`, `Reconfigure`, `DelayReport`, and
`SecurityControl` before `Open/Start`, then runs the same source-built SBC media
path and native `SUSPEND/CLOSE` teardown.  It requires 12 source AVDTP control
sends with native evidence, BT2 `auto-rsp-loop complete count=10`, one SBC
decode, teardown transitions, and BT2 `l2cap-socket-native-recv>=13` with zero
native attach/recv failures.

The abort gate `bluez-a2dp-sbc-codec-abort` validates the interrupted control
path `Discover -> GetCapabilities -> SetConfiguration -> Open -> Abort` while
keeping the same SBC endpoint and ObjectManager ownership evidence.  BT2 uses
native-control AVDTP receive, responds to `ABORT` as `OPEN->IDLE`, and both
sides clear the endpoint objects with zero native attach/recv failures.

The repeated A2DP transport gate runs the same MediaTransport-shaped fd
ownership twice in one hwsim session:

```text
PASS bluez-a2dp-transport-reconnect
round 1: acquire source fd -> write 24-byte SBC-like payload -> acquire sink fd -> read -> release
round 2: acquire source fd -> write 24-byte SBC-like payload -> acquire sink fd -> read -> release
l2cap-socket-recv=20
l2cap-socket-native-recv=2
l2cap-socket-native-attach-fail=0
l2cap-socket-native-recv-fail=0
```

The reconnect script handles AVDTP teardown separately from the next setup
round: after each media transfer it reopens the signaling listener and consumes
`Suspend`/`Close` with `auto-rsp-loop 1 2`, then starts the next
`Discover/GetCapabilities/SetConfiguration/Open/Start` responder.  This keeps
the repeated MediaTransport gate from accidentally mixing previous teardown
PDUs into the next setup lifecycle.

The bidirectional A2DP transport gate runs MediaTransport-shaped L2CAP media
in both directions in one BT1/BT2 hwsim session.  Both directions now require
the AVDTP responder to reach `START`:

```text
PASS bluez-a2dp-transport-bidir
direction 1: BT1 source -> BT2 sink, 24-byte A2DP:SBC synthetic payload
direction 2: BT2 source -> BT1 sink, 24-byte A2DP:SBC synthetic payload
BT1: bluez-audio: a2dp auto-rsp signal=0x07 ... state=OPEN->STREAMING
BT2: bluez-audio: a2dp auto-rsp signal=0x07 ... state=OPEN->STREAMING
BT1: l2cap-socket-bind=3 l2cap-socket-connect=1 l2cap-socket-listen=2 l2cap-socket-send=6 l2cap-socket-recv=10 l2cap-socket-native-recv=1
BT2: l2cap-socket-bind=3 l2cap-socket-connect=1 l2cap-socket-listen=2 l2cap-socket-send=6 l2cap-socket-recv=10 l2cap-socket-native-recv=1
l2cap-socket-native-attach-fail=0
l2cap-socket-native-recv-fail=0
```

The second direction originally exposed a test-script peer/handle mismatch:
BT1 responded to peer `1`, which selected `handle=0x0051` instead of the
shared BR/EDR `handle=0x0052`.  The gate now uses `auto-rsp-loop 2 5`, so the
reverse path keeps the responder on the correct peer/handle and validates
`START` before entering the sink media path.

The bidirectional teardown gate extends the same flow through suspend and close
in both directions:

```text
PASS bluez-a2dp-transport-bidir-teardown
BT1: non-command-skip old response, START OPEN->STREAMING, SUSPEND STREAMING->OPEN, CLOSE OPEN->IDLE
BT2: START OPEN->STREAMING, SUSPEND STREAMING->OPEN, CLOSE OPEN->IDLE
BT1: l2cap-socket-bind=4 l2cap-socket-connect=1 l2cap-socket-listen=3 l2cap-socket-send=8 l2cap-socket-recv=16 l2cap-socket-native-recv=1
BT2: l2cap-socket-bind=4 l2cap-socket-connect=1 l2cap-socket-listen=3 l2cap-socket-send=13 l2cap-socket-recv=14 l2cap-socket-native-recv=1
l2cap-socket-native-attach-fail=0
l2cap-socket-native-recv-fail=0
```

This gate required two BlueZ-adapter lifecycle fixes: `auto-rsp-loop` can seed
the AVDTP SEP state for teardown-only responder commands, and the responder now
ignores non-command AVDTP PDUs so stale responses do not consume request count.

The LE Audio gate maps to the BlueZ BAP broadcast source/sink shape:

```text
bluezaudio le-broadcast-source start 0 1
bluezaudio le-broadcast-sink sync 0 1
bluezaudio le-broadcast-sink start 0 1
bluezaudio le-broadcast-sink stop
```

Evidence includes:

```text
bluez-audio: source=third/bluez/profiles/audio/bap.c style profile=le-broadcast-source
upstream-iso-send: addr-type=0 handle=0x0101 payload-len=28
bluez-audio: source=third/bluez/profiles/audio/bap.c style profile=le-broadcast-sink command=start
upstream-iso-recv: recv-ret=28 ... payload=4c 45 2d 41 55 44 49 4f ...
bluez-audio: le broadcast sink iso payload received
```

The profile-level LE Audio gate combines BAP/PACS/ASCS/BASS-shaped control
evidence with the ISO payload path:

```text
bluezaudio le-bap-control source-announce 0 1
bluezaudio le-bap-control source-start 0 1
bluezaudio le-broadcast-source start 0 1
bluezaudio le-bap-control sink-discover 0 1
bluezaudio le-bap-control sink-config 0 1
bluezaudio le-broadcast-sink sync 0 1
bluezaudio le-bap-control sink-sync 0 1
bluezaudio le-broadcast-sink start 0 1
```

Evidence includes:

```text
PASS bluez-le-audio-profile
source=third/bluez/profiles/audio/pacs.c ... command=sink-discover
source=third/bluez/profiles/audio/ascs.c ... command=sink-config
source=third/bluez/profiles/audio/bass.c ... command=sink-sync
upstream-iso-recv: recv-ret=28 ... payload=4c 45 2d 41 55 44 49 4f ...
```

The broadcast restart gate repeats the BIS/BIG-shaped source/sink lifecycle
twice in one BLE hwsim session:

```text
PASS bluez-le-audio-broadcast-restart
round 1: source-announce/source-start -> broadcast-source start -> sink sync/start -> payload -> source-stop/sink stop
round 2: source-announce/source-start -> broadcast-source start -> sink sync/start -> payload -> source-stop/sink stop
BLE1: iso-socket-bind=2 iso-socket-connect=2 iso-socket-send=2
BLE2: iso-socket-bind=2 iso-socket-connect=2 iso-socket-recv=6 iso-socket-native-recv=3
```

The LE Audio transport gate adds the BlueZ `profiles/audio/transport.c`
Acquire/Release shape to the unicast/CIS path:

```text
bluezaudio le-unicast-control source-config/source-enable 0 1
bluezaudio media-transport unicast-source-acquire-write-release 0 1
bluezaudio le-unicast-control sink-discover/sink-config/sink-enable 0 1
bluezaudio media-transport unicast-sink-acquire-read-release 0 1
```

Evidence includes:

```text
PASS bluez-le-audio-transport
bluez-audio: source=third/bluez/profiles/audio/transport.c style profile=media-transport
bluez-audio: media transport acquire fd=iso ... read-mtu=251 write-mtu=251
upstream-iso-write: payload-len=28 send-ret=28
upstream-iso-recv: recv-ret=28 ... payload=4c 45 2d 41 55 44 49 4f ...
bluez-audio: media transport release complete role=sink
```

The repeated LE Audio transport gate runs the same MediaTransport-shaped ISO
fd ownership twice in one BLE hwsim session:

```text
PASS bluez-le-audio-transport-reconnect
round 1: source-config/source-enable -> acquire ISO source fd -> write 28-byte CIS payload -> acquire ISO sink fd -> read -> release
round 2: source-config/source-enable -> acquire ISO source fd -> write 28-byte CIS payload -> acquire ISO sink fd -> read -> release
iso-socket-bind=2
iso-socket-connect=2
iso-socket-send=2
iso-socket-recv=4
iso-socket-native-recv=2
```

The bidirectional LE Audio transport gate uses two CIS handles in the same
BLE1/BLE2 hwsim session, so each peer acts once as source and once as sink:

```text
PASS bluez-le-audio-transport-bidir
CIS 1 handle 0x0201: BLE1 source -> BLE2 sink
CIS 2 handle 0x0202: BLE2 source -> BLE1 sink
BLE1: source-release cig=0 cis=1 handle=0x0201 -> streaming->idle
BLE2: source-release cig=0 cis=2 handle=0x0202 -> streaming->idle
BLE1: iso-socket-bind=2 iso-socket-connect=2 iso-socket-send=1 iso-socket-recv=2 iso-socket-native-recv=1
BLE2: iso-socket-bind=2 iso-socket-connect=2 iso-socket-send=1 iso-socket-recv=2 iso-socket-native-recv=1
```

The repeated bidirectional LE Audio transport gate runs the same two-CIS
MediaTransport-shaped lifecycle twice in one BLE1/BLE2 hwsim session:

```text
PASS bluez-le-audio-transport-bidir-reconnect
round 1 CIS 1 handle 0x0201: BLE1 source -> BLE2 sink
round 1 CIS 2 handle 0x0202: BLE2 source -> BLE1 sink
round 2 CIS 1 handle 0x0201: BLE1 source -> BLE2 sink
round 2 CIS 2 handle 0x0202: BLE2 source -> BLE1 sink
BLE1: source-release cig=0 cis=1 handle=0x0201 -> streaming->idle, twice
BLE2: source-release cig=0 cis=2 handle=0x0202 -> streaming->idle, twice
BLE1: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
BLE2: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
```

The bidirectional disable gate adds a minimal BlueZ BAP/ASCS-shaped teardown
control step after the same two-CIS MediaTransport flow:

```text
PASS bluez-le-audio-transport-bidir-disable
CIS 1 handle 0x0201: BLE1 source -> BLE2 sink, then source-disable/source-release and sink-disable
CIS 2 handle 0x0202: BLE2 source -> BLE1 sink, then source-disable/source-release and sink-disable
BLE1: source-disable cig=0 cis=1 handle=0x0201 -> streaming->configured
BLE1: sink-disable cig=0 cis=2 handle=0x0202 -> streaming->configured
BLE2: sink-disable cig=0 cis=1 handle=0x0201 -> streaming->configured
BLE2: source-disable cig=0 cis=2 handle=0x0202 -> streaming->configured
BLE1: iso-socket-bind=2 iso-socket-connect=2 iso-socket-send=1 iso-socket-recv=2 iso-socket-native-recv=1
BLE2: iso-socket-bind=2 iso-socket-connect=2 iso-socket-send=1 iso-socket-recv=2 iso-socket-native-recv=1
```

The bidirectional QoS update gate adds a minimal BAP/ASCS-shaped reconfigure
loop: both source and sink sides disable, update QoS, re-enable, and then move
a second ISO payload over the same CIS handle:

```text
PASS bluez-le-audio-transport-bidir-qos-update
CIS 1 handle 0x0201: BLE1 source -> BLE2 sink, disable, QoS update, re-enable, second payload
CIS 2 handle 0x0202: BLE2 source -> BLE1 sink, disable, QoS update, re-enable, second payload
BLE1: source-qos-update cig=0 cis=1 handle=0x0201 -> configured->configured qos-updated
BLE1: sink-qos-update cig=0 cis=2 handle=0x0202 -> configured->configured qos-updated
BLE2: sink-qos-update cig=0 cis=1 handle=0x0201 -> configured->configured qos-updated
BLE2: source-qos-update cig=0 cis=2 handle=0x0202 -> configured->configured qos-updated
BLE1: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
BLE2: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
```

The bidirectional QoS reject gate adds the matching minimal error/recovery
path: both source and sink sides reject one invalid QoS update, remain
configured, then accept a valid QoS update, re-enable, and move a second ISO
payload:

```text
PASS bluez-le-audio-transport-bidir-qos-reject
CIS 1 handle 0x0201: BLE1 source -> BLE2 sink, invalid QoS reject, valid QoS update, second payload
CIS 2 handle 0x0202: BLE2 source -> BLE1 sink, invalid QoS reject, valid QoS update, second payload
BLE1: source-qos-reject cig=0 cis=1 handle=0x0201 -> status=0x0d reason=invalid-qos
BLE1: sink-qos-reject cig=0 cis=2 handle=0x0202 -> status=0x0d reason=invalid-qos
BLE2: sink-qos-reject cig=0 cis=1 handle=0x0201 -> status=0x0d reason=invalid-qos
BLE2: source-qos-reject cig=0 cis=2 handle=0x0202 -> status=0x0d reason=invalid-qos
BLE1: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
BLE2: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
```

The bidirectional QoS cancel gate covers the same retry shape for a cancelled
QoS procedure:

```text
PASS bluez-le-audio-transport-bidir-qos-cancel
CIS 1 handle 0x0201: BLE1 source -> BLE2 sink, QoS cancel, valid QoS update, second payload
CIS 2 handle 0x0202: BLE2 source -> BLE1 sink, QoS cancel, valid QoS update, second payload
BLE1: source-qos-cancel cig=0 cis=1 handle=0x0201 -> status=0x0e reason=procedure-cancelled
BLE1: sink-qos-cancel cig=0 cis=2 handle=0x0202 -> status=0x0e reason=procedure-cancelled
BLE2: sink-qos-cancel cig=0 cis=1 handle=0x0201 -> status=0x0e reason=procedure-cancelled
BLE2: source-qos-cancel cig=0 cis=2 handle=0x0202 -> status=0x0e reason=procedure-cancelled
BLE1: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
BLE2: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
```

The bidirectional release/reconfigure gate covers ASE release to idle followed
by a fresh PACS/ASCS/BAP-shaped configuration and a second ISO payload:

```text
PASS bluez-le-audio-transport-bidir-release-reconfig
CIS 1 handle 0x0201: BLE1 source -> BLE2 sink, release to idle, reconfigure, second payload
CIS 2 handle 0x0202: BLE2 source -> BLE1 sink, release to idle, reconfigure, second payload
BLE1: source-release cig=0 cis=1 handle=0x0201 -> configured->idle release-complete
BLE1: sink-release cig=0 cis=2 handle=0x0202 -> configured->idle release-complete
BLE2: sink-release cig=0 cis=1 handle=0x0201 -> configured->idle release-complete
BLE2: source-release cig=0 cis=2 handle=0x0202 -> configured->idle release-complete
BLE1: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
BLE2: iso-socket-bind=4 iso-socket-connect=4 iso-socket-send=2 iso-socket-recv=4 iso-socket-native-recv=2
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-bluezaudio-current.log
FeatherCore/build/logs/build-bt2-bluezaudio-current.log
FeatherCore/build/logs/build-ble1-bluezaudio-current.log
FeatherCore/build/logs/build-ble2-bluezaudio-current.log
FeatherCore/build/logs/build-ble1-le-audio-disable.log
FeatherCore/build/logs/build-ble2-le-audio-disable.log
FeatherCore/build/logs/build-ble1-le-audio-qos-update.log
FeatherCore/build/logs/build-ble2-le-audio-qos-update.log
FeatherCore/build/logs/build-ble1-le-audio-qos-reject.log
FeatherCore/build/logs/build-ble2-le-audio-qos-reject.log
FeatherCore/build/logs/build-ble1-le-audio-qos-cancel.log
FeatherCore/build/logs/build-ble2-le-audio-qos-cancel.log
FeatherCore/build/logs/build-ble1-le-audio-release-reconfig.log
FeatherCore/build/logs/build-ble2-le-audio-release-reconfig.log
FeatherCore/build/logs/build-ble1-bluez-le-audio-profile-current.log
FeatherCore/build/logs/build-ble2-bluez-le-audio-profile-current.log
FeatherCore/build/logs/build-bt1-bluez-a2dp-signaling-current.log
FeatherCore/build/logs/build-bt2-bluez-a2dp-signaling-current.log
FeatherCore/build/logs/build-bt1-bluez-a2dp-signaling-native.log
FeatherCore/build/logs/build-bt2-bluez-a2dp-signaling-native.log
FeatherCore/build/logs/build-bt1-bluez-a2dp-profile-current.log
FeatherCore/build/logs/build-bt2-bluez-a2dp-profile-current.log
FeatherCore/build/logs/build-bt1-bluez-a2dp-transport.log
FeatherCore/build/logs/build-bt2-bluez-a2dp-transport.log
FeatherCore/build/logs/run-bluez-a2dp-signaling-current.log
FeatherCore/build/logs/run-bluez-a2dp-signaling-native.log
FeatherCore/build/logs/run-bluez-a2dp-media-current.log
FeatherCore/build/logs/run-bluez-a2dp-profile-current.log
FeatherCore/build/logs/run-bluez-a2dp-transport.log
FeatherCore/build/logs/run-bluez-a2dp-endpoint-transport.log
FeatherCore/build/logs/run-bluez-a2dp-transport-reconnect.log
FeatherCore/build/logs/run-bluez-a2dp-transport-bidir.log
FeatherCore/build/logs/run-bluez-a2dp-transport-bidir-teardown.log
FeatherCore/build/logs/run-bluez-le-audio-current.log
FeatherCore/build/logs/run-bluez-le-audio-profile-current.log
FeatherCore/build/logs/run-bluez-le-audio-broadcast-restart.log
FeatherCore/build/logs/run-bluez-le-audio-transport.log
FeatherCore/build/logs/run-bluez-le-audio-transport-reconnect.log
FeatherCore/build/logs/run-bluez-le-audio-transport-bidir.log
FeatherCore/build/logs/run-bluez-le-audio-transport-bidir-disable.log
FeatherCore/build/logs/run-bluez-le-audio-transport-bidir-qos-update.log
FeatherCore/build/logs/run-bluez-le-audio-transport-bidir-qos-reject.log
FeatherCore/build/logs/run-bluez-le-audio-transport-bidir-qos-cancel.log
FeatherCore/build/logs/run-bluez-le-audio-transport-bidir-release-reconfig.log
FeatherCore/build/logs/run-bluez-le-audio-transport-bidir-reconnect.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-signaling-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-signaling-native/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-media-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-profile-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-transport/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-endpoint-transport/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-transport-reconnect/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-transport-bidir/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-transport-bidir-teardown/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-profile-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-broadcast-restart/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport-reconnect/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport-bidir/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport-bidir-disable/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport-bidir-qos-update/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport-bidir-qos-reject/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport-bidir-qos-cancel/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport-bidir-release-reconfig/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-transport-bidir-reconnect/run-results.json
```

This is still a profile-shaped signaling/payload gate, not full BlueZ A2DP or
LE Audio.  The next alignment step is to attach the upstream BlueZ profile
plugin and D-Bus media endpoint semantics to this same L2CAP/ISO socket path.

## A2DP repeated lifecycle diagnosis

Two repeated-lifecycle gates now exist:

```text
bluez-a2dp-transaction-reconnect-native
bluez-a2dp-profile-reconnect
```

`bluez-a2dp-transaction-reconnect-native` now passes.  It runs two rounds of
AVDTP source transactions against a native L2CAP listener:

```text
round 1 source-transaction + listen-native: PASS
round 1 l2cap-close: released
round 2 source-transaction + listen-native: PASS
round 2 l2cap-close: released
```

The fix adds sim-only Linux L2CAP close completion for the probe socket.  On
close, the sim path now detaches matching handle/CID channels from the upstream
L2CAP connection list and global channel list before releasing the socket, so a
second listener is not interrupted by the previous round's delayed
`BT_DISCONN` residue.

Artifacts:

```text
FeatherCore/build/logs/run-bluez-a2dp-transaction-reconnect-native.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-transaction-reconnect-native/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-profile-reconnect.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-profile-reconnect/run-results.json
```

`bluez-a2dp-profile-reconnect` now passes as the broader repeated-lifecycle
gate.  It adds media channel open/send/read/close after each signaling round,
so the hwsim session covers two rounds of:

```text
AVDTP source-transaction + listen-native
  -> signaling close
  -> A2DP source media send
  -> A2DP sink media read
  -> media close
```

The reconnect script binds the sink media transport to peer `1`, so BT2 logs
`peer=1 handle=0x0052` instead of the earlier `peer=0 handle=0x0000`, and the
validator checks that connected handle.  The sink read path filters out
non-media AVDTP control PDUs, the L2CAP dispatch path no longer lets an
explicit CID listener accept unrelated dynamic CIDs while in `BT_LISTEN`, and
the hwsim L2CAP bridge now keeps a small `(handle,cid)` pending queue for ACL
payloads that arrive before the matching probe socket receive queue is visible.

Latest artifact:

```text
PASS bluez-a2dp-profile-reconnect
FeatherCore/build/logs/run-bluez-a2dp-profile-reconnect.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-profile-reconnect/run-results.json
```

BT2 evidence includes the first media read through the normal socket path and
the second media read through the pending replay path:

```text
upstream-l2cap-recv: recv-ret=24 ... payload=41 32 44 50 ...
upstream-l2cap-recv: recv-ret=24 ... payload=41 32 44 50 ... pending=1
bluez-audio: a2dp sink media payload received
l2cap-socket-recv=13
l2cap-socket-native-recv=9
l2cap-socket-native-recv-fail=0
```

This closes the current BlueZ-shaped A2DP signaling+media reconnect gap.  It is
still not the final full `bluetoothd` audio plugin: D-Bus MediaEndpoint and
MediaTransport ownership, SDP/AVRCP, real SBC/codec handling, controller-driven
L2CAP channel setup, and fuller upstream `l2cap_sock.c` ownership remain open.

## BlueZ bneptest PAN ping gate

`bluezbneptest` now supports a persistent PAN lifecycle:

```text
bluezbneptest pan-up
bluezbneptest status
bluezbneptest pan-down
```

Current verified gate:

```text
PASS bluez-bneptest-ping
```

The gate uses the BlueZ `tools/bneptest.c` fd handoff shape on both BT roles:

```text
connected L2CAP fd -> BNEPCONNADD -> btn0 -> IP ping -> BNEPCONNDEL
```

Evidence includes:

```text
bluez-bneptest: source=third/bluez/tools/bneptest.c mode=pan-up
bluez-bneptest: bnep-connadd-fd ret=0 device=btn0
PING 10.77.0.2
2 packets transmitted, 2 received, 0% packet loss
PING 10.77.0.1
2 packets transmitted, 2 received, 0% packet loss
bluez-bneptest: bnep-conndel-fd ret=0
bluez-bneptest: pan-down complete
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-bluez-bneptest-ping-current.log
FeatherCore/build/logs/build-bt2-bluez-bneptest-ping-current.log
FeatherCore/build/logs/run-bluez-bneptest-ping-current.log
FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-ping-current/run-results.json
```

This moves the BNEP/IP demo closer to BlueZ network profile behavior: the BlueZ
adapter now creates `btn0` through `BNEPCONNADD` with a connected L2CAP fd and
keeps it long enough for IP traffic.  It is still not the complete BlueZ network
plugin or a fully unstaged imported Linux BNEP implementation.

## BlueZ Network Profile-shaped PAN ping gate

`blueznetwork` is the next apps-side adapter after `bluezbneptest`.  It follows
the BlueZ `profiles/network/connection.c` + `profiles/network/bnep.c` lifecycle
shape instead of the standalone `tools/bneptest.c` shape:

```text
blueznetwork connect panu
blueznetwork status
blueznetwork disconnect
```

Current verified gate:

```text
PASS bluez-network-ping
```

The gate uses this path on both BT roles:

```text
Network1 connect
  -> connected L2CAP fd
  -> BNEPCONNADD(role=PANU, flags=BNEP_SETUP_RESPONSE)
  -> btn0
  -> IP ping
  -> BNEPCONNDEL
```

Evidence includes:

```text
bluez-network: source=third/bluez/profiles/network/connection.c+profiles/network/bnep.c
bluez-network: profile interface=org.bluez.Network1 state=connecting
bluez-network: bnep-connadd ret=0 device=btn0 role=0x1115 flags=0x00000001
bluez-network: profile connected interface=btn0 uuid=0x1115
PING 10.77.0.2
2 packets transmitted, 2 received, 0% packet loss
PING 10.77.0.1
2 packets transmitted, 2 received, 0% packet loss
bluez-network: bnep-conndel ret=0 device=btn0
bluez-network: disconnect complete
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-bluez-network.log
FeatherCore/build/logs/build-bt2-bluez-network.log
FeatherCore/build/logs/run-bluez-network-ping.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-ping/run-results.json
```

`blueznetwork` also owns the Network Profile-shaped BNEP iperf matrix:

```text
PASS bluez-network-iperf-tcp
PASS bluez-network-iperf-tcp-reverse
PASS bluez-network-iperf-udp
PASS bluez-network-iperf-udp-reverse
PASS bluez-network-iperf-tcp-soak
```

Validated traffic directions:

```text
TCP bt2 -> bt1
TCP bt1 -> bt2
UDP bt2 -> bt1
UDP bt1 -> bt2
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-network-iperf-tcp.log
FeatherCore/build/logs/run-bluez-network-iperf-tcp-reverse.log
FeatherCore/build/logs/run-bluez-network-iperf-udp.log
FeatherCore/build/logs/run-bluez-network-iperf-udp-reverse.log
FeatherCore/build/logs/run-bluez-network-iperf-tcp-soak.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-iperf-tcp/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-network-iperf-tcp-reverse/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-network-iperf-udp/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-network-iperf-udp-reverse/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-network-iperf-tcp-soak/run-results.json
```

The soak gate keeps the same profile-shaped connect/disconnect path but extends
the TCP client run to 16 seconds and verifies native BNEP datapath counters plus
clean teardown after the longer transfer.

Repeated lifecycle gates are also covered:

```text
PASS bluez-network-reconnect
PASS bluez-network-reconnect-stress
```

The reconnect gate runs two rounds of:

```text
blueznetwork connect panu
ping
blueznetwork disconnect
```

The stress gate runs three rounds and requires the final native BNEP status to
return to:

```text
bnep-native-active=0
bnep-native-session-create=3
bnep-native-session-start=3
bnep-native-session-terminate=3
bnep-native-session-stop=3
bnep-native-netdev-register=3
bnep-native-netdev-unregister=3
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-network-reconnect.log
FeatherCore/build/logs/run-bluez-network-reconnect-stress.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-reconnect/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-network-reconnect-stress/run-results.json
```

This is closer to the real BlueZ Network Profile than `bluezbneptest`, because
it covers profile connect/disconnect ownership and the BlueZ
`BNEP_SETUP_RESPONSE` connadd flag.  It is still not the complete
`bluetoothd` D-Bus plugin: `manager.c`, `server.c`, service authorization,
NAP/GN server behavior, multi-peer policy, and cancel/error lifecycles still
need to be moved over.

## BlueZ bneptest PAN iperf matrix

`bluezbneptest` also owns the BNEP iperf matrix now:

```text
PASS bluez-bneptest-iperf-tcp
PASS bluez-bneptest-iperf-tcp-reverse
PASS bluez-bneptest-iperf-udp
PASS bluez-bneptest-iperf-udp-reverse
```

These gates use:

```text
bluezbneptest pan-up
ifconfig btn0 10.77.0.x
iperf -s / iperf -c
bluezbneptest pan-down
```

Validated traffic directions:

```text
TCP bt2 -> bt1
TCP bt1 -> bt2
UDP bt2 -> bt1
UDP bt1 -> bt2
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-bluez-bneptest-iperf-current.log
FeatherCore/build/logs/build-bt2-bluez-bneptest-iperf-current.log
FeatherCore/build/logs/run-bluez-bneptest-iperf-tcp-current.log
FeatherCore/build/logs/run-bluez-bneptest-iperf-tcp-reverse-current.log
FeatherCore/build/logs/run-bluez-bneptest-iperf-udp-current.log
FeatherCore/build/logs/run-bluez-bneptest-iperf-udp-reverse-current.log
FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-iperf-tcp-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-iperf-tcp-reverse-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-iperf-udp-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-iperf-udp-reverse-current/run-results.json
```

This closes the BlueZ-derived network-profile throughput gate for TCP/UDP and
both traffic directions.  The remaining work is still to replace the staging
pieces with fuller imported Linux BNEP ownership and to attach the real
`bluetoothd` network profile plugin.

## BlueZ bneptest reconnect gates

`bluezbneptest` now also covers repeated BNEP lifecycle cleanup:

```text
PASS bluez-bneptest-reconnect
PASS bluez-bneptest-reconnect-stress
```

The two gates run the same BlueZ bneptest fd handoff shape repeatedly:

```text
bluezbneptest pan-up
ping
bluezbneptest pan-down
```

Validation checks:

```text
bluez-bneptest-reconnect: connadd/conndel/session/netdev counters == 2
bluez-bneptest-reconnect-stress: connadd/conndel/session/netdev counters == 3
final bnep-native-active=0
final cnum=0 after BNEPCONNDEL
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-bneptest-reconnect-current.log
FeatherCore/build/logs/run-bluez-bneptest-reconnect-stress-current.log
FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-reconnect-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-reconnect-stress-current/run-results.json
```

This improves confidence that the BlueZ-derived network path cleans up repeated
PAN sessions, but it is still a bounded hwsim lifecycle gate rather than full
daemon-owned Network Profile state.

## A2DP SBC native source control TX tightening

The BlueZ-shaped A2DP SBC gates now require source-side AVDTP control packets to use the native L2CAP socket send path instead of accepting the old sim fallback path.

Updated evidence:

```text
PASS bluez-a2dp-sbc-codec-transport
PASS bluez-a2dp-sbc-codec-extended
PASS bluez-a2dp-sbc-codec-abort
PASS bluez-a2dp-sbc-codec-reconnect
```

The validator now rejects the old source-side form:

```text
native-ret=-107 fallback-ret=0
```

and requires native send evidence such as:

```text
send-ret=<len> native-ret=<len> attach-ret=0 fallback-ret=-95
```

This is implemented by attaching transient AVDTP L2CAP send sockets to the sim hwsim imported L2CAP channel/connection before `sendmsg()`, and by marking the socket state `BT_CONNECTED` for the send lifetime.

Artifacts:

```text
FeatherCore/build/logs/run-bluez-a2dp-sbc-native-tx-pass.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-native-tx-pass/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-sbc-extended-native-tx-pass.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-extended-native-tx-pass/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-sbc-abort-native-tx.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-abort-native-tx/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-sbc-reconnect-native-tx.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-reconnect-native-tx/run-results.json
```

Remaining boundary: these are still BlueZ-shaped app/profile gates, not a full `bluetoothd` audio daemon with complete D-Bus object ownership, SDP/AVRCP policy, controller-created L2CAP channels, and upstream AVDTP session ownership.

## A2DP source signaling session path

The A2DP SBC gates now exercise a source-side signaling session instead of sending every AVDTP request through an independent transient L2CAP send helper.

New commands:

```text
bluezaudio a2dp-signal source-session-open <peer>
bluezaudio a2dp-signal source-session-close <peer>
```

While the source session is open, `discover`, `getcap`, `setconfig`, `open`, `start`, `suspend`, `close-stream`, and `abort` use the persistent L2CAP socket and emit native write evidence:

```text
upstream-l2cap-write: payload-len=<len> send-ret=<len> native-ret=<len> attach-ret=0 fallback-ret=-95
```

Validated gates:

```text
PASS bluez-a2dp-sbc-codec-transport
PASS bluez-a2dp-sbc-codec-extended
PASS bluez-a2dp-sbc-codec-abort
PASS bluez-a2dp-sbc-codec-reconnect
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-a2dp-source-session-transport.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-source-session-transport/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-source-session-extended.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-source-session-extended/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-source-session-abort.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-source-session-abort/run-results.json
FeatherCore/build/logs/run-bluez-a2dp-source-session-reconnect-final.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-source-session-reconnect-final/run-results.json
```

Remaining limitation: the current probe infrastructure still has one global L2CAP socket, so the gate closes the signaling session before acquiring the media transport socket and opens a second short session for teardown. Full `bluetoothd` parity still requires concurrent signaling/media fd ownership, a real AVDTP session object, and D-Bus MediaTransport lifecycle ownership.
