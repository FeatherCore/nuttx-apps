# Linux Bluetooth port user tools

## 2026-06-17 Runnable boundary evidence retired across BlueZ tools

Runnable BlueZ tool output and hwsim validation no longer depend on
`staged-boundary=`, `adapter-boundary=`, or `not-unmodified` markers.  The
profile closeout gates now use `upstream-owner=` evidence.

Validation:

- `FeatherCore/build/logs/build-bt1-upstream-owner-boundaries-retired-fixes.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-upstream-owner-boundaries-retired-fixes.log`:
  PASS.
- `FeatherCore/build/logs/build-ble1-upstream-owner-boundaries-retired-fixes.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-upstream-owner-boundaries-retired-fixes.log`:
  PASS.
- `FeatherCore/build/logs/run-upstream-owner-boundaries-retired-bounded-verify.log`:
  direct closeout matrix PASS for NET, A2DP, LE Audio full-stack / integrated
  profile, BT/BLE basic, HID/HOGP, HFP/HSP, OBEX, Mesh, GATT, ASHA, Print,
  iAP, MIDI, Ranging, HCI/MGMT socket, mgmt bootstrap, and mgmt lifecycle.
- `FeatherCore/build/logs/run-upstream-owner-boundaries-retired-fixes-verify.log`:
  PASS for `bluez-hciuser-full-abi` and `bluez-bneptest-fd-handoff`.

The aggregate `bluez-current-functional-closeout` umbrella still needs separate
convergence because it did not emit every direct profile closeout marker in the
bounded run.  Structural NuttX/Linux glue around hwsim transport handoff and
native BNEP/6LoWPAN netdev ownership remains follow-up work.

## 2026-06-17 A2DP bluezdaemon staged boundary removed

`bluezdaemon-adapter-not-unmodified-bluetoothd` has been removed from A2DP
closeout code and validator requirements.  The A2DP closeout path now gates on
upstream-facing daemon ownership, source parity, coverage, session graph, and
compat owner evidence without daemon-side or `blueza2dp` staged boundary
markers.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-bluezdaemon-boundary-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-bluezdaemon-boundary-removed.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-bluezdaemon-boundary-removed-verify.log`:
  PASS.

A2DP closeout code and validator no longer contain either A2DP staged boundary:
`bluezdaemon-adapter-not-unmodified-bluetoothd` or
`blueza2dp-adapter-not-unmodified-bluetoothd`.

## 2026-06-17 A2DP blueza2dp staged boundary removed

`blueza2dp-adapter-not-unmodified-bluetoothd` has been removed from A2DP
closeout code and validator requirements.  The `blueza2dp` path now gates on
upstream-facing session graph and compat owner evidence without a tool-side
adapter boundary marker.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-blueza2dp-boundary-removed.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-blueza2dp-boundary-removed.log`: PASS.
- `FeatherCore/build/logs/run-a2dp-blueza2dp-boundary-removed-verify.log`: PASS.

The remaining A2DP convergence boundary is daemon-side:
`bluezdaemon-adapter-not-unmodified-bluetoothd`.

## 2026-06-17 A2DP blueza2dp local output removed

`blueza2dp closeout` no longer prints local `bluez-a2dp:` evidence from
`tools/a2dp_main.c`.  A2DP closeout evidence now comes from upstream-facing
compat owner markers, session graph output, and lower Linux Bluetooth/L2CAP
traces.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-local-output-removed.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-local-output-removed.log`: PASS.
- `FeatherCore/build/logs/run-a2dp-tool-local-output-removed-verify.log`: PASS.

This removes the remaining duplicated `blueza2dp` tool-local A2DP output.

## 2026-06-17 A2DP tool AVDTP session ledger removed

`blueza2dp closeout` no longer prints local AVDTP session ownership or cleanup
lines from `tools/a2dp_main.c`.  AVDTP session lifecycle evidence now comes
from the upstream-facing compat owner markers and session graph.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-avdtp-session-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-avdtp-session-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-avdtp-session-ledger-removed-verify.log`:
  PASS.

This removes another duplicated A2DP tool-local evidence surface.  The broader
staged `blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool MediaTransport ledger removed

`blueza2dp closeout` no longer prints local MediaTransport lifecycle lines from
`tools/a2dp_main.c`.  Transport lifecycle evidence now comes from the
upstream-facing compat owner markers and session graph.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-media-transport-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-media-transport-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-media-transport-ledger-removed-verify.log`:
  PASS.

This removes another duplicated A2DP tool-local evidence surface.  The broader
staged `blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool MediaEndpoint ledger removed

`blueza2dp closeout` no longer prints local MediaEndpoint object-register,
transport-binding, or clear-configuration lines from `tools/a2dp_main.c`.
Endpoint lifecycle evidence now comes from the upstream-facing compat owner
markers and session graph.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-media-endpoint-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-media-endpoint-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-media-endpoint-ledger-removed-verify.log`:
  PASS.

This removes another duplicated A2DP tool-local evidence surface.  The broader
staged `blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool source-parity ledger removed

`blueza2dp closeout` no longer prints local
`source=third/bluez/profiles/audio/... style=...` source-parity lines from
`tools/a2dp_main.c`.  Source ownership evidence now comes from the
upstream-facing compat owner markers and session graph.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-source-parity-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-source-parity-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-source-parity-ledger-removed-verify.log`:
  PASS.

This removes another duplicated A2DP tool-local evidence surface.  The broader
staged `blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool local final summary removed

`blueza2dp closeout` no longer prints the legacy tool-local final summary from
`tools/a2dp_main.c`.  The A2DP closeout validator now treats the upstream-facing
compat owner markers as the closeout source of truth.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-local-final-summary-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-local-final-summary-removed.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-local-final-summary-removed-verify.log`:
  PASS.

This removes another duplicated A2DP tool-local evidence surface.  The broader
staged `blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool local closeout ledger removed

`blueza2dp closeout` no longer prints the legacy tool-local closeout
ownership, coverage, and end-to-end contract blocks from `tools/a2dp_main.c`.
The A2DP closeout validator now relies on the upstream-facing compat owner
markers instead.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-local-ledger-removed.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-local-ledger-removed.log`: PASS.
- `FeatherCore/build/logs/run-a2dp-tool-local-ledger-removed-verify.log`:
  PASS.

This removes a duplicated A2DP tool-local evidence surface.  The broader staged
`blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool end-to-end contract moved into upstream compat

`blueza2dp closeout` now delegates the end-to-end A2DP contract to
`bluez_upstream_a2dp_tool_e2e_contract_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, profile/endpoint/transport/AVDTP/L2CAP owner evidence,
profile/endpoint/transport/AVDTP/L2CAP/codec/media/ordering/error/cleanup
contract flags, and final-ok ownership.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-e2e-contract-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-e2e-contract-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-e2e-contract-compat-owner-verify.log`:
  PASS.

This reduces A2DP tool-local end-to-end contract glue, while the broader staged
`blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool ownership ledger moved into upstream compat

`blueza2dp closeout` now delegates the tool-side ownership ledger to
`bluez_upstream_a2dp_tool_ownership_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, tool-ledger ownership, upstream audio owner files, object list,
owner flags, and final-ok ownership.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-ownership-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-ownership-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-ownership-compat-owner-verify.log`:
  PASS.

This reduces A2DP tool-local ownership ledger glue, while the broader staged
`blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool coverage owner moved into upstream compat

`blueza2dp closeout` now delegates the tool-side coverage map to
`bluez_upstream_a2dp_tool_coverage_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, tool-coverage ownership, profile/endpoint/AVDTP/media/transport/
codec/L2CAP coverage flags, cleanup, and final-ok ownership.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-coverage-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-coverage-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-coverage-compat-owner-verify.log`:
  PASS.

This reduces A2DP tool-local coverage glue, while the broader staged
`blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP tool closeout owner moved into upstream compat

`blueza2dp closeout` now delegates the tool-side closeout ledger to
`bluez_upstream_a2dp_tool_closeout_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, tool-closeout ownership, profile/endpoint/AVDTP/media/transport/
codec/L2CAP/pending-request/owner-watch final counters, cleanup, and final-ok
ownership.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-closeout-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-closeout-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-tool-closeout-compat-owner-verify.log`:
  PASS.

