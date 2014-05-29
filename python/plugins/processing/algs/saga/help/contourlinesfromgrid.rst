CONTOUR LINES FROM GRID
=======================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Minimum Contour Value[Number]``:
- ``Maximum Contour Value[Number]``:
- ``Equidistance[Number]``:

Outputs
-------

- ``Contour Lines[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:contourlinesfromgrid', input, zmin, zmax, zstep, contour)
