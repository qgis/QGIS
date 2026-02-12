# The following has been generated automatically from src/gui/symbology/qgsrendererwidget.h
try:
    QgsRendererWidget.__attribute_docs__ = {'layerVariablesChanged': 'Emitted when expression context variables on the associated vector\nlayers have been changed. Will request the parent dialog to\nre-synchronize with the variables.\n', 'symbolLevelsChanged': 'Emitted when the symbol levels settings have been changed.\n\n.. deprecated:: 3.20\n\n   No longer emitted.\n'}
    QgsRendererWidget.__virtual_methods__ = ['setContext', 'selectedSymbols', 'refreshSymbolView', 'setSymbolLevels', 'copy', 'paste', 'pasteSymbolToSelection', 'apply']
    QgsRendererWidget.__abstract_methods__ = ['renderer']
    QgsRendererWidget.__overridden_methods__ = ['createExpressionContext', 'setDockMode']
    QgsRendererWidget.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsDataDefinedValueDialog.__abstract_methods__ = ['symbolDataDefined', 'value', 'setDataDefined']
    QgsDataDefinedValueDialog.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsDataDefinedSizeDialog.__overridden_methods__ = ['symbolDataDefined', 'value', 'setDataDefined']
    QgsDataDefinedSizeDialog.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsDataDefinedRotationDialog.__overridden_methods__ = ['symbolDataDefined', 'value', 'setDataDefined']
    QgsDataDefinedRotationDialog.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
try:
    QgsDataDefinedWidthDialog.__overridden_methods__ = ['symbolDataDefined', 'value', 'setDataDefined']
    QgsDataDefinedWidthDialog.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