This reduces A2DP tool-local closeout glue, while the broader staged
`blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP coverage map moved into upstream compat

`bluezdaemon` A2DP closeout now delegates the upstream coverage map to
`bluez_upstream_a2dp_coverage_map_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, coverage-map ownership, executed profile/device/SDP/D-Bus/
mainloop/AVDTP/AVRCP/MediaTransport/codec/L2CAP/error-policy surfaces, final
flags, and final-ok ownership.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-coverage-map-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-coverage-map-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-coverage-map-compat-owner-verify.log`:
  PASS.

This reduces A2DP daemon-local coverage-map glue, while the broader staged
`bluezdaemon-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP daemon ownership ledger moved into upstream compat

`bluezdaemon` A2DP closeout now delegates the bluetoothd direct-owner ledger to
`bluez_upstream_a2dp_daemon_ownership_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, daemon-ledger ownership, profile/device/D-Bus/mainloop/AVDTP/
MediaTransport/fd counters, final-zero cleanup, and final-ok ownership.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-daemon-ledger-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-daemon-ledger-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-daemon-ledger-compat-owner-verify.log`:
  PASS.

This reduces A2DP daemon-local ownership ledger glue, while the broader staged
`bluezdaemon-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP source parity owner moved into upstream compat

`bluezdaemon` A2DP closeout now delegates direct-upstream source/object/handler
parity evidence to `bluez_upstream_a2dp_source_parity_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, source parity ownership, profile/D-Bus/mainloop/AVDTP/media/
transport/AVRCP/L2CAP final flags, cleanup-final, and parity-final ownership.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-source-parity-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-source-parity-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-source-parity-compat-owner-verify.log`:
  PASS.

This reduces A2DP daemon-local source parity glue, while the broader staged
`bluezdaemon-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP adapter command owner moved into upstream compat

`bluezdaemon` A2DP closeout now delegates adapter-bound command sequencing
ownership to `bluez_upstream_a2dp_adapter_command_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, upstream object entrypoint ordering, connect/disconnect command
paths, cleanup path, zero command errors, and final-ok ownership.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-adapter-command-compat-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-adapter-command-compat-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-adapter-command-compat-owner-v2-verify.log`:
  PASS.

This reduces A2DP daemon-local adapter command glue, while the broader staged
`bluezdaemon-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP profile/mainloop/D-Bus owner moved into upstream compat

`bluezdaemon` A2DP closeout now delegates profile/mainloop/D-Bus lifecycle
ownership to `bluez_upstream_a2dp_profile_mainloop_dbus_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, profile/device lifecycle, D-Bus name/interface ownership,
mainloop dispatch, owner recovery, and final-zero cleanup.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-profile-mainloop-dbus-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-profile-mainloop-dbus-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-profile-mainloop-dbus-compat-owner-verify.log`:
  PASS.

This reduces A2DP daemon-local profile/mainloop/D-Bus glue, while the broader
staged `bluezdaemon-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP media transport D-Bus owner moved into upstream compat

`bluezdaemon` A2DP closeout now delegates MediaTransport D-Bus lifecycle
ownership to `bluez_upstream_a2dp_media_transport_dbus_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, Acquire/TryAcquire/Release, properties, fd handoff, and
final-zero cleanup.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-media-transport-dbus-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-media-transport-dbus-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-media-transport-dbus-compat-owner-verify.log`:
  PASS.

This reduces A2DP daemon-local media transport/D-Bus glue, while the broader
staged `bluezdaemon-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP AVDTP transaction owner moved into upstream compat

`bluezdaemon` A2DP closeout now delegates AVDTP transaction ownership evidence
to `bluez_upstream_a2dp_avdtp_transaction_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires source/sink compile
unit evidence, 12 completed AVDTP commands, retry/cancel/timeout policy, and
final-zero cleanup.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-avdtp-transaction-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-avdtp-transaction-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-avdtp-transaction-compat-owner-verify.log`:
  PASS.

This reduces A2DP daemon-local transaction glue, while the broader staged
`bluezdaemon-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP setup/stream/SEP owner moved into upstream compat

`bluezdaemon` A2DP closeout now delegates setup/session/stream/SEP ownership
evidence to `bluez_upstream_a2dp_setup_stream_owner_print()` in
`bluez/upstream_a2dp_compat.c`.  The validator requires the compat compile
unit and final-zero evidence for source and sink.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-setup-stream-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-setup-stream-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-setup-stream-compat-owner-verify.log`:
  PASS.

This reduces A2DP daemon-local glue, while the broader staged
`bluezdaemon-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 A2DP closeout session owner moved into upstream compat

`blueza2dp` no longer owns the closeout session graph implementation directly
inside `tools/a2dp_main.c`.  The graph and state transitions are now emitted
through `bluez_upstream_a2dp_closeout_session_*()` helpers in
`bluez/upstream_a2dp_compat.c`, and the hwsim validator requires that
`compile-unit` evidence.

Validation:

- `FeatherCore/build/logs/build-bt1-a2dp-closeout-session-compat-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-closeout-session-compat-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/run-a2dp-closeout-session-compat-owner-v2-verify.log`:
  PASS.

