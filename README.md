# GLSLTestbed
A Windows only OpenGL 4.6 renderer for testing different rendering techniques & solutions.

![Preview](T_Preview.jpg?raw=true "Preview")

## Implemented Features
- Instanced dynamic batching.
- Frustum culling.
- Light rigid entity component system.
- Update sequencer.
- Opengl object & function wrappers.
- Shader Material/Property block system.
- PBR shading.
- HDR Bloom.
- Tone mapping.
- Ambient Occlusion.
- Multi compile shader variants.
- Octahedron environment maps.
- Batched debug renderer.
- Asset Database.

## Planned Features
- Clustered forward rendering path.
- A global illumination solution.
- Render pass abstraction.
- Depth sorted index queue.
- Color grading.
- Documentation.

## Library Dependencies
- [KTX](https://github.com/KhronosGroup/KTX-Software)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [Glad](https://glad.dav1d.de/)
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [mikktspace](http://www.mikktspace.com/)
- [stb_image](https://github.com/nothings/stb) (To be deprecated by ktx)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
