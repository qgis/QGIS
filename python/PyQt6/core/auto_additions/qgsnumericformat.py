# The following has been generated automatically from src/core/numericformats/qgsnumericformat.h
# monkey patching scoped based enum
QgsNumericFormatContext.Interpretation.Generic.__doc__ = "Generic"
QgsNumericFormatContext.Interpretation.Latitude.__doc__ = "Latitude values"
QgsNumericFormatContext.Interpretation.Longitude.__doc__ = "Longitude values"
QgsNumericFormatContext.Interpretation.__doc__ = """Interpretation of numeric values.

.. versionadded:: 3.26

* ``Generic``: Generic
* ``Latitude``: Latitude values
* ``Longitude``: Longitude values

"""
# --
QgsNumericFormatContext.Interpretation.baseClass = QgsNumericFormatContext
try:
    QgsNumericFormatContext.__group__ = ['numericformats']
except (NameError, AttributeError):
    pass
try:
    QgsNumericFormat.__group__ = ['numericformats']
except (NameError, AttributeError):
    pass