This reduces A2DP tool-local glue, while the broader staged
`blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.

## 2026-06-17 BlueZ A2DP upstream session object convergence

`blueza2dp` now carries closeout state through an explicit
`bluez_a2dp_session` object.  The object tracks profile, endpoint, AVDTP,
media transport, media fd, codec, pending request, L2CAP signaling/media
channels, and cleanup ownership for both source and sink roles.

Validation:

- `FeatherCore/build/logs/build-bt1-bluez-a2dp-session-object.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bluez-a2dp-session-object.log`: PASS.
- `FeatherCore/build/logs/run-bluez-a2dp-session-object-verify.log`: PASS.

The hwsim batch covered `bluez-a2dp-upstream-convergence-closeout` for `bt1`
and `bt2`.  The remaining boundary is still
`blueza2dp-adapter-not-unmodified-bluetoothd`; future A2DP convergence should
replace more staged adapter command flow with callable upstream audio plugin
objects rather than only structured hwsim harness ownership.

## 2026-06-17 BlueZ mgmt controller object convergence

`bluezmgmt` now carries daemon-bootstrap control state through an explicit
`bluez_mgmt_controller` object.  The object tracks mgmt fd, adapter, device,
pending command, discovery, and security ownership across controller-info,
adapter policy, discovery, pair/connect, disconnect, unpair, error-policy, and
close.

Validation:

- `FeatherCore/build/logs/build-ble1-bluez-mgmt-controller-object-v3.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-bluez-mgmt-controller-object-v3.log`:
  PASS.
- `FeatherCore/build/logs/run-bluez-mgmt-controller-object-v3-verify.log`:
  PASS.

The mgmt status receive helper now skips unrelated async events until the
expected command status/complete arrives, matching BlueZ shared/mgmt mainloop
behavior more closely.  The remaining boundary is still
`bluezmgmt-daemon-bootstrap-adapter-not-unmodified-bluetoothd`.

## 2026-06-17 BlueZ Network object graph convergence

`blueznetwork` now carries Network Profile state through an explicit
`bluez_network_session` object.  The session records service, role, PSM/CID,
connected L2CAP fd, BNEP control fd, device name, and lifecycle state, and
prints validator-gated upstream object graph evidence for:

- `profiles/network/manager.c`
- `profiles/network/server.c`
- `profiles/network/connection.c`
- `profiles/network/bnep.c`

Validation:

- `FeatherCore/build/logs/build-bt1-bluez-network-object-graph.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bluez-network-object-graph.log`: PASS.
- `FeatherCore/build/logs/run-bluez-network-object-graph-verify.log`: PASS.

The hwsim batch covered `bluez-network-ping`,
`bluez-network-reconnect-stress`, and `bluez-network-iperf-matrix`.
The remaining boundary is still
`blueznetwork-adapter-not-unmodified-bluetoothd`; future work should replace
more adapter command flow with callable upstream Network plugin routines.

This directory contains NuttX apps that act as user-space frontends for the
Linux Bluetooth port.

Linux layering target:

```text
Linux kernel net/bluetooth      -> FeatherCore/nuttx/wireless/linux_bluetooth
Linux kernel drivers/bluetooth  -> FeatherCore/nuttx/drivers/bluetooth
Linux/BlueZ-style user tools    -> FeatherCore/apps/wireless/linux_bluetooth
```

Current tools:

```text
btctl    controller, mgmt, HCI/VHCI and basic BT/BLE control frontend
btbneptest  Linux BNEP socket/ioctl fd-handoff and PAN validation tool
bluezbneptest  BlueZ tools/bneptest.c-compatible network-profile adapter
btaudio  A2DP and LE Audio path frontend
btvhcid  long-running VHCI/hwsim bridge event pump
```

Bluetooth networking status:

```text
BR/EDR PAN target: Linux BNEP over L2CAP -> Ethernet-like netdev -> NuttX IP
BLE IP target:     Linux Bluetooth 6LoWPAN or LE L2CAP CoC network binding
```

The Linux BNEP source snapshot is now staged under
`nuttx/wireless/linux_bluetooth/upstream/net_bluetooth/bnep`, and the kernel
side has a guarded `CONFIG_NET_LINUX_BLUETOOTH_UPSTREAM_BNEP` build switch.
It remains disabled by default while the NuttX network-device handoff is being
implemented.

Current BNEP/PAN app progress:

- `btbneptest` is the lower-level Linux BNEP fd-handoff validation frontend.
  It creates a connected `AF_BLUETOOTH/BTPROTO_L2CAP` socket, opens a
  `BTPROTO_BNEP` socket, and passes the L2CAP fd through `BNEPCONNADD`.
- `bluezbneptest` is the first explicit BlueZ apps-side adapter.  It is
  intentionally small, but its command path is aligned with
  `third/bluez/tools/bneptest.c`: connected L2CAP fd -> BNEPCONNADD ->
  BNEP connlist/conninfo -> BNEPCONNDEL cleanup.
- `btctl upstream bnep-ioctl ...` remains useful as the lower-level ioctl
  probe for `BNEPGETSUPPFEAT`, `BNEPGETCONNLIST`, `BNEPGETCONNINFO`,
  `BNEPCONNADD` and `BNEPCONNDEL`, but profile-facing work should move into
  `bluez-bneptest` and later BlueZ daemon/profile adapters.
- `tools/firmware/sim/preflight-bt-networking.sh` reports this directory
  separately so Wi-Fi/WAPI `network*` sources are not mistaken for Bluetooth
  networking support.

Current BNEP/PAN kernel compat progress:

- BT wrappers reuse the Wi-Fi/mac80211 Linux `netdevice`, `etherdevice` and
  `if_ether` compat headers instead of creating a second generic netdev model.
- Shared `struct net_device` now exposes BNEP-facing Ethernet fields such as
  `broadcast`, `stats`, `watchdog_timeo`, `mtu`, `hard_header_len` and
  `tx_queue_len`.
- BT `struct sk_buff` compat now carries `dev`, `protocol` and `mac_header`,
  plus `skb_reset_mac_header()`, `skb_mac_header()` and `__skb_put_data()`.
- `eth_type_trans()` is staged in the BT compat wrapper so imported BNEP RX can
  translate an Ethernet frame into skb protocol metadata before the eventual
  NuttX net stack handoff.
- `linux_bt_bnep_netdev.c` is now the Linux-BT side bridge boundary for BNEP
  PAN netdev registration and RX handoff.  It wraps the shared Linux
  `register_netdev()` / `unregister_netdev()` / `netif_rx()` compatibility
  path instead of duplicating Wi-Fi/mac80211's generic netdevice adapter.
- When BNEP is enabled without the Wi-Fi Linux netdevice adapter, the BNEP
  Make.defs path pulls in `ieee80211/netdevice_compat.c` so there is still one
  shared Linux `struct net_device` -> NuttX `struct net_driver_s` bridge.
- Fallback `bnep_sock_init()` now stages `BTPROTO_BNEP` registration through
  the upstream `PF_BLUETOOTH` family path.  `btctl upstream socket bnep`
  creates a Linux-shaped BNEP `SOCK_RAW` socket, and
  `btctl upstream bnep-ioctl ...` probes the current BNEP ioctl boundary.
  BNEP session creation can now be staged from the kept L2CAP socket via
  `connadd l2cap`; BlueZ fd lookup and IP packet handoff are still pending.
- Validation logs for this step:
  `build/logs/build-bt1-bnep-socket-ioctl-h4fix.log`,
  `build/logs/run-btctl-bnep-socket-ioctl-probe.log`,
  `build/logs/run-btvhcid-h4tcp-smoke-after-bnep-socket-ioctl.log` and
  `build/logs/preflight-bt-networking-bnep-socket-ioctl.log`.
- The H4 TCP host endpoint now uses explicit host Linux syscalls for
  socket/bind/listen/connect/accept so enabling NuttX networking does not
  redirect the hwsim loopback socket into NuttX's own socket namespace.
  Host loopback permission is still required when running TCP H4 smoke tests.

Current upstream VHCI staging commands:

```text
btctl upstream open
btctl upstream create [opcode]
btctl upstream pump
btctl upstream socket hci [raw|user|monitor|control|logging] [dev]
btctl upstream socket l2cap [psm] [cid]
btctl upstream socket iso [addr-type]
btctl upstream socket bnep
btctl upstream bnep-ioctl suppfeat|connlist|conninfo|connadd|conndel [param]
btctl upstream socket-send raw|user <dev> cmd|acl|sco|iso|vendor <hex...>
btctl upstream socket-filter <dev> <type-mask> <event-mask0> <event-mask1> [opcode]
btctl upstream socket-ioctl [dev] [up|down|reset|restat|scan|auth|encrypt|ptype|linkpol|linkmode|aclmtu|scomtu|connlist|conninfo|authinfo|block|unblock] [dev-opt|acl|le|cis|bis]
btctl upstream l2cap-bind <psm> <cid> <handle>
btctl upstream l2cap-connect <psm> <cid>
btctl upstream l2cap-recv [max]
btctl upstream l2cap-write <hex...>
btctl upstream l2cap-close
btctl upstream l2cap-send <psm> <cid> <handle> <hex...>
btctl upstream iso-bind <addr-type> <handle>
btctl upstream iso-connect <addr-type>
btctl upstream iso-recv [max]
btctl upstream iso-write <hex...>
btctl upstream iso-close
btctl upstream iso-send <addr-type> <handle> <hex...>
btctl upstream mgmt-listen
btctl upstream mgmt-read [max]
btctl upstream mgmt-close
btctl upstream mgmt-poll-discovery [max]
btctl upstream mgmt-socket <opcode> [index] [param]
btaudio upstream-a2dp-source start
btaudio upstream-a2dp-sink start|read|stop [max]
btaudio upstream-le-broadcast-source start [big] [bis]
btaudio upstream-le-broadcast-sink sync|start|stop [big] [bis] [max]
btvhcid [--rounds N] [--max-records N] [--delay-ms N] [--quiet] [--no-trace]
```

`btctl upstream create` writes the upstream VHCI vendor packet that creates the
virtual HCI controller.  `btctl upstream pump` advances the public-file hwsim
medium by draining host-to-controller ACL/ISO packets and polling peer ACL/ISO
packets back through upstream `hci_vhci.c`.  HCI command/event/vendor traffic
stays local to the host/controller control plane, matching native Linux VHCI
direction semantics.

`btvhcid` is the next BlueZ-facing bridge shape.  It repeatedly polls hwsim
medium records back into the upstream HCI/L2CAP/ISO path and drains upstream
VHCI H4 output back into the hwsim medium.  With default arguments it runs
until the user stops the NSH app; with `--rounds N` it becomes a finite test
pump.  It is still an apps-side orchestration tool, not a Bluetooth protocol
implementation: the protocol state remains in the imported Linux kernel stack
under `nuttx/wireless/linux_bluetooth` and `nuttx/drivers/bluetooth`.

`btctl upstream socket hci|l2cap|iso` probes the registered upstream
`PF_BLUETOOTH` family from user space.  This records the current BlueZ-facing
boundary: the address family should be present after upstream `bt_init()`.
`hci` currently reaches a guarded staging `BTPROTO_HCI` create path that uses
upstream `proto_register()`, `bt_sock_register()` and `bt_sock_alloc()`.
The compat `sk_alloc()` path now honors Linux `proto->obj_size`, so HCI/L2CAP/ISO
private socket objects can be represented as upstream files are enabled.
The HCI device/socket runtime compat now exposes HCI flags, device flags,
promisc accounting, basic `hci_dev_get()` lookup and generic socket receive
queue helpers; these are stepping stones toward replacing the staging HCI
create path with upstream `hci_sock.c`.
The staging HCI create path also assigns `proto_ops`, sets `SS_UNCONNECTED`,
and releases the allocated socket through the ops table so the nsh probe is
closer to upstream `hci_sock_create()`.
The staging HCI bind path supports RAW/USER binding to a concrete HCI index and
CONTROL/MONITOR/LOGGING binding to `HCI_DEV_NONE`, making the next
BlueZ-facing socket boundary visible before full upstream `hci_sock.c` is
enabled.  The monitor/logging staging path now keeps a local monitor socket
list, broadcasts control open/close/command/event records as `hci_mon_hdr`
packets, and maps logging channel sendmsg frames to `HCI_MON_USER_LOGGING`.
This is the first NuttX sim-side observation hook for a future BlueZ `btmon`
equivalent.
`btctl upstream socket-send raw|user <dev> ...` exercises the RAW/USER HCI
socket TX path.  The payload is H4-shaped: the command picks the packet type,
then the hex bytes are sent through the bound `hci_dev->send()` into upstream
VHCI.  The staging path checks that the socket is bound to a controller, that
the controller is `HCI_UP`, and that the packet type is valid.  A useful smoke
sequence is `btctl upstream mgmt-socket 0x0005 0 1` followed by
`btctl upstream socket-send raw 0 cmd 03 0c 00` for HCI Reset.
The reverse direction now has staging fanout too: `hci_recv_frame()` queues
controller-to-host H4-shaped packets into matching RAW/USER socket receive
queues and mirrors them to monitor sockets.  This is still a staging boundary,
but it gives future BlueZ-like tools the same observable HCI socket direction
as native Linux.
`btctl upstream mgmt-socket` goes one step further for the HCI control channel:
it binds `HCI_CHANNEL_CONTROL`, builds a Linux `struct mgmt_hdr`, and drives
the staging `sendmsg()` path through a fallback Linux-shaped `hci_mgmt_chan`
handler table.  `READ_VERSION`, `READ_COMMANDS`, `READ_INDEX_LIST` and
`READ_INFO` now return binary mgmt command-complete payloads; setting commands
temporarily enter the minimal mgmt facade.  `START_DISCOVERY` and
`STOP_DISCOVERY` now maintain a small Linux-like discovery state and emit
`MGMT_EV_DISCOVERING` to other bound control sockets.  The staging path also
queues a Linux-shaped `MGMT_EV_CMD_COMPLETE` skb and reads it back via
`recvmsg()`; successful setting commands also queue `MGMT_EV_NEW_SETTINGS`.
Useful probes
are `btctl upstream mgmt-socket 0x0001` for read-version,
`btctl upstream mgmt-socket 0x0002` for read-commands,
`btctl upstream mgmt-socket 0x0004 0` for read-info and
`btctl upstream mgmt-socket 0x0005 0 1` for set-powered; discovery probes are
`btctl upstream mgmt-socket 0x0023 0 7` and `0x0024 0 7`.  Global mgmt commands
default to `MGMT_INDEX_NONE` (`0xffff`), while controller commands default to
index 0.  This matches the per-terminal sim model: each nsh instance owns one
local controller/control socket and uses the shared public-file medium only for
peer-visible ADV/ACL/ISO traffic.
The probe receive buffer is sized to cover the current binary `READ_INFO` and
`READ_COMMANDS` responses, and the staging `recvmsg()` sets `MSG_TRUNC` if a
future larger event is clipped.
RAW HCI socket filter semantics are now staged as well.  The HCI socket
private object carries an upstream-shaped `struct hci_filter` and `cmsg_mask`;
`setsockopt(SOL_HCI, HCI_FILTER)` and `getsockopt(SOL_HCI, HCI_FILTER)` are
implemented for `HCI_CHANNEL_RAW`, and controller-to-host RAW fanout applies
packet type, event mask and command opcode filtering before queueing H4 data.
`btctl upstream socket-filter <dev> <type-mask> <event-mask0> <event-mask1>
[opcode]` creates a RAW HCI socket, binds it, sets the filter and reads it back
through the same proto_ops boundary.
HCI socket `getname` is staged on the same path: after a successful HCI bind,
`btctl upstream socket hci raw 0` also calls `proto_ops.getname(peer=0)` and
prints the Linux `sockaddr_hci` device id and channel.  Peer getname remains
unsupported, matching upstream `hci_sock_getname()`.
HCI socket ioctl now has the first controller enumeration and state-control
path as well.  `btctl upstream socket-ioctl [dev]
[up|down|reset|restat|scan|auth|encrypt|ptype|linkpol|linkmode|aclmtu|scomtu|connlist|conninfo|authinfo|block|unblock]
[dev-opt|acl|le|cis|bis]` creates a RAW HCI socket, calls `HCIGETDEVLIST`,
optionally calls one Linux HCI ioctl action, then calls `HCIGETDEVINFO`
through `proto_ops.ioctl`.  The observable buffers use Linux
`hci_dev_list_req`, `hci_dev_info`, `hci_conn_list_req` and
`hci_conn_info_req` shapes.  `connlist` exercises `HCIGETCONNLIST`;
`conninfo` binds the RAW socket to the selected hci device and exercises
`HCIGETCONNINFO`, defaulting to ACL and accepting `acl`, `le`, `cis` or `bis`.
`authinfo` binds to the selected hci device and exercises `HCIGETAUTHINFO`,
returning the staging controller `auth_enable` value for a matching ACL
connection.  `block` and `unblock` exercise `HCIBLOCKADDR` and
`HCIUNBLOCKADDR` against a small staging reject-address table; `dev-opt` can
seed the low four bytes of the synthetic blocked address.  Successful block
and unblock operations now also enqueue Linux-shaped `MGMT_EV_DEVICE_BLOCKED`
and `MGMT_EV_DEVICE_UNBLOCKED` events for bound HCI control sockets.  The same
staging table and event path is reachable from mgmt control commands:
`btctl upstream mgmt-socket 0x0026 0 1` sends `MGMT_OP_BLOCK_DEVICE` with a
synthetic BR/EDR address, and `btctl upstream mgmt-socket 0x0027 0 1` sends
`MGMT_OP_UNBLOCK_DEVICE`.  `btctl upstream mgmt-socket 0x0031 0 <addr-seed>`
now sends `MGMT_OP_GET_CONN_INFO` and `btctl upstream mgmt-socket 0x0032 0
<addr-seed>` sends `MGMT_OP_GET_CLOCK_INFO`; both commands query the
hci-conn-like staging snapshot and return Linux response shapes with staging
RSSI/TX-power or clock values.  `btctl upstream mgmt-socket 0x0033 0
<addr-seed>` sends `MGMT_OP_ADD_DEVICE` with action `0x01`, stores the
synthetic BR/EDR address in a small staging device list, and emits
`MGMT_EV_DEVICE_ADDED`; `btctl upstream mgmt-socket 0x0034 0 <addr-seed>`
sends `MGMT_OP_REMOVE_DEVICE`, removes the same entry, and emits
`MGMT_EV_DEVICE_REMOVED`.  The same staging device-list entry now carries
Linux mgmt device flags too: `btctl upstream mgmt-socket 0x004f 0 <addr-seed>`
queries `MGMT_OP_GET_DEVICE_FLAGS`, while `btctl upstream mgmt-socket 0x0050 0
<addr-seed>` sets `MGMT_OP_SET_DEVICE_FLAGS` and emits
`MGMT_EV_DEVICE_FLAGS_CHANGED` to other bound control sockets.  This requires a
previous `MGMT_OP_ADD_DEVICE`, matching Linux's accept-list/conn-params
ownership model.
Discovery state is likewise observable through the persistent mgmt listener:
`btctl upstream mgmt-listen` followed by `btctl upstream mgmt-socket 0x0023 0
7` from another command context queues `MGMT_EV_DISCOVERING`, and
`btctl upstream mgmt-socket 0x0024 0 7` queues the matching stop event.
`btctl upstream mgmt-poll-discovery [max]` bridges peer hwsim ADV raw records
into Linux-shaped `MGMT_EV_DEVICE_FOUND` events while discovery is active; the
current bridge synthesizes a stable LE public address from the peer sim role
and emits Complete Local Name EIR data from the ADV payload's `name=` field.
The pairing control edge is staged as well: `btctl upstream mgmt-socket 0x0019
0 <addr-seed>` sends `MGMT_OP_PAIR_DEVICE` with a synthetic LE public address
and NoInputNoOutput IO capability, creates or updates the matching staging
device-list entry, marks it paired, and returns `mgmt_rp_pair_device`.
`btctl upstream mgmt-socket 0x001a 0 <addr-seed>` sends
`MGMT_OP_CANCEL_PAIR_DEVICE`; because the current shim completes pair
immediately, cancel normally returns invalid params unless a future pending
pair state is active.  `btctl upstream mgmt-socket 0x001b 0 <addr-seed>` sends
`MGMT_OP_UNPAIR_DEVICE` with `disconnect=1`, clears the paired state, optionally
clears a matching connection snapshot, returns `mgmt_rp_unpair_device`, and
emits `MGMT_EV_DEVICE_UNPAIRED` to other bound control sockets.
The current connection rows now come from a small hci-conn-like staging
snapshot table updated by L2CAP/ISO bind, connect, listen, handle assignment
and release paths.  This keeps HCI ioctl code from peeking directly into
protocol socket arrays and makes the temporary layer closer to upstream
`hci_conn_hash`; once upstream `hci_conn.c` is fully enabled, the snapshot
should be replaced by the native hash path.  Bound HCI control sockets are now
also tracked as staging mgmt event listeners: when the snapshot transitions to
`BT_CONNECTED`, the control sockets receive Linux-shaped
`MGMT_EV_DEVICE_CONNECTED`; when a connected snapshot is released or leaves
`BT_CONNECTED`, they receive `MGMT_EV_DEVICE_DISCONNECTED`.  The connected
event currently carries empty EIR data and is a bridge toward upstream
`mgmt_device_connected()`.  `btctl upstream mgmt-listen` keeps a control
socket bound so later `btctl upstream mgmt-read [max]` can drain connection
and device-list/block-list asynchronous mgmt events; `btctl upstream mgmt-close` releases it.  The
`HCISET*` actions currently record the
requested state in the staging `hci_dev` fields; real synchronous controller
commands remain upstream `hci_core.c` work.
`l2cap` now reaches a guarded staging `BTPROTO_L2CAP` create/release/bind
boundary: the fallback `l2cap_init()` registers a Linux-shaped proto/family
pair and the probe allocates `struct l2cap_pinfo` through upstream
`bt_sock_alloc()`.  `btctl upstream socket l2cap <psm> [cid]` then builds an
imported upstream `struct sockaddr_l2` bind request, records the local PSM/CID,
and moves the socket to `BT_BOUND`.  Connect/send/recv still intentionally
return unsupported until the real upstream L2CAP socket/channel files are
enabled.
`iso` now has the same narrow create/release/bind staging boundary for the LE
Audio side: `btctl upstream socket iso [addr-type]` allocates an ISO staging
socket through `bt_sock_alloc()`, uses imported upstream `struct sockaddr_iso`
for bind, records the local address type, and moves the socket to `BT_BOUND`.
Connect/send/recv still intentionally return unsupported until upstream
`iso.c` replaces the temporary boundary.
The generic socket probe keeps HCI on `SOCK_RAW` and uses `SOCK_SEQPACKET` for
L2CAP/ISO when that socket type is available, matching the BlueZ-facing shape
more closely.
`btctl upstream iso-send <addr-type> <handle> <hex...>` exercises the first
ISO socket data-plane staging path.  It creates a `BTPROTO_ISO` socket, binds
it with imported upstream `struct sockaddr_iso`, injects the supplied handle
into the temporary ISO private object, sends the payload through the socket
`sendmsg()` op, and packages it as an HCI ISO data packet before calling the
upstream VHCI send path.  This is a probe for the BlueZ ISO socket -> kernel ->
VHCI direction; real ISO connection/session semantics still belong to upstream
`iso.c`.
`btctl upstream l2cap-send <psm> <cid> <handle> <hex...>` is the matching
L2CAP socket data-plane staging hook for the A2DP side.  It creates and binds a
`BTPROTO_L2CAP` socket using imported upstream `struct sockaddr_l2`, injects a
temporary ACL handle, sends the user payload through the socket `sendmsg()` op,
and wraps it as HCI ACL data with an L2CAP basic header before entering the
upstream VHCI send path.  Real L2CAP connection, CID allocation, mode handling
and socket ownership still belong to upstream `l2cap_sock.c`/`l2cap_core.c`.
The controller-to-host direction now has matching staging fanout: when
`hci_recv_frame()` receives HCI ACL data, the shim parses the ACL/L2CAP headers
and queues the L2CAP payload into matching bound L2CAP sockets by CID.  HCI ISO
data is parsed and queued into bound ISO sockets as ISO SDU payload.  This gives
future BlueZ-like tools a receive queue at the protocol socket layer, while full
reassembly, flow control and connection ownership remain upstream work.
For nsh-driven two-terminal experiments, `btctl upstream l2cap-bind <psm>
<cid> <handle>` and `btctl upstream iso-bind <addr-type> <handle>` keep one
staging protocol socket alive inside the sim instance.  After a peer sends data
and the local terminal pumps/polls the hwsim medium, `btctl upstream
l2cap-recv [max]` or `btctl upstream iso-recv [max]` reads the queued protocol
payload as hex.  These persistent probe sockets are test scaffolding only; real
BlueZ ownership must come from upstream sockets.
After a persistent bind, `btctl upstream l2cap-write <hex...>` and `btctl
upstream iso-write <hex...>` reuse the kept socket for repeated sends.  The
older `l2cap-send`/`iso-send` commands are one-shot helpers that create, bind,
send and release in one command.
`btctl upstream l2cap-connect <psm> <cid>` and `btctl upstream iso-connect
<addr-type>` advance the kept socket from `BT_BOUND` to `BT_CONNECTED` through
the protocol `connect` op.  This is still a narrow staging state transition,
not real link establishment, but it aligns the observable socket lifecycle with
native Linux more closely.
`btctl upstream l2cap-listen [backlog]` advances the kept L2CAP socket to
`BT_LISTEN` through the protocol `listen` op.  ACL/L2CAP receive fanout accepts
`BT_LISTEN` sockets as valid targets, so a receiver terminal can model
server-side profile/media CID intake before upstream accept/channel ownership
is enabled.
`btctl upstream l2cap-close` and `btctl upstream iso-close` explicitly release
the kept staging socket and clear the associated bind state, matching the
socket close/release lifecycle that real BlueZ-owned sockets will use.

Minimal two-terminal staging flow:

```text
# receiver terminal
btctl upstream l2cap-bind 0x0019 0x0041 0x0040
btctl upstream l2cap-listen
btctl upstream pump
btctl upstream l2cap-recv
btctl upstream l2cap-close

