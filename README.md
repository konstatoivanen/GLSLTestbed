# GLSLTestbed
A Windows only OpenGL 4.6 renderer for testing different rendering techniques & solutions.

![Preview](T_Preview_01.jpg?raw=true "HDRI Preview")
![Preview](T_Preview_02.jpg?raw=true "Fog & Shadows Preview")
![Preview](T_Preview_03.jpg?raw=true "Cascaded Shadow Maps")
![Preview](T_Preview_04.jpg?raw=true "Fog Ambient Anistropy Preview")
![Preview](T_Preview_05.jpg?raw=true "GI Preview")

## Implemented Features
- Clustered forward rendering.
- Volumetric fog & lighting.
- Realtime global illumination (Voxel cone tracing).
- Variance shadow mapping.
- Light cookies.
- Point, spot & directional lights.
- Cacaded shadow maps for directional lights.
- Physically based shading (Cook-Torrance brdf).
- HDR bloom.
- Tone mapping & color grading.
- Film grain.
- Auto exposure.
- Depth of field with auto focus.
- Screen space ambient occlusion.
- Octahedron hdr environment maps.
- Multi compile shader variants.
- Asset hot reloading.
- Shader material/property block system.
- Instanced dynamic batching.

## Planned Features
- A global illumination solution.
- Rectangular area light support.
- Exponential variance shadow maps.
- Motion vectors.
- Temporal reprojection for blending inter frame results of different effects.
- Documentation.

## Render Pipeline Execution Order
A rough overview of the steps taken to render a frame. (Some steps are omitted to avoid repetition).

- Cull geometry & lights.
- Update lights system.
	- Sort lights by type & shadowmap usage.
	- Update light data buffers.
	- Render shadowmaps for shadow casting lights.
		- Render in batches of 4 (or 1 in the case of directional lights).
		- Gather shadow casting geometry for the batch.
		- Render intermediate shadow map.
		- Perform blur.
		- Blit into shadow map atlas.
- Render scene depth & normals.
- Compute light clusters.
	- compute max depth per 2d tile.
	- assign lights to clusters & cull clusters outside of max depth range.
- Render screen space ambient occlusion from scene depth & normals.
- Forward render opaque objects.
	- Update instancing buffers.
		- Gather material properties to per shader property buffers.
		- Gather matrices to matrix buffers.
	- Draw instanced (PBR fragment shader overview).
		- Sample scene OEM for ambient specular & diffuse.
		- Sample & apply SSAO (Only affects ambient lighting).
		- Access contributing lights list from light clusters.
		- Process each light in the list through the material's BRDF.
- Render Volumetrics.
	- Compute Lighting & density per volume cell.
	- Compute integrated scattering per volume cell.
	- Composite with forward output.
- (TODO) Render transparent objects.
- Bloom & Tonemapping
	- Compute luminance histogram from forward output.
	- Compute & interpolate auto exposure from luminance histogram.
	- Render bloom layers (35 passes of separable blur outputted into 6 different bloom layers).
	- Composite bloom layers.
	- Apply auto exposure & tonemapping.
	- Apply gamma correction.

## Performance Metrics
- Average frame timings profiled on a NVIDIA RTX 2080 TI at 1080p resolution.
- The test scene has only 512 objects with 3 different materials & 2 different meshes.
- Light types are distributed so that there exists one directional light & an equal amount point & spot lights.
- Light radii are set at 40m to achieve good saturation in light clusters.
- The test scene has all features enabled (a test without volumetrics would probably run a lot faster).

Results with shadow casting lights:
- 0 lights : 1.4ms
- 5 lights : 2.1ms
- 17 lights : 3.0ms
- 33 lights : 4.3ms
- 54 lights : 5.8ms

Results with non shadow casting lights:
- 32 lights : 1.9ms
- 64 lights : 2.4ms
- 128 lights : 3.5ms
- 256 lights : 5.8ms
- 512 lights : 7.2ms

## Known Issues & Performance Drawbacks
- Matrix buffers are currently mapped per pass. Which is causing a lot of data duplication & some driver stalls.
	- A more optimal solution would be to gather matrices that contribute to the current frame into a single buffer & only build index buffers per pass.
- OpenGL doesn't support multiple command queues or async dispatches. Which leads to otherwise easily multithreadable passes having to be executed consecutively.
- Using sparse textures would probably be more performant in the following scenarios:
	- Shadow map atlas allocation. Currently the entire atlas is allocated in full. As opposed to only commiting sparse tiles for active areas.
	- Shader property instancing currently uses bindless texture handles for material properties. 
	  Creating a sparse texture atlas for them would probably be more cache friendly.
- Culling of for each pass is currently done on the cpu. This could be moved to the gpu instead.
- Vertex shader input attribute layouts are not ensured to match with vao attribute layouts in anyway.
- Vertex array objects are not shared among meshes but instead created per mesh.
	- A better approach could be to have attribute layout based vao cache & switch vertex buffers between drawcalls instead.
- Volumetric sample dithering causes artifacts on far away high frequency lighting effects.
	- This could be fixed by breaking up the low resolution sampling pattern in the composite pass with high frequency noise & then using temporal AA to hide the high frequency noise.


## Asset Sources
- [HDRI Haven](https://hdrihaven.com)
- [CC0 Textures](https://cc0textures.com/)

## Libraries & Other Dependencies
- C++ 17 support required
- OpenGL ARB Bindless texture & viewport array extension support required.
- OpenGL 4.6 support required.
- [KTX](https://github.com/KhronosGroup/KTX-Software)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [Glad](https://glad.dav1d.de/)
- [GLFW](https://www.glfw.org/)
- [GLM](https://github.com/g-truc/glm)
- [mikktspace](http://www.mikktspace.com/)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
