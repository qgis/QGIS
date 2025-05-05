# The following has been generated automatically from src/core/processing/qgsprocessingalgorithm.h
try:
    QgsProcessingAlgorithm.VectorProperties.__attribute_docs__ = {'fields': 'Fields', 'wkbType': 'Geometry (WKB) type', 'crs': 'Coordinate Reference System', 'availability': 'Availability of the properties. By default properties are not available.'}
    QgsProcessingAlgorithm.VectorProperties.__annotations__ = {'fields': 'QgsFields', 'wkbType': 'Qgis.WkbType', 'crs': 'QgsCoordinateReferenceSystem', 'availability': 'Qgis.ProcessingPropertyAvailability'}
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
    QgsProcessingAlgorithm.__virtual_methods__ = ['shortDescription', 'tags', 'shortHelpString', 'helpString', 'helpUrl', 'documentationFlags', 'icon', 'svgIconPath', 'group', 'groupId', 'flags', 'canExecute', 'checkParameterValues', 'preprocessParameters', 'autogenerateParameterValues', 'sinkProperties', 'createCustomParametersWidget', 'createExpressionContext', 'validateInputCrs', 'asPythonCommand', 'asQgisProcessCommand', 'asMap', 'supportInPlaceEdit', 'prepareAlgorithm', 'postProcessAlgorithm']
    QgsProcessingAlgorithm.__abstract_methods__ = ['name', 'displayName', 'createInstance', 'initAlgorithm', 'processAlgorithm']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsProcessingAlgorithm_addOutput = QgsProcessingAlgorithm.addOutput
    def __QgsProcessingAlgorithm_addOutput_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProcessingAlgorithm_addOutput(self, arg)
    QgsProcessingAlgorithm.addOutput = _functools.update_wrapper(__QgsProcessingAlgorithm_addOutput_wrapper, QgsProcessingAlgorithm.addOutput)

    QgsProcessingAlgorithm.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingFeatureBasedAlgorithm.__virtual_methods__ = ['inputParameterName', 'inputParameterDescription', 'inputLayerTypes', 'outputLayerType', 'sourceFlags', 'sinkFlags', 'outputWkbType', 'outputFields', 'outputCrs', 'initParameters', 'request']
    QgsProcessingFeatureBasedAlgorithm.__abstract_methods__ = ['processFeature', 'outputName']
    QgsProcessingFeatureBasedAlgorithm.__overridden_methods__ = ['flags', 'supportInPlaceEdit', 'initAlgorithm', 'processAlgorithm', 'sinkProperties']
    QgsProcessingFeatureBasedAlgorithm.__group__ = ['processing']
except (NameError, AttributeError):
    pass