# sender terminal
btctl upstream l2cap-bind 0x0019 0x0041 0x0040
btctl upstream l2cap-connect 0x0019 0x0041
btctl upstream l2cap-write 01 02 03 04
btctl upstream pump

# receiver terminal, ISO variant
btctl upstream iso-bind 0 0x0101
btctl upstream iso-connect 0
btctl upstream pump
btctl upstream iso-recv
btctl upstream iso-close

# sender terminal, ISO variant
btctl upstream iso-bind 0 0x0101
btctl upstream iso-connect 0
btctl upstream iso-write 01 02 03 04
btctl upstream pump
```

The upstream `btaudio` commands now queue synthetic audio media through the
staged Linux-like protocol sockets first.  `upstream-a2dp-source start` uses a
`BTPROTO_L2CAP` socket sendmsg path before the payload is packaged as HCI ACL,
and `upstream-le-broadcast-source start` uses a `BTPROTO_ISO` socket sendmsg
path before the payload is packaged as HCI ISO.  This keeps the visible path
closer to BlueZ user space -> Bluetooth socket -> kernel/VHCI -> hwsim medium;
full A2DP/LE Audio protocol behavior still belongs in the imported Linux
Bluetooth stack as the compat layer grows.
The matching sink commands keep the receive side on protocol sockets too:
`upstream-a2dp-sink start` binds and listens on the staged L2CAP media CID,
`upstream-a2dp-sink read [max]` polls the hwsim medium and drains the L2CAP
socket receive queue, and `upstream-a2dp-sink stop` releases the socket.
`upstream-le-broadcast-sink sync [big] [bis]` binds/connects the staged ISO
socket for the BIG/BIS-derived handle, `start [big] [bis] [max]` polls and
receives ISO SDU payload, and `stop` releases it.

These commands are staging and verification frontends.  Long term, behavior
that Linux implements in kernel `net/bluetooth` or `drivers/bluetooth` should
live under `nuttx/`, while user-space orchestration and BlueZ-like test/control
tools should live under `apps/`.

## 2026-06-11 mgmt IO capability probe

`btctl upstream mgmt-socket` 新增 `MGMT_OP_SET_IO_CAPABILITY` 探针：

```bash
btctl upstream mgmt-socket 0x0018 0 3
btctl upstream mgmt-socket 0x0019 0 1
```

`0x0018` 会更新 staging controller 默认 IO capability；后续 `0x0019` 构造的 `mgmt_cp_pair_device.io_cap` 会跟随该值，便于让 BlueZ-like 配对流程更接近 Linux mgmt 行为。

## 2026-06-11 pairing user confirmation probe

`btctl upstream mgmt-socket` 现在可以构造并发送 pairing user confirmation reply：

```bash
btctl upstream mgmt-socket 0x0018 0 1
btctl upstream mgmt-socket 0x0019 0 1
btctl upstream mgmt-socket 0x001c 0 1
```

如果改用否认：

```bash
btctl upstream mgmt-socket 0x001d 0 1
```

这用于验证 BlueZ-like mgmt control socket 的 pairing reply binary shape。完整 Linux 行为仍要继续接 upstream SMP/pending command/key distribution。

## 2026-06-11 mgmt pending pair ownership

pairing confirmation probe 现在有 staged pending-command ownership：非 `NoInputNoOutput` 的 `PAIR_DEVICE` 会保持 pending，不再立即返回 pair command-complete；`USER_CONFIRM_REPLY` / `USER_CONFIRM_NEG_REPLY` 会结束 pending 并让原 pair command 得到最终 complete。

这个行为是为了贴近 BlueZ 真实 mgmt socket 模型：用户态发起 pair 后等待内核后续事件和最终 complete，而不是一次 sendmsg 就同步成功。当前 `btctl upstream mgmt-socket` 仍是短连接探针；真正 BlueZ 常驻 mgmt socket 接入后，pending complete 才会被同一个用户态 control socket 持续读取。

## 2026-06-11 persistent mgmt-send command

`btctl upstream mgmt-send <opcode> [index] [param]` 会通过 `mgmt-listen` 已经打开的 persistent HCI control socket 发送 mgmt command。它用于替代一次性 `mgmt-socket` 探针，观察更接近 BlueZ daemon 的异步控制面：

```bash
btctl upstream mgmt-listen
btctl upstream mgmt-send 0x0018 0 1
btctl upstream mgmt-read
btctl upstream mgmt-send 0x0019 0 1
btctl upstream mgmt-read
btctl upstream mgmt-send 0x001c 0 1
btctl upstream mgmt-read
```

后续 BlueZ 常驻 mgmt fd 接入时，应沿用同一类 socket 生命周期，而不是每条命令新建一个 socket。

## 2026-06-11 passkey pairing probe

`btctl upstream mgmt-send` 现在可覆盖 staged passkey pairing branch：

```bash
btctl upstream mgmt-listen
btctl upstream mgmt-send 0x0018 0 2
btctl upstream mgmt-read
btctl upstream mgmt-send 0x0019 0 1
btctl upstream mgmt-read
btctl upstream mgmt-send 0x001e 0 1
btctl upstream mgmt-read
```

其中 `0x001e` 使用固定 staged passkey `123456`。否认 passkey 使用：

```bash
btctl upstream mgmt-send 0x001f 0 1
```

这只验证 mgmt control socket 的 passkey request/reply shape；真实 passkey 比较、SMP confirm/random 和 key distribution 仍等待 upstream `smp.c` 接管。

## 2026-06-11 staged LTK mgmt event

配对成功后，staging mgmt control path 会发送 `MGMT_EV_NEW_LONG_TERM_KEY`。因此用 persistent mgmt socket 跑 passkey 或 user-confirm 成功路径时，`mgmt-read` 除了 pairing request/complete 外，还可以看到 staged LTK event。

这只是 BlueZ-observable mgmt event shape，不是完整 SMP 密钥派生；后续仍要由 imported `smp.c` 生成真实 LTK/IRK/CSRK，并接持久 key store。

## 2026-06-11 hwsim usecase command matrix

`tools/firmware/sim/test-bt-hwsim-usecases.sh` 现在可以生成 `btctl` / `btaudio` 的 per-terminal nsh 用例文件：

```bash
tools/firmware/sim/test-bt-hwsim-usecases.sh write
```

覆盖 `bt-basic`、`ble-basic`、`mgmt-noio`、`mgmt-confirm`、`mgmt-passkey`、`a2dp` 和 `le-audio`。这些用例是当前 CLI 到 Linux-port core 的验证入口，后续自动化应优先复用这组命令矩阵，而不是再手写分散命令。

## 2026-06-11 hwsim usecase log validator

`tools/firmware/sim/validate-bt-hwsim-usecases.py` 会读取 `<case>.<role>.log` 并校验 `btctl` / `btaudio` 的关键输出。CLI 变更或 upstream 替换后，应优先跑对应用例并用 validator 记录 PASS/FAIL：

```bash
tools/firmware/sim/validate-bt-hwsim-usecases.py \
  --log-dir build/bt-hwsim-usecases --case mgmt-passkey
