# The following has been generated automatically from src/core/providers/arcgis/qgsarcgisrestutils.h
# monkey patching scoped based enum
QgsArcGisRestUtils.FeatureToJsonFlag.IncludeGeometry.__doc__ = "Whether to include the geometry definition"
QgsArcGisRestUtils.FeatureToJsonFlag.IncludeNonObjectIdAttributes.__doc__ = "Whether to include any non-objectId attributes"
QgsArcGisRestUtils.FeatureToJsonFlag.__doc__ = """Flags which control the behavior of converting features to JSON.

.. versionadded:: 3.28

* ``IncludeGeometry``: Whether to include the geometry definition
* ``IncludeNonObjectIdAttributes``: Whether to include any non-objectId attributes

"""
# --
QgsArcGisRestUtils.FeatureToJsonFlag.baseClass = QgsArcGisRestUtils
QgsArcGisRestUtils.FeatureToJsonFlags = lambda flags=0: QgsArcGisRestUtils.FeatureToJsonFlag(flags)
QgsArcGisRestUtils.FeatureToJsonFlags.baseClass = QgsArcGisRestUtils
FeatureToJsonFlags = QgsArcGisRestUtils  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsArcGisRestUtils.convertFieldType = staticmethod(QgsArcGisRestUtils.convertFieldType)
    QgsArcGisRestUtils.convertGeometryType = staticmethod(QgsArcGisRestUtils.convertGeometryType)
    QgsArcGisRestUtils.convertGeometry = staticmethod(QgsArcGisRestUtils.convertGeometry)
    QgsArcGisRestUtils.convertSpatialReference = staticmethod(QgsArcGisRestUtils.convertSpatialReference)
    QgsArcGisRestUtils.convertSymbol = staticmethod(QgsArcGisRestUtils.convertSymbol)
    QgsArcGisRestUtils.convertRenderer = staticmethod(QgsArcGisRestUtils.convertRenderer)
    QgsArcGisRestUtils.convertLabeling = staticmethod(QgsArcGisRestUtils.convertLabeling)
    QgsArcGisRestUtils.convertLabelingExpression = staticmethod(QgsArcGisRestUtils.convertLabelingExpression)
    QgsArcGisRestUtils.convertColor = staticmethod(QgsArcGisRestUtils.convertColor)
    QgsArcGisRestUtils.convertLineStyle = staticmethod(QgsArcGisRestUtils.convertLineStyle)
    QgsArcGisRestUtils.convertFillStyle = staticmethod(QgsArcGisRestUtils.convertFillStyle)
    QgsArcGisRestUtils.convertDateTime = staticmethod(QgsArcGisRestUtils.convertDateTime)
    QgsArcGisRestUtils.geometryToJson = staticmethod(QgsArcGisRestUtils.geometryToJson)
    QgsArcGisRestUtils.crsToJson = staticmethod(QgsArcGisRestUtils.crsToJson)
    QgsArcGisRestUtils.convertRectangle = staticmethod(QgsArcGisRestUtils.convertRectangle)
    QgsArcGisRestUtils.featureToJson = staticmethod(QgsArcGisRestUtils.featureToJson)
    QgsArcGisRestUtils.variantToAttributeValue = staticmethod(QgsArcGisRestUtils.variantToAttributeValue)
    QgsArcGisRestUtils.fieldDefinitionToJson = staticmethod(QgsArcGisRestUtils.fieldDefinitionToJson)
    QgsArcGisRestUtils.serviceTypeFromString = staticmethod(QgsArcGisRestUtils.serviceTypeFromString)
    QgsArcGisRestUtils.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
try:
    QgsArcGisRestContext.__group__ = ['providers', 'arcgis']
except (NameError, AttributeError):
    pass
