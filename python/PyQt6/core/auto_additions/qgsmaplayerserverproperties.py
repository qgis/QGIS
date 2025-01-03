# The following has been generated automatically from src/core/qgsmaplayerserverproperties.h
QgsServerWmsDimensionProperties.TIME = QgsServerWmsDimensionProperties.PredefinedWmsDimensionName.TIME
QgsServerWmsDimensionProperties.DATE = QgsServerWmsDimensionProperties.PredefinedWmsDimensionName.DATE
QgsServerWmsDimensionProperties.ELEVATION = QgsServerWmsDimensionProperties.PredefinedWmsDimensionName.ELEVATION
QgsServerWmsDimensionProperties.PredefinedWmsDimensionName.baseClass = QgsServerWmsDimensionProperties
QgsServerWmsDimensionProperties.WmsDimensionInfo.AllValues = QgsServerWmsDimensionProperties.WmsDimensionInfo.DefaultDisplay.AllValues
QgsServerWmsDimensionProperties.WmsDimensionInfo.MinValue = QgsServerWmsDimensionProperties.WmsDimensionInfo.DefaultDisplay.MinValue
QgsServerWmsDimensionProperties.WmsDimensionInfo.MaxValue = QgsServerWmsDimensionProperties.WmsDimensionInfo.DefaultDisplay.MaxValue
QgsServerWmsDimensionProperties.WmsDimensionInfo.ReferenceValue = QgsServerWmsDimensionProperties.WmsDimensionInfo.DefaultDisplay.ReferenceValue
try:
    QgsServerMetadataUrlProperties.MetadataUrl.__attribute_docs__ = {'url': 'URL of the link', 'type': 'Link type. Suggested to use FGDC or TC211.', 'format': 'Format specification of online resource. It is strongly suggested to either use text/plain or text/xml.'}
    QgsServerMetadataUrlProperties.MetadataUrl.__doc__ = """MetadataUrl structure.
MetadataUrl is a link to the detailed, standardized metadata about the data."""
except (NameError, AttributeError):
    pass
try:
    QgsServerWmsDimensionProperties.wmsDimensionDefaultDisplayLabels = staticmethod(QgsServerWmsDimensionProperties.wmsDimensionDefaultDisplayLabels)
except (NameError, AttributeError):
    pass
try:
    QgsServerWmsDimensionProperties.WmsDimensionInfo.__doc__ = """Setting to define QGIS Server WMS Dimension.

.. versionadded:: 3.10"""
except (NameError, AttributeError):
    pass