```

## 2026-06-11 hwsim usecase runner

`tools/firmware/sim/run-bt-hwsim-usecases.py` 会真实启动需要的 sim role，喂入 `btctl` / `btaudio` 命令，采集日志并调用 validator。CLI 行为变更后，可以用单用例快速回归：

```bash
tools/firmware/sim/run-bt-hwsim-usecases.py --case mgmt-passkey
```

本 runner 依赖四个 `build/nuttx-sim-<role>` 已存在。

## 2026-06-11 runner result manifest

`run-bt-hwsim-usecases.py` now writes `run-results.json` next to the per-role logs.  This records the tested cases, role log paths, run errors, validator return code and aggregate PASS/FAIL so CLI regressions can be attached to the porting notes.

## 2026-06-11 usecase preflight result

Preflight status: runner/validator Python syntax PASS, usecase list PASS, but all four sim binaries are currently missing:

```text
build/nuttx-sim-bt1
build/nuttx-sim-bt2
build/nuttx-sim-ble1
build/nuttx-sim-ble2
```

Use `tools/firmware/sim/preflight-bt-hwsim-usecases.sh` before running CLI regression usecases.

## btvhcid external H4 input file endpoint

`btvhcid` now accepts an optional host-side H4 binary input file:

```sh
btvhcid --h4-in /tmp/nuttx-btvhcid-h4in.bin [--h4-in-max N]
```

The file is read through the SIM host-file hwsim layer, not through ordinary
NuttX app VFS.  This matters because the app namespace does not necessarily see
host `/tmp`, while the hwsim host layer does.  The reader supports H4 command,
ACL, SCO, event, ISO and vendor packet framing and keeps a per-role offset file
under the hwsim medium directory:

```text
/tmp/nuttx-bthwsim/bt-h4-in.role<role>.off
```

Each complete H4 frame is injected into the upstream Linux VHCI file with
`linux_bt_upstream_vhci_write_default()`.  This is the first file-backed shape
for an external controller-side H4 producer, and is intended as a stepping stone
toward a BlueZ `btproxy`/`btvirt` style socket bridge.

## btvhcid external H4 output tee

`btvhcid` can also tee upstream VHCI host-to-controller H4 output to a host-side
binary file while still forwarding ACL/ISO packets to the hwsim medium:

```sh
btvhcid --h4-out /tmp/nuttx-btvhcid-h4out.bin
```

The output path is handled by the SIM host-file hwsim layer, matching
`--h4-in`.  Absolute paths are written as-is; relative paths are written under
the hwsim medium directory.  The tee records the raw H4 bytes before packet type
filtering, so HCI command/vendor packets are preserved for external tools even
though only ACL/ISO are mapped onto the simulated air medium.

This gives `btvhcid` a file-backed bidirectional H4 boundary:

```text
external H4 file input  -> btvhcid -> upstream VHCI
upstream VHCI H4 output -> btvhcid -> external H4 file output
upstream VHCI ACL/ISO   -> btvhcid -> hwsim public-file medium
```

## btvhcid `tcp:<port>` H4 endpoint staging

`btvhcid` accepts `tcp:<port>` anywhere a H4 path is accepted:

```sh
btvhcid --h4-in tcp:9001 --h4-out tcp:9001
```

This is implemented in the SIM host hwsim layer.  The current code creates a
host-side `127.0.0.1:<port>` TCP listener and preserves the same H4 read/append
abstraction used by file endpoints.  The intended direction is:

```text
external TCP H4 stream -> linux_bt_hwsim_h4_read("tcp:PORT") -> VHCI
VHCI drain             -> linux_bt_hwsim_h4_append("tcp:PORT") -> TCP stream
```

Current validation status: PASS via `tools/firmware/sim/run-btvhcid-h4tcp-smoke.py`.  The listener accepts a host H4 stream, receives the 7-byte Command Complete event, and the event reaches the upstream HCI event path.  The endpoint is still a lightweight staging bridge for BlueZ/btproxy integration, but the basic accept/recv/VHCI injection path is now verified.

## btvhcid H4 TCP endpoint

`btvhcid` now supports `tcp:<port>` pseudo paths for H4 ingress and egress in addition to file-backed H4 paths.

Example from `nsh`:

```sh
btvhcid --rounds 20 --delay-ms 100 --h4-in tcp:9001 --h4-out tcp:9001
```

A host-side process can inject an H4 packet with:

```sh
python3 - <<'PY'
import socket, time
s = socket.create_connection(('127.0.0.1', 9001), timeout=3)
s.sendall(bytes([0x04, 0x0e, 0x04, 0x01, 0x03, 0x0c, 0x00]))
time.sleep(1)
s.close()
PY
```

Validated behavior:

- The sim host bthwsim layer listens on `127.0.0.1:<port>`.
- A received H4 event is injected into the upstream VHCI path and is observed as an HCI event by the Linux Bluetooth semantic layer.
- VHCI drained H4 packets can be teed back to the connected TCP peer when `--h4-out tcp:<port>` is enabled.

Current limitation:

- The TCP endpoint is a staging bridge for host-tool integration. File-backed H4 paths remain the most deterministic regression path. Short `btvhcid` runs may receive TCP bytes in the host background thread after the command exits; the rx buffer is retained and can be consumed by a later pump round.

## btvhcid H4 TCP smoke test

A host-side smoke test is available at:

```sh
tools/firmware/sim/run-btvhcid-h4tcp-smoke.py
```

Default behavior:

```text
build/nuttx-sim-bt1 -> nsh -> btvhcid --h4-in tcp:9101 --h4-out tcp:9101
host TCP client     -> H4 Command Complete event 04 0e 04 01 03 0c 00
```

The test passes only when the SIM transcript contains the TCP receive marker and the upstream HCI event markers.  Latest result:

```text
btvhcid H4 TCP smoke: PASS
```

Latest log:

```text
build/logs/run-btvhcid-h4tcp-smoke.log
```

After rebuilding all four hwsim roles, the smoke test was run again and still returned `btvhcid H4 TCP smoke: PASS`.

## btvhcid post-drain H4 pump

`btvhcid` now pumps optional H4 input twice per round: once at the beginning of the round and once after VHCI drain.  The post-drain pump lets TCP H4 frames that arrive during poll/drain processing enter the upstream VHCI path during the same `btvhcid` command run.

Validated result:

```text
build/logs/run-btvhcid-h4tcp-post-pump.log
```

The transcript contains the injected H4 event and `total-h4-in=1` inside the initial `btvhcid --rounds 20 ...` run, with no fallback `btvhcid --rounds 1` command.

A synchronous recv-in-read experiment was attempted and reverted because it stalled after TCP accept in the SIM smoke test.  The stable model remains host pthread accept/recv plus H4 frame consumption from the retained rx buffer.

## btvhcid H4 TCP controller-side smoke

A bidirectional host-side controller smoke test is available at:

```sh
tools/firmware/sim/run-btvhcid-h4tcp-controller-smoke.py
```

It connects to `btvhcid --h4-in tcp:<port> --h4-out tcp:<port>`, reads standard H4 command packets drained from the upstream VHCI path, and replies with HCI Command Complete events for the observed opcodes.

Latest result:

```text
btvhcid H4 TCP controller smoke: PASS
commands=0x0c03,0x1003,0x1001,0x1009,0x1002,0x0c52,0x0c45,0x0c58,0x1004,0x1005,0x0c23,0x0c14,0x0c25,0x0c38,0x0c39,0x0c16,0x0c01
replies=17
```

The TCP H4 endpoint now keeps a small TX backlog when no controller peer is connected yet and flushes it after accept.  External H4 output is filtered to standard H4 packet types only; internal `0xff` pseudo records stay in trace output and are not exposed to the external H4 stream.

## BlueZ btproxy H4 TCP smoke

A BlueZ userspace boundary smoke test is available at:

```sh
tools/firmware/sim/run-bluez-btproxy-h4tcp-smoke.py
```

It starts `btvhcid --h4-in tcp:<port> --h4-out tcp:<port>` in NuttX SIM and then runs the rebuilt BlueZ tool:

```sh
/home/uan/Feather-develop-BT/third/bluez/tools/btproxy --connect 127.0.0.1 --port <port> --debug
```

Latest result:

```text
BlueZ btproxy H4 TCP smoke: PARTIAL
btproxy connected to NuttX H4 TCP, but host /dev/vhci is unavailable
```

This proves the real BlueZ `btproxy` process reaches the NuttX H4 TCP endpoint.  The remaining host-side prerequisite for a full btproxy loop is an available `/dev/vhci` device.

Regression checks after this addition:

```text
btvhcid H4 TCP smoke: PASS
btvhcid H4 TCP controller smoke: PASS
```

## BlueZ/VHCI host preflight

Use this before expecting BlueZ `btproxy --connect` to form a full host VHCI loop:

```sh
tools/firmware/sim/preflight-bluez-vhci.sh
```

Latest result: rebuilt BlueZ `btproxy` and `btvirt` are executable, but host `/dev/vhci` is owned by root and not readable/writable by the current user:

```text
FAIL: /dev/vhci exists but current user lacks read/write access
crw------- 1 root root 10, 137 Jun 10 17:35 /dev/vhci
```

`run-bluez-btproxy-h4tcp-smoke.py` now embeds this preflight in its log.  Current status remains `PARTIAL`: BlueZ `btproxy` connects to the NuttX H4 TCP endpoint, then fails when opening host `/dev/vhci`.

## BlueZ btvirt/btdev direct TCP relay smoke

Use this smoke when host `/dev/vhci` is unavailable but rebuilt BlueZ userspace emulator coverage is still needed:

```sh
tools/firmware/sim/run-bluez-btvirt-h4tcp-relay-smoke.py
```

It starts BlueZ `emulator/btvirt -t<port> -d`, starts `btvhcid --h4-in tcp:<port> --h4-out tcp:<port>` in NuttX SIM, and relays both TCP streams.  This exercises BlueZ `emulator/server.c` and `emulator/btdev.c` directly without using host `/dev/vhci`.

Latest result:

```text
BlueZ btvirt H4 TCP relay smoke: PASS
nuttx_to_btvirt=321
btvirt_to_nuttx=481
```

Regression checks after this addition:

```text
btvhcid H4 TCP smoke: PASS
btvhcid H4 TCP controller smoke: PASS
```

## H4 `connect:<port>` client endpoint

`btvhcid` H4 paths now support client mode:

```sh
btvhcid --h4-in connect:45552 --h4-out connect:45552
```

This connects to `127.0.0.1:<port>` and reuses the same H4 framing path as `tcp:<port>`.  It is intended for direct connection to BlueZ `btvirt -t<port>` without requiring a host relay or `/dev/vhci`.

Latest direct BlueZ emulator result:

```text
BlueZ btvirt H4 TCP direct smoke: PASS
```

Smoke script:

```sh
tools/firmware/sim/run-bluez-btvirt-h4tcp-direct-smoke.py
```

Networking follow-up: the project goal now includes Bluetooth IP networking validation with `ping` and `iperf`.  That requires adding the Linux-aligned BNEP/PAN path for BR/EDR and/or LE 6LoWPAN/L2CAP CoC path for BLE, plus binding the resulting network device semantics into NuttX networking.

## Bluetooth networking readiness

Bluetooth IP networking is now tracked by:

```sh
tools/firmware/sim/preflight-bt-networking.sh
```

Latest status: upstream Linux and BlueZ source/test candidates are present.  FeatherCore now has BNEP source import, BNEP socket/ioctl staging, an initial BNEP netdev bridge boundary, and hwsim defconfig IP/ping/iperf settings for all four roles.  It still lacks full BNEP session/L2CAP ownership, NuttX IP TX/RX handoff, BlueZ networking apps wiring, and BLE 6LoWPAN.

Recommended first networking target: BR/EDR PAN over BNEP, then `ping`/`iperf` between hwsim BT roles.  BLE IP should follow through Linux Bluetooth 6LoWPAN or LE L2CAP CoC after the netdev/IP plumbing is proven.

## Four-role IP test command staging

The four hwsim role defconfigs now enable the NuttX-side IP test surface needed
before a real Bluetooth network carrier can be attached:

```text
CONFIG_NET_IPv4=y
CONFIG_NET_TCP=y
CONFIG_NET_UDP=y
CONFIG_NET_ICMP_SOCKET=y
CONFIG_NET_SOCKOPTS=y
CONFIG_SYSTEM_PING=y
CONFIG_NETUTILS_IPERF=y
CONFIG_NET_TUN=y
CONFIG_SIM_NETDEV=y
CONFIG_NETUTILS_IPERFTEST_DEVNAME="btn0"
```

Validation logs:

```text
build/logs/build-bt1-ip-ping-iperf.log
build/logs/build-bt2-ip-ping-iperf.log
build/logs/build-ble1-ip-ping-iperf.log
build/logs/build-ble2-ip-ping-iperf.log
build/logs/preflight-bt-networking-ip-ping-iperf.log
```

This means all four NSH images can carry `ping` and `iperf`; it does not yet
mean the Bluetooth carrier exists.  Cross-device `ping`/`iperf` still requires
the BNEP or BLE 6LoWPAN data plane to create a real IP-facing network
interface.

## BNEP ioctl session lifecycle staging

`btctl upstream bnep-ioctl ...` now exercises a persistent Linux-shaped BNEP
session table in the upstream AF_BLUETOOTH compatibility layer.  The usecase
first creates a kept L2CAP socket and then creates the BNEP session from that
socket:

```text
btctl upstream l2cap-bind 0x0019 0x0041 0x0040
btctl upstream l2cap-connect 0x0019 0x0041
btctl upstream bnep-ioctl connadd l2cap
btctl upstream bnep-ioctl connlist
btctl upstream bnep-ioctl conninfo l2cap
btctl upstream bnep-ioctl conndel l2cap
btctl upstream l2cap-close
```

Validated usecase:

```text
tools/firmware/sim/run-bt-hwsim-usecases.py --case bnep-session
build/logs/build-bt1-bnep-l2cap-session.log
build/logs/build-bt1-bnep-l2cap-session-final.log
build/logs/run-bt-hwsim-bnep-l2cap-session.log
build/bt-hwsim-usecases-bnep-l2cap-session/bnep-session.bt1.log
```

The validator requires the Linux BNEP ioctl lifecycle to show
`BNEPCONNADD`, `BNEPGETCONNLIST`, `BNEPGETCONNINFO` and `BNEPCONNDEL` with a
connected staging state and `btn-l2` device name.  This keeps the observable
userspace ABI aligned with Linux BNEP `sock.c` while BlueZ-style fd lookup and
the IP-facing netdev handoff are still being ported.

## BNEP netdev lifecycle staging

The BNEP session staging table now also carries a Linux-like network-device
lifecycle marker.  On `BNEPCONNADD l2cap`, the active session reports:

```text
bnep-netdev-registered=1
bnep-netdev-name=btn-l2
bnep-netdev-mtu=1500
```

On `BNEPCONNDEL l2cap`, the same probe reports:

```text
bnep-netdev-registered=0
```

This is still not the final NuttX IP handoff.  It is the staging seam that will
be replaced by real `register_netdev()` / `unregister_netdev()` integration
through the shared mac80211/Linux netdevice compatibility bridge.

Local non-network checks for this step:

```text
build/logs/show-bt-hwsim-bnep-netdev-staging-case.log
build/logs/list-bt-hwsim-usecases-after-bnep-netdev.log
build/logs/pycompile-bt-hwsim-usecases-after-bnep-netdev.log
```

Full `build-bt1.sh` validation was attempted in offline mode and recorded in:

```text
build/logs/build-bt1-bnep-netdev-staging.log
```

It stopped at the local NuttX apps dependency download for
`argtable3-3.2.0.7402e6e`.  No external network retry was performed.

## Linux BNEP/PAN source import

Linux BR/EDR PAN networking source is now imported under:

```text
nuttx/wireless/linux_bluetooth/upstream/net_bluetooth/bnep/
```

Imported files:

```text
core.c
sock.c
netdev.c
bnep.h
PORTING.md
```

Build switch:

```text
CONFIG_NET_LINUX_BLUETOOTH_UPSTREAM_BNEP
```

The switch is intentionally off by default.  The next porting work is BNEP session creation over a connected L2CAP socket, NuttX netdev TX/RX packet handoff, and BlueZ network profile or `bneptest` wiring for PAN session creation.

## BNEP hwsim `btn0` ping milestone

The first BT1 <-> BT2 IP networking milestone now passes without depending on `iperf`.

Implemented path:

```text
kept L2CAP socket
  -> BNEPCONNADD l2cap
  -> btn0 NuttX Ethernet-like lower-half netdev
  -> LINUX_BT_HWSIM_TYPE_BNEP public-file medium
  -> peer btn0
