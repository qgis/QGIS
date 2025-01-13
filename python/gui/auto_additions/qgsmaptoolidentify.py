# The following has been generated automatically from src/gui/maptools/qgsmaptoolidentify.h
QgsMapToolIdentify.IdentifyMode.baseClass = QgsMapToolIdentify
QgsMapToolIdentify.LayerType.baseClass = QgsMapToolIdentify
LayerType = QgsMapToolIdentify  # dirty hack since SIP seems to introduce the flags in module
try:
    QgsMapToolIdentify.IdentifyProperties.__attribute_docs__ = {'searchRadiusMapUnits': 'Identify search radius is map units. Use negative value to ignore', 'skip3DLayers': 'Skip identify results from layers that have a 3d renderer set'}
    QgsMapToolIdentify.IdentifyProperties.__group__ = ['maptools']
except (NameError, AttributeError):
    pass
try:
    QgsMapToolIdentify.__attribute_docs__ = {'identifyProgress': 'Emitted when the identify action progresses.\n\n:param processed: number of objects processed so far\n:param total: total number of objects to process\n', 'identifyMessage': 'Emitted when the identify operation needs to show a user-facing message\n\n:param message: Message to show to the user\n', 'changedRasterResults': 'Emitted when the format of raster ``results`` is changed and need to be updated in user-facing displays.\n'}
    QgsMapToolIdentify.__signal_arguments__ = {'identifyProgress': ['processed: int', 'total: int'], 'identifyMessage': ['message: str'], 'changedRasterResults': ['results: List[QgsMapToolIdentify.IdentifyResult]']}
    QgsMapToolIdentify.__group__ = ['maptools']
except (NameError, AttributeError):
    pass
try:
    QgsMapToolIdentify.IdentifyResult.__group__ = ['maptools']
except (NameError, AttributeError):
    pass
