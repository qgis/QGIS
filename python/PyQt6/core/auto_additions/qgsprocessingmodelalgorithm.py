# The following has been generated automatically from src/core/processing/models/qgsprocessingmodelalgorithm.h
try:
    QgsProcessingModelAlgorithm.VariableDefinition.__attribute_docs__ = {'value': 'Value of variable', 'source': "Original source of variable's value", 'description': 'Translated description of variable'}
    QgsProcessingModelAlgorithm.VariableDefinition.__annotations__ = {'value': 'object', 'source': 'QgsProcessingModelChildParameterSource', 'description': str}
    QgsProcessingModelAlgorithm.VariableDefinition.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingModelAlgorithm.safeName = staticmethod(QgsProcessingModelAlgorithm.safeName)
    QgsProcessingModelAlgorithm.__overridden_methods__ = ['initAlgorithm', 'flags', 'name', 'displayName', 'group', 'groupId', 'icon', 'svgIconPath', 'shortHelpString', 'shortDescription', 'helpUrl', 'canExecute', 'asPythonCommand', 'createExpressionContext', 'createInstance', 'processAlgorithm']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsProcessingModelAlgorithm_updateModelParameter = QgsProcessingModelAlgorithm.updateModelParameter
    def __QgsProcessingModelAlgorithm_updateModelParameter_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProcessingModelAlgorithm_updateModelParameter(self, arg)
    QgsProcessingModelAlgorithm.updateModelParameter = _functools.update_wrapper(__QgsProcessingModelAlgorithm_updateModelParameter_wrapper, QgsProcessingModelAlgorithm.updateModelParameter)

    QgsProcessingModelAlgorithm.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
