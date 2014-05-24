GRID DIFFERENCE
===============

Description
-----------
This algorithm creates a new grid layer as the result of the difference between 2 other grid layers. 
Parameters
----------

- ``A[Raster]``: first grid layer
- ``B[Raster]``: second grid layer

Outputs
-------

- ``Difference (A - B)[Raster]``: the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:griddifference', a, b, c)
