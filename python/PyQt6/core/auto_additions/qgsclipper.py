# The following has been generated automatically from src/core/qgsclipper.h
QgsClipper.XMax = QgsClipper.Boundary.XMax
QgsClipper.XMin = QgsClipper.Boundary.XMin
QgsClipper.YMax = QgsClipper.Boundary.YMax
QgsClipper.YMin = QgsClipper.Boundary.YMin
QgsClipper.ZMax = QgsClipper.Boundary.ZMax
QgsClipper.ZMin = QgsClipper.Boundary.ZMin
try:
    QgsClipper.__attribute_docs__ = {'MAX_X': 'Maximum X-coordinate of the rectangular box used for clipping.', 'MIN_X': 'Minimum X-coordinate of the rectangular box used for clipping.', 'MAX_Y': 'Maximum Y-coordinate of the rectangular box used for clipping.', 'MIN_Y': 'Minimum Y-coordinate of the rectangular box used for clipping.'}
    QgsClipper.trimFeature = staticmethod(QgsClipper.trimFeature)
    QgsClipper.trimPolygon = staticmethod(QgsClipper.trimPolygon)
    QgsClipper.clippedLine = staticmethod(QgsClipper.clippedLine)
    QgsClipper.clipLineSegment = staticmethod(QgsClipper.clipLineSegment)
except (NameError, AttributeError):
    pass
