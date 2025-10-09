# The following has been generated automatically from src/gui/symbology/qgssymbollayerwidget.h
try:
    QgsSymbolLayerWidget.__attribute_docs__ = {'changed': 'Should be emitted whenever configuration changes happened on this symbol\nlayer configuration. If the subsymbol is changed,\n:py:func:`~QgsSymbolLayerWidget.symbolChanged` should be emitted\ninstead.\n', 'symbolChanged': 'Should be emitted whenever the sub symbol changed on this symbol layer\nconfiguration. Normally :py:func:`~QgsSymbolLayerWidget.changed` should\nbe preferred.\n\n.. seealso:: :py:func:`changed`\n'}
    QgsSymbolLayerWidget.__virtual_methods__ = ['setContext']
    QgsSymbolLayerWidget.__abstract_methods__ = ['setSymbolLayer', 'symbolLayer']
    QgsSymbolLayerWidget.__overridden_methods__ = ['createExpressionContext']
    QgsSymbolLayerWidget.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
