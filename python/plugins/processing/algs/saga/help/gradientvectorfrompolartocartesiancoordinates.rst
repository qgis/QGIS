GRADIENT VECTOR FROM POLAR TO CARTESIAN COORDINATES
===================================================

Description
-----------

Parameters
----------

- ``Direction[Raster]``:
- ``Length[Raster]``:
- ``Polar Angle Units[Selection]``:
- ``Polar Coordinate System[Selection]``:
- ``User defined Zero Direction[Number]``:
- ``User defined Orientation[Selection]``:

Outputs
-------

- ``X Component[Raster]``:
- ``Y Component[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gradientvectorfrompolartocartesiancoordinates', dir, len, units, system, system_zero, system_orient, dx, dy)

	Available options for selection parameters:

	units(Polar Angle Units)
		0 - [0] radians
		1 - [1] degree

	system(Polar Coordinate System)
		0 - [0] mathematical
		1 - [1] geographical
		2 - [2] user defined

	system_orient(User defined Orientation)
		0 - [0] clockwise
		1 - [1] counterclockwise
