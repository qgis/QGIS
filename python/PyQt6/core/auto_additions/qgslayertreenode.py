# The following has been generated automatically from src/core/layertree/qgslayertreenode.h
QgsLayerTreeNode.NodeGroup = QgsLayerTreeNode.NodeType.NodeGroup
QgsLayerTreeNode.NodeLayer = QgsLayerTreeNode.NodeType.NodeLayer
try:
    QgsLayerTreeNode.__attribute_docs__ = {'willAddChildren': 'Emitted when one or more nodes will be added to a node within the tree\n', 'addedChildren': 'Emitted when one or more nodes have been added to a node within the tree\n', 'willRemoveChildren': 'Emitted when one or more nodes will be removed from a node within the\ntree\n', 'removedChildren': 'Emitted when one or more nodes has been removed from a node within the\ntree\n', 'visibilityChanged': 'Emitted when check state of a node within the tree has been changed\n', 'customPropertyChanged': 'Emitted when a custom property of a node within the tree has been\nchanged or removed\n', 'expandedChanged': 'Emitted when the collapsed/expanded state of a node within the tree has\nbeen changed\n', 'nameChanged': 'Emitted when the name of the node is changed\n'}
    QgsLayerTreeNode.readXml = staticmethod(QgsLayerTreeNode.readXml)
    QgsLayerTreeNode.__virtual_methods__ = ['setItemVisibilityCheckedRecursive']
    QgsLayerTreeNode.__abstract_methods__ = ['name', 'setName', 'writeXml', 'dump', 'clone', 'resolveReferences']
    QgsLayerTreeNode.__signal_arguments__ = {'willAddChildren': ['node: QgsLayerTreeNode', 'indexFrom: int', 'indexTo: int'], 'addedChildren': ['node: QgsLayerTreeNode', 'indexFrom: int', 'indexTo: int'], 'willRemoveChildren': ['node: QgsLayerTreeNode', 'indexFrom: int', 'indexTo: int'], 'removedChildren': ['node: QgsLayerTreeNode', 'indexFrom: int', 'indexTo: int'], 'visibilityChanged': ['node: QgsLayerTreeNode'], 'customPropertyChanged': ['node: QgsLayerTreeNode', 'key: str'], 'expandedChanged': ['node: QgsLayerTreeNode', 'expanded: bool'], 'nameChanged': ['node: QgsLayerTreeNode', 'name: str']}
    QgsLayerTreeNode.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
