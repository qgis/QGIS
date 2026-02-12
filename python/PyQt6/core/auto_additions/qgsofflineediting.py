# The following has been generated automatically from src/core/qgsofflineediting.h
QgsOfflineEditing.CopyFeatures = QgsOfflineEditing.ProgressMode.CopyFeatures
QgsOfflineEditing.ProcessFeatures = QgsOfflineEditing.ProgressMode.ProcessFeatures
QgsOfflineEditing.AddFields = QgsOfflineEditing.ProgressMode.AddFields
QgsOfflineEditing.AddFeatures = QgsOfflineEditing.ProgressMode.AddFeatures
QgsOfflineEditing.RemoveFeatures = QgsOfflineEditing.ProgressMode.RemoveFeatures
QgsOfflineEditing.UpdateFeatures = QgsOfflineEditing.ProgressMode.UpdateFeatures
QgsOfflineEditing.UpdateGeometries = QgsOfflineEditing.ProgressMode.UpdateGeometries
QgsOfflineEditing.SpatiaLite = QgsOfflineEditing.ContainerType.SpatiaLite
QgsOfflineEditing.GPKG = QgsOfflineEditing.ContainerType.GPKG
try:
    QgsOfflineEditing.__attribute_docs__ = {'progressStarted': 'Emitted when the process has started.\n', 'layerProgressUpdated': 'Emitted whenever a new layer is being processed. It is possible to\nestimate the progress of the complete operation by comparing the index\nof the current ``layer`` to the total amount ``numLayers``.\n', 'progressModeSet': 'Emitted when the mode for the progress of the current operation is set.\n\n:param mode: progress mode\n:param maximum: total number of entities to process in the current\n                operation\n', 'progressUpdated': 'Emitted with the progress of the current mode\n\n:param progress: current index of processed entities\n', 'progressStopped': 'Emitted when the processing of all layers has finished\n', 'warning': 'Emitted when a warning needs to be displayed.\n\n:param title: title string for message\n:param message: A descriptive message for the warning\n'}
    QgsOfflineEditing.__signal_arguments__ = {'layerProgressUpdated': ['layer: int', 'numLayers: int'], 'progressModeSet': ['mode: QgsOfflineEditing.ProgressMode', 'maximum: int'], 'progressUpdated': ['progress: int'], 'warning': ['title: str', 'message: str']}
except (NameError, AttributeError):
    pass
