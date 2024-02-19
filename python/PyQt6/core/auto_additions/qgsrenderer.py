# The following has been generated automatically from src/core/symbology/qgsrenderer.h
QgsFeatureRenderer.SymbolLevels = QgsFeatureRenderer.Capability.SymbolLevels
QgsFeatureRenderer.MoreSymbolsPerFeature = QgsFeatureRenderer.Capability.MoreSymbolsPerFeature
QgsFeatureRenderer.Filter = QgsFeatureRenderer.Capability.Filter
QgsFeatureRenderer.ScaleDependent = QgsFeatureRenderer.Capability.ScaleDependent
QgsFeatureRenderer.Capabilities = lambda flags=0: QgsFeatureRenderer.Capability(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsFeatureRenderer.Capability.__bool__ = lambda flag: bool(_force_int(flag))
QgsFeatureRenderer.Capability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsFeatureRenderer.Capability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsFeatureRenderer.Capability.__or__ = lambda flag1, flag2: QgsFeatureRenderer.Capability(_force_int(flag1) | _force_int(flag2))
