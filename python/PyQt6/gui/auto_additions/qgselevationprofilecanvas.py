# The following has been generated automatically from src/gui/elevation/qgselevationprofilecanvas.h
try:
    QgsElevationProfileCanvas.__attribute_docs__ = {'activeJobCountChanged': 'Emitted when the number of active background jobs changes.\n', 'canvasPointHovered': 'Emitted when the mouse hovers over the specified point (in canvas\ncoordinates).\n\nThe ``profilePoint`` argument gives the hovered profile point, which may\nbe snapped.\n'}
    QgsElevationProfileCanvas.__overridden_methods__ = ['crs', 'toMapCoordinates', 'toCanvasCoordinates', 'resizeEvent', 'paintEvent', 'panContentsBy', 'centerPlotOn', 'scalePlot', 'snapToPlot', 'zoomToRect', 'wheelZoom', 'mouseMoveEvent', 'refresh']
    QgsElevationProfileCanvas.__signal_arguments__ = {'activeJobCountChanged': ['count: int'], 'canvasPointHovered': ['point: QgsPointXY', 'profilePoint: QgsProfilePoint']}
    QgsElevationProfileCanvas.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
