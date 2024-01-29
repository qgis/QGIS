# The following has been generated automatically from src/core/processing/qgsprocessingalgorithm.h
QgsProcessingAlgorithm.FlagHideFromToolbox = QgsProcessingAlgorithm.Flag.FlagHideFromToolbox
QgsProcessingAlgorithm.FlagHideFromModeler = QgsProcessingAlgorithm.Flag.FlagHideFromModeler
QgsProcessingAlgorithm.FlagSupportsBatch = QgsProcessingAlgorithm.Flag.FlagSupportsBatch
QgsProcessingAlgorithm.FlagCanCancel = QgsProcessingAlgorithm.Flag.FlagCanCancel
QgsProcessingAlgorithm.FlagRequiresMatchingCrs = QgsProcessingAlgorithm.Flag.FlagRequiresMatchingCrs
QgsProcessingAlgorithm.FlagNoThreading = QgsProcessingAlgorithm.Flag.FlagNoThreading
QgsProcessingAlgorithm.FlagDisplayNameIsLiteral = QgsProcessingAlgorithm.Flag.FlagDisplayNameIsLiteral
QgsProcessingAlgorithm.FlagSupportsInPlaceEdits = QgsProcessingAlgorithm.Flag.FlagSupportsInPlaceEdits
QgsProcessingAlgorithm.FlagKnownIssues = QgsProcessingAlgorithm.Flag.FlagKnownIssues
QgsProcessingAlgorithm.FlagCustomException = QgsProcessingAlgorithm.Flag.FlagCustomException
QgsProcessingAlgorithm.FlagPruneModelBranchesBasedOnAlgorithmResults = QgsProcessingAlgorithm.Flag.FlagPruneModelBranchesBasedOnAlgorithmResults
QgsProcessingAlgorithm.FlagSkipGenericModelLogging = QgsProcessingAlgorithm.Flag.FlagSkipGenericModelLogging
QgsProcessingAlgorithm.FlagNotAvailableInStandaloneTool = QgsProcessingAlgorithm.Flag.FlagNotAvailableInStandaloneTool
QgsProcessingAlgorithm.FlagRequiresProject = QgsProcessingAlgorithm.Flag.FlagRequiresProject
QgsProcessingAlgorithm.FlagDeprecated = QgsProcessingAlgorithm.Flag.FlagDeprecated
QgsProcessingAlgorithm.Flags = lambda flags=0: QgsProcessingAlgorithm.Flag(flags)
QgsProcessingAlgorithm.NotAvailable = QgsProcessingAlgorithm.PropertyAvailability.NotAvailable
QgsProcessingAlgorithm.Available = QgsProcessingAlgorithm.PropertyAvailability.Available
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsProcessingAlgorithm.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsProcessingAlgorithm.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProcessingAlgorithm.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProcessingAlgorithm.Flag.__or__ = lambda flag1, flag2: QgsProcessingAlgorithm.Flag(_force_int(flag1) | _force_int(flag2))
