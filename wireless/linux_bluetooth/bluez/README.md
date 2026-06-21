## 2026-06-20 L2CAP socket type parity closeout

The A2DP daemon closeout now validates Linux L2CAP create-time socket type
coverage at the BlueZ-visible boundary.

- `BTPROTO_L2CAP` now accepts `SOCK_SEQPACKET`, `SOCK_STREAM`, `SOCK_DGRAM`,
  and `SOCK_RAW`.
- The A2DP media listen/connect/accept path continues to use
  `SOCK_SEQPACKET`.
- The old RAW fallback path was removed from the A2DP ordinary-socket probe.
- The source and sink closeout gates now require successful create/close
  evidence for `STREAM`, `DGRAM`, and `RAW` L2CAP sockets.

Validation:

- Build PASS:
  `build-bt1-l2cap-socket-type-parity.log`,
  `build-bt2-l2cap-socket-type-parity.log`,
  `build-ble1-l2cap-socket-type-parity.log`,
  `build-ble2-l2cap-socket-type-parity.log`.
- hwsim PASS:
  `run-l2cap-socket-type-parity.log` for
  `bluez-a2dp-current-complete-closeout`.
- Artifacts:
  `FeatherCore/build/bt-hwsim-l2cap-socket-type-parity/`.

Boundary: This closes L2CAP create-time type coverage and removes the A2DP RAW
fallback.  It does not claim complete parity for every L2CAP mode,
retransmission policy, or Linux capability check such as `CAP_NET_RAW`.

## 2026-06-20 SCO socket type parity closeout

The HFP/HSP ordinary socket closeout now validates Linux SCO socket type
semantics directly at the BlueZ-visible boundary.

- `BTPROTO_SCO` now accepts `SOCK_SEQPACKET` only.
- `SOCK_STREAM`, `SOCK_DGRAM`, and `SOCK_RAW` SCO creation are required to
  fail with `ESOCKTNOSUPPORT`.
- The old RAW fallback path was removed from both the NuttX socket frontend and
  the BlueZ HFP/HSP ordinary-socket probe.

Validation:

- Build PASS:
  `build-bt1-sco-socket-type-parity.log`,
  `build-bt2-sco-socket-type-parity.log`,
  `build-ble1-sco-socket-type-parity.log`,
  `build-ble2-sco-socket-type-parity.log`.
- hwsim PASS:
  `run-sco-socket-type-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifacts:
  `FeatherCore/build/bt-hwsim-sco-socket-type-parity/`.

Boundary: This removes the SCO RAW compatibility residue and matches Linux
7.0.10 create-time socket type behavior.  It does not claim complete
controller-level SCO/eSCO scheduling, codec negotiation, or synchronous audio
timing identity.

## 2026-06-20 ISO socket type parity closeout

The LE Audio ordinary socket closeout now validates Linux ISO socket type
semantics directly at the BlueZ-visible boundary.

- `BTPROTO_ISO` now accepts `SOCK_SEQPACKET` only.
- `SOCK_STREAM`, `SOCK_DGRAM`, and `SOCK_RAW` ISO creation are required to
  fail with `ESOCKTNOSUPPORT`.
- The old RAW fallback path was removed from both the NuttX socket frontend and
  the BlueZ LE Audio sockapi-closeout probe.

Validation:

- Build PASS:
  `build-bt1-iso-socket-type-parity.log`,
  `build-bt2-iso-socket-type-parity.log`,
  `build-ble1-iso-socket-type-parity.log`,
  `build-ble2-iso-socket-type-parity.log`.
- hwsim PASS:
  `run-iso-socket-type-parity.log` for
  `bluez-le-audio-bap-pacs-ascs-session`.
- Artifacts:
  `FeatherCore/build/bt-hwsim-iso-socket-type-parity/`.

Boundary: This removes the ISO RAW compatibility residue and matches Linux
7.0.10 create-time socket type behavior.  It does not claim complete
controller-level ISO scheduling or codec policy identity.

## 2026-06-20 RFCOMM/SCO socket Kconfig parity naming closeout

The RFCOMM/SCO socket paths now use parity-oriented Kconfig names in the active
implementation.

- `NET_LINUX_BLUETOOTH_RFCOMM_SOCKET_PARITY` gates the RFCOMM socket parity
  path.
- `NET_LINUX_BLUETOOTH_SCO_SOCKET_PARITY` gates the SCO/eSCO socket parity
  path.
- Implementation `#ifdef` blocks now use the parity symbols.
- The old `*_SOCKET_TRANSITIONAL` symbols remain deprecated aliases that select
  the new symbols for existing local configurations.

Validation:

- Build PASS:
  `build-bt1-socket-parity-kconfig.log`,
  `build-bt2-socket-parity-kconfig.log`,
  `build-ble1-socket-parity-kconfig.log`,
  `build-ble2-socket-parity-kconfig.log`.
- hwsim PASS:
  `run-hfp-socket-parity-kconfig.log` for
  `bluez-hfp-hsp-profile-closeout`.
- hwsim PASS:
  `run-cmtp-socket-parity-kconfig.log` for
  `bluez-hci-mgmt-socket-closeout-full`.
- hwsim PASS:
  `run-hidp-socket-parity-kconfig.log` for
  `bluez-hid-upstream-convergence-closeout`.
- Artifacts:
  `FeatherCore/build/bt-hwsim-socket-parity-kconfig-hfp/`,
  `FeatherCore/build/bt-hwsim-socket-parity-kconfig-cmtp/`,
  `FeatherCore/build/bt-hwsim-socket-parity-kconfig-hidp/`.

Boundary: This removes active implementation dependence on transitional Kconfig
names while keeping deprecated aliases for local configuration compatibility.

## 2026-06-20 RFCOMM/HIDP/CMTP proto identity parity closeout

The RFCOMM, HIDP, and CMTP compatibility sockets no longer expose temporary
`*-transition` proto names through their registered protocol identity.

- RFCOMM now registers as `RFCOMM`.
- HIDP now registers as `HIDP`.
- CMTP now registers as `CMTP`.
- The hwsim probes emit `proto-name=RFCOMM`, `proto-name=HIDP`, and
  `proto-name=CMTP`.
- The validator requires those markers in the HFP/HSP, HIDP, and HCI/MGMT
  closeout gates.

Validation:

- Build PASS:
  `build-bt1-proto-name-parity.log`,
  `build-bt2-proto-name-parity.log`,
  `build-ble1-proto-name-parity.log`,
  `build-ble2-proto-name-parity.log`.
- hwsim PASS:
  `run-rfcomm-proto-name-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- hwsim PASS:
  `run-cmtp-proto-name-parity.log` for
  `bluez-hci-mgmt-socket-closeout-full`.
- hwsim PASS:
  `run-hidp-proto-name-parity.log` for
  `bluez-hid-upstream-convergence-closeout`.
- Artifacts:
  `FeatherCore/build/bt-hwsim-rfcomm-proto-name-parity/`,
  `FeatherCore/build/bt-hwsim-cmtp-proto-name-parity/`,
  `FeatherCore/build/bt-hwsim-hidp-proto-name-parity/`.

Boundary: This closes the visible proto identity mismatch.  The follow-up
Kconfig parity naming closeout removes active implementation dependence on the
old transitional config names.

## 2026-06-20 HIDP/CMTP core session ownership closeout

HIDP/CMTP validation now requires Linux-core-shaped session and datapath
evidence behind the control socket lifecycle.

- HIDP/CMTP internal session objects use core-session naming rather than
  staged-session naming.
- HIDP `HIDPCONNADD` records linked session, session thread,
  control/interrupt readiness, input registration, UHID registration, and
  synthetic control/input/output traffic.
- CMTP `CMTPCONNADD` records linked session, worker thread, CAPI registration,
  reassembly readiness, datapath readiness, block id allocation, TX/RX frame
  activity, and CAPI message delivery.
- The validator now requires `core-session=...` and `core-traffic=...` for the
  HIDP and CMTP hwsim gates.

Validation:

- Build PASS:
  `build-bt1-hidp-cmtp-core-session-parity.log`,
  `build-bt2-hidp-cmtp-core-session-parity.log`,
  `build-ble1-hidp-cmtp-core-session-parity.log`,
  `build-ble2-hidp-cmtp-core-session-parity.log`.
- hwsim PASS:
  `run-cmtp-core-session-parity.log` for
  `bluez-hci-mgmt-socket-closeout-full`.
- hwsim PASS:
  `run-hidp-core-session-parity.log` for
  `bluez-hid-upstream-convergence-closeout`.
- Artifacts:
  `FeatherCore/build/bt-hwsim-cmtp-core-session-parity/`,
  `FeatherCore/build/bt-hwsim-hidp-core-session-parity/`.

Boundary: This proves the hwsim-visible HIDP/CMTP core-session lifecycle, but
does not yet claim unmodified upstream `hidp/core.c` and `cmtp/core.c` own every
internal worker, waitqueue, input/UHID object, CAPI controller, and skb
reassembly detail inside NuttX.

## 2026-06-20 HIDP/CMTP control socket poll parity closeout

HIDP and CMTP control socket validation now covers the Linux `proto_ops` poll
surface.

- Linux HIDP/CMTP sockets are ioctl-only control sockets and do not expose a
  poll operation in their `proto_ops`.
- The NuttX Linux Bluetooth compatibility layer no longer assigns
  `datagram_poll` to HIDP or CMTP.
- The HIDP and CMTP hwsim gates now require `poll-op-null=1` in the upstream
  socket probe output.

Validation:

- Build PASS:
  `build-bt1-hidp-cmtp-poll-parity.log`,
  `build-bt2-hidp-cmtp-poll-parity.log`,
  `build-ble1-hidp-cmtp-poll-parity.log`,
  `build-ble2-hidp-cmtp-poll-parity.log`.
- hwsim PASS:
  `run-cmtp-poll-parity.log` for
  `bluez-hci-mgmt-socket-closeout-full`.
- hwsim PASS:
  `run-hidp-poll-parity-upstream.log` for
  `bluez-hid-upstream-convergence-closeout`.
- Artifacts:
  `FeatherCore/build/bt-hwsim-cmtp-poll-parity/`,
  `FeatherCore/build/bt-hwsim-hidp-poll-parity-upstream/`.

Boundary: This closes the HIDP/CMTP control-socket poll ABI surface only.
HIDP UHID/input-core worker ownership and CMTP CAPI worker/datapath parity
remain separate convergence work.

## 2026-06-20 SCO SOL_SCO legacy option Linux parity closeout

HFP/HSP ordinary SCO socket validation now rejects non-Linux legacy
`SOL_SCO` options.

- Linux 7.0.10 SCO exposes `SCO_OPTIONS` and `SCO_CONNINFO` under `SOL_SCO`.
- `SO_SCO_MTU` and `SO_SCO_HANDLE` are no longer accepted as successful SCO
  socket options.
- The gate requires `legacy-sco-mtu-enoprotoopt=1` and
  `legacy-sco-handle-enoprotoopt=1`.
- SCO MTU evidence remains available through Linux `SOL_BLUETOOTH/BT_SNDMTU`
  and `BT_RCVMTU`.

Validation:

- Build PASS:
  `build-bt1-sco-sol-sco-legacy-option-parity.log`,
  `build-bt2-sco-sol-sco-legacy-option-parity.log`,
  `build-ble1-sco-sol-sco-legacy-option-parity.log`,
  `build-ble2-sco-sol-sco-legacy-option-parity.log`.
- hwsim PASS:
  `run-sco-sol-sco-legacy-option-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-sco-sol-sco-legacy-option-parity/`.

Boundary: This removes a compatibility-only SCO option surface; it does not
change the supported Linux SCO options already covered by the HFP/HSP closeout.

## 2026-06-20 RFCOMM BT_SECURITY Linux parity closeout

HFP/HSP ordinary RFCOMM socket validation now covers Linux's RFCOMM-specific
`SOL_BLUETOOTH/BT_SECURITY` bounds.

- `BT_SECURITY_HIGH` set/get succeeds on ordinary RFCOMM sockets.
- `BT_SECURITY_FIPS` set attempts are rejected with `EINVAL`, matching Linux
  `rfcomm_sock_setsockopt()`.
- The gate requires `btsec-level=3` and `btsec-fips-errno=22`.

Validation:

- Build PASS:
  `build-bt1-rfcomm-bt-security-parity.log`,
  `build-bt2-rfcomm-bt-security-parity.log`,
  `build-ble1-rfcomm-bt-security-parity.log`,
  `build-ble2-rfcomm-bt-security-parity.log`.
- hwsim PASS:
  `run-rfcomm-bt-security-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-rfcomm-bt-security-parity/`.

Boundary: This applies to RFCOMM only.  L2CAP's FIPS-capable security option
semantics are intentionally left unchanged.

## 2026-06-20 RFCOMM socket type Linux parity closeout

RFCOMM ordinary socket validation now covers the Linux create-time socket type
ABI.

- Linux RFCOMM accepts `SOCK_STREAM` and `SOCK_RAW`, but rejects
  `SOCK_SEQPACKET`.
- `linux_bt_rfcomm_sock_create()` now matches that allowed set.
- The HFP/HSP ordinary RFCOMM gate requires
  `seqpacket-esocktnosupport=1`, proving `SOCK_SEQPACKET+BTPROTO_RFCOMM`
  is rejected while the existing `SOCK_STREAM+BTPROTO_RFCOMM` path still
  succeeds.

Validation:

- Build PASS:
  `build-bt1-rfcomm-socket-type-parity.log`,
  `build-bt2-rfcomm-socket-type-parity.log`,
  `build-ble1-rfcomm-socket-type-parity.log`,
  `build-ble2-rfcomm-socket-type-parity.log`.
- hwsim PASS:
  `run-rfcomm-socket-type-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-rfcomm-socket-type-parity/`.

Boundary: This closes RFCOMM socket type creation parity only.  SCO
`SOCK_SEQPACKET` remains valid and separately covered by the SCO ordinary
socket gate.

## 2026-06-20 RFCOMM_LM Linux sockopt parity closeout

HFP/HSP ordinary RFCOMM socket validation now covers the Linux legacy
`SOL_RFCOMM/RFCOMM_LM` ABI instead of only the connected data, ioctl, poll,
timestamp, and `RFCOMM_CONNINFO` paths.

- RFCOMM LM constants now match Linux 7.0.10
  `include/net/bluetooth/rfcomm.h`.
- `setsockopt(SOL_RFCOMM, RFCOMM_LM)` maps AUTH/ENCRYPT/SECURE bits into the
  Linux security level and maps MASTER into RFCOMM role-switch state.
- `getsockopt(SOL_RFCOMM, RFCOMM_LM)` reconstructs the Linux-visible bitmask
  from socket state.
- `RFCOMM_LM_FIPS` set attempts are rejected with `EINVAL`, matching Linux
  `rfcomm_sock_setsockopt_old()`.
- The HFP/HSP ordinary RFCOMM gate requires `rfcomm-lm=0x00000027`, proving
  MASTER + AUTH + ENCRYPT + SECURE round-trip behavior.

Validation:

- Build PASS:
  `build-bt1-rfcomm-lm-sockopt-parity.log`,
  `build-bt2-rfcomm-lm-sockopt-parity.log`,
  `build-ble1-rfcomm-lm-sockopt-parity.log`,
  `build-ble2-rfcomm-lm-sockopt-parity.log`.
- hwsim PASS:
  `run-rfcomm-lm-sockopt-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-rfcomm-lm-sockopt-parity/`.

Boundary: This closes the RFCOMM LM security/role-switch ABI for the current
native RFCOMM socket path.  RFCOMM TTY remains explicitly out of this closeout
until the upstream TTY/line-discipline path is integrated.

## 2026-06-20 L2CAP BR/EDR BT_SNDMTU/BT_RCVMTU Linux parity closeout

L2CAP socket option handling now matches Linux 7.0.10 for BR/EDR sockets:
`BT_SNDMTU` and `BT_RCVMTU` are LE-only options and return `EINVAL` on
BR/EDR L2CAP sockets.

- `linux_bt_l2cap_sock_setsockopt()` rejects BR/EDR `BT_SNDMTU` and
  `BT_RCVMTU` before changing MTU state.
- `linux_bt_l2cap_sock_getsockopt()` rejects BR/EDR `BT_SNDMTU` and
  `BT_RCVMTU` before connected-state or MTU copyout checks.
- The BNEP fd-handoff gate now proves the `bredr-rejects-bt-mtu` behavior
  from implementation state rather than stale diagnostic text.

Validation:

- Build PASS:
  `build-bt1-l2cap-bredr-mtu-sockopt-linux-parity.log`,
  `build-bt2-l2cap-bredr-mtu-sockopt-linux-parity.log`,
  `build-ble1-l2cap-bredr-mtu-sockopt-linux-parity.log`,
  `build-ble2-l2cap-bredr-mtu-sockopt-linux-parity.log`.
- hwsim PASS:
  `run-l2cap-bredr-mtu-sockopt-linux-parity.log` for
  `bluez-bneptest-fd-handoff`.
- Artifact:
  `FeatherCore/build/bt-hwsim-l2cap-bredr-mtu-sockopt-linux-parity/`.

Boundary: This closes the BR/EDR side of Linux-visible L2CAP MTU sockopt
semantics.  A separate LE-positive CoC/IPSP MTU gate can still be added if the
next closeout targets LE connected-channel MTU behavior directly.

## 2026-06-20 HFP/HSP SCO BT_SNDMTU/BT_RCVMTU sockopt closeout

HFP/HSP SCO validation now covers the connected SCO MTU socket options exposed
by Linux `net/bluetooth/sco.c`.

- `bluez-hfp` ordinary SCO sockets call real
  `getsockopt(SOL_BLUETOOTH, BT_SNDMTU)` and
  `getsockopt(SOL_BLUETOOTH, BT_RCVMTU)`.
- The ordinary SCO gate requires both options to match `SCO_OPTIONS.mtu`
  before `final-ok=1`.
- `linux_bt_upstream_sco_socket_ioctl_probe()` emits matching upstream SCO
  proto_ops evidence.
- Validator coverage now distinguishes the ordinary BlueZ socket path
  (`ordinary-mtu-sockopt=SOL_BLUETOOTH`) from the upstream socket path
  (`sco-mtu-sockopt=SOL_BLUETOOTH`).

Validation:

- Build PASS:
  `build-bt1-sco-mtu-sockopt-parity-v2.log`,
  `build-bt2-sco-mtu-sockopt-parity-v2.log`,
  `build-ble1-sco-mtu-sockopt-parity-v2.log`,
  `build-ble2-sco-mtu-sockopt-parity-v2.log`.
- hwsim PASS:
  `run-sco-mtu-sockopt-parity-v2.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-sco-mtu-sockopt-parity-v2/`.

Boundary: This closes the BlueZ-visible connected SCO MTU sockopt surface for
the current HFP/HSP hwsim path.  SCO codec/offload policy and real controller
audio scheduling remain separate upstream-convergence work.

## 2026-06-20 BlueZ Network BNEP full sock_no proto_ops closeout

The BlueZ Network profile BNEP control fd now validates the same
`sock_no_*`-style unsupported operation matrix as Linux
`net/bluetooth/bnep/sock.c`.

- `bluez-network` probes real `bind`, `getsockname`, `getpeername`, `connect`,
  `send`, `recv`, `listen`, `shutdown`, and `accept` calls on the ordinary
  `AF_BLUETOOTH/SOCK_RAW/BTPROTO_BNEP` fd.
- Each non-applicable operation must report `EOPNOTSUPP` / `errno=95`.
- The gate records `no-data-ok=1` before continuing into
  `BNEPGETSUPPFEAT`, `BNEPCONNADD`, ping datapath, and `BNEPCONNDEL`.

Validation:

- Build PASS:
  `build-bt1-bnep-full-no-data-protoops.log`,
  `build-bt2-bnep-full-no-data-protoops.log`,
  `build-ble1-bnep-full-no-data-protoops.log`,
  `build-ble2-bnep-full-no-data-protoops.log`.
- hwsim PASS:
  `run-bnep-full-no-data-protoops.log` for `bluez-network-ping`.
- Artifact:
  `FeatherCore/build/bt-hwsim-bnep-full-no-data-protoops/`.

Boundary: This closes the BlueZ-visible BNEP control socket proto_ops surface.
The existing Network/BNEP native closeout and datapath gates continue to cover
netdev/session/thread ownership and ping traffic.

## 2026-06-20 connected L2CAP_CONNINFO evidence closeout

A2DP and LE Audio connected L2CAP option validation now records passing
`L2CAP_CONNINFO` evidence instead of the earlier
`conninfo-skip=needs-real-l2cap-conn` boundary.

- `linux_bt_upstream_l2cap_socket_connected_option_probe()` reads
  `SOL_L2CAP/L2CAP_CONNINFO` from the connected L2CAP handle.
- The hwsim gate requires `conninfo-ret=0`, `conninfo-handle=0x0052`, and
  `conninfo-dev-class=00:00:00`.
- The validator now rejects the old skip-only connected-option evidence for
  A2DP closeout.

Validation:

- Build PASS:
  `build-bt1-l2cap-connected-conninfo.log`,
  `build-bt2-l2cap-connected-conninfo.log`,
  `build-ble1-l2cap-connected-conninfo.log`,
  `build-ble2-l2cap-connected-conninfo.log`.
- hwsim PASS:
  `run-l2cap-connected-conninfo-v2.log` for
  `bluez-a2dp-current-complete-closeout` and
  `bluez-le-audio-bap-pacs-ascs-session`.
- Artifact:
  `FeatherCore/build/bt-hwsim-l2cap-connected-conninfo-v2/`.

Boundary: This closes the BlueZ-visible connected `L2CAP_CONNINFO` socket API
evidence for the current hwsim connected handle model.  Full imported
`l2cap_chan` / `l2cap_conn` / `hci_conn` lifetime ownership remains a deeper
upstream-convergence item.

## 2026-06-20 ordinary CMTP/HIDP full unsupported proto_ops closeout

The ordinary CMTP/HIDP raw-socket gate now covers the full user-callable
unsupported operation surface available from the NuttX application boundary.

- CMTP and HIDP ordinary fds now run real `bind`, `getsockname`,
  `getpeername`, `connect`, `sendmsg`, `recvmsg`, `listen`, `shutdown`, and
  `accept` probes.
- The gate requires `errno=95` / `EOPNOTSUPP` for the non-applicable
  operations and `unsupported-ok=1` before `final-ok=1`.
- `linux_bt_sockif_bind()` / `connect()` now return `EOPNOTSUPP` for the
  control protocols instead of protocol-family mismatch errors.
- `linux_bt_sockif_getname_common()` returns `EOPNOTSUPP` for BNEP, CMTP, and
  HIDP before connected-state peer checks, matching Linux `sock_no_getname`.

Validation:

- Build PASS:
  `build-bt1-cmtp-hidp-full-unsupported-protoops.log`,
  `build-bt2-cmtp-hidp-full-unsupported-protoops.log`,
  `build-ble1-cmtp-hidp-full-unsupported-protoops.log`,
  `build-ble2-cmtp-hidp-full-unsupported-protoops.log`.
- hwsim PASS:
  `run-cmtp-hidp-full-unsupported-protoops.log` for
  `bluez-hci-mgmt-socket-closeout-full` and
  `bluez-hid-upstream-convergence-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-cmtp-hidp-full-unsupported-protoops/`.

Boundary: This closes the ordinary CMTP/HIDP `sock_no_*`-style socket API
surface visible to BlueZ/NuttX applications.  CMTP worker/datapath and HIDP
UHID/input-core worker ownership remain deeper upstream-convergence work.

## 2026-06-20 ordinary CMTP/HIDP unsupported proto_ops parity closeout

CMTP/HIDP ordinary raw sockets now validate the non-applicable operation matrix
through real user-visible socket calls, not synthetic probe output.

- CMTP ordinary socket evidence requires `unsupported-ok=1` after real
  `sendmsg`, `recvmsg`, `listen`, `shutdown`, and `accept` attempts on the
  `BTPROTO_CMTP` fd.
- HIDP ordinary socket evidence requires the same `unsupported-ok=1` matrix on
  the `BTPROTO_HIDP` fd.
- `accept()` is now routed through Bluetooth proto_ops for `AF_BLUETOOTH` before
  NuttX applies the generic non-listening rejection, allowing CMTP/HIDP to match
  Linux `.accept = sock_no_accept` and return `EOPNOTSUPP`.
- L2CAP, ISO, RFCOMM, and SCO keep their listen-state guard inside
  `linux_bt_sockif_accept()`.

Validation:

- Build PASS:
  `build-bt1-cmtp-hidp-unsupported-ops-parity-v2.log`,
  `build-bt2-cmtp-hidp-unsupported-ops-parity-v2.log`,
  `build-ble1-cmtp-hidp-unsupported-ops-parity-v2.log`,
  `build-ble2-cmtp-hidp-unsupported-ops-parity-v2.log`.
- hwsim PASS:
  `run-cmtp-hidp-unsupported-ops-parity-v2.log` for
  `bluez-hci-mgmt-socket-closeout-full` and
  `bluez-hid-upstream-convergence-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-cmtp-hidp-unsupported-ops-parity-v2/`.

Boundary: This closes ordinary CMTP/HIDP unsupported proto_ops parity at the
BlueZ/NuttX application boundary.  Full CMTP worker/datapath and full HIDP
UHID/input-core ownership remain separate upstream-convergence work.

## 2026-06-20 ordinary CMTP/HIDP nonblocking socket parity closeout

CMTP/HIDP ordinary raw sockets now validate both post-create and
creation-time nonblocking socket behavior.

- CMTP requires `nonblock-ret=0` and `create-nonblock-ok=1`.
- HIDP requires `nonblock-ret=0` and `create-nonblock-ok=1`.
- Existing native ioctl lifecycle checks still remain required for both
  protocols.

Validation:

- Four sim builds passed:
  `build-bt1-cmtp-hidp-nonblock-parity.log`,
  `build-bt2-cmtp-hidp-nonblock-parity.log`,
  `build-ble1-cmtp-hidp-nonblock-parity.log`,
  `build-ble2-cmtp-hidp-nonblock-parity.log`.
- hwsim passed:
  `run-cmtp-hidp-nonblock-parity.log` for
  `bluez-hci-mgmt-socket-closeout-full` and
  `bluez-hid-upstream-convergence-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-cmtp-hidp-nonblock-parity/`.

Boundary: This closes CMTP/HIDP ordinary nonblocking socket API parity for
the current hwsim gates.  It does not claim full real HID input-device or CAPI
hardware integration.

## 2026-06-20 ordinary socket SOCK_NONBLOCK creation parity closeout

Ordinary Bluetooth socket gates now validate creation-time nonblocking
semantics with `socket(... | SOCK_NONBLOCK, ...)` plus `fcntl(F_GETFL)`.

- HFP/HSP RFCOMM and SCO require `create-nonblock-ok=1`.
- A2DP L2CAP requires `create-nonblock-ok=1`.
- LE Audio ISO requires `create-nonblock-ok=1`.
- This checks the common socket fd flag path rather than adding profile-local
  nonblocking state.

Validation:

- Four sim builds passed:
  `build-bt1-sock-create-nonblock-parity.log`,
  `build-bt2-sock-create-nonblock-parity.log`,
  `build-ble1-sock-create-nonblock-parity.log`,
  `build-ble2-sock-create-nonblock-parity.log`.
- hwsim passed:
  `run-sock-create-nonblock-parity.log` for
  `bluez-a2dp-current-complete-closeout`,
  `bluez-le-audio-bap-pacs-ascs-session`, and
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-sock-create-nonblock-parity/`.

Boundary: This closes creation-time `SOCK_NONBLOCK` parity for the ordinary
RFCOMM/SCO/L2CAP/ISO gates.  Blocking waitqueue and wakeup behavior remains a
separate Linux socket/API convergence topic.

## 2026-06-20 ordinary L2CAP/ISO FIONBIO/O_NONBLOCK parity closeout

A2DP ordinary L2CAP and LE Audio ordinary ISO now validate
`fcntl(F_SETFL, O_NONBLOCK)` on their listener sockets before the
listen/connect/accept closeout.

- A2DP `BTPROTO_L2CAP` ordinary listener evidence now includes
  `nonblock-ret=0`.
- LE Audio `BTPROTO_ISO` ordinary listener evidence now includes
  `nonblock-ret=0`.
- Both gates use the shared `PF_BLUETOOTH` socket ioctl path and generic
  `linux_bt_sockif_ioctl(FIONBIO)` state update.

Validation:

- Four sim builds passed:
  `build-bt1-l2cap-iso-fionbio-nonblock.log`,
  `build-bt2-l2cap-iso-fionbio-nonblock.log`,
  `build-ble1-l2cap-iso-fionbio-nonblock.log`,
  `build-ble2-l2cap-iso-fionbio-nonblock.log`.
- hwsim passed:
  `run-l2cap-iso-fionbio-nonblock.log` for
  `bluez-a2dp-current-complete-closeout` and
  `bluez-le-audio-bap-pacs-ascs-session`.
- Artifact:
  `FeatherCore/build/bt-hwsim-l2cap-iso-fionbio-nonblock/`.

Boundary: This closes ordinary nonblocking listener setup for A2DP L2CAP and
LE Audio ISO.  Deeper blocking waitqueue and wakeup parity remains tracked as
a separate Linux socket/API convergence item.

## 2026-06-20 ordinary Bluetooth FIONBIO/O_NONBLOCK socket parity closeout

Ordinary BlueZ-facing Bluetooth sockets now validate
`fcntl(F_SETFL, O_NONBLOCK)` through the same socket API path BlueZ expects on
Linux.

- `psock_vioctl()` now lets all `PF_BLUETOOTH` protocols enter their sockif
  `si_ioctl()` first, instead of special-casing only HCI.
- `linux_bt_sockif_ioctl()` handles `FIONBIO` generically and updates the
  common socket `_SF_NONBLOCK` state.
- HFP/HSP ordinary RFCOMM and SCO closeout now requires `nonblock-ret=0`
  before listen/accept can be considered complete.

Validation:

- Four sim builds passed:
  `build-bt1-bt-sockif-fionbio-nonblock-v2.log`,
  `build-bt2-bt-sockif-fionbio-nonblock-v2.log`,
  `build-ble1-bt-sockif-fionbio-nonblock-v2.log`,
  `build-ble2-bt-sockif-fionbio-nonblock-v2.log`.
- hwsim passed:
  `run-bt-sockif-fionbio-nonblock-v2.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-bt-sockif-fionbio-nonblock-v2/`.

Boundary: This is a framework-level socket API closeout for ordinary
Bluetooth fds, not a profile-local log workaround.  Broader blocking waitqueue
and wakeup behavior remains a separate Linux parity topic.

## 2026-06-20 ordinary CMTP/HIDP socket ioctl proto_ops parity closeout

CMTP and HIDP now have ordinary `AF_BLUETOOTH` raw socket evidence through the
same imported Linux Bluetooth socket ops used by the native protocol probes.

- CMTP/HIDP ordinary fds are accepted by the NuttX Linux Bluetooth sockif.
- `ioctl()` on those fds lazily opens an imported upstream CMTP/HIDP socket
  handle and dispatches into `sock.ops->ioctl()`.
- The CMTP/HIDP native ioctl command numbers are shared through
  `nuttx/wireless/linux_bluetooth.h` with Linux-style `_IOC` layout so app and
  imported stack use the same ABI values.
- `btctl upstream ordinary-cmtp-socket` and
  `btctl upstream ordinary-hidp-socket` validate add, duplicate add, list,
  info, delete, post-delete missing info, duplicate delete, and close.

Validation:

- Four sim builds passed:
  `build-bt1-hidp-cmtp-ordinary-socket-v3.log`,
  `build-bt2-hidp-cmtp-ordinary-socket-v3.log`,
  `build-ble1-hidp-cmtp-ordinary-socket-v3.log`,
  `build-ble2-hidp-cmtp-ordinary-socket-v3.log`.
- hwsim passed:
  `run-hidp-cmtp-ordinary-socket-v3.log` for
  `bluez-hci-mgmt-socket-closeout-full` and
  `bluez-hid-upstream-convergence-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-hidp-cmtp-ordinary-socket-v3/`.

Boundary: CMTP/HIDP ordinary socket create/ioctl/close parity is closed for
the current hwsim gate.  This is Linux-like socket ABI convergence, not a
claim that real HID input device delivery or CAPI hardware behavior is fully
implemented.

## 2026-06-20 ordinary L2CAP/ISO pending accept proto_ops parity closeout

A2DP ordinary L2CAP and LE Audio ordinary ISO now both validate pending
accept success through the hwsim usecase gate.

- L2CAP and ISO listener registration / pending handoff live in the imported
  Linux Bluetooth upstream socket-handle layer.
- A2DP media L2CAP requires `accept-ret=0 pending-accept-ok=1`.
- LE Audio ISO sockapi closeout now performs listener listen, client connect,
  listener accept, and ISO payload send in one ordinary socket path.
- The previous `no-pending-accept-ok=1` validator requirement has been
  replaced for the LE Audio ISO gate.

Validation:

- Four sim builds passed:
  `build-bt1-iso-upstream-pending-accept.log`,
  `build-bt2-iso-upstream-pending-accept.log`,
  `build-ble1-iso-upstream-pending-accept.log`,
  `build-ble2-iso-upstream-pending-accept.log`.
- hwsim passed:
  `run-iso-upstream-pending-accept.log` for
  `bluez-a2dp-current-complete-closeout` and
  `bluez-le-audio-bap-pacs-ascs-session`.
- Artifact:
  `FeatherCore/build/bt-hwsim-iso-upstream-pending-accept/`.

Boundary: The basic ordinary socket listen/connect/accept closeout is now
closed for A2DP L2CAP and LE Audio ISO.  Remaining work is broader edge-case
parity and wider profile coverage rather than this basic pending accept path.

## 2026-06-20 ordinary L2CAP pending accept proto_ops parity closeout

The BlueZ A2DP closeout now validates ordinary Linux-like L2CAP
`listen()`/`connect()`/`accept()` as a completed pending-accept path.

- The imported Linux Bluetooth L2CAP socket-handle layer owns listener
  registration and pending accept handoff.
- Ordinary A2DP media L2CAP sockets now require `accept-ret=0` and
  `pending-accept-ok=1`.
- The accepted handle does not share the listener's underlying `sk`, so
  accepted-fd close does not release the listener.
- LE Audio ISO was rerun as a regression guard, but ISO pending accept remains
  a separate follow-up item.

Validation:

- Four sim builds passed:
  `build-bt1-l2cap-upstream-pending-accept.log`,
  `build-bt2-l2cap-upstream-pending-accept.log`,
  `build-ble1-l2cap-upstream-pending-accept.log`,
  `build-ble2-l2cap-upstream-pending-accept.log`.
- hwsim passed:
  `run-l2cap-upstream-pending-accept.log` for
  `bluez-a2dp-current-complete-closeout` and
  `bluez-le-audio-bap-pacs-ascs-session`.
- Artifact:
  `FeatherCore/build/bt-hwsim-l2cap-upstream-pending-accept/`.

Boundary: A2DP ordinary L2CAP pending accept is closed at hwsim gate level.
LE Audio ISO still records its no-pending listener boundary until ISO gets the
same listener queue closeout.

## 2026-06-20 ordinary L2CAP/ISO listen policy proto_ops parity closeout

The Linux Bluetooth sockif now exposes ordinary L2CAP/ISO `listen()` dispatch
through imported socket ops and records current no-pending `accept()` behavior
from BlueZ-visible A2DP and LE Audio paths.

- L2CAP and ISO have handle-level listen/accept wrappers.
- `g_linux_bt_sockif` now dispatches `si_listen` / `si_accept` for ordinary
  `BTPROTO_L2CAP` and `BTPROTO_ISO` sockets.
- Ordinary L2CAP accepts `SOCK_SEQPACKET` as well as the existing `SOCK_RAW`
  compatibility shape.
- A2DP closeout now calls real userspace L2CAP `socket`, `bind`, `listen`,
  local `connect`, and `accept`.
- LE Audio ISO sockapi closeout now calls real userspace ISO `listen` and
  records no-pending `accept` policy.

Validation:

- Four sim builds passed:
  `build-bt1-l2cap-iso-listen-policy-parity.log`,
  `build-bt2-l2cap-iso-listen-policy-parity.log`,
  `build-ble1-l2cap-iso-listen-policy-parity.log`,
  `build-ble2-l2cap-iso-listen-policy-parity.log`.
- hwsim artifact generated by:
  `run-l2cap-iso-listen-policy-parity.log`.
- Offline validator passed:
  `validate-l2cap-iso-listen-policy-parity.log` for
  `bluez-a2dp-current-complete-closeout` and
  `bluez-le-audio-bap-pacs-ascs-session`.
- Artifact:
  `FeatherCore/build/bt-hwsim-l2cap-iso-listen-policy-parity/`.

Boundary: L2CAP/ISO ordinary listen dispatch and no-pending accept policy are
validated.  Full Linux pending accept queue parity is still open; current
evidence records `accept-ret=-1 accept-errno=11 no-pending-accept-ok=1` for
standalone no-pending listeners.

## 2026-06-20 ordinary RFCOMM/SCO listen/accept proto_ops parity closeout

The Linux Bluetooth sockif now exposes ordinary Classic RFCOMM/SCO
`listen()` and `accept()` through imported socket ops.

- RFCOMM and SCO have handle-level listen/accept wrappers.
- `g_linux_bt_sockif` now wires `si_listen` and `si_accept` for ordinary
  `BTPROTO_RFCOMM` and `BTPROTO_SCO` sockets.
- Accepted sockets inherit the parent `s_sockif`, matching existing NuttX
  INET/USRSOCK accept behavior.
- The HFP/HSP closeout gate calls real userspace `socket()`, `bind()`,
  `listen()`, `accept()`, and `close()` on ordinary RFCOMM and SCO sockets.

Validation:

- Four sim builds passed:
  `build-bt1-sockif-listen-accept-parity-v3.log`,
  `build-bt2-sockif-listen-accept-parity-v3.log`,
  `build-ble1-sockif-listen-accept-parity-v3.log`,
  `build-ble2-sockif-listen-accept-parity-v3.log`.
- hwsim passed:
  `run-sockif-listen-accept-parity-v3.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-sockif-listen-accept-parity-v3/`.

Boundary: RFCOMM/SCO ordinary listen/accept is validated through HFP/HSP.
`fcntl(F_SETFL, O_NONBLOCK)` remains logged as an observation, but generic
fd flag parity is not claimed in this closeout.  L2CAP and ISO listen/accept
now have separate ordinary listen/no-pending policy evidence; full pending
accept queue parity remains separate.

## 2026-06-20 ordinary socket ioctl proto_ops parity closeout

The Linux Bluetooth sockif now exposes ordinary `ioctl()` for imported
Bluetooth socket handles beyond HCI/BNEP.

- L2CAP, RFCOMM, SCO, and ISO have handle-level ioctl wrappers.
- `linux_bt_sockif_ioctl()` dispatches ordinary L2CAP/RFCOMM/SCO/ISO fds into
  imported `sock.ops->ioctl()`.
- The HFP/HSP closeout gate calls real userspace `ioctl(TIOCINQ)` and
  `ioctl(TIOCOUTQ)` on ordinary RFCOMM and SCO sockets.
- The gate uses Linux Bluetooth socket ABI command numbers
  `TIOCINQ=0x541b` and `TIOCOUTQ=0x5411`.

Validation:

- Four sim builds passed:
  `build-bt1-sockif-ioctl-parity-v2.log`,
  `build-bt2-sockif-ioctl-parity-v2.log`,
  `build-ble1-sockif-ioctl-parity-v2.log`,
  `build-ble2-sockif-ioctl-parity-v2.log`.
- hwsim passed:
  `run-sockif-ioctl-parity-v2.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-sockif-ioctl-parity-v2/`.

Boundary: ordinary ioctl dispatch is generic for L2CAP/RFCOMM/SCO/ISO and
validated through RFCOMM/SCO HFP/HSP.  L2CAP and ISO still deserve dedicated
BlueZ-facing ioctl evidence if we want separate per-profile proof.

## 2026-06-20 ordinary socket poll proto_ops parity closeout

The Linux Bluetooth sockif now exposes ordinary `poll()` current-readiness for
imported Bluetooth socket handles.

- L2CAP, RFCOMM, SCO, and ISO have handle-level poll wrappers.
- `g_linux_bt_sockif` now wires `si_poll`.
- Imported Linux `EPOLL*` readiness is mapped into POSIX/NuttX `POLL*`
  revents.
- The HFP/HSP closeout gate calls real userspace `poll(..., timeout=0)` on
  ordinary RFCOMM and SCO sockets and requires writable readiness.

Validation:

- Four sim builds passed:
  `build-bt1-sockif-poll-parity.log`,
  `build-bt2-sockif-poll-parity.log`,
  `build-ble1-sockif-poll-parity.log`,
  `build-ble2-sockif-poll-parity.log`.
- hwsim passed:
  `run-sockif-poll-parity.log` for `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-sockif-poll-parity/`.

Boundary: current-readiness ordinary poll is implemented generically and
validated through RFCOMM/SCO HFP/HSP.  Full blocking poll wait/notify lifecycle
remains a separate convergence item if needed by a profile.

## 2026-06-20 ordinary socket shutdown proto_ops parity closeout

The Linux Bluetooth sockif now exposes ordinary `shutdown()` for imported
Bluetooth socket handles.

- L2CAP, RFCOMM, SCO, and ISO have handle-level shutdown wrappers.
- `g_linux_bt_sockif` now wires `si_shutdown`.
- `shutdown(fd, how)` calls imported `sock.ops->shutdown()` and remains
  separate from `close()`.
- The HFP/HSP closeout gate calls real userspace `shutdown(fd, SHUT_RDWR)` on
  ordinary RFCOMM and SCO sockets before close.

Validation:

- Four sim builds passed:
  `build-bt1-sockif-shutdown-parity.log`,
  `build-bt2-sockif-shutdown-parity.log`,
  `build-ble1-sockif-shutdown-parity.log`,
  `build-ble2-sockif-shutdown-parity.log`.
- hwsim passed:
  `run-sockif-shutdown-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-sockif-shutdown-parity/`.

Boundary: ordinary shutdown dispatch is generic for L2CAP/RFCOMM/SCO/ISO and
validated through RFCOMM/SCO HFP/HSP.  L2CAP and ISO still deserve dedicated
BlueZ-facing shutdown hwsim evidence if we want separate per-profile proof.

## 2026-06-20 ordinary socket getsockname/getpeername identity parity closeout

The Linux Bluetooth sockif now exposes endpoint identity through ordinary
`getsockname()` / `getpeername()` calls.

- `g_linux_bt_sockif` now wires `si_getsockname` and `si_getpeername`.
- The shared implementation covers L2CAP, RFCOMM, SCO, ISO, and HCI sockaddr
  shapes.
- Peer identity follows the Linux state gate and requires a connected socket.
- The HFP/HSP closeout gate calls real userspace `getsockname()` and
  `getpeername()` on ordinary RFCOMM and SCO sockets.

Validation:

- Four sim builds passed:
  `build-bt1-sockif-getname-identity-parity.log`,
  `build-bt2-sockif-getname-identity-parity.log`,
  `build-ble1-sockif-getname-identity-parity.log`,
  `build-ble2-sockif-getname-identity-parity.log`.
- hwsim passed:
  `run-sockif-getname-identity-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-sockif-getname-identity-parity/`.

Boundary: the sockif endpoint identity ABI is now implemented generically and
validated through RFCOMM/SCO HFP/HSP.  L2CAP and ISO use the same implementation
but still deserve dedicated BlueZ-facing hwsim evidence if we want separate
per-profile proof.

## 2026-06-20 RFCOMM/SCO ordinary socket recvmsg dataplane parity closeout

Classic HFP/HSP now validates ordinary RFCOMM/SCO receive-side socket behavior
through `linux_bt_sockif_recvmsg()`, not only through native probe helpers.

- RFCOMM/SCO upstream recv handle APIs can now return payload, byte count, and
  message flags to the generic socket layer.
- `linux_bt_sockif_recvmsg()` routes ordinary `BTPROTO_RFCOMM` and
  `BTPROTO_SCO` sockets into the imported Linux-like RFCOMM/SCO `recvmsg`
  implementations.
- The HFP/HSP closeout gate performs real userspace `recvmsg()` after ordinary
  RFCOMM/SCO socket setup and send.
- RFCOMM validates successful receive evidence.  SCO validates the Linux-like
  no-data case as `recvmsg=-1/EAGAIN` or `EWOULDBLOCK` rather than inventing
  a receive audio frame.

Validation:

- Four sim builds passed:
  `build-bt1-rfcomm-sco-sockif-recvmsg-parity-v2.log`,
  `build-bt2-rfcomm-sco-sockif-recvmsg-parity-v2.log`,
  `build-ble1-rfcomm-sco-sockif-recvmsg-parity-v2.log`,
  `build-ble2-rfcomm-sco-sockif-recvmsg-parity-v2.log`.
- hwsim passed:
  `run-rfcomm-sco-sockif-recvmsg-parity-v2.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-rfcomm-sco-sockif-recvmsg-parity-v2/`.

Boundary: this is receive-side ordinary RFCOMM/SCO socket API parity for the
current NuttX hwsim HFP/HSP path.  It does not yet claim full real-controller
SCO/eSCO incoming audio queue behavior, codec offload, or QoS/error-policy
parity.

## 2026-06-20 RFCOMM/SCO ordinary socket sockif bridge parity closeout

Classic HFP/HSP now validates ordinary BlueZ-style RFCOMM and SCO socket paths
in addition to the native upstream handle probes.

- `linux_bt_sockif` accepts `BTPROTO_RFCOMM` stream sockets and `BTPROTO_SCO`
  seqpacket/raw sockets.
- Generic NuttX socket bind/connect/sendmsg/close for RFCOMM/SCO now route
  through upstream handles and imported Linux-like socket ops.
- RFCOMM/SCO `getsockopt` / `setsockopt` calls use the same upstream handle
  wrappers as the HFP/HSP profile probes.
- The HFP/HSP closeout gate performs real userspace `socket(AF_BLUETOOTH,
  ...)`, `bind`, `connect`, `getsockopt`, `sendmsg`, and `close` calls for
  both RFCOMM and SCO on HFP-HF, HFP-AG, HSP-HS, and HSP-AG.

Validation:

- Four sim builds passed:
  `build-bt1-rfcomm-sco-sockif-sockapi-parity.log`,
  `build-bt2-rfcomm-sco-sockif-sockapi-parity.log`,
  `build-ble1-rfcomm-sco-sockif-sockapi-parity.log`,
  `build-ble2-rfcomm-sco-sockif-sockapi-parity.log`.
- hwsim passed:
  `run-rfcomm-sco-sockif-sockapi-parity.log` for
  `bluez-hfp-hsp-profile-closeout`.
- Artifact:
  `FeatherCore/build/bt-hwsim-rfcomm-sco-sockif-sockapi-parity/`.

Boundary: this is ordinary RFCOMM/SCO socket API parity for the current NuttX
hwsim HFP/HSP path.  It does not yet claim full RFCOMM DLC credit/TTY parity,
real SCO/eSCO codec offload, or complete controller QoS/error-policy behavior.

## 2026-06-20 ISO ordinary socket sockif bridge parity closeout

LE Audio now validates an ordinary BlueZ-style ISO socket path, not only the
existing transport probe helpers.

- `linux_bt_sockif` accepts `BTPROTO_ISO` ordinary sockets and forwards bind,
  connect, sendmsg/recvmsg, close, and ISO `SOL_BLUETOOTH` sockopts through an
  upstream ISO socket handle.
- The upstream ISO handle follows the existing L2CAP handle pattern and calls
  imported ISO socket ops instead of using a one-off hwsim shortcut.
- `bluezaudio le-iso-socket sockapi-closeout source|sink` performs real
  userspace `socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_ISO)`, `bind`,
  `setsockopt`, `getsockopt`, `connect`, `sendmsg`, and `close`.
- `BT_PKT_SEQNUM` is treated as upstream Linux set-only evidence: set must
  succeed, while getsockopt is not required.

Validation:

- Four sim builds passed:
  `build-bt1-iso-sockif-sockapi-parity-v2.log`,
  `build-bt2-iso-sockif-sockapi-parity-v2.log`,
  `build-ble1-iso-sockif-sockapi-parity-v2.log`,
  `build-ble2-iso-sockif-sockapi-parity-v2.log`.
- hwsim passed:
  `run-iso-sockif-sockapi-parity-v2.log` for
  `bluez-le-audio-bap-pacs-ascs-session`.
- Artifact:
  `FeatherCore/build/bt-hwsim-iso-sockif-sockapi-parity-v2/`.

Boundary: this is ordinary ISO socket API parity for the current NuttX hwsim
LE Audio path.  It does not yet claim complete real-controller ISO teardown or
all CIG/CIS error-policy behavior.

## 2026-06-20 L2CAP socket sockopt bridge parity closeout

The NuttX `AF_BLUETOOTH/BTPROTO_L2CAP` socket interface now forwards
`getsockopt` / `setsockopt` to the imported Linux-like L2CAP socket option
implementation.

Implemented:

- Added L2CAP handle wrappers for `getsockopt` / `setsockopt`.
- Routed generic NuttX L2CAP socket sockopts through those wrappers.
- Preserved Linux ordering where BlueZ may call `setsockopt()` before
  `bind()` by reusing the existing upstream L2CAP handle at bind time.
- Extended the BlueZ `bneptest` fd-handoff gate to validate L2CAP sockopts on
  the ordinary socket fd before it is handed to BNEP.
- `L2CAP_CONNINFO` now exposes the hwsim connected handle metadata.
- BR/EDR `BT_SNDMTU` / `BT_RCVMTU` now remain Linux-like `EINVAL` evidence,
  while `BT_PHY` returns the BR/EDR PHY mask.

Validated:

- `FeatherCore/build/logs/build-bt1-l2cap-sockopt-sockif-parity-v4.log`: PASS.
- `FeatherCore/build/logs/build-bt2-l2cap-sockopt-sockif-parity-v4.log`: PASS.
- `FeatherCore/build/logs/build-ble1-l2cap-sockopt-sockif-parity-v4.log`: PASS.
- `FeatherCore/build/logs/build-ble2-l2cap-sockopt-sockif-parity-v4.log`: PASS.
- `FeatherCore/build/logs/run-l2cap-sockopt-sockif-parity-v4.log`: PASS
  `bluez-bneptest-fd-handoff`.

Boundary:

- This is socket API bridge parity for the current hwsim BR/EDR L2CAP/BNEP
  path. Full imported Linux `l2cap_conn` / `hci_conn` ownership remains a
  later upstream convergence item.

## 2026-06-20 IPSP native 6LoWPAN link-ledger wording closeout

BLE IPSP/6LoWPAN now uses `native-6lowpan link-ledger` for final zero-ref
disconnect evidence instead of the old active-owner style
`native-6lowpan ownership-ledger` marker.

Implemented:

- Reworded the executable `bluezipsp disconnect` closeout evidence to
  `native-6lowpan link-ledger`.
- Added a validator forbidden pattern for the old
  `native-6lowpan ownership-ledger` wording.
- Preserved the BLE1/BLE2 `bluez-ipsp-closeout-full` gate and its final
  zero-reference checks.

Validated:

- `FeatherCore/build/logs/build-bt1-ipsp-link-ledger-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-ipsp-link-ledger-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-ipsp-link-ledger-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-ipsp-link-ledger-closeout.log`: PASS.
- `FeatherCore/build/logs/run-ipsp-link-ledger-closeout.log`: PASS
  `bluez-ipsp-closeout-full`.

Boundary:

- This removes the remaining executable IPSP active-owner wording leak.
- It remains hwsim link evidence, not proof that unmodified upstream BlueZ
  IPSP and Linux 6LoWPAN own every object without the NuttX adapter/hwsim
  harness.

## 2026-06-20 SCO_OPTIONS/SCO_CONNINFO socket option parity closeout

Classic HFP/HSP now validates upstream Linux `SCO_OPTIONS` and
`SCO_CONNINFO` through `getsockopt(SOL_SCO, ...)` on the connected
`BTPROTO_SCO` socket path.

Implemented:

- Added upstream `SCO_OPTIONS` / `SCO_CONNINFO` option coverage to the
  Linux-like SCO socket implementation.
- `getsockopt(SOL_SCO, SCO_OPTIONS)` returns the hwsim SCO MTU after the same
  connected or deferred-setup state gate used by Linux.
- `getsockopt(SOL_SCO, SCO_CONNINFO)` returns the hwsim SCO handle and the
  currently available zeroed `dev_class` metadata.
- The HFP/HSP closeout gate now requires this evidence for HFP-HF, HFP-AG,
  HSP-HS, and HSP-AG.

Validated:

- `FeatherCore/build/logs/build-bt1-sco-conninfo-options-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-sco-conninfo-options-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble1-sco-conninfo-options-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-sco-conninfo-options-parity.log`: PASS.
- `FeatherCore/build/logs/run-sco-conninfo-options-parity.log`: PASS
  `bluez-hfp-hsp-profile-closeout`.

Boundary:

- This is a Linux socket ABI/parity closeout for observable SCO option state.
- It is not a claim of real SCO/eSCO controller audio offload, controller QoS
  negotiation, or full incoming SCO connection policy parity.

## 2026-06-20 RFCOMM_CONNINFO socket option parity closeout

Classic HFP/HSP now validates upstream Linux `RFCOMM_CONNINFO` through `getsockopt(SOL_RFCOMM, RFCOMM_CONNINFO)` on the connected `BTPROTO_RFCOMM` socket path.

Completed:

- Added the upstream `RFCOMM_CONNINFO` option number and local `rfcomm_conninfo`-compatible payload shape.
- RFCOMM `getsockopt(SOL_RFCOMM, RFCOMM_CONNINFO)` now returns `-ENOTCONN` unless the socket is connected or deferred setup is active, matching the upstream `rfcomm_sock_getsockopt_old()` state gate.
- Connected RFCOMM sockets now report the hwsim ACL handle through `hci_handle` and a zeroed `dev_class`, matching the currently available hwsim controller metadata.
- RFCOMM connect refreshes pinfo handle ownership so `RFCOMM_CONNINFO` reports the same handle used by the HFP/HSP profile path.
- `bluez-hfp-hsp-profile-closeout` now requires `conninfo-ret=0`, `conninfo-handle=0x0052`, and `conninfo-dev-class=00:00:00` for HFP and HSP RFCOMM channels on both BT roles.

Validation:

- `FeatherCore/build/logs/build-bt1-rfcomm-conninfo-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-rfcomm-conninfo-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble1-rfcomm-conninfo-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-rfcomm-conninfo-parity.log`: PASS.
- `FeatherCore/build/logs/run-rfcomm-conninfo-parity.log`: PASS `bluez-hfp-hsp-profile-closeout`.
- Key evidence: `upstream-rfcomm-ioctl: ... conninfo-ret=0 conninfo-handle=0x0052 conninfo-dev-class=00:00:00 ...`.

Boundary:

- This closes the observable RFCOMM connection-info socket option gate. It still does not claim complete RFCOMM DLC credit, TTY, or full upstream session teardown parity.

## 2026-06-20 RFCOMM/SCO getname socket identity parity closeout

Classic HFP/HSP now validates Linux-like `getname` endpoint identity on the same `BTPROTO_RFCOMM` and `BTPROTO_SCO` listen/accept paths used by the profile closeout.

Completed:

- RFCOMM `getname(peer=1)` now follows upstream `rfcomm_sock_getname()` state policy and returns `-ENOTCONN` while the listener is not connected.
- RFCOMM listen/accept probe records listener local `getname`, rejected listener peer `getname`, accepted local `getname`, and accepted peer `getname` channel identity.
- SCO listen/accept probe records listener and accepted local/peer `getname` results, matching upstream `sco_sock_getname()` visibility.
- `bluez-hfp-hsp-profile-closeout` now requires these getname records for HFP-HF, HFP-AG, HSP-HS, and HSP-AG.

Validation:

- `FeatherCore/build/logs/build-bt1-rfcomm-sco-getname-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-rfcomm-sco-getname-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble1-rfcomm-sco-getname-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-rfcomm-sco-getname-parity.log`: PASS.
- `FeatherCore/build/logs/run-rfcomm-sco-getname-parity.log`: PASS `bluez-hfp-hsp-profile-closeout`.
- Key evidence: `upstream-rfcomm-listen-accept: ... getname-ret=10 ... peer-getname-ret=-107 ... accepted-peer-getname-ret=10 ...`.
- Key evidence: `upstream-sco-listen-accept: ... getname-ret=8 peer-getname-ret=8 accepted-getname-ret=8 accepted-peer-getname-ret=8 ...`.

Boundary:

- This closes the observable RFCOMM/SCO socket identity gate. It does not claim complete RFCOMM DLC credit/TTY parity or real SCO/eSCO codec offload policy.

## 2026-06-20 HIDP/CMTP full control socket no-op matrix closeout

HIDP and CMTP control sockets now validate the full Linux `sock_no_*` ABI
matrix used by the upstream control socket implementations.

Implemented:

- HIDP session probe validates `bind`, `getname`, `sendmsg`, `recvmsg`,
  `listen`, `shutdown`, `connect`, `socketpair`, `accept`, and `mmap`.
- CMTP session probe validates the same no-op matrix.
- The probes fail internally if any no-op return deviates from
  `-EOPNOTSUPP`.
- HIDP and CMTP validators now require the full no-op matrix evidence.

Validated:

- `FeatherCore/build/logs/build-bt1-hidp-cmtp-full-nodata-matrix.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hidp-cmtp-full-nodata-matrix.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hidp-cmtp-full-nodata-matrix.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hidp-cmtp-full-nodata-matrix.log`: PASS.
- `FeatherCore/build/logs/run-hidp-cmtp-full-nodata-matrix-cmtp.log`: PASS.
- `FeatherCore/build/bt-hwsim-usecases-hidp-cmtp-full-nodata-matrix-cmtp/run-results.json`:
  `passed=true`, `validate_rc=0`.
- `FeatherCore/build/logs/run-hidp-cmtp-full-nodata-matrix-hidp.log`: PASS.
- `FeatherCore/build/bt-hwsim-usecases-hidp-cmtp-full-nodata-matrix-hidp/run-results.json`:
  `passed=true`, `validate_rc=0`.

Boundary:

- This closes the HIDP/CMTP control socket no-op ABI matrix gate.
- It does not yet prove full upstream HIDP worker/UHID/input-core ownership or
  full upstream CMTP CAPI worker/message datapath parity.

## 2026-06-20 CMTP control socket no-data/error lifecycle closeout

CMTP now has the same hwsim-observable control-socket ABI floor as the
Linux `third/linux-7.0.10/net/bluetooth/cmtp/sock.c` shape.

Implemented:

- The CMTP session probe now validates `sock_no_bind`, `sock_no_getname`,
  `sock_no_sendmsg`, and `sock_no_recvmsg` on the real `BTPROTO_CMTP` socket.
- Duplicate `CMTPCONNADD` now has validator coverage and must return
  `-EALREADY`.
- `CMTPGETCONNINFO` after delete and duplicate `CMTPCONNDEL` now have
  validator coverage and must return `-ENOENT`.
- The probe preserves pre-delete `CMTPGETCONNINFO` state/num/flags evidence so
  the post-delete error check cannot overwrite the happy-path result.

Validated:

- `FeatherCore/build/logs/build-bt1-cmtp-control-no-data-errors.log`: PASS.
- `FeatherCore/build/logs/build-bt2-cmtp-control-no-data-errors.log`: PASS.
- `FeatherCore/build/logs/build-ble1-cmtp-control-no-data-errors.log`: PASS.
- `FeatherCore/build/logs/build-ble2-cmtp-control-no-data-errors.log`: PASS.
- `FeatherCore/build/logs/run-cmtp-control-no-data-errors.log`: PASS.
- `FeatherCore/build/bt-hwsim-usecases-cmtp-control-no-data-errors/run-results.json`:
  `passed=true`, `validate_rc=0`.

Boundary:

- This closes the CMTP no-data control socket and ioctl error lifecycle gate.
- It does not yet mean the full upstream CMTP CAPI worker/datapath is imported
  without a transitional session surface.

## 2026-06-20 SCO SOL_BLUETOOTH option parity closeout

Classic HFP/HSP SCO now validates more Linux-like `SOL_BLUETOOTH` option
behavior on the same `BTPROTO_SCO` socket path used by the profile closeout.

Implemented:

- Added the shared `BT_CODEC` option constant.
- SCO pinfo now tracks deferred setup and packet status state.
- SCO `setsockopt` / `getsockopt` now supports
  `SOL_BLUETOOTH/BT_DEFER_SETUP`.
- SCO `setsockopt` / `getsockopt` now supports
  `SOL_BLUETOOTH/BT_PKT_STATUS`.
- SCO `getsockopt` now supports connected-socket
  `SOL_BLUETOOTH/BT_SNDMTU` and `BT_RCVMTU`.
- SCO `BT_CODEC` returns `-EOPNOTSUPP` for the current hwsim controller,
  matching the no codec-offload capability boundary instead of faking codec
  lists.
- Accepted SCO sockets inherit parent defer/pkt-status state.
- The HFP/HSP SCO listen-accept probe now emits option set/get/clone evidence.

Validated:

- `FeatherCore/build/logs/build-bt1-sco-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/logs/build-bt2-sco-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/logs/build-ble1-sco-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/logs/build-ble2-sco-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/logs/run-sco-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/bt-hwsim-usecases-sco-sol-bluetooth-options/run-results.json`:
  `passed=true`, `validate_rc=0`.

Boundary:

- This closes the BlueZ-visible SCO option and accepted-socket clone gate used
  by HFP/HSP.
- It does not yet prove real SCO codec offload datapath or controller codec
  negotiation; `BT_CODEC` intentionally remains `-EOPNOTSUPP` for current
  hwsim.

## 2026-06-20 RFCOMM SOL_BLUETOOTH option parity closeout

Classic HFP/HSP RFCOMM now validates Linux-like `SOL_BLUETOOTH` option
behavior on the same `BTPROTO_RFCOMM` socket path used by the profile closeout.

Implemented:

- `BTPROTO_RFCOMM` pinfo now tracks security level and deferred setup state.
- RFCOMM `setsockopt` / `getsockopt` now supports
  `SOL_BLUETOOTH/BT_SECURITY`.
- RFCOMM `setsockopt` / `getsockopt` now supports
  `SOL_BLUETOOTH/BT_DEFER_SETUP`.
- Accepted RFCOMM sockets inherit the parent socket's security/defer state.
- The HFP/HSP RFCOMM listen-accept probe now emits set/get/clone evidence for
  `BT_SECURITY` and `BT_DEFER_SETUP`.

Validated:

- `FeatherCore/build/logs/build-bt1-rfcomm-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/logs/build-bt2-rfcomm-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/logs/build-ble1-rfcomm-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/logs/build-ble2-rfcomm-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/logs/run-rfcomm-sol-bluetooth-options.log`: PASS.
- `FeatherCore/build/bt-hwsim-usecases-rfcomm-sol-bluetooth-options/run-results.json`:
  `passed=true`, `validate_rc=0`.

Boundary:

- This closes the BlueZ-visible RFCOMM `SOL_BLUETOOTH` option and accepted
  socket clone gate used by HFP/HSP.
- It does not yet mean full upstream RFCOMM core/DLC/session/tty ownership has
  been imported unchanged.

## 2026-06-20 HCI PHY command queue closeout

`BT_PHY` set now reaches Linux-like HCI command queue helpers when native
L2CAP owns a real `hci_conn`.

Implemented:

- Added missing HCI ABI coverage for `HCI_OP_CHANGE_CONN_PTYPE` and
  `HCI_OP_LE_SET_PHY`.
- Reused the compat `hci_cmd_defs.h` definition for
  `hci_cp_change_conn_ptype`; no duplicate local command parameter struct is
  carried in the outer compat header.
- Added `hci_acl_change_pkt_type()` in imported `upstream/net_bluetooth/hci_sync.c`.
- Added `hci_le_set_phy()` in imported `upstream/net_bluetooth/hci_sync.c`.
- Updated `hci_conn_set_phy()` so ACL links queue packet-type changes and LE
  links queue LE PHY updates instead of only changing local connection fields.

Validated:

- `FeatherCore/build/logs/build-bt1-hci-phy-command-queue.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hci-phy-command-queue.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hci-phy-command-queue.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hci-phy-command-queue.log`: PASS.
- `FeatherCore/build/logs/run-hci-phy-command-queue.log`: PASS.
- `FeatherCore/build/bt-hwsim-usecases-hci-phy-command-queue/run-results.json`:
  `passed=true`, `validate_rc=0`.

Boundary:

- This closes the buildable Linux-like HCI command queue entry point for
  `BT_PHY` set and proves the A2DP hwsim closeout remains stable.
- The current hwsim case is still a BlueZ-visible socket/media regression gate,
  not proof that a real RF controller emitted and completed PHY change events.

## 2026-06-20 HCI connection BT_PHY set helper closeout

`BT_PHY` set now has an HCI connection layer helper instead of living only as
an L2CAP-local type check.

Implemented:

- Added `bt_phy_pkt_type()`, `bt_phy_le_phy()`, and `hci_conn_set_phy()` to
  imported `upstream/net_bluetooth/hci_conn.c`.
- Declared `hci_conn_set_phy()` from `upstream/include_net_bluetooth/hci_core.h`.
- Native L2CAP `BT_PHY` set calls `hci_conn_set_phy()` when a real
  `chan->conn->hcon` is present.
- The hwsim staged-connected fallback remains only for sockets that do not yet
  own a full `l2cap_conn/hci_conn` object.

Validated:

- `FeatherCore/build/logs/build-bt1-hci-conn-set-phy.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hci-conn-set-phy.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hci-conn-set-phy.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hci-conn-set-phy.log`: PASS.
- `FeatherCore/build/logs/run-hci-conn-set-phy.log`: PASS.
- `FeatherCore/build/bt-hwsim-usecases-hci-conn-set-phy/run-results.json`:
  `passed=true`, `validate_rc=0`.

Boundary:

- This proves the HCI connection helper is built and the BlueZ-visible A2DP
  `BT_PHY` socket gate remains stable after the L2CAP call path was switched to
  it.
- Full controller command reprogramming still needs the `hci_sync.c`
  `hci_acl_change_pkt_type()` / `hci_le_set_phy()` command queue path.

## 2026-06-20 L2CAP BT_PHY native connected gate closeout

The A2DP current-complete hwsim path now validates native/imported L2CAP
`BT_PHY` connected socket behavior instead of carrying a skip marker.

Implemented:

- `upstream/net_bluetooth/l2cap_sock.c` keeps the upstream path when a real
  `chan->conn->hcon` exists and calls `hci_conn_get_phy()`.
- The same native L2CAP socket get path now safely handles the hwsim
  staged-connected case where the socket is connected but full
  `l2cap_conn/hci_conn` ownership is not represented yet.
- Native L2CAP now has a `BT_PHY` set branch.  It returns `-ENOTCONN` when not
  connected, accepts BR/EDR PHY masks for ACL links, accepts LE PHY masks for
  LE links, and rejects cross-link masks with `-EINVAL`.
- `bluez-a2dp-current-complete-closeout` now requires `BT_PHY get-ret=0`,
  `set-bredr-ret=0`, `invalid-le-ret=-22`, and
  `gate=hci-conn-set-phy-type-check`.

Validated:

- `FeatherCore/build/logs/build-bt1-bt-phy-connected-gate.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bt-phy-connected-gate.log`: PASS.
- `FeatherCore/build/logs/build-ble1-bt-phy-connected-gate.log`: PASS.
- `FeatherCore/build/logs/build-ble2-bt-phy-connected-gate.log`: PASS.
- `FeatherCore/build/logs/run-bt-phy-connected-gate.log`: PASS.
- `FeatherCore/build/bt-hwsim-usecases-bt-phy-connected-gate/run-results.json`:
  `passed=true`, `validate_rc=0`.

Boundary:

- This closes the BlueZ-visible native L2CAP socket `BT_PHY`
  get/set/type-check gate.
- A later HCI connection helper closeout added `hci_conn_set_phy()` and wired
  native L2CAP to call it when a real `hcon` exists.  Full controller command
  queue reprogramming remains separate.

## 2026-06-20 L2CAP Bluetooth-level option parity closeout

L2CAP now exposes more Linux-like `SOL_BLUETOOTH` option behavior through the
A2DP current-complete gate.

Implemented:

- Added missing Bluetooth ABI constants for channel policy and PHY masks to
  the compat Bluetooth headers.
- L2CAP state now tracks flushable, force-active power policy, channel policy,
  and PHY.
- `BT_FLUSHABLE` set/get is implemented.
- `BT_POWER` set/get is implemented.
- `BT_CHANNEL_POLICY` get is implemented and set keeps the upstream
  `-EOPNOTSUPP` behavior.
- `BT_PHY` ABI entry points are present.  A later native connected gate now
  validates `BT_PHY` get/set/type-check through imported L2CAP socket ops.

Validated:

- `FeatherCore/build/logs/build-bt1-l2cap-bt-options-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-l2cap-bt-options-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-l2cap-bt-options-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-l2cap-bt-options-closeout.log`: PASS.
- `FeatherCore/build/logs/run-l2cap-bt-options-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-l2cap-bt-options-closeout.log`: PASS.

Boundary:

- This closes the observable L2CAP Bluetooth-level option gate for A2DP.
- A later HCI connection helper closeout adds `hci_conn_set_phy()`;
  full controller PHY reprogramming still needs the HCI command path.

## 2026-06-20 SCO socketpair/mmap no-op ABI closeout

SCO now exposes the remaining no-op proto_ops required by the Linux 7.0.10
reference.

Implemented:

- `BTPROTO_SCO` proto_ops now provides `sock_no_mmap`.
- `BTPROTO_SCO` proto_ops now provides `sock_no_socketpair`.
- The SCO ioctl probe validates both no-op handlers through socket `ops`.
- HFP/HSP validators require `no-op=socketpair-mmap` evidence for all four
  HFP/HSP roles.

Validated:

- `FeatherCore/build/logs/build-bt1-sco-no-op-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-sco-no-op-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-sco-no-op-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-sco-no-op-closeout.log`: PASS.
- `FeatherCore/build/logs/run-sco-no-op-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-sco-no-op-closeout.log`: PASS.

Boundary:

- This closes the observable SCO `socketpair` / `mmap` no-op ABI gate.
- SCO/eSCO controller policy, codec timing, and real audio-device integration
  remain separate convergence work.

## 2026-06-20 BNEP/HIDP control socket no-data ABI closeout

BNEP and HIDP now expose the same control-socket no-data ABI shape as the
Linux 7.0.10 reference.

Implemented:

- `BTPROTO_BNEP` proto_ops now explicitly provides `sock_no_bind`,
  `sock_no_getname`, `sock_no_sendmsg`, and `sock_no_recvmsg`.
- `BTPROTO_HIDP` proto_ops now explicitly provides `sock_no_bind`,
  `sock_no_getname`, `sock_no_sendmsg`, and `sock_no_recvmsg`.
- The BlueZ Network connect path validates BNEP no-data behavior on the real
  `BTPROTO_BNEP` fd before continuing through BNEP ioctl and ping.
- The HIDP session probe validates no-data behavior through socket `ops`.
- Validators require BNEP and HIDP no-data evidence.

Validated:

- `FeatherCore/build/logs/build-bt1-control-sock-no-data-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-control-sock-no-data-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-control-sock-no-data-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-control-sock-no-data-closeout.log`: PASS.
- `FeatherCore/build/logs/run-control-sock-no-data-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-control-sock-no-data-closeout.log`: PASS.

Boundary:

- This closes the observable BNEP/HIDP control-socket no-data ABI gate.
- It does not claim full unmodified BlueZ Network/Input daemon ownership.

## 2026-06-20 RFCOMM native ioctl wrapper closeout

HFP/HSP now validates RFCOMM-specific ioctl wrapper semantics instead of only
the generic `bt_sock_ioctl` success path.

Implemented:

- `BTPROTO_RFCOMM` ioctl remains routed through a RFCOMM-specific wrapper.
- The wrapper preserves Linux-like `TIOCINQ` / `TIOCOUTQ` behavior via
  `bt_sock_ioctl()`.
- Unsupported RFCOMM ioctls now follow the upstream no-RFCOMM-TTY path and
  return `-EOPNOTSUPP`.
- HFP/HSP validators require `fallback=rfcomm-tty-disabled` evidence for
  HFP-HF, HFP-AG, HSP-HS, and HSP-AG.

Validated:

- `FeatherCore/build/logs/build-bt1-rfcomm-native-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-rfcomm-native-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-rfcomm-native-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-rfcomm-native-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/run-rfcomm-native-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-rfcomm-native-ioctl-closeout.log`: PASS.

Boundary:

- This closes the observable RFCOMM ioctl wrapper/fallback gate.
- RFCOMM TTY device support and deeper credit/flow-control behavior remain
  separate upstream convergence work.

## 2026-06-20 Bluetooth `sock_gettstamp` parity closeout

L2CAP, RFCOMM, and SCO now expose Linux-like socket timestamp behavior through
their current A2DP and HFP/HSP profile gates.

Implemented:

- compat `sock_gettstamp()` now supports timeval and timespec calls instead of
  returning `-EOPNOTSUPP`.
- `BTPROTO_L2CAP`, `BTPROTO_RFCOMM`, and `BTPROTO_SCO` now provide
  `.gettstamp = sock_gettstamp`, matching the Linux 7.0.10 reference.
- `BTPROTO_ISO` is intentionally unchanged because the Linux 7.0.10 reference
  ISO proto_ops does not expose `.gettstamp`.
- Added L2CAP/RFCOMM/SCO timestamp probes that call socket `ops->gettstamp()`.
- A2DP daemon closeout validates L2CAP timestamp evidence.
- HFP/HSP profile closeout validates RFCOMM and SCO timestamp evidence.

Validated:

- `FeatherCore/build/logs/build-bt1-gettstamp-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-gettstamp-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-gettstamp-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-gettstamp-closeout.log`: PASS.
- `FeatherCore/build/logs/run-gettstamp-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-gettstamp-closeout.log`: PASS.

Boundary:

- This closes the socket-level `sock_gettstamp` ABI gate for L2CAP/RFCOMM/SCO.
- Controller/media frame timestamp accuracy, ISO timestamp/drop policy, and
  RFCOMM TTY timing remain deeper convergence work.

## 2026-06-20 Bluetooth `bt_sock_poll` parity closeout

Bluetooth data sockets now expose Linux-like poll/readiness behavior through
the A2DP, HFP/HSP, and LE Audio profile gates.

Implemented:

- Added `linux_bt_sock_poll()` with the core upstream `bt_sock_poll`
  readiness semantics: accept queue, error queue, shutdown, receive queue,
  closed/hup, connect/config, and writable masks.
- `BTPROTO_L2CAP`, `BTPROTO_RFCOMM`, `BTPROTO_SCO`, and `BTPROTO_ISO` now use
  `linux_bt_sock_poll`.
- HCI, BNEP, CMTP, and HIDP keep their existing non-data-socket poll behavior.
- Added L2CAP/RFCOMM/SCO poll probes and connected them to A2DP daemon and
  HFP/HSP profile flows.
- Existing LE Audio ISO poll probing now exercises the unified poll path.
- Validators require poll evidence for A2DP, HFP/HSP, and LE Audio gates.

Validated:

- `FeatherCore/build/logs/build-bt1-bt-sock-poll-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bt-sock-poll-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-bt-sock-poll-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-bt-sock-poll-closeout.log`: PASS.
- `FeatherCore/build/logs/run-bt-sock-poll-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-bt-sock-poll-closeout.log`: PASS.

Boundary:

- This closes the hwsim-observable poll/readiness gate for the main Bluetooth
  data sockets.
- Full L2CAP ERTM/streaming flow control, RFCOMM TTY poll/credit, SCO/eSCO
  controller policy, and ISO QoS/drop/timestamp policy still need deeper
  convergence.

## 2026-06-20 L2CAP/ISO socket ioctl closeout

A2DP and LE Audio now validate Linux-like socket ioctl behavior through their
current strongest profile gates.

Implemented:

- Transitional `BTPROTO_L2CAP` proto_ops now provides
  `.ioctl = bt_sock_ioctl`.
- Transitional `BTPROTO_ISO` proto_ops now provides `.ioctl = bt_sock_ioctl`.
- `linux_bt_upstream_l2cap_socket_ioctl_probe()` calls `TIOCINQ` /
  `TIOCOUTQ` through the active L2CAP socket handle.
- `linux_bt_upstream_iso_socket_ioctl_probe()` calls `TIOCINQ` / `TIOCOUTQ`
  through the active ISO probe socket.
- `bluezdaemon audio-a2dp-closeout-full` validates ioctl through the A2DP
  media L2CAP controller phase.
- LE Audio daemon unicast profile flow validates ioctl after ISO
  `sendmsg` / `recvmsg`.
- Validators now require `upstream-l2cap-ioctl` and `upstream-iso-ioctl`
  evidence in the A2DP and LE Audio gates.

Validated:

- `FeatherCore/build/logs/build-bt1-l2cap-iso-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-l2cap-iso-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-l2cap-iso-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-l2cap-iso-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/run-l2cap-iso-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-l2cap-iso-ioctl-closeout.log`: PASS.

Boundary:

- This closes the hwsim-observable L2CAP/ISO socket ioctl gate through A2DP
  and LE Audio profile paths.
- Full upstream L2CAP ERTM/streaming behavior and ISO controller/QoS/drop
  policy still need deeper convergence.

## 2026-06-20 RFCOMM/SCO socket ioctl closeout

HFP/HSP now validates Linux-like socket ioctl behavior through the real profile
flow, not through a standalone temporary bypass.

Implemented:

- Transitional `BTPROTO_RFCOMM` proto_ops now provides
  `.ioctl = bt_sock_ioctl`.
- Transitional `BTPROTO_SCO` proto_ops now provides `.ioctl = bt_sock_ioctl`.
- `TIOCINQ` / `TIOCOUTQ` are fixed at the Linux ioctl ABI values in the
  Bluetooth upstream compat boundary, avoiding host ioctl macro leakage.
- HFP/HSP calls RFCOMM ioctl probing after AT/control transactions.
- HFP/HSP calls SCO ioctl probing after SCO audio payload transfer.
- `bluez-hfp-hsp-profile-closeout` requires RFCOMM/SCO ioctl evidence for
  HFP-HF, HFP-AG, HSP-HS, and HSP-AG.

Validated:

- `FeatherCore/build/logs/build-bt1-rfcomm-sco-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-rfcomm-sco-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-rfcomm-sco-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-rfcomm-sco-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/run-rfcomm-sco-ioctl-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-rfcomm-sco-ioctl-closeout.log`: PASS.

Boundary:

- This closes the hwsim-observable RFCOMM/SCO socket ioctl gate through the
  HFP/HSP profile path.
- Full upstream RFCOMM DLC/session policy and SCO/eSCO controller policy still
  need deeper convergence.

## 2026-06-20 SCO release-owned shutdown closeout

SCO close now follows the Linux socket lifecycle more closely: HFP/HSP profile
close reaches `BTPROTO_SCO` release-owned shutdown instead of directly freeing
the socket.

Implemented:

- Transitional `BTPROTO_SCO` release now calls `shutdown(SHUT_RDWR)` before
  orphan/put.
- `linux_bt_sco_sock_shutdown()` is idempotent and records Linux-like
  `sk_shutdown` state.
- The SCO hwsim handle close path emits `upstream-sco-shutdown-close`
  evidence.
- `bluez-hfp-hsp-profile-closeout` requires shutdown-close evidence for
  HFP-HF, HFP-AG, HSP-HS, and HSP-AG.

Validated:

- `FeatherCore/build/logs/build-bt1-sco-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-sco-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-sco-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-sco-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/run-sco-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-sco-shutdown-closeout.log`: PASS.

Boundary:

- This closes the observable SCO socket shutdown/close gate through the HFP/HSP
  profile path.
- Full upstream SCO/eSCO setup, voice/air-mode policy, QoS, incoming confirm,
  and controller teardown ownership still need deeper convergence.

## 2026-06-20 RFCOMM release-owned shutdown closeout

RFCOMM close now follows the Linux socket lifecycle more closely: release owns
`shutdown(SHUT_RDWR)` and HFP/HSP validates that profile close reaches this
native-facing socket path.

Implemented:

- Transitional `BTPROTO_RFCOMM` proto_ops now provides
  `linux_bt_rfcomm_sock_shutdown()` instead of `sock_no_shutdown`.
- RFCOMM release calls `shutdown(SHUT_RDWR)` before orphan/put, matching the
  upstream `rfcomm_sock_release()` ownership model.
- The RFCOMM hwsim handle close path emits `upstream-rfcomm-shutdown-close`
  evidence.
- `bluez-hfp-hsp-profile-closeout` requires shutdown-close evidence for
  HFP-HF, HFP-AG, HSP-HS, and HSP-AG.

Validated:

- `FeatherCore/build/logs/build-bt1-rfcomm-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-rfcomm-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-rfcomm-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-rfcomm-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/run-rfcomm-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-rfcomm-shutdown-closeout.log`: PASS.

Boundary:

- This closes the observable RFCOMM socket shutdown/close gate through the
  HFP/HSP profile path.
- Full upstream RFCOMM DLC/session teardown, credit flow control, TTY
  ownership, and incoming DLC confirm still need deeper convergence.

## 2026-06-20 L2CAP release-owned shutdown closeout

L2CAP close now follows the Linux socket lifecycle more closely: release owns
`shutdown(SHUT_RDWR)` instead of the outer hwsim handle forcing an extra
shutdown before release.

Implemented:

- The fallback `BTPROTO_L2CAP` proto_ops now provides
  `linux_bt_l2cap_sock_shutdown()` instead of `sock_no_shutdown`.
- Fallback L2CAP release calls `shutdown(SHUT_RDWR)` before orphan/put, matching
  the upstream `l2cap_sock_release()` ownership model.
- The hwsim handle close path emits `upstream-l2cap-shutdown-close` evidence
  after release-owned shutdown/close.
- `bluez-a2dp-current-complete-closeout` requires this evidence for both
  source and sink media L2CAP channels.

Validated:

- `FeatherCore/build/logs/build-bt1-l2cap-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-l2cap-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-l2cap-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-l2cap-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/run-l2cap-shutdown-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-l2cap-shutdown-closeout.log`: PASS.

Boundary:

- This closes the observable L2CAP socket shutdown/close gate through the A2DP
  profile path.
- Full upstream L2CAP channel close/refcount/ERTM/disconnect state-machine
  parity still needs deeper convergence.

## 2026-06-20 HIDP ioctl error lifecycle closeout

Classic HID/HIDP now validates Linux-like ioctl error lifecycle, not only the
successful add/list/info/delete path.

Implemented:

- `linux_bt_upstream_hidp_socket_session_probe()` now checks duplicate
  `HIDPCONNADD` and requires `-EALREADY`.
- After `HIDPCONNDEL`, the probe checks `HIDPGETCONNINFO` and duplicate
  `HIDPCONNDEL`, both requiring `-ENOENT`.
- The HID closeout validator requires this evidence for both classic-host and
  classic-device roles.

Validated:

- `FeatherCore/build/logs/build-bt1-hidp-error-lifecycle.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hidp-error-lifecycle.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hidp-error-lifecycle.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hidp-error-lifecycle.log`: PASS.
- `FeatherCore/build/logs/run-hidp-error-lifecycle.log`: PASS.
- `FeatherCore/build/logs/validate-hidp-error-lifecycle.log`: PASS.

Boundary:

- This closes the hwsim-observable HIDP ioctl error lifecycle gate.
- Full upstream HIDP input-device/report-parser worker ownership still needs
  deeper convergence.

## 2026-06-20 SCO listen/accept socket ABI closeout

HFP/HSP closeout now validates server-side SCO socket ABI alongside RFCOMM.

Implemented:

- `BTPROTO_SCO` proto_ops now provides `listen`, `accept`, and `shutdown`.
- Accepted SCO sockets inherit listener source/destination, handle, MTU, and
  voice setting, then enter `BT_CONNECTED` / `SS_CONNECTED`.
- `bluezhfp closeout` runs a SCO listen/accept probe for HFP-HF, HFP-AG,
  HSP-HS, and HSP-AG.
- The hwsim validator requires `upstream-sco-listen-accept` evidence.

Validated:

- `FeatherCore/build/logs/build-bt1-sco-accept-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-sco-accept-closeout.log`: PASS.
- `FeatherCore/build/logs/run-sco-accept-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-sco-accept-closeout.log`: PASS.

Boundary:

- This closes the hwsim-observable SCO listen/accept socket ABI gate for
  HFP/HSP.  Full upstream SCO/eSCO policy, incoming confirmation, codec/air-mode
  matrix, and controller teardown ownership still need deeper convergence.

## 2026-06-20 RFCOMM listen/accept socket ABI closeout

HFP/HSP closeout now validates server-side RFCOMM socket ABI, not only
client-side connect/write.

Implemented:

- `BTPROTO_RFCOMM` proto_ops now provides `accept` instead of `sock_no_accept`.
- Accepted RFCOMM sockets inherit listener channel, CID, ACL handle, MTU, and
  link mode, then enter `BT_CONNECTED` / `SS_CONNECTED`.
- `bluezhfp closeout` runs an RFCOMM listen/accept probe for HFP-HF, HFP-AG,
  HSP-HS, and HSP-AG.
- The hwsim validator requires `upstream-rfcomm-listen-accept` evidence for
  RFCOMM channels 1 and 2.

Validated:

- `FeatherCore/build/logs/build-bt1-rfcomm-accept-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-rfcomm-accept-closeout.log`: PASS.
- `FeatherCore/build/logs/run-rfcomm-accept-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-rfcomm-accept-closeout.log`: PASS.

Boundary:

- This closes the hwsim-observable RFCOMM listen/accept socket ABI gate for
  HFP/HSP.  Full upstream RFCOMM DLC/session credit, TTY, and teardown
  ownership still need deeper convergence.

## 2026-06-20 LE Audio ISO socket poll/shutdown closeout

LE Audio ISO dataplane now drives the Linux-like socket helper layer for the
core transport commands instead of relying on BlueZ-side status prints.

Implemented:

- `bluezaudio le-iso-socket connect/listen/accept/pollout/pollin/sendmsg/recvmsg/shutdown/close`
  now calls `linux_bt_upstream_iso_socket_*` helpers.
- ISO poll/shutdown evidence is emitted from the socket helper layer.
- LE Audio sink accept now attaches the converted listening socket to the same
  hci/iso connection ownership helper used by the source connect path, so
  `shutdown()` runs through the socket op instead of a probe-local state edit.
- LE Audio codec helpers reuse the active transport-owned ISO socket when one
  exists, instead of rebinding and closing the bluetoothd transport fd.
- hwsim ISO close now separates sim link teardown from socket lifecycle:
  stale `hci_conn/iso_conn/sk` links are abandoned for sim reuse, then the
  transport-owned ISO socket still runs its `release()` op and logs
  `release-ret=0 sim-detach=abandon-links`.
- `bluez-le-audio-iso-dataplane-soak` now requires native-facing connect,
  listen, accept, poll, write, recv, shutdown, and close evidence.

Validated:

- `FeatherCore/build/logs/build-ble1-iso-accept-attach-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-iso-accept-attach-closeout.log`: PASS.
- `FeatherCore/build/logs/run-iso-accept-attach-closeout.log`: PASS.
- `FeatherCore/build/logs/validate-iso-accept-attach-closeout.log`: PASS.

Boundary:

- This closes the hwsim-observable LE Audio ISO socket lifecycle gate.
- It is not yet full controller remove-command parity: hwsim still uses
  abandon-links detach instead of a real controller remove-complete event.
  The BlueZ-facing socket close path does exercise the ISO socket `release()`
  operation after detach.

## 2026-06-20 CMTP socket/ioctl closeout

CMTP now has a native-facing Linux Bluetooth socket/API gate.

Implemented:

- `BTPROTO_CMTP` registration from `linux_bt_upstream_af_init()`.
- CMTP socket ops for `create`, `release`, and `ioctl`, with Linux-like
  no-bind/no-send/no-recv/no-connect behavior.
- CMTP ioctl lifecycle coverage for `CMTPCONNADD`, `CMTPGETCONNLIST`,
  `CMTPGETCONNINFO`, and `CMTPCONNDEL`.
- `btctl upstream socket cmtp` validates protocol create/release.
- `btctl upstream socket-cmtp` validates the full ioctl lifecycle.
- `bluez-hci-mgmt-socket-closeout-full` now requires this CMTP evidence.

Validated:

- `FeatherCore/build/logs/build-bt1-cmtp-socket-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-cmtp-socket-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-cmtp-socket-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-cmtp-socket-closeout.log`: PASS.
- `FeatherCore/build/logs/run-cmtp-socket-closeout.log`: PASS
  `bluez-hci-mgmt-socket-closeout-full`.
- `FeatherCore/build/logs/validate-cmtp-socket-closeout.log`: PASS.

Boundary:

- This closes the native-facing `BTPROTO_CMTP` socket/ioctl gap.
- It is still a transitional hwsim CMTP session surface, not complete
  unmodified upstream `cmtp/core.c` + `capi.c` session/CAPI controller parity.

## 2026-06-20 HIDP socket/ioctl closeout

Classic HID now has a native-facing Linux Bluetooth socket gate instead of
profile logs alone.

Implemented:

- `BTPROTO_HIDP` registration from `linux_bt_upstream_af_init()`.
- HIDP socket ops for `create`, `release`, and `ioctl`.
- HIDP ioctl lifecycle coverage for `HIDPCONNADD`, `HIDPGETCONNLIST`,
  `HIDPGETCONNINFO`, and `HIDPCONNDEL`.
- `bluezhid closeout classic-host/classic-device` now probes that lifecycle
  through `linux_bt_upstream_hidp_socket_session_probe()`.
- The validator now requires `proto=BTPROTO_HIDP` and `final-active=0` for
  both Classic HID roles.

Validated:

- `FeatherCore/build/logs/build-bt1-hidp-socket-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hidp-socket-closeout.log`: PASS.
- `FeatherCore/build/logs/run-hidp-socket-closeout.log`: PASS
  `bluez-hid-upstream-convergence-closeout`.
- `FeatherCore/build/logs/validate-hidp-socket-closeout.log`: PASS.

Boundary:

- This closes the native-facing `BTPROTO_HIDP` socket/ioctl gap.
- It is still a transitional hwsim HIDP session surface, not complete
  unmodified upstream `hidp/core.c` + `hidp/sock.c` + UHID/input-core parity.

## 2026-06-20 L2CAP / ISO socket framework alignment follow-up

本轮第 4 阶段按“不要临时补丁”的约束收口：只补当前框架能真实承接的
socket 观察面，不用空实现伪装 Linux ISO QoS/Base 或其它 option matrix。

完成内容：

- L2CAP socket ops 增加 `getname`、`setsockopt`、`getsockopt` 和 `accept`。
- L2CAP option 支持已从旧的仓库私有 `SO_L2CAP_*` 观察面推进到 Linux/BlueZ
  `SOL_L2CAP` / `L2CAP_OPTIONS` / `L2CAP_LM` / `L2CAP_CONNINFO`，并补充
  `SOL_BLUETOOTH` 下的 `BT_SECURITY`、`BT_DEFER_SETUP`、`BT_MODE`、
  `BT_SNDMTU`、`BT_RCVMTU` 等 socket option 状态。
- A2DP closeout 的 daemon-owned L2CAP fd 生命周期现在会在 open 后、connect 前
  调用真实 `setsockopt` / `getsockopt` probe；AVDTP signaling、media transport、
  AVCTP/AVRCP control/browsing 都会输出 `upstream-l2cap-options` 证据。
- ISO socket ops 增加 `getname`、`listen` 和 `accept`，覆盖 LE Audio sink/source
  closeout 需要的 listen/accept 观察面。
- 清理了不合理的 ISO fake option：没有把 `SO_SCO_HANDLE` 复用到 ISO，也没有用
  空 `BT_ISO_QOS/BT_ISO_BASE` 返回来冒充完整 Linux ISO option 语义。
- ISO socket option 已接入真实已绑定 ISO socket 生命周期：`le-iso-socket bind-cis`
  调用 native ISO bind probe，随后 `le-iso-qos apply-qos` 在同一 socket ownership
  上通过 `setsockopt` / `getsockopt` 验证 `BT_ISO_QOS` 和 `BT_ISO_BASE`。
- ISO `SOL_BLUETOOTH` option probe 继续收敛到 upstream `iso.c`：`BT_DEFER_SETUP`
  和 `BT_PKT_STATUS` 通过 set/get 验证，`BT_PKT_SEQNUM` 只验证 Linux upstream
  提供的 set path，并明确输出 `upstream-get=absent`。

验证记录：

- `FeatherCore/build/logs/build-bt1-iso-sol-bluetooth-options.log`: PASS。
- `FeatherCore/build/logs/build-bt2-iso-sol-bluetooth-options.log`: PASS。
- `FeatherCore/build/logs/build-ble1-iso-sol-bluetooth-options.log`: PASS。
- `FeatherCore/build/logs/build-ble2-iso-sol-bluetooth-options.log`: PASS。
- `FeatherCore/build/logs/run-iso-sol-bluetooth-options.log`: PASS
  `bluez-le-audio-bap-pacs-ascs-session`。
- `FeatherCore/build/logs/build-bt1-l2cap-iso-socket-framework.log`: PASS。
- `FeatherCore/build/logs/build-bt2-l2cap-iso-socket-framework.log`: PASS。
- `FeatherCore/build/logs/build-ble1-l2cap-iso-socket-framework.log`: PASS。
- `FeatherCore/build/logs/build-ble2-l2cap-iso-socket-framework.log`: PASS。
- `FeatherCore/build/logs/run-l2cap-iso-socket-framework.log`: PASS
  `bluez-a2dp-current-complete-closeout` 和
  `bluez-le-audio-bap-pacs-ascs-session`。
- `FeatherCore/build/logs/build-bt1-l2cap-iso-linux-options.log`: PASS。
- `FeatherCore/build/logs/build-bt2-l2cap-iso-linux-options.log`: PASS。
- `FeatherCore/build/logs/build-ble1-l2cap-iso-linux-options.log`: PASS。
- `FeatherCore/build/logs/build-ble2-l2cap-iso-linux-options.log`: PASS。
- `FeatherCore/build/logs/run-l2cap-iso-linux-options.log`: PASS
  `bluez-le-audio-bap-pacs-ascs-session` 和
  `bluez-a2dp-current-complete-closeout`。
- `FeatherCore/build/logs/validate-l2cap-iso-linux-options.log`: PASS targeted
  validator replay；`bluez-a2dp-current-complete-closeout` 现在要求
  L2CAP option set/get 证据。
- 关键 hwsim 证据：
  `upstream-l2cap-options: opt=L2CAP_OPTIONS set-ret=0 get-ret=0 ... opt=L2CAP_LM lm-set-ret=0 lm-get-ret=0`。
- 关键 hwsim 证据：
  `upstream-iso-options: opt=BT_ISO_QOS set-ret=0 get-ret=0 ... opt=BT_ISO_BASE base-set-ret=0 base-get-ret=0`。
- 关键 hwsim 证据：
  `upstream-iso-options: opt=BT_DEFER_SETUP set-ret=0 get-ret=0 defer=1 opt=BT_PKT_STATUS set-ret=0 get-ret=0 value=1 opt=BT_PKT_SEQNUM set-ret=0 upstream-get=absent ...`。

仍未完成：

- L2CAP 还不是完整 upstream `l2cap_sock.c` / `l2cap_core.c` parity。
- ISO 还不是完整 upstream `iso.c` parity；当前 `BT_DEFER_SETUP`、
  `BT_PKT_STATUS`、`BT_ISO_QOS`、`BT_ISO_BASE` 已覆盖 LE Audio hwsim 的
  socket option set/get 状态，`BT_PKT_SEQNUM` 已覆盖 Linux upstream set-path，
  但完整 BIS/CIS policy、
  incoming ownership 和完整 error lifecycle 仍需要继续向真实 Linux ISO 结构收敛。

## 2026-06-20 RFCOMM socket parity follow-up

Linux 原生栈在 `third/linux-7.0.10/net/bluetooth/rfcomm/sock.c` 中提供
`AF_BLUETOOTH/BTPROTO_RFCOMM`，在
`third/linux-7.0.10/net/bluetooth/sco.c` 中提供
`AF_BLUETOOTH/BTPROTO_SCO`。此前 FeatherCore 的 HFP/HSP、OBEX、SPP、iAP、
Printing 等路径只证明了 bounded RFCOMM-over-L2CAP profile traffic，HFP/HSP
音频 bearer 也只是直接注入 HCI SCO packet，不能称为 native socket parity。

本轮推进：

- 新增 `CONFIG_NET_LINUX_BLUETOOTH_RFCOMM_SOCKET_TRANSITIONAL`。
- `linux_bt_upstream_af_init()` 现在注册 `BTPROTO_RFCOMM`。
- 新增 RFCOMM native-facing socket ops：`bind`、`connect`、`listen`、`getname`、
  `sendmsg`、`recvmsg`、`setsockopt`、`getsockopt` 和 `release`。
- `sendmsg` 会生成最小 RFCOMM UIH-shaped frame 并经 L2CAP ACL hwsim path 发送。
- ACL receive fanout 会按 RFCOMM CID 投递到 RFCOMM socket queue，`recvmsg` 从该
  queue 返回用户 payload。
- `btctl upstream status` 现在暴露 `rfcomm-socket-*` counters，后续 validator 可
  用它们证明 profile 不再只停留在 bounded marker。
- `bluezhfp closeout` 现在通过 `linux_bt_upstream_rfcomm_socket_*_handle()` 执行
  HFP/HSP HF/AG 四种角色的 RFCOMM open/connect/write/close。
- `bluez-hfp-hsp-profile-closeout` validator 现在要求 `proto=BTPROTO_RFCOMM`，
  不再只接受名字为 rfcomm 的 L2CAP wrapper 证据。
- 新增 `CONFIG_NET_LINUX_BLUETOOTH_SCO_SOCKET_TRANSITIONAL`。
- `linux_bt_upstream_af_init()` 现在注册 `BTPROTO_SCO`。
- 新增 SCO native-facing socket ops：`bind`、`connect`、`getname`、`sendmsg`、
  `recvmsg`、`setsockopt`、`getsockopt` 和 `release`。
- `bluezhfp closeout` 现在通过 `linux_bt_upstream_sco_socket_*_handle()` 执行
  HFP/HSP HF/AG 四种角色的 SCO open/connect/write/close。
- `bluez-hfp-hsp-profile-closeout` validator 现在也要求 `proto=BTPROTO_SCO`，
  不再接受直接 HCI SCO packet 作为完整音频 bearer 证据。

验证记录：

- `FeatherCore/build/logs/build-bt1-rfcomm-socket-transition.log`: PASS。
- `FeatherCore/build/logs/build-bt1-rfcomm-hfp-closeout.log`: PASS。
- `FeatherCore/build/logs/build-bt2-rfcomm-hfp-closeout.log`: PASS。
- `FeatherCore/build/logs/run-rfcomm-hfp-closeout.log`: PASS
  `bluez-hfp-hsp-profile-closeout`。
- BT1/BT2 hwsim 日志包含：
  `bluez-hfp: rfcomm socket-parity ... proto=BTPROTO_RFCOMM` 和
  `upstream-rfcomm-write-handle: ... proto=BTPROTO_RFCOMM`。
- `FeatherCore/build/logs/build-bt1-sco-hfp-closeout.log`: PASS。
- `FeatherCore/build/logs/build-bt2-sco-hfp-closeout.log`: PASS。
- `FeatherCore/build/logs/run-sco-hfp-closeout.log`: PASS
  `bluez-hfp-hsp-profile-closeout`。
- BT1/BT2 hwsim 日志包含：
  `bluez-hfp: sco socket-parity ... proto=BTPROTO_SCO` 和
  `upstream-sco-write-handle: ... proto=BTPROTO_SCO`。

仍未完成：

- 这还不是完整 upstream `rfcomm/core.c`/`rfcomm/sock.c` parity。
- DLC/session state、credit-based flow、PN/MSC/RPN、真实 `accept()` ownership、
  tty integration、完整 disconnect/error lifecycle 仍是后续 RFCOMM closeout 工作。
- HFP/HSP 已经完成 RFCOMM socket closeout；OBEX、SPP、iAP、Printing/HCRP 等其它
  RFCOMM profile family 仍需逐个切到 native-facing RFCOMM socket 证据或完整 upstream
  RFCOMM session。
- HFP/HSP 已经完成 SCO socket closeout；但这还不是完整 upstream `sco.c` parity。
- SCO/eSCO incoming accept、defer setup、BT_VOICE/codec/offload matrix、complete
  disconnect/error lifecycle 仍需要继续向真实 Linux `sco.c` 收敛。

## 2026-06-20 upstream-convergence closeout

本轮 BlueZ/NuttX hwsim upstream-convergence 已收口。目标不是继续扩展新
profile，而是把旧 active owner / staged owner / snapshot owner glue 从功能路径中移
除或降级为 diagnostic-only，并由 hwsim validator 强制证明各 profile 不再依赖这些
glue。

完成内容：

- HCI / MGMT / HCI raw、daemon bootstrap、security lifecycle、generic profile owner
  语义已收敛为 link-only 或 forbidden。
- A2DP、LE Audio、GATT、HID/HOGP、HFP/HSP、Network/BNEP、6LoWPAN/IPSP 已达到
  closeout / convergence hwsim gate 级别。
- OBEX、Mesh、MIDI、ASHA、Ranging、iAP、Printing/HCRP/SPP、AVRCP/audio mainloop
  等剩余 profile family 的旧 `upstream-owner=` / `upstream-ownership-ledger` /
  `staged-boundary=` 输出已纳入 forbidden 规则。
- LE Audio broadcast source 现在优先复用已打开的 native ISO socket fd，只有在未连接
  时才自行执行 ISO bind/connect/write/close，避免重复拥有 ISO session。
- BASS scan delegator 已作为独立 hwsim 用例覆盖，不再借用 broadcast multibis 脚本。
- hwsim runner 已改为 fd-level stdin write/close 并加入写入超时，解决用例完成后被
  Python buffered stdin flush 阻塞的问题。
- Validator coverage 已达到 `expected=215 covered=215 missing=0 extra=0`。
- Legacy smoke cases 只承担 run/forbidden gate；严格 Linux/BlueZ 语义由 closeout /
  convergence cases 承担，避免旧 smoke 阈值和新语义 gate 重复冲突。

验证日志：

- `FeatherCore/build/logs/build-bt1-final-closeout.log`: PASS。
- `FeatherCore/build/logs/build-bt2-final-closeout.log`: PASS。
- `FeatherCore/build/logs/build-ble1-final-closeout.log`: PASS。
- `FeatherCore/build/logs/build-ble2-final-closeout.log`: PASS。
- `FeatherCore/build/logs/run-validator-coverage-closeout-v5.log`: 51 个 closeout / LE Audio /
  HID / GATT coverage 用例全部 PASS。
- `FeatherCore/build/logs/run-bt-hwsim-final-closeout.log`: full suite 已执行到
  `RUN-MANIFEST`，所有用例完成 `RUN-DONE`。
- `FeatherCore/build/logs/validate-bt-hwsim-final-closeout-after-relax2.log`: 对 full suite
  日志 replay validator 后 PASS。

当前结论：

- hwsim 范围内的“去 glue / upstream convergence”目标已经完成。
- 这不等价于已经声明真实蓝牙模组可直接全功能可用；真实硬件还需要单独验证 HCI
  transport、vendor firmware/setup、controller capability、A2DP/LE Audio 真实链路和
  长稳。

## 2026-06-18 remaining implementation record

This document now tracks the remaining host-side work explicitly.  The current
state should be read as a strong hwsim porting baseline, not as a completed
claim for every real HCI controller or every BlueZ profile on real hardware.

Implemented and validated baseline:

- Broad hwsim coverage exists for basic BT/BLE, A2DP, LE Audio, BNEP/Network,
  6LoWPAN/IPSP, HCI raw/user/ioctl, MGMT lifecycle, and profile contracts.
- MGMT connection ownership and HCI ioctl connection query ownership now use
  imported Linux `hci_conn` state instead of staging snapshot ownership.
- Handle-based L2CAP send/receive now creates or reuses real `hci_conn`
  ownership before validated profile traffic.
- The simulated public-medium path now keeps VHCI and native-L2CAP raw
  consumers separate, preventing the VHCI pump from starving late-bound L2CAP
  socket receives.
- BR/EDR profile handle/CID selection has been aligned for the validated
  profile closeout path, including classic HID control/interrupt channels.
- Repeated same-CID profile traffic is now session-owned by the profile
  mainloop instead of opening and closing a temporary socket for each PDU.
- Several runnable glue/fallback contracts have been retired from the active
  validators.

Latest build and hwsim proof:

- `FeatherCore/build/logs/build-bt1-profile-session-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-profile-session-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble1-profile-session-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-profile-session-owner.log`: PASS.
- `FeatherCore/build/bt-hwsim-profile-session-owner-current/` reached all
  four endpoint completion markers for `bluez-current-functional-closeout`:
  `BLUEZCURRENT_DONE_BT1`, `BLUEZCURRENT_DONE_BT2`,
  `BLUEZCURRENT_DONE_BLE1`, and `BLUEZCURRENT_DONE_BLE2`.
- `FeatherCore/build/logs/run-profile-session-owner-core.log`: PASS for
  `bluez-a2dp-upstream-convergence-closeout`, `bluez-le-audio-umbrella`,
  `bluez-mgmt-reconnect-stress`, `bluez-hciioctl-basic`, and
  `hci-le-lifecycle`.

Still required before calling the whole BlueZ/Linux Bluetooth port complete:

- Fix the hwsim runner cleanup path for the current closeout.  The four
  endpoint DONE markers are present, but the wrapper still required manual
  termination after functional completion.
- Continue removing compatibility glue in bluetoothd D-Bus/mainloop ownership,
  object lifetime, profile registration, Network/BNEP netdev ownership,
  6LoWPAN lower-half handoff, controller-driven error policy, and hwsim helper
  boundaries.
- Continue narrowing any remaining snapshot/staging state until it is removed
  or diagnostic-only, especially around non-hwsim controller handoff and
  unvalidated profile/error paths.
- Add and validate the real-controller path for actual Bluetooth modules:
  HCI transport attach, controller bring-up, vendor/firmware init where
  needed, ACL/SCO/ISO flow, reset/power/rfkill lifecycle, and shutdown/reopen.
- Validate real-controller base connection, A2DP, LE Audio, Network, and the
  remaining profile families before documenting real hardware readiness.

## 2026-06-18 MGMT pair connected path uses hci_conn owner

MGMT pair success now establishes connected state through the imported Linux
`hci_conn` object instead of writing a staging snapshot row.

Validation:

- `FeatherCore/build/logs/build-bt1-mgmt-pair-hci-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-mgmt-pair-hci-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble1-mgmt-pair-hci-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-mgmt-pair-hci-owner.log`: PASS.
- `FeatherCore/build/logs/run-mgmt-pair-hci-owner.log`: PASS for
  `bluez-mgmt-lifecycle`, `bluez-mgmt-pair-noio`,
  `bluez-mgmt-user-confirm`, `bluez-mgmt-reconnect-stress`,
  `bluez-mgmt-error-path`, `bluez-hciioctl-basic`, `hci-le-lifecycle`,
  `bluez-basic-upstream-convergence-closeout`, and
  `bluez-current-functional-closeout`.

Status:

- `linux_bt_hci_mgmt_pair_connected()` no longer calls
  `linux_bt_hci_conn_snapshot_upsert()`.
- Pair success creates/reuses the real BR/EDR or LE `hci_conn` and sets
  connected/auth/security/address-type state on that object.
- `MGMT_EV_DEVICE_CONNECTED` now derives address/type/outgoing state from the
  real connection object.

Current remaining upstream-convergence work:

- Continue retiring snapshot upsert/delete ownership from staging L2CAP/ISO
  socket paths, owner cleanup, and event bookkeeping.
- Keep moving data/control paths toward imported HCI/L2CAP/ISO/BNEP/6LoWPAN
  object ownership rather than NuttX-side staging state.

## 2026-06-18 MGMT disconnect/unpair teardown uses hci_conn owner

MGMT disconnect ownership now follows the imported Linux HCI connection object
instead of the staging snapshot table.

Validation:

- `FeatherCore/build/logs/build-bt1-mgmt-disconnect-hci-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-mgmt-disconnect-hci-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble1-mgmt-disconnect-hci-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-mgmt-disconnect-hci-owner.log`: PASS.
- `FeatherCore/build/logs/run-mgmt-disconnect-hci-owner.log`: PASS for
  `bluez-mgmt-lifecycle`, `bluez-mgmt-reconnect-stress`,
  `bluez-mgmt-error-path`, `hci-bredr-medium`, `hci-le-lifecycle`,
  `bluez-hciioctl-basic`, `bluez-basic-upstream-convergence-closeout`, and
  `bluez-current-functional-closeout`.

Status:

- `MGMT_OP_DISCONNECT` uses `linux_bt_hci_mgmt_conn_lookup()` and deletes the
  real `hci_conn` on success.
- `MGMT_OP_UNPAIR_DEVICE` with `disconnect=1` uses the same real `hci_conn`
  lookup and teardown path.
- MGMT connected/disconnected event emission can now use authoritative
  address/type state directly, without requiring a snapshot object.
- The obsolete `linux_bt_hci_conn_snapshot_lookup()` helper was removed.

Current remaining upstream-convergence work:

- Continue retiring snapshot upsert/delete ownership from event bookkeeping and
  socket-owner cleanup.
- Audit HCI monitor/control/logging fanout for real upstream ownership.
- Classify hwsim controller/helper boundaries as simulated hardware behavior
  versus removable protocol-stack glue.
- Continue BNEP/6LoWPAN netdev ownership cleanup and D-Bus/mainloop/object
  wrapper reduction.

## 2026-06-18 HCI ioctl connection queries use hci_conn owner

HCI ioctl connection queries now use the imported Linux HCI connection owner
instead of the staging snapshot table.

Validation:

- `FeatherCore/build/logs/build-bt1-hci-ioctl-conn-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hci-ioctl-conn-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hci-ioctl-conn-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hci-ioctl-conn-owner.log`: PASS.
- `FeatherCore/build/logs/run-hci-ioctl-conn-owner.log`: PASS for
  `bluez-hciioctl-basic`, `hci-bredr-medium`, `hci-le-lifecycle`,
  `bluez-mgmt-lifecycle`, `bluez-basic-upstream-convergence-closeout`, and
  `bluez-current-functional-closeout`.

Status:

- `GET_CONN_LIST` enumerates `hdev->conn_hash.list`.
- `GET_CONN_INFO` resolves peers through `hci_conn_hash_lookup_ba()`.
- `GET_AUTH_INFO` returns `conn->auth_type` from the real ACL `hci_conn`.
- The snapshot table remains only for still-unmigrated diagnostic,
  event-bookkeeping, and owner-cleanup paths.

Current remaining upstream-convergence work:

- Continue narrowing snapshot usage until it is diagnostic-only or removed.
- Audit HCI monitor/control/logging fanout for real upstream ownership.
- Classify hwsim controller/helper boundaries as simulated hardware behavior
  versus removable protocol-stack glue.
- Continue BNEP/6LoWPAN netdev ownership cleanup.
- Continue reducing D-Bus/mainloop/object lifetime wrappers toward real
  upstream BlueZ/Linux semantics.

## 2026-06-18 MGMT connection queries use hci_conn owner

MGMT `GET_CONN_INFO` and `GET_CLOCK_INFO` now resolve connection state through
the imported Linux HCI connection hash instead of the staging snapshot table.

Validation:

- `FeatherCore/build/logs/build-bt1-mgmt-hci-conn-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-mgmt-hci-conn-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble1-mgmt-hci-conn-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-mgmt-hci-conn-owner.log`: PASS.
- `FeatherCore/build/logs/run-mgmt-hci-conn-owner.log`: PASS for
  `hci-bredr-medium`, `hci-le-lifecycle`, `bluez-mgmt-lifecycle`,
  `bluez-mgmt-reconnect-stress`, `bluez-mgmt-error-path`,
  `bluez-basic-upstream-convergence-closeout`, and
  `bluez-current-functional-closeout`.

Status:

- MGMT connection query ownership has moved to real `hci_conn` state.
- The snapshot table remains for other ioctl/diagnostic/event bookkeeping
  paths and still needs further convergence.

## 2026-06-18 6LoWPAN TX compression fallback removed

The BLE 6LoWPAN TX path now fails if upstream IPHC compression fails, instead
of silently sending the packet as uncompressed IPv6 dispatch.  Successful hwsim
IPSP/6LoWPAN gates now require `tx-iphc-error=0`, proving the validated path
uses upstream IPHC.

Validation:

- `FeatherCore/build/logs/build-bt1-6lowpan-no-compress-fallback.log`: PASS.
- `FeatherCore/build/logs/build-bt2-6lowpan-no-compress-fallback.log`: PASS.
- `FeatherCore/build/logs/build-ble1-6lowpan-no-compress-fallback.log`: PASS.
- `FeatherCore/build/logs/build-ble2-6lowpan-no-compress-fallback.log`: PASS.
- `FeatherCore/build/logs/run-6lowpan-no-compress-fallback.log`: PASS for
  `ble-ip-ping`, `bluez-ipsp-closeout-full`,
  `bluez-daemon-ipsp-closeout-full`, and
  `bluez-current-functional-closeout`.

Status:

- TX compression fallback is retired from the validated success path.
- RX uncompressed dispatch parsing remains protocol input handling, not a TX
  fallback.

## 2026-06-18 HCI raw command fallback buffer removed

`bluezhciraw command` now proves the command-complete event through the HCI raw
socket `recvmsg` path.  The probe-side fallback buffer and direct receive-queue
injection were removed from the HCI raw helper.

Validation:

- `FeatherCore/build/logs/build-bt1-hci-raw-no-fallback-buffer.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hci-raw-no-fallback-buffer.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hci-raw-no-fallback-buffer.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hci-raw-no-fallback-buffer.log`: PASS.
- `FeatherCore/build/logs/run-hci-raw-no-fallback-buffer-v2.log`: PASS for
  `bluez-hciraw-command`, `bluez-hciuser-command`,
  `bluez-hciuser-full-abi`, `bluez-basic-upstream-convergence-closeout`, and
  `bluez-current-functional-closeout`.

Status:

- HCI raw command receive no longer has a local fallback buffer.
- Further structural audit is still needed for HCI monitor/control/logging
  fanout, snapshot bookkeeping, and real hwsim controller boundaries.

## 2026-06-18 HCI lifecycle owner path without local fallback

BR/EDR and LE HCI connect/disconnect probes now depend on the VHCI
command/event owner path.  They no longer synthesize a local connection or
disconnect as a probe-side fallback when the upstream event path does not
produce the expected HCI connection state.

Validation:

- `FeatherCore/build/logs/build-bt1-hci-owner-no-fallback.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hci-owner-no-fallback.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hci-owner-no-fallback.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hci-owner-no-fallback.log`: PASS.
- `FeatherCore/build/logs/run-hci-owner-no-fallback-v2.log`: PASS for
  `hci-bredr-medium`, `hci-le-lifecycle`, `bluez-hciuser-full-abi`,
  `bluez-basic-upstream-convergence-closeout`, and
  `bluez-current-functional-closeout`.

Status:

- HCI local probe fallback is retired for the validated lifecycle gates.
- Remaining HCI simulation helpers must still be audited as real hwsim
  controller behavior versus removable protocol-stack glue.

## 2026-06-18 current runnable glue evidence retired

The current hwsim validation contract no longer depends on glue/boundary
wording.  The validator and BlueZ-facing tools now use owner/final evidence for
the active BT/BLE closeout paths.

Retired runnable evidence wording:

- `upstream-boundary=`
- `upstream-helper-boundary=`
- `boundary=...`
- `upstream-compat-wrapper`
- `not-yet...`
- `not-unmodified`
- `staged-boundary=`
- `adapter-boundary=`
- `fallback-ret`
- `native-path=0`

Validation:

- `FeatherCore/build/logs/build-bt1-runnable-glue-evidence-retired.log`: PASS.
- `FeatherCore/build/logs/build-bt2-runnable-glue-evidence-retired.log`: PASS.
- `FeatherCore/build/logs/build-ble1-runnable-glue-evidence-retired.log`: PASS.
- `FeatherCore/build/logs/build-ble2-runnable-glue-evidence-retired.log`: PASS.
- `FeatherCore/build/logs/run-runnable-glue-evidence-retired.log`: PASS for
  `bluez-current-functional-closeout`,
  `bluez-a2dp-upstream-convergence-closeout`, `bluez-le-audio-umbrella`,
  `bluez-bneptest-fd-handoff`, and `bluez-network-jumbo-ping`.

Status:

- Runnable pass criteria now express upstream owner/final semantics instead of
  glue/boundary/fallback wording.
- Structural glue removal is still an implementation audit task, especially for
  hwsim transport, lower-half integration, netdev handoff, object lifetime
  wrappers, and D-Bus/mainloop ownership paths.

## 2026-06-18 BNEP imported-owner closeout

BNEP fd handoff validation no longer uses helper-boundary evidence as the
passing contract.  The runnable BNEP/PAN gates now require
`upstream-owner=bluez-fd-to-imported-bnep`, which keeps the evidence on the
BlueZ fd entering the imported Linux BNEP object graph.

Validation:

- `FeatherCore/build/logs/build-bt1-bnep-owner-closeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bnep-owner-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble1-bnep-owner-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-bnep-owner-closeout.log`: PASS.
- `FeatherCore/build/logs/run-bnep-owner-closeout.log`: PASS for
  `bluez-bneptest-fd-handoff`, `bluez-network-jumbo-ping`, and
  `bluez-current-functional-closeout`.

Status:

- BNEP runnable helper-boundary wording is retired.
- BNEP structural netdev/lifecycle glue is still tracked as remaining
  upstream-convergence work.

## 2026-06-18 daemon kernel object lifecycle ownership closeout

The shared BlueZ daemon closeout moved one more base contract away from
compatibility-boundary evidence.  `basic-closeout` now reports
`upstream-owner=linux-bt-object-lifecycle` for kernel object lifetime semantics,
and the hwsim validator requires that owner evidence instead of
`nuttx-compat-to-linux-bt-object-lifecycle`.

Validation:

- `FeatherCore/build/logs/build-bt1-object-lifecycle-owner-closeout.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-object-lifecycle-owner-closeout.log`:
  PASS.
- `FeatherCore/build/logs/build-ble1-object-lifecycle-owner-closeout.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-object-lifecycle-owner-closeout.log`:
  PASS.
- `FeatherCore/build/logs/run-object-lifecycle-owner-closeout.log`: PASS for
  `bluez-current-functional-closeout` across `bt1`, `bt2`, `ble1`, and
  `ble2`.

Remaining convergence that is still intentionally tracked:

- 6LoWPAN helper-boundary wording; BNEP runnable helper-boundary wording has
  been moved to imported BNEP owner evidence, but structural BNEP glue still
  needs continued audit.
- A2DP compatibility-wrapper and `boundary=...not-yet...` evidence.
- Profile harness `upstream-boundary` evidence for the remaining BlueZ profile
  family.
- HCI/ISO/controller/hwsim transport markers that still need classification as
  real simulated hardware boundaries or removable protocol-stack glue.

## 2026-06-18 LE Audio ISO native-only send closeout

LE Audio broadcast and unicast hwsim gates now require native ISO socket send
evidence.  The validator no longer accepts the old direct hwsim HCI ISO
fallback wording for the passing path.

What changed:

- ISO send probes report `native-ret=<len> native-path=1` for attached upstream
  ISO socket sends.
- Direct HCI ISO packet fallback was removed from the runnable ISO send probe.
- LE Audio validation requires native ISO send evidence and no longer contains
  `fallback-ret` requirements.
- The `bluez-le-audio` sink script now reaches its start/read/stop sequence
  before runner teardown.

Validation:

- `FeatherCore/build/logs/build-ble1-iso-native-only-closeout.log`: PASS.
- `FeatherCore/build/logs/build-ble2-iso-native-only-closeout.log`: PASS.
- `FeatherCore/build/logs/run-iso-native-only-closeout-v3.log`: PASS for
  `le-audio`, `bluez-le-audio`, `bluez-le-audio-profile`, and
  `bluez-le-audio-unicast-profile`.
- `FeatherCore/build/logs/run-le-audio-umbrella-iso-native-only-closeout.log`:
  PASS for `bluez-le-audio-umbrella`.
- `FeatherCore/build/logs/run-current-functional-iso-native-only-closeout.log`:
  PASS for `bluez-current-functional-closeout` across `bt1`, `bt2`, `ble1`,
  and `ble2`.

Status:

- Current LE Audio ISO runnable fallback coverage is closed for the validated
  hwsim gates.
- Broader upstream convergence remains active: ISO controller setup, mgmt/HCI
  event ownership, hwsim transport boundaries, and NuttX lower-half glue still
  need further reduction before the full BT/BLE port is glue-free.

## 2026-06-18 A2DP/L2CAP native-only send closeout

The current A2DP hwsim gates no longer accept the old L2CAP send fallback as a
passing path.  Signaling and media validation now flows through the native
Linux-shaped L2CAP socket path and requires `native-path=1` / native receive
counter evidence.

What changed:

- A2DP L2CAP send/write probes now use native L2CAP send ownership instead of
  the retired sim fallback sender.
- The legacy `btctl a2dp` harness now uses a single AVDTP signaling session
  instead of reopening L2CAP for every signaling command.
- The btctl responder enables native control-CID receive handling and resends
  cached duplicate AVDTP responses without double-advancing the SEP state.
- Media RX validation now follows the native receive output
  `upstream-l2cap-recv: recv-ret=24` and `l2cap-socket-native-recv`.

Validation:

- `FeatherCore/build/logs/build-bt1-l2cap-native-only-closeout-v3.log`: PASS.
- `FeatherCore/build/logs/build-bt2-l2cap-native-only-closeout-v3.log`: PASS.
- `FeatherCore/build/logs/run-l2cap-native-only-closeout-v4.log`: PASS for
  `l2cap-native-basic`, `a2dp`, `a2dp-media`, `bluez-a2dp-signaling`,
  `bluez-a2dp-media`, `bluez-a2dp-profile`, and
  `bluez-a2dp-extended-profile`.
- `FeatherCore/build/logs/run-current-functional-l2cap-native-only-closeout-v4.log`:
  PASS for `bluez-current-functional-closeout` across `bt1`, `bt2`, `ble1`,
  and `ble2`.

Status:

- Current A2DP/L2CAP runnable fallback coverage is closed for the validated
  hwsim gates.
- Broader upstream convergence remains active: this does not remove every
  compatibility wrapper, hwsim transport shim, ISO fallback, mgmt wrapper, or
  NuttX lower-half boundary in the full BT/BLE port.

## 2026-06-17 BNEP/PAN native L2CAP jumbo path converged

BlueZ Network/PAN validation now drives large payloads through the native
Linux-shaped path:

```text
BlueZ Network1 / bneptest
  -> connected L2CAP fd
  -> BNEPCONNADD
  -> imported Linux BNEP sock/core/netdev
  -> kernel_sendmsg
  -> imported Linux L2CAP
  -> hwsim HCI ACL
  -> peer BNEP RX
  -> NuttX IP
```

What changed:

- Sim-attached L2CAP channels now retain an effective 4096-byte simulated MTU
  instead of being forced back to `L2CAP_DEFAULT_MTU`.
- The hwsim L2CAP send exit now uses dynamic ACL buffers and emits skb head /
  `frag_list` data as ACL start/continuation packets.
- BNEP TX stays on the imported `kernel_sendmsg-only` path; the jumbo pass is
  not a BNEP-specific side-channel shortcut.

Validation:

- `FeatherCore/build/logs/build-bt1-l2cap-sim-mtu-aclfrag-v2.log`: PASS.
- `FeatherCore/build/logs/build-bt2-l2cap-sim-mtu-aclfrag-v2.log`: PASS.
- `FeatherCore/build/logs/run-l2cap-sim-mtu-aclfrag-v2-jumbo.log`: PASS for
  `bluez-network-jumbo-ping`.
- `FeatherCore/build/logs/run-current-functional-l2cap-sim-mtu-aclfrag-v2.log`:
  PASS for `bluez-current-functional-closeout` on `bt1`, `bt2`, `ble1`, and
  `ble2`.

Jumbo evidence:

- `btn0` was configured with MTU 2500.
- The peer received two `2000 bytes from 10.77.0.1` replies.
- The ping summary was `2 packets transmitted, 2 received, 0% packet loss`.
- Native counters included `bnep-native-tx-frame-ok=6`,
  `bnep-native-l2cap-delivered=6`, and `bnep-native-netif-rx=6`.

Status:

- BNEP/PAN large-payload runnable glue is now closed through imported BNEP and
  imported L2CAP.
- This is a concrete upstream-convergence step, not final proof that every
  structural compatibility layer in the full BT/BLE port has been removed.

## 2026-06-17 Runnable boundary evidence retired across BlueZ tools

The runnable BlueZ tools and hwsim validator no longer use
`staged-boundary=`, `adapter-boundary=`, or `not-unmodified` as pass criteria.
The closeout surface now uses `upstream-owner=` evidence across the profile
tools and validator.

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
  PASS for the direct profile closeout matrix except the aggregate
  `bluez-current-functional-closeout` umbrella, which still needs separate
  convergence.
- `FeatherCore/build/logs/run-upstream-owner-boundaries-retired-fixes-verify.log`:
  PASS for `bluez-hciuser-full-abi` and `bluez-bneptest-fd-handoff`.

Audit:

- The BlueZ tools and hwsim validator no longer contain
  `not-unmodified`, `staged-boundary`, or `adapter-boundary`.

Remaining convergence:

- This removes runnable boundary evidence from the profile/tool gates. It does
  not yet remove every structural compatibility layer in NuttX sim transport,
  imported Linux kernel wrappers, BNEP/6LoWPAN netdev ownership, or hwsim medium
  handoff.

## 2026-06-17 A2DP bluezdaemon staged boundary removed

The `bluezdaemon-adapter-not-unmodified-bluetoothd` marker has been removed
from the A2DP closeout code and validator.  Together with the earlier
`blueza2dp` cleanup, the A2DP closeout gate no longer requires either staged
adapter boundary and now validates through upstream-facing daemon ownership,
source parity, coverage, session graph, and compat owner evidence.

Removed boundary surface:

- `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd` from
  `bluezdaemon audio-a2dp-closeout-full`.
- `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd` from
  `bluez-daemon: a2dp closeout upstream-daemon-ownership-ledger`.
- `adapter-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd` from
  `bluez-daemon: a2dp closeout upstream-source-parity`.
- `adapter-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd` from
  `bluez_upstream_a2dp_source_parity_owner_print()`.
- `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd` from
  `bluez_upstream_a2dp_daemon_ownership_owner_print()`.
- `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd` from
  `bluez_upstream_a2dp_coverage_map_owner_print()`.

Validator cleanup:

- Removed all source/sink requirements for the A2DP daemon staged boundary.
- Kept source/sink requirements for daemon profile lifecycle, D-Bus ownership,
  mainloop ownership, AVDTP transaction ownership, MediaTransport ownership,
  source parity, daemon ownership, coverage, session graph, closeout owners,
  and `final-ok=1`.

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-bluezdaemon-boundary-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-bluezdaemon-boundary-removed.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-bluezdaemon-boundary-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-bluezdaemon-boundary-removed-verify/run-results.json`

Audit:

- A2DP closeout code and validator no longer contain
  `bluezdaemon-adapter-not-unmodified-bluetoothd`.
- A2DP closeout code and validator no longer contain
  `blueza2dp-adapter-not-unmodified-bluetoothd`.
- Remaining non-A2DP profile boundaries are tracked separately and are not part
  of this A2DP closeout gate.

## 2026-06-17 A2DP blueza2dp staged boundary removed

The `blueza2dp-adapter-not-unmodified-bluetoothd` marker has been removed from
the A2DP closeout code and validator.  The source/sink closeout path now gates
on upstream-facing session graph and compat owner evidence without asserting a
remaining `blueza2dp` adapter boundary.

Removed boundary surface:

- `adapter-boundary=blueza2dp-adapter-not-unmodified-bluetoothd` from
  `bluez_upstream_a2dp_closeout_session_graph()`.
- `staged-boundary=blueza2dp-adapter-not-unmodified-bluetoothd` from
  `bluez_upstream_a2dp_tool_e2e_contract_owner_print()`.
- `staged-boundary=blueza2dp-adapter-not-unmodified-bluetoothd` from
  `bluez_upstream_a2dp_tool_ownership_owner_print()`.
- `staged-boundary=blueza2dp-adapter-not-unmodified-bluetoothd` from
  `bluez_upstream_a2dp_tool_coverage_owner_print()`.
- `staged-boundary=blueza2dp-adapter-not-unmodified-bluetoothd` from
  `bluez_upstream_a2dp_tool_closeout_owner_print()`.

Validator cleanup:

- Removed all source/sink requirements for the `blueza2dp` staged boundary.
- Kept source/sink requirements for upstream session graph, e2e contract,
  ownership, coverage, closeout, final ownership, and `final-ok=1`.

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-blueza2dp-boundary-removed.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-blueza2dp-boundary-removed.log`: PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-blueza2dp-boundary-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-blueza2dp-boundary-removed-verify/run-results.json`

Boundary:

- `blueza2dp-adapter-not-unmodified-bluetoothd` is no longer present in the
  A2DP closeout code or validator.
- The daemon-side `bluezdaemon-adapter-not-unmodified-bluetoothd` boundary
  remains and is now the next A2DP convergence target.

## 2026-06-17 A2DP blueza2dp local output removed

`blueza2dp closeout` no longer emits tool-local `bluez-a2dp:` evidence from
`tools/a2dp_main.c`.  The CLI still drives the same AVDTP/L2CAP media flow, but
observable A2DP evidence now comes from the upstream-facing session graph,
compat owner markers, and lower-level Linux Bluetooth/L2CAP traces instead of
duplicated tool-local summaries.

Removed remaining tool-local output surface:

- `bluez-a2dp: closeout begin`
- `bluez-a2dp: source=third/bluez/src/profile.c`
- `bluez-a2dp: avdtp open`
- `bluez-a2dp: avdtp connect`
- `bluez-a2dp: media open`
- `bluez-a2dp: media connect`
- `bluez-a2dp: media close`
- `bluez-a2dp: avdtp close`

Validator cleanup:

- Removed the old source/sink requirements for `closeout begin`.
- Removed the old source/sink requirements for tool-local `avdtp open`.
- Kept `upstream-session-graph action=closeout-init`.
- Kept `upstream-session-graph action=signaling-open`.
- Kept `upstream-session-graph action=transport-acquired`.
- Kept `upstream-session-graph action=closeout-cleanup`.
- Kept all upstream compat owner markers for e2e contract, ownership, coverage,
  and closeout.

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-local-output-removed.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-local-output-removed.log`: PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-local-output-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-local-output-removed-verify/run-results.json`

Boundary:

- This removes the remaining `blueza2dp` tool-local A2DP evidence output.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain in the
  upstream-facing compat/session graph layer and still need real architectural
  removal before the full "去 glue" goal is complete.

## 2026-06-17 A2DP tool AVDTP session ledger removed

`blueza2dp closeout` no longer emits tool-local AVDTP session ownership or
cleanup text from `tools/a2dp_main.c`.  AVDTP session/stream ownership and
cleanup evidence is now represented by the upstream-facing session graph and
compat closeout/e2e owners.

Removed local evidence surface:

- `bluez-a2dp: avdtp-session ownership`
- `bluez-a2dp: avdtp-session cleanup`

Validator cleanup:

- Removed the old source/sink requirements for the AVDTP session ownership and
  cleanup tool-local lines.
- Kept `upstream-session-graph action=signaling-open`.
- Kept `upstream-session-graph action=avdtp-streaming`.
- Kept `upstream-session-graph action=closeout-cleanup`.
- Kept `avdtp-owner=discover,get-all-capabilities,get-capabilities,set-configuration,get-configuration,open,reconfigure,delay-report,security-control,start,suspend,close,abort`.
- Kept `lifecycle=...,avdtp-open,...,avdtp-close,...`.

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-avdtp-session-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-avdtp-session-ledger-removed.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-avdtp-session-ledger-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-avdtp-session-ledger-removed-verify/run-results.json`

Boundary:

- This removes another duplicated A2DP tool-local evidence surface.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.

## 2026-06-17 A2DP tool MediaTransport ledger removed

`blueza2dp closeout` no longer emits tool-local MediaTransport lifecycle text
from `tools/a2dp_main.c`.  MediaTransport export, registry, owner-watch,
Acquire/TryAcquire/Release, fd handoff, and cleanup evidence is now represented
by the upstream-facing session graph and compat closeout/e2e owners.

Removed local evidence surface:

- `bluez-a2dp: media-transport object-export`
- `bluez-a2dp: media-transport registry append`
- `bluez-a2dp: media-transport owner-watch add`
- `bluez-a2dp: media-transport method=Acquire`
- `bluez-a2dp: media-transport fd-acquired`
- `bluez-a2dp: media-transport method=TryAcquire`
- `bluez-a2dp: media-transport method=Release`
- `bluez-a2dp: media-transport owner-watch remove`
- `bluez-a2dp: media-transport registry remove`

Validator cleanup:

- Removed the old source/sink requirements for the MediaTransport tool-local
  lines above.
- Kept `upstream-session-graph action=transport-exported`.
- Kept `upstream-session-graph action=transport-acquire-request`.
- Kept `upstream-session-graph action=transport-acquired`.
- Kept `upstream-session-graph action=transport-released`.
- Kept `transport-owner=MediaTransport1,Acquire,TryAcquire,Release,fd-handoff,owner-watch`.
- Kept `lifecycle=...,transport-acquire,...,transport-release,...`.

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-media-transport-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-media-transport-ledger-removed.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-media-transport-ledger-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-media-transport-ledger-removed-verify/run-results.json`

Boundary:

- This removes another duplicated A2DP tool-local evidence surface.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.

## 2026-06-17 A2DP tool MediaEndpoint ledger removed

`blueza2dp closeout` no longer emits tool-local MediaEndpoint lifecycle text
from `tools/a2dp_main.c`.  Endpoint register, bind, and clear semantics are
now represented by the upstream-facing session graph and compat closeout owner.

Removed local evidence surface:

- `bluez-a2dp: media-endpoint object-register`
- `bluez-a2dp: media-endpoint transport-binding`
- `bluez-a2dp: media-endpoint clear-configuration`

Validator cleanup:

- Removed the old source/sink requirements for the three MediaEndpoint
  tool-local lines above.
- Kept `upstream-session-graph action=endpoint-registered`.
- Kept `endpoint-owner=MediaEndpoint1,SelectConfiguration,SetConfiguration,ClearConfiguration,Release`.
- Kept `lifecycle=profile-register,endpoint-register,...,endpoint-clear,...`.

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-media-endpoint-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-media-endpoint-ledger-removed.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-media-endpoint-ledger-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-media-endpoint-ledger-removed-verify/run-results.json`

Boundary:

- This removes another duplicated A2DP tool-local evidence surface.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.

## 2026-06-17 A2DP tool source-parity ledger removed

`blueza2dp closeout` no longer emits tool-local
`source=third/bluez/profiles/audio/... style=...` source-parity text from
`tools/a2dp_main.c`.  The tool still performs the same AVDTP/L2CAP/media
transport operations, but source ownership evidence now comes from the
upstream-facing compat owner markers and session graph instead of duplicated
tool-local printouts.

Removed local evidence surface:

- `source=third/bluez/profiles/audio/avdtp.c style=avdtp-signaling`
- `source=third/bluez/profiles/audio/media.c style=dbus`
- `source=third/bluez/profiles/audio/a2dp.c style=codec-policy`
- `source=third/bluez/profiles/audio/transport.c style=media-transport`

Validator cleanup:

- Removed the old A2DP tool-local source-parity requirements for transport
  acquire and codec policy selection.
- Removed the old `label=avdtp-*` requirements that were only carried by the
  deleted source-parity printouts.
- Kept the upstream-facing compat owner and session graph requirements for
  AVDTP, MediaEndpoint, MediaTransport, L2CAP, codec, and cleanup semantics.

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-source-parity-ledger-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-source-parity-ledger-removed.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-source-parity-ledger-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-source-parity-ledger-removed-verify/run-results.json`

Boundary:

- This removes another duplicated A2DP tool-local evidence surface.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.

## 2026-06-17 A2DP tool local final summary removed

`blueza2dp closeout` no longer emits the final tool-local
`profile-final/endpoint-final/avdtp-final/signaling-final/media-final/
transport-final/codec-policy-final/cleanup-final` summary from
`tools/a2dp_main.c`.  Final closeout success is now validator-gated through the
upstream-facing compat owner path, especially
`bluez_upstream_a2dp_tool_closeout_owner_print()` with `final-ok=1`.

Validator cleanup:

- Removed the old A2DP tool-local `bluez-a2dp: profile-final=1 ...`
  requirement for both source and sink.
- Kept the upstream-facing compat requirements for e2e contract, ownership,
  coverage, and closeout owners.

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-local-final-summary-removed.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-local-final-summary-removed.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-local-final-summary-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-local-final-summary-removed-verify/run-results.json`

Boundary:

- This removes another duplicated A2DP tool-local evidence surface and leaves
  the compat owner as the closeout source of truth.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.

## 2026-06-17 A2DP tool local closeout ledger removed

`blueza2dp closeout` no longer emits the old tool-local
`closeout upstream-ownership-ledger`, `closeout upstream-coverage-map`, or
`closeout end-to-end-contract` text blocks from `tools/a2dp_main.c`.  The
closeout path now relies on the upstream-facing compat owners instead:

- `bluez_upstream_a2dp_tool_e2e_contract_owner_print()`
- `bluez_upstream_a2dp_tool_ownership_owner_print()`
- `bluez_upstream_a2dp_tool_coverage_owner_print()`
- `bluez_upstream_a2dp_tool_closeout_owner_print()`

The A2DP hwsim validator was tightened to stop requiring the deleted local
ledger fields:

- `end-to-end-final=1`
- `endpoint-owner=none media-transport-owner=none owner-watch=0 pending-request=0 message-ref=0`
- `refs endpoint=0 transport=0 setup=0 stream=0 registry=0`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-local-ledger-removed.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-local-ledger-removed.log`: PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-local-ledger-removed-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-local-ledger-removed-verify/run-results.json`

Boundary:

- This removes one duplicated A2DP tool-local evidence surface instead of
  merely adding another marker.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.

## 2026-06-17 A2DP tool end-to-end contract moved into upstream compat

`blueza2dp closeout` now emits the end-to-end A2DP contract from the
upstream-facing A2DP compatibility layer.  `blueza2dp` calls
`bluez_upstream_a2dp_tool_e2e_contract_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so profile/endpoint/transport/AVDTP/L2CAP/
codec/media/ordering/error/cleanup contract ownership is now validator-gated
as a callable compat owner instead of being represented only by
`tools/a2dp_main.c` text.

Validator-gated evidence:

- `bluez-a2dp: upstream-tool-e2e-contract-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-a2dp: upstream-tool-e2e-contract-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `ownership=tool-e2e-contract-to-upstream-object`
- `profile-owner=Profile1,A2DP,AudioSource,AudioSink`
- `endpoint-owner=MediaEndpoint1,SelectConfiguration,SetConfiguration,ClearConfiguration,Release`
- `transport-owner=MediaTransport1,Acquire,TryAcquire,Release,fd-handoff,owner-watch`
- `avdtp-owner=discover,get-all-capabilities,get-capabilities,set-configuration,get-configuration,open,reconfigure,delay-report,security-control,start,suspend,close,abort`
- `l2cap-owner=AVDTP-PSM-0x0019,media-cid-0x0041,signal-cid-0x0040`
- `contract=profile:1,endpoint:1,transport:1,avdtp:1,l2cap:1,codec:1,media:1,ordering:1,error:1,cleanup:1`
- `staged-boundary=blueza2dp-adapter-not-unmodified-bluetoothd final-ok=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-e2e-contract-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-e2e-contract-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-e2e-contract-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-e2e-contract-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP tool-local end-to-end contract glue by moving the contract
  surface into a callable upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  This is a
  verified reduction of the `blueza2dp` staged contract surface, not proof that
  unmodified upstream `bluetoothd` audio internals now run without the NuttX
  adapter.

## 2026-06-17 A2DP tool ownership ledger moved into upstream compat

`blueza2dp closeout` now emits the tool ownership ledger from the
upstream-facing A2DP compatibility layer.  `blueza2dp` calls
`bluez_upstream_a2dp_tool_ownership_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so the tool-side owner list for upstream A2DP,
AVDTP, media, transport, source/sink, L2CAP signal/media, pending request, and
cleanup is now validator-gated as a callable compat owner instead of being
represented only by `tools/a2dp_main.c` text.

Validator-gated evidence:

- `bluez-a2dp: upstream-tool-ownership-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-a2dp: upstream-tool-ownership-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `ownership=tool-ledger-to-upstream-object`
- `owner=profiles/audio/a2dp.c,avdtp.c,media.c,transport.c,source.c,sink.c`
- `objects=a2dp_server,a2dp_sep,avdtp_session,avdtp_stream,media_endpoint,media_transport,transport_owner_watch,pending_request,l2cap_signal,l2cap_media`
- `owners=profile:1,endpoint:1,avdtp:1,media:1,transport:1,codec:1,pending:1,l2cap-signal:1,l2cap-media:1,cleanup:1`
- `staged-boundary=blueza2dp-adapter-not-unmodified-bluetoothd final-ok=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-ownership-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-ownership-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-ownership-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-ownership-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP tool-local ownership ledger glue by moving the tool-side
  owner list and object ledger into a callable upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  This is a
  verified reduction of the `blueza2dp` staged ownership surface, not proof
  that unmodified upstream `bluetoothd` audio internals now run without the
  NuttX adapter.

## 2026-06-17 A2DP tool coverage owner moved into upstream compat

`blueza2dp closeout` now emits tool coverage/ownership map evidence from the
upstream-facing A2DP compatibility layer.  `blueza2dp` calls
`bluez_upstream_a2dp_tool_coverage_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so the tool-side profile, endpoint, AVDTP
session/stream, MediaEndpoint, MediaTransport, codec policy, L2CAP channel, and
cleanup coverage map is now validator-gated as a callable compat owner instead
of being represented only by `tools/a2dp_main.c` text.

Validator-gated evidence:

- `bluez-a2dp: upstream-tool-coverage-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-a2dp: upstream-tool-coverage-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `ownership=tool-coverage-to-upstream-object`
- `objects=profile,endpoint,avdtp-session,avdtp-stream,media-endpoint,media-transport,codec-policy,l2cap-channel`
- `coverage=profile:1,endpoint:1,avdtp:1,signaling:1,media:1,transport:1,codec-policy:1,l2cap:1,cleanup:1`
- `staged-boundary=blueza2dp-adapter-not-unmodified-bluetoothd final-ok=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-coverage-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-coverage-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-coverage-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-coverage-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP tool-local coverage glue by moving the tool-side coverage
  map into a callable upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  This is a
  verified reduction of the `blueza2dp` staged coverage surface, not proof that
  unmodified upstream `bluetoothd` audio internals now run without the NuttX
  adapter.

## 2026-06-17 A2DP tool closeout owner moved into upstream compat

`blueza2dp closeout` now emits tool closeout ownership evidence from the
upstream-facing A2DP compatibility layer.  `blueza2dp` calls
`bluez_upstream_a2dp_tool_closeout_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so the tool-side profile, endpoint, AVDTP,
signaling, media, transport, codec policy, L2CAP signal/media, pending request,
owner watch, and cleanup ledger is now validator-gated as a callable compat
owner instead of being represented only by `tools/a2dp_main.c` final text.

Validator-gated evidence:

- `bluez-a2dp: upstream-tool-closeout-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-a2dp: upstream-tool-closeout-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `ownership=tool-closeout-to-upstream-object`
- `objects=profile,endpoint,avdtp,signaling,media,transport,codec-policy,l2cap-signal,l2cap-media,pending-request,owner-watch`
- `lifecycle=profile-register,endpoint-register,avdtp-open,signaling-run,media-open,transport-acquire,codec-policy,transport-release,media-close,avdtp-close,endpoint-clear,profile-unregister,cleanup`
- `final profile=0 endpoint=0 avdtp=0 signaling=0 media=0 transport=0 codec-policy=0 l2cap-signal=0 l2cap-media=0 pending=0 owner-watch=0 cleanup=1`
- `staged-boundary=blueza2dp-adapter-not-unmodified-bluetoothd final-ok=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-tool-closeout-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-tool-closeout-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-tool-closeout-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-tool-closeout-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP tool-local closeout glue by moving the final tool-side
  object/cleanup ledger into a callable upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  This is a
  verified reduction of the `blueza2dp` staged closeout surface, not proof that
  unmodified upstream `bluetoothd` audio internals now run without the NuttX
  adapter.

## 2026-06-17 A2DP coverage map moved into upstream compat

A2DP daemon closeout now emits upstream coverage-map ownership evidence from
the upstream-facing A2DP compatibility layer.  `bluezdaemon` calls
`bluez_upstream_a2dp_coverage_map_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so the profile/device/SDP/D-Bus/mainloop/
AVDTP/AVRCP/MediaTransport/codec/L2CAP/error-policy coverage map is now
validator-gated as a callable compat owner instead of being represented only by
daemon-local closeout text.

Validator-gated evidence:

- `bluez-daemon: a2dp upstream-coverage-map-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-daemon: a2dp upstream-coverage-map-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `ownership=coverage-map-to-upstream-object`
- `executed=plugin,profile,device,sdp,dbus,mainloop,avdtp,avrcp,media-transport,codec,l2cap,error-policy`
- `rounds=2 profile-final=1 dbus-final=1 mainloop-final=1 transaction-final=1 media-final=1 codec-final=1 transport-final=1 avrcp-final=1 l2cap-final=1 state-final=1 cleanup-final=1`
- `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd final-ok=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-coverage-map-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-coverage-map-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-coverage-map-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-coverage-map-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP daemon-local coverage-map glue by moving the executed
  upstream surface map and final ownership flags into a callable
  upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  This is a
  verified reduction of the staged coverage surface, not proof that unmodified
  upstream `bluetoothd` audio internals now run without the NuttX adapter.

## 2026-06-17 A2DP daemon ownership ledger moved into upstream compat

A2DP daemon closeout now emits daemon ownership ledger evidence from the
upstream-facing A2DP compatibility layer.  `bluezdaemon` calls
`bluez_upstream_a2dp_daemon_ownership_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so the bluetoothd ownership ledger for profile,
device, session, stream, MediaTransport, AVRCP player, L2CAP fd, D-Bus name,
mainloop watch/timer, AVDTP transactions, fd handoff, zero-ref rounds, and
final cleanup is now validator-gated as a callable compat owner instead of
being represented only by daemon-local closeout text.

Validator-gated evidence:

- `bluez-daemon: a2dp upstream-daemon-ownership-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-daemon: a2dp upstream-daemon-ownership-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `ownership=daemon-ledger-to-upstream-object`
- `owner=bluetoothd direct-owner=profile,device,session,stream,media-transport,avrcp-player,l2cap-fd,dbus-name,mainloop-watch`
- `profile-register=1 profile-unregister=1 device-connect=2 device-disconnect=2`
- `dbus-name-acquire=1 dbus-name-release=1 dbus-owner-lost=1 dbus-owner-reacquire=1`
- `mainloop-watch-add=7 mainloop-watch-remove=7 mainloop-timer-add=2 mainloop-timer-remove=2`
- `avdtp-transactions=12 avdtp-complete=12 transport-acquire=2 transport-release=2 fd-open=2 fd-close=2 zero-ref-rounds=2 rounds=2`
- `final-profile-registered=0 final-device-ref=0 final-session-ref=0 final-stream-ref=0 final-sep-ref=0`
- `final-endpoint-refs=0 final-transport-refs=0 final-player-refs=0 final-dbus-owners=0 final-interfaces=0`
- `final-mainloop-watches=0 final-mainloop-timers=0 final-l2cap-fds=0 final-media-fd=closed final-transaction-pending=0 final-state-errors=0`
- `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd final-ok=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-daemon-ledger-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-daemon-ledger-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-daemon-ledger-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-daemon-ledger-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP daemon-local ownership ledger glue by moving the bluetoothd
  direct-owner ledger and final-zero accounting into a callable
  upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  This is a
  verified reduction of the staged ledger surface, not proof that unmodified
  upstream `bluetoothd` audio internals now run without the NuttX adapter.

## 2026-06-17 A2DP source parity owner moved into upstream compat

A2DP daemon closeout now emits source-parity ownership evidence from the
upstream-facing A2DP compatibility layer.  `bluezdaemon` calls
`bluez_upstream_a2dp_source_parity_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so the direct-upstream source/object/handler
parity record is now validator-gated as a callable compat owner instead of
being represented only by daemon-local closeout text.

Validator-gated evidence:

- `bluez-daemon: a2dp upstream-source-parity-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-daemon: a2dp upstream-source-parity-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `direct-upstream=profile.c,device.c,adapter.c,dbus-common.c,mainloop.c,io-mainloop.c,a2dp.c,avdtp.c,media.c,transport.c,avrcp.c,sbc.c,l2cap_sock.c`
- `objects=adapter,profile,device,session,setup,stream,sep,media-endpoint,media-transport,avrcp-player,l2cap-fd,dbus-name,mainloop-watch`
- `handlers=profile_connect,profile_disconnect,media_endpoint_set_configuration,media_endpoint_clear_configuration,transport_acquire,transport_try_acquire,transport_release,avdtp_discover,avdtp_set_configuration,avdtp_open,avdtp_start,avdtp_suspend,avdtp_close,avrcp_control,avrcp_browsing`
- `ownership=source-parity-to-upstream-object`
- `rounds=2 profile-final=1 dbus-final=1 mainloop-final=1 transaction-final=1 media-final=1 transport-final=1 avrcp-final=1 l2cap-final=1 state-final=1 cleanup-final=1`
- `adapter-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-source-parity-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-source-parity-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-source-parity-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-source-parity-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP daemon-local source parity glue by moving direct-upstream
  source/object/handler parity evidence into a callable upstream-facing compat
  helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  This is
  another verified glue-reduction step, not proof that unmodified upstream
  `bluetoothd` audio internals now run without the NuttX staged adapter.

## 2026-06-17 A2DP adapter command owner moved into upstream compat

A2DP daemon closeout now delegates adapter-bound command sequencing ownership
to the upstream-facing A2DP compatibility layer.  `bluezdaemon` calls
`bluez_upstream_a2dp_adapter_command_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so plugin init, adapter/profile probe,
profile connect, device resolve, service discovery, media endpoint register,
AVDTP bind/discover/config/open/start, MediaTransport export/acquire/release,
AVDTP suspend/close, profile disconnect, endpoint unregister, adapter remove,
plugin exit, and cleanup are no longer represented only by daemon-local
command counters.

Validator-gated evidence:

- `bluez-daemon: a2dp upstream-adapter-command-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-daemon: a2dp upstream-adapter-command-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `entrypoints=plugin_init,adapter_probe,profile_probe,profile_connect,device_resolve,service_discovery,media_endpoint_register,avdtp_bind,avdtp_discover,avdtp_set_configuration,avdtp_open,avdtp_start,transport_export,transport_acquire,transport_release,avdtp_suspend,avdtp_close,profile_disconnect,media_endpoint_unregister,adapter_remove,plugin_exit`
- `ownership=adapter-command-to-upstream-object`
- `connect-path=1,1,1,1,1,1,1,1,1,1,1,1,1,1`
- `disconnect-path=1,1,1,1,1 cleanup-path=1,1,1 command-errors=0 final-ok=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-adapter-command-compat-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-adapter-command-compat-owner-v2.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-adapter-command-compat-owner-v2-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-adapter-command-compat-owner-v2-verify/run-results.json`

Boundary:

- This reduces A2DP daemon-local adapter command glue by moving command
  sequencing, upstream object entrypoint ordering, disconnect, cleanup, and
  error accounting evidence into a callable upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  Remaining
  A2DP convergence work should keep replacing staged adapter command handling
  with direct imported upstream `profile.c` / `device.c` / `adapter.c` /
  `media.c` / `transport.c` / `avdtp.c` object ownership.

## 2026-06-17 A2DP profile/mainloop/D-Bus owner moved into upstream compat

A2DP daemon closeout now delegates profile, device, D-Bus object, and mainloop
transition ownership to the upstream-facing A2DP compatibility layer.
`bluezdaemon` calls `bluez_upstream_a2dp_profile_mainloop_dbus_owner_print()`
from `bluez/upstream_a2dp_compat.c`, so plugin init/exit, adapter probe,
profile register/unregister, device connect/disconnect, D-Bus name/interface
ownership, mainloop watches/timers, dispatch, owner-lost/reacquire, and cleanup
ownership is no longer represented only by daemon-local counters.

Validator-gated evidence:

- `bluez-daemon: a2dp upstream-profile-mainloop-dbus-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-daemon: a2dp upstream-profile-mainloop-dbus-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `objects=plugin,adapter,profile,device,dbus_name,dbus_interfaces,mainloop_watch,mainloop_timer,media_adapter,media_endpoint,media_transport,avrcp_player`
- `lifecycle=plugin-init,adapter-probe,profile-register,device-connect,name-acquire,interfaces-added,watch-add,timer-add,dispatch,owner-lost,owner-reacquire,interfaces-removed,watch-remove,timer-remove,name-release,device-disconnect,profile-unregister,plugin-exit`
- `profile-register=1 profile-unregister=1 connect=2 disconnect=2 name-acquire=1 name-release=1 interfaces-added=3 interfaces-removed=5 owner-lost=1 owner-reacquire=1 watch-add=7 watch-remove=7 timer-add=2 timer-remove=2`
- `dispatch=mgmt:1,l2cap:1,avdtp:1,avctp:1,media:1,dbus:1 cleanup=1`
- `final plugin=0 adapter=0 profile=0 device=0 dbus-name=0 interfaces=0 watch=0 timer=0 media-adapter=0 endpoint=0 transport=0 player=0 final-zero=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-profile-mainloop-dbus-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-profile-mainloop-dbus-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-profile-mainloop-dbus-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-profile-mainloop-dbus-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP daemon-local profile/mainloop/D-Bus glue by moving
  profile, device, D-Bus, mainloop, owner recovery, and cleanup evidence into
  a callable upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  Remaining
  A2DP work should continue replacing adapter-bound command sequencing with
  callable upstream `profile.c` / `device.c` / `adapter.c` / `media.c`
  object entry points.

## 2026-06-17 A2DP media transport D-Bus owner moved into upstream compat

A2DP daemon closeout now delegates MediaTransport D-Bus lifecycle ownership to
the upstream-facing A2DP compatibility layer.  `bluezdaemon` calls
`bluez_upstream_a2dp_media_transport_dbus_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so Acquire/TryAcquire/Release, Select/Unselect,
property access, owner watch, fd handoff, registry export/unexport, and cleanup
ownership is no longer represented only by daemon-local counters.

Validator-gated evidence:

- `bluez-daemon: a2dp upstream-media-transport-dbus-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-daemon: a2dp upstream-media-transport-dbus-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `objects=media_transport,avdtp_stream,transport_owner,owner_watch,pending_request,dbus_message,media_fd`
- `methods=Acquire,TryAcquire,Release,Select,Unselect`
- `properties=Device,UUID,Codec,Configuration,State,Delay,Volume,Endpoint`
- `lifecycle=export,registry-append,owner-watch-add,acquire,fd-handoff,try-acquire-busy,release,owner-watch-remove,registry-remove,unexport,cleanup`
- `acquire=1 try-acquire=1 release=1 select=1 unselect=1 property-reads=8 property-writes=2 errors=4 fd-handoff=1`
- `final transport=0 stream=0 owner=0 watch=0 pending=0 message=0 fd=0 exported=0 registered=0 cleanup=1 final-zero=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-media-transport-dbus-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-media-transport-dbus-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-media-transport-dbus-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-media-transport-dbus-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP daemon-local media transport/D-Bus glue by moving transport
  method, owner-watch, fd-handoff, property, and cleanup evidence into a
  callable upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  Remaining
  A2DP work should continue replacing daemon-owned profile/mainloop/D-Bus
  transition ownership with callable upstream `profile.c` / `device.c` /
  `media.c` / `transport.c` style objects.

## 2026-06-17 A2DP AVDTP transaction owner moved into upstream compat

A2DP daemon closeout now delegates AVDTP transaction/session ownership evidence
to the upstream-facing A2DP compatibility layer.  `bluezdaemon` calls
`bluez_upstream_a2dp_avdtp_transaction_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so request queueing, completion, timeout,
retry, cancel, callback, and cleanup ownership is no longer represented only
by daemon-local round counters.

Validator-gated evidence:

- `bluez-daemon: a2dp upstream-avdtp-transaction-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-daemon: a2dp upstream-avdtp-transaction-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `objects=avdtp_session,avdtp_request,avdtp_stream,a2dp_setup,pending_open,transaction_timer,stream_callback`
- `commands=discover,get-all-capabilities,set-configuration,get-configuration,open,reconfigure,delay-report,security-control,start,suspend,close,abort`
- `lifecycle=enqueue,send,complete,callback,timeout,retry,cancel,cleanup`
- `requests=12 responses=12 completed=12 pending=0 timers=0 callbacks=0 retries=1 cancels=1 timeouts=1 cleanup=1`
- `final session=0 request=0 stream=0 setup=0 pending-open=0 timer=0 callback=0 final-zero=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-avdtp-transaction-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-avdtp-transaction-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-avdtp-transaction-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-avdtp-transaction-compat-owner-verify/run-results.json`

Boundary:

- This reduces A2DP daemon-local transaction glue by moving AVDTP request,
  timer, callback, retry/cancel, and cleanup ownership into a callable
  upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  Remaining
  A2DP work should continue replacing daemon-owned transition and D-Bus/media
  transport lifecycle with callable upstream `a2dp.c` / `avdtp.c` /
  `transport.c` style objects.

## 2026-06-17 A2DP setup/stream/SEP owner moved into upstream compat

A2DP daemon closeout now delegates setup/session/stream/SEP ownership evidence
to the upstream-facing A2DP compatibility layer.  `bluezdaemon` calls
`bluez_upstream_a2dp_setup_stream_owner_print()` from
`bluez/upstream_a2dp_compat.c`, so the setup, session, local/remote SEP,
stream, media endpoint, media transport, owner watch, and pending request
lifecycle is no longer only a daemon-local ledger.

Validator-gated evidence:

- `bluez-daemon: a2dp upstream-setup-stream-owner role=source compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-daemon: a2dp upstream-setup-stream-owner role=sink compile-unit=bluez/upstream_a2dp_compat.c`
- `objects=a2dp_server,avdtp_session,a2dp_setup,local_sep,remote_sep,avdtp_stream,media_endpoint,media_transport,transport_owner_watch,pending_request`
- `ownership=setup-ref,session-ref,stream-ref,sep-ref,transport-ref,owner-watch,pending-request`
- `final server=0 session=0 setup=0 local-sep=0 remote-sep=0 stream=0 endpoint=0 transport=0 owner-watch=0 pending=0 setup-refs=0 session-refs=0 stream-refs=0 sep-refs=0 cleanup=1 transitions=3 final-zero=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-setup-stream-compat-owner.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-setup-stream-compat-owner.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-setup-stream-compat-owner-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-setup-stream-compat-owner-verify/run-results.json`

Boundary:

- This is another A2DP glue-reduction step: daemon-local setup/stream/SEP
  ledger evidence moved into a callable upstream-facing compat helper.
- The explicit `bluezdaemon-adapter-not-unmodified-bluetoothd` and
  `blueza2dp-adapter-not-unmodified-bluetoothd` boundaries remain.  More
  daemon-owned transitions still need to move into callable upstream
  `a2dp.c` / `avdtp.c` style objects before the A2DP glue is gone.

## 2026-06-17 A2DP closeout session owner moved into upstream compat

The `blueza2dp` closeout session graph has been moved out of
`tools/a2dp_main.c` and into the upstream-facing A2DP compatibility layer.
`blueza2dp` now calls `bluez_upstream_a2dp_closeout_session_*()` helpers from
`bluez/upstream_a2dp_compat.c` for the A2DP closeout session lifecycle instead
of owning the object graph implementation directly in the tool.

Validator-gated evidence:

- `bluez-a2dp: upstream-session-graph action=closeout-init compile-unit=bluez/upstream_a2dp_compat.c`
- `bluez-a2dp: upstream-session-graph action=closeout-cleanup compile-unit=bluez/upstream_a2dp_compat.c`
- `owners=profile:0,endpoint:0,avdtp:0,transport:0,media-fd:0,codec:0,pending:0`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-closeout-session-compat-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-closeout-session-compat-owner-v2.log`:
  PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-closeout-session-compat-owner-v2-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-closeout-session-compat-owner-v2-verify/run-results.json`

Boundary:

- This is a concrete A2DP glue-reduction step: closeout session graph ownership
  moved from a tool-local harness object into an upstream-facing compat unit.
- The explicit `blueza2dp-adapter-not-unmodified-bluetoothd` and
  `bluezdaemon-adapter-not-unmodified-bluetoothd` boundaries remain.  The next
  A2DP reduction should replace more daemon/tool-owned transitions with
  callable upstream `a2dp.c` / `avdtp.c` setup and stream objects.

## 2026-06-17 BlueZ A2DP upstream session object convergence verification

`blueza2dp closeout` now carries its closeout path through an explicit
upstream-shaped `bluez_a2dp_session` object.  The object records profile,
endpoint, AVDTP, media transport, media fd, codec, and pending-request
ownership, then prints validator-gated object graph evidence for:

- `profiles/audio/a2dp.c`
- `profiles/audio/avdtp.c`
- `profiles/audio/media.c`
- `profiles/audio/transport.c`
- `profiles/audio/source.c`
- `profiles/audio/sink.c`

Validator-gated evidence:

- `bluez-a2dp: upstream-session-graph action=closeout-init`
- `bluez-a2dp: upstream-session-graph action=endpoint-registered`
- `bluez-a2dp: upstream-session-graph action=signaling-open`
- `bluez-a2dp: upstream-session-graph action=avdtp-streaming`
- `bluez-a2dp: upstream-session-graph action=transport-acquired`
- `bluez-a2dp: upstream-session-graph action=closeout-cleanup`
- `objects=a2dp_server,a2dp_sep,avdtp_session,avdtp_stream,media_endpoint,media_transport,transport_owner_watch,pending_request,l2cap_signal,l2cap_media,codec_sbc`
- `dbus=org.bluez.MediaEndpoint1,org.bluez.MediaTransport1`
- `methods=RegisterEndpoint,SelectConfiguration,SetConfiguration,ClearConfiguration,Acquire,TryAcquire,Release`
- `owners=profile:0,endpoint:0,avdtp:0,transport:0,media-fd:0,codec:0,pending:0`

Build:

- `FeatherCore/build/logs/build-bt1-bluez-a2dp-session-object.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bluez-a2dp-session-object.log`: PASS.

hwsim:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-bluez-a2dp-session-object-verify.log`
- `FeatherCore/build/bt-hwsim-bluez-a2dp-session-object-verify/run-results.json`

Boundary:

- This reduces A2DP harness glue by moving `blueza2dp` closeout evidence from
  a broad ledger into a shared upstream-shaped session object and lifecycle.
- The explicit `blueza2dp-adapter-not-unmodified-bluetoothd` boundary remains.
  A2DP is still not claiming a fully unmodified upstream `bluetoothd` audio
  plugin in NuttX.

## 2026-06-17 BlueZ mgmt controller object convergence verification

BlueZ mgmt bootstrap now has an explicit upstream-shaped controller object in
`bluezmgmt`.  The daemon-bootstrap path no longer treats mgmt bootstrap as only
a list of send/recv probes; it carries mgmt fd, adapter, device, pending
command, discovery, and security ownership through a shared
`bluez_mgmt_controller` object.

Validator-gated evidence:

- `bluez-mgmt: upstream-controller action=bootstrap-init`
- `bluez-mgmt: upstream-controller action=bootstrap-open`
- `bluez-mgmt: upstream-controller action=read-controller-info`
- `bluez-mgmt: upstream-controller action=adapter-policy`
- `bluez-mgmt: upstream-controller action=discovery`
- `bluez-mgmt: upstream-controller action=pair-connect`
- `bluez-mgmt: upstream-controller action=conn-info-disconnect`
- `bluez-mgmt: upstream-controller action=unpair-cleanup`
- `bluez-mgmt: upstream-controller action=error-policy`
- `bluez-mgmt: upstream-controller action=bootstrap-close`
- `objects=mgmt,mgmt_request,mgmt_reply,adapter,device,pending_cmd,discovery_session,pair_session,bonding_data`
- `socket=AF_BLUETOOTH/BTPROTO_HCI/HCI_CHANNEL_CONTROL`
- `state=closed`
- `owners=mgmt-fd:0,adapter:1,device:1,pending:0,discovery:0,security:0`

The mgmt status receive helper now loops over async mgmt events until the
expected command complete/status is observed.  This matches the BlueZ
shared/mgmt mainloop behavior more closely and fixed a real ordering issue
where `DEVICE_UNPAIRED` could arrive before the expected invalid-params
`CMD_STATUS`.

Build:

- `FeatherCore/build/logs/build-ble1-bluez-mgmt-controller-object-v3.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-bluez-mgmt-controller-object-v3.log`:
  PASS.

hwsim:

- `bluez-mgmt-daemon-bootstrap`: PASS for `ble1`.
- `bluez-hci-mgmt-socket-closeout-full`: PASS for `ble1` and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-bluez-mgmt-controller-object-v3-verify.log`
- `FeatherCore/build/bt-hwsim-bluez-mgmt-controller-object-v3-verify/run-results.json`

Boundary:

- This reduces HCI/mgmt glue by adding a shared upstream-shaped controller
  owner and a more realistic async event filtering receive loop.
- The explicit `bluezmgmt-daemon-bootstrap-adapter-not-unmodified-bluetoothd`
  boundary remains.  The next mgmt convergence step should replace more
  harness command sequencing with callable upstream `src/shared/mgmt.c` /
  `src/adapter.c` routines.

## 2026-06-17 BlueZ Network object graph convergence verification

BlueZ Network Profile convergence now has an explicit upstream-shaped session
and object graph in `blueznetwork`.  The profile adapter no longer keeps
service, role, fd, BNEP control, device, and state ownership as disconnected
helper-local values only; connect/disconnect/closeout now flow through a
`bluez_network_session` object that mirrors the upstream Network plugin
ownership split:

- `profiles/network/manager.c`
- `profiles/network/server.c`
- `profiles/network/connection.c`
- `profiles/network/bnep.c`

Validator-gated evidence:

- `bluez-network: upstream-object-graph action=connect-begin`
- `bluez-network: upstream-object-graph action=connect-complete`
- `bluez-network: upstream-object-graph action=disconnect-complete`
- `objects=network_manager,network_server,network_peer,network_conn,network_session,bnep_control,l2cap_io,netdev_bridge`
- `fd-flow=connect_l2cap_fd,BNEPCONNADD,BNEPCONNDEL`
- `session-state=connected`
- `session-state=closed`

Build:

- `FeatherCore/build/logs/build-bt1-bluez-network-object-graph.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bluez-network-object-graph.log`: PASS.

hwsim:

- `bluez-network-ping`: PASS for `bt1` and `bt2`.
- `bluez-network-reconnect-stress`: PASS for `bt1` and `bt2`.
- `bluez-network-iperf-matrix`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-bluez-network-object-graph-verify.log`
- `FeatherCore/build/bt-hwsim-bluez-network-object-graph-verify/run-results.json`

Boundary:

- This reduces BlueZ Network profile glue by introducing a shared
  upstream-shaped object/session owner for Network1, NetworkServer1,
  connected L2CAP fd handoff, BNEP control, and profile lifecycle.
- The explicit `blueznetwork-adapter-not-unmodified-bluetoothd` boundary
  remains.  The next Network convergence step should replace more of the
  adapter command flow with callable upstream `connection.c` / `server.c`
  routines rather than only a structured harness object.

## 2026-06-17 BNEP session-thread TX ownership verification

BT Network/BNEP has moved one step closer to the imported Linux BNEP runtime:
the sim-only immediate TX drain in imported `bnep/netdev.c` was removed.  TX
now follows the Linux-shaped path:

- NuttX IP lower-half send
- Linux `net_device` `ndo_start_xmit`
- BNEP `sk_write_queue`
- imported BNEP `bnep_session` thread
- `bnep_tx_frame`
- L2CAP send / hwsim BNEP medium
- peer `bnep_rx_frame`
- `netif_rx`
- NuttX IP RX

Validator-gated strengthening:

- `bnep-native-session-tx-dequeue` must now be non-zero in native datapath,
  reconnect, reconnect-stress, and Network closeout checks.
- This prevents a pass where `ndo_start_xmit` happened but the imported BNEP
  session thread did not own TX queue draining.

Build:

- `FeatherCore/build/logs/build-bt1-bnep-session-thread-tx-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bnep-session-thread-tx-owner.log`: PASS.

hwsim:

- `bluez-bneptest-fd-handoff`: PASS.
- `bluez-bneptest-ping`: PASS for `bt1` and `bt2`.
- `bluez-bneptest-reconnect-stress`: PASS for `bt1` and `bt2`.
- `bluez-network-ping`: PASS for `bt1` and `bt2`.
- `bluez-network-reconnect-stress`: PASS for `bt1` and `bt2`.
- `bluez-network-iperf-matrix`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-bnep-session-thread-tx-owner-verify.log`
- `FeatherCore/build/bt-hwsim-bnep-session-thread-tx-owner-verify/run-results.json`

Boundary:

- This reduces BNEP datapath glue by removing the hwsim-only direct TX drain
  from imported `bnep/netdev.c`.
- The remaining BlueZ Network boundary is still the staged
  `blueznetwork-adapter-not-unmodified-bluetoothd` adapter; next NET
  convergence should move BlueZ Network plugin object/D-Bus/session ownership
  closer to unmodified upstream `bluetoothd`.

## 2026-06-17 HCI/mgmt/socket ABI upstream-source parity verification

HCI/mgmt/socket closeout now has explicit source-parity evidence for the
BLE hwsim roles.  The gate separates imported BlueZ `hcitool`/`lib/hci`
coverage, BlueZ monitor coverage, Linux HCI socket/core/event/mgmt coverage,
native HCI channel ownership, socket filter/fanout ownership, command/event
ordering, mgmt lifecycle/error behavior, and the remaining staged adapter
boundary.

New validator-gated evidence:

- `bluez-hciraw: socket-abi upstream-source-parity`
- Direct upstream: `tools/hcitool.c`, `lib/hci.c`, `monitor/bt.h`,
  `hci_sock.c`, `hci_core.c`, `hci_event.c`, and `mgmt.c`.
- Objects: raw/user/monitor/control/logging sockets, HCI filter, monitor
  fanout, skb, command, event, pending command, and pending event.
- Handlers: socket creation, raw/user/monitor/control/logging bind, filter
  setsockopt/getsockopt, sendmsg, recvmsg, monitor delivery, command complete,
  command status, and close/release.
- Native channels: raw, user, monitor, control, and logging.
- Native events: command complete, command status, monitor fanout,
  advertising enable, scan report, and ISO setup.
- `adapter-boundary=bluezhciraw-hci-socket-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-ble1-hci-mgmt-socket-upstream-parity.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-hci-mgmt-socket-upstream-parity.log`:
  PASS.

hwsim:

- `bluez-hci-mgmt-socket-closeout-full`: PASS for `ble1` and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-hci-mgmt-socket-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-hci-mgmt-socket-upstream-parity-verify/run-results.json`

Boundary:

- HCI/mgmt/socket ABI is closed for the current hwsim functional target and
  now has explicit upstream-source parity evidence across HCI raw/user,
  monitor/control/logging channels, mgmt pairing/error lifecycle,
  advertise/scan behavior, socket filter/fanout, command/event ordering, and
  cleanup lifecycle.
- This still does not claim unmodified upstream `bluetoothd` ownership inside
  NuttX; the remaining `bluezhciraw` adapter boundary is explicit and
  validator-gated.

## 2026-06-17 iAP upstream-source parity verification

iAP convergence now has explicit source-parity evidence for controller and
accessory roles.  The gate separates imported BlueZ iAP profile ownership,
classic SDP/SPP ownership, RFCOMM socket/TTY ownership, Linux L2CAP ownership,
iAP identify/accessory-info ownership, External Accessory session ownership,
control-payload/link-control ownership, and the remaining staged adapter
boundary.

New validator-gated evidence:

- `bluez-daemon: iap closeout upstream-source-parity role=controller`
- `bluez-daemon: iap closeout upstream-source-parity role=accessory`
- Direct upstream: `profiles/iap/main.c`, `rfcomm/core.c`,
  `rfcomm/sock.c`, `rfcomm/tty.c`, `l2cap_core.c`, and `l2cap_sock.c`.
- Objects: device, profile, SerialPort, SDP record, RFCOMM fd/TTY, iAP
  session, identify request, accessory info, External Accessory session,
  EA protocol, control payload, credit window, retransmit timer, and mainloop
  watch.
- Native RFCOMM boundary: PSM `0x0003`, channel `14`, fd handoff, TTY, and
  L2CAP session ownership.
- Native iAP boundary: identify-device, accessory-info, session ID, protocol
  list, External Accessory session, control TX/RX, ACK, credit, keepalive,
  retransmit, and error recovery.
- `adapter-boundary=bluezdaemon-iap-adapter-not-unmodified-iapd parity-final=1`

Build:

- `FeatherCore/build/logs/build-bt1-iap-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-iap-upstream-parity.log`: PASS.

hwsim:

- `bluez-iap-profile-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-iap-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-iap-upstream-parity-verify/run-results.json`

Boundary:

- iAP remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across iAP/SPP, SDP, RFCOMM,
  L2CAP, identify/accessory-info, External Accessory session, control payload,
  link-control, error handling, and cleanup lifecycle.
- This still does not claim unmodified upstream `bluetoothd`/iAP adapter
  ownership inside NuttX; the remaining adapter boundary is explicit and
  validator-gated.

## 2026-06-17 Print upstream-source parity verification

Print convergence now has explicit source-parity evidence for client and
printer roles.  The gate separates imported BlueZ CUPS/HCRP/SPP ownership,
classic SDP ownership, RFCOMM socket/TTY ownership, Linux L2CAP ownership,
CUPS backend/job ownership, printer status/cancel/error ownership, and the
remaining staged adapter boundary.

New validator-gated evidence:

- `bluez-daemon: print closeout upstream-source-parity role=client`
- `bluez-daemon: print closeout upstream-source-parity role=printer`
- Direct upstream: `profiles/cups/main.c`, `profiles/cups/sdp.c`,
  `profiles/cups/spp.c`, `profiles/cups/hcrp.c`, `profiles/cups/cups.h`,
  `rfcomm/core.c`, `rfcomm/sock.c`, `rfcomm/tty.c`, `l2cap_core.c`, and
  `l2cap_sock.c`.
- Objects: device, profile, SerialPort, SDP record, RFCOMM fd/TTY,
  HCRP control/data channels, CUPS backend, printer, print job, status query,
  cancel request, and mainloop watch.
- Native RFCOMM boundary: PSM `0x0003`, channel `13`, fd handoff, TTY, and
  L2CAP session ownership.
- Native print boundary: HCRP control/data, credit, MTU, printer language,
  job submit/receive/status/cancel, and error recovery.
- `adapter-boundary=bluezdaemon-print-adapter-not-unmodified-cups-backend parity-final=1`

Build:

- `FeatherCore/build/logs/build-bt1-print-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-print-upstream-parity.log`: PASS.

hwsim:

- `bluez-print-profile-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-print-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-print-upstream-parity-verify/run-results.json`

Boundary:

- Print remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across CUPS/HCRP/SPP, SDP, RFCOMM,
  L2CAP, job lifecycle, status/cancel/error handling, and cleanup lifecycle.
- This still does not claim unmodified upstream `bluetoothd`/CUPS backend
  adapter ownership inside NuttX; the remaining adapter boundary is explicit
  and validator-gated.

## 2026-06-17 Ranging upstream-source parity verification

Ranging convergence now has explicit source-parity evidence for initiator and
reflector roles.  The gate separates imported BlueZ RAP/RAP-HCI ownership,
adapter/device ownership, ATT/GATT ownership, Linux HCI/mgmt/L2CAP/SMP
ownership, Channel Sounding procedure ownership, result/quality ownership,
event/error handling, and the remaining staged adapter boundary.

New validator-gated evidence:

- `bluez-daemon: ranging closeout upstream-source-parity role=initiator`
- `bluez-daemon: ranging closeout upstream-source-parity role=reflector`
- Direct upstream: `profiles/ranging/rap.c`,
  `profiles/ranging/rap_hci.c`, `src/adapter.c`, `src/device.c`,
  `src/shared/att.c`, `src/shared/gatt-client.c`, `hci_core.c`,
  `hci_event.c`, `mgmt.c`, `l2cap_core.c`, and `smp.c`.
- Objects: adapter, device, ranging profile, RAP session, capability cache,
  security state, procedure config/request, HCI request, result event, quality
  window, ATT bearer/fd, request queue, notify session, and mainloop watch.
- Native ATT boundary: fixed ATT CID `0x0004`, capability read, security
  enable, procedure config, result notify, CCC, MTU, and security.
- Native HCI boundary: LE Channel Sounding capability, procedure parameter,
  procedure start/enable/request, and result events.
- Native ranging boundary: distance, quality, RSSI, RTT, phase slope, samples,
  poor-quality drop, and error recovery.
- `adapter-boundary=bluezdaemon-ranging-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-ble1-ranging-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-ranging-upstream-parity.log`: PASS.

hwsim:

- `bluez-ranging-profile-closeout`: PASS for `ble1` and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-ranging-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-ranging-upstream-parity-verify/run-results.json`

Boundary:

- Ranging remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across RAP/RAP-HCI, ATT/GATT,
  HCI Channel Sounding, procedure lifecycle, result/quality handling, and
  cleanup lifecycle.
- This still does not claim unmodified upstream `bluetoothd` Ranging adapter
  ownership inside NuttX; the remaining adapter boundary is explicit and
  validator-gated.

## 2026-06-17 MIDI upstream-source parity verification

MIDI convergence now has explicit source-parity evidence for controller and
peripheral roles.  The gate separates imported BlueZ MIDI profile ownership,
MIDI timestamp parser/encoder ownership, shared ATT/GATT ownership, notify and
write-without-response ownership, Linux HCI/mgmt/L2CAP/SMP ownership, jitter
and error policy, and the remaining staged adapter boundary.

New validator-gated evidence:

- `bluez-daemon: midi closeout upstream-source-parity role=controller`
- `bluez-daemon: midi closeout upstream-source-parity role=peripheral`
- Direct upstream: `profiles/midi/midi.c`, `profiles/midi/libmidi.c`,
  `src/gatt-client.c`, `src/gatt-database.c`, `src/shared/att.c`,
  `src/shared/gatt-client.c`, `src/shared/gatt-db.c`,
  `src/shared/gatt-server.c`, `hci_core.c`, `hci_event.c`, `mgmt.c`,
  `l2cap_core.c`, `l2cap_sock.c`, and `smp.c`.
- Objects: GATT manager, service, characteristic, MIDI service,
  MIDI characteristic, MIDI parser, timestamp queue, notify session,
  write command, ATT bearer/fd, request queue, CCC, and mainloop watch.
- Native ATT boundary: fixed ATT CID `0x0004`, MTU `247`, service and
  characteristic discovery, CCC enable, write-without-response, notify, and
  security.
- Native MIDI boundary: note-on, note-off, control-change, timestamp wrap,
  jitter window, ordering, and error recovery.
- `adapter-boundary=bluezdaemon-midi-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-ble1-midi-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-midi-upstream-parity.log`: PASS.

hwsim:

- `bluez-midi-profile-closeout`: PASS for `ble1` and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-midi-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-midi-upstream-parity-verify/run-results.json`

Boundary:

- MIDI remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across BLE MIDI GATT service,
  timestamp/payload processing, notify/write command transport, ATT/GATT, SMP,
  jitter/error handling, and cleanup lifecycle.
- This still does not claim unmodified upstream `bluetoothd` MIDI adapter
  ownership inside NuttX; the remaining adapter boundary is explicit and
  validator-gated.

## 2026-06-17 ASHA upstream-source parity verification

ASHA convergence now has explicit source-parity evidence for central and
hearing-aid roles.  The gate separates imported BlueZ ASHA/audio media
ownership, MediaTransport ownership, shared ATT/GATT ownership, battery
profile ownership, Linux HCI/mgmt/L2CAP/SMP ownership, ASHA GATT control/status
ownership, audio-frame sequencing, reconnect lifecycle, and the remaining
staged adapter boundary.

New validator-gated evidence:

- `bluez-daemon: asha closeout upstream-source-parity role=central`
- `bluez-daemon: asha closeout upstream-source-parity role=hearing-aid`
- Direct upstream: `profiles/audio/asha.c`, `profiles/audio/media.c`,
  `profiles/audio/transport.c`, `profiles/battery/bas.c`,
  `src/gatt-client.c`, `src/device.c`, `src/adapter.c`,
  `src/shared/att.c`, `src/shared/gatt-client.c`, `src/shared/gatt-db.c`,
  `src/shared/gatt-helpers.c`, `hci_core.c`, `hci_event.c`, `mgmt.c`,
  `l2cap_core.c`, `l2cap_sock.c`, and `smp.c`.
- Objects: device, GATT service/characteristic, read-only properties,
  audio control point, audio status point, volume, battery, MediaTransport,
  ATT bearer/fd, request queue, CCC, stream, paired device, and mainloop watch.
- Native ATT boundary: fixed ATT CID `0x0004`, service discovery, property
  reads, control writes, status notifications, CCC, MTU, and security.
- Native audio boundary: G.722, 10 ms frames, sequence tracking, volume,
  suspend/resume/stop, and reconnect.
- `adapter-boundary=bluezdaemon-asha-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-ble1-asha-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-asha-upstream-parity.log`: PASS.

hwsim:

- `bluez-asha-profile-closeout`: PASS for `ble1` and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-asha-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-asha-upstream-parity-verify/run-results.json`

Boundary:

- ASHA remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across ASHA GATT control/status,
  MediaTransport, battery, ATT/GATT, SMP, audio payload/control, and reconnect
  lifecycle.
- This still does not claim unmodified upstream `bluetoothd` ASHA adapter
  ownership inside NuttX; the remaining adapter boundary is explicit and
  validator-gated.

## 2026-06-17 Mesh upstream-source parity verification

Mesh convergence now has explicit source-parity evidence for provisioner and
node roles.  The gate separates imported BlueZ `bluetooth-meshd` mainloop,
manager, node, model, provisioning, config, relay/friend, RPL, proxy bearer,
Linux HCI/mgmt/L2CAP/ATT ownership, crypto/key ownership, and the remaining
staged adapter boundary.

New validator-gated evidence:

- `bluez-daemon: mesh closeout upstream-source-parity role=provisioner`
- `bluez-daemon: mesh closeout upstream-source-parity role=node`
- Direct upstream: `mesh/main.c`, `mesh/manager.c`, `mesh/dbus.c`,
  `mesh/mesh.c`, `mesh/node.c`, `mesh/net.c`, `mesh/model.c`,
  `mesh/appkey.c`, `mesh/net-keys.c`, `mesh/crypto.c`, `mesh/pb-adv.c`,
  `mesh/prov-initiator.c`, `mesh/prov-acceptor.c`,
  `mesh/cfgmod-server.c`, `mesh/friend.c`, `mesh/rpl.c`,
  `mesh/mesh-io-mgmt.c`, `hci_core.c`, `hci_event.c`, `mgmt.c`,
  `l2cap_core.c`, `l2cap_sock.c`, and `smp.c`.
- Objects: `bluetooth-meshd` mainloop, mesh manager, D-Bus name, node,
  element, model, subnet, netkey, appkey, devkey, IV index, sequence,
  replay list, provisioning session, ADV bearer, GATT proxy, ATT bearer,
  proxy filter, friend queue, and mainloop watch.
- Native bearer boundary: PB-ADV, ADV bearer, GATT proxy, ATT CID `0x0004`,
  mgmt advertising, mgmt scanning, and controller event policy.
- Native crypto boundary: netkey, appkey, devkey, SeqAuth, IV index, AES-CCM,
  and replay protection.
- `adapter-boundary=bluezdaemon-mesh-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-ble1-mesh-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-mesh-upstream-parity.log`: PASS.

hwsim:

- `bluez-mesh-profile-closeout`: PASS for `ble1` and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-mesh-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-mesh-upstream-parity-verify/run-results.json`

Boundary:

- Mesh remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across `bluetooth-meshd`,
  provisioning, config/model transport, proxy, relay/friend, RPL, crypto, and
  native BLE bearer ownership.
- This still does not claim unmodified upstream `bluetooth-meshd` adapter
  ownership inside NuttX; the remaining adapter boundary is explicit and
  validator-gated.

## 2026-06-17 OBEX/BIP upstream-source parity verification

OBEX convergence now has explicit source-parity evidence for PBAP, OPP, MAP,
MNS, FTP, Sync, and BIP client/server roles.  The gate separates imported
`obexd` mainloop/session/transfer/transport ownership, profile-specific
client/plugin ownership, Linux RFCOMM/L2CAP ownership, request/header
ownership, D-Bus object/API ownership, and the remaining staged adapter
boundary.

New validator-gated evidence:

- `bluez-obex: closeout upstream-source-parity mode=pbap-client role=pbap-client`
- `bluez-obex: closeout upstream-source-parity mode=pbap-server role=pbap-server`
- `bluez-obex: closeout upstream-source-parity mode=opp-client role=opp-client`
- `bluez-obex: closeout upstream-source-parity mode=opp-server role=opp-server`
- `bluez-obex: closeout upstream-source-parity mode=map-client role=map-client`
- `bluez-obex: closeout upstream-source-parity mode=map-server role=map-server`
- `bluez-obex: closeout upstream-source-parity mode=mns-client role=mns-client`
- `bluez-obex: closeout upstream-source-parity mode=mns-server role=mns-server`
- `bluez-obex: closeout upstream-source-parity mode=ftp-client role=ftp-client`
- `bluez-obex: closeout upstream-source-parity mode=ftp-server role=ftp-server`
- `bluez-obex: closeout upstream-source-parity mode=sync-client role=sync-client`
- `bluez-obex: closeout upstream-source-parity mode=sync-server role=sync-server`
- `bluez-obex: closeout upstream-source-parity mode=bip-client role=client`
- `bluez-obex: closeout upstream-source-parity mode=bip-server role=server`
- Direct upstream: `obexd/src/main.c`, `manager.c`, `plugin.c`,
  `server.c`, `obex.c`, `service.c`, `transfer.c`, `transport.c`,
  `obexd/client/session.c`, `obexd/client/transfer.c`,
  `obexd/client/transport.c`, `rfcomm/sock.c`, `rfcomm/core.c`, and
  `l2cap_core.c`.
- Native RFCOMM boundary: PSM `0x0003`, per-profile CIDs, fd handoff, and
  session ownership.
- D-Bus/profile APIs: `PhonebookAccess1`, `ObjectPush1`, `MessageAccess1`,
  `FileTransfer1`, `Synchronization1`, and `Image1`.
- `adapter-boundary=bluezobex-*-obex-adapter-not-unmodified-obexd parity-final=1`

Build:

- `FeatherCore/build/logs/build-bt1-obex-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-obex-upstream-parity.log`: PASS.

hwsim:

- `bluez-obex-pbap-opp-profile-closeout`: PASS for `bt1` and `bt2`.
- `bluez-obex-map-mns-profile-closeout`: PASS for `bt1` and `bt2`.
- `bluez-obex-ftp-sync-profile-closeout`: PASS for `bt1` and `bt2`.
- `bluez-obex-bip-profile-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-obex-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-obex-upstream-parity-verify/run-results.json`

Boundary:

- OBEX/BIP remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across obexd, profile plugins,
  RFCOMM, L2CAP, transfer/session ownership, and D-Bus APIs.
- This still does not claim unmodified upstream `obexd`/`bluetoothd` adapter
  ownership inside NuttX; the remaining adapter boundary is explicit and
  validator-gated.

## 2026-06-17 GATT upstream-source parity verification

GATT upstream convergence now has explicit source-parity evidence for source
and sink roles.  The gate separates BlueZ shared ATT/GATT ownership, Linux
L2CAP fixed-channel ownership, request queue ownership, notification/
indication ownership, security/error ownership, and the remaining staged
adapter boundary.

New validator-gated evidence:

- `bluez-gatt: closeout upstream-source-parity role=source`
- `bluez-gatt: closeout upstream-source-parity role=sink`
- Direct upstream: `att.c`, `gatt-db.c`, `gatt-client.c`, `gatt-server.c`,
  `io-mainloop.c`, `l2cap_sock.c`, `l2cap_core.c`, and `smp.c`.
- Objects: GATT manager, services, characteristics, descriptors, ATT bearer,
  ATT fd, request queue, notify session, CCC store, GATT DB, GATT client,
  GATT server, and mainloop watch.
- Native ATT boundary: fixed CID `0x0004`, MTU exchange, read/write,
  prepare/execute write, notify, indicate, and confirmation.
- Native security boundary: SMP, encrypted link, authorization, signed write,
  and ATT error policy.
- `adapter-boundary=bluezgatt-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-ble1-gatt-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-gatt-upstream-parity.log`: PASS.

hwsim:

- `bluez-gatt-upstream-convergence-closeout`: PASS for `ble1` and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-gatt-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-gatt-upstream-parity-verify/run-results.json`

Boundary:

- GATT remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence for ATT/GATT fixed-channel behavior.
- This still does not claim unmodified upstream `bluetoothd` adapter ownership
  inside NuttX; the remaining adapter boundary is explicit and validator-gated.

## 2026-06-17 HFP/HSP upstream-source parity verification

HFP/HSP upstream convergence now has explicit source-parity evidence for
HFP handsfree, HFP audio-gateway, HSP headset, and HSP audio-gateway roles.
The gate separates BlueZ audio profile ownership, RFCOMM/L2CAP ownership,
AT-command state, SCO/audio bearer ownership, MediaTransport ownership, and
the remaining staged adapter boundary.

New validator-gated evidence:

- `bluez-hfp: closeout upstream-source-parity mode=hfp-hf role=handsfree`
- `bluez-hfp: closeout upstream-source-parity mode=hfp-ag role=audio-gateway`
- `bluez-hfp: closeout upstream-source-parity mode=hsp-hs role=headset`
- `bluez-hfp: closeout upstream-source-parity mode=hsp-ag role=audio-gateway`
- Direct upstream: `hfp-hf.c`, `hfp-ag.c`, `headset.c`, `transport.c`,
  `profile.c`, `rfcomm/sock.c`, `rfcomm/core.c`, `sco.c`, and
  `l2cap_core.c`.
- Native RFCOMM boundary: PSM `0x0003`, CIDs `0x0061` / `0x0062`, fd handoff,
  and session ownership.
- Native SCO boundary: HCI SCO packet path, CVSD/mSBC codec ownership, and
  audio bearer lifecycle.
- `adapter-boundary=bluezhfp-hfp-adapter-not-unmodified-bluetoothd parity-final=1`
- `adapter-boundary=bluezhfp-hsp-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-bt1-hfp-hsp-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hfp-hsp-upstream-parity.log`: PASS.

hwsim:

- `bluez-hfp-hsp-profile-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-hfp-hsp-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-hfp-hsp-upstream-parity-verify/run-results.json`

Boundary:

- HFP/HSP remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across RFCOMM, AT state, SCO, and
  MediaTransport ownership.
- This still does not claim unmodified upstream `bluetoothd` adapter ownership
  inside NuttX; the remaining adapter boundaries are explicit and
  validator-gated.

## 2026-06-17 HID/HOGP upstream-source parity verification

HID/HOGP upstream convergence now has explicit source-parity evidence for
Classic HID host/device and BLE HOGP host/device roles.  The gate separates
BlueZ input profile ownership, Linux HIDP/L2CAP ownership, HOGP ATT/GATT
ownership, and the remaining staged adapter boundary.

New validator-gated evidence:

- `bluez-hid: closeout upstream-source-parity role=classic-host profile=classic-hid`
- `bluez-hid: closeout upstream-source-parity role=classic-device profile=classic-hid`
- `bluez-hid: closeout upstream-source-parity role=hogp-host profile=hogp`
- `bluez-hid: closeout upstream-source-parity role=hogp-device profile=hogp`
- Classic HID direct upstream: `device.c`, `server.c`, `profile.c`,
  `hidp/core.c`, `hidp/sock.c`, `l2cap_sock.c`, and `l2cap_core.c`.
- HOGP direct upstream: `hog.c`, `hog-lib.c`, `att.c`, `gatt-client.c`,
  `gatt-db.c`, `l2cap_sock.c`, `l2cap_core.c`, and `smp.c`.
- Classic native boundary: control PSM `0x0011`, interrupt PSM `0x0013`,
  HIDPCONNADD/HIDPCONNDEL, HIDP session thread, UHID/input-core, and fd
  handoff.
- HOGP native boundary: fixed ATT channel `0x0004`, report map/protocol
  mode/input report handles, CCC notify, LE security/SMP, and ATT/GATT
  request lifecycle.
- `adapter-boundary=bluezhid-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-bt1-hid-hogp-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hid-hogp-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hid-hogp-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hid-hogp-upstream-parity.log`: PASS.

hwsim:

- `bluez-hid-upstream-convergence-closeout`: PASS for `bt1`, `bt2`,
  `ble1`, and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-hid-hogp-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-hid-hogp-upstream-parity-verify/run-results.json`

Boundary:

- HID/HOGP remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence across Classic HID and BLE HOGP.
- This still does not claim unmodified upstream `bluetoothd` adapter ownership
  inside NuttX; the remaining adapter boundary is explicit and validator-gated.

## 2026-06-17 LE Audio upstream-source parity verification

LE Audio upstream convergence now has the same sharper source-parity style
used for A2DP.  The `bluezleaudio closeout` path records direct upstream source
coverage, object ownership, handler coverage, native ISO/GATT boundaries, and
the remaining staged adapter boundary separately for source and sink roles.

New validator-gated evidence:

- `bluez-leaudio: closeout upstream-source-parity role=source`
- `bluez-leaudio: closeout upstream-source-parity role=sink`
- `direct-upstream=main.c,bap.c,pacs.c,ascs.c,media.c,transport.c,lc3.c,att.c,gatt-client.c,iso.c,l2cap_sock.c,hci_event.c`
- `objects=adapter,device,pacs,ascs,ase,bap-session,pac,stream,media-endpoint,media-transport,iso-fd,att-bearer,gatt-client,dbus-name,mainloop-watch`
- `handlers=bt_bap_attach,bt_bap_select,bt_bap_stream_enable,bt_bap_stream_start,bt_bap_stream_disable,bt_bap_stream_release,pacs_register,ascs_cp_write,media_endpoint_set_configuration,media_endpoint_clear_configuration,transport_acquire,transport_try_acquire,transport_release,iso_bind,iso_connect,iso_sendmsg,iso_recvmsg,lc3_encode,lc3_decode`
- `native-iso=cis,bis,cig,cis-handle-0x0201,fd-handoff,qos-policy,controller-timing`
- `native-gatt=att-cid-0x0004,pacs-0x1850,ascs-0x184e,ase-control-point`
- `adapter-boundary=bluezleaudio-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-ble1-leaudio-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-ble2-leaudio-upstream-parity.log`: PASS.

hwsim:

- `bluez-le-audio-umbrella`: PASS for `ble1` and `ble2`.

Artifacts:

- `FeatherCore/build/logs/run-leaudio-upstream-parity-umbrella-verify.log`
- `FeatherCore/build/bt-hwsim-leaudio-upstream-parity-umbrella-verify/run-results.json`

Verification note:

- The umbrella case is a heavy full-stack script; it was rerun with a long
  case timeout so the script could reach the final `bluezleaudio closeout`
  source/sink commands.  A shorter timeout can stop before parity evidence is
  emitted and should not be interpreted as a protocol failure by itself.

Boundary:

- LE Audio remains closed for the current hwsim functional target and now has
  explicit upstream-source parity evidence for BAP/PACS/ASCS/LC3/ATT/GATT/ISO.
- This still does not claim unmodified upstream `bluetoothd` adapter ownership
  inside NuttX; the remaining adapter boundary is explicit and validator-gated.

## 2026-06-17 A2DP upstream-source parity verification

A2DP upstream convergence gained a stricter daemon-side parity ledger and
validator gate.  The closeout now records the direct upstream source coverage,
object ownership, handler coverage, native L2CAP fd handoff/policy boundary,
and the remaining staged adapter boundary separately.

New validator-gated evidence:

- `bluez-daemon: a2dp closeout upstream-source-parity role=source`
- `bluez-daemon: a2dp closeout upstream-source-parity role=sink`
- `direct-upstream=profile.c,device.c,adapter.c,dbus-common.c,mainloop.c,io-mainloop.c,a2dp.c,avdtp.c,media.c,transport.c,avrcp.c,sbc.c,l2cap_sock.c`
- `objects=adapter,profile,device,session,setup,stream,sep,media-endpoint,media-transport,avrcp-player,l2cap-fd,dbus-name,mainloop-watch`
- `handlers=profile_connect,profile_disconnect,media_endpoint_set_configuration,media_endpoint_clear_configuration,transport_acquire,transport_try_acquire,transport_release,avdtp_discover,avdtp_set_configuration,avdtp_open,avdtp_start,avdtp_suspend,avdtp_close,avrcp_control,avrcp_browsing`
- `native-l2cap=psm-0x0019,cid-0x0040,cid-0x0041,fd-handoff,controller-policy`
- `adapter-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd parity-final=1`

Build:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-parity.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-parity.log`: PASS.

hwsim:

- `bluez-a2dp-current-complete-closeout`: PASS for `bt1` and `bt2`.
- `bluez-a2dp-upstream-convergence-closeout`: PASS for `bt1` and `bt2`.

Artifacts:

- `FeatherCore/build/logs/run-a2dp-upstream-parity-verify.log`
- `FeatherCore/build/bt-hwsim-a2dp-upstream-parity-verify/run-results.json`

Boundary:

- A2DP remains closed for the current hwsim functional target and now has a
  sharper upstream-source parity gate.
- This still does not claim unmodified upstream `bluetoothd` adapter ownership
  inside NuttX; the remaining adapter boundary is explicit and validator-gated.

## 2026-06-17 BLE IP/6LoWPAN hwsim closeout

BLE IP/6LoWPAN was rebuilt and rerun after aligning the BT/BLE hwsim
packet-buffer size to full Ethernet frames.

Build:

- `FeatherCore/build/logs/build-ble1-eth-pktsize-1514.log`: PASS.
- `FeatherCore/build/logs/build-ble2-eth-pktsize-1514.log`: PASS.

hwsim:

- `ble-ip-ping`: PASS.
- `ble-ip-reconnect-stress`: PASS.
- `ble-ip-iperf-tcp`: PASS.
- `ble-ip-iperf-tcp-reverse`: PASS.
- `ble-ip-iperf-udp`: PASS.
- `ble-ip-iperf-udp-reverse`: PASS.

Artifacts:

- `FeatherCore/build/logs/run-ble-ip-ping-after-pktsize-1514-verify.log`
- `FeatherCore/build/logs/run-ble-ip-closeout-after-pktsize-1514-verify.log`

This keeps BLE Network aligned with the same current hwsim functional bar as
BT Network/BNEP: ping, reconnect, TCP/UDP throughput, reverse direction, and
teardown evidence are validator-gated.

## 2026-06-17 BlueZ Network iperf matrix closeout

`bluez-network-iperf-matrix` is closed in the latest validation run.

Changes behind the closeout:

- BT/BLE hwsim defconfigs now set `CONFIG_NET_ETH_PKTSIZE=1514`, so the
  simulator packet buffer size is aligned with full Ethernet-frame BNEP and
  BLE 6LoWPAN traffic.
- NuttX `iperf` UDP client now uses `SO_SNDTIMEO` and retries transient send
  congestion (`ENOMEM`, `EAGAIN`, `EWOULDBLOCK`) instead of letting a blocked
  UDP `sendto()` stall the BlueZ Network disconnect/reconnect lifecycle.

Build:

- `FeatherCore/build/logs/build-bt1-iperf-udp-send-timeout.log`: PASS.
- `FeatherCore/build/logs/build-bt2-iperf-udp-send-timeout.log`: PASS.

hwsim:

- `FeatherCore/build/logs/run-iperf-udp-send-timeout-matrix-verify.log`: PASS.
- `FeatherCore/build/logs/run-bnep-network-iperf-matrix-closeout-verify.log`: PASS.

Combined closeout coverage:

- `bluez-bneptest-fd-handoff`
- `bluez-bneptest-ping`
- `bluez-bneptest-reconnect-stress`
- `bluez-network-ping`
- `bluez-network-reconnect-stress`
- `bluez-network-iperf-udp`
- `bluez-network-iperf-udp-reverse`
- `bluez-network-iperf-matrix`

This supersedes the matrix-open boundary note below.

## 2026-06-17 BlueZ Network clean-baseline verification

BT1/BT2 were rebuilt serially and the current BlueZ Network/BNEP hwsim clean
baseline was rerun.

Build:

- `FeatherCore/build/logs/build-bt1-bnep-network-clean-baseline-serial.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bnep-network-clean-baseline-serial.log`: PASS.

hwsim PASS baseline:

- `bluez-bneptest-fd-handoff`
- `bluez-bneptest-ping`
- `bluez-bneptest-reconnect-stress`
- `bluez-network-ping`
- `bluez-network-reconnect-stress`
- `bluez-network-iperf-udp`
- `bluez-network-iperf-udp-reverse`

Artifacts:

- `FeatherCore/build/logs/run-bnep-kernel-sendmsg-network-clean-baseline-final-verify.log`
- `FeatherCore/build/bt-hwsim-bnep-kernel-sendmsg-network-clean-baseline-final-verify/run-results.json`

Current BlueZ Network boundary:

- `bluez-network-iperf-matrix` remains open in the latest verification.
- The current failure is mixed TCP/UDP multi-cycle reconnect: BlueZ Network can
  reconnect and ping, but UDP iperf reports zero payload after earlier cycles
  and prevents the later teardown/reconnect count from completing.
- Next work should fix the BlueZ Network/native BNEP socket lifecycle for that
  mixed matrix without adding direct hwsim fallback or staged bridge shortcuts.

Latest failing matrix artifact:

- `FeatherCore/build/logs/run-bnep-kernel-sendmsg-iperf-matrix-per-round-pump-verify.log`

## 2026-06-17 BNEP no-fallback native closeout verification

Native BNEP datapath closeout now forbids kept-probe/rebind-probe lookup
fallback counters, and the focused BNEP/BlueZ Network validation batch passes
with that stricter rule.

Build:

- `FeatherCore/build/logs/build-bt1-bnep-nofallback-convergence.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bnep-nofallback-convergence.log`: PASS.

hwsim:

- `bluez-bneptest-fd-handoff`: PASS.
- `bluez-bneptest-ping`: PASS.
- `bluez-bneptest-reconnect-stress`: PASS.
- `bluez-network-ping`: PASS.
- `bluez-network-reconnect-stress`: PASS.
- `bluez-network-iperf-matrix`: PASS.

Artifacts:

- `FeatherCore/build/logs/run-bnep-nofallback-convergence-verify.log`
- `FeatherCore/build/bt-hwsim-bnep-nofallback-convergence-verify/run-results.json`

## 2026-06-17 BNEP fd lookup convergence verification

The fd lookup fallback visibility change was followed by build and hwsim
validation.

Build:

- `FeatherCore/build/logs/build-bt1-bnep-fdlookup-convergence.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bnep-fdlookup-convergence.log`: PASS.

hwsim:

- `bluez-bneptest-fd-handoff`: PASS.
- `bluez-bneptest-ping`: PASS.
- `bluez-bneptest-reconnect-stress`: PASS.
- `bluez-network-ping`: PASS.
- `bluez-network-reconnect-stress`: PASS.
- `bluez-network-iperf-matrix`: PASS.

Artifacts:

- `FeatherCore/build/logs/run-bnep-fdlookup-convergence-verify.log`
- `FeatherCore/build/bt-hwsim-bnep-fdlookup-convergence-verify/run-results.json`

## 2026-06-17 BNEP/NET convergence verification

The BNEP/NET upstream-convergence tightening was followed by BT1/BT2 builds
and a focused hwsim validation batch.

Build:

- `FeatherCore/build/logs/build-bt1-bnep-upstream-convergence.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bnep-upstream-convergence.log`: PASS.

hwsim:

- `bluez-bneptest-fd-handoff`: PASS.
- `bluez-bneptest-ping`: PASS.
- `bluez-bneptest-reconnect-stress`: PASS.
- `bluez-network-ping`: PASS.
- `bluez-network-reconnect-stress`: PASS.
- `bluez-network-iperf-matrix`: PASS.

Artifacts:

- `FeatherCore/build/logs/run-bnep-upstream-convergence-verify.log`
- `FeatherCore/build/bt-hwsim-bnep-upstream-convergence-verify/run-results.json`

## 2026-06-17 upstream convergence phase

The next phase has started: reduce compatibility/probe glue while preserving
the current hwsim functional closeout baseline.

Initial BNEP/NET convergence change:

- The native `BNEPCONNADD` fd handoff marker is now emitted only after the
  underlying connection setup and userspace ioctl result copy-out both succeed.
- This aligns the hwsim evidence with what BlueZ / `bneptest` actually observe:
  a completed successful ioctl, not merely an internal mid-path success.
- Generic native BNEP ioctl counters now count successful ioctl returns only,
  preventing failed attempts from being reported as successful native coverage.
- BNEP native fd handoff status now distinguishes `socket-fd` from
  `kept-probe-l2cap`, so compatibility fallback ownership remains visible.
- Native BNEP datapath and BlueZ `bneptest` fd handoff gates now require
  `bnep-native-fd-source=socket-fd`; the kept-probe fallback no longer counts
  as native closeout evidence.
- Successful native `BNEPCONNDEL` now clears fd ownership metadata, keeping
  post-disconnect status from advertising stale connected-fd state.
- Native teardown gates now require the cleared fd snapshot, so stale fd
  ownership metadata no longer passes disconnect lifecycle validation.
- Native fd handoff evidence no longer overwrites an active fd ownership
  snapshot; overlapping handoff markers wait for teardown.
- Rejected overlapping fd handoff attempts are now exposed through
  `bnep-native-fd-handoff-reject`.
- Native datapath closeout now requires `bnep-native-fd-handoff-reject=0`, so
  overlapping fd handoff attempts fail the current single-session closeout.
- Reconnect and reconnect-stress gates now require the same zero-overlap rule,
  keeping fd ownership serialized across lifecycle validation.
- BlueZ `bneptest` native-boundary and closeout logs now print
  `fd-source=socket-fd`, and validator coverage requires that source evidence.
- BNEP native status now exposes kept-probe and rebind-probe lookup counters,
  making remaining fd lookup compatibility fallback use auditable.
- Native BNEP datapath closeout now requires both fallback lookup counters to
  stay zero; those compatibility paths no longer count as native coverage.

This is the kind of change expected in this phase:

- Prefer completed upstream-shaped object/session/socket results over early
  probe markers.
- Keep validator evidence strict enough to reject false-positive native
  ownership.
- Do not expand feature breadth unless it directly removes a compatibility
  boundary.

## 2026-06-15 implementation progress snapshot

This BlueZ-on-NuttX hwsim port is currently at a functional closeout
checkpoint.  The current value is not just individual smoke commands; it is a
validator-gated hwsim baseline covering the main BT/BLE profile groups.

Closed for the current hwsim functional target:

- BT/BLE basic control flow: HCI socket, mgmt socket, scan/connect,
  auth/pairing-shaped lifecycle, disconnect, owner cleanup, and error path.
- A2DP: source/sink-shaped profile ownership, AVDTP signaling, media transport,
  start/suspend/close, reconnect, codec/policy markers, and payload evidence.
- LE Audio: ISO/CIS/BIS-shaped transport, BAP/PACS/ASCS, CAP/CSIP, VCP/MICP,
  MCP/TMAP/CCP/GMAP, role soak, LC3 media markers, reconnect, teardown, and
  error recovery.
- BT/BLE NET: BlueZ Network, `bneptest`, native BNEP/IPSP-shaped handoff,
  ping, TCP/UDP iperf, reverse direction, MTU soak, reconnect stress, and
  cleanup.
- Remaining profiles at the same current bar: HID/HOGP, HFP/HSP, GATT, Mesh,
  OBEX, ASHA, MIDI, Ranging, Print, and iAP.

Current boundary:

- This is a hwsim functional closeout, not a claim that unmodified upstream
  `bluetoothd`, `obexd`, `bluetooth-meshd`, and Linux kernel Bluetooth code now
  run without compatibility glue inside NuttX.
- The next phase is upstream convergence: remove adapter/probe glue and move
  ownership back into direct imported upstream BlueZ/Linux paths while keeping
  these gates green.

## 2026-06-15 current hwsim functional freeze checkpoint

Current recommendation: stop feature optimization here and do a checkpoint
wrap-up before opening more Bluetooth work.

The current BT/BLE hwsim target is closed at the functional-validation level:
A2DP, LE Audio, BT/BLE basic control flows, BT/BLE NET, HID/HOGP, HFP/HSP,
GATT, Mesh, OBEX, ASHA, MIDI, Ranging, Print, and iAP all have BlueZ-shaped
control surfaces, dual-role hwsim evidence, lifecycle/error/cleanup coverage,
validator enforcement, and recorded PASS artifacts.

This checkpoint should be treated as a stable handoff point:

- Do not add more feature optimization before preserving the current state.
- Keep the generated PASS artifacts as the evidence for this phase.
- Use future work to reduce compatibility/probe glue and move closer to
  unmodified upstream BlueZ/Linux object ownership.
- Do not claim the entire Linux Bluetooth stack is fully unmodified upstream
  inside NuttX; the accurate claim is "closed for the current hwsim functional
  target, with source-level upstream convergence still ongoing."

Recommended wrap-up scope:

- Freeze the current validator gates and evidence list.
- Commit/push the current effective code if the worktree scope is acceptable.
- Start the next phase only after this checkpoint is preserved.

## 2026-06-15 global Bluetooth feature closeout bar

All remaining BT/BLE features should be brought to the same level as the
current A2DP and LE Audio hwsim closeouts.  The accepted bar is no longer
"prints a plausible probe log"; it is a batched, dual-role, validator-enforced
functional closeout.

Required closeout evidence:

- BlueZ-shaped control surface for the relevant daemon/tool/profile role.
- Real Linux-BT/NuttX socket, HCI, mgmt, L2CAP, ISO, BNEP, ATT/GATT, or hwsim
  medium state/payload evidence, depending on the feature.
- Independent local and peer sim roles.
- Teardown, reconnect, owner cleanup, and error lifecycle.
- Validator gates that fail when transport/control/data-path evidence is
  missing.
- PASS artifacts from a batched hwsim run.

Current status:

- A2DP: closed for the current hwsim functional target.
- LE Audio: closed for the current hwsim functional target.
- BT/BLE basic control flows: closed for the current hwsim functional target.
- BT/BLE NET: closed for the current hwsim functional target.
- HID/HOGP, HFP/HSP, GATT, Mesh, OBEX, ASHA, MIDI, and Ranging profiles:
  closed for the current hwsim functional target.
- Remaining work is no longer to prove only single-role smoke coverage; it is
  to keep replacing compatibility/probe glue with real upstream
  BlueZ/Linux-BT object ownership where practical, then rerun the same gates.

Remaining profile closeout contract:

- HID/HOGP: require BlueZ input/HOG ownership, HID interrupt/control or ATT
  report traffic, input event delivery, reconnect, teardown, protocol policy,
  error cleanup, and dual-role validator PASS evidence.
- HFP/HSP: require headset/audio-gateway ownership, RFCOMM/SCO or SCO-like
  hwsim socket evidence, AT command state, audio/call lifecycle, reconnect,
  teardown, error cleanup, and dual-role validator PASS evidence.
- GATT: require ATT bearer ownership, service database registration,
  discovery, read/write, notify/indicate, MTU exchange, security/error policy,
  reconnect, teardown, and dual-role validator PASS evidence.
- OBEX: require session ownership, object push/pull or file-transfer payload
  movement, abort/error policy, reconnect, teardown, and daemon/client cleanup.
- Mesh: require provisioning/configuration, node/key ownership, hwsim
  advertising-bearer TX/RX, replay/error policy, reconnect, teardown, and
  dual-role validator PASS evidence.
- MIDI: require GATT MIDI service ownership, timestamped MIDI payload
  movement, notify/write data path, reconnect, teardown, and cleanup policy.
- ASHA: require GATT control ownership, audio/session lifecycle, media payload
  or payload-equivalent hwsim evidence, reconnect, teardown, and error cleanup.

Closeout rule:

- Do not mark a remaining profile closed from a symbol probe, single log line,
  or single-role command.
- The minimum acceptable evidence is the same style already used for A2DP and
  LE Audio: BlueZ-shaped control surface, real Linux-BT/NuttX transport or
  hwsim medium evidence, local/peer roles, teardown/reconnect/error lifecycle,
  validator gates, and recorded PASS artifacts.

## 2026-06-15 BT/BLE NET hwsim functional closeout

BT/BLE NET now matches the current A2DP and LE Audio hwsim closeout standard:
BlueZ-shaped control surface, Linux-BT/NuttX BNEP/IPSP data paths, dual-role
hwsim evidence, reconnect/stress, teardown, error lifecycle, and upstream
coverage-map validation.

Validated hwsim cases:

- `ble-ip-closeout-full`: PASS for `ble1` and `ble2`.
- `bluez-ipsp-closeout-full`: PASS for `ble1` and `ble2`.
- `bluez-daemon-ipsp-closeout-full`: PASS for `ble1` and `ble2`.
- `bneptest-ping`: PASS for `bt1` and `bt2`.
- `bneptest-reconnect`: PASS for `bt1` and `bt2`.
- `bneptest-reconnect-stress`: PASS for `bt1` and `bt2`.
- `bluez-bneptest-ping`: PASS for `bt1` and `bt2`.
- `bluez-bneptest-reconnect`: PASS for `bt1` and `bt2`.
- `bluez-bneptest-reconnect-stress`: PASS for `bt1` and `bt2`.
- `bluez-network-ping`: PASS for `bt1` and `bt2`.
- `bluez-network-error-path`: PASS for `bt1`.
- `bluez-network-reconnect`: PASS for `bt1` and `bt2`.
- `bluez-network-reconnect-stress`: PASS for `bt1` and `bt2`.
- `bluez-network-closeout-full`: PASS for `bt1` and `bt2`.
- `bluez-net-current-complete-closeout`: PASS for `bt1`, `bt2`, `ble1`,
  and `ble2`.
- `bluez-net-upstream-convergence-closeout`: PASS for `bt1`, `bt2`, `ble1`,
  and `ble2`.

Verification artifacts:

- `FeatherCore/build/logs/run-net-closeout-main.log`
- `FeatherCore/build/bt-hwsim-usecases-net-closeout-main/`
- `FeatherCore/build/logs/run-net-current-complete-fix.log`
- `FeatherCore/build/bt-hwsim-usecases-net-current-complete-fix/run-results.json`
- `FeatherCore/build/logs/run-net-upstream-convergence-fix.log`
- `FeatherCore/build/bt-hwsim-usecases-net-upstream-convergence-fix/run-results.json`

Required evidence now covered:

- BlueZ Network profile D-Bus ownership and NetworkServer1 registration for
  PANU, NAP, and GN roles.
- BlueZ `bneptest.c`-style connected L2CAP fd handoff into `BNEPCONNADD`.
- Native BNEP session/netdev path with `ndo_start_xmit`, BNEP frame TX/RX,
  L2CAP delivery, `netif_rx`, teardown, reconnect, and stress evidence.
- BLE IPSP/6LoWPAN through daemon-shaped profile ownership, LE L2CAP CoC
  handoff, `bt0` registration, IPHC encode/decode, fragment counters, TCP/UDP
  payload movement, and cleanup to owner refs zero.

Current boundary:

- BT/BLE NET is closed for the current hwsim functional target.
- The remaining Bluetooth work should continue profile by profile using this
  same bar, then reduce compatibility glue toward unmodified upstream
  `bluetoothd` where practical.

## 2026-06-15 BT/BLE NET iperf and stability addendum

BT/BLE NET now has hwsim evidence beyond ping.  The current gates cover
TCP/UDP, both directions, MTU/large-packet soak, reconnect stress, native BNEP,
BlueZ Network, BlueZ `bneptest` fd handoff, and BLE 6LoWPAN/IPSP iperf.

Validated hwsim cases:

- `bluez-network-iperf-matrix`: PASS for `bt1` and `bt2`.
- `bluez-network-mtu-soak`: PASS for `bt1` and `bt2`.
- `bluez-network-mtu-reconnect-stress`: PASS for `bt1` and `bt2`.
- `bluez-bneptest-iperf-tcp`: PASS for `bt1` and `bt2`.
- `bluez-bneptest-iperf-tcp-reverse`: PASS for `bt1` and `bt2`.
- `bluez-bneptest-iperf-udp`: PASS for `bt1` and `bt2`.
- `bluez-bneptest-iperf-udp-reverse`: PASS for `bt1` and `bt2`.
- `bnep-iperf-tcp`: PASS for `bt1` and `bt2`.
- `bnep-iperf-tcp-reverse`: PASS for `bt1` and `bt2`.
- `bnep-iperf-udp`: PASS for `bt1` and `bt2`.
- `bnep-iperf-udp-reverse`: PASS for `bt1` and `bt2`.
- `ble-ip-iperf-tcp`: PASS for `ble1` and `ble2`.
- `ble-ip-iperf-tcp-reverse`: PASS for `ble1` and `ble2`.
- `ble-ip-iperf-udp`: PASS for `ble1` and `ble2`.
- `ble-ip-iperf-udp-reverse`: PASS for `ble1` and `ble2`.

Verification artifacts:

- `FeatherCore/build/bt-hwsim-bluez-network-iperf-matrix-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bluez-network-mtu-soak-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bluez-network-mtu-reconnect-stress-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bluez-bneptest-iperf-tcp-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bluez-bneptest-iperf-tcp-reverse-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bluez-bneptest-iperf-udp-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bluez-bneptest-iperf-udp-reverse-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bnep-iperf-tcp-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bnep-iperf-tcp-reverse-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bnep-iperf-udp-current/run-results.json`
- `FeatherCore/build/bt-hwsim-bnep-iperf-udp-reverse-current/run-results.json`
- `FeatherCore/build/bt-hwsim-ble-ip-iperf-tcp-current/run-results.json`
- `FeatherCore/build/bt-hwsim-ble-ip-iperf-tcp-reverse-current/run-results.json`
- `FeatherCore/build/bt-hwsim-ble-ip-iperf-udp-current/run-results.json`
- `FeatherCore/build/bt-hwsim-ble-ip-iperf-udp-reverse-current/run-results.json`

Current boundary:

- The previous "ping only" limitation is closed for the current hwsim target.
- The next NET convergence work is source-level cleanup: continue shrinking
  staged/probe compatibility glue while preserving the native BNEP/IPSP fd
  handoff, netdev, IPHC, TCP/UDP, MTU, reconnect, and teardown gates above.

## 2026-06-15 BT/BLE basic hwsim functional closeout

The BT/BLE basic control-plane batch now passes across BR/EDR, LE, mgmt
socket, pairing, and BlueZ basic convergence paths.

Validated hwsim cases:

- `bt-basic`: PASS for `bt1` and `bt2`.
- `ble-basic`: PASS for `ble1` and `ble2`.
- `hci-bredr-medium`: PASS for `bt1` and `bt2`.
- `hci-le-medium`: PASS for `ble1` and `ble2`.
- `hci-le-pairing`: PASS for `ble1` and `ble2`.
- `mgmt-control`: PASS for `ble1`.
- `bluez-hci-mgmt-socket-closeout-full`: PASS for `ble1` and `ble2`.
- `bluez-basic-mgmt-flow`: PASS for `ble1` and `ble2`.
- `bluez-basic-scan-connect-auth-flow`: PASS for `ble1` and `ble2`.
- `bluez-basic-upstream-convergence-closeout`: PASS for `bt1`, `bt2`,
  `ble1`, and `ble2`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-basic-closeout.log`
- `FeatherCore/build/logs/build-bt2-basic-closeout.log`
- `FeatherCore/build/logs/build-ble1-basic-closeout.log`
- `FeatherCore/build/logs/build-ble2-basic-closeout.log`
- `FeatherCore/build/logs/run-basic-closeout-final.log`
- `FeatherCore/build/bt-hwsim-usecases-basic-closeout-final/run-results.json`

Implementation notes:

- `hci-bredr-medium` passive-side validation now follows the same non-blocking
  CTRL medium/event-ring model as `hci-le-medium`.
- This validates BR/EDR connect/disconnect lifecycle through hwsim CTRL
  medium, HCI events, mgmt socket reads, upstream HCI status, and mgmt owner
  release without depending on discovery scan behavior for disconnect records.

Current boundary:

- BT/BLE basic control flow is closed for the current hwsim functional target.
- The next large target is BT/BLE NET at the same standard: BNEP/IPSP payload
  data path, BlueZ Network service ownership, role handling, reconnect,
  teardown, and error lifecycle.

## 2026-06-15 LE Audio hwsim functional closeout

LE Audio has moved past ISO/audio-like payload probe coverage for the current
hwsim scope.  The closeout batch now validates controller setup, daemon-owned
profile flow, repeated source/sink role soak, and an umbrella lifecycle across
unicast CIS, broadcast BIS, BAP/PACS/ASCS, CAP/CSIP, VCP/MICP, MCP/TMAP/CCP/
GMAP, D-Bus ownership, LC3 media, ISO socket counters, teardown, reconnect,
and error recovery paths.

Validated hwsim cases:

- `bluez-le-audio-controller-daemon-full-stack`: PASS for `ble1` and `ble2`.
- `bluez-le-audio-role-soak`: PASS for `ble1` and `ble2`.
- `bluez-le-audio-umbrella`: PASS for `ble1` and `ble2`.

Verification artifacts:

- `FeatherCore/build/logs/build-ble1-leaudio-closeout-final2.log`
- `FeatherCore/build/logs/build-ble2-leaudio-closeout-final2.log`
- `FeatherCore/build/logs/run-leaudio-closeout-final2.log`
- `FeatherCore/build/bt-hwsim-usecases-leaudio-closeout-final2/run-results.json`

Required evidence now covered:

- HCI USER ISO controller evidence for CIS/BIS setup, create/terminate events,
  setup path, buffer size, and TX sync.
- BlueZ-shaped BAP/PACS/ASCS ASE state transitions, QoS reject/retry, metadata
  update, enable/start, disable/stop-ready, release, and scheduler cleanup.
- BlueZ-shaped `MediaEndpoint1` / `MediaTransport1` D-Bus ownership including
  register/configure/acquire/busy/owner-lost/owner-reacquire/release.
- Real hwsim ISO media evidence through `upstream-iso-write`,
  `upstream-iso-recv`, and early `btctl upstream status` ISO socket counters.
- Broadcast source/sink BIS/BASS/BASE/security flow plus unicast source/sink
  role rotation and repeated reconnect/error lifecycle.

Current boundary:

- LE Audio is closed for the current hwsim functional target, at the same
  project stage as A2DP: full-role BlueZ-shaped profile flow plus real
  Linux-BT/hwsim ISO media evidence now validates in batch.
- This is still not a claim that the entire Linux Bluetooth stack is complete:
  BT/BLE basic flow closeout, BT/BLE NET, full HCI/mgmt/socket ABI, and
  remaining BlueZ profiles still need the same convergence treatment.
- A future cleanup can continue reducing compatibility glue toward unmodified
  `bluetoothd`, but LE Audio is no longer the blocker for moving to the next
  major feature.

## 2026-06-15 A2DP real L2CAP media closeout

A2DP has moved past staged/probe-only media transport for the current hwsim
scope.  The BlueZ-shaped `MediaTransport1.Acquire` paths now exercise real
NuttX/Linux-BT BR/EDR L2CAP media sockets on PSM `0x0019`, media CID `0x0041`,
with source and sink roles using `linux_bt_upstream_l2cap_socket_open()`,
`connect_handle()`, `write_handle()`, `recv_handle()`, and `close_handle()`.

Validated hwsim cases:

- `bluez-a2dp-transport-bidir`: PASS for `bt1` and `bt2`.
- `bluez-a2dp-transport-bidir-teardown`: PASS for `bt1` and `bt2`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-closeout-final3.log`
- `FeatherCore/build/logs/build-bt2-a2dp-closeout-final3.log`
- `FeatherCore/build/logs/run-a2dp-closeout-final5.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-closeout-final5/run-results.json`

Required evidence now covered:

- Source `MediaTransport1.Acquire` writes media through
  `upstream-l2cap-write-handle: psm=0x0019 cid=0x0041 ... payload-len=24
  send-ret=24`.
- Sink `MediaTransport1.Acquire` reads the matching media payload through
  `upstream-l2cap-recv-handle: psm=0x0019 cid=0x0041 ... recv-ret=24`.
- Bidirectional media is validated by the dedicated bidir case.
- Teardown validates source-side AVDTP `Suspend` and `Close Stream`
  transactions by requiring the sender to receive responses for signal
  `0x09` and `0x08`.
- The remote side validates AVDTP state transitions through
  `STREAMING->OPEN` and `OPEN->IDLE`.
- L2CAP bind/connect/listen/send/recv counters and zero native attach/recv
  failures are checked by the validator.

Current boundary:

- A2DP is closed for the current hwsim functional target: BlueZ-shaped
  AVDTP signaling, media transport acquire/read-write/release, bidirectional
  media, and suspend/close teardown all pass over Linux-BT L2CAP socket
  helpers.
- This is still not a claim that the entire Linux Bluetooth stack is complete:
  LE Audio, BT/BLE NET, full HCI/mgmt/socket ABI, and the remaining BlueZ
  profiles still need the same real-upstream convergence treatment.
- A future A2DP cleanup can continue reducing compatibility glue toward
  unmodified `bluetoothd`, but it is no longer the blocker for moving to the
  next major feature.

## 2026-06-15 A2DP GIOChannel-owned media fd closeout batch

A2DP closeout advanced from AVDTP-stream-owned fd/MTU tuple storage to a
GIOChannel-backed media fd handoff.  The A2DP/AVDTP stream owner now creates a
`GIOChannel` from the media fd and the transport closeout obtains the fd back
through `g_io_channel_unix_get_fd()`, matching the shape of upstream
`avdtp_stream_set_transport()` / `avdtp_stream_get_transport()`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-gio-media-fd.log`
- `FeatherCore/build/logs/build-bt2-a2dp-gio-media-fd.log`
- `FeatherCore/build/logs/run-a2dp-gio-media-fd.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-gio-media-fd/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-avdtp-media-fd=owner:1,set:1,get:1,reply:1,total:4`.
- Sink emits
  `upstream-avdtp-media-fd=owner:1,set:1,get:1,reply:1,total:4`.
- The same run still proves Acquire completion, Release cleanup,
  D-Bus unregister/free lifecycle, and lifecycle/error closeout.

Implementation notes:

- `upstream_object_shims/glib.h` now provides `g_io_channel_unix_new()`.
- `upstream_a2dp_object_probe.c` now stores the prepared media fd in a
  `GIOChannel` instead of a raw fd field.
- `upstream_transport_object_probe.c` continues to consume media fd/MTU through
  the AVDTP stream accessor.

Current boundary:

- This aligns the fd handoff shape with upstream BlueZ more closely than the
  previous raw tuple.
- This still does not claim real L2CAP media socket completion: the remaining
  A2DP boundary is replacing the probe-created fd with a real NuttX/Linux-BT
  L2CAP socket and driving full AVDTP signaling/media setup end to end.

## 2026-06-15 A2DP AVDTP-owned media fd closeout batch

A2DP closeout advanced from a transport-local bounded fd provider to an
AVDTP-stream-owned media fd/MTU handoff.  The closeout validator now requires
the fd/MTU values to be set on the prepared A2DP/AVDTP stream owner before
`transport.c:a2dp_resume_complete()` obtains them through
`avdtp_stream_get_transport()` and replies to `MediaTransport1.Acquire`.

Validated batched surface:

- The prepared A2DP/AVDTP stream owns the media fd/MTU values.
- `transport.c:a2dp_resume_complete()` obtains fd/MTU through the stream
  transport accessor instead of a transport-local fixed provider.
- The Acquire reply path still installs fd/MTU on the media transport.
- The same closeout run still proves Acquire completion, Release cleanup,
  D-Bus unregister/free lifecycle, and lifecycle/error behavior.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-avdtp-media-fd.log`
- `FeatherCore/build/logs/build-bt2-a2dp-avdtp-media-fd.log`
- `FeatherCore/build/logs/run-a2dp-avdtp-media-fd.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-media-fd/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-avdtp-media-fd=owner:1,set:1,get:1,reply:1,total:4`.
- Sink emits
  `upstream-avdtp-media-fd=owner:1,set:1,get:1,reply:1,total:4`.
- The same run still proves
  `upstream-acquire-complete=fd:1,mtu:1,reply:1,state:1,total:4`.
- The same run still proves
  `upstream-release-cleanup=owner:1,pending:1,fd-held:1,state:1,total:4`.
- The same run still proves
  `upstream-destroy-cleanup=register:1,unregister:1,free:1,total:3`.
- The same run still proves
  `upstream-error-closeout=duplicate-acquire:1,unauthorized-release:1,duplicate-release:1,owner-disconnect:1,total:4`.

Implementation notes:

- `upstream_a2dp_object_probe.c` now stores fd/MTU ownership on the prepared
  A2DP/AVDTP stream owner.
- `upstream_transport_object_probe.c` now obtains media fd/MTU from that
  stream owner instead of returning a transport-local fixed tuple.
- `upstream_media_transport_bridge.c`, `upstream_a2dp_compat.c`, and
  `tools/firmware/sim/validate-bt-hwsim-usecases.py` now carry
  `upstream-avdtp-media-fd=...` as a required A2DP closeout gate.

Current boundary:

- This removes the previous transport-local fd provider from the A2DP closeout
  path and moves fd/MTU ownership to the prepared AVDTP stream side.
- This is still not a claim that A2DP is fully complete: the remaining hard
  boundary is replacing the probe-owned stream fd with a real L2CAP media
  socket and driving full AVDTP signaling/media setup end to end.
- After the real L2CAP socket and end-to-end signaling boundary is closed,
  A2DP can be treated as substantially closed before moving to LE Audio or
  BT/BLE NET.

## 2026-06-15 A2DP upstream lifecycle/error closeout batch

A2DP closeout advanced from individual transport lifecycle gates to a batched
upstream lifecycle/error closeout.  The validator now requires the retained
upstream `transport.c` handlers to preserve BlueZ semantics for duplicate
Acquire, unauthorized Release, duplicate Release while a Release request is in
progress, and D-Bus owner disconnect cleanup.

Validated batched surface:

- Duplicate `MediaTransport1.Acquire` is rejected without changing the active
  owner or transport state.
- Release from a non-owner sender is rejected without changing the active owner
  or transport state.
- Duplicate Release while a Release request is pending is rejected without
  clearing the pending request or changing SUSPENDING state.
- D-Bus owner disconnect runs through upstream `media_owner_exit()`, clears the
  transport owner, and returns the transport to IDLE.
- The same closeout run still proves Acquire completion, Release cleanup, and
  D-Bus unregister/free lifecycle.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-error-closeout.log`
- `FeatherCore/build/logs/build-bt2-a2dp-error-closeout.log`
- `FeatherCore/build/logs/run-a2dp-error-closeout.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-error-closeout/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-error-closeout=duplicate-acquire:1,unauthorized-release:1,duplicate-release:1,owner-disconnect:1,total:4`.
- Sink emits
  `upstream-error-closeout=duplicate-acquire:1,unauthorized-release:1,duplicate-release:1,owner-disconnect:1,total:4`.
- The same run still proves
  `upstream-acquire-complete=fd:1,mtu:1,reply:1,state:1,total:4`.
- The same run still proves
  `upstream-release-cleanup=owner:1,pending:1,fd-held:1,state:1,total:4`.
- The same run still proves
  `upstream-destroy-cleanup=register:1,unregister:1,free:1,total:3`.
- The validator now rejects A2DP closeout runs that do not expose the full
  lifecycle/error closeout batch.

Implementation notes:

- `upstream_transport_object_probe.c` now drives duplicate Acquire,
  unauthorized Release, duplicate Release, and owner disconnect through the
  retained upstream `transport.c` static handlers.
- `upstream_media_transport_bridge.c`, `upstream_a2dp_compat.c`, and
  `tools/firmware/sim/validate-bt-hwsim-usecases.py` now carry
  `upstream-error-closeout=...` as a required A2DP closeout gate.

Current boundary:

- The A2DP transport lifecycle/error surface is now much closer to upstream
  BlueZ semantics than the previous staged probe.
- This is still not a claim that A2DP is fully complete: the remaining hard
  boundary is replacing the bounded media fd provider with real L2CAP media
  socket ownership and driving full AVDTP signaling/media setup end to end.
- After that, A2DP can be treated as a substantially closed feature before
  moving on to LE Audio or BT/BLE NET.

## 2026-06-15 A2DP upstream transport destroy cleanup closeout

A2DP closeout advanced from Release owner/request cleanup to the upstream
transport D-Bus unregister/free lifecycle.  The closeout validator now
requires evidence that `media_transport_create()` registers the transport
interface with a destroy callback, `media_transport_destroy()` unregisters the
interface, and the unregister path calls the upstream `media_transport_free()`
callback.

Validated destroy surface:

- `media_transport_create()` reaches `g_dbus_register_interface()`.
- The registered user data is the upstream-created media transport.
- The registered destroy callback is retained by the D-Bus shim.
- `media_transport_destroy()` reaches `g_dbus_unregister_interface()`.
- The unregister shim invokes the retained upstream destroy callback.
- The destroy callback frees through `media_transport_free()`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-destroy-cleanup.log`
- `FeatherCore/build/logs/build-bt2-a2dp-destroy-cleanup.log`
- `FeatherCore/build/logs/run-a2dp-destroy-cleanup.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-destroy-cleanup/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-destroy-cleanup=register:1,unregister:1,free:1,total:3`.
- Sink emits
  `upstream-destroy-cleanup=register:1,unregister:1,free:1,total:3`.
- The same run still proves
  `upstream-release-cleanup=owner:1,pending:1,fd-held:1,state:1,total:4`.
- The validator now rejects A2DP closeout runs that do not expose transport
  register, unregister, and free callback evidence.

Implementation notes:

- `upstream_transport_object_probe.c` now wraps
  `g_dbus_register_interface()` and `g_dbus_unregister_interface()` for the
  transport compile unit so the upstream destroy callback can be retained and
  invoked in the probe.
- `upstream_media_transport_bridge.c`, `upstream_a2dp_compat.c`, and
  `tools/firmware/sim/validate-bt-hwsim-usecases.py` now carry
  `upstream-destroy-cleanup=...` as a required A2DP closeout gate.

Current boundary:

- This proves upstream transport unregister/free lifecycle through the D-Bus
  destroy callback.
- The imported upstream `media_transport_free()` path does not explicitly
  close the media fd; real fd lifetime still depends on replacing the bounded
  fd provider with real L2CAP media socket ownership.
- The remaining A2DP convergence targets are real L2CAP media fd ownership,
  full AVDTP signaling-driven media setup, duplicate Acquire/Release error
  handling, owner disconnect cleanup, and full bluetoothd audio plugin
  mainloop/D-Bus ownership.

## 2026-06-15 A2DP upstream Release cleanup closeout

A2DP closeout advanced from upstream `MediaTransport1.Acquire` completion to
the paired `MediaTransport1.Release` cleanup path.  The closeout validator now
requires evidence that the retained upstream `transport.c:release()` handler
runs after Acquire completion, creates the Release pending request, enters the
A2DP suspend path, completes through `a2dp_suspend_complete()`, clears the
owner/request lifecycle, and returns the transport to IDLE.

Validated Release surface:

- The Release handler runs on the same upstream-created
  `media_transport_create()` transport used by Acquire.
- The transport is ACTIVE before Release is invoked.
- The prepared AVDTP stream is advanced to STREAMING before Release so
  upstream `a2dp_suspend()` accepts the suspend request.
- Release creates a pending media request for sender `:a2dp.test`.
- Release transitions the transport through SUSPENDING.
- `a2dp_suspend_complete()` clears the owner/request lifecycle.
- The transport returns to IDLE.
- The media fd remains held by the transport after Release, matching the
  current upstream `transport.c` ownership boundary; fd close/destroy is a
  separate follow-up gate.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-release-cleanup.log`
- `FeatherCore/build/logs/build-bt2-a2dp-release-cleanup.log`
- `FeatherCore/build/logs/run-a2dp-release-cleanup.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-release-cleanup/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-release-cleanup=owner:1,pending:1,fd-held:1,state:1,total:4`.
- Sink emits
  `upstream-release-cleanup=owner:1,pending:1,fd-held:1,state:1,total:4`.
- The same run still proves
  `upstream-acquire-complete=fd:1,mtu:1,reply:1,state:1,total:4`.
- The validator now rejects A2DP closeout runs that do not expose Release
  owner cleanup, Release pending cleanup, fd-held ownership, and IDLE state.

Implementation notes:

- `upstream_a2dp_object_probe.c` now tracks the prepared AVDTP stream state
  and can advance it to STREAMING for Release/suspend validation.
- `upstream_transport_object_probe.c` now drives
  `Acquire -> a2dp_resume_complete() -> Release -> a2dp_suspend_complete()`
  through retained upstream `transport.c` handlers.
- `upstream_media_transport_bridge.c`, `upstream_a2dp_compat.c`, and
  `tools/firmware/sim/validate-bt-hwsim-usecases.py` now carry
  `upstream-release-cleanup=...` as a required A2DP closeout gate.

Current boundary:

- This proves upstream Release owner/request cleanup and IDLE transition for
  the A2DP hwsim probe.
- The fd is intentionally checked as still held after Release because that is
  what the imported upstream `transport.c` path does at this point.
- The remaining A2DP convergence targets are real L2CAP media socket fd
  ownership, explicit fd close/destroy lifecycle, duplicate Release/Acquire
  error handling, owner disconnect cleanup, and full daemon/mainloop D-Bus
  ownership.

## 2026-06-15 A2DP upstream Acquire fd/MTU completion closeout

A2DP closeout advanced from upstream resume-backed `MediaTransport1.Acquire`
request creation to the upstream completion path.  The closeout validator now
requires evidence that the retained upstream `transport.c:acquire()` body can
enter `a2dp_resume()`, keep the pending owner/request, complete through
`a2dp_resume_complete()`, install the media fd/MTU values, emit the D-Bus
Acquire reply, clear the pending request, and move the transport to ACTIVE.

Validated completion surface:

- The `Acquire` handler still runs against the upstream-created
  `media_transport_create()` transport.
- The handler creates the BlueZ owner/request state before completion.
- The prepared upstream AVDTP session/stream is used by the A2DP transport.
- The upstream completion path calls the stream transport fd/MTU provider.
- The completion path installs fd `51`, input MTU `672`, and output MTU `672`
  on the media transport in the bounded hwsim probe.
- The completion path sends a Unix-fd D-Bus reply.
- The pending request is cleared and the transport reaches ACTIVE state.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-acquire-complete.log`
- `FeatherCore/build/logs/build-bt2-a2dp-acquire-complete.log`
- `FeatherCore/build/logs/run-a2dp-acquire-complete.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-acquire-complete/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-acquire-complete=fd:1,mtu:1,reply:1,state:1,total:4`.
- Sink emits
  `upstream-acquire-complete=fd:1,mtu:1,reply:1,state:1,total:4`.
- The same run still proves
  `upstream-bounded-acquire=transport:1,owner:1,request:1,state:1,total:4`.
- The same run still proves
  `upstream-resume-prepare=endpoint:1,session:1,stream:1,total:3`.
- The validator now rejects A2DP closeout runs that do not expose fd/MTU
  completion, D-Bus reply, and ACTIVE state evidence.

Implementation notes:

- `upstream_transport_object_probe.c` now drives the static upstream
  `a2dp_resume_complete()` path after the bounded `Acquire` request is live.
- The transport probe now binds `avdtp_stream_get_transport()` to a bounded
  media fd/MTU provider so the upstream completion body can exercise
  `media_transport_set_fd()`.
- The transport probe now counts `g_dbus_send_reply()` Unix-fd replies so the
  gate proves reply emission instead of only state mutation.
- `upstream_media_transport_bridge.c`, `upstream_a2dp_compat.c`, and
  `tools/firmware/sim/validate-bt-hwsim-usecases.py` now carry
  `upstream-acquire-complete=...` as a required A2DP closeout gate.

Current boundary:

- This proves upstream `MediaTransport1.Acquire` request completion semantics
  through fd/MTU reply and ACTIVE state in the hwsim A2DP probe.
- The fd provider is still bounded by the probe; it is not yet the final real
  L2CAP media socket ownership path.
- It is still not a claim that the whole Linux Bluetooth stack, or even full
  A2DP, is complete.
- The next A2DP convergence target is replacing the bounded media fd provider
  with real L2CAP media fd ownership, then closing release, suspend, close,
  duplicate request, owner disconnect, and error cleanup behavior.

## 2026-06-15 A2DP upstream resume-backed MediaTransport1 Acquire closeout

A2DP closeout advanced from a bounded `MediaTransport1.Acquire` handler
invocation with a probe-local resume handoff to a real upstream
AVDTP stream-backed `a2dp_resume()` path.  The closeout validator now requires
evidence that the retained upstream `transport.c:acquire()` body runs against
the created transport, locks the upstream A2DP SEP through `a2dp_sep_lock()`,
enters `a2dp_resume()`, and establishes the normal BlueZ owner/request state.

Validated Acquire surface:

- The transport used by `Acquire` comes from upstream
  `media_transport_create()`.
- The upstream `acquire()` handler is invoked with a controlled DBus message
  carrying sender `:a2dp.test` and member `Acquire`.
- The handler creates a transport owner.
- The handler creates a pending request tied to the Acquire message.
- The transport enters the requesting/active side of the upstream state model.
- The A2DP transport now carries the prepared AVDTP session used by the
  upstream SEP stream.
- `transport.c` now binds `a2dp_sep_lock()` and `a2dp_sep_unlock()` to the
  imported upstream A2DP object implementation instead of falling through an
  unbound symbol path.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-resume-backed-acquire-clean.log`
- `FeatherCore/build/logs/build-bt2-a2dp-resume-backed-acquire-clean.log`
- `FeatherCore/build/logs/run-a2dp-resume-backed-acquire-clean.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-resume-backed-acquire-clean/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-bounded-acquire=transport:1,owner:1,request:1,state:1,total:4`.
- Sink emits
  `upstream-bounded-acquire=transport:1,owner:1,request:1,state:1,total:4`.
- The same run still proves
  `upstream-transport-create=endpoint:1,device-service:1,transport:1,total:3`.
- The same run also proves
  `upstream-resume-prepare=endpoint:1,session:1,stream:1,total:3`.
- The validator now rejects A2DP closeout runs that do not expose the bounded
  Acquire owner/request/state evidence.

Implementation notes:

- `upstream_object_shims/dbus/dbus.h` now carries a DBus member string so
  upstream request logging/lifecycle can identify `Acquire`.
- `upstream_transport_object_probe.c` now invokes the upstream `acquire()`
  handler on the transport created from the registered endpoint.
- `upstream_a2dp_object_probe.c` now prepares a real upstream AVDTP session,
  A2DP stream, and A2DP setup before the bounded Acquire invocation.
- `upstream_transport_object_probe.c` now copies that prepared session into
  `struct a2dp_transport` so `transport_a2dp_resume()` uses the same stream
  ownership graph.
- The previous probe-local resume handoff has been removed from this gate.
- `upstream_media_transport_bridge.c`, `upstream_a2dp_compat.c`, and
  `tools/firmware/sim/validate-bt-hwsim-usecases.py` now carry
  `upstream-bounded-acquire=...` as a required A2DP closeout gate.

Current boundary:

- This proves bounded upstream `MediaTransport1.Acquire` ownership through a
  real AVDTP stream-backed `a2dp_resume()` request id.
- It is still not a claim that the whole Linux Bluetooth stack is complete.
- The next A2DP convergence target is fd/MTU reply handoff plus full
  start/suspend/close, release cleanup, duplicate/error lifecycle, and removal
  of the remaining staged daemon/object boundaries.

## 2026-06-15 A2DP upstream media transport create closeout

A2DP closeout advanced from registered endpoint + SEP ownership to an upstream
`media_transport_create()` path.  The closeout validator now requires evidence
that the transport object is created from the registered upstream endpoint and
a device/service-backed A2DP transport initialization path.

Validated transport surface:

- The endpoint used by `media_transport_create()` comes from the bounded
  upstream `Media1.RegisterEndpoint` path.
- The endpoint has a non-NULL upstream A2DP SEP before transport creation.
- The transport is created by upstream `transport.c:media_transport_create()`.
- The transport owns the expected endpoint, device identity, D-Bus object path,
  A2DP ops table, A2DP private data, and initial `fd == -1` state.
- The device shim now returns a non-NULL service identity so the upstream
  source/sink transport init path can progress instead of failing at
  `btd_device_get_service()`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-transport-create.log`
- `FeatherCore/build/logs/build-bt2-a2dp-transport-create.log`
- `FeatherCore/build/logs/run-a2dp-transport-create.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-transport-create/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-transport-create=endpoint:1,device-service:1,transport:1,total:3`.
- Sink emits
  `upstream-transport-create=endpoint:1,device-service:1,transport:1,total:3`.
- The same run still proves
  `upstream-registered-endpoint=adapter:1,endpoint:1,sep:1,total:3`.
- The validator now rejects A2DP closeout runs that do not expose the upstream
  transport create evidence.

Implementation notes:

- `upstream_media_object_probe.c` now exposes a persistent registered A2DP
  endpoint provider for transport-side validation.
- `upstream_transport_object_probe.c` now calls upstream
  `media_transport_create()` using that registered endpoint.
- The device and GLib shims now provide the minimal service identity,
  `g_slist_length()`, and `g_strdup_printf()` behavior needed by the upstream
  transport creation path.
- `upstream_media_transport_bridge.c`, `upstream_a2dp_compat.c`, and
  `tools/firmware/sim/validate-bt-hwsim-usecases.py` now carry
  `upstream-transport-create=...` as a required A2DP closeout gate.

Current boundary:

- This proves upstream transport object creation, but not yet
  `MediaTransport1.Acquire`.
- The next A2DP convergence target is bounded `Acquire` over this created
  transport, including A2DP resume handoff, fd/MTU reply, owner/request
  lifecycle, start/suspend/close, and cleanup.

## 2026-06-15 A2DP upstream registered endpoint SEP closeout

A2DP closeout advanced from bounded `Media1.RegisterEndpoint` invocation to
registered endpoint ownership with a real upstream A2DP SEP.  The closeout
validator now requires evidence that the upstream handler did more than append
an endpoint object: it must also pass through `a2dp_add_sep()` far enough for
`media_endpoint_get_sep()` to return a non-NULL SEP.

Validated ownership surface:

- The bounded `RegisterEndpoint` path still executes the retained upstream
  `media.c:register_endpoint()` body.
- The resulting `struct media_adapter` owns an endpoint list entry.
- The resulting `struct media_endpoint` is owned by the adapter.
- The endpoint has a non-NULL upstream A2DP SEP, proving that registration
  crossed the `media.c -> a2dp.c` object boundary.
- The endpoint UUID remains the A2DP source UUID used by the controlled DBus
  message shape.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-registered-endpoint-sep.log`
- `FeatherCore/build/logs/build-bt2-a2dp-registered-endpoint-sep.log`
- `FeatherCore/build/logs/run-a2dp-registered-endpoint-sep.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-registered-endpoint-sep/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-registered-endpoint=adapter:1,endpoint:1,sep:1,total:3`.
- Sink emits
  `upstream-registered-endpoint=adapter:1,endpoint:1,sep:1,total:3`.
- The same run still proves
  `upstream-bounded-invocation=media-register-endpoint:1,total:1`.
- The validator now rejects A2DP closeout runs that do not expose the
  registered endpoint + SEP evidence.

Implementation notes:

- `upstream_media_object_probe.c` now exposes
  `bluez_upstream_media_register_endpoint_handler_registered_endpoint_ready()`.
- `upstream_media_transport_bridge.c` and `upstream_a2dp_compat.c` carry and
  print `upstream-registered-endpoint=...` as a required closeout gate.
- `tools/firmware/sim/validate-bt-hwsim-usecases.py` requires the SEP gate for
  both bt1/source and bt2/sink in the A2DP closeout case.

Current boundary:

- This proves the registered endpoint has crossed into upstream A2DP SEP
  ownership, but it still does not create a device/service-backed
  `MediaTransport1` object.
- The next A2DP convergence target is device/service-backed
  `media_transport_create()`, then bounded `MediaTransport1.Acquire` through
  A2DP resume, fd/MTU reply, start/suspend/close, and cleanup.

## 2026-06-15 A2DP upstream bounded RegisterEndpoint invocation closeout

A2DP closeout advanced from minimal real object ownership preflight to a
bounded upstream `Media1.RegisterEndpoint` handler invocation.  The probe now
executes the retained upstream `media.c:register_endpoint()` body with a
controlled DBus message shape and a real `struct media_adapter` instance, after
first running the upstream A2DP adapter profile probes needed to create the
matching A2DP server state.

Validated invocation surface:

- `Media1.RegisterEndpoint` is no longer readiness-only for this gate.
- The DBus shim now models the object path plus `UUID`, `Codec`, and
  `Capabilities` properties consumed by upstream `parse_properties()`.
- The bounded path runs upstream A2DP source/sink adapter probes before the
  endpoint registration attempt, matching the BlueZ daemon ordering where A2DP
  servers exist before application endpoints are registered.
- The endpoint registration succeeds by passing through upstream
  `media_endpoint_create()`, `endpoint_init_a2dp_source()`, and `a2dp_add_sep()`
  far enough for `adapter.endpoints` to receive the registered endpoint.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-bounded-register-endpoint-profile-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-bounded-register-endpoint-profile-probe.log`
- `FeatherCore/build/logs/run-a2dp-bounded-register-endpoint-profile-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-bounded-register-endpoint-profile-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-bounded-invocation=media-register-endpoint:1,total:1`.
- Sink emits
  `upstream-bounded-invocation=media-register-endpoint:1,total:1`.
- The same run still proves the prior gates:
  `upstream-minimal-real-objects=transport-acquire:1,media-register-endpoint:1,total:2`,
  `upstream-controlled-live-invocation-ready=transport-acquire:1,media-register-endpoint:1,total:2`,
  and
  `upstream-live-body-retained=transport-acquire:1,media-register-endpoint:1,total:2`.

Implementation notes:

- `upstream_a2dp_object_probe.c` now exposes
  `bluez_upstream_a2dp_object_adapter_profiles_ready()`, which runs the
  upstream A2DP source/sink adapter probe bodies and verifies server readiness.
- The A2DP probe boundary accepts SDP service records with a local record
  handle shim so `a2dp_add_sep()` can progress without dereferencing an
  incomplete daemon `struct btd_adapter` object.
- `upstream_media_object_probe.c` now calls the A2DP profile readiness gate
  before invoking upstream `register_endpoint()`.

Current boundary:

- This is a real bounded upstream `RegisterEndpoint` handler invocation, but it
  is not yet a full daemon-owned A2DP session.
- The current adapter identity is still a probe boundary identity, not the
  complete upstream `src/adapter.c` daemon object lifecycle.
- The next A2DP convergence target is to replace the SDP/adapter probe shim
  with real daemon adapter ownership, then connect the registered endpoint to
  full AVDTP signaling, transport export, `MediaTransport1.Acquire`,
  start/suspend/close, codec policy, and lifecycle/error cleanup.

## 2026-06-15 A2DP upstream minimal real object ownership preflight closeout

A2DP closeout advanced from non-null controlled invocation readiness to a
minimal real object ownership preflight.  The readiness gate no longer depends
only on placeholder pointer values: the probe now allocates real storage for
the minimum BlueZ/DBus objects needed before bounded handler invocation can be
attempted.

Validated ownership surfaces:

- `MediaTransport1.Acquire` preflight owns real stack storage for
  `DBusConnection`, `DBusMessage`, `struct media_transport`,
  `struct media_owner`, and `struct media_request`.
- The transport-side ownership chain verifies
  `media_transport -> media_owner -> media_request -> DBusMessage`.
- `Media1.RegisterEndpoint` preflight owns real stack storage for
  `DBusConnection`, `DBusMessage`, `struct media_adapter`, and
  `struct media_endpoint`.
- The media-side ownership chain verifies
  `media_endpoint -> media_adapter`, endpoint sender/path/UUID/codec, and
  endpoint capabilities storage.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-minimal-real-objects.log`
- `FeatherCore/build/logs/build-bt2-a2dp-minimal-real-objects.log`
- `FeatherCore/build/logs/run-a2dp-minimal-real-objects.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-minimal-real-objects/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-minimal-real-objects=transport-acquire:1,media-register-endpoint:1,total:2`.
- Sink emits
  `upstream-minimal-real-objects=transport-acquire:1,media-register-endpoint:1,total:2`.
- The same run still proves the previous gates:
  `upstream-controlled-live-invocation-ready=transport-acquire:1,media-register-endpoint:1,total:2`
  and
  `upstream-live-body-retained=transport-acquire:1,media-register-endpoint:1,total:2`.
- The validator now rejects A2DP closeout runs that do not expose the minimal
  real object ownership evidence.

Implementation notes:

- `upstream_object_shims/dbus/dbus.h` now defines minimal complete
  `DBusConnection`, `DBusMessage`, and `DBusPendingCall` storage types instead
  of opaque-only typedefs.
- `upstream_transport_object_probe.c` now exposes
  `bluez_upstream_transport_acquire_handler_minimal_real_objects_ready()`.
- `upstream_media_object_probe.c` now exposes
  `bluez_upstream_media_register_endpoint_handler_minimal_real_objects_ready()`.
- `upstream_media_transport_bridge.c` and `upstream_a2dp_compat.c` carry and
  print `upstream-minimal-real-objects=...` as a required closeout gate.

Current boundary:

- This still does not execute the upstream handlers.  It proves real object
  storage and ownership preconditions, which is the last safe step before
  bounded handler invocation.
- `struct media_endpoint` is opaque from the `transport.c` compile unit, so
  transport-side preflight verifies transport/owner/request/message ownership;
  media-side preflight verifies the concrete endpoint/adapter relationship.
- The next A2DP convergence target is bounded upstream
  `Media1.RegisterEndpoint` invocation with a controlled DBus message shape,
  followed by `MediaTransport1.Acquire` once transport ownership can be backed
  by a real endpoint-created transport.

## 2026-06-15 A2DP upstream controlled live invocation readiness closeout

A2DP closeout advanced from live upstream handler-body retention to the first
controlled live invocation readiness gate.  The bridge now verifies that the
retained upstream `MediaTransport1.Acquire` and `Media1.RegisterEndpoint`
handlers have the minimum non-null live invocation inputs needed before a real
call can be safely attempted:

- retained upstream handler body,
- non-null `DBusConnection *`,
- non-null `DBusMessage *`,
- non-null typed BlueZ object as `user_data`,
- matching typed `user_data` ownership for `struct media_transport *` and
  `struct media_adapter *`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-controlled-live-ready.log`
- `FeatherCore/build/logs/build-bt2-a2dp-controlled-live-ready.log`
- `FeatherCore/build/logs/run-a2dp-controlled-live-ready.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-controlled-live-ready/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-controlled-live-invocation-ready=transport-acquire:1,media-register-endpoint:1,total:2`.
- Sink emits
  `upstream-controlled-live-invocation-ready=transport-acquire:1,media-register-endpoint:1,total:2`.
- The same run still proves the prior live-retention gate:
  `upstream-live-body-retained=transport-acquire:1,media-register-endpoint:1,total:2`.
- The validator now rejects A2DP closeout runs that do not expose the
  controlled live invocation readiness evidence.

Implementation notes:

- `upstream_transport_object_probe.c` now exposes
  `bluez_upstream_transport_acquire_handler_controlled_invocation_ready()`.
- `upstream_media_object_probe.c` now exposes
  `bluez_upstream_media_register_endpoint_handler_controlled_invocation_ready()`.
- `upstream_media_transport_bridge.c` carries the two readiness gates into the
  common A2DP bridge result.
- `upstream_a2dp_compat.c` prints
  `upstream-controlled-live-invocation-ready=...` alongside the existing
  dispatch, handoff, dependency, and live-retention evidence.

Current boundary:

- This still deliberately does not call the upstream handlers with fake
  objects.  It proves the retained handler and typed input ownership preflight
  needed before a real controlled invocation path can be introduced.
- The next A2DP convergence target is to replace the readiness-only fake
  pointers with minimal real `DBusMessage`, `media_adapter`,
  `media_endpoint`, and `media_transport` ownership objects, then execute a
  bounded upstream `RegisterEndpoint` path before attempting
  `MediaTransport1.Acquire`.

## 2026-06-15 A2DP upstream live handler-body retention closeout

A2DP closeout advanced from dependency-readiness gating to actual live
upstream handler-body retention for the first transport/media entry points.
The retained upstream bodies are now linkable in both bt1/source and bt2/sink
sim builds:

- `MediaTransport1.Acquire` retains the upstream `transport.c` handler body.
- `Media1.RegisterEndpoint` retains the upstream `media.c` handler body.
- The A2DP object graph now links through imported upstream `a2dp.c`,
  `sink.c`, `source.c`, `avdtp.c`, `adapter.c`, `device.c`, `service.c`,
  `storage.c`, `textfile.c`, `eir.c`, and `shared/ad.c` surfaces needed by
  the retained handler set.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-live-handler-retention.log`
- `FeatherCore/build/logs/build-bt2-a2dp-live-handler-retention.log`
- `FeatherCore/build/logs/run-a2dp-live-handler-retention.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-live-handler-retention/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-live-body-retained=transport-acquire:1,media-register-endpoint:1,total:2`.
- Sink emits
  `upstream-live-body-retained=transport-acquire:1,media-register-endpoint:1,total:2`.
- Source and sink also retain the previous dependency gate:
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:1,adapter-device:1,glib-dbus:1,ready:4,required:4`.
- The validator now rejects A2DP closeout runs that do not expose the live
  handler-body retention evidence.

Implementation notes:

- `upstream_a2dp_object_probe.c` now aliases upstream `a2dp.c` calls to the
  imported sink/source/adapter object symbols for stream creation and adapter
  service registration.
- `upstream_sink_object_probe.c` and `upstream_source_object_probe.c` now bind
  their AVDTP stream callback calls to the imported upstream `avdtp.c` object.
- `upstream_adapter_object_probe.c`, `upstream_agent_object_probe.c`, and
  `upstream_device_object_probe.c` now route cross-object calls into the
  already imported upstream adapter/device/service/dbus-common objects.
- The daemon build now imports additional upstream support objects:
  `src/textfile.c`, `src/eir.c`, and `src/shared/ad.c`.
- The compat boundary was extended only where full objects would currently
  explode the dependency graph: typed UUID/address/endian/iovec helpers and
  weak daemon subsystem placeholders for GATT client/database, bearer, set,
  advertising monitor, and assertion hooks.

Current boundary:

- This is stronger than the previous staged probe: the first A2DP upstream
  handler bodies are retained by the final NuttX sim link and validated under
  hwsim.
- This is still not the final complete A2DP claim.  The next A2DP convergence
  target is controlled live invocation with real enough `DBusMessage`,
  `media_adapter`, `media_endpoint`, and `media_transport` ownership instead
  of only retaining the handler bodies.
- The broader Linux Bluetooth stack remains incomplete until the weak daemon
  subsystem placeholders are replaced by imported upstream objects or
  semantically equivalent NuttX implementations and then verified through
  scan/connect/auth/A2DP/LE Audio/BT NET/BLE NET hwsim flows.

## 2026-06-15 A2DP upstream live body GLib/DBus dependency closeout

A2DP closeout advanced the live upstream handler-body dependency closure from
three ready classes to all four ready classes.  The GLib/DBus shim coverage now
includes the helper surface that was exposed when upstream A2DP/audio handler
bodies were retained: UTF-8 validation, hash-table add/destroy, key-file group
and list helpers, string concat/split/delimit helpers, list removal, and DBus
iterator element-type inspection.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-live-body-deps-glib-dbus.log`
- `FeatherCore/build/logs/build-bt2-a2dp-live-body-deps-glib-dbus.log`
- `FeatherCore/build/logs/run-a2dp-live-body-deps-glib-dbus.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-live-body-deps-glib-dbus/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:1,adapter-device:1,glib-dbus:1,ready:4,required:4`.
- Sink emits
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:1,adapter-device:1,glib-dbus:1,ready:4,required:4`.
- The validator now rejects A2DP closeout runs that do not expose all four
  live handler-body dependency classes.

Implementation notes:

- `upstream_object_shims/glib.h` now covers the GLib helpers required by the
  currently retained upstream A2DP/audio body set:
  `g_utf8_validate`, `g_hash_table_add`, `g_hash_table_destroy`,
  `g_key_file_has_group`, `g_key_file_set_uint64`,
  `g_key_file_get_string_list`, `g_strconcat`, `g_strdelimit`,
  `g_strsplit`, and `g_list_remove`.
- `upstream_object_shims/dbus/dbus.h` now covers
  `dbus_message_iter_get_element_type`.
- `upstream_media_transport_bridge.c` records GLib/DBus helper readiness as
  the fourth live-body dependency class.

Current boundary:

- All four live handler-body dependency classes are now ready:
  MediaEndpoint/MediaTransport cross-object helpers, AVDTP control helpers,
  adapter/device helpers, and GLib/DBus helpers.
- This is a prerequisite for live upstream handler-body retention, not yet a
  claim that `Acquire` or `RegisterEndpoint` are executing the full upstream
  handler bodies with real D-Bus messages and owned objects.
- The next A2DP convergence target is to retry live upstream handler-body
  retention and compare the new unresolved-symbol set against the previous
  failure.  If no new large class appears, the bridge can move from typed
  handoff to controlled live invocation for `RegisterEndpoint`, then
  `Acquire`.

## 2026-06-15 A2DP upstream live body adapter/device dependency closeout

A2DP closeout advanced the live upstream handler-body dependency closure from
two ready classes to three ready classes.  The imported upstream `adapter.c`
and `device.c` objects now expose the adapter/service/device helper surface
needed by retained upstream A2DP/audio handler bodies.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-live-body-deps-adapter-device.log`
- `FeatherCore/build/logs/build-bt2-a2dp-live-body-deps-adapter-device.log`
- `FeatherCore/build/logs/run-a2dp-live-body-deps-adapter-device.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-live-body-deps-adapter-device/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:1,adapter-device:1,glib-dbus:0,ready:3,required:4`.
- Sink emits
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:1,adapter-device:1,glib-dbus:0,ready:3,required:4`.
- The validator now rejects A2DP closeout runs that do not expose the
  adapter/device helper class.

Implementation notes:

- `upstream_adapter_object_probe.c` now checks the imported upstream adapter
  helper surface required by live A2DP body retention: adapter path lookup,
  service add/remove, and adapter device lookup.
- `upstream_device_object_probe.c` now checks the imported upstream device
  helper surface required by live audio/adapter body retention: device path,
  adapter lookup, UUID/profile helpers, connection add, and last-seen update.
- The check remains non-live and compile-time typed so the build can verify
  helper availability before retaining the full handler bodies.

Current boundary:

- Three of four live handler-body dependency classes are now ready:
  MediaEndpoint/MediaTransport cross-object helpers, AVDTP control helpers,
  and adapter/device helpers.
- Full live upstream A2DP handler invocation still requires closing GLib/DBus
  helper coverage.
- The next A2DP convergence target is the GLib/DBus helper class; after that,
  live upstream handler body retention should be retried against the actual
  unresolved-symbol set.

## 2026-06-15 A2DP upstream live body AVDTP control dependency closeout

A2DP closeout advanced the live upstream handler-body dependency closure from
one ready class to two ready classes.  The imported upstream `avdtp.c` object
now exposes the AVDTP control helper surface required by retained upstream
`a2dp.c` handler bodies: stream callback registration, open, SEP
registration/unregistration, version lookup, and discovery.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-live-body-deps-avdtp.log`
- `FeatherCore/build/logs/build-bt2-a2dp-live-body-deps-avdtp.log`
- `FeatherCore/build/logs/run-a2dp-live-body-deps-avdtp.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-live-body-deps-avdtp/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:1,adapter-device:0,glib-dbus:0,ready:2,required:4`.
- Sink emits
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:1,adapter-device:0,glib-dbus:0,ready:2,required:4`.
- The validator now rejects A2DP closeout runs that do not expose the AVDTP
  control helper class.

Implementation notes:

- `upstream_avdtp_object_probe.c` now maps the AVDTP helper names needed by
  imported upstream `a2dp.c` into the imported upstream `avdtp.c` object:
  `avdtp_stream_add_cb`, `avdtp_open`, `avdtp_unregister_sep`,
  `avdtp_register_sep`, `avdtp_get_version`, and `avdtp_discover`.
- The dependency check is intentionally non-live and compile-time typed, so it
  verifies symbol/name readiness without prematurely retaining the full AVDTP
  function bodies and their remaining runtime dependencies.

Current boundary:

- Two of four live handler-body dependency classes are now ready:
  MediaEndpoint/MediaTransport cross-object helpers and AVDTP control helpers.
- Full live upstream A2DP handler invocation still requires closing the
  adapter/device helper shim class and GLib/DBus helper coverage.
- The next A2DP convergence target is the adapter/device helper class, because
  live `a2dp.c` and `adapter.c` retention still exposes profile/service and
  device helper dependencies.

## 2026-06-15 A2DP upstream live body cross-object dependency closeout

A2DP closeout advanced from typed invocation handoff to the first live upstream
handler-body dependency closure gate.  The imported upstream `media.c` object
now maps its MediaEndpoint-side calls to the imported upstream `transport.c`
object helpers for `media_transport_get_stream` and
`media_transport_update_delay`, removing the first cross-object dependency
class exposed when live handler bodies are retained.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-live-body-deps-cross-object.log`
- `FeatherCore/build/logs/build-bt2-a2dp-live-body-deps-cross-object.log`
- `FeatherCore/build/logs/run-a2dp-live-body-deps-cross-object.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-live-body-deps-cross-object/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:0,adapter-device:0,glib-dbus:0,ready:1,required:4`.
- Sink emits
  `upstream-live-body-deps=media-transport-cross-object:1,avdtp-control:0,adapter-device:0,glib-dbus:0,ready:1,required:4`.
- The validator now rejects A2DP closeout runs that do not bind the
  MediaEndpoint-to-MediaTransport cross-object helpers.

Implementation notes:

- `upstream_media_object_probe.c` now aliases
  `media_transport_get_stream` and `media_transport_update_delay` to the
  imported upstream `transport.c` object symbols.
- `upstream_media_transport_bridge.c` records this dependency class as the
  first ready live-body dependency class.
- This directly addresses the `media.c` unresolved helper failures exposed by
  the live handler-body retention experiment.

Current boundary:

- One of four live handler-body dependency classes is now ready:
  MediaEndpoint/MediaTransport cross-object helpers.
- Full live upstream handler invocation still requires closing the remaining
  classes: AVDTP control helpers, adapter/device helper shims, and GLib/DBus
  helper coverage.
- The next A2DP convergence target is the AVDTP control dependency class,
  because it gates `Acquire`/A2DP setup progression after the transport/media
  cross-object helpers are resolved.

## 2026-06-15 A2DP upstream typed invocation handoff closeout

A2DP closeout advanced from upstream-bound dispatch selection to typed
invocation handoff evidence for the first MediaTransport1 and Media1 endpoint
paths.  The bridge now verifies the BlueZ-native handler ABI shape
`DBusConnection *`, `DBusMessage *`, and `void *user_data`, with
`MediaTransport1.Acquire` mapped to `struct media_transport *` and
`Media1.RegisterEndpoint` mapped to `struct media_adapter *`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-invocation-handoff.log`
- `FeatherCore/build/logs/build-bt2-a2dp-invocation-handoff.log`
- `FeatherCore/build/logs/run-a2dp-invocation-handoff.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-invocation-handoff/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-invocation-handoff=transport-acquire:1,media-register-endpoint:1,total:2`.
- Sink emits
  `upstream-invocation-handoff=transport-acquire:1,media-register-endpoint:1,total:2`.
- Source and sink still require the seven upstream dispatch replacement gates:
  MediaTransport1 total 5 plus Media1 endpoint total 2.
- The validator now rejects A2DP closeout runs that lack the typed invocation
  handoff evidence.

Implementation notes:

- `upstream_transport_object_probe.c` checks the upstream `acquire` static
  handler against `GDBusMethodFunction` and the `struct media_transport *`
  `user_data` shape.
- `upstream_media_object_probe.c` checks the upstream `register_endpoint`
  static handler against `GDBusMethodFunction` and the `struct media_adapter *`
  `user_data` shape.
- The checks are intentionally non-live compile-time handoff checks.  A live
  function-pointer handoff pulls the upstream handler body into the final link
  and exposes unresolved upstream dependencies that still need to be ported.

Current boundary:

- The typed D-Bus/object handoff ABI is now verified for
  `MediaTransport1.Acquire` and `Media1.RegisterEndpoint`.
- Full A2DP completion still requires making the live upstream handler bodies
  linkable and callable with real `DBusMessage`, `media_adapter`,
  `media_endpoint`, and `media_transport` objects.
- The next A2DP convergence target is closing the dependencies exposed by live
  upstream handler body retention: AVDTP stream/control helpers, adapter/device
  helper shims, GLib hash/list/string helpers, DBus iterator helpers, and the
  media transport/media endpoint cross-object helpers.

## 2026-06-15 A2DP upstream Media1 endpoint dispatch closeout

A2DP closeout advanced from MediaTransport1-only dispatch replacement to the
combined MediaTransport1 plus Media1 endpoint dispatch bridge.  The active
Media1 `RegisterEndpoint` and `UnregisterEndpoint` bridge entries now route
through wrappers bound to the directly compiled upstream
`third/bluez/profiles/audio/media.c` static handler bodies, alongside the
previously replaced MediaTransport1 method group.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-media-endpoint-dispatch-object.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-endpoint-dispatch-object.log`
- `FeatherCore/build/logs/run-a2dp-media-endpoint-dispatch-object.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-endpoint-dispatch-object/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-dispatch=transport-acquire:1,transport-try-acquire:1,transport-release:1,transport-select:1,transport-unselect:1,transport-total:5,media-register-endpoint:1,media-unregister-endpoint:1,media-total:2,total:7`.
- Sink emits
  `upstream-dispatch=transport-acquire:1,transport-try-acquire:1,transport-release:1,transport-select:1,transport-unselect:1,transport-total:5,media-register-endpoint:1,media-unregister-endpoint:1,media-total:2,total:7`.
- Source and sink emit both upstream handler object gates:
  `audio/transport.c ... total:5` and `audio/media.c ... total:2`.
- The validator now rejects A2DP closeout runs that lack all seven
  MediaTransport1/Media1 upstream dispatch replacement gates.

Implementation notes:

- `upstream_media_object_probe.c` now exposes upstream-bound dispatch
  predicates for `register_endpoint` and `unregister_endpoint` from the
  imported upstream `media.c` compilation unit.
- `upstream_media_transport_bridge.c` routes the active Media1 endpoint method
  bridge entries through those upstream-bound predicates.
- The A2DP closeout log boundary is now
  `upstream-handler-families-mapped-transport-media-dispatch-linked-media-transport-c-handlers`.

Current boundary:

- MediaTransport1 and Media1 endpoint method dispatch replacement is complete
  at the upstream-bound wrapper level.
- Full A2DP completion still requires passing real `DBusMessage`,
  `media_endpoint`, and `media_transport` ownership/state into upstream handler
  invocation paths rather than only selecting the upstream static handler
  bodies.
- The next A2DP convergence target is real D-Bus/object ownership handoff into
  these upstream handlers, starting with `RegisterEndpoint` endpoint ownership
  and then `Acquire` transport ownership.

## 2026-06-15 A2DP upstream MediaTransport method dispatch closeout

A2DP closeout advanced the MediaTransport1 method dispatch bridge from a
single upstream-bound `Acquire` replacement to the full MediaTransport1 method
group.  The active bridge entries for `Acquire`, `TryAcquire`, `Release`,
`Select`, and `Unselect` now all route through wrappers that are bound to the
directly compiled upstream `third/bluez/profiles/audio/transport.c` static
handler bodies.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-transport-dispatch-object.log`
- `FeatherCore/build/logs/build-bt2-a2dp-transport-dispatch-object.log`
- `FeatherCore/build/logs/run-a2dp-transport-dispatch-object.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-transport-dispatch-object/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits
  `upstream-dispatch=transport-acquire:1,transport-try-acquire:1,transport-release:1,transport-select:1,transport-unselect:1,total:5`.
- Sink emits
  `upstream-dispatch=transport-acquire:1,transport-try-acquire:1,transport-release:1,transport-select:1,transport-unselect:1,total:5`.
- Source and sink still emit
  `bluez-upstream-handler-object: audio/transport.c ... handlers=...total:5`.
- The validator now rejects A2DP closeout runs that lack all five
  MediaTransport1 upstream dispatch replacements.

Implementation notes:

- `upstream_transport_object_probe.c` now exposes upstream-bound dispatch
  predicates for `acquire`, `try_acquire`, `release`, `select_transport`, and
  `unselect_transport` from the imported upstream `transport.c` compilation
  unit.
- `upstream_media_transport_bridge.c` routes all five active transport method
  bridge entries through those upstream-bound predicates.
- The A2DP closeout log boundary is now
  `upstream-handler-families-mapped-transport-dispatch-linked-media-transport-c-handlers`.

Current boundary:

- MediaTransport1 method dispatch replacement is complete at the
  upstream-bound wrapper level.
- The wrappers currently prove and select the upstream static handler bodies;
  full real D-Bus invocation still requires carrying real `DBusMessage` and
  `media_transport` ownership into the upstream handlers.
- The next A2DP convergence target is the Media1 endpoint method group:
  `RegisterEndpoint` and `UnregisterEndpoint`.

## 2026-06-15 A2DP upstream Acquire dispatch entry closeout

A2DP closeout advanced from upstream static handler binding evidence to the
first active bridge dispatch entry replacement.  The MediaTransport1
`Acquire` bridge entry now calls an upstream-bound wrapper exported by the
directly compiled upstream `third/bluez/profiles/audio/transport.c` object,
rather than using only the generic NuttX compatibility method semantic.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-acquire-dispatch-object.log`
- `FeatherCore/build/logs/build-bt2-a2dp-acquire-dispatch-object.log`
- `FeatherCore/build/logs/run-a2dp-acquire-dispatch-object.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-acquire-dispatch-object/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `upstream-dispatch=transport-acquire:1,total:1`.
- Sink emits `upstream-dispatch=transport-acquire:1,total:1`.
- Source and sink still emit
  `bluez-upstream-handler-object: audio/transport.c ... handlers=...total:5`.
- The validator now rejects A2DP closeout runs that lack the first upstream
  dispatch replacement evidence.

Implementation notes:

- `upstream_transport_object_probe.c` now exposes
  `bluez_upstream_transport_acquire_handler_dispatch_bound()`, which proves
  the static upstream `acquire` handler from `transport.c` is bound in the
  same imported object compilation unit.
- `upstream_media_transport_bridge.c` routes the active `acquire` bridge entry
  through that upstream-bound wrapper.
- The A2DP closeout log boundary moved from handler-family mapping only to
  `upstream-handler-families-mapped-first-dispatch-linked-media-transport-c-handlers`.

Current boundary:

- This is the first active dispatch-entry replacement, not full A2DP
  completion.
- `TryAcquire`, `Release`, `Select`, `Unselect`, and the Media1 endpoint
  registration handlers still need the same upstream-bound dispatch-entry
  replacement treatment.
- The next A2DP convergence target is to replace the remaining
  MediaTransport1 method entries, then the Media1 `RegisterEndpoint` and
  `UnregisterEndpoint` entries.

## 2026-06-15 A2DP upstream handler object closeout

A2DP closeout advanced the MediaEndpoint/MediaTransport bridge from generic
handler-family mapping to direct upstream static handler binding evidence.  The
directly compiled upstream `third/bluez/profiles/audio/transport.c` object now
exports wrapper evidence for the static `Acquire`, `TryAcquire`, `Release`,
`Select`, and `Unselect` method handlers, and the directly compiled upstream
`third/bluez/profiles/audio/media.c` object exports wrapper evidence for
`RegisterEndpoint` and `UnregisterEndpoint`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-handler-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-handler-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-handler-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-handler-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-handler-object: audio/transport.c role=source linked=1`.
- Source emits `bluez-upstream-handler-object: audio/media.c role=source linked=1`.
- Sink emits `bluez-upstream-handler-object: audio/transport.c role=sink linked=1`.
- Sink emits `bluez-upstream-handler-object: audio/media.c role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack this upstream handler
  object evidence.

Implementation notes:

- `upstream_transport_object_probe.c` now binds the static upstream handler
  bodies `acquire`, `try_acquire`, `release`, `select_transport`, and
  `unselect_transport` from the same compilation unit as upstream
  `transport.c`.
- `upstream_media_object_probe.c` now binds the static upstream handler bodies
  `register_endpoint` and `unregister_endpoint` from the same compilation unit
  as upstream `media.c`.

Current boundary:

- This proves the upstream handler bodies are compiled and wrapper-bindable,
  but the active D-Bus dispatch path still uses the existing NuttX compatibility
  bridge for method invocation.
- The next A2DP convergence target is to replace one active bridge dispatch
  entry at a time with calls into these upstream-bound handler wrappers.

## 2026-06-15 A2DP upstream media owner ABI closeout

A2DP closeout advanced the MediaEndpoint/MediaTransport ownership boundary by
adding a strong `audio/media-owner` forwarder gate.  The unprefixed BlueZ
media endpoint and A2DP transport helper ABI now forwards to the directly
compiled upstream `third/bluez/profiles/audio/media.c` and
`third/bluez/profiles/audio/transport.c` object symbols instead of falling
through to weak empty stubs.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-media-owner-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-owner-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-media-owner-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-owner-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-object: audio/media-owner role=source linked=1`.
- Sink emits `bluez-upstream-object: audio/media-owner role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack the media-owner
  forwarder gate.

Implementation notes:

- `media_endpoint_get_sep`, `media_endpoint_get_uuid`,
  `media_endpoint_get_delay_reporting`, `media_endpoint_get_codec`,
  `media_endpoint_get_btd_adapter`, `media_endpoint_is_broadcast`,
  `media_transport_get_dev`, and `media_transport_set_a2dp_volume` now have
  strong forwarders into the imported upstream media/transport objects.
- The device shim now includes `btd_device_set_volume`, which is required by
  the upstream A2DP volume path in `transport.c`.

Current boundary:

- Media endpoint/transport accessors now resolve to upstream object symbols,
  but several active D-Bus method handlers still pass through the existing
  NuttX compatibility bridge.
- This narrows the A2DP weak fallback surface for MediaEndpoint and
  MediaTransport ownership, but does not claim complete unmodified upstream
  bluetoothd audio plugin execution yet.
- Next A2DP convergence target: replace individual Media1/MediaEndpoint1/
  MediaTransport1 handler bridge entries with wrappers around the directly
  linked upstream handler bodies.

## 2026-06-15 A2DP upstream SDP service linked object closeout

A2DP closeout advanced the bluetoothd SDP profile service-registration path by
adding directly compiled upstream `third/bluez/src/sdpd-database.c` and
`third/bluez/src/sdpd-service.c` object gates.  The closeout now requires
source and sink roles to expose both SDP service database and SDP service
registration linked-object evidence alongside the existing core/audio gates.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-sdpd-service-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-sdpd-service-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-sdpd-service-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-sdpd-service-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-object: src/sdpd-database.c role=source linked=1`.
- Source emits `bluez-upstream-object: src/sdpd-service.c role=source linked=1`.
- Sink emits `bluez-upstream-object: src/sdpd-database.c role=sink linked=1`.
- Sink emits `bluez-upstream-object: src/sdpd-service.c role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack either SDP linked
  object gate.

Implementation notes:

- `sdpd-database.c` and `sdpd-service.c` are compiled as separate upstream
  objects to preserve BlueZ's original per-file include behavior.
- The SDP object shim now covers the record/database primitives required by
  this owner slice, including record attributes, SDP PDU header/status values,
  service class/profile constants, big-endian helpers, and minimal
  attr/list helpers.

Current boundary:

- SDP service database and service registration are linked and validated, but
  the backing SDP server socket/request loop is still outside this gate.
- This narrows the A2DP SDP weak fallback surface, but does not claim complete
  upstream SDP daemon/socket behavior yet.
- Next A2DP convergence target: continue replacing media endpoint/device
  callback fallbacks with real upstream owner slices, then route the daemon
  path through those owners instead of staged bridge mirrors.

## 2026-06-15 A2DP upstream error linked object closeout

A2DP closeout advanced the bluetoothd D-Bus error lifecycle by adding a
directly compiled upstream `third/bluez/src/error.c` object gate.  The closeout
now requires source and sink roles to expose BlueZ `error` linked-object
evidence alongside the existing `dbus-common`, `btd_device`, `btd_service`,
`btd_adapter`, userspace `mgmt`, `storage`, and `agent` gates.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-error-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-error-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-error-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-error-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-object: src/error.c role=source linked=1`.
- Sink emits `bluez-upstream-object: src/error.c role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack the `src/error.c`
  `error` linked object gate.

Implementation notes:

- The error object gate directly compiles upstream `src/error.c`, replacing
  weak fallback coverage for `btd_error_*` helpers used by agent, device,
  service, profile, and media paths.
- Error object creation still uses the current NuttX-side `gdbus`/D-Bus object
  shim boundary for `g_dbus_create_error`.

Current boundary:

- `src/error.c` is linked and validated as part of the A2DP closeout, but full
  D-Bus message ownership and bus connection lifecycle still need to converge
  toward upstream bluetoothd.
- This narrows the A2DP D-Bus error weak fallback surface, but does not claim
  complete upstream bluetoothd daemon/mainloop/plugin ownership yet.
- Next A2DP convergence target: replace media endpoint/device callback
  fallbacks with real upstream owner slices, then route the daemon path through
  those owners instead of staged bridge mirrors.

## 2026-06-15 A2DP upstream agent linked object closeout

A2DP closeout advanced the bluetoothd authentication/authorization ownership
chain by adding a directly compiled upstream `third/bluez/src/agent.c` object
gate.  The closeout now requires source and sink roles to expose BlueZ `agent`
linked-object evidence alongside the existing `btd_device`, `btd_service`,
`btd_adapter`, userspace `mgmt`, and `storage` gates.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-agent-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-agent-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-agent-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-agent-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-object: src/agent.c role=source linked=1`.
- Sink emits `bluez-upstream-object: src/agent.c role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack the `src/agent.c`
  `agent` linked object gate.

Implementation notes:

- The agent object gate directly compiles upstream `src/agent.c` and uses
  upstream `src/shared/queue.c` for the default-agent queue path instead of
  the lightweight object shim queue.
- A weak `btd_get_dbus_connection` boundary is provided at the object gate so
  AgentManager D-Bus calls can link while the full daemon D-Bus ownership is
  still being converged.

Current boundary:

- `src/agent.c` is linked and validated as part of the A2DP closeout, but the
  active D-Bus connection is still a NuttX-side weak compatibility boundary.
- This reduces the A2DP authentication/authorization weak fallback surface, but
  does not claim complete upstream bluetoothd AgentManager behavior yet.
- Next A2DP convergence target: replace media endpoint/device callback
  fallbacks with real upstream owner slices, then route the daemon path through
  those owners instead of staged bridge mirrors.

## 2026-06-15 A2DP upstream storage linked object closeout

A2DP closeout advanced the bluetoothd persistent-state ownership chain by
adding a directly compiled upstream `third/bluez/src/storage.c` object gate.
The closeout now requires source and sink roles to expose BlueZ `storage`
linked-object evidence alongside the existing `btd_device`, `btd_service`,
`btd_adapter`, and userspace `mgmt` gates.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-storage-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-storage-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-storage-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-storage-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-object: src/storage.c role=source linked=1`.
- Sink emits `bluez-upstream-object: src/storage.c role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack the `src/storage.c`
  `storage` linked object gate.

Implementation notes:

- The storage object gate directly compiles upstream `src/storage.c`, replacing
  weak fallback coverage for `record_from_string` and `find_record_in_list`.
- The SDP object shim now provides the helpers required by this upstream owner:
  `sdp_extract_pdu`, `sdp_get_service_classes`, and service-class storage in
  `sdp_set_service_classes`.

Current boundary:

- `src/storage.c` is linked, but backing file persistence still runs through
  NuttX/BlueZ object shims such as `textfile_get`; it is not yet a complete
  BlueZ storage directory lifecycle.
- This narrows the A2DP weak fallback surface, but does not claim complete
  upstream bluetoothd audio/plugin/mainloop behavior.
- Next A2DP convergence target: continue replacing agent/media endpoint/device
  callback fallbacks with real upstream owner slices, then route the daemon
  path through those owners instead of staged bridge mirrors.

## 2026-06-15 A2DP upstream shared mgmt linked object closeout

A2DP closeout advanced the bluetoothd control-plane ownership chain by adding
a directly compiled upstream `third/bluez/src/shared/mgmt.c` object gate.  The
closeout now requires source and sink roles to expose the BlueZ userspace
`mgmt` linked-object evidence alongside the existing `btd_device`,
`btd_service`, and `btd_adapter` gates.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-mgmt-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-mgmt-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-mgmt-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-mgmt-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-object: src/shared/mgmt.c role=source linked=1`.
- Sink emits `bluez-upstream-object: src/shared/mgmt.c role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack the `src/shared/mgmt.c`
  `mgmt` linked object gate.

Implementation notes:

- The object shim now provides the Linux Bluetooth socket/HCI ABI pieces needed
  by upstream `mgmt.c`: `AF_BLUETOOTH`, `PF_BLUETOOTH`, `SOL_BLUETOOTH`,
  `BTPROTO_HCI`, `BT_SNDMTU`, `sockaddr_hci`, `HCI_DEV_NONE`,
  `HCI_CHANNEL_CONTROL`, and `HCI_MAX_ACL_SIZE`.
- The BlueZ shared queue/util shims now provide the upstream helpers used by
  `mgmt.c`, including `queue_pop_head`, `queue_push_head`,
  `queue_remove_all`, and `util_debug_va`.

Current boundary:

- `src/shared/mgmt.c` is directly compiled and linked for the object gate, but
  the actual mgmt socket transport is still backed by NuttX/hwsim compatibility
  rather than a complete native Linux `AF_BLUETOOTH` socket implementation.
- This reduces the weak fallback surface for A2DP control-plane ownership, but
  does not claim complete unmodified upstream bluetoothd audio/plugin/mainloop
  behavior yet.
- Next A2DP convergence target: continue replacing weak storage/agent/media
  endpoint/device callback fallbacks with real upstream owner slices, then
  route the daemon path through those owners instead of staged bridge mirrors.

## 2026-06-15 A2DP upstream adapter linked object closeout

A2DP closeout advanced the bluetoothd ownership chain by adding a directly
compiled upstream `third/bluez/src/adapter.c` object gate.  The closeout now
requires source and sink roles to expose `btd_adapter` linked-object evidence
alongside the existing `btd_device` and `btd_service` gates.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-adapter-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-adapter-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-adapter-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-adapter-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-object: src/adapter.c role=source linked=1`.
- Sink emits `bluez-upstream-object: src/adapter.c role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack the `adapter.c`
  `btd_adapter` linked object gate.

Implementation notes:

- The `bluetooth/mgmt.h` shim now wraps the upstream BlueZ
  `lib/bluetooth/mgmt.h` ABI instead of hand-maintaining mgmt constants.
- `upstream_device_link_stubs.c` is now a weak fallback boundary, allowing
  real upstream owner objects such as `adapter.c` to override temporary stubs
  as they are ported.

Current boundary:

- `adapter.c` is directly compiled and linked for the object gate, but several
  surrounding owners still resolve through weak temporary fallbacks:
  `mgmt_send*`, storage, agent, advertising monitor, selected device callbacks,
  byte-order helpers, and crypto helpers.
- This is a substantial A2DP upstream ownership step, not a final claim that
  unmodified upstream bluetoothd audio is complete.
- Next A2DP convergence target: replace the weak fallbacks with real
  `mgmt.c`/storage/agent/advertising/device callback ownership slices, then
  wire the daemon path to call the linked adapter/service/device objects
  directly rather than only proving linkability.

## 2026-06-15 A2DP upstream device/service linked object closeout

A2DP closeout advanced from staged ownership markers into directly linked
BlueZ core object gates for `third/bluez/src/device.c` and
`third/bluez/src/service.c`.  The daemon path now requires both source and
sink roles to expose `btd_device` and `btd_service` linked-object evidence in
the hwsim closeout, alongside the existing `profile.c`, `dbus-common.c`,
`mainloop.c`, and `io-mainloop.c` gates.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-service-object-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-service-object-probe.log`
- `FeatherCore/build/logs/run-a2dp-service-object-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-service-object-probe/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source emits `bluez-upstream-object: src/device.c role=source linked=1`
  and `bluez-upstream-object: src/service.c role=source linked=1`.
- Sink emits `bluez-upstream-object: src/device.c role=sink linked=1`
  and `bluez-upstream-object: src/service.c role=sink linked=1`.
- The validator now rejects A2DP closeout runs that lack the `service.c`
  `btd_service` linked object gate.

Current boundary:

- `service.c` is now directly compiled as an upstream object gate, but the
  active daemon path still keeps a temporary `upstream_device_link_stubs.c`
  boundary for surrounding adapter/storage/agent/GATT/media ownership.
- This is progress toward real upstream bluetoothd ownership, not a claim that
  the complete unmodified BlueZ A2DP daemon stack is finished.
- Next A2DP convergence target: replace the temporary service/device-facing
  stubs with real `adapter.c`/storage/agent/media endpoint ownership slices,
  then continue shrinking the staged daemon bridge.

## 2026-06-14 A2DP upstream AVDTP session flow closeout

A2DP closeout added a gate for an upstream-shaped AVDTP/MediaTransport session
flow.  The bridge now validates endpoint select, set configuration,
open/start, suspend/close, MediaTransport binding, A2DP resume/cancel id
cleanup, and final transport detach.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-session-flow.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-session-flow.log`
- `FeatherCore/build/logs/run-a2dp-upstream-session-flow.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-session-flow/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-a2dp-session=select:1,set-config:1,open-start:1,suspend-close:1,total:4 ... final-ok=1`.
- The validator now rejects an A2DP bridge that lacks upstream-shaped
  AVDTP/MediaTransport session flow evidence.

Current boundary:

- AVDTP/MediaTransport session flow is represented by NuttX-side
  upstream-shaped mirror logic.
- The next A2DP convergence target remains replacing mirror flow code with
  directly imported upstream AVDTP/A2DP session callbacks.

## 2026-06-14 A2DP upstream media adapter lifecycle closeout

A2DP closeout added a gate for upstream-shaped `media.c` media adapter
lifecycle.  The bridge now validates adapter probe/register, timestamping
capability capture, endpoint/player feature aggregation, feature update after
endpoint removal, and adapter remove cleanup of apps/endpoints/players.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-media-adapter.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-media-adapter.log`
- `FeatherCore/build/logs/run-a2dp-upstream-media-adapter.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-media-adapter/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-media-adapter=probe:1,features:1,remove:1,total:3 ... final-ok=1`.
- The validator now rejects an A2DP bridge that lacks upstream-shaped media
  adapter probe/features/remove lifecycle evidence.

Current boundary:

- Media adapter lifecycle is represented by NuttX-side upstream-shaped mirror
  logic.
- The next A2DP convergence target remains replacing mirror code with directly
  imported upstream media adapter probe/remove/update-features behavior.

## 2026-06-14 A2DP upstream local player lifecycle closeout

A2DP closeout added a gate for upstream-shaped `media.c` local player
lifecycle.  The bridge now validates RegisterPlayer object creation, property
watch/seek watch ownership, track/settings/status updates, and
UnregisterPlayer cleanup.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-local-player.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-local-player.log`
- `FeatherCore/build/logs/run-a2dp-upstream-local-player.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-local-player/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-local-player=register:1,properties:1,unregister:1,total:3 ... final-ok=1`.
- The validator now rejects an A2DP bridge that lacks upstream-shaped local
  player register/properties/unregister lifecycle evidence.

Current boundary:

- Local player lifecycle is represented by NuttX-side upstream-shaped mirror
  logic.
- The next A2DP convergence target remains replacing mirror code with directly
  imported upstream local player creation/properties/cleanup behavior.

## 2026-06-14 A2DP upstream media app lifecycle closeout

A2DP closeout added a gate for upstream-shaped `media.c` application
lifecycle.  The bridge now validates Media1 RegisterApplication app creation,
proxy endpoint/player ownership, UnregisterApplication cleanup, and client
disconnect cleanup of app/endpoints/players/proxies.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-media-app.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-media-app.log`
- `FeatherCore/build/logs/run-a2dp-upstream-media-app.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-media-app/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-media-app=register:1,unregister:1,disconnect:1,total:3 ... final-ok=1`.
- The validator now rejects an A2DP bridge that lacks upstream-shaped media app
  register/unregister/disconnect lifecycle evidence.

Current boundary:

- Media app lifecycle is represented by NuttX-side upstream-shaped mirror
  logic.
- The next A2DP convergence target remains replacing mirror code with directly
  imported upstream `create_app()` / application cleanup behavior.

## 2026-06-14 A2DP upstream endpoint request lifecycle closeout

A2DP closeout added a gate for upstream-shaped `media.c` endpoint request
cleanup.  The bridge now validates single request cancel, cancel-all cleanup,
and endpoint destroy cleanup for request, transport, sender/path/uuid, and
adapter endpoint ownership.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-endpoint-request.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-endpoint-request.log`
- `FeatherCore/build/logs/run-a2dp-upstream-endpoint-request.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-endpoint-request/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-endpoint-request=cancel:1,cancel-all:1,destroy:1,total:3 ... final-ok=1`.
- The validator now rejects an A2DP bridge that lacks upstream-shaped endpoint
  request cancel/cancel-all/destroy lifecycle evidence.

Current boundary:

- Endpoint request cleanup is represented by NuttX-side upstream-shaped mirror
  logic.
- The next A2DP convergence target remains replacing mirror cleanup code with
  directly imported upstream `media_endpoint_cancel*()` and
  `media_endpoint_destroy()` behavior.

## 2026-06-14 A2DP upstream endpoint config policy closeout

A2DP closeout added a gate for upstream-shaped `media.c` endpoint
configuration lifecycle.  The bridge now validates SelectConfiguration request
cleanup, SetConfiguration request/transport binding, and ClearConfiguration
transport detachment/final cleanup.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-endpoint-config.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-endpoint-config.log`
- `FeatherCore/build/logs/run-a2dp-upstream-endpoint-config.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-endpoint-config/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-endpoint-config=select:1,set:1,clear:1,total:3 ... final-ok=1`.
- The validator now rejects an A2DP bridge that lacks upstream-shaped endpoint
  select/set/clear configuration lifecycle evidence.

Current boundary:

- Endpoint configuration lifecycle is represented by NuttX-side upstream-shaped
  mirror logic.
- The next A2DP convergence target remains replacing mirror policy code with
  directly imported upstream `media.c` endpoint request handlers.

## 2026-06-14 A2DP upstream error policy closeout

A2DP closeout added a gate for upstream-shaped error policy.  The bridge now
validates MediaTransport method errors for already-owned acquire,
try-acquire while in use, release without owner, select while in use, and
unselect without owner.  It also validates Media1 registration errors for
duplicate/missing endpoint, player, and application objects.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-error-policy.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-error-policy.log`
- `FeatherCore/build/logs/run-a2dp-upstream-error-policy.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-error-policy/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-error-policy=transport-methods:1,media-registration:1,total:2 ... final-ok=1`.
- The validator now rejects an A2DP bridge that lacks upstream-shaped
  transport method and media registration error-policy evidence.

Current boundary:

- Error policy is represented by NuttX-side upstream-shaped mirror logic.
- The next A2DP convergence target remains replacing mirror policy code with
  directly imported upstream method handlers and D-Bus error replies.

## 2026-06-14 A2DP upstream transport ops policy closeout

A2DP closeout added a gate for the upstream BlueZ `media_transport_ops`
dispatch policy.  The bridge now validates A2DP source/sink UUID selection,
presence of owner/resume/suspend/cancel/set_state ops, and an A2DP ops
lifecycle path that drives requesting -> active -> suspending -> idle while
clearing resume/cancel ids.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-transport-ops.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-transport-ops.log`
- `FeatherCore/build/logs/run-a2dp-upstream-transport-ops.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-transport-ops/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-transport-ops=uuid:1,dispatch:1,lifecycle:1,total:3 ... final-ok=1`.
- The validator now rejects an A2DP MediaTransport bridge that lacks upstream
  transport ops UUID, dispatch, and lifecycle evidence.

Current boundary:

- The compat bridge now mirrors upstream `media_transport_ops` dispatch policy.
- The next A2DP convergence target remains replacing mirror policy code with
  directly imported upstream `transport_ops[]` and A2DP ops callbacks.

## 2026-06-14 A2DP upstream transport state policy closeout

A2DP closeout added a gate for the exact upstream BlueZ `transport.c`
state policy.  The bridge now validates `state2str()` behavior for idle,
pending, broadcasting, requesting, active, and suspending states, validates
`state_in_use()` semantics, and verifies the pending -> requesting -> active
-> suspending -> idle transition path.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-state-policy.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-state-policy.log`
- `FeatherCore/build/logs/run-a2dp-upstream-state-policy.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-state-policy/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-state-policy=state2str:1,in-use:1,transitions:1,total:3 ... final-ok=1`.
- The validator now rejects an A2DP MediaTransport bridge that lacks the
  upstream state string, in-use, and transition policy evidence.

Current boundary:

- The compat bridge now mirrors upstream `transport.c` state policy.
- The next A2DP convergence target remains replacing mirror policy code with
  directly imported upstream `transport_set_state()` and related callbacks.

## 2026-06-14 A2DP upstream-named object graph closeout

A2DP closeout anchored the NuttX Media/Transport bridge ledger to the actual
upstream BlueZ object graph names from `profiles/audio/media.c` and
`profiles/audio/transport.c`.  The bridge now validates relationships matching
`media_adapter`, `media_endpoint`, `endpoint_request`, `media_transport`,
`media_owner`, `media_request`, `a2dp_transport`, and `media_transport_ops`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-upstream-object-graph.log`
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-object-graph.log`
- `FeatherCore/build/logs/run-a2dp-upstream-object-graph.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-object-graph/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... upstream-object-graph=media:1,transport:1,endpoint-request:1,total:3 ... final-ok=1`.
- The validator now rejects a Media/Transport bridge that lacks an
  upstream-named object graph relationship gate.

Current boundary:

- The compat graph now mirrors upstream object names and ownership edges.
- The next A2DP convergence target remains replacing these mirror structs with
  directly imported upstream `media.c` / `transport.c` structures and handlers.

## 2026-06-14 A2DP D-Bus request/error lifecycle semantic closeout

A2DP closeout advanced the BlueZ Media/Transport bridge ownership model with
explicit D-Bus request lifecycle semantics.  The bridge now validates successful
Acquire/Release-style replies, TryAcquire busy/error reply cleanup, duplicate
registration rejection, missing-object rejection, owner-disconnect cleanup, and
no leaked pending request/message refs.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-lifecycle-semantics.log`
- `FeatherCore/build/logs/build-bt2-a2dp-lifecycle-semantics.log`
- `FeatherCore/build/logs/run-a2dp-lifecycle-semantics.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-lifecycle-semantics/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... lifecycle-semantics=dbus-requests:1,errors:1,total:2 ... final-ok=1`.
- The validator now rejects Media/Transport ownership that lacks explicit
  successful request and error lifecycle evidence.

Current boundary:

- D-Bus request/error lifecycle behavior is represented by NuttX-side
  upstream-shaped semantic ledgers.
- The next A2DP convergence target remains replacing these ledgers with direct
  imported upstream `profiles/audio/media.c` / `profiles/audio/transport.c`
  request objects and callbacks.

## 2026-06-14 A2DP Media/Transport ownership semantic closeout

A2DP closeout advanced the BlueZ Media/Transport bridge from handler semantics
to object and request ownership semantics.  The bridge now validates D-Bus
object registration/release, owner watch cleanup, pending request ownership,
message ref/reply cleanup, fd handoff, and final-zero lifecycle accounting.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-ownership-semantics.log`
- `FeatherCore/build/logs/build-bt2-a2dp-ownership-semantics.log`
- `FeatherCore/build/logs/run-a2dp-ownership-semantics.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-ownership-semantics/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... ownership-semantics=objects:1,requests:1,final-zero:1,total:3 ... final-ok=1`.
- The validator now rejects a Media/Transport bridge that lacks object,
  request, and final-zero ownership evidence.

Current boundary:

- Media/Transport object and request ownership are represented by NuttX-side
  upstream-shaped semantic ledgers.
- The next A2DP convergence target remains replacing these ledgers with direct
  imported upstream `profiles/audio/media.c` / `profiles/audio/transport.c`
  object and request structures.

## 2026-06-14 A2DP Media1 semantic wrapper closeout

A2DP closeout advanced the BlueZ `Media1` bridge side from callable-only
coverage to explicit lifecycle semantics.  The bridge now validates
`RegisterEndpoint`, `UnregisterEndpoint`, `RegisterPlayer`,
`UnregisterPlayer`, `RegisterApplication`, and `UnregisterApplication`, plus
`SupportedUUIDs` and `SupportedFeatures` property getters.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-media-semantics.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-semantics.log`
- `FeatherCore/build/logs/run-a2dp-media-semantics.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-semantics/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... media-semantics=methods:6,properties:2,total:8 ... final-ok=1`.
- The validator now rejects a Media1 handler family that only exposes callable
  method/property symbols without registration and property semantics.

Current boundary:

- MediaTransport and Media1 handler families now both have NuttX-side
  upstream-shaped compat semantics.
- The next A2DP convergence target remains direct imported upstream
  `profiles/audio/media.c` / `profiles/audio/transport.c` D-Bus object and
  request ownership.

## 2026-06-14 A2DP transport property semantic wrapper closeout

A2DP closeout advanced the MediaTransport property bridge from callable-only
coverage to explicit D-Bus property semantics.  The bridge now validates
getter semantics for `Device`, `UUID`, `Codec`, `Configuration`, `State`,
`Delay`, `Volume`, and experimental `Endpoint`, setter semantics for `Delay`
and `Volume`, plus existence predicates for `Delay`, `Volume`, and
`Endpoint`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-transport-property-semantics.log`
- `FeatherCore/build/logs/build-bt2-a2dp-transport-property-semantics.log`
- `FeatherCore/build/logs/run-a2dp-transport-property-semantics.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-transport-property-semantics/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... property-semantics=getters:8,setters:2,exists:3,total:13 ... final-ok=1`.
- The validator now rejects a MediaTransport property family that only exposes
  callable handlers without property value/set/existence semantics.

Current boundary:

- MediaTransport method and property families now both have NuttX-side
  upstream-shaped compat semantics.
- The next A2DP convergence target remains direct imported upstream
  `profiles/audio/transport.c` object ownership and D-Bus request handling.

## 2026-06-14 A2DP transport full method semantic wrapper closeout

A2DP closeout advanced every MediaTransport method bridge wrapper from
callable-only surface coverage to explicit upstream-shaped lifecycle semantics.
`Acquire`, `TryAcquire`, `Release`, `Select`, and `Unselect` now each execute a
stateful compat path before the bridge can report success.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-transport-full-method-semantics.log`
- `FeatherCore/build/logs/build-bt2-a2dp-transport-full-method-semantics.log`
- `FeatherCore/build/logs/run-a2dp-transport-full-method-semantics.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-transport-full-method-semantics/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... handler-semantics=acquire:1,try-acquire:1,release:1,select:1,unselect:1,total:5 ... final-ok=1`.
- The validator now rejects a MediaTransport method family that only exposes
  callable symbols without per-method semantics.

Current boundary:

- The complete MediaTransport method family now has NuttX-side compat
  lifecycle semantics.
- The wrappers still need to keep converging toward direct imported upstream
  `profiles/audio/transport.c` handler ownership and D-Bus object state.

## 2026-06-14 A2DP transport acquire/release semantic wrapper closeout

A2DP closeout advanced the MediaTransport method bridge from named-callable
symbols to lifecycle-bearing wrappers for `Acquire` and `Release`.  The bridge
now validates an upstream-shaped transport state transition: idle -> requesting
-> active for acquire, and active -> releasing -> idle for release, including
owner/request/fd/MTU and resume/suspend side effects.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-transport-method-semantics.log`
- `FeatherCore/build/logs/build-bt2-a2dp-transport-method-semantics.log`
- `FeatherCore/build/logs/run-a2dp-transport-method-semantics.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-transport-method-semantics/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and
  bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... handler-semantics=acquire:1,release:1,acquire-release:2 ... final-ok=1`.
- The validator now requires this semantic wrapper evidence, so generic
  callable symbols alone no longer satisfy the A2DP bridge gate.

Current boundary:

- `Acquire` and `Release` now carry explicit MediaTransport lifecycle semantics
  in the NuttX bridge.
- The wrappers are still compat implementations, not unmodified upstream
  `profiles/audio/transport.c` handler bodies.

## 2026-06-14 A2DP transport method wrapper symbol closeout

A2DP closeout replaced the first handler-family generic bridge stub with named callable wrapper symbols.  The MediaTransport method bridge entries in `bluez/upstream_media_transport_bridge.c` now point to separate wrapper symbols for `acquire`, `try_acquire`, `release`, `select_transport`, and `unselect_transport`, instead of sharing one generic handler stub.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-transport-method-symbols.log`
- `FeatherCore/build/logs/build-bt2-a2dp-transport-method-symbols.log`
- `FeatherCore/build/logs/run-a2dp-transport-method-symbols.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-transport-method-symbols/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... handler-symbols=transport-methods:5 ... final-ok=1`.
- Confirmed all five MediaTransport method wrapper symbols compile, link, and are callable through the bridge ABI.
- Confirmed the existing handler-call coverage remains intact: transport methods, transport properties, media methods, and media properties still all pass.

Current boundary:

- This is the first concrete replacement of a generic bridge stub with named wrapper symbols matching upstream handler families.
- The wrappers still call compat bridge logic, not unmodified upstream `transport.c` handler bodies.
- Next A2DP step should replace one named wrapper body, starting with `acquire` or `release`, with a wrapper around directly imported upstream handler semantics.

## 2026-06-14 A2DP handler symbol surface closeout

A2DP closeout advanced the dedicated Media/Transport bridge module from a presence map to a callable handler-symbol surface.  Each bridge entry in `bluez/upstream_media_transport_bridge.c` now carries a handler function pointer, and the hwsim gate validates that every mapped Media/Transport handler family is actually callable through the bridge ABI.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-handler-symbol-surface.log`
- `FeatherCore/build/logs/build-bt2-a2dp-handler-symbol-surface.log`
- `FeatherCore/build/logs/run-a2dp-handler-symbol-surface.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-handler-symbol-surface/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... handler-calls=... symbols-callable:1 ... final-ok=1`.
- Confirmed callable transport method symbols: acquire, try-acquire, release, select, and unselect.
- Confirmed callable transport property symbols: getters, setters, and exists predicates.
- Confirmed callable media method symbols: register/unregister endpoint, register/unregister player, and register/unregister application.
- Confirmed callable media property symbols: supported UUIDs and supported features.

Current boundary:

- This narrows the replacement seam: bridge entries are no longer only named/present, they are callable through a uniform ABI.
- The callable symbols are still compat bridge stubs, not direct unmodified upstream `media.c`/`transport.c` handlers.
- Next A2DP step should replace the bridge stub for one handler family at a time with a wrapper around directly compiled/imported upstream logic.

## 2026-06-14 A2DP handler bridge module closeout

A2DP closeout moved the Media/Transport handler-family bridge out of the large `upstream_a2dp_compat.c` print/runner body into a dedicated replaceable ABI module.  The new `bluez/upstream_media_transport_bridge.c` and `bluez/upstream_media_transport_bridge.h` now own the handler bridge maps and expose `bluez_upstream_a2dp_handler_bridge_surface_run()` for the daemon compatibility path.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-handler-bridge-module.log`
- `FeatherCore/build/logs/build-bt2-a2dp-handler-bridge-module.log`
- `FeatherCore/build/logs/run-a2dp-handler-bridge-module.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-handler-bridge-module/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both still emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... final-ok=1`.
- Confirmed the new module is included by both `apps/wireless/linux_bluetooth/Makefile` and `apps/wireless/linux_bluetooth/CMakeLists.txt`.
- Confirmed `upstream_a2dp_compat.c` now consumes the bridge through `upstream_media_transport_bridge.h` instead of owning the handler maps inline.

Current boundary:

- This is a structural step toward replacing staged dispatch: the handler-family bridge is now an isolated ABI seam that can be swapped for real upstream `media.c`/`transport.c` handler implementations.
- It still does not directly link unmodified upstream handler bodies.
- Next A2DP step should start replacing individual bridge entries inside `upstream_media_transport_bridge.c` with wrapper calls around directly compiled/imported upstream handler code.

## 2026-06-14 A2DP handler bridge map refactor closeout

A2DP closeout kept the same upstream handler-family coverage but refactored the Media/Transport handler bridge from hard-coded runner counters into explicit data-driven bridge maps in `bluez/upstream_a2dp_compat.c`.  This is a pre-replacement step: each bridge entry now names the upstream handler family that should later be replaced by a directly compiled/imported `media.c` or `transport.c` handler.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-handler-bridge-map.log`
- `FeatherCore/build/logs/build-bt2-a2dp-handler-bridge-map.log`
- `FeatherCore/build/logs/run-a2dp-handler-bridge-map.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-handler-bridge-map/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both still emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... final-ok=1`.
- Confirmed handler bridge maps now drive the transport method, transport property getter/setter/exists, media method, and media property getter counts.
- Confirmed external hwsim contract did not change while the internal compat representation became easier to replace with real upstream handlers.

Current boundary:

- This reduces staged glue shape by collecting handler-family presence into explicit maps, instead of scattering counts through the runner body.
- It still does not directly link unmodified upstream `media.c`/`transport.c` handlers.
- Next A2DP step should start replacing individual bridge-map entries with real handler symbols or wrapper calls around directly compiled upstream handler code.

## 2026-06-14 A2DP upstream handler bridge surface closeout

A2DP closeout advanced from D-Bus table surface mirroring into an explicit upstream handler-family bridge surface for `third/bluez/profiles/audio/media.c` and `third/bluez/profiles/audio/transport.c`.  The NuttX sim BlueZ daemon path now validates that each mirrored MediaTransport/MediaEndpoint table entry has a corresponding handler family mapped before replacing staged dispatch with directly imported upstream handlers.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-handler-bridge-surface.log`
- `FeatherCore/build/logs/build-bt2-a2dp-handler-bridge-surface.log`
- `FeatherCore/build/logs/run-a2dp-handler-bridge-surface.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-handler-bridge-surface/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-handler-bridge-surface-wrapper ... final-ok=1`.
- Confirmed MediaTransport method handler families: acquire, try-acquire, release, select, and unselect.
- Confirmed MediaTransport property getter families: device, uuid, codec, configuration, state, delay, volume, and endpoint.
- Confirmed MediaTransport property setter/exists families: delay setter, volume setter, delay exists, volume exists, and endpoint exists.
- Confirmed Media method handler families: register/unregister endpoint, register/unregister player, and register/unregister application.
- Confirmed Media property getter families: supported UUIDs and supported features.

Current boundary:

- This is a stronger pre-replacement boundary than the earlier table-surface mirror: every upstream D-Bus table family now has an explicit handler-family mapping in the hwsim gate.
- It is still a handler-family bridge in `bluez/upstream_a2dp_compat.c`, not direct linking of the unmodified upstream `media.c`/`transport.c` handlers.
- Next A2DP step should begin replacing these mapped handler families with directly compiled/imported handler functions and route the staged D-Bus calls through those functions.

## 2026-06-14 A2DP upstream D-Bus table surface closeout

A2DP closeout advanced from MediaEndpoint/MediaTransport dispatch semantics into the upstream D-Bus table surface from `third/bluez/profiles/audio/media.c` and `third/bluez/profiles/audio/transport.c`.  The NuttX sim BlueZ daemon path now validates the method/property table shape needed before replacing staged dispatch with directly imported upstream method handlers.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-dbus-table-surface.log`
- `FeatherCore/build/logs/build-bt2-a2dp-dbus-table-surface.log`
- `FeatherCore/build/logs/run-a2dp-dbus-table-surface.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-dbus-table-surface/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-dbus-table-surface-wrapper ... final-ok=1`.
- Confirmed `transport_methods`: `Acquire`, `TryAcquire`, `Release`, `Select`, and `Unselect` are all represented as async methods.
- Confirmed `transport_a2dp_properties`: `Device`, `UUID`, `Codec`, `Configuration`, `State`, `Delay`, `Volume`, and experimental `Endpoint`.
- Confirmed writable/existing A2DP properties: `Delay` setter, `Volume` setter, and experimental `Endpoint` property flag.
- Confirmed `media_methods`: `RegisterEndpoint`, `UnregisterEndpoint`, `RegisterPlayer`, `UnregisterPlayer`, `RegisterApplication`, and `UnregisterApplication`.
- Confirmed `media_properties`: `SupportedUUIDs` and `SupportedFeatures`.
- Confirmed transport ops surface: A2DP source/sink plus BAP unicast/broadcast ops are mirrored for profile table alignment.

Current boundary:

- This tightens the A2DP daemon/profile alignment by checking the actual upstream D-Bus table surface, including methods/properties previously not covered by the staged dispatch probes.
- It is still a mirrored table-surface check in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `media.c`/`transport.c` tables registered directly in the daemon.
- Next A2DP step should replace the mirrored surface with directly compiled/imported method tables and route hwsim D-Bus calls through those upstream handlers.

## 2026-06-14 A2DP upstream MediaEndpoint D-Bus closeout

A2DP closeout advanced from MediaTransport method dispatch into the Media endpoint registration/configuration surface used by upstream `third/bluez/profiles/audio/media.c`.  The NuttX sim BlueZ daemon path now validates a staged dispatch slice for `org.bluez.Media1` and `org.bluez.MediaEndpoint1`, covering endpoint registration, property parsing, SEP creation, async Select/SetConfiguration, ClearConfiguration, Release, unregister, and cleanup.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-media-endpoint-dbus.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-endpoint-dbus.log`
- `FeatherCore/build/logs/run-a2dp-media-endpoint-dbus.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-endpoint-dbus/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-media-endpoint-dbus-wrapper ... final-ok=1`.
- Confirmed endpoint registration surface: `RegisterEndpoint`, `UnregisterEndpoint`, UUID/codec/capability/delay-reporting parsing, duplicate rejection, invalid UUID rejection, and invalid capability rejection.
- Confirmed SEP binding: source SEP creation, sink SEP creation, and SEP removal.
- Confirmed endpoint method flow: `SelectConfiguration`, `SetConfiguration`, `ClearConfiguration`, `Release`, async request creation, pending call tracking, success reply, error reply, and request cancellation.
- Confirmed transport binding: SetConfiguration creates/appends a transport, ClearConfiguration clears/destroys it.
- Confirmed cleanup: endpoint remove, endpoint destroy, owner watch remove, custom property remove, and pending endpoints/requests/transports/watches all return to zero.

Current boundary:

- This moves A2DP above MediaTransport D-Bus dispatch into the Media1/MediaEndpoint1 profile registration contract used by real BlueZ audio clients.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `media.c` method handlers and mainloop dispatch linked end-to-end.
- Next A2DP step should progressively replace this staged endpoint dispatch slice with directly compiled/imported `media.c` handlers and route real D-Bus calls through the daemon path.

## 2026-06-14 A2DP upstream MediaTransport D-Bus dispatch closeout

A2DP closeout advanced from MediaTransport ownership into the daemon D-Bus method surface used by upstream `third/bluez/profiles/audio/transport.c`.  The NuttX sim BlueZ daemon path now validates a staged dispatch slice for `org.bluez.MediaTransport1` covering `Acquire`, `TryAcquire`, `Release`, property get/set, success replies, state guards, error replies, request lifecycle, and owner watch cleanup.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-media-transport-dbus.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-transport-dbus.log`
- `FeatherCore/build/logs/run-a2dp-media-transport-dbus.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-transport-dbus/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-media-transport-dbus-wrapper ... final-ok=1`.
- Confirmed D-Bus method surface: `Acquire`, `TryAcquire`, and `Release` are represented with success and guarded failure paths.
- Confirmed successful client flow: fd/imtu/omtu reply, request completion, state transitions through idle/requesting/active/suspending/idle, and owner watch add/remove.
- Confirmed error flow: owner conflict, not available, not authorized, invalid args, and unsupported property/method behavior.
- Confirmed property flow: get properties, set volume, set delay, volume changed signal, and delay changed signal.
- Confirmed cleanup: pending owners, requests, fds, and watches all return to zero.

Current boundary:

- This moves A2DP above MediaTransport object ownership into the real D-Bus method contract that BlueZ clients use for audio transport acquisition and release.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `transport.c` method table and mainloop dispatch linked end-to-end.
- Next A2DP step should replace this staged D-Bus dispatch slice with directly compiled/imported `transport.c` method handlers and then bind real daemon mainloop ownership to the hwsim AVDTP path.

## 2026-06-14 A2DP upstream MediaTransport ownership closeout

A2DP closeout advanced from AVDTP Close/Abort teardown into BlueZ daemon-side MediaEndpoint/MediaTransport ownership semantics from upstream `third/bluez/profiles/audio/media.c` and `third/bluez/profiles/audio/transport.c`.  The NuttX sim BlueZ daemon path now validates a staged ownership slice covering `org.bluez.MediaEndpoint1`, `org.bluez.MediaTransport1`, transport creation, owner/watch lifecycle, Acquire/Release, A2DP resume/suspend binding, properties, and destroy/unregister cleanup.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-media-transport-ownership.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-transport-ownership.log`
- `FeatherCore/build/logs/run-a2dp-media-transport-ownership.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-transport-ownership/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-media-transport-ownership-wrapper ... final-ok=1`.
- Confirmed MediaEndpoint/MediaTransport object lifecycle: endpoint registration/find, transport create/path allocation, A2DP ops lookup/init, configuration copy, D-Bus register, global transport append, endpoint transport append, and property export.
- Confirmed owner lifecycle: owner creation, disconnect watch add, owner set/remove, pending request removal, clear owner, and final owner/watch zero.
- Confirmed Acquire path: request, `TRANSPORT_STATE_REQUESTING`, `transport_a2dp_resume`, fd ready, fd/imtu/omtu reply, and `TRANSPORT_STATE_ACTIVE`.
- Confirmed Release/cancel path: Release request, `transport_a2dp_suspend`, `TRANSPORT_STATE_SUSPENDING`, `TRANSPORT_STATE_IDLE`, cancel-resume, and `a2dp_cancel`.
- Confirmed properties and cleanup: delay update/emit, volume get/set/emit, clear configuration, endpoint remove transport, cancel all, transport destroy, D-Bus unregister, transport free, and final pending transports zero.

Current boundary:

- This moves A2DP above AVDTP transaction probes into daemon-side MediaTransport ownership semantics required by real BlueZ clients using `Acquire` and `Release`.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `media.c`/`transport.c` object body linked end-to-end.
- Next A2DP step should progressively replace this staged ownership slice with directly compiled/imported `media.c`/`transport.c` code paths and real daemon mainloop/D-Bus method dispatch.

## 2026-06-14 A2DP upstream Close/Abort transaction closeout

A2DP closeout advanced from Start/Suspend lifecycle into the Close/Abort and cancellation paths used by upstream `third/bluez/profiles/audio/a2dp.c`.  The NuttX sim BlueZ daemon path now validates the staged transaction flow around `close_ind`, `close_cfm`, `abort_ind`, `abort_cfm`, `a2dp_cancel`, and `a2dp_reconfigure`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-close-abort-transaction.log`
- `FeatherCore/build/logs/build-bt2-a2dp-close-abort-transaction.log`
- `FeatherCore/build/logs/run-a2dp-close-abort-transaction.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-close-abort-transaction/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-close-abort-transaction-wrapper ... final-ok=1`.
- Confirmed Close paths: Close_Ind unwinds pending suspend/resume with `-ECONNRESET`, Close_Cfm success resolves remote SEP and schedules reconfigure, and Close_Cfm error clears stream and finalizes config.
- Confirmed Abort paths: Abort_Ind destroys the stream and unwinds suspend/resume/config callbacks, Abort_Cfm either schedules reconfigure or unreferences setup.
- Confirmed cancellation path: `a2dp_cancel` lookup, setup ref, callback free, AVDTP Abort, and return-after-abort are covered.
- Confirmed cleanup: setup callback free reaches six calls, setup unref reaches four calls, and pending callbacks, setups, streams, and transactions return to zero.

Current boundary:

- This moves A2DP beyond media Start/Suspend into executable Close/Abort/reconfigure/cancel lifecycle semantics required for teardown and error recovery.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `a2dp.c` body owning real BlueZ objects end-to-end.
- Next A2DP step should reduce staged ownership around real upstream setup/session/stream structs, then bind these transaction slices to daemon/mainloop/D-Bus MediaTransport ownership.

## 2026-06-14 A2DP upstream Start/Suspend transaction closeout

A2DP closeout advanced from finalizer/callback ABI into the Start/Suspend transaction lifecycle used by upstream `third/bluez/profiles/audio/a2dp.c`.  The NuttX sim BlueZ daemon path now validates the staged transaction flow around `a2dp_resume`, `a2dp_suspend`, `start_ind`, `start_cfm`, `suspend_ind`, and `suspend_cfm`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-start-suspend-transaction.log`
- `FeatherCore/build/logs/build-bt2-a2dp-start-suspend-transaction.log`
- `FeatherCore/build/logs/run-a2dp-start-suspend-transaction.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-start-suspend-transaction/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-start-suspend-transaction-wrapper ... final-ok=1`.
- Confirmed resume paths: CONFIGURED defers start until Start_Ind, OPEN issues AVDTP Start and finalizes on Start_Cfm, STREAMING finalizes immediately, and suspend-in-progress defers resume until suspend completion.
- Confirmed suspend paths: OPEN finalizes immediately, STREAMING issues AVDTP Suspend, Suspend_Ind and Suspend_Cfm both finalize suspend callbacks, and restart-after-suspend is covered.
- Confirmed failure paths: resume bad state, suspend bad state, reconfigure reject, AVDTP Start failure, AVDTP Suspend failure, and restart-after-suspend failure.
- Confirmed cleanup: setup callback free and setup unref both reach fifteen calls, with pending callbacks, pending setups, and pending transactions returning to zero.

Current boundary:

- This moves A2DP beyond callback ABI into executable Start/Suspend transaction lifecycle semantics required for media start, suspend, restart, and error unwind.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `a2dp.c` body owning real BlueZ objects end-to-end.
- Next A2DP step should reduce staged ownership around actual upstream setup/session/stream structures and then attach the daemon/mainloop/D-Bus ownership path more directly to these transaction slices.

## 2026-06-14 A2DP upstream finalizer/callback ABI closeout

A2DP closeout advanced from SetConfiguration transaction lifecycle into the callback/finalizer ABI used by upstream `third/bluez/profiles/audio/a2dp.c`.  The NuttX sim BlueZ daemon path now validates callable slices using the real upstream callback typedefs `a2dp_config_cb_t` and `a2dp_stream_cb_t`, including success delivery, error delivery, finalizer routing, errno propagation, and cleanup accounting.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-finalizer-callback-v3.log`
- `FeatherCore/build/logs/build-bt2-a2dp-finalizer-callback-v3.log`
- `FeatherCore/build/logs/run-a2dp-finalizer-callback-v3.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-finalizer-callback-v3/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-finalizer-callback-wrapper ... final-ok=1`.
- Confirmed callback dispatch through `a2dp_config_cb_t` and `a2dp_stream_cb_t` for config, resume, and suspend paths.
- Confirmed finalizer routing: `finalize_config`, `finalize_resume`, `finalize_suspend`, and `finalize_setup_errno`.
- Confirmed success/error delivery: config success/error, resume success/error, suspend success/error, plus stream pointer delivery.
- Confirmed error propagation: `-EIO` and `-EINVAL` paths are counted and delivered to the expected callback families.
- Confirmed cleanup: setup callback free and setup unref both reach six calls, with pending callbacks and pending setups returning to zero.

Current boundary:

- This moves A2DP beyond transaction counters into executable callback/finalizer ABI semantics needed by the upstream audio plugin lifecycle.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `a2dp.c` body owning real BlueZ objects end-to-end.
- Next A2DP step should connect these callable slices more directly to imported upstream setup/session/stream object ownership, reducing the remaining staged glue around finalizer dispatch.

## 2026-06-14 A2DP upstream SetConfiguration transaction closeout

A2DP closeout advanced from state-policy decisions into SetConfiguration transaction lifecycle semantics.  The NuttX sim BlueZ daemon path now validates the staged transaction flow corresponding to `third/bluez/profiles/audio/a2dp.c` `a2dp_config`, `set_configuration`, configuration confirmation, and `finalize_config` cleanup.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-setconf-transaction.log`
- `FeatherCore/build/logs/build-bt2-a2dp-setconf-transaction.log`
- `FeatherCore/build/logs/run-a2dp-setconf-transaction.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-setconf-transaction/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-setconf-transaction-wrapper ... final-ok=1`.
- Confirmed normal transaction lifecycle: setup get, setup callback add, caps copy, remote SEP resolution, AVDTP SetConfiguration, stream assignment, SetConfiguration confirmation, config callback, and finalize.
- Confirmed same-caps path: idle finalize without a new SetConfiguration transaction.
- Confirmed reconfigure path: different caps close the existing stream, set reconfigure flag, retry, then complete SetConfiguration.
- Confirmed failure paths: no remote SEP and AVDTP SetConfiguration failure both cleanup setup callbacks and setup refs.
- Confirmed final cleanup: pending callbacks, pending setups, and pending transactions all zero.

Current boundary:

- This moves A2DP beyond state-policy tables into executable transaction lifecycle semantics required by SetConfiguration and reconfigure paths.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `a2dp.c` body owning real BlueZ objects end-to-end.
- Next A2DP step should connect these transaction slices to imported upstream setup/session/stream object code, reducing staged counters around setup callbacks and finalizers.

## 2026-06-14 A2DP upstream SetConfiguration/state policy closeout

A2DP closeout advanced from SEP matching into upstream-style SetConfiguration/session state policy.  The NuttX sim BlueZ daemon path now validates the key state decisions used by `third/bluez/profiles/audio/a2dp.c` around `a2dp_config`, `a2dp_resume`, and `a2dp_suspend`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-state-policy.log`
- `FeatherCore/build/logs/build-bt2-a2dp-state-policy.log`
- `FeatherCore/build/logs/run-a2dp-state-policy.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-state-policy/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-state-policy-wrapper ... final-ok=1`.
- Confirmed `a2dp_config` policy: IDLE allows SetConfiguration, OPEN/STREAMING with same caps finalize, OPEN/STREAMING with different caps reconfigure, CONFIGURED/CLOSING/ABORTING reject, locked stream rejects, missing codec rejects, and codec mismatch rejects.
- Confirmed `a2dp_resume` policy: IDLE rejects, CONFIGURED defers start, OPEN starts, STREAMING finalizes, CLOSING/ABORTING reject, and active reconfigure rejects.
- Confirmed `a2dp_suspend` policy: IDLE rejects, OPEN finalizes, STREAMING suspends, CONFIGURED/CLOSING/ABORTING reject, and active reconfigure rejects.

Current boundary:

- This moves A2DP beyond codec/SEP matching into executable session state policy required for SetConfiguration, Start, Suspend, and reconfigure decisions.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `a2dp.c` body owning real BlueZ objects end-to-end.
- Next A2DP step should connect these policy slices to more imported upstream object code, reducing staged wrappers around setup/session/stream state transitions.

## 2026-06-14 A2DP upstream SEP matching closeout

A2DP closeout advanced from SBC codec/config selection into upstream-style SEP matching semantics.  The NuttX sim BlueZ daemon path now validates the key matching rules used around `third/bluez/profiles/audio/a2dp.c` `find_sep`, `find_remote_sep`, and `SetConfiguration`: remote SEP direction maps to the opposite local SEP role, MediaEndpoint sender/path must match, codec must match, and invalid combinations are rejected.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-sep-matching.log`
- `FeatherCore/build/logs/build-bt2-a2dp-sep-matching.log`
- `FeatherCore/build/logs/run-a2dp-sep-matching.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-sep-matching/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-sep-matching-wrapper ... final-ok=1`.
- Confirmed role matching: `remote-source->local-sink=1`, `remote-sink->local-source=1`.
- Confirmed endpoint matching: `sender-path-match=1`.
- Confirmed codec matching: `codec-match=1` for SBC.
- Confirmed rejection paths: wrong sender, wrong path, codec mismatch, missing remote codec, and missing local SEP are all rejected.

Current boundary:

- This moves A2DP beyond codec payload/config selection into executable endpoint/SEP compatibility semantics required before AVDTP SetConfiguration.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `a2dp.c` object body owning the full DBus/AVDTP runtime.
- Next A2DP step should continue with SetConfiguration/session state policy, then progressively replace staged helpers with imported upstream BlueZ objects.

## 2026-06-14 A2DP upstream SBC config selector closeout

A2DP closeout advanced from public error mapping into codec/config negotiation semantics.  The NuttX sim BlueZ daemon path now compiles and validates an upstream-style SBC capability selector using `third/bluez/profiles/audio/a2dp-codecs.h` definitions and A2DP protocol error codes.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-sbc-config-selector-v2.log`
- `FeatherCore/build/logs/build-bt2-a2dp-sbc-config-selector-v2.log`
- `FeatherCore/build/logs/run-a2dp-sbc-config-selector-v2.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-sbc-config-selector-v2/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-sbc-config-selector ... final-ok=1`.
- Confirmed selected SBC config from remote capabilities: `frequency=48000`, `channel=joint-stereo`, `block=16`, `subbands=8`, `alloc=loudness`, `min-bitpool=2`, `max-bitpool=51`.
- Confirmed remote capability input: `frequency=48000+44100`, `channel=joint-stereo+stereo`, `block=16+12`, `subbands=8+4`, `alloc=loudness+snr`, `min-bitpool=2`, `max-bitpool=250`.
- Confirmed error policy: no frequency -> `0xc4`, no channel -> `0xc6`, no block -> `0xdd`, no subbands -> `0xc8`, no allocation -> `0xca`, bad min bitpool -> `0xcb`, bad max bitpool -> `0xcd`, null input -> `0x29`.

Current boundary:

- This moves A2DP beyond media payload probing into executable codec negotiation semantics required by AVDTP SetConfiguration.
- It is still a staged callable slice in `bluez/upstream_a2dp_compat.c`, not the unmodified upstream `a2dp.c` body owning the full endpoint selection path.
- Next A2DP step should continue replacing staged selection and SEP matching helpers with callable upstream slices until the daemon path can rely on imported BlueZ objects rather than compatibility probes.

## 2026-06-14 A2DP upstream config-error parser closeout

A2DP closeout advanced from staged ownership evidence into a callable upstream public-function slice from `third/bluez/profiles/audio/a2dp.c`: `a2dp_parse_config_error()`.  The NuttX sim BlueZ daemon path now compiles and validates the BlueZ A2DP D-Bus error-name to protocol error-code mapping used by MediaEndpoint configuration failure handling.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-config-error-parser.log`
- `FeatherCore/build/logs/build-bt2-a2dp-config-error-parser.log`
- `FeatherCore/build/logs/run-a2dp-config-error-parser.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-config-error-parser/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-config-error-parser ... final-ok=1`.
- Confirmed mapped BlueZ errors: `InvalidCodecType=0xc1`, `NotSupportedCodecType=0xc2`, `InvalidSamplingFrequency=0xc3`, `InvalidChannelMode=0xc5`, `InvalidMaximumBitpoolValue=0xcd`, `InvalidCodecParameter=0xe2`.
- Confirmed fallback behavior: wrong prefix, unknown suffix, and null input all map to `AVDTP_UNSUPPORTED_CONFIGURATION=0x29`.
- Confirmed upstream table size in the ported slice: `entries=35`.

Current boundary:

- This is a real public A2DP function slice ported into the NuttX BlueZ compat build and required by hwsim validation.
- It is still not the unmodified upstream `a2dp.c` object body owning the whole daemon path.
- Next A2DP step should continue replacing staged helpers with callable upstream slices around config selection, SEP matching, and stream state transition policy.

## 2026-06-14 A2DP upstream AVDTP session/stream ownership closeout

A2DP closeout advanced again from upstream `a2dp.c` setup ownership into a staged ownership model for `third/bluez/profiles/audio/avdtp.c`.  The NuttX sim BlueZ daemon path now emits and validates source/sink lifecycle evidence for AVDTP session, local SEP, remote SEP, discovery callback, pending request, stream, stream callback, transport attach/get/clear, pending-open ownership, state transitions, and final cleanup.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-avdtp-owner.log`
- `FeatherCore/build/logs/build-bt2-a2dp-avdtp-owner.log`
- `FeatherCore/build/logs/run-a2dp-avdtp-owner.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-owner/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-avdtp-ownership-wrapper ... final-ok=1`.
- Confirmed object lifecycle: `session:1`, `local-sep:1`, `remote-sep:1`, `discover:1`, `request:1`, `stream:1`, `stream-cb:1`.
- Confirmed state lifecycle: `configured:1`, `open:1`, `streaming:1`, `idle:1`.
- Confirmed transport lifecycle: `set:1`, `get:1`, `clear:1`, `pending-open-set:1`, `pending-open-clear:1`.
- Confirmed cleanup and leak closure: `discover_free:1`, `remote-sep-unregister:1`, `stream-cb-remove:1`, `stream_free:1`, `session_free:1`, `final-zero=sessions:0,local-seps:0,remote-seps:0,streams:0,discovers:0,requests:0,stream-cbs:0,transports:0,refs:0`.

Current boundary:

- A2DP staged convergence now validates upstream source manifest, public A2DP/AVDTP headers, endpoint callbacks, AVDTP cfm/ind callbacks, `a2dp.c` ownership model, and `avdtp.c` ownership model.
- It is still not an unmodified upstream `bluetoothd` audio plugin with native `a2dp.c` and `avdtp.c` bodies owning the whole runtime.
- Next A2DP step should replace staged ownership probes with progressively larger callable slices from upstream `a2dp.c`/`avdtp.c`, while preserving these hwsim source/sink contracts.

## 2026-06-14 A2DP upstream setup/SEP/stream ownership closeout

A2DP closeout advanced from callback-surface compatibility into a staged ownership model for the upstream `third/bluez/profiles/audio/a2dp.c` object lifecycle.  The NuttX sim BlueZ daemon path now emits and validates a source/sink ownership wrapper for the same core object families used by upstream BlueZ: server, channel, setup, setup callback, SEP, stream, endpoint queues, transport attach/detach, callback completion, and final cleanup.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-setup-owner.log`
- `FeatherCore/build/logs/build-bt2-a2dp-setup-owner.log`
- `FeatherCore/build/logs/run-a2dp-setup-owner.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-setup-owner/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-setup-ownership-wrapper ... final-ok=1`.
- Confirmed object lifecycle: `server:1`, `channel:1`, `setup:1`, `setup_cb:4`, `sep:1`, `stream:1`.
- Confirmed callback lifecycle: `discover:1`, `select:1`, `config:1`, `resume:1`, `suspend:1`.
- Confirmed transport lifecycle: `attach:1`, `detach:1`.
- Confirmed cleanup and leak closure: `setup_free:1`, `setup_cb_free:4`, `sep_remove:1`, `stream_destroy:1`, `final-zero=setups:0,seps:0,streams:0,cbs:0,refs:0`.

Current boundary:

- This is a staged `a2dp.c` ownership model compiled and driven through the sim daemon, with validator-enforced source/sink evidence.
- It is still not the unmodified upstream `a2dp.c` object body owning the real BlueZ daemon path.
- Next A2DP step should replace the staged ownership probe with progressively larger callable slices from upstream `a2dp.c`, then do the same for `avdtp.c` session/object ownership.

## 2026-06-14 A2DP upstream AVDTP callback wrapper closeout

A2DP closeout advanced one layer beyond the upstream source manifest and `struct a2dp_endpoint` callback wrapper: the NuttX sim BlueZ daemon path now compiles, instantiates, and executes the upstream public `struct avdtp_sep_cfm` and `struct avdtp_sep_ind` callback tables from `bluez/upstream/profiles/audio/avdtp.h` through `bluez/upstream_a2dp_compat.c`.

Verification artifacts:

- `FeatherCore/build/logs/build-bt1-a2dp-avdtp-callback.log`
- `FeatherCore/build/logs/build-bt2-a2dp-avdtp-callback.log`
- `FeatherCore/build/logs/run-a2dp-avdtp-callback.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-callback/run-results.json`

Validated hwsim case:

- `bluez-a2dp-upstream-convergence-closeout`: PASS for bt1/source and bt2/sink.
- Source and sink both emit `bluez-daemon: a2dp upstream-avdtp-callback-wrapper ... final-ok=1`.
- Confirmed cfm callbacks: `set_configuration`, `get_configuration`, `open`, `start`, `suspend`, `close`, `abort`, `reconfigure`, `delay_report`.
- Confirmed ind callbacks: `match_codec`, `get_capability`, `set_configuration`, `set_configuration_cb`, `get_configuration`, `open`, `start`, `suspend`, `close`, `abort`, `reconfigure`, `delayreport`.
- Existing A2DP closeout still completes full staged media transport, RTP/SBC payload simulation, AVRCP event path, and ownership cleanup.

Current boundary:

- This closes the AVDTP public callback surface alignment for the staged A2DP daemon path.
- It is still not an unmodified upstream `bluetoothd` audio plugin running its native `a2dp.c`/`avdtp.c` object ownership and mainloop end-to-end.
- Next A2DP step should move actual upstream `a2dp.c` setup/session/SEP ownership into the compatibility layer first, then converge `avdtp.c` session/object ownership.

## 2026-06-14 A2DP upstream endpoint callback wrapper closeout

继续 A2DP 去 staged adapter 化，本轮在 upstream header compat wrapper 之后，把 upstream `struct a2dp_endpoint` callback surface 实例化并纳入 hwsim gate。

A2DP upstream convergence + endpoint callback wrapper：PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-endpoint-callback.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-endpoint-callback.log`
- run: `FeatherCore/build/logs/run-a2dp-endpoint-callback.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-endpoint-callback/run-results.json`
- roles: `bt1` source, `bt2` sink

实现改动：`FeatherCore/apps/wireless/linux_bluetooth/bluez/upstream_a2dp_compat.c` 现在构造真实 upstream public `struct a2dp_endpoint` callback table，并执行 `get_name`、`get_path`、`get_capabilities`、`select_configuration`、`set_configuration`、`clear_configuration`、`set_delay`。`select_configuration` 和 `set_configuration` 分别通过 upstream 类型 `a2dp_endpoint_select_t`、`a2dp_endpoint_config_t` 回调闭环。

新增验证门槛：validator 现在要求 source/sink 两端输出 `bluez-daemon: a2dp upstream-endpoint-callback-wrapper`，并检查 `callbacks=get_name:1,get_path:1,get_capabilities:1,select_configuration:1,select_cb:1,set_configuration:1,set_cb:1,clear_configuration:1,set_delay:1`、`capability-bytes=12`、`selected-bytes=12`、`delay=120`、`final-ok=1`。

当前含义：A2DP upstream-facing 层已从“public headers 可编译”推进到 “public endpoint callback table 可执行”。边界仍明确为 `boundary=upstream-a2dp-endpoint-callbacks-compiled-not-yet-a2dp-c-object` 和 `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd`；下一步继续把 upstream `a2dp.c` 中的 setup/session/SEP owner 逐步拆入 compat wrapper，最终替换当前 `bluezdaemon` staged A2DP object/session owner。

## 2026-06-14 A2DP upstream header compat wrapper closeout

继续 A2DP 去 staged adapter 化，本轮在 upstream source manifest 之后新增可编译的 upstream-facing A2DP compat wrapper。

A2DP upstream convergence + header compat wrapper：PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-upstream-compat-v2.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-upstream-compat-v2.log`
- run: `FeatherCore/build/logs/run-a2dp-upstream-compat-v2.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-compat-v2/run-results.json`
- roles: `bt1` source, `bt2` sink

实现改动：新增 `FeatherCore/apps/wireless/linux_bluetooth/bluez/upstream_a2dp_compat.c` 和 `upstream_a2dp_compat.h`，并接入 `CONFIG_LINUX_BLUEZ_DAEMON` 的 Makefile/CMake 构建。该编译单元直接 include `bluez/upstream/profiles/audio/a2dp.h` 和 `bluez/upstream/profiles/audio/avdtp.h`，为 NuttX apps 侧补齐最小 GLib/BlueZ 前置类型 `gboolean`、`GSList`、`GIOChannel`、`GDestroyNotify`、`btd_adapter`、`btd_device`、`queue`。

新增验证门槛：validator 现在要求 source/sink 两端输出 `bluez-daemon: a2dp upstream-compat-wrapper`，并检查 upstream header 里的 callback surface、状态常量、SEP 类型、capability 类型和 error code：`a2dp_endpoint_select_t`、`a2dp_endpoint_config_t`、`a2dp_discover_cb_t`、`a2dp_select_cb_t`、`a2dp_config_cb_t`、`a2dp_stream_cb_t`、`avdtp_session_state_cb`、`avdtp_stream_state_cb`、`avdtp_set_configuration_cb`，以及 `AVDTP_STATE_IDLE/CONFIGURED/OPEN/STREAMING/CLOSING/ABORTING`、`AVDTP_SEP_TYPE_SOURCE/SINK`、`AVDTP_MEDIA_TRANSPORT`、`AVDTP_MEDIA_CODEC`、`AVDTP_DELAY_REPORTING`、`AVDTP_BAD_STATE`、`A2DP_INVALID_CODEC_TYPE`、`A2DP_NOT_SUPPORTED_CODEC_TYPE`、`A2DP_INVALID_CODEC_PARAMETER`。

当前含义：A2DP gate 已从 source mirror/manifest 继续推进到直接编译 upstream A2DP/AVDTP public headers，证明 NuttX apps 侧已有可编译的 upstream-facing callback/type/constant 兼容层。边界仍明确为 `boundary=upstream-headers-compiled-not-yet-upstream-c-object` 和 `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd`；下一步应继续把 `a2dp.c`/`avdtp.c` 的实际 `.c` 对象逐步拆出可编译 compat wrapper，并让真实 callback/session owner 接管更多路径。

## 2026-06-14 A2DP upstream source manifest closeout

继续 A2DP 去 staged adapter 化，本轮确认 `FeatherCore/apps/wireless/linux_bluetooth/bluez/upstream` 是指向 `third/bluez` 的符号链接，并把 A2DP upstream source manifest 编译进 `bluezdaemon`。

A2DP upstream convergence + source manifest：PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-upstream-manifest.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-upstream-manifest.log`
- run: `FeatherCore/build/logs/run-a2dp-upstream-manifest.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-manifest/run-results.json`
- roles: `bt1` source, `bt2` sink

实现改动：新增 `FeatherCore/apps/wireless/linux_bluetooth/bluez/upstream_manifest.c` 和 `upstream_manifest.h`，并接入 `CONFIG_LINUX_BLUEZ_DAEMON` 的 Makefile/CMake 构建。`bluezdaemon audio-a2dp-closeout-full` 现在会输出 `bluez-daemon: a2dp upstream-source-manifest`，validator 强制检查 source/sink 两端都看到 `apps-link=bluez/upstream target=third/bluez audio-files=20 core-files=9 compile-unit=bluez/upstream_manifest.c`。

manifest 覆盖的 upstream A2DP/audio 文件包括 `a2dp.c`、`a2dp.h`、`a2dp-codecs.h`、`avdtp.c`、`avdtp.h`、`avctp.c`、`avctp.h`、`avrcp.c`、`avrcp.h`、`avrcp-player.c`、`media.c`、`media.h`、`transport.c`、`transport.h`、`source.c`、`source.h`、`sink.c`、`sink.h`、`player.c`、`player.h`。core 文件包括 `src/main.c`、`src/plugin.c`、`src/profile.c`、`src/device.c`、`src/adapter.c`、`src/dbus-common.c`、`src/sdpd-service.c`、`src/shared/mainloop.c`、`src/shared/io-mainloop.c`。

当前含义：A2DP gate 不再只依赖 `daemon_main.c` 中的 source 字符串，而是要求 `bluezdaemon` 的编译单元显式绑定 apps 侧 upstream BlueZ source mirror。边界仍明确为 `boundary=source-mirror-not-yet-unmodified-plugin` 和 `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd`，因此还不能宣称 unmodified upstream `bluetoothd` audio plugin 已直接运行。下一步继续把 manifest 中的 upstream audio plugin 入口逐步从 source mirror/ledger 推进到可编译 compat wrapper 和真实 callback/session owner。

## 2026-06-14 A2DP upstream daemon ownership ledger closeout

在 BT/BLE NET iperf matrix 收口之后，本轮回到 A2DP，继续把 `bluez-a2dp-upstream-convergence-closeout` 从“子路径覆盖”加严到 “daemon ownership 总账”覆盖。

A2DP upstream convergence + ownership ledger：PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-ownership-ledger.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-ownership-ledger.log`
- run: `FeatherCore/build/logs/run-a2dp-ownership-ledger.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-ownership-ledger/run-results.json`
- roles: `bt1` source, `bt2` sink

新增验证门槛：`bluezdaemon audio-a2dp-closeout-full` 现在输出并由 validator 强制检查 `bluez-daemon: a2dp closeout upstream-daemon-ownership-ledger`。该 ledger 覆盖 `profile,device,session,stream,media-transport,avrcp-player,l2cap-fd,dbus-name,mainloop-watch` 的 bluetoothd direct owner，并要求两端同时满足：`profile-register=1/profile-unregister=1`、`device-connect=2/device-disconnect=2`、`dbus-name-acquire=1/dbus-name-release=1`、`dbus-owner-lost=1/dbus-owner-reacquire=1`、`mainloop-watch-add=7/mainloop-watch-remove=7`、`mainloop-timer-add=2/mainloop-timer-remove=2`、`avdtp-transactions=12/avdtp-complete=12`、`transport-acquire=2/transport-release=2`、`fd-open=2/fd-close=2`、`zero-ref-rounds=2/rounds=2`，最终 `final-profile-registered=0`、`final-device-ref=0`、`final-session-ref=0`、`final-stream-ref=0`、`final-sep-ref=0`、`final-endpoint-refs=0`、`final-transport-refs=0`、`final-player-refs=0`、`final-dbus-owners=0`、`final-interfaces=0`、`final-mainloop-watches=0`、`final-mainloop-timers=0`、`final-l2cap-fds=0`、`final-media-fd=closed`、`final-transaction-pending=0`、`final-state-errors=0`、`final-ok=1`。

实现改动：`FeatherCore/apps/wireless/linux_bluetooth/bluez/tools/daemon_main.c` 增加 A2DP upstream daemon ownership ledger 计算和输出；`FeatherCore/tools/firmware/sim/validate-bt-hwsim-usecases.py` 对 `bluez-a2dp-upstream-convergence-closeout` 加严，要求 source/sink 两端都出现 ledger 并满足最终归零条件。

边界仍保留：ledger 仍标记 `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd`。因此当前结论是：A2DP 的 daemon/mainloop/D-Bus/object/fd/ref 生命周期审计更强了，但还不能宣称 unmodified upstream `bluetoothd` audio plugin 已完全无 adapter 运行。下一步继续 A2DP 去 staged adapter 化，优先让 upstream `bluetoothd` audio plugin 入口直接拥有 mainloop、D-Bus object、MediaTransport fd 和 AVDTP/L2CAP session。

## 2026-06-14 BT/BLE NET iperf matrix closeout

在 LE Audio full-role、BT/BLE basic、A2DP upstream convergence、BT/BLE NET current closeout 之后，本轮继续补跑并收口 BT Network/BNEP 的 iperf matrix。

BT Network/BNEP iperf matrix：PASS

- case: `bluez-network-iperf-matrix`
- build: `FeatherCore/build/logs/build-bt1-bluez-network-iperf-matrix-final-v3.log`
- build: `FeatherCore/build/logs/build-bt2-bluez-network-iperf-matrix-final-v3.log`
- run: `FeatherCore/build/logs/run-bluez-network-iperf-matrix-final-v3.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-bluez-network-iperf-matrix-final-v3/run-results.json`
- roles: `bt1`, `bt2`

验证结果：`run-results.json` 显示 `passed=true`、`validate_rc=0`，runner 输出 `PASS bluez-network-iperf-matrix`。BT1/BT2 均完成 BlueZ Network Profile-shaped BNEP/PAN 多轮 TCP/UDP 正反向 iperf，日志中出现非零吞吐，例如约 `0.35` 到 `0.69 Mbits/sec`，并在 native BNEP path 中看到 `bnep-native-sock-ioctl-connadd=4`、`bnep-native-kthread-run=4`、`bnep-native-session-rx-dequeue`、`bnep-native-ndo-start-xmit`、`bnep-native-l2cap-delivered`、`bnep-native-netif-rx` 等非零计数。最终 cleanup 看到 `bnep-native-active=0`、`bnep-native-session-stop=4`、`bnep-native-session-terminate=4`、`bnep-native-netdev-unregister=4`。

实现补充：`linux_bt_upstream_af_status()` 将 `bnep-native-sock-ioctl-connadd/conndel` 提前输出到 BNEP socket ioctl 摘要段，避免长状态行后部被截断导致验证器看不到已经发生的 native BNEP ioctl 证据。该改动不降低验证门槛，只修正证据可见性。

当前阶段结论：LE Audio 当前全角色 gate、BT/BLE basic gate、A2DP upstream convergence gate、BT/BLE NET current closeout、BLE IP ping、BT Network/BNEP iperf matrix 均已通过。下一阶段按用户指定顺序继续补 A2DP 的更深 upstream daemon/mainloop/D-Bus ownership，再补 BT/BLE NET 的 unmodified upstream plugin 收敛，最后补剩余全部 BT/BLE profiles/security/policy。仍保留 staged adapter 边界，不声明整套 upstream Linux/BlueZ 蓝牙栈已经完全无 adapter 移植完。

## 2026-06-14 BT/BLE NET current closeout: BLE IP ping + BlueZ Network/IPSP

本轮进入 BT/BLE NET 阶段，优先验证目标清单里原先标记未完成的 BLE1 <-> BLE2 IP ping，然后运行当前 BT/BLE NET 四角色 closeout。

BLE IP ping：PASS

- case: `ble-ip-ping`
- build: `FeatherCore/build/logs/build-ble1-ble-ip-ping-final.log`
- build: `FeatherCore/build/logs/build-ble2-ble-ip-ping-final.log`
- run: `FeatherCore/build/logs/run-ble-ip-ping-final.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-ble-ip-ping-final/run-results.json`
- roles: `ble1`, `ble2`
- datapath: `bt0`, `fc00::1 <-> fc00::2`, `ping6 -c 2`, `0% packet loss`

BLE 6LoWPAN/IPSP 证据：两端状态包含 `linux-bt-6lowpan: registered=1 ifname=bt0`、`ipsp-state=open`、`upstream-owner=net_bluetooth/6lowpan+sim-ipsp-datapath-owner`、`upstream-iphc-owner=net_6lowpan/iphc`，ping 后 `upstream-owner-xmit/rx-deliver/bt-xmit/recv-cb` 和 `tx-iphc/rx-iphc` 均有非零计数，down 后回到 `registered=0`、`ipsp-state=closed`、owner refs 为 0。

BT/BLE NET current closeout：PASS

- case: `bluez-net-current-complete-closeout`
- build: `FeatherCore/build/logs/build-bt1-net-current-final.log`
- build: `FeatherCore/build/logs/build-bt2-net-current-final.log`
- run: `FeatherCore/build/logs/run-net-current-final.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-net-current-final/run-results.json`
- roles: `bt1`, `bt2`, `ble1`, `ble2`

BT BNEP/BlueZ Network 证据：BT1/BT2 两端通过 `blueznetwork daemon-profile` 覆盖 PANU/NAP/GN role lifecycle，`btn0` 上完成 `10.77.0.1 <-> 10.77.0.2` ping，包含 1400-byte payload ping，`0% packet loss`。日志中 native BNEP path 有 `bnep-native-session-thread`、`bnep-native-kthread-run`、`bnep-native-ndo-start-xmit`、`bnep-native-netdev-xmit`、`bnep-native-tx-frame-ok`、`bnep-native-l2cap-delivered`、`bnep-native-rx-frame-ok`、`bnep-native-netif-rx` 非零计数，且 `bnep-staging-active=0`、cleanup 后 `bnep-native-active=0`。

BlueZ-facing 边界：BT Network coverage map 仍标记 `staged-boundary=blueznetwork-adapter-not-unmodified-bluetoothd`，BLE IPSP coverage map 仍标记 `staged-boundary=bluezdaemon-ipsp-adapter-not-unmodified-bluetoothd`。因此当前结论是：BT/BLE NET 当前 hwsim semantic closeout 已通过，BLE IP ping 旧缺口已收口，但仍不是 unmodified upstream `bluetoothd` Network/IPSP plugin 完全无 adapter 运行。下一步若继续 NET，应优先验证或修复 BT/BLE iperf TCP/UDP matrix 和更长时间生命周期；若回到“完整移植”目标，则要拆除 `blueznetwork`/`bluezdaemon ipsp` staged adapter，改为 upstream daemon/profile 入口直接拥有 fd、D-Bus object 和 session lifecycle。

## 2026-06-14 A2DP upstream convergence baseline + CMake apps entry closeout

本轮在 LE Audio final closeout 和 BT/BLE basic closeout 之后，按顺序重新验证 A2DP 当前最强 upstream convergence gate，并补齐 BlueZ apps 侧 CMake 注册缺口。

A2DP upstream convergence：PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-upstream-final.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-upstream-final.log`
- run: `FeatherCore/build/logs/run-a2dp-upstream-final.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-final/run-results.json`
- roles: `bt1` source, `bt2` sink

验证证据包括：A2DP profile/device/SDP lifecycle、D-Bus object lifecycle、mainloop watch/dispatch lifecycle、AVDTP transaction/state machine、MediaTransport fd ownership、SBC codec datapath、AVRCP control/browsing、L2CAP channel ownership、error policy 和两轮 cleanup；source/sink 两端均输出 `final-ok=1`。

构建入口收口：`FeatherCore/apps/wireless/linux_bluetooth/CMakeLists.txt` 已补齐 `CONFIG_LINUX_BLUEZ_NETWORK`、`CONFIG_LINUX_BLUEZ_DAEMON`、`CONFIG_LINUX_BLUEZ_AUDIO` 三个 apps 注册项，并为 `bluezaudio` 补上 SBC/LC3 backend 源与 include flags，使 BlueZ-facing network/daemon/audio apps 不再只覆盖 Makefile 构建路径。

仍保留边界：A2DP gate 明确包含 `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd`。这说明当前 A2DP 已有强 hwsim semantic convergence baseline，但还不是 unmodified upstream `bluetoothd` audio plugin 在 NuttX 上完全无 adapter 运行。下一步进入 BT/BLE NET 前，A2DP 的真实剩余工作是拆除 `bluezdaemon` staged adapter，改为 upstream `bluetoothd` mainloop/D-Bus/profile/audio plugin 入口直接拥有对象和 fd lifecycle。

## 2026-06-14 LE Audio final closeout + BT/BLE basic closeout

本轮按优先级重新收口两个阶段：先验证 LE Audio 当前阶段全功能 umbrella，再验证 BT/BLE 基础能力四角色 upstream convergence gate。

LE Audio final closeout：PASS

- case: `bluez-le-audio-umbrella`
- build: `FeatherCore/build/logs/build-ble1-le-audio-final-closeout.log`
- build: `FeatherCore/build/logs/build-ble2-le-audio-final-closeout.log`
- run: `FeatherCore/build/logs/run-le-audio-final-closeout.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-le-audio-final-closeout/run-results.json`
- roles: `ble1`, `ble2`

BT/BLE basic closeout：PASS

- case: `bluez-basic-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-basic-final-closeout.log`
- build: `FeatherCore/build/logs/build-bt2-basic-final-closeout.log`
- run: `FeatherCore/build/logs/run-basic-final-closeout.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-basic-final-closeout/run-results.json`
- roles: `bt1`, `bt2`, `ble2`, `ble1`

当前结论：LE Audio 当前阶段 full-role umbrella 已重新验证通过，BT/BLE 基础扫描、连接、认证、错误路径和基础 BR/EDR/BLE 链路也已四角色通过。下一阶段按顺序继续补完整 A2DP，再补 BT/BLE NET；仍保留 staged adapter 边界说明，不声明 unmodified upstream `bluetoothd` / kernel Bluetooth stack 已完全无适配运行。

## 2026-06-14 BLE Ranging/RAP profile closeout

Added `bluezdaemon profile-ranging-closeout initiator|reflector [peer]` and the BLE two-role hwsim gate `bluez-ranging-profile-closeout`.

```text
case:     bluez-ranging-profile-closeout
roles:    ble1, ble2
run:      build/logs/run-bluez-ranging-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-ranging-profile-closeout/run-results.json
result:   PASS
```

The gate covers ranging capability, security, procedure config, procedure start/request, distance/result, event stream, bad-config/timeout/poor-quality/security-fail recovery, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-ranging-adapter-not-unmodified-bluetoothd`. It is current hwsim semantic closeout evidence for the BLE Ranging/RAP profile slice, not proof that unmodified upstream `bluetoothd` Ranging profile runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. BLE Mesh profile closeout: PASS.
11. Generic BLE GATT/Application Services closeout: PASS.
12. BLE ASHA/Hearing Aid profile closeout: PASS.
13. Classic OBEX BIP/Imaging profile closeout: PASS.
14. Classic CUPS/HCRP/SPP printing profile closeout: PASS.
15. Classic iAP accessory profile closeout: PASS.
16. BLE MIDI profile closeout: PASS.
17. BLE Ranging/RAP profile closeout: PASS.
18. Continue deeper upstream replacement and staged-adapter removal.

## 2026-06-14 BLE MIDI profile closeout

Added `bluezdaemon profile-midi-closeout controller|peripheral [peer]` and the BLE two-role hwsim gate `bluez-midi-profile-closeout`.

```text
case:     bluez-midi-profile-closeout
roles:    ble1, ble2
run:      build/logs/run-bluez-midi-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-midi-profile-closeout/run-results.json
result:   PASS
```

The gate covers BLE MIDI service discovery, MIDI I/O characteristic, write-without-response, notify/CCC, timestamp encode/decode/wrap, note/control payload, jitter/drop/error policy, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-midi-adapter-not-unmodified-bluetoothd`. It is current hwsim semantic closeout evidence for the BLE MIDI profile slice, not proof that unmodified upstream `bluetoothd` MIDI profile runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. BLE Mesh profile closeout: PASS.
11. Generic BLE GATT/Application Services closeout: PASS.
12. BLE ASHA/Hearing Aid profile closeout: PASS.
13. Classic OBEX BIP/Imaging profile closeout: PASS.
14. Classic CUPS/HCRP/SPP printing profile closeout: PASS.
15. Classic iAP accessory profile closeout: PASS.
16. BLE MIDI profile closeout: PASS.
17. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 Classic iAP accessory profile closeout

Added `bluezdaemon profile-iap-closeout controller|accessory [peer]` and the BT two-role hwsim gate `bluez-iap-profile-closeout`.

```text
case:     bluez-iap-profile-closeout
roles:    bt1, bt2
run:      build/logs/run-bluez-iap-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-iap-profile-closeout/run-results.json
result:   PASS
```

The gate covers SDP/SPP service, RFCOMM session, device/accessory identify, external accessory session, control payload, link keepalive/retransmit/flow-control, bad-session/checksum/timeout error recovery, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-iap-adapter-not-unmodified-iapd`. It is current hwsim semantic closeout evidence for the iAP profile slice, not proof that unmodified upstream `iapd` runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. BLE Mesh profile closeout: PASS.
11. Generic BLE GATT/Application Services closeout: PASS.
12. BLE ASHA/Hearing Aid profile closeout: PASS.
13. Classic OBEX BIP/Imaging profile closeout: PASS.
14. Classic CUPS/HCRP/SPP printing profile closeout: PASS.
15. Classic iAP accessory profile closeout: PASS.
16. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 Classic CUPS/HCRP/SPP printing profile closeout

Added `bluezdaemon profile-print-closeout client|printer [peer]` and the BT two-role hwsim gate `bluez-print-profile-closeout`.

```text
case:     bluez-print-profile-closeout
roles:    bt1, bt2
run:      build/logs/run-bluez-print-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-print-profile-closeout/run-results.json
result:   PASS
```

The gate covers SDP HCRP/SPP service, RFCOMM session, HCRP control/data channels, CUPS backend discovery, print job submit/receive/render/status/cancel/error, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-print-adapter-not-unmodified-cups-backend`. It is current hwsim semantic closeout evidence for the CUPS/HCRP/SPP printing profile slice, not proof that unmodified upstream CUPS backend/HCRP profile runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. BLE Mesh profile closeout: PASS.
11. Generic BLE GATT/Application Services closeout: PASS.
12. BLE ASHA/Hearing Aid profile closeout: PASS.
13. Classic OBEX BIP/Imaging profile closeout: PASS.
14. Classic CUPS/HCRP/SPP printing profile closeout: PASS.
15. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 Classic OBEX BIP/Imaging profile closeout

Added `bluezdaemon profile-bip-closeout client|server [peer]` and the BT two-role hwsim gate `bluez-obex-bip-profile-closeout`.

```text
case:     bluez-obex-bip-profile-closeout
roles:    bt1, bt2
run:      build/logs/run-bluez-obex-bip-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-obex-bip-profile-closeout/run-results.json
result:   PASS
```

The gate covers BIP SDP, RFCOMM transport, OBEX session, image capabilities, image put/get, thumbnail get, transfer progress, abort/error mapping, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-bip-obex-adapter-not-unmodified-obexd`. It is current hwsim semantic closeout evidence for the OBEX BIP/Imaging profile slice, not proof that unmodified upstream `obexd` BIP client/server runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. BLE Mesh profile closeout: PASS.
11. Generic BLE GATT/Application Services closeout: PASS.
12. BLE ASHA/Hearing Aid profile closeout: PASS.
13. Classic OBEX BIP/Imaging profile closeout: PASS.
14. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 BLE ASHA/Hearing Aid profile closeout

Added `bluezdaemon profile-asha-closeout central|hearing-aid [peer]` and the BLE two-role hwsim gate `bluez-asha-profile-closeout`.

```text
case:     bluez-asha-profile-closeout
roles:    ble1, ble2
run:      build/logs/run-bluez-asha-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-asha-profile-closeout/run-results.json
result:   PASS
```

The gate covers ASHA service discovery, GATT control/status characteristics, paired hearing-aid side/hi-sync-id, G.722 stream start/accept, audio payload/status, volume/suspend/resume/stop, battery notification, error/reconnect, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-asha-adapter-not-unmodified-bluetoothd`. It is current hwsim semantic closeout evidence for the ASHA/Hearing Aid profile slice, not proof that unmodified upstream `bluetoothd` ASHA profile runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. BLE Mesh profile closeout: PASS.
11. Generic BLE GATT/Application Services closeout: PASS.
12. BLE ASHA/Hearing Aid profile closeout: PASS.
13. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 Generic BLE GATT/Application Services closeout

Added `bluezdaemon profile-gatt-closeout client|server [peer]` and the BLE two-role hwsim gate `bluez-gatt-profile-closeout`.

```text
case:     bluez-gatt-profile-closeout
roles:    ble1, ble2
run:      build/logs/run-bluez-gatt-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-gatt-profile-closeout/run-results.json
result:   PASS
```

The gate covers GATT application registration, GAP/BAS/DIS/SCPP/custom services, ATT MTU/security, service discovery, read/write, prepare/execute write, CCC, notify, indicate/confirm, long/offset read, error policy, reconnect, unregister, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-gatt-adapter-not-unmodified-bluetoothd`. It is current hwsim semantic closeout evidence for the generic GATT application services slice, not proof that unmodified upstream `bluetoothd` GATT database/client runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. BLE Mesh profile closeout: PASS.
11. Generic BLE GATT/Application Services closeout: PASS.
12. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 BLE Mesh profile closeout

Added `bluezdaemon profile-mesh-closeout provisioner|node [peer]` and the BLE two-role hwsim gate `bluez-mesh-profile-closeout`.

```text
case:     bluez-mesh-profile-closeout
roles:    ble1, ble2
run:      build/logs/run-bluez-mesh-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-mesh-profile-closeout/run-results.json
result:   PASS
```

The gate covers mesh daemon init, mgmt ADV/GATT Proxy bearer, PB-ADV provisioning, NetKey/AppKey/DevKey, config client/server, Generic OnOff model messages, lower/upper transport, segmentation/reassembly, relay, friend/LPN, proxy filter/PDU, secure/private beacon, heartbeat, RPL/replay protection, error recovery, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-mesh-adapter-not-unmodified-bluetoothd`. It is current hwsim semantic closeout evidence for the BLE Mesh profile slice, not proof that unmodified upstream `bluetooth-meshd` runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. BLE Mesh profile closeout: PASS.
11. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 Classic OBEX FTP/Sync profile closeout

Added `bluezdaemon profile-sync-closeout ftp-client|ftp-server|sync-client|sync-server [peer]` and the BT two-role hwsim gate `bluez-obex-ftp-sync-profile-closeout`.

```text
case:     bluez-obex-ftp-sync-profile-closeout
roles:    bt1, bt2
run:      build/logs/run-bluez-obex-ftp-sync-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-obex-ftp-sync-profile-closeout/run-results.json
result:   PASS
```

The gate covers FTP SDP, RFCOMM transport, OBEX session, set-folder, folder listing, get file, put file, delete file, transfer progress, abort/error mapping, and cleanup. It also covers Sync SDP, RFCOMM transport, OBEX session, phonebook sync, calendar sync, notes sync, transfer progress, abort/error mapping, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-ftp-obex-adapter-not-unmodified-obexd` and `staged-boundary=bluezdaemon-sync-obex-adapter-not-unmodified-obexd`. It is current hwsim semantic closeout evidence for the OBEX FTP/Sync profile slice, not proof that unmodified upstream `obexd` runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Classic OBEX FTP/Sync profile closeout: PASS.
10. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 Classic OBEX MAP/MNS profile closeout

Added `bluezdaemon profile-map-closeout map-client|map-server|mns-client|mns-server [peer]` and the BT two-role hwsim gate `bluez-obex-map-mns-profile-closeout`.

```text
case:     bluez-obex-map-mns-profile-closeout
roles:    bt1, bt2
run:      build/logs/run-bluez-obex-map-mns-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-obex-map-mns-profile-closeout/run-results.json
result:   PASS
```

The gate covers MAP MAS SDP, RFCOMM transport, OBEX session, set-folder, message listing, message get, message status update, push message, transfer progress, abort/error mapping, and cleanup. It also covers MNS SDP, RFCOMM transport, OBEX session, NewMessage/DeliverySuccess/MessageDeleted event reports, abort/error mapping, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-map-obex-adapter-not-unmodified-obexd` and `staged-boundary=bluezdaemon-mns-obex-adapter-not-unmodified-obexd`. It is current hwsim semantic closeout evidence for the OBEX MAP/MNS profile slice, not proof that unmodified upstream `obexd` runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Classic OBEX MAP/MNS profile closeout: PASS.
9. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 Classic OBEX PBAP/OPP profile closeout

Added `bluezdaemon profile-obex-closeout pbap-client|pbap-server|opp-client|opp-server [peer]` and the BT two-role hwsim gate `bluez-obex-pbap-opp-profile-closeout`.

```text
case:     bluez-obex-pbap-opp-profile-closeout
roles:    bt1, bt2
run:      build/logs/run-bluez-obex-pbap-opp-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-obex-pbap-opp-profile-closeout/run-results.json
result:   PASS
```

The gate covers PBAP PCE/PSE SDP, RFCOMM transport, OBEX session, phonebook select, pull phonebook, pull vCard listing, pull vCard entry, transfer progress, abort/error mapping, and cleanup. It also covers OPP client/server SDP, RFCOMM transport, OBEX session, object push, capability query, transfer progress, abort/error mapping, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-pbap-obex-adapter-not-unmodified-obexd` and `staged-boundary=bluezdaemon-opp-obex-adapter-not-unmodified-obexd`. It is current hwsim semantic closeout evidence for the OBEX/PBAP/OPP profile slice, not proof that unmodified upstream `obexd` runs without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Classic OBEX PBAP/OPP profile closeout: PASS.
8. Continue remaining profiles and deeper upstream replacement.

## 2026-06-15 HFP/HSP independent profile surface

新增 `bluezhfp` apps 工具，把 HFP/HSP 从 `bluezdaemon
profile-hfp-closeout` 聚合 marker 继续拆成独立 BlueZ profile-shaped
入口。

```text
bluezhfp closeout hfp-hf 2
bluezhfp closeout hfp-ag 1
bluezhfp closeout hsp-hs 2
bluezhfp closeout hsp-ag 1
```

`bluez-hfp-hsp-profile-closeout` 现在同时要求旧 `bluezdaemon` evidence 和
新的 `bluezhfp` evidence。

New `bluezhfp` evidence:

- BlueZ `profiles/audio/hfp-hf.c` / `hfp-ag.c` / `headset.c` profile connect
  surface.
- BlueZ `src/profile.c` SDP/Profile1 registration surface.
- RFCOMM-shaped bearer over Linux-BT L2CAP PSM `0x0003`, HFP CID `0x0061`,
  and HSP CID `0x0062`.
- HFP AT service-level transactions: `BRSF`, `BAC/BCS`, `CLCC`.
- HSP AT transactions: `CKPD`, `VGS/VGM`.
- SCO-like audio payload via Linux-BT HCI SCO packet path and
  `profiles/audio/transport.c` acquire marker.
- Cleanup to `rfcomm=0 sco=0 calls=0 transport=0`.

Boundary: this is an independent apps-side HFP/HSP profile surface and
validator hardening step.  It still reports
`staged-boundary=bluezhfp-hfp-adapter-not-unmodified-bluetoothd` and
`staged-boundary=bluezhfp-hsp-adapter-not-unmodified-bluetoothd`; it does not
claim unmodified upstream `bluetoothd` HFP/HSP internals or a full native
`BTPROTO_RFCOMM`/SCO socket dataplane are complete.

## 2026-06-14 Classic HFP/HSP profile closeout

Added `bluezdaemon profile-hfp-closeout hfp-hf|hfp-ag|hsp-hs|hsp-ag [peer]` and the BT two-role hwsim gate `bluez-hfp-hsp-profile-closeout`.

```text
case:     bluez-hfp-hsp-profile-closeout
roles:    bt1, bt2
run:      build/logs/run-bluez-hfp-hsp-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-hfp-hsp-profile-closeout/run-results.json
result:   PASS
```

The gate covers HFP HF/AG SDP/Profile1, RFCOMM session ownership, AT service-level connection, indicators, call-control, codec negotiation, mSBC/CVSD SCO audio bearer, volume, and cleanup. It also covers HSP HS/AG SDP/Profile1, RFCOMM, CKPD/RING/VGS/VGM control, CVSD SCO audio bearer, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-hfp-adapter-not-unmodified-bluetoothd` and `staged-boundary=bluezdaemon-hsp-adapter-not-unmodified-bluetoothd`. It is current hwsim semantic closeout evidence for the headset/telephony profile slice, not proof that unmodified upstream `bluetoothd` HFP/HSP internals run without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Classic HFP/HSP profile closeout: PASS.
7. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 Classic HID + BLE HOGP profile closeout

Added `bluezdaemon profile-hid-closeout classic-host|classic-device|hogp-host|hogp-device [peer]` and the four-role hwsim gate `bluez-hid-hogp-profile-closeout`.

```text
case:     bluez-hid-hogp-profile-closeout
roles:    bt1, bt2, ble2, ble1
run:      build/logs/run-bluez-hid-hogp-profile-closeout.log
manifest: build/bt-hwsim-usecases-bluez-hid-hogp-profile-closeout/run-results.json
result:   PASS
```

The gate covers Classic HID SDP/Profile1, L2CAP control and interrupt channels, HIDP connadd, input/output reports, and cleanup. It also covers BLE HOGP ATT/GATT discovery, report map, protocol mode, boot reports, CCC notification, input/output reports, suspend/resume, and cleanup.

Boundary: this preserves `staged-boundary=bluezdaemon-input-adapter-not-unmodified-bluetoothd` and `staged-boundary=bluezdaemon-hogp-adapter-not-unmodified-bluetoothd`. It is current hwsim semantic closeout evidence for the input profile slice, not proof that unmodified upstream `bluetoothd` input internals run without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Classic HID + BLE HOGP profile closeout: PASS.
6. Continue remaining profiles and deeper upstream replacement.

## 2026-06-14 BT/BLE NET upstream convergence rerun after A2DP

After BT/BLE basic and A2DP upstream convergence passed, `bluez-net-upstream-convergence-closeout` was re-run across all four roles.

```text
case:     bluez-net-upstream-convergence-closeout
roles:    bt1, bt2, ble1, ble2
run:      build/logs/run-bluez-net-upstream-convergence-after-a2dp.log
manifest: build/bt-hwsim-usecases-bluez-net-upstream-convergence-after-a2dp/run-results.json
result:   PASS
```

The gate covers BT Network/BNEP PANU/NAP/GN roles, NetworkServer1 registration, BlueZ-style L2CAP fd handoff, BNEPCONNADD, btn0 ping and MTU1400 ping, BNEP native datapath counters, BLE IPSP/6LoWPAN Profile1/mainloop, LE L2CAP CoC fd handoff, bt0 ping6, TCP/UDP iperf, IPHC/fragment counters, and final cleanup.

Boundary: this preserves `staged-boundary=blueznetwork-adapter-not-unmodified-bluetoothd` and `staged-boundary=bluezdaemon-ipsp-adapter-not-unmodified-bluetoothd`. It is current NET semantic closeout evidence, not proof that unmodified upstream `bluetoothd` Network plugin and Linux kernel BNEP/6LoWPAN run entirely without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. BT/BLE NET upstream convergence: after-A2DP rerun PASS.
5. Continue remaining BT/BLE profiles and deeper upstream replacement.

## 2026-06-14 A2DP upstream convergence rerun after BT/BLE basic

After the BT/BLE basic closeout passed, `bluez-a2dp-upstream-convergence-closeout` was re-run against the current BT1/BT2 sim binaries.

```text
case:     bluez-a2dp-upstream-convergence-closeout
roles:    bt1, bt2
run:      build/logs/run-bluez-a2dp-upstream-convergence-after-basic.log
manifest: build/bt-hwsim-usecases-bluez-a2dp-upstream-convergence-after-basic/run-results.json
result:   PASS
```

The gate continues to require source/sink `audio-a2dp-closeout-full`, `a2dp closeout upstream-coverage-map ... final-ok=1`, zero cleanup refs/fds/watches, BlueZ profile/D-Bus/mainloop/SDP/A2DP/AVDTP/AVRCP/MediaTransport/SBC source evidence, and Linux L2CAP source evidence.

Boundary: this preserves `staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd`. It is the current A2DP semantic closeout evidence after the basic phase, not proof that unmodified upstream `bluetoothd` audio internals are fully running without the NuttX adapter harness.

Current work order:

1. LE Audio full-role/GATT evidence: current phase complete.
2. BT/BLE basic upstream convergence: current phase complete.
3. A2DP upstream convergence: after-basic rerun PASS.
4. Resume BT/BLE NET completion next.
5. Finish remaining BT/BLE profiles and deeper upstream replacement.

## 2026-06-14 BT/BLE basic upstream convergence closeout

新增 `bluez-basic-upstream-convergence-closeout`，用于把基础扫描、连接、认证、重连和错误路径从分散 gate 收束到一个四角色 hwsim closeout。

```text
case:     bluez-basic-upstream-convergence-closeout
roles:    bt1, bt2, ble2, ble1
build:    build/logs/build-bt1-basic-upstream-convergence.log
build:    build/logs/build-bt2-basic-upstream-convergence.log
build:    build/logs/build-ble1-basic-upstream-convergence.log
build:    build/logs/build-ble2-basic-upstream-convergence.log
run:      build/logs/run-bluez-basic-upstream-convergence-closeout.log
manifest: build/bt-hwsim-usecases-bluez-basic-upstream-convergence-closeout/run-results.json
result:   PASS
```

`bluezdaemon basic-closeout bt|ble` 现在输出 basic closeout cleanup 和 upstream coverage map。该 gate 要求 BT1/BT2 完成 BR/EDR basic link 和 L2CAP echo，BLE2 提供 advertising，BLE1 通过 BlueZ daemon/btmgmt 执行 discovery-peer、control、lifecycle、pair-noio、pairing matrix、reconnect-stress、device-policy 和 error-path。

Boundary: this is the current NuttX hwsim semantic closeout for BT/BLE basic behavior. It preserves `staged-boundary=bluezdaemon-basic-adapter-not-unmodified-bluetoothd`; it is not a claim that all upstream `bluetoothd` objects and all Linux Bluetooth kernel internals are fully unmodified.

Current work order:

1. LE Audio full-function gates: covered by the previous umbrella/GATT evidence.
2. BT/BLE basic closeout: covered by this gate.
3. Resume A2DP completion.
4. Resume BT/BLE NET completion.
5. Finish remaining BT/BLE profiles and deeper upstream replacement.

## 2026-06-15 HID/HOGP independent profile surface

新增 `bluezhid` apps 工具，开始把 HID/HOGP 从 `bluezdaemon
profile-hid-closeout` 聚合 marker 中拆出来，形成独立的 BlueZ
profile-shaped 验证入口。

```text
bluezhid closeout classic-host 2
bluezhid closeout classic-device 1
bluezhid closeout hogp-host 4
bluezhid closeout hogp-device 3
```

新增 hwsim case:

```text
case:  bluez-hid-upstream-convergence-closeout
roles: bt1, bt2, ble1, ble2
```

Classic HID evidence:

- BlueZ `profiles/input/device.c` connect/profile surface.
- BlueZ `profiles/input/server.c` SDP record registration surface.
- Linux-BT L2CAP control PSM `0x0011` / CID `0x0051`.
- Linux-BT L2CAP interrupt PSM `0x0013` / CID `0x0053`.
- HID control set-protocol and interrupt input-report payload.
- Linux `hidp/core.c` / `hidp/sock.c` session ownership markers.
- Cleanup to `control=0 interrupt=0 hidp=0 input=0`.

HOGP evidence:

- BlueZ `profiles/input/hog.c` profile surface.
- ATT fixed channel CID `0x0004`.
- GATT report-map read, protocol-mode write, input-report notification.
- BlueZ `hog-lib.c` suspend/resume, boot-report, output-report policy markers.
- Cleanup to `att=0 hogp=0 input=0 reports=0`.

Boundary: this is a new independent profile surface and validator gate; it
has not been built or run in this update.  It is still a NuttX hwsim semantic
adapter with `staged-boundary=bluezhid-adapter-not-unmodified-bluetoothd`,
not proof that unmodified upstream `bluetoothd` input/HOG plugins fully own
the runtime objects.

## 2026-06-14 BLE GATT/ATT upstream convergence closeout

新增 `bluez-gatt-upstream-convergence-closeout`，把 LE Audio 依赖的通用 BLE GATT/ATT 能力从 LE Audio umbrella 中拆出来单独验证。

```text
case:     bluez-gatt-upstream-convergence-closeout
roles:    ble1, ble2
build:    build/logs/build-ble1-gatt-upstream-convergence.log
build:    build/logs/build-ble2-gatt-upstream-convergence.log
run:      build/logs/run-bluez-gatt-upstream-convergence-closeout.log
manifest: build/bt-hwsim-usecases-bluez-gatt-upstream-convergence-closeout/run-results.json
result:   PASS
```

该 gate 覆盖 ATT bearer、MTU exchange、security、CCC、prepare/execute write、indication、ATT mainloop I/O watch、rx/tx PDU、fragment/reassemble、request queue、PACS/ASCS GATT DB register/discover/read/write/notify/release，以及 final cleanup。

Coverage map 要求 BlueZ `shared/att.c`、`gatt-db.c`、`gatt-server.c`、`io-mainloop.c`、audio `pacs.c`、`ascs.c` 和 Linux `l2cap_sock.c`、`l2cap_core.c` 都出现在 hwsim evidence 中。

2026-06-15 update: this gate now also requires the standalone `bluezgatt`
tool.  `bluezgatt closeout source|sink` owns an ATT fixed-channel shaped
L2CAP socket on CID `0x0004`, drives BlueZ shared ATT/GATT client/server/db
evidence, calls the generic GATT read/write peer API, and emits its own
coverage map:

```text
bluezgatt closeout source 0 1
bluezgatt closeout sink 0 2
```

Required `bluezgatt` evidence:

- `shared/att.c` ATT session open/close, MTU exchange, security, and error
  response policy.
- `shared/gatt-client.c` read/write request ownership.
- `shared/gatt-server.c` write-value and indication confirmation.
- `shared/gatt-db.c` attribute server registration and notification.
- Linux-BT `l2cap_sock.c` / `l2cap_core.c` ATT fixed-channel open/connect.
- Cleanup with `att=0 gatt-db=0 requests=0 watches=0`.

Boundary: this is a NuttX hwsim semantic closeout for the current BLE GATT/ATT
adapter path, not proof that unmodified upstream `bluetoothd` directly owns
every object. The explicit boundary markers remain
`staged-boundary=bluezaudio-gatt-adapter-not-unmodified-bluetoothd` and
`staged-boundary=bluezgatt-adapter-not-unmodified-bluetoothd`.

Current work order:

1. Finish LE Audio full-function gates.
2. Close BT/BLE basic abilities.
3. Resume A2DP completion.
4. Resume BT/BLE NET completion.
5. Finish remaining BT/BLE profiles and deeper upstream replacement.

## 2026-06-14 BT/BLE NET upstream convergence closeout

新增 `bluez-net-upstream-convergence-closeout`，在现有 BT/BLE NET completion 路径上强制验证 Network/IPSP upstream coverage map。

```text
case:     bluez-net-upstream-convergence-closeout
roles:    bt1, bt2, ble1, ble2
build:    FeatherCore/build/logs/build-bt1-net-upstream-convergence.log
build:    FeatherCore/build/logs/build-bt2-net-upstream-convergence.log
build:    FeatherCore/build/logs/build-ble1-net-upstream-convergence.log
build:    FeatherCore/build/logs/build-ble2-net-upstream-convergence.log
run:      FeatherCore/build/logs/run-bluez-net-upstream-convergence-closeout.log
manifest: FeatherCore/build/bt-hwsim-usecases-bluez-net-upstream-convergence-closeout/run-results.json
result:   PASS
```

新增输出和 validator 要求：

- BT side: `bluez-network: closeout upstream-coverage-map ... final-ok=1`
- BT BlueZ map: `main.c`、`profile.c`、`device.c`、`adapter.c`、`dbus-common.c`、`profiles/network/manager.c`、`server.c`、`connection.c`、`bnep.c`
- BT Linux map: `bnep/core.c`、`bnep/sock.c`、`bnep/netdev.c`、`l2cap_core.c`、`l2cap_sock.c`
- BLE side: `bluez-daemon: ipsp closeout upstream-coverage-map ... final-ok=1`
- BLE BlueZ map: `main.c`、`profile.c`、`device.c`、`adapter.c`、`dbus-common.c`、`profiles/network/connection.c`、`ipsp.c`
- BLE Linux map: `6lowpan.c`、`l2cap_core.c`、`l2cap_sock.c`、`net/6lowpan/iphc.c`
- 四角色仍要求 BT `btn0` ping reply、BNEP native TX/RX/netif counters、BLE `bt0` ping6、TCP/UDP iperf、IPHC/fragment counters 和 final cleanup。

边界仍明确保留：BT 是 `blueznetwork-adapter-not-unmodified-bluetoothd`，BLE 是 `bluezdaemon-ipsp-adapter-not-unmodified-bluetoothd`。

## 2026-06-14 A2DP upstream convergence closeout

新增 `bluez-a2dp-upstream-convergence-closeout`，在现有 `audio-a2dp-closeout-full` source/sink 流程上强制验证 upstream coverage map。

```text
case:     bluez-a2dp-upstream-convergence-closeout
roles:    bt1, bt2
build:    FeatherCore/build/logs/build-bt1-a2dp-upstream-convergence.log
build:    FeatherCore/build/logs/build-bt2-a2dp-upstream-convergence.log
run:      FeatherCore/build/logs/run-bluez-a2dp-upstream-convergence-closeout.log
manifest: FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-upstream-convergence-closeout/run-results.json
result:   PASS
```

新增输出和 validator 要求：

- `bluez-daemon: a2dp closeout upstream-coverage-map ... final-ok=1`
- BlueZ source map：`profile.c`、`device.c`、`adapter.c`、`dbus-common.c`、`shared/mainloop.c`、`shared/io-mainloop.c`、`sdpd-service.c`、`a2dp.c`、`avdtp.c`、`media.c`、`transport.c`、`avrcp.c`、`sbc.c`。
- Linux source map：`l2cap_core.c`、`l2cap_sock.c`。
- executed map：plugin/profile/device/sdp/dbus/mainloop/AVDTP/AVRCP/MediaTransport/codec/L2CAP/error-policy。
- source/sink 双端 final evidence：profile、D-Bus、mainloop、transaction、media、codec、transport、AVRCP、L2CAP、state、cleanup 全部 `=1`。

边界仍明确保留：`staged-boundary=bluezdaemon-adapter-not-unmodified-bluetoothd`。这表示 A2DP 又向 upstream object/source coverage 推进了一步，但还不是未修改 upstream `bluetoothd` audio plugin 的最终运行证明。

## 2026-06-14 BT/BLE NET current complete closeout

新增四角色 BT/BLE NET completion gate：

```text
case:     bluez-net-current-complete-closeout
roles:    bt1, bt2, ble1, ble2
run:      FeatherCore/build/logs/run-bluez-net-current-complete-closeout.log
manifest: FeatherCore/build/bt-hwsim-usecases-bluez-net-current-complete-closeout/run-results.json
result:   PASS
```

该 gate 同时覆盖：

- BT NET：BlueZ Network D-Bus ownership、NetworkServer1 PANU/NAP/GN registration、connected L2CAP fd handoff、BNEP native session/netdev datapath、ping reply、role/error lifecycle、final cleanup。
- BLE NET：bluetoothd-style IPSP Profile1 registration、mainloop ownership、LE L2CAP CoC handoff、6LoWPAN/IPHC、ping6、TCP/UDP iperf、fragment counters、final cleanup。

综合 gate 要求 BT/BLE 四个 sim 角色同时通过，并验证 BT 侧 `bnep-native-active=0`、BLE 侧 `registered=0` / owner refs 归零 / `ipsp-state=closed`。MTU1400 仍保留在专门 `bluez-network-closeout-full` gate 中作为 BT NET 压测项。

当前排序：LE Audio full-role 已优先完成；BT/BLE basic 与 BT/BLE NET 基础能力已收口。下一步继续补 A2DP 的 upstream daemon/mainloop/D-Bus 收敛，再补 BT/BLE NET 的更完整 upstream plugin 接管和剩余 profiles。

## 2026-06-14 A2DP current complete closeout

新增显式 A2DP completion gate：

```text
case:     bluez-a2dp-current-complete-closeout
roles:    bt1, bt2
run:      FeatherCore/build/logs/run-bluez-a2dp-current-complete-closeout.log
manifest: FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-current-complete-closeout/run-results.json
result:   PASS
```

该 gate 复用当前最强 `bluezdaemon audio-a2dp-closeout-full source|sink`，并要求 source/sink 两端同时覆盖 mainloop、SDP/profile、controller-created L2CAP、AVDTP transactions、AVRCP、MediaTransport fd、SBC codec、D-Bus owner recovery、error policy、两轮 lifecycle 和 final cleanup。

边界：这是当前 BlueZ-facing A2DP staged completion gate，不是未修改 upstream `bluetoothd` audio plugin 的最终证明。

## 2026-06-14 BlueZ HCI/mgmt/socket ABI closeout

新增 `bluez-hci-mgmt-socket-closeout-full`，用于把 BlueZ-facing 基础 socket ABI 合成一个可重复 gate。

验证结果：PASS

```text
case:     bluez-hci-mgmt-socket-closeout-full
roles:    ble1, ble2
run:      FeatherCore/build/logs/run-bluez-hci-mgmt-socket-closeout-full.log
manifest: FeatherCore/build/bt-hwsim-usecases-bluez-hci-mgmt-socket-closeout-full/run-results.json
```

覆盖内容：

- HCI USER command / monitor / sequence / error / init / ISO setup monitor。
- HCI USER advertising + peer scan report，经 hwsim ADV medium 验证。
- HCI ioctl basic 与 HCI RAW command socket。
- BlueZ mgmt control、pairing/error/reconnect-stress socket path。
- btmon-style HCI monitor socket。

重要语义：HCI USER channel 必须先于 ioctl/mgmt 接管 controller 运行；这次 closeout 按 Linux 独占语义调整了顺序。

## 2026-06-14 BlueZ current functional closeout

新增 `bluez-current-functional-closeout` 作为当前 BlueZ-facing 功能组合门禁。

验证结果：PASS

```text
case:     bluez-current-functional-closeout
roles:    bt1, bt2, ble1, ble2
run:      FeatherCore/build/logs/run-bluez-current-functional-closeout.log
manifest: FeatherCore/build/bt-hwsim-usecases-bluez-current-functional-closeout/run-results.json
```

覆盖内容：

- A2DP：`bluezdaemon audio-a2dp-closeout-full source|sink`。
- BT NET：BlueZ Network PANU/NAP/GN、connected L2CAP fd handoff、BNEP native datapath、MTU ping、error lifecycle、cleanup。
- BLE NET：bluetoothd-style IPSP profile/mainloop/D-Bus、LE L2CAP CoC handoff、6LoWPAN/IPHC、ping6、TCP/UDP、cleanup。

本轮修复：

- A2DP source 侧 L2CAP receive 现在使用 expected-payload 匹配，避免 `avdtp-start` response 在繁忙 hwsim 中被后续 transaction 误消费。
- A2DP sink 侧接收 command 同样按 expected-payload 匹配。
- receive wait 窗口扩展到 240 次，当前组合 gate 下 source/sink 均能打印 `audio-a2dp-closeout-full complete` 并保持 cleanup。

边界：该 gate 证明当前 BlueZ-facing A2DP、BT NET、BLE NET 组合生命周期通过，不代表所有 upstream `bluetoothd` plugin 已经无 harness 全量接管。

## 2026-06-14 LE Audio umbrella 与 basic BlueZ/mgmt closeout

本轮 BlueZ-facing 验证按“先 LE Audio、再 BT/BLE 基础能力”的顺序收口。

LE Audio 当前 full-role umbrella 已通过：

```text
case:     bluez-le-audio-umbrella
run:      FeatherCore/build/logs/run-le-audio-umbrella.log
manifest: FeatherCore/build/bt-hwsim-usecases-le-audio-umbrella/run-results.json
result:   PASS
```

覆盖 BlueZ daemon audio plugin、D-Bus、BAP/PACS/ASCS、broadcast BIG/BIS/BASS、CAP/CSIP、VCP/MICP/MCP/TMAP/CCP/GMAP、ISO socket QoS/credit、LC3 smoke 和 HCI USER ISO monitor。validator 已要求真实 ISO socket event、`upstream-iso-write` 和 `upstream-iso-recv` evidence。

BT/BLE 基础 BlueZ/mgmt gate 已通过：

```text
case set:
  bt-basic
  ble-basic
  hci-le-lifecycle
  hci-le-medium
  hci-le-pairing
  mgmt-control
  bluez-mgmt-control
  bluez-mgmt-pair-noio
  bluez-mgmt-pair-unpair
  bluez-mgmt-lifecycle
  bluez-daemon-mgmt-full-lifecycle
  bluez-basic-mgmt-flow
  bluez-basic-scan-connect-auth-flow

run:      FeatherCore/build/logs/run-basic-closeout.log
manifest: FeatherCore/build/bt-hwsim-usecases-basic-closeout/run-results.json
result:   PASS
```

本轮为 basic gate 补齐的 BlueZ/mgmt 侧证据：

- passive BLE 端也会通过 `btctl poll ctrl` 消费控制面 lifecycle record，更新本地 connection snapshot。
- hci lifecycle、medium、pairing、daemon mgmt full lifecycle 都输出 `btctl upstream hci-status`，用于验证 conn-hash、paired/disconnect 状态。
- `hci-le-medium` 不再依赖阻塞 scan；它以 CTRL poll + event ring + upstream hci-status 验证 passive 端收到的连接生命周期。
- `hci-le-pairing` passive 端会额外 poll control event，确保 pair complete、disconnect 和 cleanup 都进入本地状态机。

当前排序：LE Audio 已先收口，BT/BLE basic gate 已收口；下一步继续 A2DP 和 BT/BLE NET，再补剩余 BlueZ profiles。

边界：当前 `bluezdaemon` / `bluez*` 工具仍包含 staged BlueZ-facing adapter 和 hwsim validator evidence，不等于完整 unmodified upstream `bluetoothd` 对所有 plugin 的真实接管。

## 2026-06-14 BlueZ bneptest native fd handoff closeout

本轮收口 BT NET 的 BlueZ bneptest fd handoff 边界：`bluez-bneptest-fd-handoff` 已加严为 connected L2CAP fd 进入 imported Linux BNEP native ioctl/session 的验证。

新增强制 evidence：

- `bluezbneptest fd-handoff` 使用 `socket(AF_BLUETOOTH, BTPROTO_L2CAP)`、`bind()`、`connect()` 产生 connected L2CAP fd。
- `BNEPCONNADD` 从 BlueZ bneptest adapter 接收该 fd：`connected-l2cap-fd=<fd> bnep-fd=<fd> ioctl=BNEPCONNADD role=0x1115 device=btn0`。
- native BNEP status 在 connadd 后必须显示：
  `bnep-native-fd-handoff=1 bnep-native-fd-active=1 bnep-native-fd-psm=0x000f bnep-native-fd-cid=0x0041 bnep-native-fd-role=0x1115`。
- conndel 后必须显示：
  `bnep-native-fd-active=0 bnep-native-fd-cleanup=1`。
- `BNEPCONNADD` 路径继续要求 imported `bnep/sock.c -> bnep_add_connection()` 的 native session/netdev counters。

验证结果：PASS

- build: `FeatherCore/build/logs/build-bt1-bluez-bneptest-fd-native-handoff.log`
- build: `FeatherCore/build/logs/build-bt2-bluez-bneptest-fd-native-handoff.log`
- run: `FeatherCore/build/logs/run-bluez-bneptest-fd-native-handoff.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-fd-native-handoff/run-results.json`

当前结论：BlueZ bneptest 的 fd handoff 现在不再只是应用层 marker，而是能在 hwsim gate 中证明 connected L2CAP fd 被 native BNEP ioctl 接收、创建 `btn0` session，并在 `BNEPCONNDEL` 后清理 fd/session ownership。

## 2026-06-14 A2DP daemon owner/refcount closeout

本轮回到 A2DP 收口：`bluez-daemon-a2dp-closeout-full` 已补强为 BlueZ daemon session/transport owner/refcount 生命周期验证。

新增强制 evidence：

- source/sink 两端都必须看到 media transport FD owner active：
  `state=active owner=:client.a2dp fd-owner=:client.a2dp acquire-ref=1 transport-ref=1 media-fd=open`
- source/sink 两端两轮 session 都必须看到 active owner state：
  `state=active owner=bluetoothd dbus-owner=:client.a2dp profile-ref=1 device-ref=1 session-ref=1 stream-ref=1 sep-ref=1 endpoint-ref=1 transport-ref=1 player-ref=1`
- 每轮 cleanup 后必须看到 idle owner state：
  `state=idle owner=bluetoothd dbus-owner=:client.a2dp profile-ref=1 device-ref=1 session-ref=0 stream-ref=0 sep-ref=0 endpoint-ref=0 transport-ref=0 player-ref=0`
- final cleanup 必须看到 transport FD owner 归零：
  `state=idle owner=:client.a2dp fd-owner=none acquire-ref=0 transport-ref=0 media-fd=closed write-watch=0 read-watch=0`

同时修复了一个 BT-only 构建问题：`bluezdaemon ipsp-*` 现在受 `CONFIG_NET_LINUX_BLUETOOTH_6LOWPAN_BRIDGE` 保护，避免 BT/A2DP defconfig 链接 BLE 6LoWPAN 符号。

验证结果：PASS

- build: `FeatherCore/build/logs/build-bt1-a2dp-owner-state-closeout.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-owner-state-closeout.log`
- run: `FeatherCore/build/logs/run-a2dp-owner-state-closeout.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-owner-state-closeout/run-results.json`

当前结论：A2DP closeout 现在覆盖 BlueZ daemon profile/mainloop/D-Bus/SDP、AVDTP/AVRCP/media transport、L2CAP controller、codec/error policy，以及 session/transport owner active/idle refcount 清理闭环。

## 2026-06-14 BLE NET/IPSP owner/refcount closeout

本轮继续收口 BLE NET：`bluez-daemon-ipsp-closeout-full` 已加严为 upstream 6LoWPAN owner/refcount 生命周期验证，不再只依赖 datapath counters。

新增强制 evidence：

- active 阶段：`upstream-owner-state=netdev:1,coc:1,peer:1`
- active refs：`upstream-owner-refs=netdev:1,chan:1,peer:1`
- active datapath：`upstream-owner-active-tx` 必须非零
- cleanup 阶段：`upstream-owner-state=netdev:0,coc:0,peer:0`
- cleanup refs：`upstream-owner-refs=netdev:0,chan:0,peer:0`

实现点：

- `net/bluetooth/6lowpan.c` 维护 sim IPSP owner active/refcount state。
- `linux_bt_6lowpan_status()` 输出 owner state/refcount/active TX/RX。
- validator 在 BLE1/BLE2 两端同时检查 active 和 cleanup 状态。

验证结果：PASS

- build: `FeatherCore/build/logs/build-ble1-bluez-daemon-ipsp-owner-state.log`
- build: `FeatherCore/build/logs/build-ble2-bluez-daemon-ipsp-owner-state.log`
- run: `FeatherCore/build/logs/run-bluez-daemon-ipsp-owner-state.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-bluez-daemon-ipsp-owner-state/run-results.json`

当前结论：BLE NET/IPSP 的 daemon 路径现在覆盖 BlueZ daemon profile/D-Bus/mainloop、upstream L2CAP socket ABI、kernel 6LoWPAN owner handoff、ping/TCP/UDP/IPHC/fragment，以及断开后的 owner/refcount 清理闭环。

## 2026-06-14 bluetoothd-style LE IPSP L2CAP socket ownership tightening

本轮继续收口 BLE NET：`bluezdaemon ipsp-connect` 不再只打印 `fd-handoff=le-l2cap-coc` marker，而是在 profile connect 阶段先通过 Linux upstream L2CAP socket ABI 建立 IPSP CoC 证据，再注册 6LoWPAN `bt0`。

新增强制 evidence：

- `upstream-l2cap-bind: psm=0x0023 cid=0x0040 handle=0x0074 create-ret=0 bind-ret=0`
- `upstream-l2cap-connect: psm=0x0023 cid=0x0040 connect-ret=0`
- `upstream-l2cap-close: released`
- validator 要求两轮 daemon IPSP lifecycle 都出现上述 L2CAP socket ABI evidence。

验证结果：PASS

- build: `FeatherCore/build/logs/build-ble1-bluez-daemon-ipsp-l2cap-socket.log`
- build: `FeatherCore/build/logs/build-ble2-bluez-daemon-ipsp-l2cap-socket.log`
- run: `FeatherCore/build/logs/run-bluez-daemon-ipsp-l2cap-socket.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-bluez-daemon-ipsp-l2cap-socket/run-results.json`

`bluez-daemon-ipsp-closeout-full` 现在同时覆盖：

- daemon Profile1/mainloop/D-Bus/security lifecycle。
- LE IPSP L2CAP socket bind/connect/close ownership through upstream socket ABI。
- `fd-handoff=le-l2cap-coc owner=kernel-6lowpan profile=ipsp`。
- BLE1/BLE2 `bt0` IPv6 ping。
- TCP/UDP iperf。
- IPHC TX/RX 与 fragment counters。
- `upstream-owner=net_bluetooth/6lowpan+sim-ipsp-datapath-owner`。
- 两轮 lifecycle 与 final cleanup。

当前结论：BLE NET 的 daemon closeout 已经从 pure marker 推进到实际 upstream L2CAP socket ABI + 6LoWPAN datapath 的组合验证，离 Linux/BlueZ 原生 IPSP profile 语义更近了一步。

边界：这仍不是 unmodified upstream `bluetoothd` 的真实 IPSP plugin/ObjectManager/mainloop，也不是完全由 imported `net/bluetooth/6lowpan.c` 独占 peer/netdev callback owner。下一步 BLE NET 若继续深入，应让 `bluezdaemon` 的 profile/session/refcount/mainloop 更接近真实 BlueZ daemon 结构，并继续把 wrapper/counter evidence 替换为实际 upstream callback ownership。

## 2026-06-14 bluetoothd-style LE IPSP daemon closeout

本轮继续收口 BLE NET，从 `bluezipsp` adapter 再推进到 `bluezdaemon` 的 bluetoothd-style IPSP profile/mainloop/D-Bus lifecycle。

新增内容：

- `bluezdaemon ipsp-connect [ifname]`
- `bluezdaemon ipsp-status`
- `bluezdaemon ipsp-disconnect`
- 新增 hwsim case：`bluez-daemon-ipsp-closeout-full`

新增强制 evidence：

- daemon profile registration：`third/bluez/src/main.c + src/profile.c + src/device.c + profiles/network/connection.c + profiles/network/ipsp.c`。
- `dbus=org.bluez.Profile1`，`owner=bluetoothd`，`security=medium`，`authorize=ok`。
- daemon mainloop ownership：`watch-add=mgmt,dbus,l2cap-coc,6lowpan`，`timer-add=connect-timeout`。
- profile connect：`fd-handoff=le-l2cap-coc owner=kernel-6lowpan profile=ipsp connected=1`。
- D-Bus `InterfacesAdded` / `InterfacesRemoved` for `/org/bluez/hci0/dev_peer/ipsp0`。
- owner-lost cleanup 与 mainloop cleanup：`watches=0 timers=0 owner=bluetoothd`。

验证结果：PASS

- build: `FeatherCore/build/logs/build-ble1-bluez-daemon-ipsp-closeout.log`
- build: `FeatherCore/build/logs/build-ble2-bluez-daemon-ipsp-closeout.log`
- run: `FeatherCore/build/logs/run-bluez-daemon-ipsp-closeout.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-bluez-daemon-ipsp-closeout/run-results.json`

`bluez-daemon-ipsp-closeout-full` 同时覆盖：

- BLE1/BLE2 daemon-style IPSP connect/status/disconnect。
- Profile1 registration/mainloop/D-Bus ownership/security lifecycle。
- BLE1/BLE2 `bt0` IPv6 ping。
- TCP/UDP iperf。
- IPHC TX/RX 与 fragment counters。
- `upstream-owner=net_bluetooth/6lowpan+sim-ipsp-datapath-owner`。
- 两轮 lifecycle 与 final cleanup。

当前结论：BLE NET 已从 `btctl` probe、`bluezipsp` adapter，推进到 `bluezdaemon` bluetoothd-style IPSP closeout gate，并在完整 BLE 6LoWPAN/IP datapath 下通过。

边界：这仍然不是 unmodified upstream `bluetoothd` 的真实 IPSP plugin/ObjectManager/mainloop 实例。下一步 BLE NET 若继续深入，应把 `bluezdaemon ipsp-*` 的 markers 替换为真实 BlueZ daemon object/session/refcount/mainloop，真实 LE L2CAP CoC socket ownership，以及 imported `net/bluetooth/6lowpan.c` peer/netdev callback owner 的实际执行路径。

## 2026-06-14 BlueZ-facing LE IPSP D-Bus/profile lifecycle tightening

本轮继续收口 BLE NET 的 BlueZ 用户态语义：`bluezipsp` 不再只输出 connect/status/disconnect 的 profile-shaped marker，而是补充并强制验证 BlueZ profile/D-Bus/security lifecycle evidence。

新增强制 evidence：

- `profile register service=ipsp`，锚定 `third/bluez/src/profile.c + third/bluez/src/device.c`。
- IPSP UUID `00001820-0000-1000-8000-00805f9b34fb` 与 `org.bluez.Network1` object。
- `owner=:client.ipsp`、`security=medium`、`authorize=ok`、`connect-profile=ok`。
- D-Bus object add：`/org/bluez/hci0/dev_peer/ipsp0`，interfaces `org.bluez.Network1,org.bluez.Device1`。
- status 查询要求 `dbus-owner=:client.ipsp` 与 `connected-query=ok`。
- disconnect 要求 owner-lost cleanup、`InterfacesRemoved` 语义和 object-remove 后 `objects=0 owners=0 refs=0`。

验证结果：PASS

- build: `FeatherCore/build/logs/build-ble1-bluez-ipsp-dbus-lifecycle.log`
- build: `FeatherCore/build/logs/build-ble2-bluez-ipsp-dbus-lifecycle.log`
- run: `FeatherCore/build/logs/run-bluez-ipsp-dbus-lifecycle.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-bluez-ipsp-dbus-lifecycle/run-results.json`

`bluez-ipsp-closeout-full` 现在同时覆盖：

- BlueZ-facing IPSP profile connect/status/disconnect。
- D-Bus object ownership、owner-lost cleanup、object removal。
- Security/authorization policy marker。
- BLE1/BLE2 `bt0` IPv6 ping。
- TCP/UDP iperf、IPHC TX/RX、fragment counters。
- `upstream-owner=net_bluetooth/6lowpan+sim-ipsp-datapath-owner`。
- 两轮 lifecycle 和 final cleanup。

当前结论：BLE NET 的 BlueZ-facing closeout 已经从低层 `btctl` probe 推进到 apps/bluez 工具入口，并额外具备 profile registration、D-Bus object ownership/security lifecycle 的强验证证据。

边界：这仍是 `bluezipsp` adapter 的 BlueZ-shaped lifecycle，不是 unmodified `bluetoothd` 中真实 IPSP plugin/ObjectManager 实例。下一步若继续 BLE NET，应把这些 lifecycle markers 进一步替换为真实 bluetoothd mainloop/D-Bus object ownership、Profile1 registration、LE L2CAP CoC socket ownership 和 imported upstream 6LoWPAN callback owner 的实际代码路径。

## 2026-06-14 BlueZ-facing LE IPSP profile closeout

本轮继续收口 BLE NET 用户态入口：新增 apps 侧 BlueZ-facing LE IPSP profile adapter `bluezipsp`，避免 BLE 6LoWPAN/IPSP closeout 只依赖低层 `btctl upstream 6lowpan-*` probe。

新增内容：

- `FeatherCore/apps/wireless/linux_bluetooth/bluez/tools/ipsp_main.c`
- Kconfig/Makefile/CMake/Make.defs 注册 `CONFIG_LINUX_BLUEZ_IPSP`，默认构建 `bluezipsp`。
- `bluezipsp connect [ifname]`：以 `third/bluez/profiles/network/connection.c + profiles/network/ipsp.c` 语义标记发起 LE IPSP profile connect，并注册 `bt0`。
- `bluezipsp status`：输出 BlueZ IPSP profile marker 和完整 Linux 6LoWPAN status。
- `bluezipsp disconnect`：以 BlueZ profile disconnect marker 触发 6LoWPAN/IPSP teardown。
- 新增 hwsim case：`bluez-ipsp-closeout-full`。

验证结果：PASS

- build: `FeatherCore/build/logs/build-ble1-bluez-ipsp-closeout-v2.log`
- build: `FeatherCore/build/logs/build-ble2-bluez-ipsp-closeout-v2.log`
- run: `FeatherCore/build/logs/run-bluez-ipsp-closeout-v2.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-bluez-ipsp-closeout-v2/run-results.json`

`bluez-ipsp-closeout-full` 覆盖：

- BLE1/BLE2 通过 `bluezipsp connect` 建立 `bt0`。
- BlueZ-facing IPSP UUID `00001820-0000-1000-8000-00805f9b34fb` 与 PSM `0x0023` marker。
- `fd-handoff=le-l2cap-coc owner=kernel-6lowpan profile=ipsp` marker。
- BLE1/BLE2 IPv6 ping。
- TCP iperf 与 UDP iperf。
- IPHC TX/RX、fragment counters、`tx-fallback=0`。
- `upstream-owner=net_bluetooth/6lowpan+sim-ipsp-datapath-owner`。
- 两轮 `bluezipsp connect -> data -> bluezipsp disconnect` lifecycle。
- final cleanup：`registered=0`、`ipsp-state=closed`。

当前结论：BLE NET 现在已有 BlueZ-facing IPSP profile-shaped 入口的完整 hwsim closeout 证据，不再只依赖 `btctl upstream 6lowpan-up/status/down` probe。

边界：`bluezipsp` 仍是 NuttX apps 侧 BlueZ-shaped adapter，不是 unmodified upstream `bluetoothd` IPSP plugin/object manager。下一步 BLE NET 若继续深入，应把 `bluezipsp` 的 connect/disconnect surface 继续收敛到真实 BlueZ daemon profile registration、D-Bus object ownership、security policy、LE L2CAP CoC socket ownership，以及 imported `net/bluetooth/6lowpan.c` 的真实 peer/netdev callback implementation。

## 2026-06-14 BLE 6LoWPAN/IPSP datapath owner tightening

本轮继续收口 BLE NET upstream ownership：在上一轮 `sim-ipsp-owner` 的 attach/detach wrapper 基础上，把 TX/RX datapath counter 也收束到 imported Linux `net/bluetooth/6lowpan.c` 侧 wrapper。

新增/调整：

- `bt_6lowpan_sim_transmit_packet()`：集中记录 owner TX、xmit、`bt_xmit()` 证据。
- `bt_6lowpan_sim_receive_packet()`：集中记录 owner RX、deliver、recv callback 证据。
- `bt_6lowpan_sim_owner()` 现在报告 `net_bluetooth/6lowpan+sim-ipsp-datapath-owner`。
- `ble-ip-closeout-full` validator 同步要求新的 owner 名称。

验证结果：PASS

- build: `FeatherCore/build/logs/build-ble1-ble-ip-sim-ipsp-datapath-owner.log`
- build: `FeatherCore/build/logs/build-ble2-ble-ip-sim-ipsp-datapath-owner.log`
- run: `FeatherCore/build/logs/run-ble-ip-sim-ipsp-datapath-owner.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-ble-ip-sim-ipsp-datapath-owner/run-results.json`

`ble-ip-closeout-full` 覆盖：

- BLE1/BLE2 `bt0` IPv6 ping。
- TCP iperf 与 reverse TCP iperf。
- UDP iperf 与 reverse UDP iperf。
- IPHC TX/RX 与 fragment counters。
- 两轮 6LoWPAN/IPSP up/down lifecycle。
- final cleanup：`registered=0`、`ipsp-state=closed`。

当前结论：BLE NET 当前 hwsim closeout 不再只是 bridge/IPSP 可用性证明；IPSP attach/detach 和 TX/RX datapath evidence 都已经由 imported Linux `net/bluetooth/6lowpan.c` 内的 sim owner wrapper 聚合，并在完整 closeout gate 下通过。

边界：这仍然是 NuttX glue + imported upstream wrapper 的推进阶段，不是 unmodified upstream `net/bluetooth/6lowpan.c` 完全独占。下一步 BLE NET 若继续深入，应把 wrapper 证据替换为真实 upstream `setup_netdev()`、peer list lookup、`bt_xmit()`、recv callback、LE L2CAP CoC callback 与 BlueZ IPSP/Profile initiation 的实际 owner implementation。

## 2026-06-14 BLE 6LoWPAN/IPSP owner handoff tightening

本轮继续推进 BLE NET upstream ownership，不再让当前 `bt0`/IPSP 生命周期证据由 `linux_bt_6lowpan_netdev.c` 零散回填 `note_*` counter。新增 imported Linux `net/bluetooth/6lowpan.c` 侧 wrapper：

- `bt_6lowpan_sim_attach_ipsp()`：集中记录 netdev register、`setup_netdev()`、LE CoC open、`chan_ready_cb()`、peer add。
- `bt_6lowpan_sim_detach_ipsp()`：集中记录 netdev unregister、peer del、LE CoC close、`chan_close_cb()`、delete netdev。
- `bt_6lowpan_sim_owner()` 现在报告 `net_bluetooth/6lowpan+sim-ipsp-owner`。

`ble-ip-ping` validator 同步加严：基础 BLE1/BLE2 IPv6 ping 现在直接要求新的 owner 字符串，以及 peer/CoC/setup/delete/chan-ready/chan-close/bt-xmit/recv-cb lifecycle counters 非零。

验证结果：PASS

- build: `FeatherCore/build/logs/build-ble1-ble-ip-sim-ipsp-owner.log`
- build: `FeatherCore/build/logs/build-ble2-ble-ip-sim-ipsp-owner.log`
- run: `FeatherCore/build/logs/run-ble-ip-sim-ipsp-owner.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-ble-ip-sim-ipsp-owner/run-results.json`

当前结论：BLE1/BLE2 `bt0` IPv6 ping 保持通过，同时 IPSP attach/detach lifecycle 的 ownership 已从 bridge 侧零散 counter 推进到 imported Linux `net/bluetooth/6lowpan.c` 中的 sim owner wrapper。

边界：这仍不是 unmodified upstream `net/bluetooth/6lowpan.c` 完整独占 owner。`bt0` 注册、NuttX lower-half、hwsim medium handoff 仍需要 NuttX glue；下一步 BLE NET 完整化应继续把真实 upstream `setup_netdev()`、peer list lookup、`bt_xmit()`、L2CAP CoC callback 和 BlueZ IPSP/Profile initiation 从 wrapper/counter 证据推进到实际 owner implementation。

## 2026-06-14 BLE 6LoWPAN/IPSP ping: upstream owner lifecycle gate

本轮继续收口 BLE NET，不再只把 `ble-ip-ping` 视为 `bt0` bridge/IPSP 可用性检查，而是把 imported Linux `net/bluetooth/6lowpan.c` owner 生命周期计数纳入强校验。

新增 validator 要求：

- `upstream-owner-peer-add` / `upstream-owner-peer-del` 非零。
- `upstream-owner-coc-open` / `upstream-owner-coc-close` 非零。
- `upstream-owner-setup-netdev` 非零。
- `upstream-owner-chan-ready` 非零。
- `upstream-owner-bt-xmit` 非零。
- 同时保留 `bt0` 注册、IPSP PSM/CID、imported `net/6lowpan/iphc.c` TX/RX、IPv6 ping 与 teardown 校验。

验证结果：PASS

- build: `FeatherCore/build/logs/build-ble1-ble-ip-upstream-owner-lifecycle.log`
- build: `FeatherCore/build/logs/build-ble2-ble-ip-upstream-owner-lifecycle.log`
- run: `FeatherCore/build/logs/run-ble-ip-upstream-owner-lifecycle.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-owner-lifecycle/run-results.json`

当前结论：BLE1/BLE2 IPv6 ping 已经不是单纯证明 NuttX-facing bridge 能收发，而是要求 imported Linux Bluetooth 6LoWPAN owner 侧的 peer lifecycle、LE CoC open/close、`setup_netdev()`、`chan_ready_cb()` 与 `bt_xmit()` 语义证据同时存在。

边界：`upstream-owner=net_bluetooth/6lowpan+bridge-ipsp` 仍然说明当前 `bt0` 设备注册和 hwsim medium handoff 还没有完全由 unmodified upstream `net/bluetooth/6lowpan.c` 独占。下一步若继续 BLE NET，应把当前 bridge-owned netdev/handoff 进一步替换成 imported upstream peer/netdev/L2CAP callback owner，同时保持本轮更严格的 `ble-ip-ping` gate 继续 PASS。

## 2026-06-14 A2DP closeout: controller-owned L2CAP channel policy

本轮继续收口 `bluez-daemon-a2dp-closeout-full`，把 A2DP/AVRCP 相关 L2CAP channel 的 controller ownership 纳入 hwsim 强校验：

- `bt1/source` 与 `bt2/sink` 都要求出现 `phase=l2cap-controller`。
- 语义锚定到 `third/linux-hwe-6.17-6.17.0/net/bluetooth/l2cap_core.c` 与 `l2cap_sock.c`。
- 覆盖 signaling/media/AVCTP/AVCTP browsing channel 创建、PSM/CID、MTU/security/mode、duplicate channel reject、timeout retry、disconnect ok。
- cleanup 要求 `channel-disconnect=signaling,media,avctp,avctp-browsing` 且 `channels=0 refs=0 retrans=0 pending=0`。

验证结果：PASS

- build: `FeatherCore/build/logs/build-bt1-a2dp-l2cap-controller.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-l2cap-controller.log`
- run: `FeatherCore/build/logs/run-a2dp-l2cap-controller.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-l2cap-controller/run-results.json`

当前结论：A2DP 的现有 hwsim closeout 已覆盖 daemon/plugin mainloop ownership、D-Bus object/owner recovery、SDP profile ownership、AVDTP transaction lifecycle、media transport acquire/release、codec/error policy，以及本轮补齐的 controller-owned L2CAP channel setup/error/cleanup 证据。

边界：这仍然不是“无改动 upstream bluetoothd 全量运行”。它是 NuttX sim/hwsim 下对 Linux/BlueZ 语义的强制 closeout probe。后续若要继续逼近完整 upstream，需要继续替换 helper/probe 为真实 upstream bluetoothd audio plugin mainloop、真实 D-Bus daemon/object ownership、真实 HCI/SDP/L2CAP refcount 与 session owner。

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

Current LE Audio codec backend:

```text
third/liblc3 -> apps/wireless/linux_bluetooth/bluez/codecs/lc3_backend.c
```

The `bluezaudio le-audio-codec` path now uses Google liblc3 directly through
`lc3_encode()` / `lc3_decode()`.  The old deterministic fallback has been
removed; current hwsim evidence is `bluez-le-audio-umbrella` with
`backend=google-liblc3 status=liblc3-linked`.

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

`bluezhciraw user-iso-setup-monitor` is the LE Audio controller setup gate.
It drives the BlueZ-facing `HCI_CHANNEL_USER` path through LE ISO controller
commands used by BAP/ISO setup: `LE Read Buffer Size V2`, `LE Set Host
Feature`, `LE Set CIG Parameters`, `LE Create CIS`, `LE Read ISO TX Sync`,
`LE Setup ISO Path`, `LE Remove CIG`, `LE Create BIG`, and `LE Terminate BIG`.
The paired hwsim case is `bluez-le-audio-controller-setup`; it also requires
`HCI_CHANNEL_MONITOR` replay and `btctl upstream status` ISO state counters so
the path is not just an application-level LE Audio log.

Current LE Audio controller setup result:

```text
PASS bluez-le-audio-controller-setup
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-controller-setup/bluez-le-audio-controller-setup.ble1.log
```

The gate is now tightened beyond Command Complete-only behavior.  The monitor
side must also observe LE Meta events for `LE CIS Established`, `LE Create BIG
Complete`, and `LE Terminate BIG Complete`:

```text
PASS bluez-le-audio-controller-setup
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-controller-meta-events/bluez-le-audio-controller-setup.ble1.log
```

It is also tightened to the Linux controller event shape for asynchronous ISO
procedures: `LE Create CIS`, `LE Create BIG`, and `LE Terminate BIG` now return
`Command Status` before their LE Meta events:

```text
PASS bluez-le-audio-controller-setup
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-controller-command-status/bluez-le-audio-controller-setup.ble1.log
```

The controller-side status line now records the same lifecycle, so the gate
checks state ownership in addition to socket-visible events:

```text
PASS bluez-le-audio-controller-setup
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-controller-lifecycle-status/bluez-le-audio-controller-setup.ble1.log
```

`bluez-le-audio-controller-reconnect` repeats the same controller setup and
teardown sequence twice in one sim lifetime.  It verifies that Command Status,
LE Meta events, and controller lifecycle counters all advance to `2` rather
than only satisfying a one-shot path.

```text
PASS bluez-le-audio-controller-reconnect
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-controller-reconnect/bluez-le-audio-controller-reconnect.ble1.log
```

The reconnect gate also checks imported HCI `conn_hash` visibility through
`btctl upstream hci-status`.  After two controller setup/teardown cycles, CIS
remains visible and the terminated BIS is gone:

```text
PASS bluez-le-audio-controller-reconnect
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-controller-conn-hash/bluez-le-audio-controller-reconnect.ble1.log
```

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
PASS bluez-le-audio-daemon-full-lifecycle
PASS bluez-le-audio-dbus-client-full
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
FeatherCore/build/logs/run-bluez-le-audio-daemon-full-lifecycle.log
FeatherCore/build/logs/run-bluez-le-audio-dbus-client-full.log
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
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-daemon-full-lifecycle/run-results.json
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-dbus-client-full/run-results.json
```

The daemon-owned LE Audio gate adds a bluetoothd-shaped ownership layer around
the existing broadcast BIS plus bidirectional unicast CIS lifecycle.  Both
BLE1 and BLE2 run `bluezaudio le-daemon register`, which records
`org.bluez` ownership, ObjectManager root `/org/bluez/hci0`, PACS
`PACSink1/PACSource1`, ASCS `ASE1/ASCS1`, BAP broadcast/unicast objects,
`Media1/MediaEndpoint1` endpoints, and `MediaTransport1` fd ownership.  The
same run then validates BIS payload, two CIS MediaTransport acquire/write/read
paths, ASCS/BAP release back to idle, and `le-daemon release` removing the
objects.

This is now stronger than a pure profile-shaped signaling/payload gate, but it
is still a daemon-shaped hwsim ownership model rather than the full upstream
`bluetoothd` mainloop running unchanged.  The next alignment step is to replace
the remaining synthetic ownership logs with actual upstream BlueZ BAP/PACS/ASCS
plugin object registration and real D-Bus object lifetime on the same ISO socket
path.

The external D-Bus client gate adds the provider side of that contract.  Both
roles now exercise `Media1.RegisterEndpoint`, `MediaEndpoint1.SelectProperties`,
`MediaEndpoint1.SetConfiguration`, `MediaTransport1.Acquire`,
`MediaTransport1.Release`, `MediaEndpoint1.ClearConfiguration`,
`Media1.UnregisterEndpoint`, and ObjectManager-style
`InterfacesAdded/InterfacesRemoved` for LC3 source and sink endpoints.  The same
run swaps roles across two CIS handles so BLE1 and BLE2 each act as source and
sink endpoint providers while the ISO MediaTransport path continues to move the
LE Audio payload.

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

The current stronger umbrella gate folds TCP forward, TCP reverse, UDP forward,
and UDP reverse into a single BT1/BT2 hwsim run:

```text
PASS bluez-network-iperf-matrix
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-network-iperf-matrix.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-iperf-matrix/run-results.json
```

Each of the four rounds performs:

```text
blueznetwork connect panu
btn0 IPv4 setup
iperf TCP or UDP traffic
blueznetwork disconnect
```

The validator requires four successful `BNEPCONNADD` calls with
`role=0x1115` and `flags=0x00000001`, all four iperf modes
(`tcp-server`, `tcp-client`, `udp-server`, `udp-client`), non-zero throughput,
four disconnect completions, and final native BNEP lifecycle counters at four
create/start/stop/terminate and register/unregister cycles with
`bnep-native-active=0`.

The same BlueZ Network-shaped path now also has a large ICMP payload stress
gate:

```text
PASS bluez-network-mtu-ping
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-network-mtu-ping.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-mtu-ping/run-results.json
```

This gate performs `ping -s 1400` in both directions after
`blueznetwork connect panu` creates `btn0` through `BNEPCONNADD`.  The
validator requires `1400 bytes from 10.77.0.2`, `1400 bytes from 10.77.0.1`,
`2 packets transmitted, 2 received, 0% packet loss`, non-zero native BNEP
datapath counters, and clean `BNEPCONNDEL` teardown with
`bnep-native-active=0`.

The large-payload path is also covered across repeated BNEP lifecycle:

```text
PASS bluez-network-mtu-reconnect-stress
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-network-mtu-reconnect-stress.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-mtu-reconnect-stress/run-results.json
```

This gate runs three rounds of `blueznetwork connect panu -> ping -s 1400 ->
blueznetwork disconnect` on both peers.  The validator requires at least four
1400-byte replies on each side, final `bnep-ioctl-connadd=3`,
`bnep-ioctl-conndel=3`, native BNEP session start/stop/terminate counters at
three, native netdev register/unregister counters at three, and
`bnep-native-active=0`.

Large-payload traffic also has a single-session soak gate:

```text
PASS bluez-network-mtu-soak
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-network-mtu-soak.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-mtu-soak/run-results.json
```

This keeps one BNEP session open and runs `ping -s 1400 -c 12` in both
directions.  The validator requires 12/12 received packets on both peers,
at least 12 large replies from each peer, native BNEP datapath counters, and
clean teardown with `bnep-native-active=0`.

The current BlueZ Network-shaped BNEP path also has a 2000-byte jumbo payload
gate:

```text
PASS bluez-network-jumbo-ping
```

Artifacts:

```text
FeatherCore/build/logs/run-bluez-network-jumbo-ping.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-jumbo-ping/run-results.json
```

This gate explicitly raises `btn0` to `mtu 2500`, then runs `ping -s 2000` in
both directions.  The validator requires the MTU change to be visible, two
2000-byte replies on both peers, native BNEP datapath counters, and clean
teardown with `bnep-native-active=0`.

A related default-MTU experiment was intentionally not hidden: with `btn0 mtu
1500`, `ping -s 2000` fails at send time with `errno=22` and no BNEP TX
counters.  That means BNEP large-frame carriage is now validated after an MTU
raise, while default-MTU IPv4 fragmentation/reassembly remains a separate
unfinished item.

Follow-up: BT1/BT2 defconfigs now enable `CONFIG_NET_IPFRAG=y` and the images
were rebuilt.  The default-MTU reproducer was rerun, but it still does not pass:
the failure moved from `errno=22` to `errno=101`, with only small BNEP frames
transmitted and no peer-side native RX.  The reproducer artifacts are:

```text
FeatherCore/build/logs/build-bt1-ipfrag.log
FeatherCore/build/logs/build-bt2-ipfrag.log
FeatherCore/build/logs/run-bluez-network-frag-ping-ipfrag-v2.log
FeatherCore/build/bt-hwsim-usecases-bluez-network-frag-ping-ipfrag-v2/run-results.json
```

So the current state is precise: `bluez-network-jumbo-ping` proves BNEP can
carry 2000-byte payloads when MTU allows them; default-MTU IPv4 fragmentation
over the BNEP netdev remains an open implementation gap.

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
## 2026-06-13: A2DP daemon plus external MediaEndpoint/Transport client gate

`bluez-daemon-a2dp-dbus-client-full` now validates a stronger A2DP lifecycle:
daemon-owned bluetoothd-shaped audio/AVRCP mainloop first, explicit BR/EDR
teardown, then external `:1.feather` MediaEndpoint registration,
MediaTransport Acquire, media payload transfer, release, and ClearConfiguration.

Validated artifacts:

```text
FeatherCore/build/logs/build-bt1-a2dp-dbus-client-full.log
FeatherCore/build/logs/build-bt2-a2dp-dbus-client-full.log
FeatherCore/build/logs/run-bluez-daemon-a2dp-dbus-client-full-v4.log
```

The gate exposed and fixed a source-side MediaTransport fd ownership issue:
`audio_main.c` now uses a handle-owned L2CAP fd for A2DP source Acquire instead
of the singleton probe socket, matching BlueZ `MediaTransport1.Acquire`
semantics more closely.

## 2026-06-13: A2DP external MediaTransport duplicate Acquire error gate

`bluez-daemon-a2dp-dbus-client-busy` keeps the daemon-owned A2DP setup and the
external `:1.feather` MediaEndpoint provider, then validates the
`MediaTransport1.Acquire` busy/error lifecycle on both source and sink sides.

The new `bluezaudio` transport commands still use the real hwsim L2CAP media
path, but issue a second Acquire while the transport is already active:

```text
bluezaudio media-transport a2dp-source-acquire-busy-write-release 2
bluezaudio media-transport a2dp-sink-acquire-busy-read-release 1
```

Required evidence:

```text
MediaTransport1.Acquire ... request=primary
PropertiesChanged State=active
org.bluez.Error.InProgress method=org.bluez.MediaTransport1.Acquire request=duplicate
A2DP:SBC:synthetic-frame write/read over L2CAP media CID 0x0041
MediaTransport1.Release
PropertiesChanged State=idle
ClearConfiguration and MediaTransport unexport
```

Validation:

```text
PASS bluez-daemon-a2dp-dbus-client-busy
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-a2dp-dbus-busy.log
FeatherCore/build/logs/build-bt2-a2dp-dbus-busy.log
FeatherCore/build/logs/run-bluez-daemon-a2dp-dbus-client-busy.log
FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-dbus-client-busy/run-results.json
```

Boundary: this improves A2DP D-Bus fd ownership/error lifecycle coverage, but it
is still a BlueZ-shaped porting gate rather than unmodified `bluetoothd`.  Full
parity still requires real upstream `profiles/audio/*` object ownership,
concurrent signaling/media fd ownership inside a true AVDTP session object, and
controller-created L2CAP channel lifecycle.

## 2026-06-13: A2DP signaling/media concurrent fd gate

`bluez-a2dp-sbc-codec-concurrent` removes the old hwsim sequencing limitation
where the source-side AVDTP signaling session had to be closed before acquiring
the media transport.  The gate keeps the source signaling socket open on
CID `0x0040`, acquires the media transport on CID `0x0041`, sends a source-built
SBC frame, then continues the original signaling session for `SUSPEND` and
`CLOSE`.

Required evidence:

```text
a2dp source session opened peer=2 handle=0x0052
signaling session retained during media peer=2 handle=0x0052 signaling-cid=0x0040 media-cid=0x0041
MediaTransport1.Acquire fd=l2cap role=source
upstream-l2cap-write-handle ... cid=0x0041 ... native-ret>0
sink sbc_decode pcm-bytes>0 checksum=...
STREAMING->OPEN
OPEN->IDLE
a2dp source session closed peer=2 handle=0x0052
```

Validation:

```text
PASS bluez-a2dp-sbc-codec-concurrent
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-a2dp-sbc-concurrent-v2.log
FeatherCore/build/logs/build-bt2-a2dp-sbc-concurrent-v2.log
FeatherCore/build/logs/run-bluez-a2dp-sbc-codec-concurrent-v2.log
FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-sbc-codec-concurrent-v2/run-results.json
```

Boundary: this closes the staged-porting limitation that forced signaling/media
to be serialized in this A2DP gate.  It still does not mean complete unmodified
`bluetoothd`: the AVDTP session and MediaTransport objects are still
BlueZ-shaped app-side porting objects, and controller-created L2CAP channel
setup/error policy is still not fully owned by upstream Linux/BlueZ code.

## 2026-06-13: A2DP daemon-owned full concurrent umbrella gate

`bluez-daemon-a2dp-full-concurrent` is the current strongest single A2DP hwsim
gate.  It combines the previously separate evidence into one BT1/BT2 lifecycle:
daemon-owned A2DP/AVRCP mainloop, external MediaEndpoint registration,
duplicate `MediaTransport1.Acquire` busy error, concurrent signaling/media fd
ownership, source-built SBC media, sink-side SBC decode, and signaling teardown.

Required evidence:

```text
bluezdaemon audio-a2dp-owner source/sink
org.bluez owner acquired by bluezdaemon
audio/avrcp mainloop registered avdtp=owned avctp=owned media=owned
external Media1.RegisterEndpoint and SetConfiguration
AVDTP source-session-open keeps signaling CID 0x0040 active
MediaTransport duplicate Acquire -> org.bluez.Error.InProgress
synthetic MediaTransport payload over media CID 0x0041
source-built libsbc frame over media CID 0x0041
sink sbc_decode PCM checksum
SUSPEND/CLOSE over original signaling session
ClearConfiguration and object removal
```

Validation:

```text
PASS bluez-daemon-a2dp-full-concurrent
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-a2dp-full-concurrent.log
FeatherCore/build/logs/build-bt2-a2dp-full-concurrent.log
FeatherCore/build/logs/run-bluez-daemon-a2dp-full-concurrent.log
FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-full-concurrent/run-results.json
```

Boundary: this is a stronger umbrella gate, not a final completion claim.  The
current code still uses BlueZ-shaped app-side ownership rather than an
unmodified upstream `bluetoothd` process with real `gdbus` dispatch, full
`profiles/audio/*` object graph, controller-created L2CAP channel setup/error
policy, and full AVDTP session object lifetime.

## 2026-06-13: A2DP full concurrent reconnect gate

`bluez-daemon-a2dp-full-concurrent-reconnect` repeats the full concurrent A2DP
umbrella path twice in one BT1/BT2 hwsim session.  This exposed a real ownership
gap: source-side AVDTP signaling still used the singleton probe socket, so after
daemon reconnect it could fail to reopen as a persistent session.  The
source-side AVDTP session now owns an independent handle-based L2CAP fd for
CID `0x0040`, while media continues to acquire a separate handle-based fd for
CID `0x0041`.

Validation:

```text
PASS bluez-daemon-a2dp-full-concurrent-reconnect
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-a2dp-full-concurrent-reconnect-v3.log
FeatherCore/build/logs/build-bt2-a2dp-full-concurrent-reconnect-v3.log
FeatherCore/build/logs/run-bluez-daemon-a2dp-full-concurrent-reconnect-v3.log
FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-full-concurrent-reconnect-v3/run-results.json
```

Required repeated evidence:

```text
daemon-owned A2DP reconnect rounds=2
MediaEndpoint lifecycle source/sink x2
source-session-open x2
duplicate MediaTransport1.Acquire busy error x2
signaling session retained during media x2
source-built libsbc media x2
sink sbc_decode x2
SUSPEND/CLOSE teardown x2
MediaEndpoint clear source/sink x2
```

Boundary: this closes the current A2DP source signaling singleton-probe
ownership gap and strengthens reconnect evidence.  It still remains a
BlueZ-shaped porting gate rather than a complete unmodified `bluetoothd` audio
plugin with real upstream `gdbus` dispatch and complete controller-created
L2CAP/AVDTP policy ownership.

## 2026-06-13: A2DP full concurrent three-round soak gate

`bluez-daemon-a2dp-full-concurrent-soak` extends the full concurrent reconnect
coverage from two rounds to three rounds in one BT1/BT2 hwsim session.  This is
the current A2DP stability gate for repeated object cleanup/recreate and fd
reopen under the daemon-owned + external endpoint path.

Validation:

```text
PASS bluez-daemon-a2dp-full-concurrent-soak
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-a2dp-full-concurrent-soak.log
FeatherCore/build/logs/build-bt2-a2dp-full-concurrent-soak.log
FeatherCore/build/logs/run-bluez-daemon-a2dp-full-concurrent-soak.log
FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-full-concurrent-soak/run-results.json
```

Required repeated evidence:

```text
daemon-owned A2DP reconnect rounds=3
MediaEndpoint lifecycle source/sink x3
source-session-open x3
duplicate MediaTransport1.Acquire busy error x3
signaling session retained during media x3
source-built libsbc media x3
sink sbc_decode x3
SUSPEND/CLOSE teardown x3
MediaEndpoint clear source/sink x3
```

Boundary: this is stronger lifecycle coverage, not an unmodified `bluetoothd`
completion claim.  The remaining A2DP work is still to replace the BlueZ-shaped
app-side objects with the real upstream daemon/object/session ownership and
controller-created L2CAP policy.

## 2026-06-13: BLE 6LoWPAN ping requires imported IPHC TX/RX

`ble-ip-ping` now requires the imported Linux `net/6lowpan/iphc.c` helper path
to be visible in the status output.  A passing run must show
`upstream-iphc-owner=net_6lowpan/iphc`, non-zero `tx-iphc` and `rx-iphc`, and
`tx-fallback=0` on both BLE peers.

Validated artifacts:

```text
FeatherCore/build/logs/build-ble1-ble-ip-upstream-iphc-gate.log
FeatherCore/build/logs/build-ble2-ble-ip-upstream-iphc-gate.log
FeatherCore/build/logs/run-ble-ip-ping-upstream-iphc-gate-v3.log
```

The passing run shows BLE1 and BLE2 both reaching `2 packets transmitted, 2
received`, `last-tx-dispatch=0x7b last-rx-dispatch=0x7b`, and
`tx-iphc=4 rx-iphc=4`.

## LE Audio controller-owned CIS hci_conn visibility

The LE Audio controller hwsim path now validates a real upstream-visible CIS
`hci_conn` entry, not only synthetic counters.  The HCI USER setup flow first
removes a stale CIG, then configures CIG/CIS, creates CIS, sets the ISO path,
and runs BIG create/terminate lifecycle.  On HCI USER close, the sim controller
keeps the controller-owned CIS established while removing the terminated BIS.

Validated gate:

```text
case: bluez-le-audio-controller-reconnect
build: FeatherCore/build/logs/build-ble1-le-audio-controller-hci-conn-list-v5.log
run:   FeatherCore/build/logs/run-bluez-le-audio-controller-hci-conn-list-v5.log
result: FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-controller-hci-conn-list-v5/run-results.json
status: PASS
```

Required evidence now includes:

```text
conn-hash acl=0 sco=0 le=0 le-peripheral=0 cis=1 bis=0 pa=0
conn[0] type=130 role=0 state=1 out=1 handle=0x0201
```

This closes the current LE Audio controller ISO setup/reconnect/hci_conn
visibility slice.  Remaining LE Audio work is the fuller bluetoothd/BAP/ASCS
MediaEndpoint ownership and policy surface, not this controller-owned HCI ISO
object visibility gate.

## LE Audio external MediaTransport duplicate Acquire error gate

`bluez-le-audio-dbus-client-full` now covers the external LE Audio
MediaEndpoint provider error path for duplicate `MediaTransport1.Acquire`.
After each source/sink transport enters `State=active`, the test issues a
second Acquire request and requires the BlueZ-shaped error:

```text
org.bluez.Error.InProgress method=org.bluez.MediaTransport1.Acquire
```

This is validated for both BLE1 and BLE2, and for both source and sink roles,
while the same run still transfers the ISO media payload and tears down the
MediaEndpoint/MediaTransport objects.

Validated gate:

```text
case:   bluez-le-audio-dbus-client-full
build:  FeatherCore/build/logs/build-ble1-le-audio-dbus-client-busy.log
build:  FeatherCore/build/logs/build-ble2-le-audio-dbus-client-busy.log
run:    FeatherCore/build/logs/run-bluez-le-audio-dbus-client-busy.log
result: FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-dbus-client-busy/run-results.json
status: PASS
```

Key evidence:

```text
MediaTransport1.Acquire ... fd=iso request=duplicate
org.bluez.Error.InProgress ... active-fd=iso
media transport write len=28 payload=LE-AUDIO:CIS:synthetic-frame
media transport read complete payload=LE-AUDIO:CIS:synthetic-frame
le daemon released pacs=0 ascs=0 bap=0 media-endpoints=0 transports=0
```

Boundary: this strengthens the D-Bus ownership/error lifecycle in the hwsim
adapter.  Full parity still requires replacing the synthetic ownership model
with actual upstream bluetoothd BAP/PACS/ASCS plugin object lifetime and real
D-Bus dispatch.

## BLE 6LoWPAN/IPSP hwsim ping status

Although this is below BlueZ audio/network profile level, the current Bluetooth
stack now has a verified BLE IP baseline for later BlueZ/network integration:

```text
PASS ble-ip-ping
PASS ble-ip-reconnect-stress
```

Artifacts:

```text
FeatherCore/build/logs/run-ble-ip-ping-current.log
FeatherCore/build/bt-hwsim-usecases-ble-ip-ping-current/run-results.json
FeatherCore/build/logs/run-ble-ip-reconnect-stress-v2.log
FeatherCore/build/bt-hwsim-usecases-ble-ip-reconnect-stress-v2/run-results.json
```

The reconnect stress gate validates three BLE 6LoWPAN/IPSP `bt0` lifecycle
rounds.  BLE2 now uses direct `connect 3` startup instead of a blocking scan,
matching the single-ping gate.  This is useful as the IP datapath baseline, but
it still uses the hwsim `bridge-ipsp` owner rather than final upstream Linux
6LoWPAN session ownership.
## LE Audio LC3 codec metadata transport gate

`bluez-le-audio-lc3-codec-transport` adds a focused bidirectional LE Audio
media gate on top of the ISO transport path.  It does not claim a real LC3
library yet; the new
`bluez/codecs/lc3_backend.[ch]` backend deliberately reports:

```text
backend=deterministic-lc3-frame status=liblc3-missing required-source=third/liblc3-or-apps-codec
```

The gate verifies the BlueZ-facing shape that a future real LC3 backend must
preserve:

```text
Media1.RegisterEndpoint codec=lc3
MediaEndpoint1.SetConfiguration codec=lc3 qos=interval-us:10000,rtn:2,latency-ms:20
MediaTransport1.Acquire fd=iso
PACS LC3 capabilities=16_2_1 metadata=context-media,location-front-left-right
BAP QoS presentation-delay-us=40000 sdu=40 phy=2m
LC3 encode/decode sequence=1 timestamp-us=10000 checksum=0xb65fbaaf
ISO write/read payload-len=40
BLE1 source -> BLE2 sink on CIS 1 / handle 0x0201
BLE2 source -> BLE1 sink on CIS 2 / handle 0x0202
```

Validation:

```text
PASS bluez-le-audio-lc3-codec-transport
```

Artifacts:

```text
FeatherCore/build/logs/build-ble1-le-audio-lc3-codec-transport-v2.log
FeatherCore/build/logs/build-ble2-le-audio-lc3-codec-transport-v2.log
FeatherCore/build/logs/run-bluez-le-audio-lc3-codec-transport.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-lc3-codec-transport/run-results.json
FeatherCore/build/logs/build-ble1-le-audio-lc3-codec-bidir.log
FeatherCore/build/logs/build-ble2-le-audio-lc3-codec-bidir.log
FeatherCore/build/logs/run-bluez-le-audio-lc3-codec-bidir.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-lc3-codec-bidir/run-results.json
```

## LE Audio controller-owned bidirectional LC3 hwsim gate

`bluez-le-audio-controller-lc3-bidir` keeps the bidirectional LC3-shaped
MediaTransport flow, but adds controller-side HCI USER ISO setup evidence before
the media exchange.  The BLE1 side runs `bluezhciraw
user-iso-setup-bidir-monitor`, then both peers exercise source and sink roles in
the same hwsim session:

```text
BLE1 source -> CIS handle 0x0201 -> BLE2 sink
BLE2 source -> CIS handle 0x0202 -> BLE1 sink
```

The validator now requires these controller and media facts together:

```text
HCI Command Status for LE Create CIS appears twice
LE CIS Established event appears for handle 0x0201
LE CIS Established event appears for handle 0x0202
hci-user-iso-create-cis=2
hci-user-iso-read-tx-sync=2
hci-user-iso-setup-path=2
conn-hash acl=0 sco=0 le=0 le-peripheral=0 cis=2 bis=0
hci-conn handle=0x0201 link=CIS
hci-conn handle=0x0202 link=CIS
LC3-like 40-byte ISO frame write/read in both directions
ISO socket bind/connect/send/recv counters
```

Validation:

```text
PASS bluez-le-audio-controller-lc3-bidir
```

Artifacts:

```text
FeatherCore/build/logs/build-ble1-le-audio-controller-lc3-bidir-v12.log
FeatherCore/build/logs/build-ble2-le-audio-controller-lc3-bidir-v12.log
FeatherCore/build/logs/run-bluez-le-audio-controller-lc3-bidir-v12.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-controller-lc3-bidir-v12/run-results.json
```

Boundary: this is the strongest current LE Audio hwsim gate, but it is still not
full unmodified BlueZ LE Audio.  The LC3 backend is still deterministic and
reports `status=liblc3-missing`; the sim ISO receive path also contains a
deterministic LC3 fallback for handles `0x0201` and `0x0202` to avoid hwsim
reader-offset timing nondeterminism.  Real `bluetoothd` BAP/CAP object
ownership, external D-Bus MediaEndpoint ownership, true LC3 encode/decode,
controller-owned CIG/CIS scheduling, and complete upstream Linux
`iso.c`/`hci_conn.c` lifecycle ownership remain open work.
## LE Audio umbrella hwsim gate

新增 `bluez-le-audio-umbrella`，作为当前 LE Audio 最强组合验证：

- `bluezhciraw` HCI USER two-CIS controller setup。
- `bluezaudio le-daemon` daemon/ObjectManager/PACS/ASCS/BAP/MediaTransport ownership。
- external `MediaEndpoint1` source/sink 注册、配置、Acquire、duplicate Acquire busy error、Release。
- broadcast BIS payload 和双向 LC3-shaped CIS ISO encode/decode。

当前结果：

```text
PASS bluez-le-audio-umbrella
```

Artifacts：

```text
FeatherCore/build/logs/run-bluez-le-audio-umbrella.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-umbrella/run-results.json
```

边界：该 gate 仍是 BlueZ-shaped adapter，不是 unmodified `bluetoothd`
BAP/CAP/BASS plugin mainloop；LC3 仍是 deterministic backend，真实 `liblc3`
和完整 upstream Linux ISO/HCI ownership 还需要继续收敛。

## LE Audio CAP/BASS hardening

`bluezaudio` 现在补充两个 LE Audio profile-facing command group：

```text
bluezaudio le-cap-control coordinator-register|group-config|group-enable|group-release|coordinator-release
bluezaudio le-bass-control assistant-register|add-source|modify-source|remove-source|assistant-release
```

`bluez-le-audio-umbrella` 已加硬为同时要求 CAP coordinator、BASS broadcast
assistant、broadcast BIS、HCI USER two-CIS setup、external MediaEndpoint busy
error 和双向 LC3-shaped CIS media。

当前结果：

```text
PASS bluez-le-audio-umbrella
run:    FeatherCore/build/logs/run-bluez-le-audio-cap-bass.log
result: FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-cap-bass/run-results.json
```

## LE Audio real LC3 backend

`third/liblc3` is now compiled into the NuttX apps-side BlueZ audio adapter.
The LE Audio umbrella gate validates real liblc3 encode/decode over hwsim ISO:

```text
PASS bluez-le-audio-umbrella
run:    FeatherCore/build/logs/run-bluez-le-audio-liblc3-nofallback.log
result: FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-liblc3-nofallback/run-results.json
```

Expected backend evidence:

```text
backend=google-liblc3 status=liblc3-linked required-source=third/liblc3
```

## LE Audio ASCS control-point coverage

`bluezaudio` 新增 `le-ascs-cp`，用于在 NuttX sim hwsim 中提供 BlueZ ASCS 风格控制点事务证据：

```text
bluezaudio le-ascs-cp config-codec source|sink [cig] [cis]
bluezaudio le-ascs-cp config-qos source|sink [cig] [cis]
bluezaudio le-ascs-cp enable source|sink [cig] [cis]
bluezaudio le-ascs-cp receiver-start-ready source|sink [cig] [cis]
bluezaudio le-ascs-cp disable source|sink [cig] [cis]
bluezaudio le-ascs-cp release source|sink [cig] [cis]
bluezaudio le-ascs-cp config-qos-reject source|sink [cig] [cis]
```

`bluez-le-audio-umbrella` 已把这些命令接到 CAP/BASS、MediaEndpoint/MediaTransport、真实 `third/liblc3` LC3 encode/decode、BIS/CIS ISO 传输链路中。最新验证：

```text
PASS bluez-le-audio-umbrella
FeatherCore/build/logs/run-bluez-le-audio-ascs-cp.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-ascs-cp/
```

## LE Audio BAP policy scheduler coverage

`bluezaudio` 新增 `le-bap-policy`，用于把 LE Audio 从单独 ASCS/MediaTransport probe 继续收敛到 BlueZ BAP policy/scheduler 语义：

```text
bluezaudio le-bap-policy scheduler-register source|sink [cig] [cis]
bluezaudio le-bap-policy select-codec source|sink [cig] [cis]
bluezaudio le-bap-policy select-qos source|sink [cig] [cis]
bluezaudio le-bap-policy select-cis source|sink [cig] [cis]
bluezaudio le-bap-policy bind-transport source|sink [cig] [cis]
bluezaudio le-bap-policy start-stream source|sink [cig] [cis]
bluezaudio le-bap-policy suspend-stream source|sink [cig] [cis]
bluezaudio le-bap-policy stop-stream source|sink [cig] [cis]
bluezaudio le-bap-policy scheduler-release source|sink [cig] [cis]
```

`bluez-le-audio-umbrella` 已要求 BAP policy scheduler、ASCS control point、CAP/BASS、MediaTransport、真实 `third/liblc3` 和 hwsim ISO 同时通过。最新验证：

```text
PASS bluez-le-audio-umbrella
FeatherCore/build/logs/run-bluez-le-audio-bap-policy.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-bap-policy/
```

## LE Audio GATT DB coverage

`bluezaudio` 新增 `le-gatt-db`，用于在 NuttX sim hwsim 中覆盖 BlueZ LE Audio 的 PACS/ASCS GATT DB 与 attribute server 语义：

```text
bluezaudio le-gatt-db register source|sink [cig] [cis]
bluezaudio le-gatt-db discover-pacs source|sink [cig] [cis]
bluezaudio le-gatt-db discover-ascs source|sink [cig] [cis]
bluezaudio le-gatt-db read-pac source|sink [cig] [cis]
bluezaudio le-gatt-db read-ase source|sink [cig] [cis]
bluezaudio le-gatt-db write-ascs-cp source|sink [cig] [cis]
bluezaudio le-gatt-db notify-ase source|sink [cig] [cis]
bluezaudio le-gatt-db release source|sink [cig] [cis]
```

`bluez-le-audio-umbrella` 已要求 GATT DB、BAP policy scheduler、ASCS control point、CAP/BASS、MediaTransport、真实 `third/liblc3` 和 hwsim ISO 同时通过。最新验证：

```text
PASS bluez-le-audio-umbrella
FeatherCore/build/logs/run-bluez-le-audio-gatt-db.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-gatt-db/
```

## LE Audio ATT bearer coverage

`bluezaudio` 新增 `le-att-bearer`，用于在 NuttX sim hwsim 中覆盖 BlueZ LE Audio 的 ATT bearer 与 GATT server 语义：

```text
bluezaudio le-att-bearer open source|sink [cig] [cis]
bluezaudio le-att-bearer mtu-exchange source|sink [cig] [cis]
bluezaudio le-att-bearer security source|sink [cig] [cis]
bluezaudio le-att-bearer enable-ccc source|sink [cig] [cis]
bluezaudio le-att-bearer prepare-write source|sink [cig] [cis]
bluezaudio le-att-bearer execute-write source|sink [cig] [cis]
bluezaudio le-att-bearer indicate source|sink [cig] [cis]
bluezaudio le-att-bearer close source|sink [cig] [cis]
```

`bluez-le-audio-umbrella` 已要求 ATT bearer、GATT DB、BAP policy scheduler、ASCS control point、CAP/BASS、MediaTransport、真实 `third/liblc3` 和 hwsim ISO 同时通过。最新验证：

```text
PASS bluez-le-audio-umbrella
FeatherCore/build/logs/run-bluez-le-audio-att-bearer.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-att-bearer/
```

## LE Audio ATT IO coverage

`bluezaudio` 新增 `le-att-io`，用于在 NuttX sim hwsim 中覆盖 BlueZ `bt_att` / mainloop / L2CAP ATT fd ownership 语义：

```text
bluezaudio le-att-io attach source|sink [cig] [cis]
bluezaudio le-att-io watch-rx source|sink [cig] [cis]
bluezaudio le-att-io watch-tx source|sink [cig] [cis]
bluezaudio le-att-io rx-pdu source|sink [cig] [cis]
bluezaudio le-att-io tx-pdu source|sink [cig] [cis]
bluezaudio le-att-io fragment-write source|sink [cig] [cis]
bluezaudio le-att-io reassemble source|sink [cig] [cis]
bluezaudio le-att-io persist-ccc source|sink [cig] [cis]
bluezaudio le-att-io flush source|sink [cig] [cis]
bluezaudio le-att-io detach source|sink [cig] [cis]
```

`bluez-le-audio-umbrella` 已要求 ATT IO、ATT bearer、GATT DB、BAP policy scheduler、ASCS control point、CAP/BASS、MediaTransport、真实 `third/liblc3` 和 hwsim ISO 同时通过。最新验证：

```text
PASS bluez-le-audio-umbrella
FeatherCore/build/logs/run-bluez-le-audio-att-io.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-att-io/
```

## LE Audio bt_att request queue coverage

`bluezaudio` 新增 `le-att-queue`，用于在 NuttX sim hwsim 中覆盖 BlueZ `bt_att` request queue、socket dispatch 与错误生命周期语义：

```text
bluezaudio le-att-queue alloc-req source|sink [cig] [cis]
bluezaudio le-att-queue enqueue source|sink [cig] [cis]
bluezaudio le-att-queue socket-read source|sink [cig] [cis]
bluezaudio le-att-queue socket-write source|sink [cig] [cis]
bluezaudio le-att-queue timeout source|sink [cig] [cis]
bluezaudio le-att-queue cancel source|sink [cig] [cis]
bluezaudio le-att-queue error-rsp source|sink [cig] [cis]
bluezaudio le-att-queue complete source|sink [cig] [cis]
bluezaudio le-att-queue free-req source|sink [cig] [cis]
```

`bluez-le-audio-umbrella` 已要求 ATT queue、ATT IO、ATT bearer、GATT DB、BAP policy scheduler、ASCS control point、CAP/BASS、MediaTransport、真实 `third/liblc3` 和 hwsim ISO 同时通过。最新验证：

```text
PASS bluez-le-audio-umbrella
FeatherCore/build/logs/run-bluez-le-audio-att-queue.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-att-queue/
```

## LE Audio ISO socket ownership coverage

`bluezaudio` 新增 `le-iso-socket`，用于在 NuttX sim hwsim 中覆盖 BlueZ MediaTransport 到 kernel ISO socket 的 fd ownership 与 socket ABI 语义：

```text
bluezaudio le-iso-socket open source|sink [cig] [cis]
bluezaudio le-iso-socket bind-cis source|sink [cig] [cis]
bluezaudio le-iso-socket connect source|sink [cig] [cis]
bluezaudio le-iso-socket listen source|sink [cig] [cis]
bluezaudio le-iso-socket accept source|sink [cig] [cis]
bluezaudio le-iso-socket pollout source|sink [cig] [cis]
bluezaudio le-iso-socket sendmsg source|sink [cig] [cis]
bluezaudio le-iso-socket pollin source|sink [cig] [cis]
bluezaudio le-iso-socket recvmsg source|sink [cig] [cis]
bluezaudio le-iso-socket timestamp source|sink [cig] [cis]
bluezaudio le-iso-socket error-eagain source|sink [cig] [cis]
bluezaudio le-iso-socket shutdown source|sink [cig] [cis]
bluezaudio le-iso-socket close source|sink [cig] [cis]
```

`bluez-le-audio-umbrella` 已要求 ISO socket、ATT queue、ATT IO、ATT bearer、GATT DB、BAP policy scheduler、ASCS control point、CAP/BASS、MediaTransport、真实 `third/liblc3` 和 hwsim ISO 同时通过。最新验证：

```text
PASS bluez-le-audio-umbrella
FeatherCore/build/logs/run-bluez-le-audio-iso-socket.log
FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-iso-socket/
```

## LE Audio ISO QoS / controller timing validation

`bluezaudio` now includes `le-iso-qos` commands for the LE Audio unicast ISO setup path:

- `configure`
- `select-phy`
- `setup-cig`
- `setup-cis`
- `apply-qos`
- `controller-timing`
- `credit-grant`
- `credit-complete`
- `teardown`

The `bluez-le-audio-umbrella` hwsim case requires these records for BLE1/BLE2 source and sink flows, alongside the existing BlueZ daemon, D-Bus media endpoint, ATT/GATT/ASCS, BAP policy, ISO socket, and LC3 transport checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-iso-qos.log`
- `FeatherCore/build/logs/build-ble2-le-audio-iso-qos.log`
- `FeatherCore/build/logs/run-bluez-le-audio-iso-qos-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-iso-qos-rerun/`

Result: `PASS bluez-le-audio-umbrella`.


## LE Audio Broadcast BIG/BIS validation

`bluezaudio` now includes `le-broadcast-iso` commands for the broadcast LE Audio path:

- `adv-start`
- `base-config`
- `big-create`
- `bis-setup`
- `bis-bind`
- `pa-sync`
- `big-sync`
- `receive-state`
- `bis-credit`
- `bis-complete`
- `big-terminate`

The `bluez-le-audio-umbrella` hwsim case requires these records across BLE1 broadcast source and BLE2 broadcast sink, alongside the existing daemon, D-Bus, BASS, BAP, ISO socket, LC3, CIS QoS, ATT/GATT, ASCS, and PACS checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-broadcast-bis.log`
- `FeatherCore/build/logs/build-ble2-le-audio-broadcast-bis.log`
- `FeatherCore/build/logs/run-bluez-le-audio-broadcast-bis-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-broadcast-bis-rerun/`

Result: `PASS bluez-le-audio-umbrella`.


## LE Audio encrypted broadcast validation

`bluezaudio` now includes `le-broadcast-security` commands for encrypted broadcast coverage:

- `set-code`
- `encrypt-big`
- `bad-code`
- `decrypt-setup`
- `receive-state-encrypted`
- `clear-code`

The `bluez-le-audio-umbrella` hwsim case requires these source/sink records alongside the existing BASS, BAP, BIG/BIS, CIS QoS, ASCS, PACS, ATT/GATT, ISO socket, D-Bus, MediaTransport, and LC3 checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-broadcast-security.log`
- `FeatherCore/build/logs/build-ble2-le-audio-broadcast-security.log`
- `FeatherCore/build/logs/run-bluez-le-audio-broadcast-security-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-broadcast-security-rerun/`

Result: `PASS bluez-le-audio-umbrella`.


## LE Audio PACS metadata/context validation

`bluezaudio le-gatt-db` now includes additional PACS commands:

- `read-location`
- `read-context`
- `update-context`
- `notify-pac`

The `bluez-le-audio-umbrella` hwsim case requires these PACS records alongside daemon, D-Bus, PACS/ASCS, BAP, BASS, BIG/BIS, encrypted broadcast, CIS QoS, ATT/GATT, ISO socket, MediaTransport, and LC3 checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-pacs-metadata.log`
- `FeatherCore/build/logs/build-ble2-le-audio-pacs-metadata.log`
- `FeatherCore/build/logs/run-bluez-le-audio-pacs-metadata.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-pacs-metadata/`

Result: `PASS bluez-le-audio-umbrella`.


## LE Audio ASCS metadata/stop-ready validation

`bluezaudio le-ascs-cp` now includes additional ASCS control point commands:

- `update-metadata`
- `receiver-stop-ready`

The `bluez-le-audio-umbrella` hwsim case requires these ASCS records alongside daemon, D-Bus, PACS/ASCS, BAP, BASS, BIG/BIS, encrypted broadcast, CIS QoS, ATT/GATT, ISO socket, MediaTransport, and LC3 checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-ascs-metadata.log`
- `FeatherCore/build/logs/build-ble2-le-audio-ascs-metadata.log`
- `FeatherCore/build/logs/run-bluez-le-audio-ascs-metadata.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-ascs-metadata/`

Result: `PASS bluez-le-audio-umbrella`.


## LE Audio VCP/VCS validation

`bluezaudio` now includes `le-vcp-control` commands for Volume Control Service/Profile coverage:

- `register`
- `discover`
- `read-state`
- `set-volume`
- `notify-state`
- `flags`
- `error`
- `release`

The `bluez-le-audio-umbrella` hwsim case requires these VCP/VCS records alongside daemon, D-Bus, PACS/ASCS, BAP, BASS, BIG/BIS, encrypted broadcast, CIS QoS, ATT/GATT, ISO socket, MediaTransport, and LC3 checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-vcp.log`
- `FeatherCore/build/logs/build-ble2-le-audio-vcp.log`
- `FeatherCore/build/logs/run-bluez-le-audio-vcp-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-vcp-rerun/`

Result: `PASS bluez-le-audio-umbrella`.


## LE Audio MICP/MICS validation

`bluezaudio` now includes `le-micp-control` commands for Microphone Control Profile/Service coverage:

- `register`
- `discover`
- `read-state`
- `mute`
- `notify-state`
- `flags`
- `error`
- `release`

The `bluez-le-audio-umbrella` hwsim case requires these MICP/MICS records alongside daemon, D-Bus, PACS/ASCS, BAP, BASS, VCP/VCS, BIG/BIS, encrypted broadcast, CIS QoS, ATT/GATT, ISO socket, MediaTransport, and LC3 checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-micp.log`
- `FeatherCore/build/logs/build-ble2-le-audio-micp.log`
- `FeatherCore/build/logs/run-bluez-le-audio-micp-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-micp-rerun/`

Result: `PASS bluez-le-audio-umbrella`.


## LE Audio CSIP/CSIS validation

`bluezaudio` now includes `le-csip-control` commands for Coordinated Set Identification Profile/Service coverage:

- `register`
- `discover`
- `read-sirk`
- `read-size`
- `read-rank`
- `lock`
- `unlock`
- `notify`
- `error`
- `release`

The `bluez-le-audio-umbrella` hwsim case requires these CSIP/CSIS records alongside daemon, D-Bus, PACS/ASCS, BAP, BASS, VCP/VCS, MICP/MICS, BIG/BIS, encrypted broadcast, CIS QoS, ATT/GATT, ISO socket, MediaTransport, and LC3 checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-csip.log`
- `FeatherCore/build/logs/build-ble2-le-audio-csip.log`
- `FeatherCore/build/logs/run-bluez-le-audio-csip-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-csip-rerun/`

Result: `PASS bluez-le-audio-umbrella`.


## LE Audio MCP/MCS validation

`bluezaudio` now includes `le-mcp-control` commands for Media Control Profile/Service coverage:

- `register`
- `discover`
- `read-player`
- `read-track`
- `play`
- `pause`
- `next`
- `notify-state`
- `search`
- `error`
- `release`

The `bluez-le-audio-umbrella` hwsim case requires these MCP/MCS records alongside daemon, D-Bus, PACS/ASCS, BAP, BASS, VCP/VCS, MICP/MICS, CSIP/CSIS, BIG/BIS, encrypted broadcast, CIS QoS, ATT/GATT, ISO socket, MediaTransport, and LC3 checks.

Validated artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-mcp.log`
- `FeatherCore/build/logs/build-ble2-le-audio-mcp.log`
- `FeatherCore/build/logs/run-bluez-le-audio-mcp-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-mcp-rerun/`

Result: `PASS bluez-le-audio-umbrella`.


## 2026-06-14 LE Audio TMAP/TMAS coverage

The NuttX sim BlueZ port now has `bluezaudio le-tmap-control` coverage for BlueZ-style `profiles/audio/tmap.c` / TMAS behavior:

- Source and sink TMAS service lifecycle: register, discover, read role, update role, notify role, error, release.
- D-Bus object ownership and role property updates via the simulated `org.bluez.TelephonyAndMediaAudio1` interface.
- ATT error and `org.bluez.Error.NotSupported` policy path for unsupported role combinations.
- Integrated into the existing `bluez-le-audio-umbrella` hwsim case with validator assertions for both `ble1` and `ble2`.

Validation artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-tmap.log`
- `FeatherCore/build/logs/build-ble2-le-audio-tmap.log`
- `FeatherCore/build/logs/run-bluez-le-audio-tmap-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-tmap-rerun/`

Result: `PASS bluez-le-audio-umbrella`.

## 2026-06-14 LE Audio CCP/TBS coverage

The NuttX sim BlueZ port now has `bluezaudio le-ccp-control` coverage for BlueZ-style `profiles/audio/ccp.c` / Telephone Bearer Service behavior:

- Source and sink TBS service lifecycle: register, discover, read bearer, read call state, originate, accept, terminate, notify call state, notify termination reason, error, release.
- D-Bus object ownership and state property updates via the simulated `org.bluez.TelephonyBearer1` interface.
- ATT error and `org.bluez.Error.Failed` policy path for unsupported call-control opcodes.
- Integrated into the existing `bluez-le-audio-umbrella` hwsim case with validator assertions for both `ble1` and `ble2`.

Validation artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-ccp.log`
- `FeatherCore/build/logs/build-ble2-le-audio-ccp.log`
- `FeatherCore/build/logs/run-bluez-le-audio-ccp.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-ccp/`

Result: `PASS bluez-le-audio-umbrella`.

## 2026-06-14 LE Audio GMAP/GMAS coverage

The NuttX sim BlueZ port now has `bluezaudio le-gmap-control` coverage for BlueZ-style `profiles/audio/gmap.c` / Gaming Audio Service behavior:

- Source and sink GMAS service lifecycle: register, discover, read role, update role, notify role, error, release.
- D-Bus object ownership and role property updates via the simulated `org.bluez.GamingAudio1` interface.
- ATT error and `org.bluez.Error.NotSupported` policy path for unsupported gaming role combinations.
- Integrated into the existing `bluez-le-audio-umbrella` hwsim case with validator assertions for both `ble1` and `ble2`.

Validation artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-gmap.log`
- `FeatherCore/build/logs/build-ble2-le-audio-gmap.log`
- `FeatherCore/build/logs/run-bluez-le-audio-gmap.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-gmap/`

Result: `PASS bluez-le-audio-umbrella`.

## 2026-06-14 LE Audio media endpoint owner lifecycle coverage

The NuttX sim BlueZ port now validates media endpoint owner lifecycle behavior for LE Audio:

- `bluezaudio le-dbus-client owner-lost` models D-Bus owner loss, transport release, and removal of `org.bluez.MediaTransport1` / `org.bluez.MediaEndpoint1`.
- `bluezaudio le-dbus-client owner-reacquire` models endpoint re-registration, transport reconfiguration, and reacquired transport ownership.
- The checks are integrated into `bluez-le-audio-umbrella` with source and sink validator assertions.

Validation artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-media-owner.log`
- `FeatherCore/build/logs/build-ble2-le-audio-media-owner.log`
- `FeatherCore/build/logs/run-bluez-le-audio-media-owner-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-media-owner-rerun/`

Result: `PASS bluez-le-audio-umbrella`.

## 2026-06-14 LE Audio bluetoothd plugin/mainloop lifecycle coverage

The NuttX sim BlueZ port now validates bluetoothd-shaped LE Audio daemon lifecycle:

- `bluezaudio le-daemon plugin-init` models audio plugin initialization, plugin loader registration, and persistent mainloop setup.
- `bluezaudio le-daemon adapter-powered` models adapter powered/probe lifecycle for `hci0`.
- `bluezaudio le-daemon mainloop-dispatch` models shared mainloop dispatch for ATT/ISO/D-Bus watches.
- `bluezaudio le-daemon profile-accept|profile-release` models LE Audio `bt_profile` accept/release and BAP attach/detach.
- `bluezaudio le-daemon plugin-exit` models plugin exit and mainloop cleanup.
- The checks are integrated into `bluez-le-audio-umbrella` with source and sink validator assertions.

Validation artifacts:

- `FeatherCore/build/logs/build-ble1-le-audio-daemon-lifecycle.log`
- `FeatherCore/build/logs/build-ble2-le-audio-daemon-lifecycle.log`
- `FeatherCore/build/logs/run-bluez-le-audio-daemon-lifecycle.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-daemon-lifecycle/`

Result: `PASS bluez-le-audio-umbrella`.

## 2026-06-14 Basic LE mgmt/HCI flow coverage

The NuttX sim BlueZ port now includes `bluezaudio le-mgmt-control` coverage for the basic mgmt/HCI flow that precedes higher-level LE Audio behavior:

- `power-on`, `scan-start`, `connect`, `security`, `cis-request`, `disconnect`, and `error` commands.
- BlueZ-style mgmt socket events for new settings, device found, device connected/disconnected, new link key, and command status.
- Linux HCI event semantics for LE enhanced connection complete, LTK request, CIS established, and disconnection complete.
- Integrated into `bluez-le-audio-umbrella` with BLE source/sink validator assertions.

Validation artifacts:

- `FeatherCore/build/logs/build-ble1-basic-mgmt-flow.log`
- `FeatherCore/build/logs/build-ble2-basic-mgmt-flow.log`
- `FeatherCore/build/logs/run-basic-mgmt-flow.log`
- `FeatherCore/build/bt-hwsim-usecases-basic-mgmt-flow/`

Result: `PASS bluez-le-audio-umbrella`.

## 2026-06-14 Standalone BlueZ basic mgmt/HCI flow coverage

The NuttX sim BlueZ port now includes a first-class `bluez-basic-mgmt-flow` hwsim case for the basic controller lifecycle that must precede profile work:

- BLE1/BLE2 daemon setup, adapter powered state, mgmt power-on, scan, connect, security/bonding, CIS setup, disconnect, and error status.
- The case is registered in the hwsim runner and validator, independent of LE Audio umbrella payload/profile validation.

Validation artifacts:

- `FeatherCore/build/logs/build-ble1-standalone-basic-mgmt-flow.log`
- `FeatherCore/build/logs/build-ble2-standalone-basic-mgmt-flow.log`
- `FeatherCore/build/logs/run-standalone-basic-mgmt-flow.log`
- `FeatherCore/build/bt-hwsim-usecases-standalone-basic-mgmt-flow/`

Result: `PASS bluez-basic-mgmt-flow`.

## 2026-06-14 Standalone BlueZ scan/connect/auth basic flow coverage

The NuttX sim BlueZ port now includes a first-class `bluez-basic-scan-connect-auth-flow` gate for the updated work order before A2DP/LE Audio/NET:

- daemon discovery/device-found, btmgmt control, pair/connect lifecycle, NoInputNoOutput pairing, pairing matrix, reconnect stress, device policy, and mgmt error path.
- The case is registered in the hwsim runner and validator independently of LE Audio umbrella validation.

Validation artifacts:

- `FeatherCore/build/logs/build-ble1-basic-scan-connect-auth-flow.log`
- `FeatherCore/build/logs/build-ble2-basic-scan-connect-auth-flow.log`
- `FeatherCore/build/logs/run-basic-scan-connect-auth-flow-rerun.log`
- `FeatherCore/build/bt-hwsim-usecases-basic-scan-connect-auth-flow-rerun/`

Result: `PASS bluez-basic-scan-connect-auth-flow`.

## 2026-06-14 BlueZ hwsim coverage update

LE Audio 优先项已经补齐到当前 sim hwsim 可验证范围：TMAP/TMAS、CCP/TBS、GMAP/GMAS、Media endpoint owner lost/reacquire、daemon/plugin/mainloop lifecycle、LE mgmt/HCI scan/connect/security/CIS/error path 均已有独立或 umbrella 用例通过。

A2DP full concurrent soak 也已扩展到更完整的 AVDTP procedure 组合：Discover、GetAllCapabilities、GetCapabilities、SetConfiguration、GetConfiguration、Reconfigure、DelayReport、SecurityControl、Open、Start、Suspend、Close，并覆盖 MediaTransport duplicate Acquire/InProgress、SBC encode/decode、endpoint lifecycle 和 L2CAP counter。通过日志：

- `build/logs/build-bt1-a2dp-extended-soak.log`
- `build/logs/build-bt2-a2dp-extended-soak.log`
- `build/logs/run-a2dp-extended-soak.log`

## 2026-06-14 LE Audio ISO data-plane soak

新增 `bluez-le-audio-iso-dataplane-soak`，用于验证 BlueZ-shaped LE Audio unicast data plane：双向 CIS1/CIS2、双轮 LC3 ISO frame、ASCS/BAP 状态、ISO socket ABI、QoS/credit、MediaTransport D-Bus lifecycle 和 release/teardown。

已通过：

- `build/logs/build-ble1-le-audio-iso-dataplane-soak.log`
- `build/logs/build-ble2-le-audio-iso-dataplane-soak.log`
- `build/logs/run-le-audio-iso-dataplane-soak.log`

## 2026-06-14 LE Audio controller-driven ISO data-plane soak

新增 `bluez-le-audio-controller-iso-dataplane-soak`。该用例把 BlueZ HCI USER LE Create CIS / CIS Established controller setup 与双向双轮 LC3 ISO MediaTransport data plane 放在同一个 hwsim 场景中验证。

已通过：

- `build/logs/build-ble1-le-audio-controller-iso-dataplane-soak.log`
- `build/logs/build-ble2-le-audio-controller-iso-dataplane-soak.log`
- `build/logs/run-le-audio-controller-iso-dataplane-soak.log`

## 2026-06-14 LE Audio daemon-owned profile flow

新增 `bluezaudio le-daemon unicast-profile-flow source|sink [cig] [cis]`，并通过 `bluez-le-audio-daemon-profile-flow`。该命令把 D-Bus MediaEndpoint/MediaTransport、ATT/GATT、ASCS CP、BAP policy scheduler、ISO QoS/socket、LC3 encode/decode 和 release/teardown 收敛到 daemon-owned profile flow 内部，减少外部 NSH staged command 拼接。

已通过：

- `build/logs/build-ble1-le-audio-daemon-profile-flow.log`
- `build/logs/build-ble2-le-audio-daemon-profile-flow.log`
- `build/logs/run-le-audio-daemon-profile-flow-rerun.log`

## 2026-06-14 LE Audio controller + daemon profile flow

新增并通过 `bluez-le-audio-controller-daemon-profile-flow`。该用例把 HCI USER LE Create CIS / CIS Established controller setup 和 `bluezaudio le-daemon unicast-profile-flow` 放在同一场景，要求 daemon-owned profile flow 完成 D-Bus MediaEndpoint/MediaTransport、ASCS/BAP、ISO QoS/socket、LC3 encode/decode 和 release/teardown。

已通过：

- `build/logs/run-le-audio-controller-daemon-profile-flow.log`

## LE Audio Broadcast/BIS daemon profile flow

`bluezaudio` 已新增：

```sh
bluezaudio le-daemon broadcast-profile-flow source|sink [big] [bis]
```

该命令把 LE Audio Broadcast/BIS 的验证从多条 staged NSH 命令收敛为一个 daemon-owned profile flow：
- source 角色覆盖 bluetoothd profile register/accept、BAP source announce/start/stop、BASE config、broadcast code、BIG create、BIS setup/bind/credit/terminate、ISO payload send。
- sink 角色覆盖 bluetoothd profile register/accept、BASS assistant register/add-source/PA sync/broadcast-code/decrypt/modify/remove/release、BAP sink discover/config/sync/release、ISO payload receive。

hwsim 用例：
- `bluez-le-audio-daemon-broadcast-profile-flow`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-daemon-broadcast-profile-flow-rerun.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-daemon-broadcast-profile-flow-rerun/`

边界：当前实现覆盖 Broadcast/BIS/BASS/BASE/ISO 数据面生命周期，但仍未宣称完整 upstream bluetoothd LE Audio broadcast object/session ownership。

## LE Audio controller-driven Broadcast/BIS profile flow

新增 hwsim 用例：

```sh
bluez-le-audio-controller-daemon-broadcast-profile-flow
```

BLE1 先运行：

```sh
bluezhciraw user-iso-setup-monitor
```

用于验证 HCI USER socket 上的 LE ISO controller path，包括 `LE Create BIG`、`LE Terminate BIG`、command status 和 BIG complete/terminate complete event。随后 BLE1/BLE2 运行：

```sh
bluezaudio le-daemon broadcast-profile-flow source 0 1
bluezaudio le-daemon broadcast-profile-flow sink 0 1
```

验证结果：
- `bluez-le-audio-controller-daemon-broadcast-profile-flow`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-controller-daemon-broadcast-profile-flow.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-daemon-broadcast-profile-flow/`

这使 LE Audio Broadcast/BIS 覆盖从单纯 profile/data-plane probe 前进到 controller event + daemon profile + BASS/BAP + ISO payload 的组合验证。

## LE Audio Broadcast/BIS reconnect validation

新增 hwsim 用例：

```sh
bluez-le-audio-controller-daemon-broadcast-reconnect
```

该用例先通过 `bluezhciraw user-iso-setup-monitor` 验证 HCI USER BIG/BIS controller setup，然后在同一 BLE1/BLE2 sim pair 上连续运行两轮：

```sh
bluezaudio le-daemon broadcast-profile-flow source 0 1
bluezaudio le-daemon broadcast-profile-flow sink 0 1
```

验证结果：
- `bluez-le-audio-controller-daemon-broadcast-reconnect`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-controller-daemon-broadcast-reconnect.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-daemon-broadcast-reconnect/`

该验证覆盖 Broadcast/BIS 的重复启动、释放、再次启动、再次 ISO payload receive 生命周期。

## LE Audio unicast/CIS reconnect validation

新增 hwsim 用例：

```sh
bluez-le-audio-controller-daemon-profile-reconnect
```

该用例先通过 `bluezhciraw user-iso-setup-bidir-monitor` 验证 HCI USER 双 CIS controller setup，然后在同一 BLE1/BLE2 sim pair 上连续运行两轮：

```sh
bluezaudio le-daemon unicast-profile-flow source 0 1
bluezaudio le-daemon unicast-profile-flow sink 0 2
bluezaudio le-daemon unicast-profile-flow sink 0 1
bluezaudio le-daemon unicast-profile-flow source 0 2
```

验证结果：
- `bluez-le-audio-controller-daemon-profile-reconnect`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-controller-daemon-profile-reconnect.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-daemon-profile-reconnect/`

该验证覆盖 unicast/CIS 的重复 source/sink 建立、LC3 payload write/recv、释放和再次建立生命周期。

## LE Audio error recovery validation

新增 hwsim 用例：

```sh
bluez-le-audio-controller-daemon-error-recovery
```

该用例覆盖：
- ASCS `config-qos-reject`，返回 `status=0x0d reason=invalid-qos`
- unicast `source-qos-reject` / `sink-qos-reject`
- unicast `source-qos-cancel` / `sink-qos-cancel`
- ISO socket `sendmsg` / `recvmsg` EAGAIN
- 错误后 unicast daemon profile flow 恢复成功
- Broadcast sink `bad-code` 后 `decrypt-setup` 恢复成功并接收 BIS ISO payload

验证结果：
- `bluez-le-audio-controller-daemon-error-recovery`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-controller-daemon-error-recovery-ok.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-daemon-error-recovery-ok/`

## LE Audio D-Bus ownership validation

新增 hwsim 用例：

```sh
bluez-le-audio-controller-daemon-dbus-ownership
```

该用例覆盖：
- `org.bluez.Media1.RegisterEndpoint`
- `org.bluez.MediaEndpoint1.SetConfiguration`
- `org.bluez.MediaTransport1.Acquire`
- duplicate Acquire `org.bluez.Error.InProgress`
- D-Bus owner lost
- MediaTransport Release with `reason=owner-lost`
- `InterfacesRemoved` for MediaTransport and MediaEndpoint
- owner reacquire / `NameOwnerAcquired`
- `InterfacesAdded` for MediaTransport
- `MediaEndpoint1.ClearConfiguration`
- ownership 变化后 daemon unicast profile flow 恢复 ISO payload

验证结果：
- `bluez-le-audio-controller-daemon-dbus-ownership`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-controller-daemon-dbus-ownership.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-daemon-dbus-ownership/`

## LE Audio full-stack ownership validation

新增 hwsim 用例：

```sh
bluez-le-audio-controller-daemon-full-stack
```

该用例在同一 BLE1/BLE2 pair 上覆盖：
- HCI USER two-CIS controller setup
- BlueZ mgmt power/scan/connect/security/CIS request
- MediaEndpoint/MediaTransport D-Bus ownership lifecycle
- duplicate Acquire `org.bluez.Error.InProgress`
- owner-lost / InterfacesRemoved / owner-reacquire
- ASCS `config-qos-reject`
- unicast QoS reject/cancel
- unicast bidirectional CIS payload
- broadcast BIS payload with BASS bad-code/decrypt recovery

验证结果：
- `bluez-le-audio-controller-daemon-full-stack`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-controller-daemon-full-stack.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-daemon-full-stack/`

## LE Audio full-stack reconnect validation

新增 hwsim 用例：

```sh
bluez-le-audio-controller-daemon-full-stack-reconnect
```

该用例在 `full-stack` 聚合路径后继续验证：
- unicast CIS payload 正向和反向恢复
- broadcast BIS payload 连续两轮 source/sink 传输
- socket send/recv/native-recv 计数增长
- D-Bus owner lost/reacquire 与 ASCS/BAP error recovery 后仍能继续跑数据面

验证结果：
- `bluez-le-audio-controller-daemon-full-stack-reconnect`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-controller-daemon-full-stack-reconnect.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-daemon-full-stack-reconnect/`

## LE Audio daemon mainloop/fd cleanup validation

新增 hwsim 用例：

```sh
bluez-le-audio-controller-daemon-mainloop-cleanup
```

该用例覆盖：
- persistent mainloop init / dispatch / exit
- ATT/ISO/D-Bus io-watch ownership
- `MediaTransport1 fd-owner=bluetoothd`
- duplicate Acquire `InProgress`
- D-Bus owner lost / owner reacquire
- BAP detach `transports=0`
- daemon release `media-endpoints=0 transports=0`
- mainloop exit `watches=0 timeouts=0`
- cleanup 后重新启动 unicast profile flow 并完成 ISO payload

验证结果：
- `bluez-le-audio-controller-daemon-mainloop-cleanup`: PASS
- run log: `FeatherCore/build/logs/run-le-audio-controller-daemon-mainloop-cleanup-rerun.log`
- artifacts: `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-daemon-mainloop-cleanup-rerun/`
## LE Audio coordinated service matrix hwsim gate

`bluez-le-audio-coordinated-services` adds a focused LE Audio service-matrix
gate for the BlueZ userspace port.  It keeps the service ownership checks
separate from the ISO/LC3 data-plane cases and verifies both source and sink
roles on BLE1/BLE2.

Covered service groups:

```text
VCP/VCS   org.bluez.VolumeControl1
MICP/MICS org.bluez.MicrophoneControl1
CSIP/CSIS org.bluez.CoordinatedSet1
MCP/MCS   org.bluez.MediaControl1
TMAP/TMAS org.bluez.TelephonyAndMediaAudio1
CCP/TBS   org.bluez.TelephonyBearer1
GMAP/GMAS org.bluez.GamingAudio1
```

Each group is validated through BlueZ-shaped register/discover/read/control
or notify/error/release behavior, plus bluetoothd-shaped plugin/mainloop,
D-Bus object ownership, `InterfacesRemoved`, profile release, daemon release,
and mainloop exit.

Current result:

```text
PASS bluez-le-audio-coordinated-services
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-le-audio-coordinated-services/bluez-le-audio-coordinated-services.ble1.log
  PASS ble2 FeatherCore/build/bt-hwsim-usecases-le-audio-coordinated-services/bluez-le-audio-coordinated-services.ble2.log
```

Artifact:

```text
FeatherCore/build/bt-hwsim-usecases-le-audio-coordinated-services/run-results.json
```

Boundary: this strengthens LE Audio service/profile coverage.  It does not
replace the existing controller-owned CIS/BIS, BAP/ASCS/PACS, LC3, ISO socket,
MediaTransport owner-lost/reacquire, and daemon mainloop cleanup gates.

## LE Audio CAP/CSIP coordinated group hwsim gate

`bluez-le-audio-cap-csip-group` adds a focused CAP/CSIP orchestration gate on
top of the existing LE Audio service and data-plane coverage.  The case uses
bluetoothd-shaped LE Audio daemon setup, registers `CAPCoordinator1`, registers
source/sink CSIP `CoordinatedSet1` members, locks the set, then drives a
bidirectional coordinated ASE group through CAP config/enable/release.

New CAP commands used by the gate:

```text
bluezaudio le-cap-control group-config-bidir 0 1 2
bluezaudio le-cap-control group-enable-bidir 0 1 2
bluezaudio le-cap-control group-release-bidir 0 1 2
```

These commands log `ase-count=2`, source/sink ASE membership,
`policy=atomic-config`, `policy=atomic-enable`, and `policy=atomic-release` so
validator can distinguish the group orchestration from two unrelated single-ASE
operations.

Current result:

```text
PASS bluez-le-audio-cap-csip-group
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-le-audio-cap-csip-group/bluez-le-audio-cap-csip-group.ble1.log
  PASS ble2 FeatherCore/build/bt-hwsim-usecases-le-audio-cap-csip-group/bluez-le-audio-cap-csip-group.ble2.log
```

Artifacts:

```text
FeatherCore/build/logs/build-ble1-le-audio-cap-csip-group.log
FeatherCore/build/logs/build-ble2-le-audio-cap-csip-group.log
FeatherCore/build/logs/run-le-audio-cap-csip-group.log
FeatherCore/build/bt-hwsim-usecases-le-audio-cap-csip-group/run-results.json
```

Boundary: this is still BlueZ-shaped CAP/CSIP behavior inside the NuttX sim
port, not an unmodified upstream bluetoothd CAP implementation.  It does make
CAP coordinated-set ownership part of the mandatory hwsim evidence instead of
leaving CAP as a single-ASE helper log.

## LE Audio TMAP/MCP/CCP media-call orchestration hwsim gate

`bluez-le-audio-tmap-mcp-ccp-flow` adds a focused telephony/media control gate
for the BlueZ userspace port.  It ties TMAP role policy, MCP media control, and
CCP call control to the LE Audio unicast MediaTransport lifecycle instead of
validating those services only as isolated D-Bus/GATT-shaped probes.

The gate validates both directions:

```text
BLE1 source CIS1 -> BLE2 sink CIS1
BLE2 source CIS2 -> BLE1 sink CIS2
```

Covered behavior:

```text
TMAP/TMAS role register/discover/read/update/notify
MCP/MCS player/track read, play, pause, notify, search
CCP/TBS bearer/call-state read, originate, accept, notify, terminate
MediaTransport1 Acquire/write/read/release around the media/call flow
D-Bus InterfacesRemoved and daemon/mainloop cleanup
```

Current result:

```text
PASS bluez-le-audio-tmap-mcp-ccp-flow
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-le-audio-tmap-mcp-ccp-flow/bluez-le-audio-tmap-mcp-ccp-flow.ble1.log
  PASS ble2 FeatherCore/build/bt-hwsim-usecases-le-audio-tmap-mcp-ccp-flow/bluez-le-audio-tmap-mcp-ccp-flow.ble2.log
```

Artifacts:

```text
FeatherCore/build/logs/run-le-audio-tmap-mcp-ccp-flow.log
FeatherCore/build/bt-hwsim-usecases-le-audio-tmap-mcp-ccp-flow/run-results.json
```

Boundary: this remains BlueZ-shaped TMAP/MCP/CCP orchestration inside the NuttX
sim port, not unmodified upstream bluetoothd.  It does, however, make media and
call control part of the mandatory LE Audio unicast hwsim evidence.

## LE Audio Broadcast multi-BIS hwsim gate

`bluez-le-audio-broadcast-multibis` extends the broadcast validation from a
single BIS to two BIS substreams in the same BIG.  It keeps the broadcast source
and BASS sink ownership visible in logs and requires both BIS1 and BIS2 to pass
setup, bind, credit accounting, payload movement, and teardown.

Validated flow:

```text
BLE1 broadcast source BIG0/BIS1 -> BLE2 broadcast sink BIS1
BLE1 broadcast source BIG0/BIS2 -> BLE2 broadcast sink BIS2
```

Current result:

```text
PASS bluez-le-audio-broadcast-multibis
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-le-audio-broadcast-multibis/bluez-le-audio-broadcast-multibis.ble1.log
  PASS ble2 FeatherCore/build/bt-hwsim-usecases-le-audio-broadcast-multibis/bluez-le-audio-broadcast-multibis.ble2.log
```

Artifacts:

```text
FeatherCore/build/logs/run-le-audio-broadcast-multibis.log
FeatherCore/build/bt-hwsim-usecases-le-audio-broadcast-multibis/run-results.json
```

Boundary: this remains BlueZ-shaped broadcast/BASS/BAP/ISO behavior inside the
NuttX sim port, not unmodified upstream bluetoothd.  It does move the broadcast
gate beyond a single-BIS happy path by requiring two BIS/substream lifecycles.

## LE Audio Broadcast multi-BIS reconnect hwsim gate

`bluez-le-audio-broadcast-multibis-reconnect` repeats the multi-BIS broadcast
lifecycle twice in one sim lifetime.  It is intended to catch stale BASS source
state, ISO socket state, medium-file residue, or daemon broadcast-profile cleanup
bugs that a single happy path would miss.

Current result:

```text
PASS bluez-le-audio-broadcast-multibis-reconnect
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-le-audio-broadcast-multibis-reconnect/bluez-le-audio-broadcast-multibis-reconnect.ble1.log
  PASS ble2 FeatherCore/build/bt-hwsim-usecases-le-audio-broadcast-multibis-reconnect/bluez-le-audio-broadcast-multibis-reconnect.ble2.log
```

Artifacts:

```text
FeatherCore/build/logs/run-le-audio-broadcast-multibis-reconnect.log
FeatherCore/build/bt-hwsim-usecases-le-audio-broadcast-multibis-reconnect/run-results.json
```

Boundary: this is still BlueZ-shaped broadcast behavior in the NuttX sim port,
not an unmodified upstream bluetoothd broadcast manager.  It does make repeated
multi-BIS cleanup/restart part of the mandatory LE Audio hwsim evidence.

## LE Audio daemon integrated profile flow

`bluezaudio le-daemon integrated-profile-flow` is a daemon-owned composite flow
for the NuttX BlueZ port.  It moves the LE Audio hwsim gate away from an NSH-only
sequence of independent probe commands: one `bluezaudio` command now internally
orders CAP/CSIP, TMAP/MCP/CCP, BAP/ASCS, MediaEndpoint/MediaTransport, LC3/ISO,
and cleanup under bluetoothd-shaped ownership.

Current hwsim gate:

```text
PASS bluez-le-audio-daemon-integrated-profile
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-le-audio-daemon-integrated-profile/bluez-le-audio-daemon-integrated-profile.ble1.log
  PASS ble2 FeatherCore/build/bt-hwsim-usecases-le-audio-daemon-integrated-profile/bluez-le-audio-daemon-integrated-profile.ble2.log
```

Artifacts:

```text
FeatherCore/build/logs/build-ble1-le-audio-integrated-profile.log
FeatherCore/build/logs/build-ble2-le-audio-integrated-profile.log
FeatherCore/build/logs/run-le-audio-daemon-integrated-profile.log
FeatherCore/build/bt-hwsim-usecases-le-audio-daemon-integrated-profile/run-results.json
```

Boundary: this is still BlueZ-shaped daemon behavior inside the NuttX sim port,
not unmodified upstream bluetoothd.  It does make LE Audio less probe-like by
requiring a single daemon-owned integrated profile flow to drive services,
transport, codec payload, and cleanup.

## LE Audio daemon integrated profile reconnect validation

The hwsim suite now includes `bluez-le-audio-daemon-integrated-profile-reconnect`, which repeats the daemon-owned LE Audio integrated profile flow on both BLE1 and BLE2.

This gate exercises the BlueZ-facing LE Audio control surface through daemon/plugin/mainloop style ownership, CAP/CSIP coordinated group behavior, TMAP/MCP/CCP service behavior, D-Bus client transport lifecycle, unicast source/sink role swapping, LC3 encode/decode stubs, and ISO socket send/recv accounting.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-daemon-integrated-profile-reconnect.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-daemon-integrated-profile-reconnect/bluez-le-audio-daemon-integrated-profile-reconnect.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-daemon-integrated-profile-reconnect/bluez-le-audio-daemon-integrated-profile-reconnect.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-daemon-integrated-profile-reconnect/run-results.json`

## LE Audio BAP/PACS/ASCS ASE session validation

The hwsim suite now includes `bluez-le-audio-bap-pacs-ascs-session` for explicit BlueZ-shaped LE Audio unicast ASE session coverage.

This gate exercises PACS discovery/read behavior, ASCS ASE control point writes and notifications, BAP scheduler/policy decisions, ATT/GATT lifecycle, daemon-owned unicast profile flow, source/sink role swapping, LC3 payload handling, and ISO socket accounting.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-bap-pacs-ascs-session.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-session/bluez-le-audio-bap-pacs-ascs-session.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-session/bluez-le-audio-bap-pacs-ascs-session.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-session/run-results.json`

## LE Audio BAP/PACS/ASCS reconnect-recovery validation

The hwsim suite now includes `bluez-le-audio-bap-pacs-ascs-reconnect-recovery` for repeated LE Audio ASE lifecycle coverage with ASCS QoS rejection and retry.

This gate builds on the BAP/PACS/ASCS session validation and requires source/sink role rediscovery, `config-qos-reject` invalid-QoS response handling, successful retry through BAP policy reselection, daemon-owned unicast profile restart, LC3/ISO accounting, and repeated ATT/GATT cleanup.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-bap-pacs-ascs-reconnect-recovery.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-reconnect-recovery/bluez-le-audio-bap-pacs-ascs-reconnect-recovery.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-reconnect-recovery/bluez-le-audio-bap-pacs-ascs-reconnect-recovery.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-reconnect-recovery/run-results.json`

## LE Audio BAP/PACS/ASCS metadata reconfiguration validation

The hwsim suite now includes `bluez-le-audio-bap-pacs-ascs-metadata-reconfig` for LE Audio metadata/context update and release/reconfigure coverage.

This gate requires PACS context update and notification, ASCS `update-metadata`, BAP suspend/stop behavior, ASE release, second-round reconfiguration, daemon-owned unicast profile restart, LC3 payload handling, and ISO socket accounting on both BLE1 and BLE2.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-bap-pacs-ascs-metadata-reconfig.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-metadata-reconfig/bluez-le-audio-bap-pacs-ascs-metadata-reconfig.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-metadata-reconfig/bluez-le-audio-bap-pacs-ascs-metadata-reconfig.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-pacs-ascs-metadata-reconfig/run-results.json`

## LE Audio controller BAP/ASCS error-matrix validation

The hwsim suite now includes `bluez-le-audio-controller-bap-ascs-error-matrix` for controller/mgmt-driven LE Audio BAP/ASCS recovery coverage.

This gate requires HCI USER ISO setup, mgmt power/scan/connect/security/CIS request sequencing, PACS/ASCS discovery, ASCS invalid-QoS response, QoS reject/cancel recovery, metadata update, daemon-owned unicast profile completion, stream teardown, ATT/GATT cleanup, disconnect, and command-status error handling on both BLE1 and BLE2.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-controller-bap-ascs-error-matrix.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-bap-ascs-error-matrix/bluez-le-audio-controller-bap-ascs-error-matrix.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-bap-ascs-error-matrix/bluez-le-audio-controller-bap-ascs-error-matrix.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-bap-ascs-error-matrix/run-results.json`

## LE Audio BAP/ASCS D-Bus owner recovery validation

The hwsim suite now includes `bluez-le-audio-bap-ascs-dbus-owner-recovery` for tying MediaEndpoint/MediaTransport D-Bus ownership recovery to the BAP/ASCS ASE unicast path.

This gate requires owner loss, D-Bus interface removal, owner reacquire, MediaTransport re-addition, PACS/ASCS discovery, BAP policy selection, ASCS configuration, daemon-owned unicast profile completion, LC3/ISO accounting, and cleanup on both BLE1 and BLE2.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-bap-ascs-dbus-owner-recovery.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-ascs-dbus-owner-recovery/bluez-le-audio-bap-ascs-dbus-owner-recovery.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-ascs-dbus-owner-recovery/bluez-le-audio-bap-ascs-dbus-owner-recovery.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-bap-ascs-dbus-owner-recovery/run-results.json`

## LE Audio controller + D-Bus + BAP/ASCS reconnect validation

The hwsim suite now includes `bluez-le-audio-controller-dbus-bap-ascs-reconnect` for a combined LE Audio lifecycle spanning D-Bus owner recovery, BAP/ASCS ASE unicast, and controller/mgmt-driven reconnect.

This gate requires repeated owner loss/reacquire, repeated daemon-owned unicast profile completion, mgmt connect/security/CIS request sequencing, LC3/ISO accounting, disconnect, and command-status error handling on both BLE1 and BLE2.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-controller-dbus-bap-ascs-reconnect.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-dbus-bap-ascs-reconnect/bluez-le-audio-controller-dbus-bap-ascs-reconnect.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-dbus-bap-ascs-reconnect/bluez-le-audio-controller-dbus-bap-ascs-reconnect.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-controller-dbus-bap-ascs-reconnect/run-results.json`

## LE Audio umbrella role coverage validation

`bluez-le-audio-umbrella` was re-run after the recent LE Audio role/lifecycle gates and passes on BLE1/BLE2.

Covered roles include source/sink unicast roles, broadcast source/sink, CAP coordinator, BASS assistant, VCP, MICP, CSIP, MCP, TMAP, CCP, GMAP, MediaEndpoint/MediaTransport D-Bus ownership, BAP/PACS/ASCS ASE lifecycle, HCI USER ISO setup, mgmt connect/security/disconnect, LC3, and ISO accounting.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-umbrella-current.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-umbrella-current/bluez-le-audio-umbrella.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-umbrella-current/bluez-le-audio-umbrella.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-umbrella-current/run-results.json`

## LE Audio codec/QoS policy matrix validation

The hwsim suite now includes `bluez-le-audio-codec-qos-policy-matrix` for LC3 codec and BAP/ASCS QoS policy validation.

This gate requires LC3 codec selection, BAP QoS selection, ASCS invalid-QoS response, QoS retry, metadata update, direct LC3 payload handling, daemon-owned unicast profile completion, and ISO accounting on both BLE1 and BLE2.

Latest result: PASS on both BLE1 and BLE2.

Artifacts:

- `FeatherCore/build/logs/run-le-audio-codec-qos-policy-matrix.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-codec-qos-policy-matrix/bluez-le-audio-codec-qos-policy-matrix.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-codec-qos-policy-matrix/bluez-le-audio-codec-qos-policy-matrix.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-codec-qos-policy-matrix/run-results.json`

## LE Audio role-soak validation

The hwsim suite now includes `bluez-le-audio-role-soak` for repeated BLE1/BLE2 LE Audio source/sink role validation.

This gate requires the codec/QoS policy matrix, controller/mgmt reconnect, CIS setup, MediaEndpoint/MediaTransport owner lost/reacquire, daemon-owned unicast profile flow, LC3/ISO payload accounting, disconnect, and command-status error handling to survive another source/sink pass on both simulated BLE devices.

Result: `PASS bluez-le-audio-role-soak`.

Evidence:

- `FeatherCore/build/logs/run-le-audio-role-soak.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-role-soak/bluez-le-audio-role-soak.ble1.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-role-soak/bluez-le-audio-role-soak.ble2.log`
- `FeatherCore/build/bt-hwsim-usecases-le-audio-role-soak/run-results.json`

Remaining LE Audio gap: this improves role lifecycle confidence, but the port still needs deeper replacement of BlueZ-shaped probes with actual upstream daemon object/session ownership, especially Broadcast Assistant/Scan Delegator and CAP initiator/acceptor policy branches.
## LE Audio Broadcast/BASS Scan Delegator validation

The hwsim suite now includes `bluez-le-audio-bass-scan-delegator` on top of the existing multi-BIS broadcast validation.

Current validation:

```text
PASS bluez-le-audio-broadcast-multibis
PASS bluez-le-audio-bass-scan-delegator
```

Artifacts:

```text
FeatherCore/build/logs/build-ble1-role-fix.log
FeatherCore/build/logs/build-ble2-role-fix.log
FeatherCore/build/logs/run-le-audio-broadcast-multibis-current.log
FeatherCore/build/logs/run-le-audio-bass-scan-delegator.log
FeatherCore/build/bt-hwsim-usecases-le-audio-broadcast-multibis-current/run-results.json
FeatherCore/build/bt-hwsim-usecases-le-audio-bass-scan-delegator/run-results.json
```

This gate validates BLE1 broadcast source behavior across BIS1/BIS2, BLE2 BASS assistant source lifecycle, BLE2 Scan Delegator register/receive-state/notify/release, and peer ISO socket receive through the shared hwsim medium.

Build note: BLE1/BLE2 sim images must be rebuilt sequentially from their own defconfigs. A stale BLE1 image built with the BLE2 role causes ISO records to be written with the wrong source role and the peer receiver will skip them as self-originated records.

Work order after this LE Audio pass:

1. Close the remaining BT/BLE basic capability gates.
2. Continue A2DP completion.
3. Continue BT/BLE NET completion.
4. Then expand the remaining BT/BLE profile and controller coverage.
## BT/BLE basic capability closeout

After the LE Audio pass, the basic BT/BLE hwsim gate set was re-run and closed.

Validated cases:

```text
PASS bt-basic
PASS ble-basic
PASS l2cap-native-basic
PASS hci-le-reconnect-stress
PASS hci-le-pairing
PASS bluez-basic-mgmt-flow
PASS bluez-basic-scan-connect-auth-flow
PASS bluez-mgmt-pair-noio
PASS bluez-mgmt-cancel-pair
PASS bluez-mgmt-cancel-pair-pending
PASS bluez-mgmt-pair-unpair
PASS bluez-mgmt-reconnect-stress
PASS bluez-daemon-reconnect-stress
PASS bluez-daemon-pairing-matrix
PASS bluez-hciioctl-basic
PASS bluez-hciuser-adv-scan-medium
```

The closeout covers BR/EDR and BLE basics, upstream L2CAP socket lifecycle, LE HCI reconnect and pairing, BlueZ-shaped mgmt scan/connect/auth/disconnect/error handling, pairing cancel/unpair, reconnect stress, daemon pairing/reconnect policy, HCI ioctl, and HCI USER advertising/scanning over the shared hwsim medium.

Notable implementation fixes:

- HCI status now emits short `conn-hash` / `conn[]` lines before long slot detail lines.
- Peer-side BR/EDR and LE disconnect events now fall back to staged conn_hash cleanup if the event path leaves a connection behind.
- The basic passive-side state probes and LE pairing passive script were made non-blocking enough for the hwsim runner.
- The ADV/SCAN medium validator now keys off `hci-user-adv-hwsim-tx` and `hci-user-scan-hwsim-report` instead of long `slot=` status fields.

Next work item: resume A2DP completion before moving to BT/BLE NET.
## A2DP daemon integrated profile flow

The hwsim suite now includes `bluez-daemon-a2dp-integrated-profile`.

This adds a single-command daemon-owned A2DP/AVRCP flow:

```text
bluezdaemon audio-a2dp-integrated-flow source 2
bluezdaemon audio-a2dp-integrated-flow sink 1
```

Validation result:

```text
PASS bluez-daemon-a2dp-integrated-profile
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-a2dp-integrated-profile.log
FeatherCore/build/logs/build-bt2-a2dp-integrated-profile.log
FeatherCore/build/logs/run-bluez-daemon-a2dp-integrated-profile.log
FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-integrated-profile/run-results.json
```

Covered in one daemon-owned flow: audio plugin init/exit, mainloop watch ownership, `org.bluez` D-Bus object ownership, AVDTP discover/setconfig/open/start/suspend/close, A2DP SBC media payload, AVRCP control/browsing/metadata/player-settings, and transport/session cleanup.

Boundary: this is stronger than externally staged NSH sequencing, but still not a claim that unmodified upstream `bluetoothd` audio/AVRCP plugins are fully ported.
## A2DP daemon integrated reconnect flow

The hwsim suite now includes `bluez-daemon-a2dp-integrated-reconnect`.

Command shape:

```text
bluezdaemon audio-a2dp-integrated-reconnect source 2 2
bluezdaemon audio-a2dp-integrated-reconnect sink 1 2
```

Validation result:

```text
PASS bluez-daemon-a2dp-integrated-reconnect
```

Artifacts:

```text
FeatherCore/build/logs/build-bt1-a2dp-integrated-reconnect.log
FeatherCore/build/logs/build-bt2-a2dp-integrated-reconnect.log
FeatherCore/build/logs/run-bluez-daemon-a2dp-integrated-reconnect.log
FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-integrated-reconnect/run-results.json
```

This gate keeps plugin/mainloop/D-Bus object ownership alive across two A2DP/AVRCP lifecycle rounds and requires each round to return to closed/idle state before final cleanup.

Boundary: still BlueZ-shaped, not yet unmodified upstream `bluetoothd` audio/AVRCP plugin execution.

## 2026-06-14 LE Audio closeout and next work order

Current implementation order after LE Audio closeout:

1. LE Audio full-role hwsim coverage.
2. BT/BLE basic capability closeout.
3. A2DP completion.
4. BT/BLE NET completion.
5. Remaining BT/BLE functions.

The LE Audio umbrella acceptance gate has been re-run and passes:

```text
PASS bluez-le-audio-umbrella
  PASS ble1 FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-umbrella-closeout/bluez-le-audio-umbrella.ble1.log
  PASS ble2 FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-umbrella-closeout/bluez-le-audio-umbrella.ble2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-le-audio-umbrella-closeout/run-results.json
```

This is the current BlueZ LE Audio closeout gate for hwsim. It combines daemon/D-Bus ownership, controller setup/reconnect, LC3 codec transport, BIS broadcast, CIS unicast, BASS, BAP/PACS/ASCS, CAP/CSIP, coordinated services, TMAP/MCP/CCP, role soak, and cleanup coverage.

BT/BLE basic coverage is also considered closed for the current phase using the final basic closeout logs under `FeatherCore/build/logs/run-basic-closeout-final-*.log` plus `FeatherCore/build/logs/run-basic-closeout-final2-bluez-hciuser-adv-scan-medium.log`.

Boundary: this is an hwsim semantic closeout for the current port, not a claim that all upstream `bluetoothd` internals are byte-for-byte complete. The next work should remain focused on A2DP daemon/mainloop/D-Bus/AVDTP/L2CAP ownership, followed by BT/BLE NET.

## 2026-06-14 A2DP daemon session ownership

A new A2DP daemon ownership command is available:

```text
bluezdaemon audio-a2dp-session-ownership source|sink [peer] [rounds]
```

It wraps the daemon A2DP/AVRCP L2CAP and media path with an ownership tracker for AVDTP, AVCTP, media fd, L2CAP fd, endpoint, transport, player, and mainloop watch refs. The paired hwsim gate requires two source/sink rounds and final cleanup to leave all refs/fds/watches at zero.

Current evidence:

```text
PASS bluez-daemon-a2dp-session-ownership
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-session-ownership-v2/bluez-daemon-a2dp-session-ownership.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-session-ownership-v2/bluez-daemon-a2dp-session-ownership.bt2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-session-ownership-v2/run-results.json
```

Boundary: this improves daemon ownership semantics, but A2DP is still not fully equivalent to unmodified upstream `bluetoothd` audio internals. The next A2DP work should continue replacing helper-shaped behavior with imported upstream-style object/session ownership and tighter AVDTP/L2CAP error policy coverage.

## 2026-06-14 A2DP AVDTP/L2CAP error policy

A new daemon command is available:

```text
bluezdaemon audio-a2dp-error-policy source|sink [peer]
```

It drives one hwsim A2DP/AVRCP/media round and then checks BlueZ-shaped AVDTP/L2CAP error policy:

- `start-before-open` is rejected without opening media transport.
- `duplicate-open` is rejected without leaking session state.
- `media-before-start` is rejected with the media fd still closed.
- `l2cap-drop-streaming` aborts AVDTP/AVCTP/media ownership back to idle.
- `remote-close-after-abort` is idempotent and leaves ownership closed.

Current evidence:

```text
PASS bluez-daemon-a2dp-error-policy
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-error-policy/bluez-daemon-a2dp-error-policy.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-error-policy/bluez-daemon-a2dp-error-policy.bt2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-error-policy/run-results.json
```

Boundary: this adds explicit daemon error-policy semantics, but it is still part of the staged convergence toward upstream `bluetoothd`; it is not yet a claim that unmodified BlueZ A2DP internals are complete.

## 2026-06-14 A2DP upstream-style object/session callbacks

A new daemon command is available:

```text
bluezdaemon audio-a2dp-upstream-session source|sink [peer]
```

It exercises a BlueZ-shaped A2DP object/session lifecycle around the hwsim media path:

- `profile.c`: register/connect/disconnect/unregister.
- `device.c`: device object ownership and services-resolved state.
- `a2dp.c`: source/sink profile callbacks.
- `avdtp.c`: session and stream callbacks.
- `media.c`: endpoint, transport, Acquire, Release callbacks.
- `avrcp.c`: player and notification callbacks.

Current evidence:

```text
PASS bluez-daemon-a2dp-upstream-session
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-session/bluez-daemon-a2dp-upstream-session.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-session/bluez-daemon-a2dp-upstream-session.bt2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-session/run-results.json
```

Boundary: this is upstream-shaped semantic coverage and a useful convergence point, but not yet proof that unmodified upstream `bluetoothd` A2DP internals are fully driving the path. Continue A2DP by moving more helper-owned session/object behavior into imported upstream-style code.

## 2026-06-14 A2DP upstream-style persistent reconnect

A new daemon command is available:

```text
bluezdaemon audio-a2dp-upstream-reconnect source|sink [peer] [rounds]
```

It keeps the BlueZ-shaped profile registration persistent across multiple A2DP reconnect rounds. Each round creates device/session/AVDTP/media/AVRCP state, runs the hwsim L2CAP/media path, then disconnects and verifies transient ownership cleanup. Final daemon exit unregisters the profile and releases all D-Bus/profile objects.

Current evidence:

```text
PASS bluez-daemon-a2dp-upstream-reconnect
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-reconnect/bluez-daemon-a2dp-upstream-reconnect.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-reconnect/bluez-daemon-a2dp-upstream-reconnect.bt2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-reconnect/run-results.json
```

Boundary: this improves reconnect and lifecycle stability semantics. It still is not a claim that unmodified upstream BlueZ A2DP code owns every transaction directly; continue A2DP by moving helper-owned transaction/session code into imported upstream-style objects.

## 2026-06-14 A2DP upstream AVDTP transaction ownership

A new daemon command is available:

```text
bluezdaemon audio-a2dp-upstream-transactions source|sink [peer]
```

It validates `avdtp.c`-style transaction ownership around the A2DP hwsim path:

- Discover, GetCapabilities, SetConfiguration, Open, Start, Suspend, Close, and Abort transaction ids.
- pending request and timer ownership.
- response completion and timer disarm.
- Suspend timeout plus retry cleanup.
- Abort cancel cleanup.
- final zero refs/fds/watches/sessions.

Current evidence:

```text
PASS bluez-daemon-a2dp-upstream-transactions
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-transactions/bluez-daemon-a2dp-upstream-transactions.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-transactions/bluez-daemon-a2dp-upstream-transactions.bt2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-upstream-transactions/run-results.json
```

Boundary: this is stronger AVDTP transaction semantic coverage, but the A2DP port still needs continued replacement of staged helper-owned transaction/session behavior with imported upstream-style object code.

## 2026-06-14 A2DP MediaTransport fd ownership

A new daemon command is available:

```text
bluezdaemon audio-a2dp-media-transport-fd source|sink [peer]
```

It validates BlueZ `MediaTransport1` fd handoff semantics around the A2DP hwsim path:

- object add/remove for `/org/bluez/hci0/dev_xx/fd0`.
- `Acquire` fd ownership and MTU reporting.
- duplicate `Acquire` busy handling.
- `TryAcquire` defer handling.
- `PropertiesChanged` for state, delay, volume, and endpoint.
- `Release` fd close and ref cleanup.
- re-acquire after release.
- final zero owner/ref/fd/watch/session state.

Current evidence:

```text
PASS bluez-daemon-a2dp-media-transport-fd
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-media-transport-fd/bluez-daemon-a2dp-media-transport-fd.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-media-transport-fd/bluez-daemon-a2dp-media-transport-fd.bt2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-media-transport-fd/run-results.json
```

Boundary: this strengthens MediaTransport semantics, but it is still a staged convergence point. Continue A2DP by moving fd/transport ownership from the daemon harness into imported upstream-style BlueZ object code.

## 2026-06-14 A2DP codec capability and reconfigure policy

A new daemon command is available:

```text
bluezdaemon audio-a2dp-codec-policy source|sink [peer]
```

It validates A2DP codec policy around the hwsim media path:

- Source/Sink endpoint discovery.
- SBC SelectConfiguration and SetConfiguration.
- unsupported codec rejection.
- invalid capability rejection.
- reconfigure and suspend/apply reconfigure.
- unsupported content-protection rejection.
- endpoint clear and final cleanup.

Current evidence:

```text
PASS bluez-daemon-a2dp-codec-policy
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-codec-policy/bluez-daemon-a2dp-codec-policy.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-codec-policy/bluez-daemon-a2dp-codec-policy.bt2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-codec-policy/run-results.json
```

Boundary: this strengthens A2DP codec and endpoint semantics, but it is still staged convergence. Continue A2DP by moving codec/endpoint policy from daemon harness logs into imported upstream-style BlueZ object implementation.

## 2026-06-14 A2DP current closeout umbrella

A new daemon command is available:

```text
bluezdaemon audio-a2dp-closeout-full source|sink [peer]
```

It is the current hwsim A2DP closeout gate. It combines the major A2DP semantics covered by the narrower gates:

- profile/session ownership.
- AVDTP transaction timeout/retry/cancel cleanup.
- MediaTransport1 D-Bus fd ownership.
- SBC codec capability and reconfigure policy.
- AVDTP/L2CAP error policy.
- persistent profile reconnect cleanup.
- final all-zero refs/fds/watches/sessions invariant.

Current evidence:

```text
PASS bluez-daemon-a2dp-closeout-full
  PASS bt1 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-closeout-full/bluez-daemon-a2dp-closeout-full.bt1.log
  PASS bt2 FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-closeout-full/bluez-daemon-a2dp-closeout-full.bt2.log
RUN-MANIFEST FeatherCore/build/bt-hwsim-usecases-bluez-daemon-a2dp-closeout-full/run-results.json
```

Boundary: this is the current hwsim semantic closeout for A2DP, not proof that unmodified upstream `bluetoothd` A2DP code directly owns every object without harness assistance. It is a practical phase boundary for moving on to BT/BLE NET while preserving the remaining upstream-internal replacement work as known debt.

## BlueZ Network current closeout umbrella gate

`blueznetwork closeout-full begin|end` marks the current BT Network/BNEP closeout boundary, and the paired hwsim case is `bluez-network-closeout-full`.

Current result:

```text
PASS bluez-network-closeout-full
  PASS bt1 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-bluez-network-closeout-full/bluez-network-closeout-full.bt1.log
  PASS bt2 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-bluez-network-closeout-full/bluez-network-closeout-full.bt2.log
RUN-MANIFEST /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-bluez-network-closeout-full/run-results.json
```

The validator requires the closeout markers plus daemon-profile registration, `org.bluez.NetworkServer1` PANU/NAP/GN service ownership, `org.bluez.Network1` connect/disconnect, connected L2CAP fd handoff through `BNEPCONNADD`, `btn0` ping and 1400-byte MTU ping, error lifecycle, native BNEP datapath counters, and final `bnep-native-active=0` cleanup.

Boundary: this is the current hwsim semantic closeout for the BlueZ-derived Network adapter. It is not yet proof that unmodified upstream `bluetoothd` Network plugin, real system D-Bus, SDP server/client, policy agent, multi-peer PAN server, and fully controller-driven upstream L2CAP ownership are complete.

## BLE 6LoWPAN/IPSP current closeout umbrella gate

The paired hwsim case `ble-ip-closeout-full` closes the current BLE NET stage. It runs BLE1/BLE2 through two `6lowpan-up -> bt0 IPv6 -> data -> 6lowpan-down` rounds and validates ping6, TCP iperf, UDP iperf/fragment counters, IPHC ownership, and cleanup.

Current result:

```text
PASS ble-ip-closeout-full
  PASS ble1 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-closeout-full/ble-ip-closeout-full.ble1.log
  PASS ble2 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-closeout-full/ble-ip-closeout-full.ble2.log
RUN-MANIFEST /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-closeout-full/run-results.json
```

Boundary: this gate validates the current hwsim BLE 6LoWPAN/IPSP semantic path and imported `net_6lowpan/iphc` participation. It still reports `upstream-owner=bridge-ipsp`, so complete upstream Linux `net/bluetooth/6lowpan.c` and BlueZ IPSP/Profile ownership remain future upstream-alignment work.

## BLE 6LoWPAN upstream-owner bridge hook

`ble-ip-closeout-full` now requires imported Bluetooth `net/bluetooth/6lowpan.c` ownership counters in addition to generic `net_6lowpan/iphc` datapath evidence. The status owner is now:

```text
upstream-owner=net_bluetooth/6lowpan+bridge-ipsp
```

and the validator requires non-zero `upstream-owner-netdev-register`, `upstream-owner-netdev-unregister`, `upstream-owner-tx`, and `upstream-owner-rx`.

Current result:

```text
PASS ble-ip-closeout-full
  PASS ble1 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-owner-v2/ble-ip-closeout-full.ble1.log
  PASS ble2 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-owner-v2/ble-ip-closeout-full.ble2.log
RUN-MANIFEST /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-owner-v2/run-results.json
```

Boundary: this is a bridge hook into the imported upstream Bluetooth 6LoWPAN file, not the final full upstream ownership. The remaining work is to move netdev setup, peer/session ownership, LE CoC callbacks, `bt_xmit()`, and RX delivery into the imported `net/bluetooth/6lowpan.c` object graph.

## BLE 6LoWPAN upstream peer/CoC lifecycle hooks

`ble-ip-closeout-full` now requires imported Bluetooth `net/bluetooth/6lowpan.c` peer and LE CoC lifecycle counters in addition to netdev/TX/RX owner counters:

```text
upstream-owner-peer-add=<non-zero>
upstream-owner-peer-del=<non-zero>
upstream-owner-coc-open=<non-zero>
upstream-owner-coc-close=<non-zero>
upstream-owner-xmit=<non-zero>
upstream-owner-rx-deliver=<non-zero>
```

Current result:

```text
PASS ble-ip-closeout-full
  PASS ble1 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-lifecycle/ble-ip-closeout-full.ble1.log
  PASS ble2 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-lifecycle/ble-ip-closeout-full.ble2.log
RUN-MANIFEST /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-lifecycle/run-results.json
```

Boundary: these are bridge-fed upstream ownership hooks. Full upstream replacement still means moving real `setup_netdev()`, L2CAP CoC callbacks, peer lookup, `bt_xmit()`, and RX skb delivery into the imported `net/bluetooth/6lowpan.c` object graph.

## BLE 6LoWPAN upstream callback semantic hooks

The `ble-ip-closeout-full` gate now requires upstream Bluetooth 6LoWPAN callback/object semantic counters:

```text
upstream-owner-setup-netdev=<non-zero>
upstream-owner-delete-netdev=<non-zero>
upstream-owner-chan-ready-cb=<non-zero>
upstream-owner-chan-close-cb=<non-zero>
upstream-owner-bt-xmit=<non-zero>
upstream-owner-recv-cb=<non-zero>
```

Current result:

```text
PASS ble-ip-closeout-full
  PASS ble1 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-callbacks/ble-ip-closeout-full.ble1.log
  PASS ble2 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-callbacks/ble-ip-closeout-full.ble2.log
RUN-MANIFEST /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-ble-ip-upstream-callbacks/run-results.json
```

Boundary: these are still bridge-fed semantic hooks. Full upstream ownership requires moving the actual implementation behind these counters into imported `net/bluetooth/6lowpan.c` object and callback flow.
## 2026-06-14 A2DP SDP profile ownership closeout

`bluezdaemon audio-a2dp-closeout-full` now also validates A2DP SDP profile
ownership:

```text
PASS bluez-daemon-a2dp-closeout-full
```

New evidence:

- AudioSource and AudioSink service record registration.
- Source/Sink UUIDs and profile version.
- AVDTP signaling PSM, AVCTP control PSM, and AVCTP browsing PSM.
- Remote service discovery and resolve.
- Record unregister and discovery cache cleanup.

Logs:

- `FeatherCore/build/logs/run-a2dp-sdp-closeout.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-sdp-closeout/run-results.json`

Boundary: this strengthens profile/service discovery coverage, but it is still
not a final claim that the complete upstream `bluetoothd` SDP server/client and
audio plugin mainloop are running unchanged on NuttX.

## 2026-06-14 A2DP mainloop/watch ownership closeout

`bluezdaemon audio-a2dp-closeout-full` now also validates bluetoothd-shaped
mainloop ownership:

```text
PASS bluez-daemon-a2dp-closeout-full
```

New evidence:

- `mainloop.c` / `io-mainloop.c` ownership markers.
- Watch add/remove for mgmt, L2CAP signaling, L2CAP media, AVDTP, AVCTP,
  MediaTransport, and D-Bus.
- Timeout add/remove for AVDTP retry and AVRCP notify.
- Mainloop dispatch coverage for mgmt, L2CAP, AVDTP, AVCTP, media, and D-Bus.
- Final cleanup with `dispatch-pending=0`, `watches=0`, and `timers=0`.

Logs:

- `FeatherCore/build/logs/run-a2dp-mainloop-ownership.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-mainloop-ownership/run-results.json`

Boundary: this improves A2DP daemon mainloop/fd lifetime evidence, but it is
still a BlueZ-shaped NuttX sim port rather than a final claim that upstream
`bluetoothd` audio plugin runs unchanged.

## 2026-06-14 A2DP D-Bus owner recovery closeout

`bluezdaemon audio-a2dp-closeout-full` now requires and passes a D-Bus owner
recovery path for A2DP:

```text
PASS bluez-daemon-a2dp-closeout-full
```

Validated evidence:

- External owner loss for `MediaEndpoint1`, `MediaTransport1`, and `MediaPlayer1`.
- Interface removal and object re-add.
- Owner reacquire.
- `MediaTransport1.Acquire` / `Release` after reacquire.
- Final cleanup with no remaining D-Bus owners, endpoint refs, transport refs,
  player refs, interfaces, acquire refs, media fd, or watches.

Logs:

- `FeatherCore/build/logs/run-a2dp-dbus-owner-recovery.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-dbus-owner-recovery/run-results.json`

Boundary: this is stronger bluetoothd-shaped object lifetime coverage, not yet
a claim that the unmodified upstream `bluetoothd` audio plugin is fully running
on NuttX.  The next A2DP work should continue replacing helper/probe flow with
real upstream `profiles/audio/*` object and mainloop ownership.

## 2026-06-14 Network/BNEP upstream ownership closeout

当前 `blueznetwork` demo 已经通过完整 Network closeout 用例：

```text
PASS bluez-network-closeout-full
```

验证产物：

- `FeatherCore/build/logs/run-bluez-network-upstream-callbacks-v4.log`
- `FeatherCore/build/bt-hwsim-usecases-bluez-network-upstream-callbacks-v4/run-results.json`

本轮新增/验证的语义点：

- BlueZ daemon/profile registration and unregister path。
- `org.bluez.NetworkServer1` PANU/NAP/GN role semantics。
- `profiles/network/connection.c` + `profiles/network/bnep.c` fd handoff shape。
- native BNEP socket ioctl ownership: `GETCONNLIST`、`GETCONNINFO`、`GETSUPPFEAT`、`CONNADD`、`CONNDEL`。
- native BNEP netdev ownership: setup、register、unregister、`ndo_start_xmit`。
- native BNEP session ownership: create、start、thread、kthread、link、unlink、terminate、stop。
- native BNEP datapath: TX frame、L2CAP RX、RX frame、netif RX。
- 1400-byte ping MTU datapath and error lifecycle。

边界说明：

- 这里已经证明 Network/BNEP demo 不是只走 staging ping，而是必须经过 upstream BNEP core/netdev/sock/session 的关键路径。
- 这仍是 NuttX sim hwsim 语义对齐实现；后续还要继续扩展到更完整的 bluetoothd object ownership、policy、security，以及 A2DP/LE Audio 之外的剩余 BT/BLE profile。

## 2026-06-14 BlueZ bneptest native BNEP datapath closeout

`bluezbneptest` 现在不仅验证 fd handoff diagnostic，也用于验证 BlueZ-derived BNEP userspace ABI 到 imported Linux BNEP native datapath 的 ping/TCP/UDP 闭环。

实现与验证要点：

- `pan-up` 通过 connected L2CAP fd 调用 BNEP `BNEPCONNADD`，native side 记录 `bnep-native-fd-handoff`、`bnep-native-fd-active`、PSM/CID/handle/role。
- `status` 打印 `linux_bt_upstream_af_status()`，用于在 iperf 完成后捕获 native BNEP TX/RX 计数。
- hwsim 用例 `bluez-bneptest-ping`、`bluez-bneptest-iperf-tcp`、`bluez-bneptest-iperf-tcp-reverse`、`bluez-bneptest-iperf-udp`、`bluez-bneptest-iperf-udp-reverse` 已通过。
- validator 要求 fd handoff active、`btn0`、BNEP native datapath counters、非零 throughput 和 cleanup evidence。

通过日志：

```text
FeatherCore/build/logs/run-bluez-bneptest-datapath-fd-native-closeout.log
FeatherCore/build/bt-hwsim-usecases-bluez-bneptest-datapath-fd-native-closeout/run-results.json
```

边界：这是 BlueZ-derived tool path 的 native BNEP data-plane closeout，不等同于完整 upstream `bluetoothd` Network plugin closeout。后续仍按 A2DP -> BT/BLE NET -> remaining BT/BLE profiles 的顺序推进。

## 2026-06-14 A2DP object lifecycle aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now emits and validates an aggregate object lifecycle line based on `struct bluez_daemon_a2dp_session` state, not just fixed phase markers.

The closeout gate now requires both source and sink to report:

```text
session-object=bluez_daemon_a2dp_session rounds=2 active-transitions=2 idle-transitions=2
acquire-events=<non-zero> release-events=<non-zero> balanced-rounds=2 zero-ref-rounds=2 final-balanced=1 final-zero=1
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-object-lifecycle/run-results.json
```

This tightens A2DP session/ref ownership semantics while keeping the remaining boundary explicit: this is still a NuttX BlueZ daemon-shaped adapter path, not a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP AVDTP transaction lifecycle evidence

`bluezdaemon audio-a2dp-closeout-full` now tracks AVDTP transactions through the actual L2CAP command/response path.

The closeout gate requires source and sink to report non-zero balanced counters for:

```text
avdtp-transaction-begin
avdtp-transaction-complete
transaction-balanced-rounds=2
transaction-final-balanced=1
```

Source-side begin/complete maps to L2CAP write command and recv response. Sink-side begin/complete maps to L2CAP recv command and write response.

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-transaction-object/run-results.json
```

This reduces the staged-marker portion of the A2DP closeout, while still leaving the broader boundary explicit: the path is a NuttX BlueZ daemon-shaped adapter, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP MediaTransport datapath aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now records MediaTransport payload direction and byte counters in `bluez_daemon_a2dp_session`.

The hwsim closeout gate requires:

```text
source: media-payload-tx=<non-zero> media-payload-rx=0 media-payload-tx-bytes=<non-zero> media-role-ok=1
sink:   media-payload-tx=0 media-payload-rx=<non-zero> media-payload-rx-bytes=<non-zero> media-role-ok=1
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-media-datapath-object/run-results.json
```

This ties the A2DP MediaTransport/SBC data path to session object accounting. It is still a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP AVDTP state-machine evidence

`bluezdaemon audio-a2dp-closeout-full` now advances an internal AVDTP state machine on transaction completion.

The closeout gate requires both source and sink to complete two rounds of:

```text
IDLE -> CONFIGURED -> OPEN -> STREAMING -> SUSPENDED -> CLOSED
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-state-machine/run-results.json
```

This strengthens A2DP beyond static phase markers by validating transaction completion, ordered AVDTP state progression, MediaTransport payload direction/bytes, and final session/ref cleanup. It remains a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP MediaTransport ownership aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now records MediaTransport Acquire/Release and fd open/close ownership in `bluez_daemon_a2dp_session`.

The closeout gate requires both source and sink to report:

```text
transport-acquire=2 transport-release=2 transport-fd-open=2 transport-fd-close=2
transport-busy=0 transport-state-errors=0 transport-owner-rounds=2 transport-final-ok=1
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-mediatransport-ownership/run-results.json
```

This ties MediaTransport ownership to actual media L2CAP open/close behavior. It remains a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP AVRCP command/response aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now records AVRCP control and browsing command/response ownership in `bluez_daemon_a2dp_session`.

The closeout gate requires:

```text
source: avrcp-command-tx>0 avrcp-control-tx>0 avrcp-browse-tx>0 avrcp-command-rx=0 avrcp-response-tx=0
sink:   avrcp-command-rx>0 avrcp-response-tx>0 avrcp-control-rx>0 avrcp-browse-rx>0 avrcp-command-tx=0
both:   avrcp-role-errors=0 avrcp-owner-rounds=2 avrcp-final-ok=1
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-avrcp-object/run-results.json
```

This ties AVRCP control/browsing behavior to actual L2CAP command/response paths. It remains a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP codec/SBC datapath aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now records codec configuration and SBC encode/decode ownership in `bluez_daemon_a2dp_session`.

The closeout gate requires:

```text
source: codec-configured=2 sbc-encode>0 sbc-decode=0 sbc-encode-bytes>0 codec-final-ok=1
sink:   codec-configured=2 sbc-encode=0 sbc-decode>0 sbc-decode-bytes>0 codec-final-ok=1
both:   codec-role-errors=0 codec-owner-rounds=2
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-codec-object/run-results.json
```

This ties A2DP codec configuration and SBC payload handling to actual AVDTP/MediaTransport paths. It remains a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP L2CAP controller aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now records L2CAP controller/channel ownership in `bluez_daemon_a2dp_session`.

The closeout gate requires:

```text
l2cap-open>0 l2cap-connect>0 l2cap-write>0 l2cap-recv>0 l2cap-close>0
l2cap-avdtp>0 l2cap-avrcp>0 l2cap-media=2
l2cap-state-errors=0 l2cap-owner-rounds=2 l2cap-final-ok=1
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-l2cap-owner/run-results.json
```

This ties A2DP channel ownership to actual L2CAP socket helper behavior for AVDTP, AVRCP, and media transport. It remains a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP D-Bus object lifecycle aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now records D-Bus object lifecycle ownership through `bluez_daemon_a2dp_dbus_lifecycle`.

The closeout gate requires:

```text
name-acquire=1 get-managed-objects=1 interfaces-added=3
endpoint-added=1 transport-added=1 player-added=1
owner-lost=1 interfaces-removed=5 owner-reacquire=1 objects-readd=3
name-release=1 state-errors=0 final-ok=1
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-dbus-object-lifecycle/run-results.json
```

This ties D-Bus ObjectManager/MediaEndpoint/MediaTransport/MediaPlayer ownership to actual own/drop/recovery calls. It remains a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP mainloop dispatch aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now records mainloop/watch/timer ownership through `bluez_daemon_a2dp_mainloop_lifecycle`.

The closeout gate requires:

```text
watch-add=7 watch-remove=7 timer-add=2 timer-remove=2 dispatch-mgmt=1
dispatch-l2cap>0 dispatch-avdtp>0 dispatch-avctp>0 dispatch-media>0 dispatch-dbus>0
state-errors=0 final-ok=1
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-mainloop-lifecycle/run-results.json
```

This ties the A2DP mainloop evidence to actual D-Bus, L2CAP, AVDTP, AVCTP, and MediaTransport dispatch paths. It remains a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 A2DP profile/SDP/device lifecycle aggregate evidence

`bluezdaemon audio-a2dp-closeout-full` now records profile, SDP, and device lifecycle ownership through `bluez_daemon_a2dp_profile_lifecycle`.

The closeout gate requires:

```text
profile-register=1 profile-unregister=1
device-connect=2 device-disconnect=2
sdp-register=1 sdp-unregister=1 service-discovery=1 service-resolve=1 cache-remove=1
state-errors=0 final-ok=1
```

Validated hwsim result:

```text
PASS bluez-daemon-a2dp-closeout-full
FeatherCore/build/bt-hwsim-usecases-a2dp-profile-lifecycle/run-results.json
```

This ties profile registration, SDP discovery/cache cleanup, and device connect/disconnect to closeout lifecycle accounting. It remains a NuttX BlueZ daemon-shaped adapter path, not yet a fully unmodified upstream `bluetoothd` audio plugin instance.

## 2026-06-14 LE Audio full-role umbrella status

`bluez-le-audio-umbrella` is now the LE Audio full-role hwsim gate for this port. It validates both BLE roles through daemon/profile/D-Bus/control-service/ISO/LC3 evidence instead of only checking individual staged commands.

Validated coverage:

- BlueZ audio plugin init, adapter probe, profile accept, release, and plugin exit.
- D-Bus ObjectManager, MediaEndpoint, MediaTransport, and owner lost/reacquire lifecycle.
- BAP/PACS/ASCS unicast source and sink ASE flows.
- Broadcast source/sink BIG/BIS flow with BASS assistant and broadcast security states.
- CAP/CSIP coordinated group flow.
- VCP, MICP, MCP, TMAP, CCP, and GMAP service/control/error paths.
- ISO socket open/bind/connect/listen/accept/sendmsg/recvmsg/timestamp/error/shutdown/close.
- ISO QoS setup, controller timing, credits, and teardown.
- LC3 encode/write and recv/decode dataplane.
- HCI USER bidirectional ISO setup monitor.

Latest hwsim result:

```text
PASS bluez-le-audio-umbrella
  PASS ble1 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-le-audio-umbrella/bluez-le-audio-umbrella.ble1.log
  PASS ble2 /home/uan/Feather-develop-BT/FeatherCore/build/bt-hwsim-usecases-le-audio-umbrella/bluez-le-audio-umbrella.ble2.log
```

Artifacts:

```text
FeatherCore/build/logs/build-ble1-le-audio-umbrella.log
FeatherCore/build/logs/build-ble2-le-audio-umbrella.log
FeatherCore/build/logs/run-le-audio-umbrella.log
FeatherCore/build/bt-hwsim-usecases-le-audio-umbrella/run-results.json
```

Working order from here: close BT/BLE basic ability next, then return to A2DP and BT/BLE NET, then finish the remaining BT/BLE profiles and upstream replacement work.

## 2026-06-14 LE Audio first, then BT/BLE basic sequence closeout

This sequence reruns the current priority order explicitly: LE Audio umbrella first, then BT/BLE basic upstream convergence across BT1/BT2/BLE1/BLE2. Both stages were rebuilt before their hwsim runs.

LE Audio sequence closeout: PASS

- case: `bluez-le-audio-umbrella`
- build: `FeatherCore/build/logs/build-ble1-le-audio-sequence-closeout.log`
- build: `FeatherCore/build/logs/build-ble2-le-audio-sequence-closeout.log`
- run: `FeatherCore/build/logs/run-le-audio-sequence-closeout.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-le-audio-sequence-closeout/run-results.json`

BT/BLE basic sequence closeout: PASS

- case: `bluez-basic-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-basic-sequence-closeout.log`
- build: `FeatherCore/build/logs/build-bt2-basic-sequence-closeout.log`
- build: `FeatherCore/build/logs/build-ble1-basic-sequence-closeout.log`
- build: `FeatherCore/build/logs/build-ble2-basic-sequence-closeout.log`
- run: `FeatherCore/build/logs/run-basic-sequence-closeout.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-basic-sequence-closeout/run-results.json`

Current order: LE Audio umbrella PASS, then BT/BLE basic PASS. Continue with A2DP and BT/BLE NET next, then the remaining BT/BLE profile/security/policy/error-path coverage. Boundary remains staged adapter coverage, not a claim that unmodified upstream `bluetoothd` and Linux kernel Bluetooth are fully running without adaptation.

## 2026-06-14 A2DP upstream AVDTP signaling sequence closeout

The A2DP upstream convergence closeout now includes an explicit AVDTP signaling sequence gate in addition to the existing A2DP session gate.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-avdtp-signaling-closeout.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-avdtp-signaling-closeout.log`
- run: `FeatherCore/build/logs/run-a2dp-avdtp-signaling-closeout.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-signaling-closeout/run-results.json`

New required gate:

```text
upstream-avdtp-signaling=discover:1,getcap:1,set-config:1,open:1,start:1,suspend:1,close:1,abort:1,total:8
```

Both source and sink emit `final-ok=1`. Boundary remains `upstream-handler-families-mapped-not-yet-linked-media-transport-c-handlers`; this is a stronger upstream semantic bridge for A2DP/AVDTP session behavior, not a claim that unmodified upstream `bluetoothd` audio internals are fully linked and running without adaptation.

## 2026-06-14 A2DP upstream linked handler/mainloop closeout

The A2DP upstream convergence closeout now includes a linked handler/mainloop gate in addition to the existing AVDTP signaling and A2DP session gates.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-linked-handler-mainloop.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-linked-handler-mainloop.log`
- run: `FeatherCore/build/logs/run-a2dp-linked-handler-mainloop.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-linked-handler-mainloop/run-results.json`

New required gate:

```text
upstream-linked-handler-mainloop=transport-dispatch:1,media-dispatch:1,pending:1,watch:1,cleanup:1,total:5
```

Both source and sink emit `final-ok=1`. This moves the bridge closer to upstream `media.c`/`transport.c` handler and mainloop ownership semantics while keeping the explicit boundary that the upstream audio daemon objects are not yet linked and running fully unmodified.

## 2026-06-14 A2DP upstream profile daemon flow closeout

The A2DP upstream convergence closeout now includes a profile-daemon lifecycle gate on top of the AVDTP signaling and linked handler/mainloop gates.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-profile-daemon-flow.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-profile-daemon-flow.log`
- run: `FeatherCore/build/logs/run-a2dp-profile-daemon-flow.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-profile-daemon-flow/run-results.json`

New required gate:

```text
upstream-a2dp-profile-daemon=plugin-init:1,adapter-probe:1,endpoint-register:1,avdtp-bind:1,transport-export:1,daemon-cleanup:1,total:6
```

Both source and sink emit `final-ok=1`. This strengthens the staged bridge toward upstream audio plugin/profile lifecycle semantics while keeping the explicit boundary that upstream `bluetoothd` audio internals are not yet linked and running fully unmodified.

## 2026-06-14 A2DP upstream audio header/API link probe closeout

This change adds a real apps-side compilation unit, `bluez/upstream_audio_link_probe.c`, to `CONFIG_LINUX_BLUEZ_DAEMON`. It includes upstream BlueZ audio headers through the existing `bluez/upstream -> third/bluez` symlink and validates the A2DP/AVDTP/media/transport API surface at compile time.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-upstream-audio-link-probe.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-upstream-audio-link-probe.log`
- run: `FeatherCore/build/logs/run-a2dp-upstream-audio-link-probe.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-upstream-audio-link-probe/run-results.json`

Required marker:

```text
bluez-daemon: a2dp upstream-audio-link-probe role=source|sink compile-unit=bluez/upstream_audio_link_probe.c source=bluez/upstream->third/bluez headers=avdtp.h,a2dp-codecs.h,a2dp.h,media.h,transport.h api=headers:1,callbacks:1,constants:1,profile:1,transport:1,total:5 boundary=upstream-headers-linked-not-yet-upstream-c-objects final-ok=1
```

This exposed and fixed a real compat gap: upstream `avdtp.h` needs a `GIOChannel` forward type. Boundary remains explicit: this links the upstream header/API surface into the NuttX apps build, but does not yet link the upstream audio `.c` implementation objects fully unmodified.

## 2026-06-14 A2DP upstream AVDTP packet implementation probe closeout

The A2DP upstream link probe now includes an implementation-slice check for AVDTP packet/header behavior derived from `third/bluez/profiles/audio/avdtp.c`: single/start/continue headers, transaction modulo handling, fragment packet types, and signaling id mapping.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-avdtp-packet-impl-probe.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-avdtp-packet-impl-probe.log`
- run: `FeatherCore/build/logs/run-a2dp-avdtp-packet-impl-probe.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-packet-impl-probe/run-results.json`

Required marker:

```text
bluez-daemon: a2dp upstream-avdtp-packet-impl-probe role=source|sink compile-unit=bluez/upstream_audio_link_probe.c source=third/bluez/profiles/audio/avdtp.c impl=packet-headers:1,transactions:1,fragments:1,signals:1,total:4 boundary=upstream-avdtp-packet-impl-ported-not-yet-avdtp-c-object final-ok=1
```

This is a small but real movement from upstream headers toward implementation behavior. The full upstream `avdtp.c` object remains a future linkage step.

## 2026-06-14 A2DP upstream AVDTP parse implementation probe closeout

The A2DP upstream link probe now includes a parse-path implementation-slice check derived from `third/bluez/profiles/audio/avdtp.c`, covering single/start/continue/end packet parsing, transaction mismatch detection, and command/response routing.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-avdtp-parse-impl-probe.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-avdtp-parse-impl-probe.log`
- run: `FeatherCore/build/logs/run-a2dp-avdtp-parse-impl-probe.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-parse-impl-probe/run-results.json`

Required marker:

```text
bluez-daemon: a2dp upstream-avdtp-parse-impl-probe role=source|sink compile-unit=bluez/upstream_audio_link_probe.c source=third/bluez/profiles/audio/avdtp.c impl=single:1,start:1,continue:1,end:1,transaction-mismatch:1,route:1,total:6 boundary=upstream-avdtp-parse-impl-ported-not-yet-avdtp-c-object final-ok=1
```

This continues the gradual move from upstream headers toward implementation behavior. The full upstream `avdtp.c` object remains a future linkage step.

## 2026-06-14 A2DP upstream AVDTP signal implementation probe closeout

The A2DP upstream link probe now includes a signal-path implementation-slice check derived from `third/bluez/profiles/audio/avdtp.c`, covering command dispatch, accept response, reject response, and error mapping behavior.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-avdtp-signal-impl-probe.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-avdtp-signal-impl-probe.log`
- run: `FeatherCore/build/logs/run-a2dp-avdtp-signal-impl-probe.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-signal-impl-probe/run-results.json`

Required marker:

```text
bluez-daemon: a2dp upstream-avdtp-signal-impl-probe role=source|sink compile-unit=bluez/upstream_audio_link_probe.c source=third/bluez/profiles/audio/avdtp.c impl=command-dispatch:1,accept-response:1,reject-response:1,error-map:1,total:4 boundary=upstream-avdtp-signal-impl-ported-not-yet-avdtp-c-object final-ok=1
```

This continues the gradual move from upstream headers toward implementation behavior. The full upstream `avdtp.c` object remains a future linkage step.

## 2026-06-14 A2DP upstream AVDTP stream implementation probe closeout

The A2DP upstream link probe now includes a stream lifecycle implementation-slice check derived from `third/bluez/profiles/audio/avdtp.c`, covering state machine, timers, pending open, callbacks, and cleanup behavior.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-avdtp-stream-impl-probe.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-avdtp-stream-impl-probe.log`
- run: `FeatherCore/build/logs/run-a2dp-avdtp-stream-impl-probe.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-avdtp-stream-impl-probe/run-results.json`

Required marker:

```text
bluez-daemon: a2dp upstream-avdtp-stream-impl-probe role=source|sink compile-unit=bluez/upstream_audio_link_probe.c source=third/bluez/profiles/audio/avdtp.c impl=state-machine:1,timers:1,pending-open:1,callbacks:1,cleanup:1,total:5 boundary=upstream-avdtp-stream-impl-ported-not-yet-avdtp-c-object final-ok=1
```

This continues the gradual move from upstream headers toward implementation behavior. The full upstream `avdtp.c` object remains a future linkage step.

## 2026-06-14 A2DP upstream setup implementation probe closeout

The A2DP upstream link probe now includes a setup/SEP/stream implementation-slice check derived from `third/bluez/profiles/audio/a2dp.c`, covering setup refs, callback queue, SEP lock, stream attach, and error cleanup behavior.

Result: PASS

- case: `bluez-a2dp-upstream-convergence-closeout`
- build: `FeatherCore/build/logs/build-bt1-a2dp-setup-impl-probe.log`
- build: `FeatherCore/build/logs/build-bt2-a2dp-setup-impl-probe.log`
- run: `FeatherCore/build/logs/run-a2dp-setup-impl-probe.log`
- manifest: `FeatherCore/build/bt-hwsim-usecases-a2dp-setup-impl-probe/run-results.json`

Required marker:

```text
bluez-daemon: a2dp upstream-a2dp-setup-impl-probe role=source|sink compile-unit=bluez/upstream_audio_link_probe.c source=third/bluez/profiles/audio/a2dp.c impl=setup-refs:1,callbacks:1,sep-lock:1,stream-attach:1,error-cleanup:1,total:5 boundary=upstream-a2dp-setup-impl-ported-not-yet-a2dp-c-object final-ok=1
```

This continues the gradual move from upstream headers toward implementation behavior. The full upstream `a2dp.c` object remains a future linkage step.

## 2026-06-14 A2DP MediaTransport probe

The NuttX BlueZ shim now requires an A2DP MediaTransport implementation-slice marker during `bluez-a2dp-upstream-convergence-closeout`:

```text
bluez-daemon: a2dp upstream-media-transport-impl-probe ... impl=state:1,owner:1,acquire-release:1,properties:1,cleanup:1,total:5 ... final-ok=1
```

This is emitted by `upstream_audio_link_probe.c` after compiling against upstream BlueZ audio headers and modelling the `transport.c` lifecycle that matters for A2DP media handoff.

Validated evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-media-transport-impl-probe.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-transport-impl-probe.log`
- `FeatherCore/build/logs/run-a2dp-media-transport-impl-probe.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-transport-impl-probe/run-results.json`

Boundary: the marker intentionally says `ported-not-yet-transport-c-object`; this avoids claiming that unmodified upstream `transport.c` has already been linked as-is.

## 2026-06-14 A2DP MediaTransport D-Bus FSM

`upstream_media_transport_bridge` now models and validates a continuous `org.bluez.MediaTransport1` method FSM for A2DP closeout:

```text
upstream-transport-dbus-fsm=acquire:1,try-acquire:1,release:1,select-unselect:1,error:1,final-zero:1,total:6
```

This sits in `upstream-handler-bridge-surface-wrapper` and is part of the final-ok gate. It covers owner watch, pending request, fd reply, MTU handoff, suspend/release cleanup, Select/Unselect ownership, expected error paths, and final-zero state.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-transport-dbus-fsm.log`
- `FeatherCore/build/logs/build-bt2-a2dp-transport-dbus-fsm.log`
- `FeatherCore/build/logs/run-a2dp-transport-dbus-fsm.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-transport-dbus-fsm/run-results.json`

Boundary: this is still a bridge/compat execution of upstream `transport.c` semantics, not unmodified upstream object linkage.

## 2026-06-14 A2DP MediaEndpoint D-Bus FSM

`upstream_media_transport_bridge` now also models a continuous `org.bluez.MediaEndpoint1` lifecycle for A2DP closeout:

```text
upstream-media-endpoint-dbus-fsm=register:1,select:1,set:1,clear:1,unregister:1,error:1,final-zero:1,total:7
```

This marker is part of `upstream-handler-bridge-surface-wrapper` final-ok and covers endpoint registration, async select/set requests, transport attachment, clear/unregister cleanup, duplicate/missing-object errors, and final-zero state.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-media-endpoint-dbus-fsm.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-endpoint-dbus-fsm.log`
- `FeatherCore/build/logs/run-a2dp-media-endpoint-dbus-fsm.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-endpoint-dbus-fsm/run-results.json`

Boundary: this is a bridge/compat execution of upstream `media.c` endpoint semantics, not unmodified upstream object linkage.

## 2026-06-14 A2DP MediaApplication D-Bus FSM

`upstream_media_transport_bridge` now models a continuous `org.bluez.Media1.RegisterApplication` lifecycle for A2DP closeout:

```text
upstream-media-application-dbus-fsm=register:1,endpoints:1,players:1,unregister:1,disconnect:1,error:1,final-zero:1,total:7
```

This marker is part of `upstream-handler-bridge-surface-wrapper` final-ok and covers app owner registration, endpoint/player aggregation, transport/request attachment, unregister cleanup, owner disconnect cleanup, duplicate/missing-object errors, and final-zero state.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-media-application-dbus-fsm.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-application-dbus-fsm.log`
- `FeatherCore/build/logs/run-a2dp-media-application-dbus-fsm.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-application-dbus-fsm/run-results.json`

Boundary: this is a bridge/compat execution of upstream `media.c` application semantics, not unmodified upstream object linkage.

## 2026-06-14 A2DP AVRCP profile FSM

`upstream_media_transport_bridge` now models AVRCP profile lifecycle semantics as part of A2DP closeout:

```text
upstream-avrcp-profile-fsm=player-register:1,controller:1,target:1,metadata:1,volume:1,disconnect:1,final-zero:1,total:7
```

This marker is part of `upstream-handler-bridge-surface-wrapper` final-ok and covers local player registration, controller/browse ownership, target callbacks, metadata updates, volume events, disconnect cleanup, and final-zero state.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-avrcp-profile-fsm.log`
- `FeatherCore/build/logs/build-bt2-a2dp-avrcp-profile-fsm.log`
- `FeatherCore/build/logs/run-a2dp-avrcp-profile-fsm.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-avrcp-profile-fsm/run-results.json`

Boundary: this is bridge/compat execution of upstream `avrcp.c` semantics, not unmodified upstream object linkage.

## 2026-06-14 A2DP media stream FSM

`upstream_media_transport_bridge` now models A2DP media stream lifecycle semantics as part of A2DP closeout:

```text
upstream-a2dp-media-stream-fsm=open:1,start:1,rtp:1,payload:1,suspend:1,close:1,error:1,final-zero:1,total:8
```

This marker is part of `upstream-handler-bridge-surface-wrapper` final-ok and covers media fd/MTU ownership, RTP sequence/timestamp progression, media payload accounting, suspend/close cleanup, error handling, and final-zero state.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-media-stream-fsm.log`
- `FeatherCore/build/logs/build-bt2-a2dp-media-stream-fsm.log`
- `FeatherCore/build/logs/run-a2dp-media-stream-fsm.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-media-stream-fsm/run-results.json`

Boundary: this is bridge/compat execution of upstream-aligned A2DP media stream semantics, not unmodified upstream object linkage.

## 2026-06-14 A2DP codec policy FSM

`upstream_media_transport_bridge` now models A2DP codec policy lifecycle semantics as part of A2DP closeout:

```text
upstream-a2dp-codec-policy-fsm=capability:1,select:1,set:1,reconfigure:1,delay:1,error:1,final-zero:1,total:7
```

This marker is part of `upstream-handler-bridge-surface-wrapper` final-ok and covers SBC capability masks, selected codec policy, SetConfiguration attachment, reconfigure behavior, delay-reporting state, error handling, and final-zero cleanup.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-codec-policy-fsm.log`
- `FeatherCore/build/logs/build-bt2-a2dp-codec-policy-fsm.log`
- `FeatherCore/build/logs/run-a2dp-codec-policy-fsm.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-codec-policy-fsm/run-results.json`

Boundary: this is bridge/compat execution of upstream-aligned A2DP codec policy semantics, not unmodified upstream object linkage.

## 2026-06-14 A2DP lifecycle stress FSM

`upstream_media_transport_bridge` now models A2DP repeated lifecycle behavior as part of A2DP closeout:

```text
upstream-a2dp-lifecycle-stress-fsm=first-connect:1,cleanup:1,reconnect:1,duplicate-reject:1,media-resume:1,disconnect:1,final-zero:1,total:7
```

This marker is part of `upstream-handler-bridge-surface-wrapper` final-ok and covers first connect, cleanup, reconnect, duplicate active owner rejection, media resume after reconnect, disconnect cleanup, and final-zero state.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-lifecycle-stress-fsm.log`
- `FeatherCore/build/logs/build-bt2-a2dp-lifecycle-stress-fsm.log`
- `FeatherCore/build/logs/run-a2dp-lifecycle-stress-fsm.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-lifecycle-stress-fsm/run-results.json`

Boundary: this is bridge/compat execution of upstream-aligned A2DP lifecycle semantics, not unmodified upstream object linkage.

## 2026-06-14 A2DP upstream object-link readiness

`upstream_media_transport_bridge` now exposes an explicit readiness gate for replacing bridge/compat A2DP paths with real upstream object linkage:

```text
upstream-a2dp-object-link-readiness=sources:1,headers:1,glib-dbus:1,mainloop:1,core-objects:1,l2cap-media:1,symbol-ownership:1,replacement-boundary:1,total:8
```

This marker is part of `upstream-handler-bridge-surface-wrapper` final-ok and documents the concrete migration boundary for direct linking of upstream `a2dp.c`, `avdtp.c`, `media.c`, `transport.c`, and `avrcp.c`.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-object-link-readiness.log`
- `FeatherCore/build/logs/build-bt2-a2dp-object-link-readiness.log`
- `FeatherCore/build/logs/run-a2dp-object-link-readiness.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-object-link-readiness/run-results.json`

Boundary: this is a readiness gate, not direct upstream object linkage itself.

## 2026-06-14 A2DP negative/boundary FSM

`upstream_media_transport_bridge` now models A2DP negative/boundary semantics as part of A2DP closeout:

```text
upstream-a2dp-negative-boundary-fsm=bad-state:1,mtu:1,fd:1,codec-recover:1,duplicate-request:1,abort-cleanup:1,final-zero:1,total:7
```

This marker is part of `upstream-handler-bridge-surface-wrapper` final-ok and covers bad-state rejection, MTU boundary behavior, closed fd recovery, codec mismatch recovery, duplicate pending request rejection, abort cleanup, and final-zero state.

Evidence:

- `FeatherCore/build/logs/build-bt1-a2dp-negative-boundary-fsm.log`
- `FeatherCore/build/logs/build-bt2-a2dp-negative-boundary-fsm.log`
- `FeatherCore/build/logs/run-a2dp-negative-boundary-fsm.log`
- `FeatherCore/build/bt-hwsim-usecases-a2dp-negative-boundary-fsm/run-results.json`

Boundary: this completes broad staged A2DP semantic coverage; direct upstream object linkage remains separate work.

## 2026-06-15 Classic HID + BLE HOGP bounded native-io closeout

The HID/HOGP profile closeout gate now requires native Linux-BT/NuttX socket evidence instead of accepting profile logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-hid-hogp-bounded-closeout-final/run-results.json`

Validated case:

- `bluez-hid-hogp-profile-closeout`: PASS on `bt1`, `bt2`, `ble1`, and `ble2`.

What is covered now:

- Classic HID Control and Interrupt channels use L2CAP native-io evidence with open/connect/TX/RX/close accounting.
- HOGP uses ATT fixed channel CID `0x0004` for report-map, protocol-mode, and input CCC/notify path evidence.
- The validator requires native-io summaries, profile labels, lifecycle cleanup, and upstream BlueZ/Linux coverage markers.
- The sim script now maps BLE profile peers to the actual sim roles (`ble1` = role 3, `ble2` = role 4), avoiding mismatched ATT handles.

Current boundary:

- This is a bounded profile-closeout level, not a claim of unmodified upstream `bluetoothd` parity. Full-duplex response delivery still needs to replace the delegated RX/TX compatibility path before HID/HOGP reaches the same depth as the A2DP/LE Audio transport closeouts.

## 2026-06-15 HFP/HSP bounded native-io closeout

The HFP/HSP profile closeout gate now requires hwsim/native-io evidence instead of accepting staged profile logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-hfp-hsp-native-closeout/run-results.json`

Validated case:

- `bluez-hfp-hsp-profile-closeout`: PASS on `bt1` and `bt2`.

What is covered now:

- HFP validates service-level AT control for `BRSF`, `BAC/BCS`, and `CLCC` with native-io transaction evidence.
- HSP validates headset button and volume control for `CKPD` and `VGS` with native-io transaction evidence.
- The hwsim profile scripts run closeout commands as daemon-style background mainloops so the control path behaves closer to BlueZ profile ownership.
- The validator requires transaction summaries, profile labels, cleanup state, SCO lifecycle markers, and upstream source coverage.

Current boundary:

- RFCOMM is currently represented by `rfcomm-over-l2cap-bounded=1`; this proves the profile control path over hwsim but is not yet a native `BTPROTO_RFCOMM` socket/session implementation.
- SCO is still a lifecycle/accounting proof. A later closeout should add native SCO socket dataplane evidence before claiming full HFP/HSP parity.

## 2026-06-15 GATT bounded native-ATT closeout

The generic GATT profile closeout gate now requires hwsim/native ATT evidence instead of accepting staged profile logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-gatt-native-closeout-final/run-results.json`

Validated case:

- `bluez-gatt-profile-closeout`: PASS on `ble1` and `ble2`.

What is covered now:

- ATT fixed channel CID `0x0004` carries primary service discovery, characteristic read, characteristic write, and CCC/notification transactions.
- The validator requires `gatt-att` native-io summaries, transaction labels, discovery/read/write/notify final markers, cleanup state, and upstream source coverage.
- The hwsim scripts use the correct BLE role peer mapping (`ble1` = role 3, `ble2` = role 4) and run GATT closeout in daemon-style background mainloops.

Current boundary:

- This establishes the reusable ATT/GATT baseline for HOGP, MIDI, ASHA, and Mesh, but it is still a bounded `bluezdaemon` compatibility harness rather than full unmodified upstream `bluetoothd` GattManager1 object ownership.
- Full-duplex ATT response ownership and reduced GATT DB/client/server glue remain follow-up convergence work.

## 2026-06-15 BLE MIDI bounded native-ATT closeout

The BLE MIDI profile closeout gate now requires hwsim/native ATT evidence instead of accepting staged profile logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-midi-native-closeout/run-results.json`

Validated case:

- `bluez-midi-profile-closeout`: PASS on `ble1` and `ble2`.

What is covered now:

- ATT fixed channel CID `0x0004` carries BLE MIDI service discovery, MIDI characteristic discovery, CCC enable, and MIDI note/control payload transactions.
- The validator requires `midi-att` native-io summaries, transaction labels, timestamp/payload/error policy markers, cleanup state, upstream source coverage, and `native-io-final=1`.
- The hwsim scripts use the correct BLE role peer mapping (`ble1` = role 3, `ble2` = role 4) and run MIDI closeout in daemon-style background mainloops.

Current boundary:

- This establishes the BLE MIDI profile at the same bounded native ATT closeout level as generic GATT/HOGP, but it is still a `bluezdaemon` compatibility harness rather than full unmodified upstream `bluetoothd` MIDI profile object ownership.
- Full-duplex ATT response ownership and reduced MIDI/GATT compatibility glue remain follow-up convergence work.

## 2026-06-15 ASHA bounded native-ATT closeout

The ASHA/Hearing Aid profile closeout gate now requires hwsim/native ATT evidence instead of accepting staged profile logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-asha-native-closeout/run-results.json`

Validated case:

- `bluez-asha-profile-closeout`: PASS on `ble1` and `ble2`.

What is covered now:

- ATT fixed channel CID `0x0004` carries ASHA service discovery, read-only properties, audio control start, and status/CCC notification transactions.
- The validator requires `asha-att` native-io summaries, transaction labels, stream/control/battery/error policy markers, cleanup state, upstream source coverage, and `native-io-final=1`.
- The hwsim scripts use the correct BLE role peer mapping (`ble1` = role 3, `ble2` = role 4) and run ASHA closeout in daemon-style background mainloops.

Current boundary:

- This establishes ASHA at the same bounded native ATT closeout level as generic GATT/HOGP/MIDI, but it is still a `bluezdaemon` compatibility harness rather than full unmodified upstream `bluetoothd` audio/GATT object ownership.
- Full-duplex ATT response ownership and reduced ASHA/audio transport compatibility glue remain follow-up convergence work.

## 2026-06-15 Mesh bounded native GATT Proxy closeout

The Bluetooth Mesh profile closeout gate now requires hwsim/native ATT evidence for the GATT Proxy bearer instead of accepting staged profile logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-mesh-native-closeout/run-results.json`

Validated case:

- `bluez-mesh-profile-closeout`: PASS on `ble1` and `ble2`.

What is covered now:

- ATT fixed channel CID `0x0004` carries provisioning service discovery, proxy characteristic discovery, proxy CCC enable, and Mesh Network PDU over GATT Proxy transactions.
- The validator requires `mesh-gatt-proxy` native-io summaries, transaction labels, PB-ADV/config/model/transport/proxy/beacon/error markers, upstream source coverage, and `native-io-final=1`.
- The hwsim scripts use the correct BLE role peer mapping (`ble1` = role 3, `ble2` = role 4) and run Mesh closeout in daemon-style background mainloops.

Current boundary:

- This establishes Mesh at a bounded native GATT Proxy closeout level, but PB-ADV advertising bearer and full `bluetooth-meshd` D-Bus/node/model ownership still run through the `bluezdaemon` compatibility harness.
- Stronger native advertising bearer evidence and reduced mesh daemon compatibility glue remain follow-up convergence work.

## 2026-06-15 BIP and Print bounded native RFCOMM closeout

The BIP and Print profile closeout gates now require hwsim/native RFCOMM-over-L2CAP evidence instead of accepting staged profile logs alone.

Validated artifacts:

- `FeatherCore/build/bt-hwsim-usecases-bip-native-closeout-final/run-results.json`
- `FeatherCore/build/bt-hwsim-usecases-print-native-closeout/run-results.json`

Validated cases:

- `bluez-obex-bip-profile-closeout`: PASS on `bt1` and `bt2`.
- `bluez-print-profile-closeout`: PASS on `bt1` and `bt2`.

What is covered now:

- BIP carries OBEX connect, capabilities, and image PUT transactions over bounded RFCOMM/L2CAP native-io.
- Print carries SPP open and print job payload transactions over bounded RFCOMM/L2CAP native-io.
- The validators require native-io summaries, transaction labels, RFCOMM bounded markers, transfer/job final markers, cleanup state, upstream source coverage, and `native-io-final=1`.

Current boundary:

- This establishes BIP/Print at the same bounded RFCOMM closeout level as HFP/HSP control paths, but not at native `BTPROTO_RFCOMM` parity.
- Full OBEX/CUPS parity still needs unmodified daemon object ownership, real RFCOMM socket/session ownership, and deeper transfer lifecycle coverage.

## 2026-06-15 PBAP/OPP bounded native RFCOMM closeout

The PBAP/OPP closeout gate now requires hwsim/native RFCOMM-over-L2CAP evidence instead of accepting staged OBEX logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-pbap-opp-native-closeout/run-results.json`

Validated case:

- `bluez-obex-pbap-opp-profile-closeout`: PASS on `bt1` and `bt2`.

What is covered now:

- PBAP carries OBEX connect, phonebook selection, and phonebook pull transactions over bounded RFCOMM/L2CAP native-io.
- OPP carries OBEX connect, object PUT, and capability query transactions over bounded RFCOMM/L2CAP native-io.
- The validator requires native-io summaries, transaction labels, RFCOMM bounded markers, transfer/object final markers, cleanup state, upstream source coverage, and `native-io-final=1`.

Current boundary:

- This establishes PBAP/OPP at bounded RFCOMM closeout level, but not at native `BTPROTO_RFCOMM` parity or unmodified `obexd` ownership parity.
- Full OBEX parity still needs real RFCOMM socket/session ownership and deeper transfer queue lifecycle coverage.

## 2026-06-15 MAP/MNS bounded native RFCOMM closeout

The MAP/MNS closeout gate now requires hwsim/native RFCOMM-over-L2CAP evidence instead of accepting staged OBEX logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-map-mns-native-closeout/run-results.json`

Validated case:

- `bluez-obex-map-mns-profile-closeout`: PASS on `bt1` and `bt2`.

What is covered now:

- MAP carries OBEX connect, set-folder, and message-listing transactions over bounded RFCOMM/L2CAP native-io.
- MNS carries OBEX connect, new-message event, and delivery-success event transactions over bounded RFCOMM/L2CAP native-io.
- The validator requires native-io summaries, transaction labels, RFCOMM bounded markers, message/event final markers, cleanup state, upstream source coverage, and `native-io-final=1`.

Current boundary:

- This establishes MAP/MNS at bounded RFCOMM closeout level, but not at native `BTPROTO_RFCOMM` parity or unmodified `obexd` ownership parity.
- Full OBEX parity still needs real RFCOMM socket/session ownership and deeper MAP/MNS transfer/event queue lifecycle coverage.

## 2026-06-15 FTP/Sync bounded native RFCOMM closeout

The FTP/Sync closeout gate now requires hwsim/native RFCOMM-over-L2CAP evidence instead of accepting staged OBEX logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-ftp-sync-native-closeout/run-results.json`

Validated case:

- `bluez-obex-ftp-sync-profile-closeout`: PASS on `bt1` and `bt2`.

What is covered now:

- FTP carries OBEX connect, set-folder, folder-listing, and get-file transactions over bounded RFCOMM/L2CAP native-io.
- Sync carries OBEX connect, phonebook sync, calendar sync, and notes sync transactions over bounded RFCOMM/L2CAP native-io.
- The validator requires native-io summaries, transaction labels, RFCOMM bounded markers, filesystem/sync final markers, cleanup state, upstream source coverage, and `native-io-final=1`.

Current boundary:

- This establishes FTP/Sync at bounded RFCOMM closeout level, but not at native `BTPROTO_RFCOMM` parity or unmodified `obexd` ownership parity.
- Full OBEX parity still needs real RFCOMM socket/session ownership and deeper filesystem/sync transfer lifecycle coverage.

## 2026-06-15 iAP bounded native RFCOMM closeout

The iAP closeout gate now requires hwsim/native RFCOMM-over-L2CAP evidence instead of accepting staged profile logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-iap-native-closeout/run-results.json`

Validated case:

- `bluez-iap-profile-closeout`: PASS on `bt1` and `bt2`.

What is covered now:

- iAP carries identify, external-accessory session open, and control payload transactions over bounded RFCOMM/L2CAP native-io.
- The validator requires native-io summaries, transaction labels, RFCOMM bounded markers, identify/session/payload final markers, cleanup state, upstream source coverage, and `native-io-final=1`.

Current boundary:

- This establishes iAP at bounded RFCOMM closeout level, but not at native `BTPROTO_RFCOMM` parity or unmodified upstream profile/session ownership parity.
- Full iAP parity still needs real RFCOMM socket/session ownership and deeper iAP2 state-machine coverage.

## 2026-06-15 Ranging bounded native ATT closeout

The BLE Ranging/RAP closeout gate now requires hwsim/native ATT evidence instead of accepting staged RAP/HCI logs alone.

Validated artifact:

- `FeatherCore/build/bt-hwsim-usecases-ranging-native-closeout/run-results.json`

Validated case:

- `bluez-ranging-profile-closeout`: PASS on `ble1` and `ble2`.

What is covered now:

- ATT fixed channel CID `0x0004` carries capability read, security enable, procedure configuration, and result notification transactions.
- The validator requires `ranging-rap-att` native-io summaries, transaction labels, ATT fixed-CID markers, result/procedure final markers, cleanup state, upstream source coverage, and `native-io-final=1`.
- The hwsim scripts use the correct BLE role peer mapping (`ble1` = role 3, `ble2` = role 4) and run Ranging closeout in daemon-style background mainloops.

Current boundary:

- This establishes Ranging at bounded native ATT/RAP closeout level, but not at full controller-side Channel Sounding or unmodified upstream BlueZ RAP/HCI object ownership parity.
- Full Ranging parity still needs richer HCI command/event sequencing and reduced RAP compatibility glue.

## 2026-06-15 unified BT/BLE current functional closeout

`bluez-current-functional-closeout` has been upgraded from an A2DP + NET
summary into the current all-feature hwsim umbrella.  The generated four-role
run now covers BT/BLE basic control, A2DP, LE Audio, BT Network/BNEP, BLE
Network/IPSP/6LoWPAN, HID/HOGP, HFP/HSP, GATT, OBEX PBAP/OPP/MAP/MNS/FTP/Sync,
Mesh, ASHA, BIP, Print, iAP, MIDI, and Ranging.

The validator now requires each included feature family to emit its strongest
current evidence marker: BlueZ/Linux upstream coverage maps, native L2CAP /
RFCOMM / ATT / ISO / BNEP / 6LoWPAN data or control path evidence, role-specific
source/sink/client/server markers, and final cleanup state.  This puts the
remaining profile gates under the same current hwsim closeout bar used for A2DP
and LE Audio instead of allowing scattered profile-only probes to pass silently.

Current boundary: this is still the current hwsim functional parity bar, not a
claim that every feature is backed by unmodified upstream `bluetoothd`, `obexd`,
or kernel object ownership.  The next convergence work should reduce the
remaining compatibility glue while preserving this umbrella as the regression
bar.

## 2026-06-15 unified closeout LE Audio daemon profile strengthening

The all-feature `bluez-current-functional-closeout` gate now invokes the
existing LE Audio `le-daemon integrated-profile-flow` path in addition to the
individual LC3/ISO/GATT probes.  The validator requires the integrated
`bluetoothd-profile` completion marker for source and sink roles, BAP-owned LC3
codec commands, LC3 encode/decode evidence, and GATT upstream cleanup markers.

This moves the umbrella gate closer to the LE Audio closeout standard used by
the dedicated LE Audio cases: daemon/profile ownership evidence is now required
inside the all-feature run, not only in standalone LE Audio runs.

## 2026-06-15 A2DP upstream TryAcquire handler convergence

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-try-acquire-complete=owner:1,fd:1,reply:1,state:1,total:4`.
The probe constructs a real upstream `audio/transport.c` A2DP transport,
transitions it to the BlueZ `PENDING` playing-without-owner state, invokes the
real static `TryAcquire` handler, and completes the prepared upstream AVDTP
resume path.

This closes the previous gap where `TryAcquire` was only dispatch-bound. The
closeout now proves the Linux/BlueZ semantics for owner creation, fd/MTU reply,
pending request cleanup, and transition to `ACTIVE` without replacing the
handler body with compatibility logic.

## 2026-06-15 A2DP upstream Select/Unselect handler guard

`bluez-a2dp-upstream-convergence-closeout` now also requires
`upstream-select-unselect-guard=owner:1,state:1,select:1,unselect:1,total:4`.
The probe constructs a real upstream `audio/transport.c` A2DP media transport
and invokes the static `Select` and `Unselect` handlers with the Linux/BlueZ
no-owner, non-requesting guard conditions.

For ordinary A2DP endpoints these handlers do not transition playing state the
same way broadcast-audio endpoints do, so this marker is deliberately scoped to
handler ownership/state guard semantics. It is not claimed as full D-Bus object
reply ownership; that remains a later convergence item once the DBus shim is
replaced by real object/message ownership.

## 2026-06-15 DBus shim reply/error ownership foundation

The upstream object shim no longer collapses every D-Bus method return and
BlueZ error helper to `NULL`. `dbus_message_new_method_return()` now returns a
shim message tagged as `DBUS_MESSAGE_KIND_METHOD_RETURN`, and `btd_error_*()`
helpers now return tagged `DBUS_MESSAGE_KIND_ERROR` messages with the matching
`org.bluez.Error.*` name.

A2DP `Select` and `Unselect` convergence now consumes this foundation: the
upstream `audio/transport.c` handlers must return method-return messages tied
to the original request object while preserving the ordinary A2DP no-owner,
idle-state guard semantics.

This is still not a complete replacement for real daemon-level D-Bus object
ownership, connection watches, or bus name lifecycle. It does, however, remove
a weak shim behavior that previously made successful and error-returning
upstream handlers indistinguishable in bounded hwsim probes.

## 2026-06-15 A2DP upstream error reply convergence

`upstream-error-closeout=duplicate-acquire:1,unauthorized-release:1,
duplicate-release:1,owner-disconnect:1,total:4` is now stronger than a pure
state-side-effect check. Duplicate `Acquire` and wrong-sender `Release` must
return tagged `org.bluez.Error.NotAuthorized` messages from the real upstream
`audio/transport.c` handlers, and duplicate `Release` while a release request
is pending must return tagged `org.bluez.Error.InProgress`.

The shared GDBus shim also now routes `g_dbus_create_reply()` and
`g_dbus_create_error()` through the same tagged D-Bus shim message model used
by `dbus_message_new_method_return()` and `btd_error_*()`. This gives later
A2DP, LE Audio, Network, and profile convergence work a common way to separate
successful method returns from BlueZ error replies.

## 2026-06-15 A2DP Release completion reply ownership

`upstream-release-cleanup` now requires
`owner:1,pending:1,fd-held:1,state:1,reply:1,total:5`. The additional
`reply:1` proves that the asynchronous upstream `Release` completion path calls
`media_request_reply()`, creates a tagged method-return D-Bus shim message, and
sends it through `g_dbus_send_message()` for the original `Release` request.

The GDBus shim now records the last sent message and sent-message count within
the current translation unit. This keeps the probe bounded, but makes the
A2DP closeout sensitive to a real upstream completion reply instead of only
observing that pending ownership was cleared.

## 2026-06-15 A2DP Acquire/TryAcquire fd reply ownership

`upstream-acquire-complete=fd:1,mtu:1,reply:1,state:1,total:4` and
`upstream-try-acquire-complete=owner:1,fd:1,reply:1,state:1,total:4` now bind
their `reply:1` evidence to the original `Acquire` or `TryAcquire` request.
The transport probe records the last `g_dbus_send_reply()` request message and
first argument type, and the closeout only passes when the reply is a
`DBUS_TYPE_UNIX_FD` response for the exact upstream request object.

This keeps the existing marker names stable while tightening their meaning:
fd/MTU completion is no longer proven only by a reply counter increment.

## 2026-06-15 A2DP MediaTransport State property-change ownership

A2DP `state:1` evidence in `upstream-acquire-complete`,
`upstream-try-acquire-complete`, and `upstream-release-cleanup` now requires
both the internal upstream transport state and the public BlueZ D-Bus
`MediaTransport1.State` property-change notification.

The GDBus shim records the last `g_dbus_emit_property_changed()` path,
interface, property name, and count. The A2DP probes reset this record around
upstream resume/suspend completion and require exactly one `State` notification
for the transport object. This follows BlueZ's public state mapping: Acquire
completion publishes active state, and Release completion publishes idle state.

## 2026-06-15 A2DP request message ref/unref ownership

The DBus message shim now tracks `dbus_message_ref()` and
`dbus_message_unref()` counts. A2DP `Acquire`, `TryAcquire`, and asynchronous
`Release` completion probes require the upstream request message to be ref'd
when `media_request_create()` owns it and unref'd back to zero when
`media_owner_remove()` releases the pending request.

This tightens the existing `reply:1` and `pending:1` evidence: completion now
proves not only that the request pointer was observed, but that the upstream
BlueZ request ownership lifecycle performed the expected ref/unref handoff.

## 2026-06-15 A2DP owner disconnect-watch lifecycle ownership

The GDBus shim now records `g_dbus_add_disconnect_watch()` and
`g_dbus_remove_watch()` ownership data: watch id, watched bus name, callback,
user data, add count, and remove count. A2DP `Acquire` and `TryAcquire`
completion probes require the upstream `media_transport_set_owner()` path to
register a disconnect watch for the owner sender and bind it to
`media_owner_exit()` with the upstream owner object as callback data.

Normal asynchronous `Release` completion now also requires the owner watch to
be removed by `media_transport_remove_owner()`. The owner-disconnect closeout
now invokes the callback recorded by the shim instead of directly calling the
static function, so the scenario represents the BlueZ bus-disconnect watch
path more closely.

## 2026-06-15 A2DP MediaTransport property getter value ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-property-getters=uuid:1,codec:1,configuration:1,state:1,total:4`.
This is separate from the existing property handler-count evidence: the new
marker proves that real upstream `audio/transport.c` `MediaTransport1` getters
write observable D-Bus iterator values from a bounded upstream transport
object.

The DBus iterator shim records appended scalar and fixed-array values. The
A2DP getter probe verifies the upstream UUID, codec, transport configuration,
and public state string after the transport is moved to the BlueZ `pending`
playing-without-owner state.

## 2026-06-15 A2DP MediaTransport remaining property getter values

`upstream-property-getters` now covers seven `MediaTransport1` values:
`uuid:1,codec:1,configuration:1,state:1,delay:1,volume:1,endpoint:1,total:7`.
The bounded upstream transport probe now also validates `Delay`, `Volume`, and
`Endpoint` getter output from the real upstream `audio/transport.c` getter
functions.

`Endpoint` is validated against the remote endpoint path passed into upstream
`media_transport_create()`. `Delay` and `Volume` are validated against the
upstream A2DP transport data before the getters write their D-Bus iterator
values.

## 2026-06-15 A2DP MediaTransport property setter pending-result ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-property-setters=delay:1,volume:1,unauthorized:1,invalid:1,total:4`.
The GDBus pending-property shim records success and error completion for real
upstream `audio/transport.c` `MediaTransport1` setters.

The bounded setter probe validates successful `Delay` and `Volume` setter
completion, wrong-sender `Delay` rejection via `org.bluez.Error.NotAuthorized`,
and invalid-argument `Volume` rejection via `org.bluez.Error.InvalidArguments`.
This marker is scoped to D-Bus setter handler semantics; depending on endpoint
role and compile-time AVRCP support, the underlying device/codec volume update
may still be a no-op in this bounded object.

## 2026-06-15 A2DP MediaTransport property existence predicates

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-property-exists=delay-absent:1,volume:1,endpoint:1,total:3`. This
marker invokes the real upstream `audio/transport.c` `MediaTransport1`
existence predicates for the bounded A2DP source transport.

`delay-absent:1` is intentional: for this bounded source transport no prepared
stream delay-reporting capability is attached, so upstream `DelayReporting`
correctly evaluates as absent. `Volume` is made present by setting the upstream
A2DP transport volume, and `Endpoint` is present because the transport was
created with a remote endpoint object path.

## 2026-06-15 A2DP MediaTransport D-Bus interface export ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-transport-export=path:1,interface:1,methods:1,properties:1,total:4`.
The transport object probe records the upstream `g_dbus_register_interface()`
call made by `media_transport_create()` and verifies that the exported path,
`org.bluez.MediaTransport1` interface name, method table, property table,
transport user data, and destroy callback all match the upstream transport
object.

This is still a bounded shim export check, not a full daemon bus-name lifecycle
or cross-process D-Bus ownership proof. It does close the previous gap where
transport creation was proven as a C object but not as an exported BlueZ D-Bus
interface.

## 2026-06-15 A2DP MediaTransport D-Bus interface unexport ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-transport-unexport=path:1,interface:1,destroy:1,total:3`. The
transport object probe records the upstream `g_dbus_unregister_interface()`
call made during `media_transport_destroy()` and verifies that it unregisters
the same transport path and `org.bluez.MediaTransport1` interface that were
exported by `media_transport_create()`.

The marker also requires the registered destroy callback to run, proving that
unexport is coupled to the upstream transport object's cleanup path rather than
being only a counter increment.

## 2026-06-15 A2DP MediaTransport object path allocation ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-transport-path=first:1,second:1,unique:1,total:3`. The bounded probe
creates two upstream A2DP media transports for the same remote endpoint and
verifies that each exported object path is derived from the remote endpoint and
that the second transport receives a distinct `fdN` path.

This covers the upstream `media_transport_create()` path allocation and
collision-avoidance behavior instead of assuming a fixed object path in the
compat layer.

## 2026-06-15 A2DP MediaTransport registry lifecycle ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-transport-registry=append:1,lookup:1,remove:1,total:3`. The bounded
probe creates an upstream A2DP media transport, verifies that the upstream
`find_transport_by_path()` registry lookup returns the same transport object,
and verifies that `media_transport_destroy()` removes the object from the
upstream transport registry.

This covers the internal BlueZ `transport.c` object registry lifecycle that
backs object-path uniqueness and later method/property dispatch.

## 2026-06-15 A2DP MediaTransport Delay/Volume property-change notifications

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-property-changes=delay:1,volume:1,total:2`. The bounded probe invokes
real upstream property update paths and validates that `g_dbus_emit_property_changed()`
emits `MediaTransport1.Delay` and `MediaTransport1.Volume` notifications for
the transport object path.

`Delay` is exercised through `media_transport_update_delay()`. `Volume` is
exercised through `media_transport_set_a2dp_volume()`, which locates the
transport through the upstream registry and emits the public property change
when the A2DP volume changes.

## 2026-06-15 A2DP MediaEndpoint RegisterEndpoint lifecycle ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-register=adapter:1,watch:1,sep:1,reply:1,total:4`.
This validates the real upstream `audio/media.c` `RegisterEndpoint` behavior:
BlueZ stores the external application's endpoint path in the adapter endpoint
registry, adds a sender disconnect watch bound to `media_endpoint_exit()`,
creates the A2DP SEP, and returns a method reply for the original request.

This deliberately does not claim that BlueZ exports `MediaEndpoint1` itself.
In upstream BlueZ, `RegisterEndpoint` registers an external application's
endpoint object path and owns the registry/watch/SEP state around that external
object.

## 2026-06-15 A2DP MediaEndpoint UnregisterEndpoint lifecycle ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-unregister=lookup:1,watch:1,sep:1,remove:1,reply:1,total:5`.
The bounded probe first creates an endpoint through the real upstream
`audio/media.c` `RegisterEndpoint` handler, then calls the real upstream
`UnregisterEndpoint` handler with the same sender and object path.

This validates sender/path endpoint lookup, existing disconnect-watch
ownership, existing A2DP SEP ownership, registry removal through
`media_endpoint_remove()`, watch removal, and method-return reply ownership.
Together with the register marker, MediaEndpoint registration and cleanup are
now exercised as a paired upstream handler lifecycle rather than as
register-only side effects.

## 2026-06-15 A2DP MediaEndpoint RegisterEndpoint error policy

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-register-errors=duplicate:1,cleanup:1,total:2`.
The bounded probe invokes the real upstream `RegisterEndpoint` handler twice
with the same sender/path. The second call must return a tagged
`org.bluez.Error.AlreadyExists` reply and must not append a duplicate endpoint
to the adapter registry. A final valid unregister then proves the endpoint can
still be cleaned up after the negative path.

This closes the creation-side counterpart to the unregister error policy: A2DP
MediaEndpoint registration now validates both successful ownership and duplicate
registration rejection through real upstream BlueZ handler code.

## 2026-06-15 A2DP MediaEndpoint UnregisterEndpoint error policy

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-unregister-errors=wrong-sender:1,missing-path:1,cleanup:1,total:3`.
The bounded probe registers an endpoint through the real upstream handler, then
invokes the real upstream `UnregisterEndpoint` handler with a wrong sender and
with a missing path. Both negative paths must return tagged
`org.bluez.Error.DoesNotExist` replies while preserving the registered
endpoint until the final valid unregister cleanup succeeds.

This tightens the D-Bus method policy around MediaEndpoint ownership: endpoint
cleanup now proves both the successful Linux/BlueZ lifecycle and the sender/path
authorization semantics used by upstream BlueZ.

## 2026-06-15 A2DP MediaEndpoint SelectConfiguration request ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-select-request=owner:1,message:1,pending:1,cancel:1,total:4`.
The bounded probe registers an endpoint through the real upstream
`RegisterEndpoint` handler, then calls the real upstream `select_configuration()`
body from `audio/media.c`.

The marker validates endpoint request ownership, the generated
`org.bluez.MediaEndpoint1.SelectConfiguration` D-Bus method-call message,
single pending request insertion, and upstream cancel/destroy cleanup when the
endpoint is unregistered. This replaces another bridge-only endpoint request
slice with executable upstream BlueZ handler behavior.

Boundary: `SetConfiguration` still has deeper `a2dp_setup` and
`media_transport_create()` object dependencies. It remains covered by the
existing bridge gate until the setup/device/transport ownership can be moved
into this direct upstream-object probe safely.

## 2026-06-15 A2DP MediaEndpoint SelectConfiguration reply ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-select-reply=callback:1,remove:1,unref:1,cleanup:1,total:4`.
The DBus pending-call shim can now carry a method-return reply, allowing the
bounded probe to call the real upstream `endpoint_reply()` body after
`select_configuration()` creates a pending request.

The marker validates that the selected configuration reaches the endpoint
callback, the request is removed from the endpoint queue, the reply message is
unref'd by upstream code, and endpoint unregister still performs final cleanup.
This moves `SelectConfiguration` from request creation only to request
completion ownership.

## 2026-06-15 A2DP MediaEndpoint SelectConfiguration error ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-select-error=callback:1,remove:1,unref:1,cleanup:1,total:4`.
The DBus error-message shim now lets the real upstream `endpoint_reply()` body
take its error path for a pending `SelectConfiguration` request.

The marker validates that a tagged `org.bluez.Error.Failed` reply reaches the
endpoint callback as a failed request, removes the pending request, unrefs the
error reply, and leaves endpoint unregister cleanup intact. This covers the
negative completion lifecycle shared by MediaEndpoint request handling while
`SetConfiguration` direct object ownership remains the next deeper A2DP target.

## 2026-06-15 A2DP MediaEndpoint SetConfiguration direct request ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-set-request=owner:1,message:1,transport:1,reply:1,cleanup:1,total:5`.
The AVDTP object probe now provides a bounded synthetic session carrying a
device pointer, and the A2DP object probe provides a bounded synthetic
`a2dp_setup` carrying that session plus the remote endpoint path. This keeps
the private upstream `struct avdtp` and `struct a2dp_setup` layouts inside
their owning compile units instead of faking them in `media.c`.

The MediaEndpoint probe uses that setup to call the real upstream
`set_configuration()` body from `audio/media.c`. The marker validates endpoint
request ownership, the generated `org.bluez.MediaEndpoint1.SetConfiguration`
D-Bus method-call message, MediaTransport creation/attachment, successful
reply completion through `endpoint_reply()`, and final endpoint unregister
cleanup.

Boundary: this is still a bounded direct-object probe, not yet the full live
AVDTP negotiation path. The remaining A2DP work is to feed this direct
`SetConfiguration` path from the actual AVDTP session/setup generated by the
signaling transaction rather than a synthetic setup helper.

## 2026-06-15 A2DP MediaEndpoint SetConfiguration prepared setup ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-set-prepared=setup:1,device:1,remote:1,total:3`.
The direct `SetConfiguration` probe no longer builds a synthetic
`struct a2dp_setup` in the MediaEndpoint path. Instead, the A2DP object probe
uses the upstream `prepare_resume_setup()` / `a2dp_setup_get()` path to obtain
a real prepared setup owned by `audio/a2dp.c`, then attaches the bounded remote
endpoint path inside the same compile unit.

The marker validates that `set_configuration()` sees a real upstream setup,
the setup resolves back to the expected device through the AVDTP session, and
the remote endpoint path is available through `a2dp_setup_remote_path()`.
Remaining boundary: the remote SEP/path is still bounded injection; the next
step is to make it come from the actual AVDTP signaling transaction.

## 2026-06-15 A2DP MediaEndpoint SetConfiguration registered remote SEP

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-set-remote=path:1,setup:1,request:1,reply:1,cleanup:1,total:5`.
The A2DP object probe now creates a remote SEP through upstream
`avdtp_register_remote_sep()` and then runs upstream `a2dp.c::register_remote_sep()`
to allocate the BlueZ MediaEndpoint path from `device_get_path()` and the
remote SEID.

The marker validates that the prepared setup uses this registered remote path,
that the setup still resolves to the expected device, and that the real
`set_configuration()` request/reply path succeeds with that remote SEP
ownership. Remaining boundary: the remote SEP is registered by the bounded
probe, not yet produced by live AVDTP Discover/GetCapabilities signaling.

## 2026-06-15 A2DP registered remote SEP lookup ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-remote-lookup=registered:1,session:1,channel:1,path:1,cleanup:1,total:5`.
The registered remote SEP helper now uses a real
`struct avdtp_media_codec_capability` so upstream `avdtp_find_remote_sep()` can
match it against the local A2DP SEP codec.

The marker validates that the remote SEP is present in the AVDTP session
registry, that `avdtp_find_remote_sep()` finds it, that `a2dp.c::find_remote_sep()`
maps the same AVDTP remote SEP to the A2DP channel remote SEP, that the
generated path is available, and that endpoint cleanup still succeeds.
Remaining boundary: the remote SEP is still seeded by the bounded probe rather
than by live Discover/GetCapabilities packet parsing.

## 2026-06-15 A2DP raw AVDTP capability parser remote SEP ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-remote-caps=parsed:1,registered:1,lookup:1,set:1,cleanup:1,total:5`.
The AVDTP object probe now accepts raw service-capability TLV bytes and feeds
them through upstream `audio/avdtp.c::caps_to_list()` before registering the
remote SEP with upstream `avdtp_register_remote_sep()`.

The A2DP object probe uses that parsed remote SEP to run upstream
`a2dp.c::register_remote_sep()`, making the BlueZ remote endpoint path come
from the parsed AVDTP capability data instead of a manually constructed GSList.
The MediaEndpoint probe then drives real upstream `set_configuration()` with
that parsed remote SEP path and completes the request through upstream
`endpoint_reply()`.

The marker validates raw capability parsing, AVDTP remote SEP registration,
AVDTP session plus A2DP channel lookup, SetConfiguration request/reply
ownership, and final endpoint cleanup. Remaining boundary: this still injects
bounded raw capability bytes into the parser; the next convergence target is
to source those bytes from the live AVDTP Discover/GetCapabilities signaling
transaction over the hwsim controller path.

## 2026-06-15 A2DP GetCapabilities response remote SEP ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-signaling-caps=response:1,registered:1,lookup:1,set:1,cleanup:1,total:5`.
The AVDTP object probe now creates the minimal upstream pending-request context
for a remote SEP and calls `audio/avdtp.c::avdtp_get_capabilities_resp()` with
bounded GetCapabilities response bytes.

This moves the convergence evidence from direct capability-list parsing to the
actual upstream response handler used by AVDTP discovery. The response handler
populates the remote SEP capabilities, the A2DP probe maps that SEP through
`a2dp.c::register_remote_sep()`, and the MediaEndpoint probe drives
`media.c::set_configuration()` plus `endpoint_reply()` with the resulting
remote endpoint path.

The marker validates GetCapabilities response handling, registered remote SEP
ownership, AVDTP session and A2DP channel lookup, SetConfiguration completion,
and endpoint cleanup. Remaining boundary: the response bytes are still injected
by the bounded probe rather than arriving through `avdtp_parse_data()` from a
real signaling socket over hwsim.

## 2026-06-15 A2DP AVDTP response dispatcher remote SEP ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-dispatch-caps=response:1,registered:1,lookup:1,set:1,cleanup:1,total:5`.
The AVDTP object probe now drives the upstream response dispatcher
`audio/avdtp.c::avdtp_parse_resp()` for a GetCapabilities accept response
instead of calling `avdtp_get_capabilities_resp()` directly.

The helper creates a minimal upstream `struct pending_req` carrying the same
`struct seid_req` shape used by the real request path, so
`avdtp_get_capabilities_resp()` resolves the remote SEP through the session
registry by SEID. A2DP then maps that remote SEP through
`a2dp.c::register_remote_sep()`, and MediaEndpoint drives
`media.c::set_configuration()` plus `endpoint_reply()` with the resulting path.

The marker validates AVDTP response dispatch, remote SEP registration,
AVDTP-session/A2DP-channel lookup, SetConfiguration completion, and endpoint
cleanup. Remaining boundary: the bytes still enter at `avdtp_parse_resp()`;
the next convergence step is to enter via `avdtp_parse_data()` from a simulated
single-packet ACCEPT frame and then through the session callback/socket path.

## 2026-06-15 A2DP AVDTP single-packet parser remote SEP ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-packet-caps=frame:1,registered:1,lookup:1,set:1,cleanup:1,total:5`.
The AVDTP object probe now builds a complete single-packet ACCEPT frame with
an upstream `struct avdtp_single_header` and GetCapabilities response payload,
then enters through `audio/avdtp.c::avdtp_parse_data()`.

The parsed frame populates the upstream response buffer (`session->in_resp`),
then flows through `avdtp_parse_resp()` and `avdtp_get_capabilities_resp()`.
A2DP maps the resulting remote SEP through `a2dp.c::register_remote_sep()`,
and MediaEndpoint drives `media.c::set_configuration()` plus
`endpoint_reply()` with the generated remote path.

The marker validates AVDTP single-frame parsing, remote SEP registration,
AVDTP-session/A2DP-channel lookup, SetConfiguration completion, and endpoint
cleanup. Remaining boundary: the frame is still injected into
`avdtp_parse_data()` by the bounded probe; the next convergence target is to
arrive there through the real `session_cb()` fd read path over hwsim.

## 2026-06-15 A2DP AVDTP session callback fd-read remote SEP ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-session-caps=read:1,registered:1,lookup:1,set:1,cleanup:1,total:5`.
The AVDTP object probe now writes a complete single-packet GetCapabilities
ACCEPT frame into a pipe-backed fd, attaches that fd to a shim `GIOChannel`,
and enters upstream `audio/avdtp.c::session_cb()`.

This exercises the upstream read path before `avdtp_parse_data()`,
`avdtp_parse_resp()`, and `avdtp_get_capabilities_resp()` fill the remote SEP
capability state. The helper also uses heap-owned `struct pending_req` and
`struct seid_req`, matching upstream `session_cb()` ownership because the real
callback frees the pending request after processing the response.

A2DP maps the resulting remote SEP through `a2dp.c::register_remote_sep()`,
and MediaEndpoint drives `media.c::set_configuration()` plus
`endpoint_reply()` with the generated remote path. The remaining boundary is
that the fd is still a bounded pipe supplied by the probe, not yet the hwsim
L2CAP signaling channel established by the full AVDTP connect/discover path.

## 2026-06-15 A2DP AVDTP discover-driven remote SEP ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-discover-caps=request:1,registered:1,lookup:1,set:1,cleanup:1,total:5`.
The AVDTP object probe now calls upstream `audio/avdtp.c::avdtp_discover()`
against a socketpair-backed signaling fd. This lets upstream `send_request()`
create and own the Discover pending request, serialize the Discover command,
process a Discover ACCEPT frame through `session_cb()`, enqueue/send the
GetCapabilities request, and then process a GetCapabilities ACCEPT frame
through the same callback path.

This pulls the upstream request queue and pending-request cleanup lifecycle
into the A2DP convergence evidence instead of manually installing
`session->req`. The resulting remote SEP is discovered by upstream AVDTP,
filled with GetCapabilities response data, mapped by `a2dp.c::register_remote_sep()`,
and consumed by `media.c::set_configuration()` plus `endpoint_reply()`.

The remaining boundary is that the signaling fd is still a probe-owned
socketpair. The next convergence target is to replace that fd with the hwsim
L2CAP signaling channel created by the real AVDTP connect path.

## 2026-06-15 A2DP AVDTP l2cap_connect-driven remote SEP ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-l2cap-caps=connect:1,registered:1,lookup:1,set:1,cleanup:1,total:5`.
The btio shim now provides a socketpair-backed `bt_io_connect()` so upstream
`audio/avdtp.c::l2cap_connect()` can create the signaling `GIOChannel` instead
of the AVDTP probe directly installing a socketpair.

The AVDTP probe starts from a disconnected session and calls upstream
`avdtp_discover()`. Upstream `send_request()` calls `l2cap_connect()`, enters
CONNECTING, queues the Discover request, and then the probe completes the
connect callback so upstream transitions to CONNECTED and processes the queue.
Discover and GetCapabilities responses then flow through `session_cb()` as
before.

The resulting remote SEP is mapped by `a2dp.c::register_remote_sep()` and
consumed by `media.c::set_configuration()` plus `endpoint_reply()`. Remaining
boundary: the btio connect shim still uses a socketpair; the next step is to
bind `bt_io_connect()` to the NuttX/Linux-Bluetooth hwsim L2CAP signaling
socket implementation.

## 2026-06-15 A2DP MediaEndpoint SetConfiguration error ownership

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-set-error=callback:1,request:1,transport:1,unref:1,cleanup:1,total:5`.
The bounded direct-object probe now drives the real upstream
`endpoint_reply()` error path for a pending `SetConfiguration` request.

The marker validates failed callback delivery, pending request removal,
MediaTransport removal through the upstream `SetConfiguration` error branch,
error reply unref ownership, and final endpoint unregister cleanup. This covers
the AVDTP configuration failure recovery behavior that must not leave a stale
transport attached to the endpoint.

## 2026-06-15 A2DP MediaEndpoint ClearConfiguration direct cleanup

`bluez-a2dp-upstream-convergence-closeout` now requires
`upstream-media-endpoint-clear=message:1,send:1,transport:1,cleanup:1,total:4`.
The bounded direct-object probe first drives real upstream `set_configuration()`
to create a MediaTransport, completes the request successfully through
`endpoint_reply()`, then calls the real upstream `clear_configuration()` body.

The marker validates the generated
`org.bluez.MediaEndpoint1.ClearConfiguration` D-Bus method-call message, the
send through `g_dbus_send_message()`, MediaTransport removal/destruction, and
final endpoint unregister cleanup. With this, the direct MediaEndpoint config
slice covers SelectConfiguration request/reply/error, SetConfiguration
request/reply/error, and ClearConfiguration cleanup.
## OBEX closeout status

`bluezobex` adds an independent OBEX profile-shaped adapter for PBAP, OPP,
MAP, MNS, FTP, Sync, and BIP.  The hwsim closeout cases now require both the
existing `bluezdaemon` aggregate evidence and per-role `bluez-obex:` evidence:
RFCOMM-shaped L2CAP open/connect, OBEX transaction labels, transfer/error
policy markers, upstream source coverage, cleanup, and a staged boundary
showing this is still a NuttX adapter rather than unmodified upstream obexd.

## Remaining profile closeout status

`bluezprofile` adds independent closeout coverage for Mesh, ASHA, MIDI,
Ranging, Print, and iAP.  These profiles now emit per-role `bluez-profile:`
evidence for profile registration, bearer setup, profile transactions,
error-policy mapping, upstream source coverage, cleanup, and staged adapter
boundaries.  This brings the remaining profile-family hwsim gates up to the
same closeout style as A2DP, LE Audio, GATT, HID/HOGP, HFP/HSP, and OBEX,
while still documenting that compatibility glue remains between NuttX and the
unmodified upstream BlueZ daemon/object ownership model.

## BlueZ mgmt daemon bootstrap closeout

`bluezmgmt daemon-bootstrap` adds a stronger baseline for the basic controller
lifecycle.  It keeps one AF_BLUETOOTH/BTPROTO_HCI control socket open and
drives a BlueZ `src/shared/mgmt.c` / `src/adapter.c`-style sequence:
controller discovery, adapter policy, BR/EDR + LE enablement, discovery
start/stop, NoInputNoOutput pair/connect, connection-info query, disconnect,
unpair cleanup, and mgmt error-policy mapping.  The hwsim case is
`bluez-mgmt-daemon-bootstrap` and the validator requires the final staged
boundary `bluezmgmt-daemon-bootstrap-adapter-not-unmodified-bluetoothd`.

## A2DP single-entry profile closeout

`blueza2dp closeout source|sink` adds an independent A2DP profile closeout
entry point.  It keeps the AVDTP signaling bearer, media bearer,
MediaTransport acquire/release ownership, SBC codec policy, stream
start/suspend/close/abort lifecycle, upstream source coverage, and final
ownership ledger in one command.  The `bluez-a2dp-upstream-convergence-closeout`
hwsim gate now requires this `bluez-a2dp:` evidence in addition to the existing
`bluezdaemon audio-a2dp-closeout-full` umbrella evidence.  The boundary remains
`blueza2dp-adapter-not-unmodified-bluetoothd`, so this is a stronger staged
profile closeout rather than a claim that unmodified upstream `a2dp.c` and
`avdtp.c` own the full runtime yet.

## LE Audio single-entry profile closeout

`bluezleaudio closeout source|sink` adds an independent LE Audio closeout
entry point.  It gathers the PACS/ASCS GATT control path, BAP policy,
ASCS control-point lifecycle, ISO socket media path, LC3-like SDU payload,
MediaTransport acquire/release ownership, upstream source coverage, cleanup,
and final ownership ledger in one command.  The `bluez-le-audio-umbrella`
hwsim gate now requires this `bluez-leaudio:` evidence on both roles.  The
boundary remains `bluezleaudio-adapter-not-unmodified-bluetoothd`, so this is
a stronger staged profile closeout rather than a claim that unmodified upstream
`bap.c`, `pacs.c`, and `ascs.c` own the complete runtime yet.

## 2026-06-15 BlueZ bneptest native BNEP closeout tightening

This pass tightens the BT Network/BNEP convergence gate without replacing it with
another staged marker.  `bluezbneptest native-closeout` now reuses the same
BlueZ-derived path as `fd-handoff`: `socket(AF_BLUETOOTH, SOCK_RAW,
BTPROTO_L2CAP)` creates a connected L2CAP fd, `socket(AF_BLUETOOTH, SOCK_RAW,
BTPROTO_BNEP)` receives that fd through `BNEPCONNADD`, and the imported Linux
BNEP socket/core/netdev path owns the resulting `btn0` session until
`BNEPCONNDEL` teardown.

The hwsim validators now require explicit source and ownership evidence for:

- BlueZ userspace shape: `third/bluez/tools/bneptest.c`.
- Linux BNEP socket ABI: `net/bluetooth/bnep/sock.c`, including
  `BNEPGETSUPPFEAT`, `BNEPCONNADD`, `BNEPGETCONNLIST`, `BNEPGETCONNINFO`, and
  `BNEPCONNDEL`.
- Linux BNEP session ownership: `bnep_add_connection`, `session_link`,
  `kthread_run`, `session_thread`, stop/unlink/terminate cleanup.
- Linux BNEP netdev ownership: `netdev_setup`, register/unregister, `btn0`,
  `ndo_start_xmit`, L2CAP delivery, and `netif_rx` evidence.

`bluez-bneptest-fd-handoff` now runs both the existing compatibility handoff and
the stricter native closeout.  `bluez-net-upstream-convergence-closeout` also
injects `bluezbneptest native-closeout` on BT1 and BT2, so BT/BLE NET upstream
convergence cannot pass on Network Profile markers alone.

Boundary: this is still a BlueZ-facing hwsim semantic closeout.  It proves the
NuttX port exercises the imported Linux BNEP socket/core/netdev behavior through
a BlueZ-shaped fd handoff, but it is not yet a claim that unmodified upstream
`bluetoothd` owns the complete Network plugin lifecycle without adapter glue.

## 2026-06-15 bluezprofile expansion for HID/HOGP, HFP/HSP, and GATT

`bluezprofile` now covers the profile families that previously relied only on
`bluezdaemon profile-*` closeout evidence:

- Classic HID host/device over HID L2CAP control/report ownership.
- BLE HOGP host/device over ATT report-map, protocol-mode, and input notify
  transactions.
- HFP HF/AG and HSP HS/AG over RFCOMM-shaped AT command ownership.
- Generic GATT client/server over ATT discovery, read, write, CCC, notify, and
  application registration ownership.

The existing hwsim gates keep their daemon-style profile closeouts and now also
run the independent `bluezprofile closeout ...` command per role.  The validator
requires the new `bluez-profile:` source, role, transaction-label, lifecycle, and
staged-boundary evidence for `bluez-hid-hogp-profile-closeout`,
`bluez-hfp-hsp-profile-closeout`, and `bluez-gatt-profile-closeout`.

Boundary: these remain bounded profile semantic closeouts through the NuttX hwsim
adapter.  The bar is now closer to the A2DP/LE Audio closeout style because each
role has explicit BlueZ source mapping and bearer transaction evidence, but full
unmodified upstream `bluetoothd` object ownership is still a separate replacement
track.

## 2026-06-15 OBEX obexd ownership tightening

`bluezobex` now emits a stricter `obexd`-style ownership model for PBAP, OPP,
MAP/MNS, FTP/Sync, and BIP.  Each closeout still drives the same RFCOMM-shaped
transport and OBEX transactions, but the evidence now also models the upstream
`obexd` daemon/object lifecycle:

- `obexd` mainloop/session-bus ownership and `org.bluez.obex` bus name.
- plugin/server ownership from `obexd/src/main.c`, `plugin.c`, and `server.c`.
- Session object ownership with `/org/bluez/obex/session0` and the role-specific
  profile API, such as `PhonebookAccess1`, `ObjectPush1`, `MessageAccess1`,
  `FileTransfer1`, `Synchronization1`, or `Image1`.
- RFCOMM transport ownership mapped through `obexd/client/transport.c` and
  `obexd/src/transport.c`.
- Transfer object ownership with `org.bluez.obex.Transfer1` and
  `/org/bluez/obex/session0/transfer0`.
- Final ownership ledger requiring mainloop, session, transport, transfer,
  profile object, and cleanup finals.

The OBEX hwsim validators now require these `obexd` object/ownership markers in
addition to the existing per-profile OBEX transaction evidence.  This moves OBEX
closer to the A2DP/LE Audio closeout standard: it is no longer only a profile
packet probe, but a bounded `obexd` object-lifecycle semantic closeout.

Boundary: this remains an adapter-backed hwsim closeout.  The next true upstream
replacement step is to make the actual imported `obexd` mainloop and D-Bus object
registration own these sessions directly instead of emitting the equivalent
ownership ledger from the NuttX closeout adapter.

## 2026-06-15 BlueZ security/auth ownership closeout tightening

`bluezmgmt security-closeout` adds an explicit security/auth closeout to the
basic scan/connect/auth path.  It reuses the existing mgmt control socket daemon
bootstrap sequence and adds a BlueZ security ownership ledger around it:

- `org.bluez.Agent1` registration and IO capability ownership.
- Upstream `src/agent.c` object probe evidence.
- Device bonding lifecycle ownership from `src/device.c`: create bonding,
  bonding complete, bearer paired, bearer bonded, and unpair cleanup.
- Linux mgmt event-order evidence for command complete, settings, discovering,
  connected, key, disconnected, and unpaired events.
- Linux SMP/key lifecycle source mapping for LTK/IRK/CSRK and staged key-store
  cleanup.
- Error policy coverage for cancel-pending, confirm-negative, passkey-negative,
  and invalid IO capability.

`bluez-basic-scan-connect-auth-flow` and
`bluez-basic-upstream-convergence-closeout` now run this security closeout after
the pairing matrix and before reconnect stress.  Their validators require the new
agent/device/mgmt/SMP/key-store final ledger.

Boundary: this is stronger BlueZ/Linux security semantic alignment, but it still
uses the NuttX mgmt adapter and staged keyring.  Full completion still requires
real upstream SMP key derivation and persistent BlueZ storage ownership without
adapter-emitted ledgers.

## 2026-06-15 HCI socket ABI ownership tightening

`bluezhciraw socket-abi-closeout` adds a focused HCI socket ABI ownership closeout
inside `bluez-hci-mgmt-socket-closeout-full`.  The command opens and binds the
Linux Bluetooth HCI socket channels that BlueZ-facing tools depend on:

- `HCI_CHANNEL_RAW` for hcitool/lib/hci command traffic with HCI filter readback.
- `HCI_CHANNEL_USER` for controller-owned userspace command traffic.
- `HCI_CHANNEL_MONITOR` for btmon-style fanout of user/raw HCI events.
- `HCI_CHANNEL_CONTROL` for the mgmt control socket namespace.
- `HCI_CHANNEL_LOGGING` for btmon/user logging channel ownership.

The closeout drives raw and user command send/recv, observes monitor fanout, then
emits an ownership ledger mapped to Linux `hci_sock.c`, `hci_core.c`,
`hci_event.c`, and `mgmt.c`.  `bluezhciioctl basic` remains the ioctl owner in the
same full gate; `socket-abi-closeout` deliberately does not claim ioctl ownership
it does not execute itself.

Boundary: this tightens the BlueZ-facing HCI socket ABI toward Linux semantics,
but it remains a NuttX hwsim adapter closeout.  Full completion still requires
removing the remaining compatibility shims so imported BlueZ tools and daemon
paths own these sockets directly without adapter-emitted ledgers.

## 2026-06-15 basic lifecycle and hwsim medium ownership tightening

The basic BlueZ closeout now records lifecycle and hwsim medium ownership in the
same gate that covers scan, connect, authentication, and reconnect stress.

`bluezdaemon reconnect-stress` now emits:

- multi-round lifecycle ledger for mgmt open, adapter power, pair rounds,
  connect complete, disconnect cleanup, and final zero references.
- hwsim medium ledger for `CTRL`, `ADV`, and `ACL` records, role-local consumer
  offsets, stale-record skipping, replay guard, and cleanup final state.

`bluezdaemon basic-closeout bt|ble` now emits a medium lifecycle map covering all
current hwsim record types: `CTRL`, `ADV`, `ACL`, `ISO`, and `BNEP`.  The map
records role/channel offset consumers, destination/sequence filtering, FNV-1a
checksum protection, reconnect replay guard, abnormal-exit offset resume, and
cleanup final state.

The basic hwsim validators require these lifecycle and medium ledgers.  This
moves the basic path closer to Linux-like long-running behavior instead of only
proving one-shot command success.

Boundary: this documents and validates the current hwsim medium semantics; it is
not yet a full replacement for kernel-side queueing, socket wait queues, and
BlueZ daemon-owned persistent storage without adapter assistance.

## 2026-06-15 BLE IPSP/6LoWPAN native ownership tightening

BLE NET/IPSP now carries stronger Linux ownership evidence in both the standalone
`bluezipsp` path and the daemon-style `bluezdaemon ipsp` path.

`bluezipsp connect/status/disconnect` now reports:

- native `net/bluetooth/6lowpan.c` ownership for register, peer add, channel
  attach, unregister, channel release, peer unref, and netdev unregister.
- LE L2CAP CoC ownership through `l2cap_core.c` and `l2cap_sock.c` with PSM
  `0x0023`, CID `0x0040`, connected state, and fd handoff.
- `bt0` netdev ownership with `ndo_start_xmit`, `netif_rx`, MTU 1280, and
  6LoWPAN address length.
- `net/6lowpan/iphc.c` ownership for TX/RX IPHC and fragmentation datapath.
- a final upstream coverage map for BlueZ Network/IPSP plus Linux 6LoWPAN,
  L2CAP CoC, and IPHC.

`bluezdaemon ipsp` now emits the same native ownership phases during profile
connect, status, and disconnect.  The validators for `bluez-ipsp-closeout-full`,
`bluez-daemon-ipsp-closeout-full`, and the BLE roles of
`bluez-net-upstream-convergence-closeout` require these native ownership markers.

Boundary: this is a stronger hwsim semantic closeout for BLE NET.  It still uses
NuttX adapter functions to expose the ownership ledger; the remaining upstream
replacement work is to let imported BlueZ Network/IPSP and Linux 6LoWPAN objects
own these sessions directly without adapter-emitted evidence.

## A2DP upstream object ownership closeout standard

The `blueza2dp closeout source|sink` path now treats the standalone hwsim tool as an upstream BlueZ ownership probe, not only as a packet smoke test.  A passing A2DP closeout must account for these Linux/BlueZ semantics:

- `org.bluez.MediaEndpoint1` object registration with endpoint, setup, remote SEP, and stream references.
- AVDTP signaling session ownership across discovery, capabilities, configuration, open, streaming, suspend, close, and abort states.
- `org.bluez.MediaTransport1` object export, registry insertion/removal, owner-watch add/remove, and fd handoff through `Acquire` / `TryAcquire` / `Release`.
- Media L2CAP ownership for the RTP/SBC payload path, with the final transport fd closed and all endpoint/transport/session references back to zero.
- Explicit staged boundary marker documenting that this is still the NuttX hwsim BlueZ adapter, but its externally observed ownership contract is aligned with upstream BlueZ/Linux behavior.

This is the standard the remaining BT/BLE profiles must meet before they are considered closeout-level rather than compatibility-glue-level.

## LE Audio upstream object ownership closeout standard

The `bluezleaudio closeout source|sink` path now follows the same ownership-closeout standard as A2DP.  A passing LE Audio closeout must account for these upstream BlueZ/Linux semantics:

- PACS and ASCS GATT database ownership, including `org.bluez.PACSource1`, `org.bluez.PACSink1`, `org.bluez.ASCS1`, and `org.bluez.ASE1` object registration.
- ATT/GATT request lifecycle for MTU exchange, PACS discovery, ASCS discovery, ASCS control-point writes, notify watch ownership, and final pending-request cleanup.
- BAP session ownership for local/remote PACs, ASE, stream, QoS, ISO I/O, and transport references.
- LC3 MediaEndpoint and MediaTransport ownership, including registry insert/remove, owner-watch add/remove, `Acquire`, `TryAcquire`, `Release`, and fd handoff.
- ISO socket ownership from bind/connect through LC3 SDU transfer and close, with final BAP/PACS/ASCS/ASE/endpoint/transport references back to zero.

This makes LE Audio a closeout-level profile gate rather than an ISO payload probe.  Remaining profiles should use this same structure: object register, session ownership, controller/socket datapath, error lifecycle, cleanup ledger, and hwsim validator coverage.

## BT Network native BNEP ownership closeout standard

`bluezbneptest native-closeout` and `blueznetwork connect/status/disconnect` now use the same native BNEP evidence model.  A passing BT Network closeout must prove:

- Connected L2CAP fd handoff to `BNEPCONNADD`, matching BlueZ `bneptest.c` and Network Profile behavior.
- Linux BNEP socket ioctl ownership for add/list/info/delete.
- Linux BNEP session ownership for `bnep_add_connection`, `register_netdev`, session link, `kthread_run`, session thread, RX/TX queues, and cleanup.
- Linux netdev datapath ownership for `ndo_start_xmit`, `bnep_tx_frame`, L2CAP/hwsim delivery, `bnep_rx_frame`, and `netif_rx` into NuttX IP.
- Final teardown ledger with fd/session/thread/netdev/queue/skb/L2CAP/BNEP references all released.

This is the Network-profile equivalent of the A2DP and LE Audio ownership closeout gates.

## BLE IPSP / 6LoWPAN ownership closeout standard

The BLE IP path now follows the same closeout model as BT BNEP, A2DP, and LE Audio.  A passing BLE IPSP/6LoWPAN closeout must prove:

- BlueZ Network/IPSP profile object ownership for `org.bluez.Network1` and the IPSP service UUID.
- LE L2CAP CoC ownership for IPSP PSM `0x0023` and fixed hwsim CID `0x0040`.
- Imported Linux `net/bluetooth/6lowpan.c` ownership for peer add/delete, CoC open/close, channel ready/close callbacks, and BT transmit/receive callbacks.
- Imported Linux `net/6lowpan/iphc.c` ownership for IPv6 header compression/decompression.
- NuttX lower-half netdev ownership for `bt0`, TX from NuttX IPv6 into 6LoWPAN/IPHC/L2CAP/hwsim, and RX from hwsim/L2CAP/IPHC into NuttX IPv6.
- Final cleanup ledger with netdev, CoC, peer, channel references, active TX/RX counters, and pending skb state all returned to zero.

The BLE ping gate must therefore prove both datapath success (`PING6`) and lifecycle cleanup, not only object registration.

## BT BNEP iperf closeout standard

BlueZ Network Profile-shaped iperf tests now require two layers of evidence:

- Throughput evidence: non-zero Bytes and non-zero Mbits/sec for TCP/UDP forward and reverse flows.
- Ownership evidence: `Network1.Connect` creates a connected L2CAP fd, passes it through `BNEPCONNADD`, attaches `btn0`, drives imported Linux BNEP session/netdev datapath, and tears down to `bnep-native-active=0`.

This keeps `btbneptest` and `bluez-bneptest` as lower-level fd/ioctl probes while making `blueznetwork` the profile-level iperf closeout path.

## HCI / mgmt / socket ABI closeout standard

The HCI and mgmt closeout gates now require ownership evidence in addition to command success:

- `bluezmgmt daemon-bootstrap` records the Linux mgmt event order for controller info, adapter policy, discovery, pairing, connection info, disconnect, unpair, and invalid-parameter error handling.
- `bluezmgmt security-closeout` records Agent1 ownership, device bonding ownership, SMP key lifecycle ownership, mgmt event ordering, and final zero-reference cleanup.
- `bluezdaemon pairing-matrix` records daemon/mainloop ownership for mgmt watches, Agent1 watches, pair timers, callback cleanup, and final device/bond refs.
- `bluezhciraw socket-abi-closeout` records RAW, USER, MONITOR, CONTROL, and LOGGING HCI channels, monitor fanout, command/event order, and final fd/filter/skb/pending-command cleanup.

This keeps the user-space BlueZ side aligned with Linux behavior: BlueZ drives the controller through HCI/mgmt sockets and the hwsim gate verifies both event ordering and lifecycle ownership.

## Full BT/BLE profile closeout ownership standard

The hwsim profile gates now use the same closeout standard as the A2DP and
LE Audio work: a profile is not considered converged just because it reaches a
synthetic data path.  Each closeout must also show explicit Linux/BlueZ-style
ownership and lifecycle cleanup.

Current daemon/tool closeout requirements include:

- `bluez-daemon: <profile> closeout ownership-ledger ...` for daemon-owned
  profiles, with released D-Bus/profile objects, zero adapter/device refs,
  zero mainloop watches, zero pending requests/events, active error-policy
  coverage, and `cleanup-final=1`.
- Tool-level `upstream-ownership-ledger` for HID/HOGP, HFP/HSP, GATT and OBEX,
  covering their native bearer fd/session ownership and final cleanup.
- Validator-side automatic inference: whenever a case declares
  `closeout cleanup`, the validator also requires the matching ownership
  ledger.  This keeps future HID/HOGP, HFP/HSP, GATT, OBEX, Mesh, MIDI, ASHA,
  Ranging and related profiles aligned with the A2DP/LE Audio closeout bar.

This is still a staged NuttX-sim hwsim convergence layer, not a claim that the
unmodified upstream `bluetoothd` binary is running unchanged.  The intended
semantic bar is Linux-compatible lifecycle behavior: D-Bus object ownership,
profile registration/release, native HCI/L2CAP/RFCOMM/ATT/SCO/ISO bearer
ownership, error cleanup, reconnect safety, and final reference cleanup must be
visible in the per-role hwsim logs.

## CMake/Makefile profile build parity and bluezprofile ownership gate

The apps-side BlueZ profile port now keeps the CMake application registration in
step with the Makefile path for the daemon-shaped adapter.  The CMake daemon
source list includes the upstream textfile/EIR/advertising object probes and
uses both the upstream object shim include directory and the imported
`bluez/upstream` include directory.

The independent `bluezprofile closeout ...` command now also emits explicit
cleanup and `upstream-ownership-ledger` records.  The hwsim validator infers
these requirements from every `bluez-profile: ... command=connect` requirement,
so the independent remaining-profile command path must show released profile and
service objects, closed bearer fd ownership, zero adapter/device refs, zero
mainloop watches, zero pending requests/events, active error-policy coverage and
`cleanup-final=1`.

This closes a practical build/command gap: remaining profiles are no longer only
aggregated under `bluezdaemon`; their standalone apps-side command path has the
same ownership evidence bar used by the daemon and by the A2DP/LE Audio profile
closeouts.

## bluezprofile upstream-shaped lifecycle state machines

`bluezprofile closeout ...` no longer reports only three generic transaction
labels.  Each standalone profile closeout now emits a family-specific
`upstream-state-machine` record with the BlueZ owner files, lifecycle states,
D-Bus/native objects and Linux bearer involved in that profile family.

Examples of the state-machine contracts now visible to hwsim logs include:

- HID: SDP registration, Profile1 connect, L2CAP control/interrupt, HIDP
  connadd, input events, virtual unplug and release.
- HOGP: GATT discovery, report map, protocol mode, CCC enable, input/output
  reports, suspend/resume and release.
- HFP/HSP: RFCOMM session ownership, AT/SLC or headset-control lifecycle, SCO
  bearer ownership and transport release.
- GATT: application registration, ATT attach, service discovery, read/write,
  notify/indicate, ATT error response and unregister.
- Mesh, ASHA, MIDI, Ranging, Print and iAP: profile-specific lifecycle states
  matching their imported BlueZ owner areas rather than a generic payload probe.

The validator now automatically requires the `upstream-state-machine` line for
every declared `bluez-profile: ... command=connect` requirement, in addition to
the existing standalone `upstream-ownership-ledger` requirement.  This moves the
remaining profile command path one step closer to real upstream BlueZ semantics:
a case can no longer pass with only a bearer write and a staged-boundary marker.

## bluezprofile protocol-specific PDU semantics

The standalone `bluezprofile closeout ...` path no longer writes a uniform
`family:role:label` payload for every profile family.  Each transaction now
emits an `upstream-pdu` record and builds a payload carrying profile-specific
BlueZ/Linux semantics:

- HID uses HID control/interrupt semantics such as set-protocol and input/output
  reports.
- HOGP, GATT, ASHA and MIDI use ATT/GATT-style operations such as read request,
  write request, CCC enable and handle-value notify.
- HFP/HSP use RFCOMM AT command semantics for SLC, codec negotiation, call list,
  button and volume control.
- Mesh uses PB-ADV/proxy/network PDU semantics.
- Ranging uses RAP/LE Channel Sounding capability and result-report semantics.
- Print and iAP use SDP/RFCOMM/HCRP or iAP2 control/session semantics.

The hwsim validator now requires an `upstream-pdu` line for every
`bluezprofile closeout` command, alongside the state-machine and ownership
ledger.  This removes another staged shortcut: remaining profile tests must now
show profile-specific protocol intent before the bearer write, rather than only
proving that a generic payload crossed L2CAP/RFCOMM/ATT.

## bluezprofile opcode/wire payload convergence

`bluezprofile closeout ...` now builds protocol-specific payloads instead of
only appending semantic text to the generic transaction label.  The command
emits `opcode=` and `wire=` for every transaction, then writes a payload that is
closer to the corresponding upstream bearer:

- HIDP control/interrupt byte frames for classic HID.
- ATT opcodes for HOGP, GATT, ASHA and BLE MIDI discovery/control/notify paths.
- RFCOMM AT command frames for HFP/HSP.
- Mesh PB-ADV/proxy/access-message byte frames.
- RAP/LE Channel Sounding capability/result byte frames for Ranging.
- SDP/RFCOMM/HCRP/iAP2 control text frames for Print and iAP.

The hwsim validator now requires each standalone `bluezprofile` transaction to
show `upstream-pdu`, `opcode=` and `wire=`, plus the existing state-machine and
ownership-ledger evidence.  This is still a compatibility-layer bridge, but it
removes the previous generic string-payload shortcut and gives the hwsim medium
profile-specific wire intent to carry.

## bluezprofile PDU codec split

The standalone `bluezprofile` path now routes profile transaction encoding
through `upstream_profile_pdu_codec.c` instead of keeping the opcode/wire tables
inside `tools/profile_main.c`.  `profile_main.c` is now responsible for profile
selection, bearer open/connect/write/close and lifecycle logging; the codec owns
profile-specific payload construction, semantic labels, opcode names, wire names
and upstream owner provenance.

The `bluez-profile: upstream-pdu` log now includes:

- `codec-source=apps/wireless/linux_bluetooth/bluez/upstream_profile_pdu_codec.c`
- `upstream-owner=third/bluez/...`
- `opcode=...`
- `wire=...`
- `payload-hex=...`

Both Makefile and CMake application registration include the new codec source
when `CONFIG_LINUX_BLUEZ_PROFILE=y`.  The validator requires these provenance
and hex payload fields for every standalone `bluezprofile closeout` transaction.
This makes the next convergence step clearer: replace the codec internals with
direct upstream BlueZ parser/encoder reuse family by family, while keeping the
same command and hwsim evidence contract.

## bluezprofile codec encoder/decoder contract

The standalone `bluezprofile` PDU codec now performs a lightweight roundtrip
self-check for every profile transaction.  After encoding a profile-specific
payload, the codec classifies the payload through a matching upstream-shaped
decoder and reports both the intended opcode and the decoded opcode.

`bluez-profile: upstream-pdu` now includes:

- `encoder=bluez-profile-pdu-encoder:upstream-shaped`
- `decoder=bluez-profile-pdu-decoder:upstream-shaped`
- `decoded-opcode=...`
- `roundtrip=1`

The validator requires these fields for every standalone `bluezprofile closeout`
PDU.  This adds a stronger contract than hex payload evidence alone: the payload
must be recognizable by the profile codec as the same protocol opcode it was
intended to encode.  The next reduction in compatibility glue should replace
these upstream-shaped encoder/decoder branches with direct calls into imported
BlueZ/shared ATT, HID/HOGP, Mesh, RFCOMM profile, Ranging and OBEX/iAP parser or
builder code family by family.

## bluezprofile request lifecycle codec ownership

The standalone `bluezprofile` PDU record now carries more than encoder/decoder
roundtrip state.  Each transaction also reports the upstream-shaped parser,
handler, policy, error and cleanup owner that would own the same request in the
real BlueZ/Linux profile path:

- `parser-owner=...`
- `handler-owner=...`
- `policy-owner=...`
- `error-owner=...`
- `cleanup-owner=...`

The hwsim validator requires these fields for every standalone profile PDU.
This moves the remaining profile path one step closer to upstream behavior:
HID/HOGP, HFP/HSP, GATT, Mesh, ASHA, MIDI, Ranging, Print and iAP transactions
must now identify not only their wire opcode, but also the BlueZ/Linux lifecycle
owner responsible for parsing, dispatch, policy decisions, error mapping and
cleanup.

Boundary: these owner strings are still an adapter-side convergence map.  The
next reduction in glue is to replace each family-specific branch with direct
calls into the imported upstream parser/handler code while preserving the same
hwsim-visible lifecycle contract.

## bluezprofile request lifecycle result gate

The same `bluez-profile: upstream-pdu` record now also reports the result of the
upstream-shaped request lifecycle:

- `parse-ok=1`
- `parse-detail=...-structure-ok`
- `dispatch-result=handler-dispatched`
- `handler-ok=1`
- `policy-ok=1`
- `error-status=success-or-profile-error-mapped`
- `error-map-ok=1`
- `cleanup-state=request-state-released`
- `cleanup-ok=1`

The validator requires these fields for every standalone profile transaction.
This makes the closeout stricter than a payload or owner-map check: the PDU must
be parseable, dispatched to a profile handler, accepted by the profile policy,
mapped through the error path, and released back to a clean request state before
the profile can satisfy the current hwsim semantic bar.

`parse-ok` is now produced by a wire-specific structure check rather than by the
opcode roundtrip alone.  The codec validates ATT, HIDP, RFCOMM AT, Mesh, RAP /
LE Channel Sounding, HCRP and iAP2-shaped payloads before dispatching the
transaction.  The validator requires the corresponding `parse-detail` marker so
future changes cannot silently fall back to opcode-only acceptance.

The parser has also been tightened from bearer-level acceptance to
opcode-specific structure checks.  For example, ATT read, write and notify
payloads now require the expected opcode and minimum handle/value layout; HIDP
distinguishes set-protocol from input-report frames; RFCOMM AT checks BRSF,
codec, call-list, button and volume command prefixes; Mesh distinguishes PB-ADV,
Proxy Config and Access Message payloads; RAP / LE Channel Sounding distinguishes
capability, exchange and result-report forms; HCRP and iAP2 check their own
profile text frames.  `parse-detail` therefore names the accepted opcode family,
not merely the carrier.

The PDU record now exposes parser result metadata:

- `parse-backend=...`
- `parsed-handle=0x....`
- `parsed-value-len=...`

ATT-like profiles report the parsed ATT handle and value length, while HIDP,
RFCOMM, Mesh, RAP, HCRP and iAP2-shaped parsers report the backend that accepted
the opcode-specific payload and the payload/value length they consumed.  The
validator requires this metadata on both positive and malformed-PDU paths.  This
creates a stable hwsim-visible parser contract for replacing the current
adapter-side parser branches with imported upstream parser calls.

ATT-like profile parsing is now routed through a single
`bluez-shared-att-core:*` backend inside the profile codec.  HOGP, GATT, ASHA
and BLE MIDI no longer each carry separate ATT parsing branches in the main
profile parser path; they share the same read-by-type, read, read-by-group,
write, write+notify, notify and MIDI timestamp parser output model.  The
validator requires `parse-backend=bluez-shared-att-core:` for these families.
The next convergence step is to replace this backend implementation with direct
calls into imported BlueZ `src/shared/att.c` style parser code while preserving
the same hwsim-visible output fields.

The ATT-like backend now also reports dispatch and error-response metadata:

- `parsed-opcode=0x..`
- `handler-backend=bluez-shared-att-dispatch:...`
- `att-error-code=0x..`
- `att-error-rsp=...`
- `att-request-state=...`
- `att-mtu=...`
- `att-security-state=...`
- `att-requires-security=...`

Positive HOGP, GATT, ASHA and MIDI transactions must dispatch through the shared
ATT backend with `att-error-code=0x00` and `att-error-rsp=none`.  Malformed PDU
probes must instead report `handler-backend=profile-error-dispatch:rejected`,
`att-error-code=0x04`, and
`att-error-rsp=ATT_ERROR_RSP_INVALID_PDU-or-profile-error`.  This makes the
compatibility seam closer to BlueZ `bt_att` behavior: parse, dispatch, status
mapping and request cleanup are visible as distinct fields.

The same seam now records the request queue and policy state expected around
BlueZ `bt_att`: valid ATT-like requests report
`att-request-state=queued-dispatched-completed`, `att-mtu=23`, and
`att-security-state=security-checked`; malformed probes report
`att-request-state=queued-rejected-released` and
`att-security-state=security-not-consumed`.  This keeps MTU and security policy
visible before the backend is replaced with direct upstream ATT request queue
code.

The request queue evidence now includes request identity and pending-count
transitions:

- `att-request-id=0x..`
- `att-pending-before=1`
- `att-pending-after=0`
- `att-completion-cb=bluez-shared-att-callback:complete`
- `att-queue-backend=bluez-shared-att-queue:single-flight`
- malformed probes use `att-request-id=0xff` and
  `att-completion-cb=bluez-shared-att-callback:error`
  with `att-queue-backend=bluez-shared-att-queue:error-single-flight`

This mirrors the important external behavior of BlueZ `bt_att` request
ownership: a request is queued, dispatched, completed or errored through a
callback, then removed from the pending set.  The validator requires these
fields for ATT-like profile families and their malformed-PDU probes.

The request id and pending counters are now produced through a shared
`bluez_profile_att_queue` helper instead of being assigned independently in each
parser branch.  This keeps HOGP, GATT, ASHA and MIDI on a single ATT request
ownership seam that can later be replaced with imported `bt_att` pending queue
objects.

The queue helper now creates an explicit `bluez_profile_att_request` object for
each positive or malformed request.  The hwsim-visible PDU record includes:

- `att-request-owner=bluez-shared-att-request`
- `att-request-lifecycle=alloc-enqueue-dispatch-complete-unref`
- malformed probes use
  `att-request-lifecycle=alloc-enqueue-reject-error-unref`
- `att-request-ref-before=1`
- `att-request-ref-after=0`
- `att-timeout-state=timer-armed-cleared`
- malformed probes use `att-timeout-state=timer-armed-cleared-on-error`
- `att-cancel-state=not-cancelled`
- `att-timer-before=1`
- `att-timer-after=0`

The validator requires these fields for ATT-like profile families.  This makes
the staged queue helper closer to imported `bt_att` ownership rules: request
allocation, queue publication, completion or error callback, and final unref are
all explicit before the backend is replaced with the real upstream object.
Timer cleanup is part of the same contract, so an ATT-like closeout cannot pass
while leaving the request timeout lifecycle implicit.

ATT-like closeout now also includes an explicit cancel probe.  HOGP, GATT, ASHA
and MIDI emit `bluez-profile: upstream-cancel-pdu ...` after the positive and
malformed-PDU paths.  The cancel probe uses a valid ATT read request shape, then
cancels it before dispatch completion:

- `att-request-state=queued-cancelled-released`
- `att-cancel-state=cancelled-before-dispatch`
- `att-request-id=0xfe`
- `att-completion-cb=bluez-shared-att-callback:cancel`
- `att-queue-backend=bluez-shared-att-queue:cancel-single-flight`
- `att-request-lifecycle=alloc-enqueue-cancel-unref`
- `att-timeout-state=timer-armed-cleared-on-cancel`
- `att-request-ref-after=0`
- `att-timer-after=0`

The validator requires this cancel transaction for ATT-like families.  The
current hwsim gate therefore covers all three request endings expected from the
future `bt_att` replacement seam: complete, error and cancel.

## bluezprofile negative request lifecycle gate

Each standalone `bluezprofile closeout ...` role now sends one malformed
profile PDU after the normal profile transactions.  This emits a separate
`bluez-profile: upstream-error-pdu ...` record and writes the malformed payload
through the same hwsim bearer as the valid transactions.

The expected result is intentionally different from the positive path:

- `roundtrip=0`
- `parse-ok=0`
- `parse-detail=malformed-profile-pdu-rejected`
- `dispatch-result=handler-rejected`
- `handler-ok=0`
- `policy-ok=1`
- `error-status=profile-error-mapped`
- `error-map-ok=1`
- `cleanup-state=request-state-released`
- `cleanup-ok=1`

The validator requires this negative lifecycle for every standalone profile
role.  This prevents the remaining profile gates from proving only the happy
path; malformed ATT, HIDP, RFCOMM, Mesh, RAP, HCRP, or iAP2-shaped payloads
must be rejected, mapped to a profile error, and cleaned up without leaking the
request state.

## bluezprofile end-to-end ownership contract

`bluezprofile closeout ...` now also emits an A2DP/LE Audio-style
`bluez-profile: closeout end-to-end-contract ... end-to-end-final=1` record for
every remaining profile family.  The contract makes each standalone profile
closeout account for:

- BlueZ profile owner files.
- D-Bus or native runtime objects.
- Profile session state ownership.
- Linux Bluetooth bearer ownership.
- Profile-specific request/response or payload datapath ownership.
- Ordering from register to connect, I/O and cleanup.
- Error lifecycle.
- Final cleanup ownership.

The hwsim validator infers this requirement from every
`bluez-profile: ... command=connect` record.  HID/HOGP, HFP/HSP, GATT, Mesh,
ASHA, MIDI, Ranging, Print, and iAP therefore use the same current semantic bar
as A2DP and LE Audio: a profile cannot pass with only a bearer write, PDU codec
roundtrip, or cleanup marker.

Boundary: this is still a NuttX hwsim BlueZ-facing adapter contract.  It aligns
the externally visible semantics with Linux/BlueZ ownership and cleanup
behavior, but it is not yet a claim that unmodified upstream profile daemons own
the runtime without compatibility glue.

## 2026-06-17 BlueZ Network/BNEP hwsim status

Current BlueZ-derived Network/BNEP validation now uses the imported Linux BNEP
path without the old direct BNEP-to-hwsim TX shortcut.

Verified hwsim gates after build:

- `bluez-bneptest-fd-handoff`
- `bluez-bneptest-ping`
- `bluez-bneptest-reconnect-stress`
- `bluez-network-ping`
- `bluez-network-reconnect-stress`

Important semantic marker:

```text
bnep-native-core-tx-path=kernel_sendmsg-only
```

This means the BlueZ-facing connected L2CAP fd is handed to `BNEPCONNADD`, and
imported BNEP transmits through `kernel_sendmsg()` into L2CAP/hwsim instead of
building hwsim ACL packets in BNEP core.

Residual:

- `bluez-network-iperf-matrix` still needs lifecycle/harness cleanup.  It shows
  non-zero native BNEP TX/RX datapath counters, but does not complete all four
  strict connect/disconnect cycles required by the matrix validator.

## 2026-06-18 BNEP/L2CAP ownership and full current closeout

The current four-role BlueZ functional closeout now passes without hidden BNEP
packet loss after tightening the fd handoff and L2CAP channel attach semantics.

Important fixes:

- `BNEPCONNADD` keeps ownership of the connected L2CAP fd until imported BNEP
  session teardown instead of letting the BlueZ-facing command close/free the
  socket prematurely.
- The BNEP-owned L2CAP socket is attached to the sim BR/EDR L2CAP channel at
  handoff time.  This gives `bnep_tx_frame() -> kernel_sendmsg() -> L2CAP` the
  negotiated hwsim MTU needed for 1400-byte PAN payloads.
- VHCI hwsim send errors are now returned to the caller, so ACL/ISO medium
  failures cannot be silently counted as successful TX.
- The full closeout role matrix uses valid PAN pairings:
  `PANU <-> NAP`, `PANU <-> GN`, and `GN <-> PANU`.
- The validator expects two BT1 PANU connects plus one GN connect, matching the
  valid matrix instead of the old invalid `NAP <-> GN` pairing.

Passing full closeout evidence:

```text
PASS bluez-current-functional-closeout
  PASS bt1
  PASS bt2
  PASS ble1
  PASS ble2
```

Artifacts:

```text
FeatherCore/build/logs/run-bnep-attach-owner-closeout.log
FeatherCore/build/bt-hwsim-bnep-attach-owner-closeout/
```

The same run was scanned for these regressions and none were present:

```text
BNEP TX failed
No response
100% packet loss
data-path-failed
```

Core gates revalidated after this change:

```text
PASS bluez-a2dp-upstream-convergence-closeout
PASS bluez-le-audio-umbrella
PASS bluez-mgmt-reconnect-stress
PASS bluez-hciioctl-basic
PASS hci-le-lifecycle
```

Artifacts:

```text
FeatherCore/build/logs/run-core-gates-after-bnep-attach.log
FeatherCore/build/bt-hwsim-core-gates-after-bnep-attach/
```

Current boundary:

This is a strong hwsim closeout for the current NuttX BlueZ-facing adapter path.
It should not be overstated as a fully glue-free upstream `bluetoothd` runtime.
The remaining upstream-convergence work is to continue replacing compatibility
seams with direct imported BlueZ/Linux objects while keeping the full closeout
and core gates passing.

## 2026-06-18 BNEP no-staging fallback closeout

The current BlueZ-facing BT/BLE hwsim closeout now rejects the legacy
`BNEP-staging` fallback at compile-evidence level.  Native BT Network/BNEP
validation must show `bnep-staging-fallback-compiled=0` as well as zero staging
activity counters.

Validation artifacts:

- `FeatherCore/build/logs/build-bt1-bnep-no-staging-fallback-v3.log`: PASS.
- `FeatherCore/build/logs/build-bt2-bnep-no-staging-fallback-v3.log`: PASS.
- `FeatherCore/build/logs/build-ble1-bnep-no-staging-fallback-v3.log`: PASS.
- `FeatherCore/build/logs/build-ble2-bnep-no-staging-fallback-v3.log`: PASS.
- `FeatherCore/build/logs/run-bnep-no-staging-fallback-closeout.log`: PASS.
- `FeatherCore/build/bt-hwsim-bnep-no-staging-fallback-closeout/run-results.json`:
  `passed=true`, `validate_rc=0`.

Validated case:

- `bluez-current-functional-closeout`: PASS for BT1, BT2, BLE1, and BLE2.
- BNEP status includes `bnep-staging-fallback-compiled=0`,
  `bnep-staging-active=0`, `bnep-staging-add=0`, `bnep-staging-del=0`, and
  native imported-BNEP datapath evidence.

Current boundary:

- The old BNEP staging socket fallback is now diagnostic-only through
  `CONFIG_NET_LINUX_BLUETOOTH_BNEP_STAGING_DIAGNOSTIC`.
- BlueZ Network/PAN hwsim validation remains on the imported Linux BNEP
  socket/core/netdev path and must not silently fall back to the staging socket
  registry.
- Remaining BlueZ-side convergence is still D-Bus/mainloop/object ownership,
  profile registration/lifetime, and reducing the non-BNEP compatibility
  surfaces toward directly imported upstream behavior.

## 2026-06-19 HCI/L2CAP/ISO socket fallback gate

The common BT/BLE hwsim closeout now rejects active HCI, L2CAP and ISO staging
socket fallback builds at compile-evidence level.

Required status markers:

```text
hci-sock-fallback-compiled=0
l2cap-sock-fallback-compiled=0
iso-sock-fallback-compiled=0
```

Validation artifacts:

- `FeatherCore/build/logs/build-bt1-socket-fallback-compiled-gate-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-socket-fallback-compiled-gate-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-ble1-socket-fallback-compiled-gate-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-socket-fallback-compiled-gate-v2.log`:
  PASS.
- `FeatherCore/build/logs/run-socket-fallback-compiled-gate-core-v2.log`:
  PASS.
- `FeatherCore/build/bt-hwsim-socket-fallback-compiled-gate-core-v2/`:
  artifact directory.

Validated cases:

```text
PASS bluez-basic-upstream-convergence-closeout
PASS bluez-le-audio-umbrella
```

Negative scan:

- No `hci-sock-fallback-compiled=1`.
- No `l2cap-sock-fallback-compiled=1`.
- No `iso-sock-fallback-compiled=1`.
- No fallback-related validator-missing marker.

Current boundary:

- The validated hwsim base and LE Audio paths now prove they are using the
  upstream-selected HCI/L2CAP/ISO socket implementation rather than the old
  staging socket registries.
- The remaining HCI/L2CAP/ISO fallback source should still be moved behind
  explicit diagnostic-only Kconfig options or deleted in a later no-glue pass.
- This is another reduction of active compatibility glue, not a claim that the
  whole BlueZ/Linux Bluetooth port is fully glue-free yet.

## 2026-06-19 HCI/L2CAP/ISO staging fallback diagnostic-only closeout

The legacy HCI, L2CAP and ISO staging socket fallbacks have been moved out of
the ordinary fallback path.  They now require explicit diagnostic Kconfig
selection:

```text
CONFIG_NET_LINUX_BLUETOOTH_HCI_SOCK_STAGING_DIAGNOSTIC
CONFIG_NET_LINUX_BLUETOOTH_L2CAP_STAGING_DIAGNOSTIC
CONFIG_NET_LINUX_BLUETOOTH_ISO_STAGING_DIAGNOSTIC
```

Behavioral rule:

- With upstream AF_BLUETOOTH enabled, disabling an upstream HCI/L2CAP/ISO socket
  implementation no longer silently registers the corresponding staging socket.
- Without the diagnostic option, the init path fails explicitly with `-ENOSYS`
  instead of hiding the missing upstream implementation behind compatibility
  glue.

Validation artifacts:

- `FeatherCore/build/logs/build-bt1-socket-fallback-diagnostic-only.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-socket-fallback-diagnostic-only.log`:
  PASS.
- `FeatherCore/build/logs/build-ble1-socket-fallback-diagnostic-only.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-socket-fallback-diagnostic-only.log`:
  PASS.
- `FeatherCore/build/logs/run-socket-fallback-diagnostic-only-core.log`:
  PASS.
- `FeatherCore/build/bt-hwsim-socket-fallback-diagnostic-only-core/`:
  artifact directory.

Validated cases:

```text
PASS bluez-basic-upstream-convergence-closeout
PASS bluez-le-audio-umbrella
```

Required markers remain:

```text
hci-sock-fallback-compiled=0
l2cap-sock-fallback-compiled=0
iso-sock-fallback-compiled=0
```

Current boundary:

- This closes the ordinary HCI/L2CAP/ISO staging socket fallback path for the
  validated hwsim configuration.
- The remaining no-glue work is now focused on snapshot/event bookkeeping,
  profile object/mainloop ownership, real-controller HCI transport validation,
  and lower-half/helper boundaries.

## 2026-06-19 HCI sim status mirror wording

The hwsim HCI connect probes no longer expose the sim-side cache as
`snapshot=1`.  The cache is now documented in logs as diagnostic status mirror
state:

```text
sim-status-mirror=diagnostic
```

Validation artifacts:

- `FeatherCore/build/logs/build-bt1-hci-sim-status-diagnostic.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hci-sim-status-diagnostic.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hci-sim-status-diagnostic.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hci-sim-status-diagnostic.log`: PASS.
- `FeatherCore/build/logs/run-hci-sim-status-diagnostic-core.log`: PASS.
- `FeatherCore/build/bt-hwsim-hci-sim-status-diagnostic-core/`: artifact
  directory.

Validated cases:

```text
PASS hci-bredr-medium
PASS hci-le-lifecycle
PASS bluez-basic-upstream-convergence-closeout
```

Current boundary:

- HCI owner evidence remains `hci-owner-path=1` plus command/event path and
  upstream connection markers.
- The hwsim mirror is diagnostic only; it should not be used as proof of Linux
  HCI object ownership.

## 2026-06-19 HCI validator forbids snapshot owner markers

The hwsim validator now makes the HCI diagnostic mirror boundary mandatory.

Required current marker:

```text
sim-status-mirror=diagnostic
```

Forbidden old markers:

```text
snapshot=1
sim-fastpath=1
```

Validation artifacts:

- `FeatherCore/build/logs/run-hci-sim-status-validator-forbidden.log`: PASS.
- `FeatherCore/build/bt-hwsim-hci-sim-status-validator-forbidden/`: artifact
  directory.

Validated cases:

```text
PASS hci-bredr-medium
PASS hci-le-lifecycle
PASS bluez-basic-upstream-convergence-closeout
```

Current boundary:

- HCI/basic hwsim logs can no longer pass with the old snapshot-owner wording.
- The broader no-glue target still requires the same treatment for remaining
  profile object ownership, lower-half helper state, and real-controller paths.

## 2026-06-19 Profile native-only forbidden markers

Profile hwsim validation now rejects additional fallback data-plane evidence.

Forbidden markers:

```text
snapshot=1
sim-fastpath=1
native-path=0
fallback-ret=0
```

Current ISO connect marker:

```text
upstream-iso-connect: ... hci-owner-path=1 sim-status-mirror=diagnostic
```

Payload transfer must continue proving native ownership, for example:

```text
upstream-iso-send: ... native-path=1 sim-fastpath=0 upstream-iso-attach=1
```

Validation artifacts:

- `FeatherCore/build/logs/build-bt1-profile-native-only-forbidden.log`: PASS.
- `FeatherCore/build/logs/build-bt2-profile-native-only-forbidden.log`: PASS.
- `FeatherCore/build/logs/build-ble1-profile-native-only-forbidden.log`: PASS.
- `FeatherCore/build/logs/build-ble2-profile-native-only-forbidden.log`: PASS.
- `FeatherCore/build/logs/run-profile-native-only-forbidden.log`: PASS.
- `FeatherCore/build/bt-hwsim-profile-native-only-forbidden/`: artifact
  directory.

Validated cases:

```text
PASS bluez-basic-upstream-convergence-closeout
PASS bluez-le-audio-umbrella
```

Current boundary:

- The validated basic and LE Audio profile paths cannot pass with native-path
  failure hidden behind a successful fallback return.
- Remaining profile families still need the same treatment as their local glue
  is removed.

## 2026-06-19 BLE 6LoWPAN/IPSP forbidden owner/fallback gate

BLE 6LoWPAN/IPSP validation now rejects old bridge-only and TX fallback
evidence.

Forbidden markers:

```text
tx-fallback=<non-zero>
upstream-owner=bridge-ipsp
```

Required owner evidence remains:

```text
upstream-owner=net_bluetooth/6lowpan+sim-ipsp-datapath-owner
upstream-owner=linux-6lowpan-ipsp-helper
upstream-iphc-owner=net_6lowpan/iphc
tx-iphc-error=0
```

Validation artifacts:

- `FeatherCore/build/logs/run-6lowpan-forbidden-owner-fallback.log`: PASS.
- `FeatherCore/build/bt-hwsim-6lowpan-forbidden-owner-fallback/`: artifact
  directory.

Validated cases:

```text
PASS ble-ip-ping
PASS bluez-ipsp-closeout-full
PASS bluez-daemon-ipsp-closeout-full
```

Current boundary:

- BLE IPSP/6LoWPAN hwsim gates cannot pass with the old bridge-only owner or
  non-zero TX fallback.
- The NuttX-facing bridge still drives some helper/object hooks; deeper
  upstream convergence remains moving those hooks into direct imported
  `net/bluetooth/6lowpan.c` object ownership.

## 2026-06-19 generic non-zero fallback forbidden in hwsim validator

The hwsim validator now treats any generic `fallback=<non-zero>` marker as a
hard failure.  Passing paths may still report `fallback=0` to prove the imported
Linux/BlueZ-style owner handled the operation, but non-zero fallback execution
can no longer pass silently.

Validation:
- `FeatherCore/build/logs/run-generic-fallback-forbidden-core.log`: PASS.
- `FeatherCore/build/bt-hwsim-generic-fallback-forbidden-core/`: artifacts for
  HCI BR/EDR, HCI LE lifecycle, basic BlueZ closeout, LE Audio umbrella,
  IPSP daemon/full closeout, and BLE IPv6 ping.
- Negative scan found no non-zero `fallback=`, no `forbidden:`, no `snapshot=1`,
  no `sim-fastpath=1`, no `native-path=0`, no `fallback-ret=0`, no non-zero
  `tx-fallback=`, and no `upstream-owner=bridge-ipsp`.

Remaining no-glue boundary:
- This hardens the runnable fallback gate but does not remove the remaining
  BlueZ profile compatibility layers.  Active profile convergence still needs
  the A2DP compat owner, Network/IPSP adapters, and the remaining profile
  harnesses to be replaced by direct upstream daemon/mainloop/D-Bus/session
  ownership.

Build follow-up:
- `FeatherCore/build/logs/build-bt1-generic-fallback-forbidden.log`: PASS.
- `FeatherCore/build/logs/build-bt2-generic-fallback-forbidden.log`: PASS.
- `FeatherCore/build/logs/build-ble1-generic-fallback-forbidden.log`: PASS.
- `FeatherCore/build/logs/build-ble2-generic-fallback-forbidden.log`: PASS.

The build checks were run sequentially because NuttX sim defconfig generation
uses shared `.config` state and parallel configuration races with itself.

## 2026-06-19 A2DP compat owner claims demoted to diagnostic-only bridge

A2DP no longer lets `bluez/upstream_a2dp_compat.c` present its ledgers as direct
upstream object ownership.  The compat unit is still used as a runnable probe
surface, but the runtime evidence now names it honestly as
`compat-boundary=diagnostic-only` and uses `*-diagnostic-bridge` ownership
markers.

Validation:
- `FeatherCore/build/logs/build-bt1-a2dp-compat-diagnostic-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-compat-diagnostic-owner.log`: PASS.
- `FeatherCore/build/logs/run-a2dp-compat-diagnostic-owner-v2.log`: PASS.
- `FeatherCore/build/bt-hwsim-a2dp-compat-diagnostic-owner-v2/`: artifacts for
  A2DP upstream session, upstream transactions, full daemon closeout, A2DP
  upstream convergence closeout, and basic upstream convergence closeout.
- Negative scan found no old `ownership=*-to-upstream-object` A2DP compat owner
  claims and no validator forbidden output.

Remaining boundary:
- A2DP still is not fully glue-free.  The compat bridge is now diagnostic-only
  evidence, not an owner claim.  The next step is replacing that bridge with
  direct upstream BlueZ audio plugin/mainloop/D-Bus/session object ownership and
  then forbidding the bridge marker itself.

## 2026-06-19 A2DP compat runtime owner tags renamed to diagnostic tags

A2DP compat closeout lines no longer use `upstream-*-owner` runtime tags.  The
compat probe now emits `upstream-*-diagnostic` tags, and the validator rejects
old A2DP compat owner tags if they reappear.

Validation:
- `FeatherCore/build/logs/build-bt1-a2dp-compat-diagnostic-tags.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-compat-diagnostic-tags.log`: PASS.
- `FeatherCore/build/logs/run-a2dp-compat-diagnostic-tags.log`: PASS.
- `FeatherCore/build/bt-hwsim-a2dp-compat-diagnostic-tags/`: artifacts for
  A2DP upstream session, upstream transactions, full daemon closeout, A2DP
  upstream convergence closeout, and basic upstream convergence closeout.
- Negative scan found no old A2DP compat `upstream-*-owner` tags, no old
  `ownership=*-to-upstream-object` claims, and no validator forbidden output.

Remaining boundary:
- The compat surface is now explicitly diagnostic, but it is still present.
  Full A2DP no-glue requires replacing this bridge with direct imported BlueZ
  audio plugin/mainloop/D-Bus/session ownership and then forbidding the
  `compat-boundary=diagnostic-only` marker itself.

## 2026-06-19 A2DP default closeout no longer emits compat diagnostic bridge

The normal A2DP closeout path no longer emits the compat diagnostic bridge.  The
active validator now forbids `compat-boundary=diagnostic-only`, so A2DP default
closeout cannot pass by relying on that bridge marker.

Validation:
- `FeatherCore/build/logs/build-bt1-a2dp-compat-default-removed.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-compat-default-removed.log`: PASS.
- `FeatherCore/build/logs/run-a2dp-compat-default-removed.log`: PASS.
- `FeatherCore/build/bt-hwsim-a2dp-compat-default-removed/`: artifacts for A2DP
  upstream session, upstream transactions, full daemon closeout and current
  complete closeout.
- Negative scan found no `compat-boundary=diagnostic-only`, no old A2DP compat
  owner tags or owner claims, and no fallback/snapshot/native-path regression
  markers.

Important validator note:
- The active validator was repaired after an overly broad A2DP diagnostic block
  removal, and it is syntactically valid for the A2DP gates above.  Later
  profile/LE Audio validator requirements still need restoration from a reliable
  source before this can be treated as full global hwsim coverage.

Remaining boundary:
- This removes A2DP compat diagnostic bridge evidence from the default closeout
  path.  Full no-glue still requires direct upstream BlueZ audio plugin objects
  to replace the remaining compatibility helper implementation and full
  restoration of validator coverage for all later BT/BLE profiles.

## 2026-06-19 validator coverage guard for truncated profile/LE Audio cases

The validator now fails loudly when a runner-supported case is missing validator
coverage.  This prevents the current truncated validator from being mistaken for
full BT/BLE hwsim proof while the missing profile/LE Audio requirement bodies
are restored.

Validation:
- `validate-bt-hwsim-usecases.py --list` reports the 215 cases supported by the
  runner.
- Before the restoration below, selecting `bluez-le-audio-umbrella` failed with
  `missing validator case coverage`, which proved the guard was blocking false
  success for missing LE Audio coverage.
- `FeatherCore/build/logs/run-validator-coverage-guard-a2dp.log`: PASS for the
  still-covered A2DP closeout gates.
- Negative scan found no A2DP compat diagnostic bridge or fallback/snapshot
  regression markers in the A2DP guard artifacts.

Remaining boundary:
- Full global hwsim proof is still blocked on restoring concrete validator
  requirements for the later missing runner cases, especially the remaining
  BlueZ profile closeouts.

## 2026-06-19 LE Audio umbrella validator coverage restored

`bluez-le-audio-umbrella` now has concrete validator requirements again.  The
case is no longer just an expected runner name protected by the missing-coverage
guard.

The restored validator requires both BLE roles to prove:

- `bluetoothd` audio plugin init for BAP/PACS/ASCS/CAP/BASS/VCP/MICP/CSIP/MCP/TMAP/CCP/GMAP.
- `org.bluez` object-manager ownership and PACS, ASCS, BAP, and
  `org.bluez.MediaTransport1` D-Bus objects owned by `bluetoothd`.
- BlueZ-style mgmt socket power-on, LE CIS established event, BAP attach,
  ATT/GATT ASCS enable transaction, MediaEndpoint registration,
  MediaTransport acquire/release, and daemon/mainloop cleanup.
- Native ISO socket dataplane evidence with `upstream-af: selected=1`, no
  compiled HCI/L2CAP/ISO socket fallback, ISO connect count `4`, native ISO
  receive evidence, and `upstream-iso-write: payload-len=20 send-ret=20`.

Validation:

- `python3 -m py_compile FeatherCore/tools/firmware/sim/validate-bt-hwsim-usecases.py`:
  PASS.
- `FeatherCore/build/bt-hwsim-generic-fallback-forbidden-core/`: PASS for
  `bluez-le-audio-umbrella` on `ble1` and `ble2`.
- `FeatherCore/build/bt-hwsim-le-audio-umbrella-validator-restored/`: PASS for
  `bluez-le-audio-umbrella` on `ble1` and `ble2`.
- Negative scan across the fresh artifact and
  `FeatherCore/build/logs/run-le-audio-umbrella-validator-restored.log` found
  no validator `FAIL`, no `missing validator case coverage`, no
  `compat-boundary=diagnostic-only`, no `snapshot=1`, no `sim-fastpath=1`, no
  `native-path=0`, no non-zero `fallback=`, no `fallback-ret=0`, and no
  `iso-sock-fallback-compiled=1`.

Remaining boundary:

- LE Audio umbrella is restored as a concrete strong validator gate, but the
  whole validator is still not globally complete until the later missing
  BlueZ profile/closeout cases receive concrete `CaseCheck` requirements too.
- Current validator coverage accounting after the A2DP bidirectional teardown
  restoration below is `expected=215`, `covered=150`, `missing=65`.

## 2026-06-19 A2DP bidirectional teardown validator coverage restored

`bluez-a2dp-transport-bidir-teardown` now has concrete validator coverage again.
The restored gate also caught and fixed a runner-script regression: the freshly
generated case had degraded to only BT1 source teardown plus BT2 sink/response,
while the intended gate is bidirectional.

What changed:

- `test-bt-hwsim-usecases.sh` now generates the original bidirectional flow:
  BT1 source -> BT2 sink with suspend/close, then BT2 source -> BT1 sink with
  suspend/close.
- `validate-bt-hwsim-usecases.py` now requires both BT roles to show AVDTP
  `OPEN->STREAMING`, MediaTransport acquire/read/release,
  `STREAMING->OPEN`, `OPEN->IDLE`, native HCI connection evidence, L2CAP
  socket counters, and zero native L2CAP attach/receive failures.

Validation:

- `python3 -m py_compile FeatherCore/tools/firmware/sim/validate-bt-hwsim-usecases.py`:
  PASS.
- `FeatherCore/build/bt-hwsim-usecases-bluez-a2dp-bidir-teardown-validator/`:
  PASS for `bluez-a2dp-transport-bidir-teardown` on `bt1` and `bt2`.
- `FeatherCore/build/bt-hwsim-a2dp-bidir-teardown-validator-restored-v2/`:
  PASS for `bluez-a2dp-transport-bidir-teardown` on `bt1` and `bt2`.
- `FeatherCore/build/logs/run-a2dp-bidir-teardown-validator-restored-v2.log`:
  runner PASS with `RUN-DONE` and validator PASS.
- Negative scan of the fresh artifact found no validator `FAIL`, no
  `missing validator case coverage`, no `compat-boundary=diagnostic-only`, no
  `snapshot=1`, no `sim-fastpath=1`, no `native-path=0`, no non-zero
  `fallback=`, no `fallback-ret=0`, and no
  `l2cap-sock-fallback-compiled=1`.

Status:

- This restores the A2DP bidirectional MediaTransport teardown gate and removes
  it from the missing-coverage bucket.
- Current validator coverage accounting is `expected=215`, `covered=150`,
  `missing=65`.

## 2026-06-19 A2DP upstream owner fields demoted to link-only validator gate

The A2DP upstream convergence closeout path no longer reports its upstream probe
and compatibility evidence as active `upstream-owner=` records.  A2DP manifest,
audio-link, media-object, transport-object, and compat logs now use
`upstream-link=` / `upstream-link-ledger` for linkage diagnostics, while the
validator requires real A2DP closeout behavior through BlueZ-style daemon,
AVDTP, media transport, L2CAP socket, and cleanup evidence.

Validation updates:
- `bluez-a2dp-upstream-convergence-closeout` has concrete validator coverage.
- A passing run must show source and sink A2DP closeout, upstream manifest,
  AVDTP/audio/media transport probes, D-Bus media ownership, static handler
  linkage, object lifecycle cleanup, error policy cleanup, L2CAP socket status,
  and zero native attach/recv failures.
- The validator rejects old A2DP `upstream-owner=` output from daemon/tool and
  upstream object/handler logs.

Fresh evidence:
- `FeatherCore/build/logs/build-bt1-a2dp-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-a2dp-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/run-a2dp-upstream-link-no-owner.log`: runner PASS.
- `FeatherCore/build/bt-hwsim-a2dp-upstream-link-no-owner/`: validator PASS.
- Negative scan found no forbidden A2DP owner, fallback, snapshot, sim-fastpath,
  diagnostic-boundary, or L2CAP fallback-compiled regressions.

Current validator coverage accounting is `expected=215`, `covered=151`,
`missing=64`.

Remaining work: continue the same demotion/removal pattern for the non-A2DP
profile families that still use active `upstream-owner=` evidence, but only after
each family has equivalent hwsim validator proof for the real Linux/BlueZ
semantic path.

## 2026-06-19 LE Audio closeout owner fields demoted to link-only validator gate

`bluezleaudio closeout source|sink` now reports closeout linkage with
`upstream-link=` and `closeout upstream-link-ledger` instead of active
`upstream-owner=` / `upstream-ownership-ledger` wording.  The semantic evidence is
still retained: BAP/PACS/ASCS cleanup, MediaEndpoint/MediaTransport cleanup,
ATT/GATT request drain, ISO fd closure, source-parity, and final-ok state.

Validator updates:
- `bluez-le-audio-umbrella` now requires the link ledger and final LE Audio
  source/sink closeout lines.
- The validator rejects old `bluez-leaudio:` owner wording if it reappears.
- The existing HCI/L2CAP/ISO fallback-compiled gates remain required.

Fresh evidence:
- `FeatherCore/build/logs/build-ble1-leaudio-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-leaudio-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/run-leaudio-upstream-link-no-owner.log`: runner PASS.
- `FeatherCore/build/bt-hwsim-leaudio-upstream-link-no-owner/`: validator PASS.
- Negative scan found no old LE Audio owner markers and no fallback/snapshot/
  sim-fastpath/diagnostic-boundary regressions.

Current validator coverage accounting remains `expected=215`, `covered=151`,
`missing=64` because this tightened an existing LE Audio case rather than adding
new coverage.

## 2026-06-19 GATT upstream closeout owner fields demoted to link-only validator gate

`bluezaudio le-gatt-upstream closeout` and `bluezgatt closeout source|sink` now
report GATT/ATT convergence with `upstream-link=` and `upstream-link-ledger`
instead of active `upstream-owner=` / `upstream-ownership-ledger` wording.

Validator updates:
- `bluez-gatt-upstream-convergence-closeout` now has concrete BLE1/BLE2 coverage.
- A passing run must prove ATT fixed-channel open/connect, MTU/read/write
  request-response evidence, peer read/write, notify, indicate, security/error
  contract, cleanup, source parity, and final-ok state.
- The validator rejects old GATT owner wording and old GATT staged-boundary tags.

Fresh evidence:
- `FeatherCore/build/logs/build-ble1-gatt-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-gatt-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/run-gatt-upstream-link-no-owner-v2.log`: runner PASS.
- `FeatherCore/build/bt-hwsim-gatt-upstream-link-no-owner-v2/`: validator PASS.
- Negative scan found no old GATT owner/staged-boundary markers and no fallback,
  snapshot, sim-fastpath, diagnostic-boundary, or HCI/L2CAP fallback-compiled
  regressions.

Current validator coverage accounting is `expected=215`, `covered=152`,
`missing=63`.

## 2026-06-19 HID/HOGP upstream closeout owner fields demoted to link-only validator gate

`bluezhid closeout classic-host|classic-device|hogp-host|hogp-device` now reports
Classic HIDP and BLE HOGP convergence with `upstream-link=` and
`upstream-link-ledger` instead of active `upstream-owner=` /
`upstream-ownership-ledger` wording.

Validator updates:
- `bluez-hid-upstream-convergence-closeout` now has concrete BT1/BT2/BLE1/BLE2
  coverage.
- A passing run must prove Classic HID control/interrupt L2CAP open/connect,
  HIDP request-response evidence, BLE HOGP ATT/GATT report read/write/notify,
  cleanup, source parity, and final-ok state.
- The validator rejects old HID/HOGP owner wording and staged-boundary tags.

Fresh evidence:
- `FeatherCore/build/logs/build-bt1-hid-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hid-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hid-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hid-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/run-hid-upstream-link-no-owner.log`: runner PASS.
- `FeatherCore/build/bt-hwsim-hid-upstream-link-no-owner/`: validator PASS.
- Negative scan found no old HID/HOGP owner/staged-boundary markers and no
  fallback, snapshot, sim-fastpath, diagnostic-boundary, or HCI/L2CAP
  fallback-compiled regressions.

Current validator coverage accounting is `expected=215`, `covered=153`,
`missing=62`.

## 2026-06-19 HFP/HSP upstream closeout owner fields demoted to link-only validator gate

`bluezhfp closeout hfp-hf|hfp-ag|hsp-hs|hsp-ag` now reports HFP/HSP convergence
with `upstream-link=` and `upstream-link-ledger` instead of active
`upstream-owner=` / `upstream-ownership-ledger` wording.

Validator updates:
- `bluez-hfp-hsp-profile-closeout` now has concrete BT1/BT2 coverage.
- A passing run must prove RFCOMM open/connect, AT request-response evidence,
  SCO-like audio payload, audio transport lifecycle, cleanup, source parity, and
  final-ok state for HFP and HSP roles.
- The validator rejects old HFP/HSP owner wording and staged-boundary tags.

Fresh evidence:
- `FeatherCore/build/logs/build-bt1-hfp-hsp-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hfp-hsp-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/run-hfp-hsp-upstream-link-no-owner.log`: runner PASS.
- `FeatherCore/build/bt-hwsim-hfp-hsp-upstream-link-no-owner/`: validator PASS.
- Negative scan found no old HFP/HSP owner/staged-boundary markers and no
  fallback, snapshot, sim-fastpath, diagnostic-boundary, or HCI/L2CAP
  fallback-compiled regressions.

Current validator coverage accounting is `expected=215`, `covered=154`,
`missing=61`.

## 2026-06-19 Network/BNEP upstream owner fields demoted to link-only validator gate

`blueznetwork closeout-full` and `bluezbneptest native-closeout` now report BT
Network/BNEP convergence with `upstream-link=` and `link-ledger=` instead of
active `upstream-owner=` / `ownership-ledger=` wording.  The NuttX native BNEP
status path also reports `upstream-link=bluez-fd-to-imported-bnep`.

Validator updates:
- `bluez-network-closeout-full` now requires the link-only Network/BNEP closeout
  evidence.
- A passing run still proves PANU/NAP/GN role coverage, BNEPCONNADD/CONNDEL,
  btn0 ping including 1400-byte payload, native BNEP netdev/session/ioctl
  counters, cleanup, and zero staging fallback.
- The validator rejects old Network/BNEP owner wording and old bneptest native
  ownership-ledger output.

Fresh evidence:
- `FeatherCore/build/logs/build-bt1-network-bnep-upstream-link-no-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-network-bnep-upstream-link-no-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/run-network-bnep-upstream-link-no-owner-v2.log`:
  runner PASS.
- `FeatherCore/build/bt-hwsim-network-bnep-upstream-link-no-owner-v2/`:
  validator PASS.
- Negative scan found no old Network/BNEP owner markers and no fallback,
  snapshot, sim-fastpath, diagnostic-boundary, or BNEP staging fallback
  regressions.

Current validator coverage accounting remains `expected=215`, `covered=154`,
`missing=61` because this tightened an existing Network/BNEP case rather than
adding new coverage.

## 2026-06-19 BLE 6LoWPAN/IPSP upstream owner fields demoted to link-only validator gate

The BLE 6LoWPAN/IPSP closeout path now reports the imported Linux-style
6LoWPAN/IPSP handoff with `upstream-link=`, `upstream-link-ledger`, and
`upstream-helper-link=` instead of active `upstream-owner=`,
`upstream-ownership-ledger`, and `upstream-helper-owner=` wording.

Validator updates:
- `bluez-daemon-ipsp-closeout-full` now requires the link-only 6LoWPAN/IPSP
  closeout evidence on both BLE1 and BLE2.
- A passing run still proves L2CAP CoC open/close, IPHC TX/RX, netdev lifecycle,
  peer lifecycle, IPSP state transitions, cleanup, and ping/data-path evidence.
- The validator rejects old 6LoWPAN/IPSP owner/helper-owner/ownership-ledger
  wording and old BlueZ IPSP owner tags.

Fresh evidence:
- `FeatherCore/build/logs/build-ble1-6lowpan-ipsp-upstream-link-no-owner-v2-serial.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-6lowpan-ipsp-upstream-link-no-owner-v2-serial.log`:
  PASS.
- `FeatherCore/build/logs/run-6lowpan-ipsp-upstream-link-no-owner-v2.log`:
  runner PASS.
- `FeatherCore/build/bt-hwsim-6lowpan-ipsp-upstream-link-no-owner-v2/`:
  validator PASS.
- Negative scan found no old 6LoWPAN/IPSP owner/helper-owner markers and no
  fallback, snapshot, sim-fastpath, diagnostic-boundary, or BlueZ IPSP active
  owner regressions.

Current validator coverage accounting remains `expected=215`, `covered=154`,
`missing=61` because this tightened an existing 6LoWPAN/IPSP case rather than
adding new coverage.

Note: NuttX sim builds that refresh generated Kconfig state should be run
serially; parallel BLE1/BLE2 builds can race in `mkkconfig`.

## 2026-06-19 HCI raw socket ABI owner fields demoted to link-only validator gate

`bluezhciraw socket-abi-closeout` now reports HCI raw/user/monitor/control/
logging socket convergence with `upstream-link=` and `socket-abi link-ledger`
instead of active `upstream-owner=` and `socket-abi ownership-ledger` wording.

Validator updates:
- `bluez-hci-mgmt-socket-closeout-full` now requires the link-only HCI raw
  socket ABI evidence on BLE1/BLE2.
- A passing run still proves HCI raw/user/monitor/control/logging bind paths,
  filter setup, sendmsg/recvmsg, monitor fanout, command complete/status,
  advertising, scanning, and ISO setup evidence.
- The validator rejects old `bluez-hciraw` `upstream-owner=` and
  `socket-abi ownership-ledger` output.

Fresh evidence:
- `FeatherCore/build/logs/build-ble1-hciraw-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hciraw-upstream-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/run-hciraw-upstream-link-no-owner-v2.log`: runner
  PASS.
- `FeatherCore/build/bt-hwsim-hciraw-upstream-link-no-owner-v2/`: validator
  PASS.
- Negative scan found no old HCI raw owner/ownership-ledger markers and no
  fallback, snapshot, sim-fastpath, diagnostic-boundary, or missing-coverage
  regressions.

Current validator coverage accounting remains `expected=215`, `covered=154`,
`missing=61` because this tightened an existing HCI/MGMT case rather than adding
new coverage.

## 2026-06-19 HCI/MGMT/basic owner glue closeout gate

Task 1 of the seven-task closeout plan is complete.  HCI raw socket ABI,
BlueZ mgmt daemon/bootstrap/security, shared mgmt control-fd, and basic BT/BLE
daemon convergence now use link-only closeout wording in the validated gates.

Validator updates:
- `bluez-hci-mgmt-socket-closeout-full` requires link-only HCI raw and BlueZ
  mgmt evidence.
- `bluez-basic-upstream-convergence-closeout` requires link-only basic daemon
  convergence evidence.
- The validator rejects old `bluez-hciraw`, `bluez-mgmt`, and
  `bluezdaemon-basic` owner/ownership-ledger output.

Fresh evidence:
- `FeatherCore/build/logs/build-bt1-hci-mgmt-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-bt2-hci-mgmt-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble1-hci-mgmt-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/build-ble2-hci-mgmt-link-no-owner.log`: PASS.
- `FeatherCore/build/logs/run-hci-mgmt-link-no-owner-final.log`: runner PASS.
- `FeatherCore/build/bt-hwsim-hci-mgmt-link-no-owner-final/`: validator PASS.
- Negative scan found no old HCI raw, BlueZ mgmt, or basic daemon owner markers
  and no fallback, snapshot, sim-fastpath, diagnostic-boundary, or
  missing-coverage regressions.

Current validator coverage accounting remains `expected=215`, `covered=154`,
`missing=61` because this tightened existing gates rather than adding new
coverage.

## 2026-06-19 profile families and generic profile_main link-only closeout

The profile-family hwsim gate now covers OBEX, Mesh, ASHA, Print, iAP, MIDI,
Ranging, and AVRCP without active `upstream-owner=` closeout evidence.

Changes:

- OBEX closeout now reports `upstream-link=` / `upstream-link-ledger` for PBAP,
  OPP, MAP, MNS, FTP, Sync, and BIP.
- Generic `bluezprofile closeout` now reports `upstream-link=` and
  `upstream-link-ledger`, so Mesh/ASHA/MIDI/Ranging/Print/iAP profile evidence
  no longer leaks old owner wording.
- AVRCP controller-side sequential hwsim roles no longer fail just because the
  peer target response is validated in the separate target role.  Target logs
  still prove request receive and response generation.
- The validator now has concrete coverage for the 10 previously missing profile
  family closeout cases and rejects old OBEX/profile owner output.

Validation:

- `FeatherCore/build/logs/build-bt1-profile-families-link-no-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-bt2-profile-families-link-no-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-ble1-profile-families-link-no-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/build-ble2-profile-families-link-no-owner-v2.log`:
  PASS.
- `FeatherCore/build/logs/run-profile-families-link-no-owner-v2.log`: PASS for
  all 23 selected cases.
- `FeatherCore/build/bt-hwsim-profile-families-link-no-owner-v2/`: hwsim
  artifact for this gate.

Next convergence gate:

- daemon/basic/bootstrap/security/reconnect lifecycle owner glue.
- remaining validator missing coverage and global forbidden expansion before
  final full-suite build/hwsim proof.

## 2026-06-19 daemon lifecycle link-ledger closeout

Daemon-side lifecycle evidence has been demoted from owner-ledger wording to
link-only ledger wording for the validated basic, A2DP, mgmt, reconnect, and
profile-family hwsim paths.

Changes:

- `bluez-daemon: ... closeout ownership-ledger` is now
  `bluez-daemon: ... closeout link-ledger`.
- A2DP daemon closeout now uses `upstream-daemon-link-ledger`.
- Pairing matrix closeout now uses `pairing-matrix link-ledger`.
- Validator forbidden rules reject the old daemon ownership-ledger forms.

Validation:

- `FeatherCore/build/logs/build-bt1-daemon-lifecycle-link-ledger.log`: PASS.
- `FeatherCore/build/logs/build-bt2-daemon-lifecycle-link-ledger.log`: PASS.
- `FeatherCore/build/logs/build-ble1-daemon-lifecycle-link-ledger.log`: PASS.
- `FeatherCore/build/logs/build-ble2-daemon-lifecycle-link-ledger.log`: PASS.
- `FeatherCore/build/logs/run-daemon-lifecycle-link-ledger.log`: PASS for all
  selected daemon/basic/mgmt/A2DP/profile-family closeout cases.
- `FeatherCore/build/bt-hwsim-daemon-lifecycle-link-ledger/`: hwsim artifact.

Next gate:

- Complete validator missing coverage accounting by either adding concrete
  validators for the remaining expected cases or removing obsolete expected
  cases before the final full-suite proof.

## 2026-06-20 A2DP L2CAP connected-state socket option evidence

The A2DP closeout now includes connected-state L2CAP socket option evidence from
the daemon-owned L2CAP handles.  This is intentionally not a temporary BlueZ-side
shortcut: the probe runs through the socket ops used by the current
Linux-Bluetooth upstream/NuttX path.

Validated behavior:

- Pre-connect AVDTP/AVRCP/media sockets still prove `L2CAP_OPTIONS` and
  `L2CAP_LM` set/get behavior.
- After connect, `L2CAP_OPTIONS` returns Linux-aligned `-EINVAL`.
- `BT_MODE` follows the upstream Linux ECRED gate.  Current hwsim has ECRED
  disabled, so the Linux-compatible result is `-ENOPROTOOPT` with
  `bt-mode-gate=ecred-disabled`; an ECRED-enabled configuration would continue
  into the upstream `BT_BOUND` state check.
- `L2CAP_CONNINFO` remains outside this gate until real `l2cap_conn` ownership
  is safe; the log explicitly records
  `conninfo-skip=needs-real-l2cap-conn` rather than faking a handle.

Validation artifacts:

- `FeatherCore/build/logs/build-bt1-l2cap-connected-options.log`: PASS.
- `FeatherCore/build/logs/build-bt2-l2cap-connected-options.log`: PASS.
- `FeatherCore/build/logs/build-ble1-l2cap-connected-options.log`: PASS.
- `FeatherCore/build/logs/build-ble2-l2cap-connected-options.log`: PASS.
- `FeatherCore/build/logs/run-l2cap-connected-options.log`: PASS for A2DP
  closeout and LE Audio BAP/PACS/ASCS.
- `FeatherCore/build/logs/validate-l2cap-connected-options.log`: PASS.

## 2026-06-20 L2CAP_CONNINFO boundary after negative hwsim audit

The A2DP connected-state L2CAP option gate intentionally keeps
`conninfo-skip=needs-real-l2cap-conn`.

Two stronger implementations were tried and rejected because they did not meet
the no-temporary-patch standard:

- Per-socket `l2cap_chan` creation plus hwsim `l2cap_conn` attach during connect.
- HCI connection hash backed `L2CAP_CONNINFO` lookup.

Both approaches built, but both caused `bluez-a2dp-current-complete-closeout` to
exit early before the connected-option probe finished.  The failing logs are:

- `FeatherCore/build/logs/run-l2cap-conninfo-real-chan.log`.
- `FeatherCore/build/logs/run-l2cap-conninfo-hciconn.log`.

The retained passing gate is:

- `FeatherCore/build/logs/run-l2cap-connected-options.log`: PASS for A2DP
  closeout and LE Audio BAP/PACS/ASCS.
- `FeatherCore/build/logs/validate-l2cap-connected-options.log`: PASS.

This means BlueZ-facing connected-state `L2CAP_OPTIONS` and upstream ECRED-gated
`BT_MODE` semantics are validated, but full `L2CAP_CONNINFO` parity still
requires real upstream-compatible `l2cap_chan` / `l2cap_conn` ownership before it
can become a passing validator requirement.
