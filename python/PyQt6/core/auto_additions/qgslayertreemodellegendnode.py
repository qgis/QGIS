# The following has been generated automatically from src/core/layertree/qgslayertreemodellegendnode.h
QgsLayerTreeModelLegendNode.CustomRole.baseClass = QgsLayerTreeModelLegendNode
try:
    QgsLayerTreeModelLegendNode.__attribute_docs__ = {'dataChanged': 'Emitted on internal data change so the layer tree model can forward the\nsignal to views\n', 'sizeChanged': 'Emitted when the size of this node changes.\n\n.. versionadded:: 3.16\n'}
    QgsLayerTreeModelLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsSymbolLegendNode.__overridden_methods__ = ['flags', 'data', 'setData', 'drawSymbol', 'exportSymbolToJson', 'setEmbeddedInParent', 'setUserLabel', 'isScaleOK', 'invalidateMapBasedData']
    QgsSymbolLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsSimpleLegendNode.__overridden_methods__ = ['data']
    QgsSimpleLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsImageLegendNode.__overridden_methods__ = ['data', 'drawSymbol', 'exportSymbolToJson']
    QgsImageLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsRasterSymbolLegendNode.__overridden_methods__ = ['flags', 'data', 'setData', 'drawSymbol', 'exportSymbolToJson']
    QgsRasterSymbolLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsWmsLegendNode.__overridden_methods__ = ['data', 'drawSymbol', 'exportSymbolToJson', 'invalidateMapBasedData']
    QgsWmsLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsDataDefinedSizeLegendNode.__overridden_methods__ = ['data', 'draw']
    QgsDataDefinedSizeLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLabelLegendNode.__overridden_methods__ = ['data', 'drawSymbol', 'exportSymbolToJson']
    QgsVectorLabelLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
