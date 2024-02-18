# The following has been generated automatically from src/gui/qgsnewhttpconnection.h
QgsNewHttpConnection.ConnectionWfs = QgsNewHttpConnection.ConnectionType.ConnectionWfs
QgsNewHttpConnection.ConnectionWms = QgsNewHttpConnection.ConnectionType.ConnectionWms
QgsNewHttpConnection.ConnectionWcs = QgsNewHttpConnection.ConnectionType.ConnectionWcs
QgsNewHttpConnection.ConnectionOther = QgsNewHttpConnection.ConnectionType.ConnectionOther
QgsNewHttpConnection.ConnectionTypes = lambda flags=0: QgsNewHttpConnection.ConnectionType(flags)
QgsNewHttpConnection.FlagShowTestConnection = QgsNewHttpConnection.Flag.FlagShowTestConnection
QgsNewHttpConnection.FlagHideAuthenticationGroup = QgsNewHttpConnection.Flag.FlagHideAuthenticationGroup
QgsNewHttpConnection.FlagShowHttpSettings = QgsNewHttpConnection.Flag.FlagShowHttpSettings
QgsNewHttpConnection.Flags = lambda flags=0: QgsNewHttpConnection.Flag(flags)
QgsNewHttpConnection.WFS_VERSION_MAX = QgsNewHttpConnection.WfsVersionIndex.WFS_VERSION_MAX
QgsNewHttpConnection.WFS_VERSION_1_0 = QgsNewHttpConnection.WfsVersionIndex.WFS_VERSION_1_0
QgsNewHttpConnection.WFS_VERSION_1_1 = QgsNewHttpConnection.WfsVersionIndex.WFS_VERSION_1_1
QgsNewHttpConnection.WFS_VERSION_2_0 = QgsNewHttpConnection.WfsVersionIndex.WFS_VERSION_2_0
QgsNewHttpConnection.WFS_VERSION_API_FEATURES_1_0 = QgsNewHttpConnection.WfsVersionIndex.WFS_VERSION_API_FEATURES_1_0
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsNewHttpConnection.ConnectionType.__bool__ = lambda flag: bool(_force_int(flag))
QgsNewHttpConnection.ConnectionType.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsNewHttpConnection.ConnectionType.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsNewHttpConnection.ConnectionType.__or__ = lambda flag1, flag2: QgsNewHttpConnection.ConnectionType(_force_int(flag1) | _force_int(flag2))
QgsNewHttpConnection.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsNewHttpConnection.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsNewHttpConnection.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsNewHttpConnection.Flag.__or__ = lambda flag1, flag2: QgsNewHttpConnection.Flag(_force_int(flag1) | _force_int(flag2))
