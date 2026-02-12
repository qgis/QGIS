# The following has been generated automatically from src/core/geometry/qgsquadrilateral.h
QgsQuadrilateral.Distance = QgsQuadrilateral.ConstructionOption.Distance
QgsQuadrilateral.Projected = QgsQuadrilateral.ConstructionOption.Projected
QgsQuadrilateral.Point1 = QgsQuadrilateral.Point.Point1
QgsQuadrilateral.Point2 = QgsQuadrilateral.Point.Point2
QgsQuadrilateral.Point3 = QgsQuadrilateral.Point.Point3
QgsQuadrilateral.Point4 = QgsQuadrilateral.Point.Point4
try:
    QgsQuadrilateral.rectangleFrom3Points = staticmethod(QgsQuadrilateral.rectangleFrom3Points)
    QgsQuadrilateral.rectangleFromExtent = staticmethod(QgsQuadrilateral.rectangleFromExtent)
    QgsQuadrilateral.squareFromDiagonal = staticmethod(QgsQuadrilateral.squareFromDiagonal)
    QgsQuadrilateral.rectangleFromCenterPoint = staticmethod(QgsQuadrilateral.rectangleFromCenterPoint)
    QgsQuadrilateral.fromRectangle = staticmethod(QgsQuadrilateral.fromRectangle)
    QgsQuadrilateral.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
