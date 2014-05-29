PROFILES FROM LINES
===================

Description
-----------

Parameters
----------

- ``DEM[Raster]``:
- ``Values[MultipleInput]``:
- ``Lines[Vector]``:
- ``Name[TableField]``:
- ``Each Line as new Profile[Boolean]``:

Outputs
-------

- ``Profiles[Vector]``:
- ``Profiles[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:profilesfromlines', dem, values, lines, name, split, profile, profiles)
