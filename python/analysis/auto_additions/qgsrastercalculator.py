# The following has been generated automatically from src/analysis/raster/qgsrastercalculator.h
# monkey patching scoped based enum
QgsRasterCalculator.Result.Success.__doc__ = "Calculation successful"
QgsRasterCalculator.Result.CreateOutputError.__doc__ = "Error creating output data file"
QgsRasterCalculator.Result.InputLayerError.__doc__ = "Error reading input layer"
QgsRasterCalculator.Result.Canceled.__doc__ = "User canceled calculation"
QgsRasterCalculator.Result.ParserError.__doc__ = "Error parsing formula"
QgsRasterCalculator.Result.MemoryError.__doc__ = "Error allocating memory for result"
QgsRasterCalculator.Result.BandError.__doc__ = "Invalid band number for input"
QgsRasterCalculator.Result.CalculationError.__doc__ = "Error occurred while performing calculation"
QgsRasterCalculator.Result.__doc__ = """Result of the calculation

* ``Success``: Calculation successful
* ``CreateOutputError``: Error creating output data file
* ``InputLayerError``: Error reading input layer
* ``Canceled``: User canceled calculation
* ``ParserError``: Error parsing formula
* ``MemoryError``: Error allocating memory for result
* ``BandError``: Invalid band number for input
* ``CalculationError``: Error occurred while performing calculation

"""
# --
try:
    QgsRasterCalculatorEntry.__attribute_docs__ = {'ref': 'Name of entry.', 'raster': 'Raster layer associated with entry.', 'bandNumber': 'Band number for entry. Numbering for bands usually starts at 1 for the first band, not 0.'}
    QgsRasterCalculatorEntry.__annotations__ = {'ref': str, 'raster': 'QgsRasterLayer', 'bandNumber': int}
    QgsRasterCalculatorEntry.rasterEntries = staticmethod(QgsRasterCalculatorEntry.rasterEntries)
    QgsRasterCalculatorEntry.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterCalculator.__group__ = ['raster']
except (NameError, AttributeError):
    pass
