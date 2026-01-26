# The following has been generated automatically from src/gui/attributetable/qgsfieldconditionalformatwidget.h
try:
    QgsFieldConditionalFormatWidget.__attribute_docs__ = {'rulesUpdated': 'Emitted when the conditional styling rules are updated.\n\nThe ``fieldName`` argument indicates the name of the field whose rules\nhave been modified, or an empty ``fieldName`` indicates that a row-based\nrule was updated.\n'}
    QgsFieldConditionalFormatWidget.defaultPresets = staticmethod(QgsFieldConditionalFormatWidget.defaultPresets)
    QgsFieldConditionalFormatWidget.__signal_arguments__ = {'rulesUpdated': ['fieldName: str']}
    QgsFieldConditionalFormatWidget.__group__ = ['attributetable']
except (NameError, AttributeError):
    pass
try:
    QgsEditConditionalFormatRuleWidget.__attribute_docs__ = {'ruleSaved': 'Emitted when a user has opted to save the current rule.\n', 'ruleDeleted': 'Emitted when a user has opted to deleted the current rule.\n', 'canceled': 'Emitted when a user has opted to cancel the rule modification.\n'}
    QgsEditConditionalFormatRuleWidget.__group__ = ['attributetable']
except (NameError, AttributeError):
    pass
