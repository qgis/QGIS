# The following has been generated automatically from src/core/vector/qgsvectorlayerexporter.h
try:
    QgsVectorLayerExporterTask.__attribute_docs__ = {'exportComplete': 'Emitted when exporting the layer is successfully completed.\n', 'errorOccurred': 'Emitted when an error occurs which prevented the layer being exported (or if\nthe task is canceled). The export ``error`` and ``errorMessage`` will be reported.\n'}
except NameError:
    pass
QgsVectorLayerExporter.exportLayer = staticmethod(QgsVectorLayerExporter.exportLayer)
QgsVectorLayerExporterTask.withLayerOwnership = staticmethod(QgsVectorLayerExporterTask.withLayerOwnership)
try:
    QgsVectorLayerExporterTask.__signal_arguments__ = {'errorOccurred': ['error: Qgis.VectorExportResult', 'errorMessage: str']}
except NameError:
    pass
try:
    QgsVectorLayerExporter.__group__ = ['vector']
except NameError:
    pass
try:
    QgsVectorLayerExporterTask.__group__ = ['vector']
except NameError:
    pass
