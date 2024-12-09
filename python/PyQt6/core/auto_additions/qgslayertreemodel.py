# The following has been generated automatically from src/core/layertree/qgslayertreemodel.h
QgsLayerTreeModel.ShowLegend = QgsLayerTreeModel.Flag.ShowLegend
QgsLayerTreeModel.ShowLegendAsTree = QgsLayerTreeModel.Flag.ShowLegendAsTree
QgsLayerTreeModel.DeferredLegendInvalidation = QgsLayerTreeModel.Flag.DeferredLegendInvalidation
QgsLayerTreeModel.UseEmbeddedWidgets = QgsLayerTreeModel.Flag.UseEmbeddedWidgets
QgsLayerTreeModel.UseTextFormatting = QgsLayerTreeModel.Flag.UseTextFormatting
QgsLayerTreeModel.AllowNodeReorder = QgsLayerTreeModel.Flag.AllowNodeReorder
QgsLayerTreeModel.AllowNodeRename = QgsLayerTreeModel.Flag.AllowNodeRename
QgsLayerTreeModel.AllowNodeChangeVisibility = QgsLayerTreeModel.Flag.AllowNodeChangeVisibility
QgsLayerTreeModel.AllowLegendChangeState = QgsLayerTreeModel.Flag.AllowLegendChangeState
QgsLayerTreeModel.ActionHierarchical = QgsLayerTreeModel.Flag.ActionHierarchical
QgsLayerTreeModel.UseThreadedHitTest = QgsLayerTreeModel.Flag.UseThreadedHitTest
QgsLayerTreeModel.Flags = lambda flags=0: QgsLayerTreeModel.Flag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsLayerTreeModel.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsLayerTreeModel.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsLayerTreeModel.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsLayerTreeModel.Flag.__or__ = lambda flag1, flag2: QgsLayerTreeModel.Flag(_force_int(flag1) | _force_int(flag2))
try:
    QgsLayerTreeModel.__attribute_docs__ = {'messageEmitted': 'Emits a message than can be displayed to the user in a GUI class\n\n.. versionadded:: 3.14\n', 'hitTestStarted': 'Emitted when a hit test for visible legend items starts.\n\n.. seealso:: :py:func:`hitTestInProgress`\n\n.. seealso:: :py:func:`hitTestCompleted`\n\n.. versionadded:: 3.32\n', 'hitTestCompleted': 'Emitted when a hit test for visible legend items completes.\n\n.. seealso:: :py:func:`hitTestInProgress`\n\n.. seealso:: :py:func:`hitTestStarted`\n\n.. versionadded:: 3.32\n'}
    QgsLayerTreeModel.index2legendNode = staticmethod(QgsLayerTreeModel.index2legendNode)
    QgsLayerTreeModel.scaleIconSize = staticmethod(QgsLayerTreeModel.scaleIconSize)
    QgsLayerTreeModel.iconGroup = staticmethod(QgsLayerTreeModel.iconGroup)
    QgsLayerTreeModel.__signal_arguments__ = {'messageEmitted': ['message: str', 'level: Qgis.MessageLevel = Qgis.MessageLevel.Info', 'duration: int = 5']}
    QgsLayerTreeModel.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
