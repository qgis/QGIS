FUZZIFY
=======

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``A[Number]``:
- ``B[Number]``:
- ``C[Number]``:
- ``D[Number]``:
- ``Membership Function Type[Selection]``:
- ``Adjust to Grid[Boolean]``:

Outputs
-------

- ``Fuzzified Grid[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:fuzzify', input, a, b, c, d, type, autofit, output)

	Available options for selection parameters:

	type(Membership Function Type)
		0 - [0] linear
		1 - [1] sigmoidal
		2 - [2] j-shaped
