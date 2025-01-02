# The following has been generated automatically from src/gui/processing/models/qgsmodelcomponentgraphicitem.h
QgsModelComponentGraphicItem.Normal = QgsModelComponentGraphicItem.State.Normal
QgsModelComponentGraphicItem.Selected = QgsModelComponentGraphicItem.State.Selected
QgsModelComponentGraphicItem.Hover = QgsModelComponentGraphicItem.State.Hover
QgsModelComponentGraphicItem.Unused = QgsModelComponentGraphicItem.Flag.Unused
QgsModelComponentGraphicItem.Flags = lambda flags=0: QgsModelComponentGraphicItem.Flag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsModelComponentGraphicItem.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsModelComponentGraphicItem.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsModelComponentGraphicItem.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsModelComponentGraphicItem.Flag.__or__ = lambda flag1, flag2: QgsModelComponentGraphicItem.Flag(_force_int(flag1) | _force_int(flag2))
try:
    QgsModelComponentGraphicItem.__attribute_docs__ = {'requestModelRepaint': 'Emitted by the item to request a repaint of the parent model scene.\n', 'aboutToChange': 'Emitted when the definition of the associated component is about to be changed\nby the item.\n\nThe ``text`` argument gives the translated text describing the change about to occur, and the\noptional ``id`` can be used to group the associated undo commands.\n', 'changed': 'Emitted when the definition of the associated component is changed\nby the item.\n', 'repaintArrows': 'Emitted when item requests that all connected arrows are repainted.\n', 'updateArrowPaths': 'Emitted when item requires that all connected arrow paths are recalculated.\n', 'sizePositionChanged': "Emitted when the item's size or position changes.\n"}
    QgsModelComponentGraphicItem.__signal_arguments__ = {'aboutToChange': ['text: str', 'id: int = 0']}
    QgsModelComponentGraphicItem.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
try:
    QgsModelChildAlgorithmGraphicItem.__attribute_docs__ = {'runFromHere': 'Emitted when the user opts to run the model from this child algorithm.\n\n.. versionadded:: 3.38\n', 'runSelected': 'Emitted when the user opts to run selected steps from the model.\n\n.. versionadded:: 3.38\n', 'showPreviousResults': 'Emitted when the user opts to view previous results from this child algorithm.\n\n.. versionadded:: 3.38\n', 'showLog': 'Emitted when the user opts to view the previous log from this child algorithm.\n\n.. versionadded:: 3.38\n'}
    QgsModelChildAlgorithmGraphicItem.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
try:
    QgsModelParameterGraphicItem.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
try:
    QgsModelOutputGraphicItem.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
try:
    QgsModelCommentGraphicItem.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
try:
    QgsModelGroupBoxGraphicItem.__group__ = ['processing', 'models']
except (NameError, AttributeError):
    pass
