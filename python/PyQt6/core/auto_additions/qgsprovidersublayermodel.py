# The following has been generated automatically from src/core/providers/qgsprovidersublayermodel.h
# monkey patching scoped based enum
QgsProviderSublayerModel.Role.ProviderKey.__doc__ = "Provider key"
QgsProviderSublayerModel.Role.LayerType.__doc__ = "Layer type"
QgsProviderSublayerModel.Role.Uri.__doc__ = "Layer URI"
QgsProviderSublayerModel.Role.Name.__doc__ = "Layer name"
QgsProviderSublayerModel.Role.Description.__doc__ = "Layer description"
QgsProviderSublayerModel.Role.Path.__doc__ = "Layer path"
QgsProviderSublayerModel.Role.FeatureCount.__doc__ = "Feature count (for vector sublayers)"
QgsProviderSublayerModel.Role.WkbType.__doc__ = "WKB geometry type (for vector sublayers)"
QgsProviderSublayerModel.Role.GeometryColumnName.__doc__ = "Geometry column name (for vector sublayers)"
QgsProviderSublayerModel.Role.LayerNumber.__doc__ = "Layer number"
QgsProviderSublayerModel.Role.IsNonLayerItem.__doc__ = "``True`` if item is a non-sublayer item (e.g. an embedded project)"
QgsProviderSublayerModel.Role.NonLayerItemType.__doc__ = "Item type (for non-sublayer items)"
QgsProviderSublayerModel.Role.Flags.__doc__ = "Sublayer flags"
QgsProviderSublayerModel.Role.__doc__ = """Custom model roles

* ``ProviderKey``: Provider key
* ``LayerType``: Layer type
* ``Uri``: Layer URI
* ``Name``: Layer name
* ``Description``: Layer description
* ``Path``: Layer path
* ``FeatureCount``: Feature count (for vector sublayers)
* ``WkbType``: WKB geometry type (for vector sublayers)
* ``GeometryColumnName``: Geometry column name (for vector sublayers)
* ``LayerNumber``: Layer number
* ``IsNonLayerItem``: ``True`` if item is a non-sublayer item (e.g. an embedded project)
* ``NonLayerItemType``: Item type (for non-sublayer items)
* ``Flags``: Sublayer flags

"""
# --
# monkey patching scoped based enum
QgsProviderSublayerModel.Column.Name.__doc__ = "Layer name"
QgsProviderSublayerModel.Column.Description.__doc__ = "Layer description"
QgsProviderSublayerModel.Column.__doc__ = """Model columns

* ``Name``: Layer name
* ``Description``: Layer description

"""
# --
try:
    QgsProviderSublayerModel.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsProviderSublayerModel.NonLayerItem.__group__ = ['providers']
except (NameError, AttributeError):
    pass
try:
    QgsProviderSublayerProxyModel.__group__ = ['providers']
except (NameError, AttributeError):
    pass