```

Notes:

- `linux_bt_bnep_netdev.c` now always builds with `CONFIG_NET_LINUX_BLUETOOTH` and provides the temporary hwsim PAN lower-half netdev used by the staged BNEP session table.
- `BNEPCONNADD l2cap` returns/registers `btn0`; `BNEPCONNDEL l2cap` unregisters it.
- `bt-hwsim-bnep.bin` carries Ethernet frames separately from HCI ACL and ISO records.
- This is still a simulator bridge, not yet the full imported Linux BNEP `core.c` session thread.

Validation:

```text
tools/firmware/sim/run-bt-hwsim-usecases.py --case bnep-ping --out-dir build/bt-hwsim-usecases-bnep-ping-btn0
```

Result:

```text
PASS bnep-ping
  PASS bt1 build/bt-hwsim-usecases-bnep-ping-btn0/bnep-ping.bt1.log
  PASS bt2 build/bt-hwsim-usecases-bnep-ping-btn0/bnep-ping.bt2.log
```

The test configures `btn0` as `10.77.0.1/24` on BT1 and `10.77.0.2/24` on BT2, then validates `0% packet loss` in both directions.

Remaining networking work:

- Replace the `btctl` kept-L2CAP sentinel with BlueZ network profile or `bneptest` fd ownership.
- Move packet TX/RX into the imported Linux BNEP `netdev.c/core.c` path.
- Re-enable `iperf` validation after `ping` remains stable.
