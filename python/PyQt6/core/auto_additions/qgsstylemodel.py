# The following has been generated automatically from src/core/symbology/qgsstylemodel.h
QgsStyleModel.Name = QgsStyleModel.Column.Name
QgsStyleModel.Tags = QgsStyleModel.Column.Tags
QgsStyleModel.Role = QgsStyleModel.CustomRole
# monkey patching scoped based enum
QgsStyleModel.TypeRole = QgsStyleModel.CustomRole.Type
QgsStyleModel.Role.TypeRole = QgsStyleModel.CustomRole.Type
QgsStyleModel.TypeRole.is_monkey_patched = True
QgsStyleModel.TypeRole.__doc__ = "Style entity type, see QgsStyle.StyleEntity"
QgsStyleModel.TagRole = QgsStyleModel.CustomRole.Tag
QgsStyleModel.Role.TagRole = QgsStyleModel.CustomRole.Tag
QgsStyleModel.TagRole.is_monkey_patched = True
QgsStyleModel.TagRole.__doc__ = "String list of tags"
QgsStyleModel.EntityName = QgsStyleModel.CustomRole.EntityName
QgsStyleModel.EntityName.is_monkey_patched = True
QgsStyleModel.EntityName.__doc__ = "Entity name (since QGIS 3.26)"
QgsStyleModel.SymbolTypeRole = QgsStyleModel.CustomRole.SymbolType
QgsStyleModel.Role.SymbolTypeRole = QgsStyleModel.CustomRole.SymbolType
QgsStyleModel.SymbolTypeRole.is_monkey_patched = True
QgsStyleModel.SymbolTypeRole.__doc__ = "Symbol type (for symbol or legend patch shape entities)"
QgsStyleModel.IsFavoriteRole = QgsStyleModel.CustomRole.IsFavorite
QgsStyleModel.Role.IsFavoriteRole = QgsStyleModel.CustomRole.IsFavorite
QgsStyleModel.IsFavoriteRole.is_monkey_patched = True
QgsStyleModel.IsFavoriteRole.__doc__ = "Whether entity is flagged as a favorite"
QgsStyleModel.LayerTypeRole = QgsStyleModel.CustomRole.LayerType
QgsStyleModel.Role.LayerTypeRole = QgsStyleModel.CustomRole.LayerType
QgsStyleModel.LayerTypeRole.is_monkey_patched = True
QgsStyleModel.LayerTypeRole.__doc__ = "Layer type (for label settings entities)"
QgsStyleModel.CompatibleGeometryTypesRole = QgsStyleModel.CustomRole.CompatibleGeometryTypes
QgsStyleModel.Role.CompatibleGeometryTypesRole = QgsStyleModel.CustomRole.CompatibleGeometryTypes
QgsStyleModel.CompatibleGeometryTypesRole.is_monkey_patched = True
QgsStyleModel.CompatibleGeometryTypesRole.__doc__ = "Compatible layer geometry types (for 3D symbols)"
QgsStyleModel.StyleName = QgsStyleModel.CustomRole.StyleName
QgsStyleModel.StyleName.is_monkey_patched = True
QgsStyleModel.StyleName.__doc__ = "Name of associated QgsStyle (QgsStyle.name()) (since QGIS 3.26)"
QgsStyleModel.StyleFileName = QgsStyleModel.CustomRole.StyleFileName
QgsStyleModel.StyleFileName.is_monkey_patched = True
QgsStyleModel.StyleFileName.__doc__ = "File name of associated QgsStyle (QgsStyle.fileName()) (since QGIS 3.26)"
QgsStyleModel.IsTitleRole = QgsStyleModel.CustomRole.IsTitle
QgsStyleModel.Role.IsTitleRole = QgsStyleModel.CustomRole.IsTitle
QgsStyleModel.IsTitleRole.is_monkey_patched = True
QgsStyleModel.IsTitleRole.__doc__ = "True if the index corresponds to a title item (since QGIS 3.26)"
QgsStyleModel.CustomRole.__doc__ = "Custom model roles.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsStyleModel.Role\n\n.. versionadded:: 3.36\n\n" + '* ``TypeRole``: ' + QgsStyleModel.CustomRole.Type.__doc__ + '\n' + '* ``TagRole``: ' + QgsStyleModel.CustomRole.Tag.__doc__ + '\n' + '* ``EntityName``: ' + QgsStyleModel.CustomRole.EntityName.__doc__ + '\n' + '* ``SymbolTypeRole``: ' + QgsStyleModel.CustomRole.SymbolType.__doc__ + '\n' + '* ``IsFavoriteRole``: ' + QgsStyleModel.CustomRole.IsFavorite.__doc__ + '\n' + '* ``LayerTypeRole``: ' + QgsStyleModel.CustomRole.LayerType.__doc__ + '\n' + '* ``CompatibleGeometryTypesRole``: ' + QgsStyleModel.CustomRole.CompatibleGeometryTypes.__doc__ + '\n' + '* ``StyleName``: ' + QgsStyleModel.CustomRole.StyleName.__doc__ + '\n' + '* ``StyleFileName``: ' + QgsStyleModel.CustomRole.StyleFileName.__doc__ + '\n' + '* ``IsTitleRole``: ' + QgsStyleModel.CustomRole.IsTitle.__doc__
# --
QgsStyleModel.CustomRole.baseClass = QgsStyleModel
try:
    QgsStyleModel.__group__ = ['symbology']
except NameError:
    pass
try:
    QgsStyleProxyModel.__group__ = ['symbology']
except NameError:
    pass
