POLYGONS TO EDGES AND NODES
===========================

Description
-----------
This algorithm creates:

- a lines layer corresponding to the edges (borders) of each feature of the polygon layer 
- a points layer corresponding to the nodes of each feature of the polygons layer 


Parameters
----------

- ``Polygons[Vector]``: polygons layer in input

Outputs
-------

- ``Edges[Vector]``: lines layer
- ``Nodes[Vector]``: points layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:polygonstoedgesandnodes', polygons, edges, nodes)
