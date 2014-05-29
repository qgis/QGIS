WATERSHED SEGMENTATION
======================

Description
-----------

Parameters
----------

- ``Grid[Raster]``:
- ``Output[Selection]``:
- ``Method[Selection]``:
- ``Join Segments based on Threshold Value[Selection]``:
- ``Threshold[Number]``:
- ``Allow Edge Pixels to be Seeds[Boolean]``:
- ``Borders[Boolean]``:

Outputs
-------

- ``Segments[Raster]``:
- ``Seed Points[Vector]``:
- ``Borders[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:watershedsegmentation', grid, output, down, join, threshold, edge, bborders, segments, seeds, borders)

	Available options for selection parameters:

	output(Output)
		0 - [0] Seed Value
		1 - [1] Segment ID

	down(Method)
		0 - [0] Minima
		1 - [1] Maxima

	join(Join Segments based on Threshold Value)
		0 - [0] do not join
		1 - [1] seed to saddle difference
		2 - [2] seeds difference
