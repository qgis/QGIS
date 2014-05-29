VARIOGRAM SURFACE
=================

Description
-----------

Parameters
----------

- ``Points[Vector]``:
- ``Attribute[TableField]``:
- ``Number of Distance Classes[Number]``:
- ``Skip Number[Number]``:

Outputs
-------

- ``Number of Pairs[Raster]``:
- ``Variogram Surface[Raster]``:
- ``Covariance Surface[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:variogramsurface', points, field, distcount, nskip, count, variance, covariance)
