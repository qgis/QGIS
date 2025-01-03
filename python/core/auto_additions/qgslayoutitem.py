# The following has been generated automatically from src/core/layout/qgslayoutitem.h
try:
    QgsLayoutItem.ExportLayerDetail.__attribute_docs__ = {'name': 'User-friendly name for the export layer', 'mapLayerId': 'Associated map layer ID, or an empty string if this export layer is not associated with a map layer', 'compositionMode': 'Associated composition mode if this layer is associated with a map layer\n\n.. versionadded:: 3.14', 'opacity': 'Associated opacity, if this layer is associated with a map layer\n\n.. versionadded:: 3.14', 'mapTheme': 'Associated map theme, or an empty string if this export layer does not need to be associated with a map theme', 'groupName': 'Associated group name, if this layer is associated with an export group.\n\n.. versionadded:: 3.40'}
    QgsLayoutItem.ExportLayerDetail.__doc__ = """Contains details of a particular export layer relating to a layout item.

.. versionadded:: 3.10"""
    QgsLayoutItem.ExportLayerDetail.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItem.__attribute_docs__ = {'frameChanged': "Emitted if the item's frame style changes.\n", 'lockChanged': "Emitted if the item's lock status changes.\n\n.. seealso:: :py:func:`isLocked`\n\n.. seealso:: :py:func:`setLocked`\n", 'rotationChanged': 'Emitted on item rotation change.\n', 'sizePositionChanged': "Emitted when the item's size or position changes.\n", 'backgroundTaskCountChanged': 'Emitted whenever the number of background tasks an item is executing changes.\n\n.. versionadded:: 3.10\n', 'clipPathChanged': "Emitted when the item's clipping path has changed.\n\n.. seealso:: :py:func:`clipPath`\n\n.. versionadded:: 3.16\n"}
    QgsLayoutItem.__signal_arguments__ = {'rotationChanged': ['newRotation: float'], 'backgroundTaskCountChanged': ['count: int']}
    QgsLayoutItem.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemRenderContext.__group__ = ['layout']
except (NameError, AttributeError):
    pass
