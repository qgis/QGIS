# The following has been generated automatically from src/gui/attributetable/qgsdualview.h
QgsDualView.AttributeTable = QgsDualView.ViewMode.AttributeTable
QgsDualView.AttributeEditor = QgsDualView.ViewMode.AttributeEditor
QgsDualView.ViewMode.baseClass = QgsDualView
QgsDualView.NoAction = QgsDualView.FeatureListBrowsingAction.NoAction
QgsDualView.PanToFeature = QgsDualView.FeatureListBrowsingAction.PanToFeature
QgsDualView.ZoomToFeature = QgsDualView.FeatureListBrowsingAction.ZoomToFeature
QgsDualView.FeatureListBrowsingAction.baseClass = QgsDualView
try:
    QgsDualView.__attribute_docs__ = {'displayExpressionChanged': 'Emitted whenever the display expression is successfully changed\n\n:param expression: The expression that was applied\n', 'filterChanged': 'Emitted whenever the filter changes\n', 'filterExpressionSet': 'Emitted when a filter expression is set using the view.\n\n:param expression: filter expression\n:param type: filter type\n', 'formModeChanged': 'Emitted when the form changes mode.\n\n:param mode: new mode\n', 'showContextMenuExternally': 'Emitted when selecting context menu on the feature list to create the context menu individually\n\n:param menu: context menu\n:param fid: feature id of the selected feature\n'}
    QgsDualView.requiredAttributes = staticmethod(QgsDualView.requiredAttributes)
    QgsDualView.__signal_arguments__ = {'displayExpressionChanged': ['expression: str'], 'filterExpressionSet': ['expression: str', 'type: QgsAttributeForm.FilterType'], 'formModeChanged': ['mode: QgsAttributeEditorContext.Mode'], 'showContextMenuExternally': ['menu: QgsActionMenu', 'fid: QgsFeatureId']}
    QgsDualView.__group__ = ['attributetable']
except NameError:
    pass
try:
    QgsAttributeTableAction.__group__ = ['attributetable']
except NameError:
    pass
try:
    QgsAttributeTableMapLayerAction.__group__ = ['attributetable']
except NameError:
    pass
