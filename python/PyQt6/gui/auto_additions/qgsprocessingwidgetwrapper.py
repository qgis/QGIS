# The following has been generated automatically from src/gui/processing/qgsprocessingwidgetwrapper.h
# monkey patching scoped based enum
QgsProcessingParametersGenerator.Flag.SkipDefaultValueParameters.__doc__ = "Parameters which are unchanged from their default values should not be included"
QgsProcessingParametersGenerator.Flag.__doc__ = "Flags controlling parameter generation.\n\n.. versionadded:: 3.24\n\n" + '* ``SkipDefaultValueParameters``: ' + QgsProcessingParametersGenerator.Flag.SkipDefaultValueParameters.__doc__
# --
QgsProcessingParametersGenerator.Flags = lambda flags=0: QgsProcessingParametersGenerator.Flag(flags)
QgsAbstractProcessingParameterWidgetWrapper.__attribute_docs__ = {'widgetValueHasChanged': 'Emitted whenever the parameter value (as defined by the wrapped widget) is changed.\n'}
