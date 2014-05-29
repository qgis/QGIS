GRADIENT VECTOR FROM CARTESIAN TO POLAR COORDINATES
===================================================

Description
-----------

Parameters
----------

- ``X Component[Raster]``:
- ``Y Component[Raster]``:
- ``Polar Angle Units[Selection]``:
- ``Polar Coordinate System[Selection]``:
- ``User defined Zero Direction[Number]``:
- ``User defined Orientation[Selection]``:

Outputs
-------

- ``Direction[Raster]``:
- ``Length[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:gradientvectorfromcartesiantopolarcoordinates', dx, dy, units, system, system_zero, system_orient, dir, len)

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
