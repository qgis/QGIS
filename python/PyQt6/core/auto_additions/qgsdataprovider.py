# The following has been generated automatically from src/core/providers/qgsdataprovider.h
QgsDataProvider.NoDataCapabilities = QgsDataProvider.DataCapability.NoDataCapabilities
QgsDataProvider.File = QgsDataProvider.DataCapability.File
QgsDataProvider.Dir = QgsDataProvider.DataCapability.Dir
QgsDataProvider.Database = QgsDataProvider.DataCapability.Database
QgsDataProvider.Net = QgsDataProvider.DataCapability.Net
QgsDataProvider.DataCapabilities = lambda flags=0: QgsDataProvider.DataCapability(flags)
QgsDataProvider.EvaluateDefaultValues = QgsDataProvider.ProviderProperty.EvaluateDefaultValues
QgsDataProvider.CustomData = QgsDataProvider.ProviderProperty.CustomData
QgsDataProvider.FlagTrustDataSource = QgsDataProvider.ReadFlag.FlagTrustDataSource
QgsDataProvider.SkipFeatureCount = QgsDataProvider.ReadFlag.SkipFeatureCount
QgsDataProvider.FlagLoadDefaultStyle = QgsDataProvider.ReadFlag.FlagLoadDefaultStyle
QgsDataProvider.SkipGetExtent = QgsDataProvider.ReadFlag.SkipGetExtent
QgsDataProvider.SkipFullScan = QgsDataProvider.ReadFlag.SkipFullScan
QgsDataProvider.ForceReadOnly = QgsDataProvider.ReadFlag.ForceReadOnly
QgsDataProvider.SkipCredentialsRequest = QgsDataProvider.ReadFlag.SkipCredentialsRequest
QgsDataProvider.ParallelThreadLoading = QgsDataProvider.ReadFlag.ParallelThreadLoading
QgsDataProvider.ReadFlags = lambda flags=0: QgsDataProvider.ReadFlag(flags)
def _force_int(v): return v if isinstance(v, int) else int(v.value)


QgsDataProvider.ReadFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsDataProvider.ReadFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsDataProvider.ReadFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsDataProvider.ReadFlag.__or__ = lambda flag1, flag2: QgsDataProvider.ReadFlag(_force_int(flag1) | _force_int(flag2))
