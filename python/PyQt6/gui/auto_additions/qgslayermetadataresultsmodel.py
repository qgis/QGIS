# The following has been generated automatically from src/gui/qgslayermetadataresultsmodel.h
QgsLayerMetadataResultsModel.Roles = QgsLayerMetadataResultsModel.CustomRole
# monkey patching scoped based enum
QgsLayerMetadataResultsModel.Metadata = QgsLayerMetadataResultsModel.CustomRole.Metadata
QgsLayerMetadataResultsModel.Metadata.is_monkey_patched = True
QgsLayerMetadataResultsModel.Metadata.__doc__ = "Layer metadata role"
QgsLayerMetadataResultsModel.CustomRole.__doc__ = """The Roles enum represents the user roles for the model.

.. note::

   Prior to QGIS 3.36 this was available as QgsLayerMetadataResultsModel.Roles

.. versionadded:: 3.36

* ``Metadata``: Layer metadata role

"""
# --
QgsLayerMetadataResultsModel.CustomRole.baseClass = QgsLayerMetadataResultsModel
QgsLayerMetadataResultsModel.Identifier = QgsLayerMetadataResultsModel.Sections.Identifier
QgsLayerMetadataResultsModel.Title = QgsLayerMetadataResultsModel.Sections.Title
QgsLayerMetadataResultsModel.Abstract = QgsLayerMetadataResultsModel.Sections.Abstract
QgsLayerMetadataResultsModel.DataProviderName = QgsLayerMetadataResultsModel.Sections.DataProviderName
QgsLayerMetadataResultsModel.GeometryType = QgsLayerMetadataResultsModel.Sections.GeometryType
try:
    QgsLayerMetadataResultsModel.__attribute_docs__ = {'progressChanged': 'Emitted when the progress changed to ``progress``.\n'}
    QgsLayerMetadataResultsModel.__signal_arguments__ = {'progressChanged': ['progress: int']}
except (NameError, AttributeError):
    pass
