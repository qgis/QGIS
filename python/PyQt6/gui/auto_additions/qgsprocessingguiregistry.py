# The following has been generated automatically from src/gui/processing/qgsprocessingguiregistry.h
try:
    QgsProcessingDialogFactory.__virtual_methods__ = ['createWidget']
    QgsProcessingDialogFactory.__abstract_methods__ = ['createWidget', 'createScriptEditorDialog']
    QgsProcessingDialogFactory.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingContextFactory.__abstract_methods__ = ['createContext']
    QgsProcessingContextFactory.__group__ = ['processing']
except (NameError, AttributeError):
    pass
try:
    QgsProcessingGuiRegistry.__overridden_methods__ = ['createWidgetContext']
    QgsProcessingGuiRegistry.__group__ = ['processing']
except (NameError, AttributeError):
    pass
