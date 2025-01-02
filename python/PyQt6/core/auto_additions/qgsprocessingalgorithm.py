# The following has been generated automatically from src/core/processing/qgsprocessingalgorithm.h
try:
    QgsProcessingAlgorithm.VectorProperties.__attribute_docs__ = {'fields': 'Fields', 'wkbType': 'Geometry (WKB) type', 'crs': 'Coordinate Reference System', 'availability': 'Availability of the properties. By default properties are not available.'}
    QgsProcessingAlgorithm.VectorProperties.__doc__ = """Properties of a vector source or sink used in an algorithm.

.. versionadded:: 3.14"""
    QgsProcessingAlgorithm.VectorProperties.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingAlgorithm.invalidSourceError = staticmethod(QgsProcessingAlgorithm.invalidSourceError)
    QgsProcessingAlgorithm.invalidRasterError = staticmethod(QgsProcessingAlgorithm.invalidRasterError)
    QgsProcessingAlgorithm.invalidSinkError = staticmethod(QgsProcessingAlgorithm.invalidSinkError)
    QgsProcessingAlgorithm.invalidPointCloudError = staticmethod(QgsProcessingAlgorithm.invalidPointCloudError)
    QgsProcessingAlgorithm.writeFeatureError = staticmethod(QgsProcessingAlgorithm.writeFeatureError)
    QgsProcessingAlgorithm.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingFeatureBasedAlgorithm.__group__ = ['processing']
except (NameError, AttributeError):
    pass
