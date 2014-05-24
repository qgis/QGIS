GLOBAL MORAN'S I FOR GRIDS
==========================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Case of contiguity[Selection]``:

Outputs
-------

- ``Result[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:globalmoransiforgrids', grid, contiguity, result)

	Available options for selection parameters:

	contiguity(Case of contiguity)
		0 - [0] Rook
		1 - [1] Queen
