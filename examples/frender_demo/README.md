# FRender Demo

`frender_demo` validates the first-stage FRender boundary.

It does not submit a generic command list to `nuttx/graphics`.  Instead, it:

- Builds an FRender command list.
- Prints software and NuttX graphics capability snapshots.
- Executes commands through the software backend.
- Prints an RGBA8888 checksum.

Expected NSH usage:

```text
nsh> frender_demo
```
