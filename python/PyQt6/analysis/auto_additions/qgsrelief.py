# The following has been generated automatically from src/analysis/raster/qgsrelief.h
# monkey patching scoped based enum
QgsRelief.Result.Success.__doc__ = "Calculation succeeded"
QgsRelief.Result.InvalidInput.__doc__ = "Invalid input layer"
QgsRelief.Result.OutputCreationFailed.__doc__ = "Creation of output layer failed"
QgsRelief.Result.InvalidInputSize.__doc__ = "Input raster was too small (at least 3 rows are required)"
QgsRelief.Result.Canceled.__doc__ = "Operation was canceled"
QgsRelief.Result.__doc__ = """Calculation results.

.. versionadded:: 4.2

* ``Success``: Calculation succeeded
* ``InvalidInput``: Invalid input layer
* ``OutputCreationFailed``: Creation of output layer failed
* ``InvalidInputSize``: Input raster was too small (at least 3 rows are required)
* ``Canceled``: Operation was canceled

"""
# --
try:
    QgsRelief.__group__ = ['raster']
except (NameError, AttributeError):
    pass
