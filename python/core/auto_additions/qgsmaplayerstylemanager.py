# The following has been generated automatically from src/core/qgsmaplayerstylemanager.h
try:
    QgsMapLayerStyleManager.__attribute_docs__ = {'styleAdded': 'Emitted when a new style has been added\n', 'styleRemoved': 'Emitted when a style has been removed\n', 'styleRenamed': 'Emitted when a style has been renamed\n', 'currentStyleChanged': 'Emitted when the current style has been changed\n'}
    QgsMapLayerStyleManager.isDefault = staticmethod(QgsMapLayerStyleManager.isDefault)
    QgsMapLayerStyleManager.__signal_arguments__ = {'styleAdded': ['name: str'], 'styleRemoved': ['name: str'], 'styleRenamed': ['oldName: str', 'newName: str'], 'currentStyleChanged': ['currentName: str']}
except (NameError, AttributeError):
    pass
