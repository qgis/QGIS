TOPOGRAPHIC CORRECTION
======================

Description
-----------

Parameters
----------

- ``Elevation[Raster]``:
- ``Original Image[Raster]``:
- ``Azimuth[Number]``:
- ``Height[Number]``:
- ``Method[Selection]``:
- ``Minnaert Correction[Number]``:
- ``Maximum Cells (C Correction Analysis)[Number]``:
- ``Value Range[Selection]``:

Outputs
-------

- ``Corrected Image[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:topographiccorrection', dem, original, azi, hgt, method, minnaert, maxcells, maxvalue, corrected)

	Available options for selection parameters:

	method(Method)
		0 - [0] Cosine Correction (Teillet et al. 1982)
		1 - [1] Cosine Correction (Civco 1989)
		2 - [2] Minnaert Correction
		3 - [3] Minnaert Correction with Slope (Riano et al. 2003)
		4 - [4] Minnaert Correction with Slope (Law & Nichol 2004)
		5 - [5] C Correction
		6 - [6] Normalization (after Civco, modified by Law & Nichol)

	maxvalue(Value Range)
		0 - [0] 1 byte (0-255)
		1 - [1] 2 byte (0-65535)
