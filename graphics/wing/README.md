# WING GUI

WING GUI is the GUI library layer for Feather.

It is intentionally separate from WING Desktop:

- WING GUI provides drawing primitives, surfaces, context state, and later UI widgets/layout.
- WING Desktop is an optional desktop environment built on top of WING GUI.
- A WING GUI application can run directly from NSH or another launcher without WING Desktop.
- A WING GUI application only needs extra desktop packaging when it wants to be installed, launched, and managed inside WING Desktop.

Current scope:

- Software RGBA8888 surface.
- Minimal immediate drawing context.
- Rectangle clear/fill/draw primitives.
- A small demo scene helper used by `examples/wing_gui_demo`.

Next backend steps:

- Add a NuttX NX/framebuffer presentation backend.
- Add input event adaptation.
- Add retained UI tree/layout/widgets above this drawing layer.
