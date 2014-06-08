DISTANCE TO NEAREST HUB
=======================

Description
-----------

Parameters
----------

- ``Source Points Layer[Vector]``:
- ``Destination Hubs Layer[Vector]``:
- ``Hub Layer Name Attribute[TableField]``:
- ``Output Shape Type[Selection]``:
- ``Measurement Unit[Selection]``:

Outputs
-------

- ``Output[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:distancetonearesthub', sourcename, destname, nameattribute, shapetype, units, savename)

	Available options for selection parameters:

	shapetype(Output Shape Type)
		0 - Point
		1 - Line to Hub

	units(Measurement Unit)
		0 - Meters
		1 - Feet
		2 - Miles
		3 - Kilometers
		4 - Layer Units
