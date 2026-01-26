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
    QgsSettingsEditorWidgetWrapperTemplate.__virtual_methods__ = ['configureEditorPrivateImplementation']
    QgsSettingsEditorWidgetWrapperTemplate.__abstract_methods__ = ['id', 'setSettingFromWidget', 'setWidgetValue', 'valueFromWidget', 'createWrapper']
    QgsSettingsEditorWidgetWrapperTemplate.__overridden_methods__ = ['id', 'setWidgetFromSetting', 'setSettingFromWidget', 'setWidgetFromVariant', 'variantValueFromWidget', 'createWrapper', 'createEditorPrivate', 'configureEditorPrivate']
    QgsSettingsEditorWidgetWrapperTemplate.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsStringLineEditWrapper.__overridden_methods__ = ['createWrapper', 'id', 'setSettingFromWidget', 'valueFromWidget', 'setWidgetValue', 'enableAutomaticUpdatePrivate']
    QgsSettingsStringLineEditWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsStringComboBoxWrapper.__overridden_methods__ = ['createWrapper', 'id', 'setSettingFromWidget', 'valueFromWidget', 'setWidgetValue', 'enableAutomaticUpdatePrivate']
    QgsSettingsStringComboBoxWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsBoolCheckBoxWrapper.__overridden_methods__ = ['createWrapper', 'id', 'setSettingFromWidget', 'valueFromWidget', 'setWidgetValue', 'enableAutomaticUpdatePrivate']
    QgsSettingsBoolCheckBoxWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsIntegerSpinBoxWrapper.__overridden_methods__ = ['createWrapper', 'id', 'setSettingFromWidget', 'valueFromWidget', 'setWidgetValue', 'enableAutomaticUpdatePrivate']
    QgsSettingsIntegerSpinBoxWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsDoubleSpinBoxWrapper.__overridden_methods__ = ['createWrapper', 'id', 'setSettingFromWidget', 'valueFromWidget', 'setWidgetValue', 'enableAutomaticUpdatePrivate']
    QgsSettingsDoubleSpinBoxWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsColorButtonWrapper.__overridden_methods__ = ['createWrapper', 'id', 'setSettingFromWidget', 'valueFromWidget', 'setWidgetValue', 'configureEditorPrivateImplementation', 'enableAutomaticUpdatePrivate']
    QgsSettingsColorButtonWrapper.__group__ = ['settings']
except (NameError, AttributeError):
    pass
