# The following has been generated automatically from src/gui/qgsrelationeditorwidget.h
QgsRelationEditorWidget.NoButton = QgsRelationEditorWidget.Button.NoButton
QgsRelationEditorWidget.Link = QgsRelationEditorWidget.Button.Link
QgsRelationEditorWidget.Unlink = QgsRelationEditorWidget.Button.Unlink
QgsRelationEditorWidget.SaveChildEdits = QgsRelationEditorWidget.Button.SaveChildEdits
QgsRelationEditorWidget.AddChildFeature = QgsRelationEditorWidget.Button.AddChildFeature
QgsRelationEditorWidget.DuplicateChildFeature = QgsRelationEditorWidget.Button.DuplicateChildFeature
QgsRelationEditorWidget.DeleteChildFeature = QgsRelationEditorWidget.Button.DeleteChildFeature
QgsRelationEditorWidget.ZoomToChildFeature = QgsRelationEditorWidget.Button.ZoomToChildFeature
QgsRelationEditorWidget.AllButtons = QgsRelationEditorWidget.Button.AllButtons
QgsRelationEditorWidget.Button.baseClass = QgsRelationEditorWidget
QgsRelationEditorWidget.Buttons = lambda flags=0: QgsRelationEditorWidget.Button(flags)
QgsRelationEditorWidget.Buttons.baseClass = QgsRelationEditorWidget
Buttons = QgsRelationEditorWidget  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsRelationEditorWidget.__overridden_methods__ = ['setEditorContext', 'config', 'setConfig', 'parentFormValueChanged', 'updateUi', 'beforeSetRelationFeature', 'afterSetRelationFeature', 'beforeSetRelations', 'afterSetRelations']
except (NameError, AttributeError):
    pass
try:
    QgsRelationEditorConfigWidget.__overridden_methods__ = ['config', 'setConfig']
except (NameError, AttributeError):
    pass
