# The following has been generated automatically from src/gui/qgssublayersdialog.h
QgsSublayersDialog.PromptAlways = QgsSublayersDialog.PromptMode.PromptAlways
QgsSublayersDialog.PromptIfNeeded = QgsSublayersDialog.PromptMode.PromptIfNeeded
QgsSublayersDialog.PromptNever = QgsSublayersDialog.PromptMode.PromptNever
QgsSublayersDialog.PromptLoadAll = QgsSublayersDialog.PromptMode.PromptLoadAll
QgsSublayersDialog.PromptMode.baseClass = QgsSublayersDialog
QgsSublayersDialog.Ogr = QgsSublayersDialog.ProviderType.Ogr
QgsSublayersDialog.Gdal = QgsSublayersDialog.ProviderType.Gdal
QgsSublayersDialog.Vsifile = QgsSublayersDialog.ProviderType.Vsifile
QgsSublayersDialog.Mdal = QgsSublayersDialog.ProviderType.Mdal
try:
    QgsSublayersDialog.LayerDefinition.__attribute_docs__ = {'layerId': 'Identifier of the layer (one unique layer id may have multiple types though)', 'layerName': 'Name of the layer (not necessarily unique)', 'count': 'Number of features (might be unused)', 'type': 'Extra type depending on the use (e.g. geometry type for vector sublayers)', 'description': 'Description.\n\n.. versionadded:: 3.10'}
    QgsSublayersDialog.LayerDefinition.__doc__ = """A structure that defines layers for the purpose of this dialog"""
except (NameError, AttributeError):
    pass
