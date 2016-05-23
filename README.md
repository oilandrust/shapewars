***************************************
**  Shapewars (work in progress):
***************************************

This project was started in winter 2015 for fun and learning purposes.
The aim is to implement a RTS to learn about AI, navigation and engine programming.

<iframe width="420" height="315" src="https://www.youtube.com/embed/yqgOq7DDcAI" frameborder="0" allowfullscreen></iframe>

Synopsis:

For millennia in the universe of shapes, it has been an uncontested fact that all shapes and entities are perfectly cubic. 
However one day on the main planet cube, an entity is discovered that changes all beliefs, 
it appears that other shapes exist, following this discovery, 
a terrible war begins to fight for the supremacy of shapes...

Features:
- Minimal engine written in C++ (mainly C syle).
- Fixed memory budget managed by stack base allocators.
- Minimal non optimized OpenGL 3.1 core profile renderer.
- Procedural simple shapes.
- NavMesh generation inspired by Mikko Mononen's Recast library.
- A* path finding and steering with the funnel algorithm.
- Font rendering with stb_truetype.

Dependencies:
- OpenGl, Glew, SDL2, stb truetype.

How to build:
- Visual studio (tested with VS2008): project files included. in source/build
- Xcode, and other project can be generated with CMake. Developed mainly on mac with XCode.

References:
- A detailed explanation of the recast nav mesh generation algorithm.
http://www.critterai.org/projects/nmgen_study/

- The recast source code.
https://github.com/recastnavigation/recastnavigation

- High level ressource about path finding and navigation.
http://www.jceipek.com/Olin-Coding-Tutorials/pathing.html

- Detailed explanation of the watershed method.
http://maverick.inria.fr/Publications/2003/HDS03/mcp.pdf

Navmesh generation principle:
The idea is to generate a mesh of convex polygons of the walkable regions of the level.
The recast algorithm does so by generating a distance field of the ground.
This is used to segment large portions of the ground using the watershed algorithm.
The contour of the segments are extracted and triangulated.
The triangles are then merged into large convex polygons and the connectivity is computed.

Know Issues:
- The nav mesh generation isn't very robust, the region flooding generates 1 cell wide regions because of local minima of the distance field.
This is a know issue with the watershed algorithm, I apply smoothing of the distance field and try to merge too small regions but some issues remain.
The contour extraction fails when there is 1 cell wide regions, but this can be fixed relatively easily.
For a 2d navmesh, I now believe that using a constrained delaunay triangulation of the walkable region could be more straightforward than the watershed method.

- The string pulling algorithm for path generation does not take the size of the agent into consideration, as a result, the agent intersects the wall.
This should be fix by shrinking the navmesh.

- Sometimes the path is not optimal, this is because the heuristics for A* on the polygon mesh considers the distance from the center of the polygon to the target. A better heuristic should solve this.

