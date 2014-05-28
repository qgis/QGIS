PATTERN ANALYSIS
================

Description
-----------

Parameters
----------

- ``Input Grid[Raster]``:
- ``Size of Analysis Window[Selection]``:
- ``Max. Number of Classes[Number]``:

Outputs
-------

- ``Relative Richness[Raster]``:
- ``Diversity[Raster]``:
- ``Dominance[Raster]``:
- ``Fragmentation[Raster]``:
- ``Number of Different Classes[Raster]``:
- ``Center Versus Neighbours[Raster]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:patternanalysis', input, winsize, maxnumclass, relative, diversity, dominance, fragmentation, ndc, cvn)

	Available options for selection parameters:

	winsize(Size of Analysis Window)
		0 - [0] 3 X 3
		1 - [1] 5 X 5
		2 - [2] 7 X 7
