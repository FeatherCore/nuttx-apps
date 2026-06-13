# FRender

FRender is Feather's application-side render backend.

It sits below WING GUI and Pinion, and above NuttX graphics public display and
accelerator interfaces.

Current first-stage scope:

- RGBA8888 memory surfaces.
- Backend-neutral command list.
- Capability declarations.
- Software backend for correctness.
- NSH checksum demo through `examples/frender_demo`.

Current non-goals:

- No direct generic command-list submission to `nuttx/graphics`.
- No DMA2D/GPU2D submit yet.
- No framebuffer present adapter yet.
- No WING/Pinion integration yet.

The intended direction is:

```text
WING GUI / Pinion
        |
        v
FRender command list / planner / backend
        |
        v
NuttX graphics public APIs
```
