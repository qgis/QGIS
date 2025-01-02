# The following has been generated automatically from src/core/qgssnappingutils.h
QgsSnappingUtils.IndexAlwaysFull = QgsSnappingUtils.IndexingStrategy.IndexAlwaysFull
QgsSnappingUtils.IndexNeverFull = QgsSnappingUtils.IndexingStrategy.IndexNeverFull
QgsSnappingUtils.IndexHybrid = QgsSnappingUtils.IndexingStrategy.IndexHybrid
QgsSnappingUtils.IndexExtent = QgsSnappingUtils.IndexingStrategy.IndexExtent
try:
    QgsSnappingUtils.LayerConfig.__attribute_docs__ = {'layer': 'The layer to configure.', 'type': 'To which geometry properties of this layers a snapping should happen.', 'tolerance': 'The range around snapping targets in which snapping should occur.', 'unit': 'The units in which the tolerance is specified.'}
    QgsSnappingUtils.LayerConfig.__doc__ = """Configures how a certain layer should be handled in a snapping operation"""
except (NameError, AttributeError):
    pass
try:
    QgsSnappingUtils.__attribute_docs__ = {'configChanged': 'Emitted when the snapping settings object changes.\n'}
    QgsSnappingUtils.__signal_arguments__ = {'configChanged': ['snappingConfig: QgsSnappingConfig']}
except (NameError, AttributeError):
    pass
