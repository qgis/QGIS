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
QgsLayerTreeModelLegendNode.CustomRole.__doc__ = "Legend node data roles\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsLayerTreeModelLegendNode.LegendNodeRoles\n\n.. versionadded:: 3.36\n\n" + '* ``RuleKeyRole``: ' + QgsLayerTreeModelLegendNode.CustomRole.RuleKey.__doc__ + '\n' + '* ``ParentRuleKeyRole``: ' + QgsLayerTreeModelLegendNode.CustomRole.ParentRuleKey.__doc__ + '\n' + '* ``NodeTypeRole``: ' + QgsLayerTreeModelLegendNode.CustomRole.NodeType.__doc__ + '\n' + '* ``IsDataDefinedSizeRole``: ' + QgsLayerTreeModelLegendNode.CustomRole.IsDataDefinedSize.__doc__
# --
QgsLayerTreeModelLegendNode.CustomRole.baseClass = QgsLayerTreeModelLegendNode
