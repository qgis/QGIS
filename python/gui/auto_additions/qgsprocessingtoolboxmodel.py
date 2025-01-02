# The following has been generated automatically from src/gui/processing/qgsprocessingtoolboxmodel.h
# monkey patching scoped based enum
QgsProcessingToolboxModelNode.NodeProvider = QgsProcessingToolboxModelNode.NodeType.Provider
QgsProcessingToolboxModelNode.NodeType.NodeProvider = QgsProcessingToolboxModelNode.NodeType.Provider
QgsProcessingToolboxModelNode.NodeProvider.is_monkey_patched = True
QgsProcessingToolboxModelNode.NodeProvider.__doc__ = "Provider node"
QgsProcessingToolboxModelNode.NodeGroup = QgsProcessingToolboxModelNode.NodeType.Group
QgsProcessingToolboxModelNode.NodeType.NodeGroup = QgsProcessingToolboxModelNode.NodeType.Group
QgsProcessingToolboxModelNode.NodeGroup.is_monkey_patched = True
QgsProcessingToolboxModelNode.NodeGroup.__doc__ = "Group node"
QgsProcessingToolboxModelNode.NodeAlgorithm = QgsProcessingToolboxModelNode.NodeType.Algorithm
QgsProcessingToolboxModelNode.NodeType.NodeAlgorithm = QgsProcessingToolboxModelNode.NodeType.Algorithm
QgsProcessingToolboxModelNode.NodeAlgorithm.is_monkey_patched = True
QgsProcessingToolboxModelNode.NodeAlgorithm.__doc__ = "Algorithm node"
QgsProcessingToolboxModelNode.NodeRecent = QgsProcessingToolboxModelNode.NodeType.Recent
QgsProcessingToolboxModelNode.NodeType.NodeRecent = QgsProcessingToolboxModelNode.NodeType.Recent
QgsProcessingToolboxModelNode.NodeRecent.is_monkey_patched = True
QgsProcessingToolboxModelNode.NodeRecent.__doc__ = "Recent algorithms node"
QgsProcessingToolboxModelNode.Favorite = QgsProcessingToolboxModelNode.NodeType.Favorite
QgsProcessingToolboxModelNode.Favorite.is_monkey_patched = True
QgsProcessingToolboxModelNode.Favorite.__doc__ = "Favorites algorithms node, since QGIS 3.40"
QgsProcessingToolboxModelNode.NodeType.__doc__ = """Enumeration of possible model node types

* ``Provider``: Provider node

  Available as ``QgsProcessingToolboxModelNode.NodeProvider`` in older QGIS releases.

* ``Group``: Group node

  Available as ``QgsProcessingToolboxModelNode.NodeGroup`` in older QGIS releases.

* ``Algorithm``: Algorithm node

  Available as ``QgsProcessingToolboxModelNode.NodeAlgorithm`` in older QGIS releases.

* ``Recent``: Recent algorithms node

  Available as ``QgsProcessingToolboxModelNode.NodeRecent`` in older QGIS releases.

* ``Favorite``: Favorites algorithms node, since QGIS 3.40

"""
# --
QgsProcessingToolboxModelNode.NodeType.baseClass = QgsProcessingToolboxModelNode
QgsProcessingToolboxModel.Roles = QgsProcessingToolboxModel.CustomRole
# monkey patching scoped based enum
QgsProcessingToolboxModel.RoleNodeType = QgsProcessingToolboxModel.CustomRole.NodeType
QgsProcessingToolboxModel.Roles.RoleNodeType = QgsProcessingToolboxModel.CustomRole.NodeType
QgsProcessingToolboxModel.RoleNodeType.is_monkey_patched = True
QgsProcessingToolboxModel.RoleNodeType.__doc__ = "Corresponds to the node's type"
QgsProcessingToolboxModel.RoleAlgorithmFlags = QgsProcessingToolboxModel.CustomRole.AlgorithmFlags
QgsProcessingToolboxModel.Roles.RoleAlgorithmFlags = QgsProcessingToolboxModel.CustomRole.AlgorithmFlags
QgsProcessingToolboxModel.RoleAlgorithmFlags.is_monkey_patched = True
QgsProcessingToolboxModel.RoleAlgorithmFlags.__doc__ = "Returns the node's algorithm flags, for algorithm nodes"
QgsProcessingToolboxModel.RoleAlgorithmId = QgsProcessingToolboxModel.CustomRole.AlgorithmId
QgsProcessingToolboxModel.Roles.RoleAlgorithmId = QgsProcessingToolboxModel.CustomRole.AlgorithmId
QgsProcessingToolboxModel.RoleAlgorithmId.is_monkey_patched = True
QgsProcessingToolboxModel.RoleAlgorithmId.__doc__ = "Algorithm ID, for algorithm nodes"
QgsProcessingToolboxModel.RoleAlgorithmName = QgsProcessingToolboxModel.CustomRole.AlgorithmName
QgsProcessingToolboxModel.Roles.RoleAlgorithmName = QgsProcessingToolboxModel.CustomRole.AlgorithmName
QgsProcessingToolboxModel.RoleAlgorithmName.is_monkey_patched = True
QgsProcessingToolboxModel.RoleAlgorithmName.__doc__ = "Untranslated algorithm name, for algorithm nodes"
QgsProcessingToolboxModel.RoleAlgorithmShortDescription = QgsProcessingToolboxModel.CustomRole.AlgorithmShortDescription
QgsProcessingToolboxModel.Roles.RoleAlgorithmShortDescription = QgsProcessingToolboxModel.CustomRole.AlgorithmShortDescription
QgsProcessingToolboxModel.RoleAlgorithmShortDescription.is_monkey_patched = True
QgsProcessingToolboxModel.RoleAlgorithmShortDescription.__doc__ = "Short algorithm description, for algorithm nodes"
QgsProcessingToolboxModel.RoleAlgorithmTags = QgsProcessingToolboxModel.CustomRole.AlgorithmTags
QgsProcessingToolboxModel.Roles.RoleAlgorithmTags = QgsProcessingToolboxModel.CustomRole.AlgorithmTags
QgsProcessingToolboxModel.RoleAlgorithmTags.is_monkey_patched = True
QgsProcessingToolboxModel.RoleAlgorithmTags.__doc__ = "List of algorithm tags, for algorithm nodes"
QgsProcessingToolboxModel.RoleProviderFlags = QgsProcessingToolboxModel.CustomRole.ProviderFlags
QgsProcessingToolboxModel.Roles.RoleProviderFlags = QgsProcessingToolboxModel.CustomRole.ProviderFlags
QgsProcessingToolboxModel.RoleProviderFlags.is_monkey_patched = True
QgsProcessingToolboxModel.RoleProviderFlags.__doc__ = "Returns the node's provider flags"
QgsProcessingToolboxModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsProcessingToolboxModel.Roles

