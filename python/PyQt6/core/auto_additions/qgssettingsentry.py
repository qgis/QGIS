# The following has been generated automatically from src/core/settings/qgssettingsentry.h
try:
    QgsSettingsEntryBase.dynamicKeyPartToList = staticmethod(QgsSettingsEntryBase.dynamicKeyPartToList)
    QgsSettingsEntryBase.__virtual_methods__ = ['typeId', 'settingsType', 'checkValueVariant']
    QgsSettingsEntryBase.__group__ = ['settings']
except (NameError, AttributeError):
    pass
try:
    QgsSettingsEntryBaseTemplate.__virtual_methods__ = ['setValuePrivate', 'convertToVariant', 'checkValuePrivate']
    QgsSettingsEntryBaseTemplate.__abstract_methods__ = ['settingsType', 'convertFromVariant']
    QgsSettingsEntryBaseTemplate.__overridden_methods__ = ['settingsType', 'checkValueVariant']
    QgsSettingsEntryBaseTemplate.__group__ = ['settings']
except (NameError, AttributeError):
    pass
