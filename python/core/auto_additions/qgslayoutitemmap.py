# The following has been generated automatically from src/core/layout/qgslayoutitemmap.h
try:
    QgsLayoutItemMapAtlasClippingSettings.__attribute_docs__ = {'changed': 'Emitted when the atlas clipping settings are changed.\n'}
    QgsLayoutItemMapAtlasClippingSettings.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemMapItemClipPathSettings.__attribute_docs__ = {'changed': 'Emitted when the item clipping settings are changed.\n'}
    QgsLayoutItemMapItemClipPathSettings.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemMap.__attribute_docs__ = {'extentChanged': "Emitted when the map's extent changes.\n\n.. seealso:: :py:func:`setExtent`\n\n.. seealso:: :py:func:`extent`\n", 'mapRotationChanged': "Emitted when the map's rotation changes.\n\n.. seealso:: :py:func:`setMapRotation`\n\n.. seealso:: :py:func:`mapRotation`\n", 'preparedForAtlas': 'Emitted when the map has been prepared for atlas rendering, just before actual rendering\n', 'layerStyleOverridesChanged': 'Emitted when layer style overrides are changed... a means to let\nassociated legend items know they should update\n', 'themeChanged': "Emitted when the map's associated ``theme`` is changed.\n\n.. note::\n\n   This signal is not emitted when the definition of the theme changes, only the map\n   is linked to a different theme then it previously was.\n\n.. versionadded:: 3.14\n", 'crsChanged': "Emitted when the map's coordinate reference system is changed.\n\n.. versionadded:: 3.18\n", 'previewRefreshed': "Emitted whenever the item's map preview has been refreshed.\n\n.. versionadded:: 3.20\n"}
    QgsLayoutItemMap.create = staticmethod(QgsLayoutItemMap.create)
    QgsLayoutItemMap.__signal_arguments__ = {'mapRotationChanged': ['newRotation: float'], 'themeChanged': ['theme: str']}
    QgsLayoutItemMap.__group__ = ['layout']
except (NameError, AttributeError):
    pass