.. versionadded:: 3.36

* ``NodeType``: Corresponds to the node's type

  Available as ``QgsProcessingToolboxModel.RoleNodeType`` in older QGIS releases.

* ``AlgorithmFlags``: Returns the node's algorithm flags, for algorithm nodes

  Available as ``QgsProcessingToolboxModel.RoleAlgorithmFlags`` in older QGIS releases.

* ``AlgorithmId``: Algorithm ID, for algorithm nodes

  Available as ``QgsProcessingToolboxModel.RoleAlgorithmId`` in older QGIS releases.

* ``AlgorithmName``: Untranslated algorithm name, for algorithm nodes

  Available as ``QgsProcessingToolboxModel.RoleAlgorithmName`` in older QGIS releases.

* ``AlgorithmShortDescription``: Short algorithm description, for algorithm nodes

  Available as ``QgsProcessingToolboxModel.RoleAlgorithmShortDescription`` in older QGIS releases.

* ``AlgorithmTags``: List of algorithm tags, for algorithm nodes

  Available as ``QgsProcessingToolboxModel.RoleAlgorithmTags`` in older QGIS releases.

* ``ProviderFlags``: Returns the node's provider flags

  Available as ``QgsProcessingToolboxModel.RoleProviderFlags`` in older QGIS releases.


