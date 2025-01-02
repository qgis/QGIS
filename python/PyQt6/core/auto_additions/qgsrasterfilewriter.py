# The following has been generated automatically from src/core/raster/qgsrasterfilewriter.h
QgsRasterFileWriter.SortRecommended = QgsRasterFileWriter.RasterFormatOption.SortRecommended
QgsRasterFileWriter.RasterFormatOptions = lambda flags=0: QgsRasterFileWriter.RasterFormatOption(flags)
try:
    QgsRasterFileWriter.FilterFormatDetails.__attribute_docs__ = {'driverName': 'Unique driver name', 'filterString': 'Filter string for file picker dialogs'}
    QgsRasterFileWriter.FilterFormatDetails.__doc__ = """Details of available filters and formats."""
    QgsRasterFileWriter.FilterFormatDetails.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterFileWriter.filterForDriver = staticmethod(QgsRasterFileWriter.filterForDriver)
    QgsRasterFileWriter.supportedFiltersAndFormats = staticmethod(QgsRasterFileWriter.supportedFiltersAndFormats)
    QgsRasterFileWriter.supportedFormatExtensions = staticmethod(QgsRasterFileWriter.supportedFormatExtensions)
    QgsRasterFileWriter.driverForExtension = staticmethod(QgsRasterFileWriter.driverForExtension)
    QgsRasterFileWriter.extensionsForFormat = staticmethod(QgsRasterFileWriter.extensionsForFormat)
    QgsRasterFileWriter.__group__ = ['raster']
except (NameError, AttributeError):
    pass
