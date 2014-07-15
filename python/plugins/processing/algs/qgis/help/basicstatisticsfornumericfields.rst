BASIC STATISTICS FOR NUMERIC FIELDS
===================================

Description
-----------

Parameters
----------

- ``Input vector layer[Vector]``:
- ``Field to calculate statistics on[TableField]``:

Outputs
-------

- ``Statistics for numeric field[HTML]``:
- ``Coefficient of Variation[Number]``:
- ``Minimum value[Number]``:
- ``Maximum value[Number]``:
- ``Sum[Number]``:
- ``Mean value[Number]``:
- ``Count[Number]``:
- ``Range[Number]``:
- ``Median[Number]``:
- ``Number of unique values[Number]``:
- ``Standard deviation[Number]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:basicstatisticsfornumericfields', input_layer, field_name, output_html_file)