"""
# --
QgsProcessingToolboxModel.CustomRole.baseClass = QgsProcessingToolboxModel
# monkey patching scoped based enum
QgsProcessingToolboxProxyModel.FilterToolbox = QgsProcessingToolboxProxyModel.Filter.Toolbox
QgsProcessingToolboxProxyModel.Filter.FilterToolbox = QgsProcessingToolboxProxyModel.Filter.Toolbox
QgsProcessingToolboxProxyModel.FilterToolbox.is_monkey_patched = True
QgsProcessingToolboxProxyModel.FilterToolbox.__doc__ = "Filters out any algorithms and content which should not be shown in the toolbox"
QgsProcessingToolboxProxyModel.FilterModeler = QgsProcessingToolboxProxyModel.Filter.Modeler
QgsProcessingToolboxProxyModel.Filter.FilterModeler = QgsProcessingToolboxProxyModel.Filter.Modeler
QgsProcessingToolboxProxyModel.FilterModeler.is_monkey_patched = True
QgsProcessingToolboxProxyModel.FilterModeler.__doc__ = "Filters out any algorithms and content which should not be shown in the modeler"
QgsProcessingToolboxProxyModel.FilterInPlace = QgsProcessingToolboxProxyModel.Filter.InPlace
QgsProcessingToolboxProxyModel.Filter.FilterInPlace = QgsProcessingToolboxProxyModel.Filter.InPlace
QgsProcessingToolboxProxyModel.FilterInPlace.is_monkey_patched = True
QgsProcessingToolboxProxyModel.FilterInPlace.__doc__ = "Only show algorithms which support in-place edits"
QgsProcessingToolboxProxyModel.FilterShowKnownIssues = QgsProcessingToolboxProxyModel.Filter.ShowKnownIssues
QgsProcessingToolboxProxyModel.Filter.FilterShowKnownIssues = QgsProcessingToolboxProxyModel.Filter.ShowKnownIssues
QgsProcessingToolboxProxyModel.FilterShowKnownIssues.is_monkey_patched = True
QgsProcessingToolboxProxyModel.FilterShowKnownIssues.__doc__ = "Show algorithms with known issues (hidden by default)"
QgsProcessingToolboxProxyModel.Filter.__doc__ = """Available filter flags for filtering the model

* ``Toolbox``: Filters out any algorithms and content which should not be shown in the toolbox

  Available as ``QgsProcessingToolboxProxyModel.FilterToolbox`` in older QGIS releases.

* ``Modeler``: Filters out any algorithms and content which should not be shown in the modeler

  Available as ``QgsProcessingToolboxProxyModel.FilterModeler`` in older QGIS releases.

* ``InPlace``: Only show algorithms which support in-place edits

  Available as ``QgsProcessingToolboxProxyModel.FilterInPlace`` in older QGIS releases.

* ``ShowKnownIssues``: Show algorithms with known issues (hidden by default)

  Available as ``QgsProcessingToolboxProxyModel.FilterShowKnownIssues`` in older QGIS releases.


"""
# --
QgsProcessingToolboxProxyModel.Filter.baseClass = QgsProcessingToolboxProxyModel
QgsProcessingToolboxProxyModel.Filters.baseClass = QgsProcessingToolboxProxyModel
Filters = QgsProcessingToolboxProxyModel  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsProcessingToolboxModel.__attribute_docs__ = {'recentAlgorithmAdded': 'Emitted whenever recent algorithms are added to the model.\n', 'favoriteAlgorithmAdded': 'Emitted whenever favorite algorithms are added to the model.\n'}
    QgsProcessingToolboxModel.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingToolboxModelNode.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingToolboxModelRecentNode.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingToolboxModelFavoriteNode.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingToolboxModelProviderNode.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingToolboxModelGroupNode.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingToolboxModelAlgorithmNode.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingToolboxProxyModel.__group__ = ['processing']
except (NameError, AttributeError):
    pass
