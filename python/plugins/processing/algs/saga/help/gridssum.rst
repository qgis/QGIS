GRIDS SUM
=========

Description
-----------
This algorithm creates a new grid layer as the result of the sum of 2 or more grid layers. 
Parameters
----------

- ``Grids[MultipleInput]``: grid layers to sum

Outputs
-------

- ``Sum[Raster]``: the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridssum', grids, result)
