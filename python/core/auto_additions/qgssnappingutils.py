# The following has been generated automatically from src/core/qgssnappingutils.h
try:
    QgsSnappingUtils.LayerConfig.__attribute_docs__ = {'layer': 'The layer to configure.', 'type': 'To which geometry properties of this layers a snapping should happen.', 'tolerance': 'The range around snapping targets in which snapping should occur.', 'unit': 'The units in which the tolerance is specified.'}
    QgsSnappingUtils.LayerConfig.__doc__ = """Configures how a certain layer should be handled in a snapping operation"""
except NameError:
    pass
try:
    QgsSnappingUtils.__attribute_docs__ = {'configChanged': 'Emitted when the snapping settings object changes.\n'}
    QgsSnappingUtils.__signal_arguments__ = {'configChanged': ['snappingConfig: QgsSnappingConfig']}
except NameError:
    pass
