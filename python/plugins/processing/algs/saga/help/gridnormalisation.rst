GRID NORMALISATION
==================

Description
-----------
This algorithm normalises the grid values according to minimum and maximum values chosen. 
Parameters
----------

- ``Grid[Raster]``: grid in input
- ``Target Range (min)[Number]``:minimum value (range)
- ``Target Range (max)[Number]``:maximum value (range)

Outputs
-------

- ``Normalised Grid[Raster]``: the resulting grid

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gridnormalisation', input, range_min, range_max, output)
