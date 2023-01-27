# The following has been generated automatically from src/core/settings/qgssettingstreenode.h
# monkey patching scoped based enum
QgsSettingsTreeNode.Type.Root.__doc__ = "Root Node"
QgsSettingsTreeNode.Type.Standard.__doc__ = "Normal Node"
QgsSettingsTreeNode.Type.NamedList.__doc__ = ""
QgsSettingsTreeNode.Type.__doc__ = 'Type of tree node\n\n' + '* ``Root``: ' + QgsSettingsTreeNode.Type.Root.__doc__ + '\n' + '* ``Standard``: ' + QgsSettingsTreeNode.Type.Standard.__doc__ + '\n' + '* ``NamedList``: ' + QgsSettingsTreeNode.Type.NamedList.__doc__
# --
QgsSettingsTreeNode.Type.baseClass = QgsSettingsTreeNode
# monkey patching scoped based enum
QgsSettingsTreeNode.Option.NamedListSelectedItemSetting.__doc__ = "Creates a setting to store which is the current item"
QgsSettingsTreeNode.Option.__doc__ = 'Options for named list nodes\n\n' + '* ``NamedListSelectedItemSetting``: ' + QgsSettingsTreeNode.Option.NamedListSelectedItemSetting.__doc__
# --
QgsSettingsTreeNode.Option.baseClass = QgsSettingsTreeNode
QgsSettingsTreeNode.Options.baseClass = QgsSettingsTreeNode
Options = QgsSettingsTreeNode  # dirty hack since SIP seems to introduce the flags in module
