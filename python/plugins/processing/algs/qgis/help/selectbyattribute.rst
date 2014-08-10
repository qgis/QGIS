SELECT BY ATTRIBUTE
===================

Description
-----------
With this algorithm you can make a subset of a layer according to specific parameters referring to an attribute table field.
For example you may want to select features of a polygons layer that have an area smaller than a specific value.

Be aware that the algorithm is case-sensitive ("qgis" is different from "Qgis" and from "QGIS")!

Parameters
----------

- ``Input Layer[Vector]``: layer in input (can be a points, lines or a polygons layer)
- ``Selection attribute[TableField]``: field of the attribute table on which perform the selection
- ``Comparison[Selection]``: type of comparison operator
- ``Value[String]``: value on which perform the comparison

Outputs
-------

- ``Output[Vector]``: the algorithm doesn't create a new layer, it just made a selection of the input layer

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:selectbyattribute', layername, attribute, comparison, comparisonvalue)

	Available options for selection parameters:

	comparison(Comparison)
		0 - ==
		1 - !=
		2 - >
		3 - >=
		4 - <
		5 - <=
		6 - begins with
		7 - contains
