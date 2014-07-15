LIST UNIQUE VALUES
==================

Description
-----------
This algorithm counts the number of unique values of an attribute table field. 

Parameters
----------

- ``Input layer[Vector]``: layer in input
- ``Target field[TableField]``: field of the attribute table with the unique values to count

Outputs
-------

- ``Unique values[HTML]``: resulting html file
- ``Total unique values[Number]``: number of the unique values found
- ``Unique values[String]``: list of each unique value

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:listuniquevalues', input_layer, field_name, output)
