# Pinion

Pinion is FeatherCore's lightweight game engine layer.  It is intended to sit
above the NuttX graphics foundation and provide small game-oriented primitives:
frame timing, surfaces, simple 2D drawing, and eventually sprites, tile maps,
input routing, and scene/ECS integration.

The first NuttX apps integration builds the C core and a framebuffer demo.  NX
window and hardware acceleration backends can be added without changing the
high-level game API.
