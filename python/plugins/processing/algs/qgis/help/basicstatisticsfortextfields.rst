BASIC STATISTICS FOR TEXT FIELDS
================================

Description
-----------

Parameters
----------

- ``Input vector layer[Vector]``:
- ``Field to calculate statistics on[TableField]``:

Outputs
-------

- ``Statistics for text field[HTML]``:
- ``Minimum length[Number]``:
- ``Maximum length[Number]``:
- ``Mean length[Number]``:
- ``Count[Number]``:
- ``Number of empty values[Number]``:
- ``Number of non-empty values[Number]``:
- ``Number of unique values[Number]``:

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:basicstatisticsfortextfields', input_layer, field_name, output_html_file)
