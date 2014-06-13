SPLIT SHAPES LAYER RANDOMLY
===========================

Description
-----------
This algorithm splits randomly in two parts the input layer. Original attributes are kept in the new attribute tables.

Parameters
----------

- ``Shapes[Vector]``: input layer
- ``Relation B / A[Number]``: split ratio between the resulting layers

Outputs
-------

- ``Group A[Vector]``: first resulting layer
- ``Group B[Vector]``: second resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:splitshapeslayerrandomly', shapes, percent, a, b)
