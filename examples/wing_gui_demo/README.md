# WING GUI Demo

`wing_gui_demo` is the first standalone validation program for WING GUI.

It does not start WING Desktop.  It allocates a software RGBA8888 surface,
renders a simple GUI scene through the WING GUI API, computes a checksum, and
prints the result.

Expected NSH usage:

```text
nsh> wing_gui_demo
```

This keeps the first bring-up step small: WING GUI can be validated before the
NX/framebuffer presentation backend and before the optional WING Desktop layer.
