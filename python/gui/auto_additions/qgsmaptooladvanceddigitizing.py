# The following has been generated automatically from src/gui/maptools/qgsmaptooladvanceddigitizing.h
try:
    QgsMapToolAdvancedDigitizing.__attribute_docs__ = {'transientGeometryChanged': "Emitted whenever the ``geometry`` associated with the tool is changed,\nincluding transient (i.e. non-finalized, hover state) changes.\n\nConnections to this signal should take care to check the CRS of\n``geometry``, as it may be either in the canvas CRS or an associated\nlayer's CRS.\n\n.. versionadded:: 4.0\n"}
    QgsMapToolAdvancedDigitizing.__virtual_methods__ = ['layer', 'cadCanvasPressEvent', 'cadCanvasReleaseEvent', 'cadCanvasMoveEvent']
    QgsMapToolAdvancedDigitizing.__overridden_methods__ = ['canvasPressEvent', 'canvasReleaseEvent', 'canvasMoveEvent', 'activate', 'deactivate']
    QgsMapToolAdvancedDigitizing.__signal_arguments__ = {'transientGeometryChanged': ['geometry: QgsReferencedGeometry']}
    QgsMapToolAdvancedDigitizing.__group__ = ['maptools']
except (NameError, AttributeError):
    pass
