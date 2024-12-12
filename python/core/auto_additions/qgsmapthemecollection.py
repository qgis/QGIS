# The following has been generated automatically from src/core/qgsmapthemecollection.h
try:
    QgsMapThemeCollection.MapThemeLayerRecord.__attribute_docs__ = {'isVisible': '``True`` if the layer is visible in the associated theme.\n\n.. versionadded:: 3.14', 'usingCurrentStyle': 'Whether current style is valid and should be applied', 'currentStyle': 'Name of the current style of the layer', 'usingLegendItems': 'Whether checkedLegendItems should be applied', 'checkedLegendItems': 'Rule keys of check legend items in layer tree model', 'expandedLegendItems': 'Rule keys of expanded legend items in layer tree view.\n\n.. versionadded:: 3.2', 'expandedLayerNode': "Whether the layer's tree node is expanded\n(only to be applied if the parent MapThemeRecord has the information about expanded nodes stored)\n\n.. versionadded:: 3.2"}
except (NameError, AttributeError):
    pass
try:
    QgsMapThemeCollection.__attribute_docs__ = {'mapThemesChanged': 'Emitted when map themes within the collection are changed.\n', 'mapThemeChanged': 'Emitted when a map theme changes definition.\n', 'mapThemeRenamed': 'Emitted when a map theme within the collection is renamed.\n\n.. versionadded:: 3.14\n', 'projectChanged': 'Emitted when the project changes\n\n.. seealso:: :py:func:`project`\n\n.. seealso:: :py:func:`setProject`\n'}
    QgsMapThemeCollection.createThemeFromCurrentState = staticmethod(QgsMapThemeCollection.createThemeFromCurrentState)
    QgsMapThemeCollection.__signal_arguments__ = {'mapThemeChanged': ['theme: str'], 'mapThemeRenamed': ['name: str', 'newName: str']}
except (NameError, AttributeError):
    pass
try:
    QgsMapThemeCollection.MapThemeRecord.readXml = staticmethod(QgsMapThemeCollection.MapThemeRecord.readXml)
except (NameError, AttributeError):
    pass
