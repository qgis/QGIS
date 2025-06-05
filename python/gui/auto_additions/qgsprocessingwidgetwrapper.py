# The following has been generated automatically from src/gui/processing/qgsprocessingwidgetwrapper.h
# monkey patching scoped based enum
QgsProcessingParametersGenerator.Flag.SkipDefaultValueParameters.__doc__ = "Parameters which are unchanged from their default values should not be included"
QgsProcessingParametersGenerator.Flag.SkipValidation.__doc__ = "Skip validation of parameters. \n.. versionadded:: 3.44"
QgsProcessingParametersGenerator.Flag.__doc__ = """Flags controlling parameter generation.

.. versionadded:: 3.24

* ``SkipDefaultValueParameters``: Parameters which are unchanged from their default values should not be included
* ``SkipValidation``: Skip validation of parameters.

  .. versionadded:: 3.44


"""
# --
try:
    QgsAbstractProcessingParameterWidgetWrapper.__attribute_docs__ = {'widgetValueHasChanged': 'Emitted whenever the parameter value (as defined by the wrapped widget)\nis changed.\n'}
    QgsAbstractProcessingParameterWidgetWrapper.__virtual_methods__ = ['setWidgetContext', 'customProperties', 'registerProcessingContextGenerator', 'postInitialize', 'stretch', 'setDialog', 'createLabel']
    QgsAbstractProcessingParameterWidgetWrapper.__abstract_methods__ = ['createWidget', 'setWidgetValue', 'widgetValue']
    QgsAbstractProcessingParameterWidgetWrapper.__overridden_methods__ = ['createExpressionContext']
    QgsAbstractProcessingParameterWidgetWrapper.__signal_arguments__ = {'widgetValueHasChanged': ['wrapper: QgsAbstractProcessingParameterWidgetWrapper']}
    QgsAbstractProcessingParameterWidgetWrapper.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterWidgetFactoryInterface.__virtual_methods__ = ['createModelerWidgetWrapper', 'createParameterDefinitionWidget', 'compatibleParameterTypes', 'compatibleOutputTypes', 'compatibleDataTypes', 'modelerExpressionFormatString', 'defaultModelSource']
    QgsProcessingParameterWidgetFactoryInterface.__abstract_methods__ = ['parameterType', 'createWidgetWrapper']
    QgsProcessingParameterWidgetFactoryInterface.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingContextGenerator.__abstract_methods__ = ['processingContext']
    QgsProcessingContextGenerator.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParametersGenerator.__abstract_methods__ = ['createProcessingParameters']
    QgsProcessingParametersGenerator.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingHiddenWidgetWrapper.__overridden_methods__ = ['setWidgetValue', 'widgetValue', 'linkedVectorLayer', 'createWidget', 'createLabel']
    QgsProcessingHiddenWidgetWrapper.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterWidgetContext.__group__ = ['processing']
except (NameError, AttributeError):
    pass
