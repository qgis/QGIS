CLOSE GAPS WITH SPLINE
======================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Mask[Raster]``:
- ``Only Process Gaps with Less Cells[Number]``:
- ``Maximum Points[Number]``:
- ``Number of Points for Local Interpolation[Number]``:
- ``Extended Neighourhood[Boolean]``:
- ``Neighbourhood[Selection]``:
- ``Radius (Cells)[Number]``:
- ``Relaxation[Number]``:

Outputs
-------

- ``Closed Gaps Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:closegapswithspline', grid, mask, maxgapcells, maxpoints, localpoints, extended, neighbours, radius, relaxation, closed)

	Available options for selection parameters:

	neighbours(Neighbourhood)
		0 - [0] Neumann
		1 - [1] Moore
