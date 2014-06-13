RANDOM SELECTION WITHIN SUBSETS
===============================

Description
-----------

Parameters
----------

- ``Input layer[Vector]``:
- ``ID Field[TableField]``:
- ``Method[Selection]``:
- ``Number/percentage of selected features[Number]``:

Outputs
-------

- ``Selection[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:randomselectionwithinsubsets', input, field, method, number)

	Available options for selection parameters:

	method(Method)
		0 - Number of selected features
		1 - Percentage of selected features
