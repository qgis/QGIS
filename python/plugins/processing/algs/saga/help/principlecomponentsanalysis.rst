PRINCIPLE COMPONENTS ANALYSIS
=============================

Description
-----------

Parameters
----------

- ``Table[Table]``:
- ``Method[Selection]``:
- ``Number of Components[Number]``:

Outputs
-------

- ``Principle Components[Table]``:

See also
---------


Console usage
-------------


::

	processing.runalg('saga:principlecomponentsanalysis', table, method, nfirst, pca)

	Available options for selection parameters:

	method(Method)
		0 - [0] correlation matrix
		1 - [1] variance-covariance matrix
		2 - [2] sums-of-squares-and-cross-products matrix
