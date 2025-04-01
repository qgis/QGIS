# The following has been generated automatically from src/core/vector/qgsvectorlayerexporter.h
try:
    QgsVectorLayerExporter.OutputField.__attribute_docs__ = {'field': 'Destination field definition', 'expression': 'The expression for the exported field from the source fields'}
    QgsVectorLayerExporter.OutputField.__annotations__ = {'field': 'QgsField', 'expression': str}
    QgsVectorLayerExporter.OutputField.__doc__ = """Encapsulates output field definition.

.. versionadded:: 3.44"""
    QgsVectorLayerExporter.OutputField.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerExporterTask.__attribute_docs__ = {'exportComplete': 'Emitted when exporting the layer is successfully completed.\n', 'errorOccurred': 'Emitted when an error occurs which prevented the layer being exported\n(or if the task is canceled). The export ``error`` and ``errorMessage``\nwill be reported.\n'}
    QgsVectorLayerExporterTask.withLayerOwnership = staticmethod(QgsVectorLayerExporterTask.withLayerOwnership)
    QgsVectorLayerExporterTask.__overridden_methods__ = ['cancel', 'run', 'finished']
    QgsVectorLayerExporterTask.__signal_arguments__ = {'errorOccurred': ['error: Qgis.VectorExportResult', 'errorMessage: str']}
    QgsVectorLayerExporterTask.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerExporter.exportLayer = staticmethod(QgsVectorLayerExporter.exportLayer)
    QgsVectorLayerExporter.__overridden_methods__ = ['addFeatures', 'addFeature', 'lastError', 'flushBuffer']
    QgsVectorLayerExporter.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerExporter.ExportOptions.__group__ = ['vector']
except (NameError, AttributeError):
    pass
