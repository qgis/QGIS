# The following has been generated automatically from src/core/qgsmaplayer.h
QgsMapLayer.Style = QgsMapLayer.PropertyType.Style
QgsMapLayer.Metadata = QgsMapLayer.PropertyType.Metadata
QgsMapLayer.Identifiable = QgsMapLayer.LayerFlag.Identifiable
QgsMapLayer.Removable = QgsMapLayer.LayerFlag.Removable
QgsMapLayer.Searchable = QgsMapLayer.LayerFlag.Searchable
QgsMapLayer.Private = QgsMapLayer.LayerFlag.Private
QgsMapLayer.LayerFlag.baseClass = QgsMapLayer
QgsMapLayer.LayerFlags = lambda flags=0: QgsMapLayer.LayerFlag(flags)
QgsMapLayer.LayerFlags.baseClass = QgsMapLayer
LayerFlags = QgsMapLayer  # dirty hack since SIP seems to introduce the flags in module
QgsMapLayer.LayerConfiguration = QgsMapLayer.StyleCategory.LayerConfiguration
QgsMapLayer.Symbology = QgsMapLayer.StyleCategory.Symbology
QgsMapLayer.Symbology3D = QgsMapLayer.StyleCategory.Symbology3D
QgsMapLayer.Labeling = QgsMapLayer.StyleCategory.Labeling
QgsMapLayer.Fields = QgsMapLayer.StyleCategory.Fields
QgsMapLayer.Forms = QgsMapLayer.StyleCategory.Forms
QgsMapLayer.Actions = QgsMapLayer.StyleCategory.Actions
QgsMapLayer.MapTips = QgsMapLayer.StyleCategory.MapTips
QgsMapLayer.Diagrams = QgsMapLayer.StyleCategory.Diagrams
QgsMapLayer.AttributeTable = QgsMapLayer.StyleCategory.AttributeTable
QgsMapLayer.Rendering = QgsMapLayer.StyleCategory.Rendering
QgsMapLayer.CustomProperties = QgsMapLayer.StyleCategory.CustomProperties
QgsMapLayer.GeometryOptions = QgsMapLayer.StyleCategory.GeometryOptions
QgsMapLayer.Relations = QgsMapLayer.StyleCategory.Relations
QgsMapLayer.Temporal = QgsMapLayer.StyleCategory.Temporal
QgsMapLayer.Legend = QgsMapLayer.StyleCategory.Legend
QgsMapLayer.Elevation = QgsMapLayer.StyleCategory.Elevation
QgsMapLayer.Notes = QgsMapLayer.StyleCategory.Notes
QgsMapLayer.AllStyleCategories = QgsMapLayer.StyleCategory.AllStyleCategories
QgsMapLayer.StyleCategory.baseClass = QgsMapLayer
QgsMapLayer.StyleCategories = lambda flags=0: QgsMapLayer.StyleCategory(flags)
QgsMapLayer.StyleCategories.baseClass = QgsMapLayer
StyleCategories = QgsMapLayer  # dirty hack since SIP seems to introduce the flags in module
QgsMapLayer.FlagDontResolveLayers = QgsMapLayer.ReadFlag.FlagDontResolveLayers
QgsMapLayer.FlagTrustLayerMetadata = QgsMapLayer.ReadFlag.FlagTrustLayerMetadata
QgsMapLayer.FlagReadExtentFromXml = QgsMapLayer.ReadFlag.FlagReadExtentFromXml
QgsMapLayer.FlagForceReadOnly = QgsMapLayer.ReadFlag.FlagForceReadOnly
QgsMapLayer.ReadFlags = lambda flags=0: QgsMapLayer.ReadFlag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsMapLayer.LayerFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsMapLayer.LayerFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsMapLayer.LayerFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsMapLayer.LayerFlag.__or__ = lambda flag1, flag2: QgsMapLayer.LayerFlag(_force_int(flag1) | _force_int(flag2))
QgsMapLayer.StyleCategory.__bool__ = lambda flag: bool(_force_int(flag))
QgsMapLayer.StyleCategory.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsMapLayer.StyleCategory.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsMapLayer.StyleCategory.__or__ = lambda flag1, flag2: QgsMapLayer.StyleCategory(_force_int(flag1) | _force_int(flag2))
QgsMapLayer.ReadFlag.__bool__ = lambda flag: bool(_force_int(flag))
QgsMapLayer.ReadFlag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsMapLayer.ReadFlag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsMapLayer.ReadFlag.__or__ = lambda flag1, flag2: QgsMapLayer.ReadFlag(_force_int(flag1) | _force_int(flag2))
