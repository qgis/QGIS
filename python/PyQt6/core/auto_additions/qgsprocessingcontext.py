# The following has been generated automatically from src/core/processing/qgsprocessingcontext.h
QgsProcessingContext.Unused = QgsProcessingContext.Flag.Unused
QgsProcessingContext.Flags = lambda flags=0: QgsProcessingContext.Flag(flags)
# monkey patching scoped based enum
QgsProcessingContext.ProcessArgumentFlag.IncludeProjectPath.__doc__ = "Include the associated project path argument"
QgsProcessingContext.ProcessArgumentFlag.__doc__ = """Flags controlling the results given by :py:func:`~QgsProcessingContext.asQgisProcessArguments`.

.. versionadded:: 3.24

* ``IncludeProjectPath``: Include the associated project path argument

"""
# --
QgsProcessingContext.ProcessArgumentFlags = lambda flags=0: QgsProcessingContext.ProcessArgumentFlag(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsProcessingContext.Flag.__bool__ = lambda flag: bool(_force_int(flag))
QgsProcessingContext.Flag.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsProcessingContext.Flag.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsProcessingContext.Flag.__or__ = lambda flag1, flag2: QgsProcessingContext.Flag(_force_int(flag1) | _force_int(flag2))
try:
    QgsProcessingContext.LayerDetails.__attribute_docs__ = {'name': "Friendly name for layer, possibly for use when loading layer into project.\n\n.. warning::\n\n   Instead of directly using this value, prefer to call :py:func:`~LayerDetails.setOutputLayerName` to\n   generate a layer name which respects the user's local Processing settings.", 'forceName': "Set to ``True`` if LayerDetails.name should always be used as the loaded layer name, regardless\nof the user's local Processing settings.\n\n.. versionadded:: 3.16", 'outputName': 'Associated output name from algorithm which generated the layer.', 'groupName': 'Optional name for a layer tree group under which to place the layer when loading it into a project.\n\n.. versionadded:: 3.32', 'layerSortKey': 'Optional sorting key for sorting output layers when loading them into a project.\n\nLayers with a greater sort key will be placed over layers with a lesser sort key.\n\n.. versionadded:: 3.32', 'layerTypeHint': 'Layer type hint.\n\n.. versionadded:: 3.4', 'project': 'Destination project'}
    QgsProcessingContext.LayerDetails.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingContext.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingLayerPostProcessorInterface.__group__ = ['processing']
except (NameError, AttributeError):
    pass
