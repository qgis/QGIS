RASTER LAYER STATISTICS
=======================

Description
-----------
This algorithm calculate basic statistics of the raster layer.

Parameters
----------

- ``Input layer[Raster]``: the raster layer 

Outputs
-------

- ``Statistics[HTML]``:
- ``Minimum value[Number]``: minimux value of the cells
- ``Maximum value[Number]``: maximum value of the cells
- ``Sum[Number]``: sum of all the cells values
- ``Mean value[Number]``: mean value of the cells
- ``valid cells count[Number]``: number of valid cells
- ``No-data cells count[Number]``: number of no-data cells
- ``Standard deviation[Number]``: standard deviation of the cells

See also
---------


Console usage
-------------


::

	processing.runalg('qgis:rasterlayerstatistics', input, output_html_file)
