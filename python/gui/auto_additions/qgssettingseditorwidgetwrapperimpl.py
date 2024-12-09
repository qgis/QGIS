# The following has been generated automatically from src/gui/settings/qgssettingseditorwidgetwrapperimpl.h
# monkey patching scoped based enum
QgsSettingsStringComboBoxWrapper.Mode.Text.__doc__ = "Value is defined as the text entry"
QgsSettingsStringComboBoxWrapper.Mode.Data.__doc__ = "Value is defined as data entry with Qt.UserRole"
QgsSettingsStringComboBoxWrapper.Mode.__doc__ = """Mode to determine if the value is hold in the combo box text or data

* ``Text``: Value is defined as the text entry
* ``Data``: Value is defined as data entry with Qt.UserRole

"""
# --
try:
    QgsSettingsEditorWidgetWrapperTemplate.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsStringLineEditWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsStringComboBoxWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsBoolCheckBoxWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsIntegerSpinBoxWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsDoubleSpinBoxWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsColorButtonWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
