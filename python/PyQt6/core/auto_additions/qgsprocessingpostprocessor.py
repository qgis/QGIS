# The following has been generated automatically from src/core/processing/qgsprocessingpostprocessor.h
try:
    QgsProcessingResultsHandler.ResultLayerDetails.__attribute_docs__ = {'layer': 'Associated map layer.', 'targetLayerTreeGroup': 'Optional target layer tree group, where the layer should be placed.', 'sortKey': 'Sort order key for ordering output layers in the layer tree.', 'destinationProject': 'Destination QGIS project.'}
    QgsProcessingResultsHandler.ResultLayerDetails.__annotations__ = {'layer': 'QgsMapLayer', 'targetLayerTreeGroup': 'QgsLayerTreeGroup', 'sortKey': int, 'destinationProject': 'QgsProject'}
    QgsProcessingResultsHandler.ResultLayerDetails.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingResultsHandler.determineOutputName = staticmethod(QgsProcessingResultsHandler.determineOutputName)
    QgsProcessingResultsHandler.layerTreeResultsGroup = staticmethod(QgsProcessingResultsHandler.layerTreeResultsGroup)
    QgsProcessingResultsHandler.configureResultLayerTreeLayer = staticmethod(QgsProcessingResultsHandler.configureResultLayerTreeLayer)
    QgsProcessingResultsHandler.addResultLayers = staticmethod(QgsProcessingResultsHandler.addResultLayers)
    QgsProcessingResultsHandler.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingLayerPostProcessor.__virtual_methods__ = ['postProcessLayer']
    QgsProcessingLayerPostProcessor.__group__ = ['processing']
except (NameError, AttributeError):
    pass
