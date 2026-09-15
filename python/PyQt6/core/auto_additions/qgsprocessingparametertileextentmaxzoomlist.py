# The following has been generated automatically from src/core/processing/qgsprocessingparametertileextentmaxzoomlist.h
try:
    QgsTileExtentMaxZoomRegion.__attribute_docs__ = {'extent': 'Bounding box extent and associated CRS', 'maxZoom': 'Maximum zoom level applied within this extent'}
    QgsTileExtentMaxZoomRegion.__annotations__ = {'extent': 'QgsReferencedRectangle', 'maxZoom': int}
    QgsTileExtentMaxZoomRegion.__doc__ = """Represents a spatial region paired with a custom maximum zoom level.

.. versionadded:: 4.4"""
    QgsTileExtentMaxZoomRegion.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterTileExtentMaxZoomList.typeName = staticmethod(QgsProcessingParameterTileExtentMaxZoomList.typeName)
    QgsProcessingParameterTileExtentMaxZoomList.toVariant = staticmethod(QgsProcessingParameterTileExtentMaxZoomList.toVariant)
    QgsProcessingParameterTileExtentMaxZoomList.maxZoomForTile = staticmethod(QgsProcessingParameterTileExtentMaxZoomList.maxZoomForTile)
    QgsProcessingParameterTileExtentMaxZoomList.__overridden_methods__ = ['clone', 'type', 'checkValueIsAcceptable', 'valueAsPythonString', 'valueAsString', 'valueAsJsonObject']
    QgsProcessingParameterTileExtentMaxZoomList.__group__ = ['processing']
except (NameError, AttributeError):
    pass
