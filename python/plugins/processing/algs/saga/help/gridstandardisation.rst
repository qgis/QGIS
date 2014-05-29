GRID STANDARDISATION
====================

Description
-----------
This algorithm standardises the grid layer values. It is also possible to choose the stretch factor, that is the "spreading" of the new values.  
Parameters
----------

- ``Grid[Raster]``: grid layer in input
- ``Stretch Factor[Number]``: stretching factor (standard deviation)

Outputs
-------

- ``Standardised Grid[Raster]``:the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridstandardisation', input, stretch, output)
