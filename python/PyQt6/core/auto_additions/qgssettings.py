# The following has been generated automatically from src/core/settings/qgssettings.h
QgsSettings.NoSection = QgsSettings.Section.NoSection
QgsSettings.Core = QgsSettings.Section.Core
QgsSettings.Gui = QgsSettings.Section.Gui
QgsSettings.Server = QgsSettings.Section.Server
QgsSettings.Plugins = QgsSettings.Section.Plugins
QgsSettings.Auth = QgsSettings.Section.Auth
QgsSettings.App = QgsSettings.Section.App
QgsSettings.Providers = QgsSettings.Section.Providers
QgsSettings.Expressions = QgsSettings.Section.Expressions
QgsSettings.Misc = QgsSettings.Section.Misc
QgsSettings.Gps = QgsSettings.Section.Gps
try:
    QgsSettings.globalSettingsPath = staticmethod(QgsSettings.globalSettingsPath)
    QgsSettings.setGlobalSettingsPath = staticmethod(QgsSettings.setGlobalSettingsPath)
    QgsSettings.__group__ = ['settings']
except (NameError, AttributeError):
    pass
