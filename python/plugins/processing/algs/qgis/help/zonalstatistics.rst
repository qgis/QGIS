ZONAL STATISTICS
================

Description
-----------

This algorithm calculates some statistics values for pixels of input raster
inside certain zones, defined as polygon layer.

Following values calculated for each zone:

* minimum
* maximum
* sum
* count
* mean
* standard deviation
* number of unique values
* range
* variance

All results stored in output vector layer in columns with user-defined prefix.


Parameters
----------

- ``Raster layer``: The raster layer to use.
- ``Raster band``: The band number.
- ``Vector layer containing zones``: The vector layer with zones.
- ``Output column prefix``: User-defined prefix for output columns.
- ``Load whole raster in memory``: Determines if raster band will be loaded in
  memory or readed by chunks. Useful only when disk IO or raster scanning
  inefficiencies are your limiting factor.

Outputs
-------

- ``Output layer``: The resulting layer with new columns added.

See also
--------


Console usage
-------------


::

  processing.runalg("qgis:zonalstatistics", input_raster, band_number, input_vector, column_prefix, load_flag, output_layer)
