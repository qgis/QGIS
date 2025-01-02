# The following has been generated automatically from src/core/vector/qgsvectorlayerexporter.h
try:
    QgsVectorLayerExporterTask.__attribute_docs__ = {'exportComplete': 'Emitted when exporting the layer is successfully completed.\n', 'errorOccurred': 'Emitted when an error occurs which prevented the layer being exported (or if\nthe task is canceled). The export ``error`` and ``errorMessage`` will be reported.\n'}
    QgsVectorLayerExporterTask.withLayerOwnership = staticmethod(QgsVectorLayerExporterTask.withLayerOwnership)
    QgsVectorLayerExporterTask.__signal_arguments__ = {'errorOccurred': ['error: Qgis.VectorExportResult', 'errorMessage: str']}
    QgsVectorLayerExporterTask.__group__ = ['vector']
except (NameError, AttributeError):
    pass
try:
    QgsVectorLayerExporter.exportLayer = staticmethod(QgsVectorLayerExporter.exportLayer)
    QgsVectorLayerExporter.__group__ = ['vector']
except (NameError, AttributeError):
    pass
