# The following has been generated automatically from src/gui/processing/qgsprocessingtoolboxmodel.h
QgsProcessingToolboxModelNode.NodeProvider = QgsProcessingToolboxModelNode.NodeType.NodeProvider
QgsProcessingToolboxModelNode.NodeGroup = QgsProcessingToolboxModelNode.NodeType.NodeGroup
QgsProcessingToolboxModelNode.NodeAlgorithm = QgsProcessingToolboxModelNode.NodeType.NodeAlgorithm
QgsProcessingToolboxModelNode.NodeRecent = QgsProcessingToolboxModelNode.NodeType.NodeRecent
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
QgsProcessingToolboxModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsProcessingToolboxModel.Roles\n\n.. versionadded:: 3.36\n\n" + '* ``RoleNodeType``: ' + QgsProcessingToolboxModel.CustomRole.NodeType.__doc__ + '\n' + '* ``RoleAlgorithmFlags``: ' + QgsProcessingToolboxModel.CustomRole.AlgorithmFlags.__doc__ + '\n' + '* ``RoleAlgorithmId``: ' + QgsProcessingToolboxModel.CustomRole.AlgorithmId.__doc__ + '\n' + '* ``RoleAlgorithmName``: ' + QgsProcessingToolboxModel.CustomRole.AlgorithmName.__doc__ + '\n' + '* ``RoleAlgorithmShortDescription``: ' + QgsProcessingToolboxModel.CustomRole.AlgorithmShortDescription.__doc__ + '\n' + '* ``RoleAlgorithmTags``: ' + QgsProcessingToolboxModel.CustomRole.AlgorithmTags.__doc__ + '\n' + '* ``RoleProviderFlags``: ' + QgsProcessingToolboxModel.CustomRole.ProviderFlags.__doc__
# --
QgsProcessingToolboxModel.CustomRole.baseClass = QgsProcessingToolboxModel
QgsProcessingToolboxProxyModel.FilterToolbox = QgsProcessingToolboxProxyModel.Filter.FilterToolbox
QgsProcessingToolboxProxyModel.FilterModeler = QgsProcessingToolboxProxyModel.Filter.FilterModeler
QgsProcessingToolboxProxyModel.FilterInPlace = QgsProcessingToolboxProxyModel.Filter.FilterInPlace
QgsProcessingToolboxProxyModel.FilterShowKnownIssues = QgsProcessingToolboxProxyModel.Filter.FilterShowKnownIssues
QgsProcessingToolboxProxyModel.Filters = lambda flags=0: QgsProcessingToolboxProxyModel.Filter(flags)
QgsProcessingToolboxProxyModel.Filters.baseClass = QgsProcessingToolboxProxyModel
Filters = QgsProcessingToolboxProxyModel  # dirty hack since SIP seems to introduce the flags in module
