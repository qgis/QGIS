# The following has been generated automatically from src/core/raster/qgsrasterminmaxorigin.h
QgsRasterMinMaxOrigin.None_ = QgsRasterMinMaxOrigin.Limits.None_
QgsRasterMinMaxOrigin.MinMax = QgsRasterMinMaxOrigin.Limits.MinMax
QgsRasterMinMaxOrigin.StdDev = QgsRasterMinMaxOrigin.Limits.StdDev
QgsRasterMinMaxOrigin.CumulativeCut = QgsRasterMinMaxOrigin.Limits.CumulativeCut
QgsRasterMinMaxOrigin.WholeRaster = QgsRasterMinMaxOrigin.Extent.WholeRaster
QgsRasterMinMaxOrigin.CurrentCanvas = QgsRasterMinMaxOrigin.Extent.CurrentCanvas
QgsRasterMinMaxOrigin.UpdatedCanvas = QgsRasterMinMaxOrigin.Extent.UpdatedCanvas
QgsRasterMinMaxOrigin.Exact = QgsRasterMinMaxOrigin.StatAccuracy.Exact
QgsRasterMinMaxOrigin.Estimated = QgsRasterMinMaxOrigin.StatAccuracy.Estimated
try:
    QgsRasterMinMaxOrigin.__attribute_docs__ = {'CUMULATIVE_CUT_LOWER': 'Default cumulative cut lower limit', 'CUMULATIVE_CUT_UPPER': 'Default cumulative cut upper limit', 'DEFAULT_STDDEV_FACTOR': 'Default standard deviation factor'}
    QgsRasterMinMaxOrigin.limitsString = staticmethod(QgsRasterMinMaxOrigin.limitsString)
    QgsRasterMinMaxOrigin.limitsFromString = staticmethod(QgsRasterMinMaxOrigin.limitsFromString)
    QgsRasterMinMaxOrigin.extentString = staticmethod(QgsRasterMinMaxOrigin.extentString)
    QgsRasterMinMaxOrigin.extentFromString = staticmethod(QgsRasterMinMaxOrigin.extentFromString)
    QgsRasterMinMaxOrigin.statAccuracyString = staticmethod(QgsRasterMinMaxOrigin.statAccuracyString)
    QgsRasterMinMaxOrigin.statAccuracyFromString = staticmethod(QgsRasterMinMaxOrigin.statAccuracyFromString)
    QgsRasterMinMaxOrigin.__group__ = ['raster']
except (NameError, AttributeError):
    pass
