# The following has been generated automatically from src/analysis/raster/qgsninecellfilter.h
# monkey patching scoped based enum
QgsNineCellFilter.Result.Success.__doc__ = "Operation completed successfully"
QgsNineCellFilter.Result.InputLayerError.__doc__ = "Error reading input file"
QgsNineCellFilter.Result.DriverError.__doc__ = "Could not open the driver for the specified format"
QgsNineCellFilter.Result.CreateOutputError.__doc__ = "Error creating output file"
QgsNineCellFilter.Result.InputBandError.__doc__ = "Error reading input raster band"
QgsNineCellFilter.Result.OutputBandError.__doc__ = "Error reading output raster band"
QgsNineCellFilter.Result.RasterSizeError.__doc__ = "Raster height is too small (need at least 3 rows)"
QgsNineCellFilter.Result.Canceled.__doc__ = "User canceled calculation"
QgsNineCellFilter.Result.__doc__ = """
.. versionadded:: 3.44

* ``Success``: Operation completed successfully
* ``InputLayerError``: Error reading input file
* ``DriverError``: Could not open the driver for the specified format
* ``CreateOutputError``: Error creating output file
* ``InputBandError``: Error reading input raster band
* ``OutputBandError``: Error reading output raster band
* ``RasterSizeError``: Raster height is too small (need at least 3 rows)
* ``Canceled``: User canceled calculation

"""
# --
try:
    QgsNineCellFilter.__abstract_methods__ = ['processNineCellWindow']
    QgsNineCellFilter.__group__ = ['raster']
except (NameError, AttributeError):
    pass
