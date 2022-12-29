# The following has been generated automatically from src/core/settings/qgssettingstreeelement.h
# monkey patching scoped based enum
QgsSettingsTreeElement.Type.Root.__doc__ = "Root Element"
QgsSettingsTreeElement.Type.Standard.__doc__ = "Normal Element"
QgsSettingsTreeElement.Type.NamedList.__doc__ = ""
QgsSettingsTreeElement.Type.__doc__ = 'Type of tree element\n\n' + '* ``Root``: ' + QgsSettingsTreeElement.Type.Root.__doc__ + '\n' + '* ``Standard``: ' + QgsSettingsTreeElement.Type.Standard.__doc__ + '\n' + '* ``NamedList``: ' + QgsSettingsTreeElement.Type.NamedList.__doc__
# --
QgsSettingsTreeElement.Type.baseClass = QgsSettingsTreeElement
# monkey patching scoped based enum
QgsSettingsTreeElement.Option.NamedListSelectedItemSetting.__doc__ = ""
QgsSettingsTreeElement.Option.__doc__ = 'Options for named list elements\n\n' + '* ``NamedListSelectedItemSetting``: ' + QgsSettingsTreeElement.Option.NamedListSelectedItemSetting.__doc__
# --
QgsSettingsTreeElement.Option.baseClass = QgsSettingsTreeElement
QgsSettingsTreeElement.Options.baseClass = QgsSettingsTreeElement
Options = QgsSettingsTreeElement  # dirty hack since SIP seems to introduce the flags in module
