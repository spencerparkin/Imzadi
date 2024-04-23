
# Collision System

The goal here is to provide a means of tracking various shapes in space and being able to answer collision-based questions about those shapes,
such as which shapes are in collision with which others (and *how* in the form of contact points and contact normals), as well as which shapes
are hit by a given ray-cast.  What the collision system does *not* do is perform any collision resolution, constraint-solving, physics, or any
other kind of motion.  Those details are up to the calling application.

I'm also going to try to make the system queries and commands asynchronous.  There will also be a need to provide synchronization of the
collision system thread with the main thread.  Specifically, there should be an efficient way for the main thread to stall waiting for all
queries or commands made to the collision system thread to finish.  The difference between a command and a query is that the former does not
warrent a response (or result), while the latter does.

Internally, queries will perform a broad phase search followed by a narrow phase.  The broad phase is facilitated by a spacial partioning
data-structure.  The narrow phase performs detailed calculations about how two shapes collide, if at all.  The system should be extendable in
the sense that different types of collision shapes may be added to the system, provided support for how those shape types can intersect all
existing shape types is added.  Typical shapes may include boxes, spheres, cylinders, convex/planar polygons, etc.

A good testing ground for the collision system library will be a simple platformer game with a character that can run and jump over some
polygonal-based terrain.  In the case of the character, what's rendered does not necessarily need to reflect exactly what's being used to
collide that character against the terrain.  The collision shape of the terrain, too, need not be an exact replica of what's rendered, though
in this simple test-case, that is probably the easiest thing to do.  There should be a debug mode of the collision system where the collision
shapes can be rendered, as well as possibly the extents of the spacial-partitioning tree.