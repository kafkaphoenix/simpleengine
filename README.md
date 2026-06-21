# A simple engine
Modern C++23 OpenGL4.6 engine that can be used as a starting point for more complex engines or games. It focuses on clarity and a small feature set while keeping modern practices.

## Engine Features
- OpenGL forward renderer with static instancing (mesh/material batching), animated model support, frustum culling and post-processing.
- Diffuse lighting with multiple light sources.
- Third-person and first-person camera controllers.
- Basic stats display.
- Simple event system for input handling and window events.
- Asset manager with caching using lightweight handles.
- Configuration system using TOML files.

For a more detailed look at the engine’s architecture, check out the [game-engine](https://kafkaphoenix.github.io/categories/game-engine/) blog section and the [architecture](docs/architecture.md).

## Build & Run

- Configure: `make configure`
- Build: `make build`
- Run: `make run`
- RenderDoc: `make renderdoc`
- Static analysis: `make tidy`
- Format: `make format`

See the [build instructions](docs/build.md) for detailed steps on how to build and run the engine.

## Demo Sponza scene

A demo scene is included with the Sponza model and an animated fox as the player character.

### Controls
- WASD: Move
- Left Shift: Run
- Mouse: Look
- V: Toggle camera mode (first/third person)
- F3: Wireframe toggle
- F4: Cycle post-process effect (none, tone map, inversion, grayscale, sharpen, blur, edge detect)
- F5: Cycle stats mode (basic/timing averages)
- F6: Cycle material debug view (lit, normals, albedo, NdotL, roughness, metallic, occlusion, normal map,
    linear depth, material ID, OIT revealage, emissive, shadow placeholder)
- F12: Toggle fullscreen
- Esc: Quit

## Runtime Config

Settings live in `config.toml` and are grouped by subsystem:

- `window`: title, size, position, vsync, mode
- `player`: spawn position
- `characterController`: walk/run speeds, mouse sensitivity/smoothing, fixed-step options
- `camera`: projection settings (FOV, near/far, aspect)
- `cameraController`: camera mode (first/third person) and related parameters
- `render`: MSAA sample count and anisotropy level
- `postProcess`: exposure
- `stats`: enable flag and update interval

<table>
  <tr>
    <td align="center">
      <img src="docs/img/sponza.png" width="100%"><br>
      <em>Sponza model</em>
    </td>
    <td align="center">
      <img src="docs/img/animated_model.png" width="100%"><br>
      <em>Animated fox model</em>
    </td>
  </tr>
</table>