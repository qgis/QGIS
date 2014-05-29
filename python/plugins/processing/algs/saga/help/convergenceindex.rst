CONVERGENCE INDEX
=================

Description
-----------
It calculates an index of convergence/divergence regarding to overland flow. 
By its meaning it is similar to plan or horizontal curvature, but gives much smoother results. 
The calculation uses the aspects of surrounding cells, i.e. it looks to which degree surrounding cells point to the center cell. 
The result is given as percentages, negative values correspond to convergent, positive to divergent flow conditions. 
Minus 100 would be like a peak of a cone, plus 100 a pit, and 0 an even slope.

Parameters
----------

- ``Elevation[Raster]``:
- ``Method[Selection]``:
- ``Gradient Calculation[Selection]``:

Outputs
-------

- ``Convergence Index[Raster]``:

See also
---------
Koethe, R. / Lehmeier, F. (1996): 'SARA, System zur Automatischen Relief-Analyse', Benutzerhandbuch, 2. Auflage 
[Geogr. Inst. Univ. Goettingen, unpublished]

Console usage
-------------


::

	processing.runalg('saga:convergenceindex', elevation, method, neighbours, result)

	Available options for selection parameters:

	method(Method)
		0 - [0] Aspect
		1 - [1] Gradient

	neighbours(Gradient Calculation)
		0 - [0] 2 x 2
		1 - [1] 3 x 3
