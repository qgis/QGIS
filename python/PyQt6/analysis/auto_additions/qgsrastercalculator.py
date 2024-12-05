# The following has been generated automatically from src/analysis/raster/qgsrastercalculator.h
QgsRasterCalculator.Success = QgsRasterCalculator.Result.Success
QgsRasterCalculator.CreateOutputError = QgsRasterCalculator.Result.CreateOutputError
QgsRasterCalculator.InputLayerError = QgsRasterCalculator.Result.InputLayerError
QgsRasterCalculator.Canceled = QgsRasterCalculator.Result.Canceled
QgsRasterCalculator.ParserError = QgsRasterCalculator.Result.ParserError
QgsRasterCalculator.MemoryError = QgsRasterCalculator.Result.MemoryError
QgsRasterCalculator.BandError = QgsRasterCalculator.Result.BandError
QgsRasterCalculator.CalculationError = QgsRasterCalculator.Result.CalculationError
try:
    QgsRasterCalculatorEntry.__attribute_docs__ = {'ref': 'Name of entry.', 'raster': 'Raster layer associated with entry.', 'bandNumber': 'Band number for entry. Numbering for bands usually starts at 1 for the first band, not 0.'}
    QgsRasterCalculatorEntry.rasterEntries = staticmethod(QgsRasterCalculatorEntry.rasterEntries)
    QgsRasterCalculatorEntry.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterCalculator.__group__ = ['raster']
except (NameError, AttributeError):
    pass
