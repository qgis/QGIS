# The following has been generated automatically from src/core/layertree/qgslayertreemodellegendnode.h
QgsLayerTreeModelLegendNode.LegendNodeRoles = QgsLayerTreeModelLegendNode.CustomRole
# monkey patching scoped based enum
QgsLayerTreeModelLegendNode.RuleKeyRole = QgsLayerTreeModelLegendNode.CustomRole.RuleKey
QgsLayerTreeModelLegendNode.LegendNodeRoles.RuleKeyRole = QgsLayerTreeModelLegendNode.CustomRole.RuleKey
QgsLayerTreeModelLegendNode.RuleKeyRole.is_monkey_patched = True
QgsLayerTreeModelLegendNode.RuleKeyRole.__doc__ = "Rule key of the node (QString)"
QgsLayerTreeModelLegendNode.ParentRuleKeyRole = QgsLayerTreeModelLegendNode.CustomRole.ParentRuleKey
QgsLayerTreeModelLegendNode.LegendNodeRoles.ParentRuleKeyRole = QgsLayerTreeModelLegendNode.CustomRole.ParentRuleKey
QgsLayerTreeModelLegendNode.ParentRuleKeyRole.is_monkey_patched = True
QgsLayerTreeModelLegendNode.ParentRuleKeyRole.__doc__ = "Rule key of the parent legend node - for legends with tree hierarchy (QString). Added in 2.8"
QgsLayerTreeModelLegendNode.NodeTypeRole = QgsLayerTreeModelLegendNode.CustomRole.NodeType
QgsLayerTreeModelLegendNode.LegendNodeRoles.NodeTypeRole = QgsLayerTreeModelLegendNode.CustomRole.NodeType
QgsLayerTreeModelLegendNode.NodeTypeRole.is_monkey_patched = True
QgsLayerTreeModelLegendNode.NodeTypeRole.__doc__ = "Type of node. Added in 3.16"
QgsLayerTreeModelLegendNode.IsDataDefinedSizeRole = QgsLayerTreeModelLegendNode.CustomRole.IsDataDefinedSize
QgsLayerTreeModelLegendNode.LegendNodeRoles.IsDataDefinedSizeRole = QgsLayerTreeModelLegendNode.CustomRole.IsDataDefinedSize
QgsLayerTreeModelLegendNode.IsDataDefinedSizeRole.is_monkey_patched = True
QgsLayerTreeModelLegendNode.IsDataDefinedSizeRole.__doc__ = "Set when a node is related to data defined size (title or separated legend items). Added in 3.38"
QgsLayerTreeModelLegendNode.CustomRole.__doc__ = """Legend node data roles

.. note::

   Prior to QGIS 3.36 this was available as QgsLayerTreeModelLegendNode.LegendNodeRoles

.. versionadded:: 3.36

* ``RuleKey``: Rule key of the node (QString)

  Available as ``QgsLayerTreeModelLegendNode.RuleKeyRole`` in older QGIS releases.

* ``ParentRuleKey``: Rule key of the parent legend node - for legends with tree hierarchy (QString). Added in 2.8

  Available as ``QgsLayerTreeModelLegendNode.ParentRuleKeyRole`` in older QGIS releases.

* ``NodeType``: Type of node. Added in 3.16

  Available as ``QgsLayerTreeModelLegendNode.NodeTypeRole`` in older QGIS releases.

* ``IsDataDefinedSize``: Set when a node is related to data defined size (title or separated legend items). Added in 3.38

  Available as ``QgsLayerTreeModelLegendNode.IsDataDefinedSizeRole`` in older QGIS releases.


"""
# --
QgsLayerTreeModelLegendNode.CustomRole.baseClass = QgsLayerTreeModelLegendNode
QgsLayerTreeModelLegendNode.SimpleLegend = QgsLayerTreeModelLegendNode.NodeTypes.SimpleLegend
QgsLayerTreeModelLegendNode.SymbolLegend = QgsLayerTreeModelLegendNode.NodeTypes.SymbolLegend
QgsLayerTreeModelLegendNode.RasterSymbolLegend = QgsLayerTreeModelLegendNode.NodeTypes.RasterSymbolLegend
QgsLayerTreeModelLegendNode.ImageLegend = QgsLayerTreeModelLegendNode.NodeTypes.ImageLegend
QgsLayerTreeModelLegendNode.WmsLegend = QgsLayerTreeModelLegendNode.NodeTypes.WmsLegend
QgsLayerTreeModelLegendNode.DataDefinedSizeLegend = QgsLayerTreeModelLegendNode.NodeTypes.DataDefinedSizeLegend
QgsLayerTreeModelLegendNode.EmbeddedWidget = QgsLayerTreeModelLegendNode.NodeTypes.EmbeddedWidget
QgsLayerTreeModelLegendNode.ColorRampLegend = QgsLayerTreeModelLegendNode.NodeTypes.ColorRampLegend
try:
    QgsLayerTreeModelLegendNode.ItemContext.__attribute_docs__ = {'context': 'Render context, if available', 'painter': 'Painter', 'point': 'Top-left corner of the legend item.\n\n.. deprecated:: 3.40\n\n   Use top, columnLeft, columnRight instead.', 'labelXOffset': 'Offset from the left side where label should start.\n\n.. deprecated:: 3.40\n\n   Use columnLeft, columnRight instead.', 'top': 'Top y-position of legend item.\n\n.. versionadded:: 3.10', 'columnLeft': 'Left side of current legend column. This should be used when determining\nwhere to render legend item content, correctly respecting the symbol and text\nalignment from the legend settings.\n\n.. versionadded:: 3.10', 'columnRight': 'Right side of current legend column. This should be used when determining\nwhere to render legend item content, correctly respecting the symbol and text\nalignment from the legend settings.\n\n.. versionadded:: 3.10', 'maxSiblingSymbolWidth': 'Largest symbol width, considering all other sibling legend components associated with\nthe current component.\n\n.. versionadded:: 3.10', 'patchShape': 'The patch shape to render for the node.\n\n.. versionadded:: 3.14', 'patchSize': 'Symbol patch size to render for the node.\n\nIf either the width or height are zero, then the default width/height from :py:func:`QgsLegendSettings.symbolSize()` should be used instead.\n\n.. versionadded:: 3.14', 'textDocument': 'Optional text document\n\n.. versionadded:: 3.30', 'textDocumentMetrics': 'Optional text document metrics.\n\n.. versionadded:: 3.30', 'screenProperties': 'Destination screen properties.\n\n.. versionadded:: 3.32'}
    QgsLayerTreeModelLegendNode.ItemContext.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsLayerTreeModelLegendNode.__attribute_docs__ = {'dataChanged': 'Emitted on internal data change so the layer tree model can forward the signal to views\n', 'sizeChanged': 'Emitted when the size of this node changes.\n\n.. versionadded:: 3.16\n'}
    QgsLayerTreeModelLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsLayerTreeModelLegendNode.ItemMetrics.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsSymbolLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsSimpleLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsImageLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsRasterSymbolLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsWmsLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsDataDefinedSizeLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLabelLegendNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
