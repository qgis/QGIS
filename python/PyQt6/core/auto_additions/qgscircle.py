# The following has been generated automatically from src/core/geometry/qgscircle.h
# monkey patching scoped based enum
QgsCircle.SegmentCalculationMethod.Standard.__doc__ = "Standard sagitta-based calculation"
QgsCircle.SegmentCalculationMethod.Adaptive.__doc__ = "Adaptive calculation based on radius size"
QgsCircle.SegmentCalculationMethod.AreaError.__doc__ = "Calculation based on area error"
QgsCircle.SegmentCalculationMethod.ConstantDensity.__doc__ = "Simple calculation with constant segment density"
QgsCircle.SegmentCalculationMethod.__doc__ = """Method used to calculate the number of segments for circle approximation

.. versionadded:: 3.44

* ``Standard``: Standard sagitta-based calculation
* ``Adaptive``: Adaptive calculation based on radius size
* ``AreaError``: Calculation based on area error
* ``ConstantDensity``: Simple calculation with constant segment density

"""
# --
try:
    QgsCircle.from2Points = staticmethod(QgsCircle.from2Points)
    QgsCircle.from3Points = staticmethod(QgsCircle.from3Points)
    QgsCircle.fromCenterDiameter = staticmethod(QgsCircle.fromCenterDiameter)
    QgsCircle.fromCenterPoint = staticmethod(QgsCircle.fromCenterPoint)
    QgsCircle.from3Tangents = staticmethod(QgsCircle.from3Tangents)
    QgsCircle.from3TangentsMulti = staticmethod(QgsCircle.from3TangentsMulti)
    QgsCircle.fromExtent = staticmethod(QgsCircle.fromExtent)
    QgsCircle.minimalCircleFrom3Points = staticmethod(QgsCircle.minimalCircleFrom3Points)
    QgsCircle.calculateSegments = staticmethod(QgsCircle.calculateSegments)
    QgsCircle.calculateSegmentsAdaptive = staticmethod(QgsCircle.calculateSegmentsAdaptive)
    QgsCircle.calculateSegmentsByAreaError = staticmethod(QgsCircle.calculateSegmentsByAreaError)
    QgsCircle.calculateSegmentsByConstant = staticmethod(QgsCircle.calculateSegmentsByConstant)
    QgsCircle.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
