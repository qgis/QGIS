# The following has been generated automatically from src/core/editform/qgsattributeeditorrelation.h
QgsAttributeEditorRelation.Link = QgsAttributeEditorRelation.Button.Link
QgsAttributeEditorRelation.Unlink = QgsAttributeEditorRelation.Button.Unlink
QgsAttributeEditorRelation.SaveChildEdits = QgsAttributeEditorRelation.Button.SaveChildEdits
QgsAttributeEditorRelation.AddChildFeature = QgsAttributeEditorRelation.Button.AddChildFeature
QgsAttributeEditorRelation.DuplicateChildFeature = QgsAttributeEditorRelation.Button.DuplicateChildFeature
QgsAttributeEditorRelation.DeleteChildFeature = QgsAttributeEditorRelation.Button.DeleteChildFeature
QgsAttributeEditorRelation.ZoomToChildFeature = QgsAttributeEditorRelation.Button.ZoomToChildFeature
QgsAttributeEditorRelation.AllButtons = QgsAttributeEditorRelation.Button.AllButtons
QgsAttributeEditorRelation.Button.baseClass = QgsAttributeEditorRelation
QgsAttributeEditorRelation.Buttons = lambda flags=0: QgsAttributeEditorRelation.Button(flags)
QgsAttributeEditorRelation.Buttons.baseClass = QgsAttributeEditorRelation
Buttons = QgsAttributeEditorRelation  # dirty hack since SIP seems to introduce the flags in module
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsAttributeEditorRelation.Button.__bool__ = lambda flag: bool(_force_int(flag))
QgsAttributeEditorRelation.Button.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsAttributeEditorRelation.Button.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsAttributeEditorRelation.Button.__or__ = lambda flag1, flag2: QgsAttributeEditorRelation.Button(_force_int(flag1) | _force_int(flag2))
try:
    QgsAttributeEditorRelation.__group__ = ['editform']
except (NameError, AttributeError):
    pass
