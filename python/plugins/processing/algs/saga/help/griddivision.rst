GRID DIVISION
=============

Description
-----------
This algorithm creates a new grid layer as the result of the division between 2 other grid layers. 

Parameters
----------

- ``Dividend[Raster]``: first layer
- ``Divisor[Raster]``: second layer

Outputs
-------

- ``Quotient[Raster]``: the resulting layer

See also
---------


Console usage
-------------


::

	processing.runalg('saga:griddivision', a, b, c)
