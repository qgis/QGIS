# The following has been generated automatically from src/gui/qgsabstractrelationeditorwidget.h
try:
    QgsAbstractRelationEditorWidget.__attribute_docs__ = {'relatedFeaturesChanged': 'Emit this signal, whenever the related features changed.\nThis happens for example when related features are added, removed,\nlinked or unlinked.\n\n.. versionadded:: 3.22\n'}
    QgsAbstractRelationEditorWidget.__virtual_methods__ = ['setEditorContext', 'updateUi', 'setTitle', 'beforeSetRelationFeature', 'afterSetRelationFeature', 'beforeSetRelations', 'afterSetRelations']
    QgsAbstractRelationEditorWidget.__abstract_methods__ = ['config', 'setConfig', 'parentFormValueChanged']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractRelationEditorConfigWidget.__virtual_methods__ = ['setNmRelation', 'nmRelation']
    QgsAbstractRelationEditorConfigWidget.__abstract_methods__ = ['config', 'setConfig']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractRelationEditorWidgetFactory.__abstract_methods__ = ['type', 'name', 'create', 'configWidget']
except (NameError, AttributeError):
    pass
