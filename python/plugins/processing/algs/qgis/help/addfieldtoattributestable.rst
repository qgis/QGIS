ADD FIELD TO ATTRIBUTES TABLE
=============================

Description
-----------

Parameters
----------

- ``Input layer[Vector]``:
- ``Field name[String]``:
- ``Field type[Selection]``:

Outputs
-------

- ``Output layer[Vector]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:addfieldtoattributestable', input_layer, field_name, field_type, output_layer)

	Available options for selection parameters:

	field_type(Field type)
		0 - Integer
		1 - Float
		2 - String
