# The following has been generated automatically from src/core/providers/qgsdataprovider.h
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
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsDataProvider.ReadFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsDataProvider.ReadFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsDataProvider.ReadFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsDataProvider.ReadFlag.__or__ = lambda flag1, flag2: QgsDataProvider.ReadFlag(_force_int(flag1) | _force_int(flag2))
