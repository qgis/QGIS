# The following has been generated automatically from src/core/raster/qgsrasterbandstats.h
try:
    QgsRasterBandStats.__attribute_docs__ = {'bandNumber': 'The gdal band number (starts at 1)', 'elementCount': 'The number of not no data cells in the band.', 'maximumValue': 'The maximum cell value in the raster band. NO_DATA values\nare ignored. This does not use the gdal GetMaximmum function.', 'minimumValue': 'The minimum cell value in the raster band. NO_DATA values\nare ignored. This does not use the gdal GetMinimum function.', 'mean': 'The mean cell value for the band. NO_DATA values are excluded.', 'range': 'The range is the distance between min & max.', 'stdDev': 'The standard deviation of the cell values.', 'statsGathered': 'Collected statistics', 'sum': 'The sum of all cells in the band. NO_DATA values are excluded.', 'sumOfSquares': 'The sum of the squares. Used to calculate standard deviation.', 'width': 'Number of columns used to calc statistics', 'height': 'Number of rows used to calc statistics', 'extent': 'Extent used to calc statistics'}
    QgsRasterBandStats.__group__ = ['raster']
except (NameError, AttributeError):
    pass
