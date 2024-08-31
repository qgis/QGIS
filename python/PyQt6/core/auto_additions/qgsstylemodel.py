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
QgsStyleModel.CustomRole.__doc__ = """Custom model roles.

.. note::

   Prior to QGIS 3.36 this was available as QgsStyleModel.Role

.. versionadded:: 3.36

* ``TypeRole``: Style entity type, see QgsStyle.StyleEntity
* ``TagRole``: String list of tags
* ``EntityName``: Entity name (since QGIS 3.26)
* ``SymbolTypeRole``: Symbol type (for symbol or legend patch shape entities)
* ``IsFavoriteRole``: Whether entity is flagged as a favorite
* ``LayerTypeRole``: Layer type (for label settings entities)
* ``CompatibleGeometryTypesRole``: Compatible layer geometry types (for 3D symbols)
* ``StyleName``: Name of associated QgsStyle (QgsStyle.name()) (since QGIS 3.26)
* ``StyleFileName``: File name of associated QgsStyle (QgsStyle.fileName()) (since QGIS 3.26)
* ``IsTitleRole``: True if the index corresponds to a title item (since QGIS 3.26)

"""
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
