POINTS LAYER FROM TABLE
=======================

Description
-----------
This algorithm is a useful tool if you want to transform a table in a points layer. The input table must have two columns that
refer to the X and Y coordinates. You can add the table to the QGIS map canvas (e.g. as a csv file) or you can look for your
table in your computer.

In order to have a correct result you have to choose the coordinate reference system (CRS): this means that X and Y
fields of the table must refer to the same CRS chosen. 
Default CRS refers to EPSG:4326 (WGS 84). 

Parameters
----------

- ``Input layer[Table]``: input table
- ``X field[TableField]``: table column containing the X coordinate
- ``Y field[TableField]``: table column containing the Y coordinate
- ``Target CRS``: CRS chosen

Outputs
-------

- ``Output layer[Vector]``: the resulting points layer

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:pointslayerfromtable', input, xfield, yfield, output)
