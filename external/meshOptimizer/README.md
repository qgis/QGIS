[meshOptimiser](https://github.com/zeux/meshoptimizer) v 0.13

[meshOptimiser](https://github.com/zeux/meshoptimizer) is a library that provides algorithms to optimize meshes.
One of those algorithms simplifies meshes, e.i., reduces the number of vertices/triangles of the mesh. This library is oriented GPU rendering, but it could be used for other meshes.

In QGIS the [simplification algorithm](https://github.com/zeux/meshoptimizer#simplification) is used to simplify mesh layers to speed up rendering.

As mesh layer could have millions of triangles, rendering could be very slow, especially when all the triangles are displayed in the view whereas triangles are too small to be viewed. For those situations, an option provides the possibility to simplify the mesh. Simplification leads to one or more simplified mesh that represents levels of detail. When rendering the mesh, the appropriate level of detail is chosen to have an adequate rendering depending on the view.

