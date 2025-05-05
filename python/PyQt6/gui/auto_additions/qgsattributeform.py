# The following has been generated automatically from src/gui/qgsattributeform.h
QgsAttributeForm.SingleEditMode = QgsAttributeForm.Mode.SingleEditMode
QgsAttributeForm.AddFeatureMode = QgsAttributeForm.Mode.AddFeatureMode
QgsAttributeForm.MultiEditMode = QgsAttributeForm.Mode.MultiEditMode
QgsAttributeForm.SearchMode = QgsAttributeForm.Mode.SearchMode
QgsAttributeForm.AggregateSearchMode = QgsAttributeForm.Mode.AggregateSearchMode
QgsAttributeForm.IdentifyMode = QgsAttributeForm.Mode.IdentifyMode
QgsAttributeForm.ReplaceFilter = QgsAttributeForm.FilterType.ReplaceFilter
QgsAttributeForm.FilterAnd = QgsAttributeForm.FilterType.FilterAnd
QgsAttributeForm.FilterOr = QgsAttributeForm.FilterType.FilterOr
try:
    QgsAttributeForm.__attribute_docs__ = {'attributeChanged': 'Notifies about changes of attributes, this signal is not emitted when\nthe value is set back to the original one.\n\n:param attribute: The name of the attribute that changed.\n:param value: The new value of the attribute.\n\n.. deprecated:: 3.0\n', 'widgetValueChanged': 'Notifies about changes of attributes\n\n:param attribute: The name of the attribute that changed.\n:param value: The new value of the attribute.\n:param attributeChanged: If ``True``, it corresponds to an actual change\n                         of the feature attribute\n', 'featureSaved': 'Emitted when a feature is changed or added\n', 'filterExpressionSet': 'Emitted when a filter expression is set using the form.\n\n:param expression: filter expression\n:param type: filter type\n', 'modeChanged': 'Emitted when the form changes mode.\n\n:param mode: new mode\n', 'closed': "Emitted when the user selects the close option from the form's button\nbar.\n", 'zoomToFeatures': 'Emitted when the user chooses to zoom to a filtered set of features.\n', 'flashFeatures': 'Emitted when the user chooses to flash a filtered set of features.\n', 'openFilteredFeaturesAttributeTable': 'Emitted when the user chooses to open the attribute table dialog with a\nfiltered set of features.\n\n.. versionadded:: 3.24\n'}
    QgsAttributeForm.__overridden_methods__ = ['eventFilter']
    QgsAttributeForm.__signal_arguments__ = {'widgetValueChanged': ['attribute: str', 'value: object', 'attributeChanged: bool'], 'featureSaved': ['feature: QgsFeature'], 'filterExpressionSet': ['expression: str', 'type: QgsAttributeForm.FilterType'], 'modeChanged': ['mode: QgsAttributeEditorContext.Mode'], 'zoomToFeatures': ['filter: str'], 'flashFeatures': ['filter: str'], 'openFilteredFeaturesAttributeTable': ['filter: str']}
    import functools as _functools
    __wrapped_QgsAttributeForm_addInterface = QgsAttributeForm.addInterface
    def __QgsAttributeForm_addInterface_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAttributeForm_addInterface(self, arg)
    QgsAttributeForm.addInterface = _functools.update_wrapper(__QgsAttributeForm_addInterface_wrapper, QgsAttributeForm.addInterface)

    import functools as _functools
    __wrapped_QgsAttributeForm_setExtraContextScope = QgsAttributeForm.setExtraContextScope
    def __QgsAttributeForm_setExtraContextScope_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAttributeForm_setExtraContextScope(self, arg)
    QgsAttributeForm.setExtraContextScope = _functools.update_wrapper(__QgsAttributeForm_setExtraContextScope_wrapper, QgsAttributeForm.setExtraContextScope)

except (NameError, AttributeError):
    pass
