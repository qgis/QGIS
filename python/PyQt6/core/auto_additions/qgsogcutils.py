# The following has been generated automatically from src/core/qgsogcutils.h
QgsOgcUtils.GML_2_1_2 = QgsOgcUtils.GMLVersion.GML_2_1_2
QgsOgcUtils.GML_3_1_0 = QgsOgcUtils.GMLVersion.GML_3_1_0
QgsOgcUtils.GML_3_2_1 = QgsOgcUtils.GMLVersion.GML_3_2_1
QgsOgcUtils.FILTER_OGC_1_0 = QgsOgcUtils.FilterVersion.FILTER_OGC_1_0
QgsOgcUtils.FILTER_OGC_1_1 = QgsOgcUtils.FilterVersion.FILTER_OGC_1_1
QgsOgcUtils.FILTER_FES_2_0 = QgsOgcUtils.FilterVersion.FILTER_FES_2_0
try:
    QgsOgcUtils.geometryFromGML = staticmethod(QgsOgcUtils.geometryFromGML)
    QgsOgcUtils.rectangleFromGMLBox = staticmethod(QgsOgcUtils.rectangleFromGMLBox)
    QgsOgcUtils.rectangleFromGMLEnvelope = staticmethod(QgsOgcUtils.rectangleFromGMLEnvelope)
    QgsOgcUtils.geometryToGML = staticmethod(QgsOgcUtils.geometryToGML)
    QgsOgcUtils.rectangleToGMLBox = staticmethod(QgsOgcUtils.rectangleToGMLBox)
    QgsOgcUtils.rectangleToGMLEnvelope = staticmethod(QgsOgcUtils.rectangleToGMLEnvelope)
    QgsOgcUtils.colorFromOgcFill = staticmethod(QgsOgcUtils.colorFromOgcFill)
    QgsOgcUtils.expressionFromOgcFilter = staticmethod(QgsOgcUtils.expressionFromOgcFilter)
    QgsOgcUtils.expressionToOgcExpression = staticmethod(QgsOgcUtils.expressionToOgcExpression)
    QgsOgcUtils.elseFilterExpression = staticmethod(QgsOgcUtils.elseFilterExpression)
except (NameError, AttributeError):
    pass
try:
    QgsOgcUtils.Context.__doc__ = """The Context struct stores the current layer and coordinate transform context.

.. versionadded:: 3.14"""
except (NameError, AttributeError):
    pass
