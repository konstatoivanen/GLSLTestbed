# GLSLTestbed
A Windows only OpenGL 4.6 renderer for testing different rendering techniques & solutions.

![Preview](T_Preview_01.jpg?raw=true "HDRI Preview")
![Preview](T_Preview_02.jpg?raw=true "Fog & Shadows Preview")
![Preview](T_Preview_03.jpg?raw=true "Cascaded Shadow Maps")
![Preview](T_Preview_04.jpg?raw=true "Fog Ambient Anistropy Preview")

## Implemented Features
- Instanced dynamic batching.
- Frustum culling.
- Light rigid entity component system.
- Update sequencer.
- Opengl object & function wrappers.
- Shader material/property block system.
- Clustered forward rendering path.
- Volumetric fog & lighting.
- Variance shadow mapping.
- Light cookie support.
- Point, spot & directional light support.
- PBR shading.
- HDR bloom.
- Tone mapping.
- Ambient occlusion.
- Multi compile shader variants.
- Octahedron environment maps.
- Batched debug renderer.
- Asset hot reloading.

## Planned Features
- A global illumination solution.
- Rectangular area light support.
- Exponential variance shadow maps.
- Color grading.
- Documentation.

## Known Performance Drawbacks
- Matrix buffers are currently mapped per pass. Which is causing a lot of data duplication & some driver stalls.
	- A more optimal solution would be to gather matrices that contribute to the current frame into a single buffer & only build index buffers per pass.
- OpenGL doesn't support multiple command queues or async dispatches. Which leads to otherwise easily multithreadable passes having to be executed consecutively.
- Using sparse textures would probably be more performant in the following scenarios:
	- Shadow map atlas allocation. Currently the entire atlas is allocated in full. As opposed to only commiting sparse tiles for active areas.
	- Shader property instancing currently uses bindless texture handles for material properties. 
	  Creating a sparse texture atlas for them would probably be more cache friendly.
- Culling of for each pass is currently done on the cpu. This could be moved to the gpu instead.

## Asset Sources
- [HDRI Haven](https://hdrihaven.com)
- [CC0 Textures](https://cc0textures.com/)

## Library Dependencies
- [KTX](https://github.com/KhronosGroup/KTX-Software)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [Glad](https://glad.dav1d.de/)
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [mikktspace](http://www.mikktspace.com/)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
