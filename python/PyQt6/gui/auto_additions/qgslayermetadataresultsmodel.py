# The following has been generated automatically from src/gui/qgslayermetadataresultsmodel.h
QgsLayerMetadataResultsModel.Roles = QgsLayerMetadataResultsModel.CustomRole
# monkey patching scoped based enum
QgsLayerMetadataResultsModel.Metadata = QgsLayerMetadataResultsModel.CustomRole.Metadata
QgsLayerMetadataResultsModel.Metadata.is_monkey_patched = True
QgsLayerMetadataResultsModel.Metadata.__doc__ = "Layer metadata role"
QgsLayerMetadataResultsModel.CustomRole.__doc__ = "The Roles enum represents the user roles for the model.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as QgsLayerMetadataResultsModel.Roles\n\n.. versionadded:: 3.36\n\n" + '* ``Metadata``: ' + QgsLayerMetadataResultsModel.CustomRole.Metadata.__doc__
# --
QgsLayerMetadataResultsModel.CustomRole.baseClass = QgsLayerMetadataResultsModel
QgsLayerMetadataResultsModel.Identifier = QgsLayerMetadataResultsModel.Sections.Identifier
QgsLayerMetadataResultsModel.Title = QgsLayerMetadataResultsModel.Sections.Title
QgsLayerMetadataResultsModel.Abstract = QgsLayerMetadataResultsModel.Sections.Abstract
QgsLayerMetadataResultsModel.DataProviderName = QgsLayerMetadataResultsModel.Sections.DataProviderName
QgsLayerMetadataResultsModel.GeometryType = QgsLayerMetadataResultsModel.Sections.GeometryType
QgsLayerMetadataResultsModel.__attribute_docs__ = {'progressChanged': 'Emitted when the progress changed to ``progress``.\n'}
