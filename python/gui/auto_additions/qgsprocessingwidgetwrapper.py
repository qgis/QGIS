# The following has been generated automatically from src/gui/processing/qgsprocessingwidgetwrapper.h
# monkey patching scoped based enum
QgsProcessingParametersGenerator.Flag.SkipDefaultValueParameters.__doc__ = "Parameters which are unchanged from their default values should not be included"
QgsProcessingParametersGenerator.Flag.__doc__ = """Flags controlling parameter generation.

.. versionadded:: 3.24

* ``SkipDefaultValueParameters``: Parameters which are unchanged from their default values should not be included

"""
# --
try:
    QgsAbstractProcessingParameterWidgetWrapper.__attribute_docs__ = {'widgetValueHasChanged': 'Emitted whenever the parameter value (as defined by the wrapped widget) is changed.\n'}
    QgsAbstractProcessingParameterWidgetWrapper.__signal_arguments__ = {'widgetValueHasChanged': ['wrapper: QgsAbstractProcessingParameterWidgetWrapper']}
    QgsAbstractProcessingParameterWidgetWrapper.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingContextGenerator.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParametersGenerator.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterWidgetContext.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingParameterWidgetFactoryInterface.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingHiddenWidgetWrapper.__group__ = ['processing']
except (NameError, AttributeError):
    pass
