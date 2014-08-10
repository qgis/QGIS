FIELD CALCULATOR
================

Description
-----------

This algorithm creates a new field in the attributes table of a vector layer, and fill it with values computed from the other available fields, according to a given formula.

Fields in the formula are referenced by their names. Names are case sensitive and they should not be quoted.

The formula to use is a Python expression, so any valid Python element and operation can be used.

Assuming an input vector layer with fields named ``MALES``, ``FEMALES``, and ``AREA``, the following are some valid formulas.

- ``MALES / SHAPE_AREA``. To calculate the density of male individuals.
- ``float(MALES) / FEMALES``. To calculate a sex ratio
- ``'male' if MALES > FEMALES else 'female'``. To compute a new text field which indicates the predominant sex in the population.


Parameters
----------

- ``Input layer[Vector]``: The vector layer to use.
- ``Result field name[String]``: The name of the new field to add.
- ``Field type``. The type of the field to create. The values resultign fro applying the formula should be of this type. Otherwise, a NULL value will be used.
- ``Field length``. The length of the field for string fields.
- ``Field precision``. The precision of the field for fields of type double.
- ``Formula[String]``: The formula to use, according to the explanations given above.

Outputs
-------

- ``Output layer[Vector]``: the resulting layer, with an additional field with the value computed according to the specified formula.

See also
---------

For a more advanced calculator wich allows more complex Python elements, see the Advanced Python Field Calculator algorithm

Console usage
-------------


::

	processing.runalg('qgis:fieldcalculator', input_layer, field_name, formula, output_layer)
