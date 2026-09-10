# The following has been generated automatically from src/core/qgsmaplayerutils.h
try:
    QgsSaveStyleResult.__attribute_docs__ = {'saveResult': 'Flags representing which style formats (QML and/or SLD) were successfully saved.', 'sldErrorMessages': 'List of error messages generated while exporting the style to SLD.', 'sldWarningMessages': 'List of warning messages generated while exporting the style to SLD.', 'providerSaveStyleError': 'Error message reported by the data provider when saving the style, or an empty string if no error occurred.', 'qmlError': 'Error message generated while saving the style as QML, or an empty string if no error occurred.'}
    QgsSaveStyleResult.__annotations__ = {'saveResult': 'QgsMapLayer.SaveStyleResults', 'sldErrorMessages': 'List[str]', 'sldWarningMessages': 'List[str]', 'providerSaveStyleError': str, 'qmlError': str}
except (NameError, AttributeError):
    pass
try:
    QgsMapLayerUtils.combinedExtent = staticmethod(QgsMapLayerUtils.combinedExtent)
    QgsMapLayerUtils.databaseConnection = staticmethod(QgsMapLayerUtils.databaseConnection)
    QgsMapLayerUtils.layerSourceMatchesPath = staticmethod(QgsMapLayerUtils.layerSourceMatchesPath)
    QgsMapLayerUtils.layerRefersToUri = staticmethod(QgsMapLayerUtils.layerRefersToUri)
    QgsMapLayerUtils.updateLayerSourcePath = staticmethod(QgsMapLayerUtils.updateLayerSourcePath)
    QgsMapLayerUtils.sortLayersByType = staticmethod(QgsMapLayerUtils.sortLayersByType)
    QgsMapLayerUtils.launderLayerName = staticmethod(QgsMapLayerUtils.launderLayerName)
    QgsMapLayerUtils.isOpenStreetMapLayer = staticmethod(QgsMapLayerUtils.isOpenStreetMapLayer)
    QgsMapLayerUtils.isOpenStreetMapUri = staticmethod(QgsMapLayerUtils.isOpenStreetMapUri)
    QgsMapLayerUtils.layerTypeToString = staticmethod(QgsMapLayerUtils.layerTypeToString)
    QgsMapLayerUtils.layerToolTip = staticmethod(QgsMapLayerUtils.layerToolTip)
    QgsMapLayerUtils.saveLayerStyleToDatabase = staticmethod(QgsMapLayerUtils.saveLayerStyleToDatabase)
except (NameError, AttributeError):
    pass
