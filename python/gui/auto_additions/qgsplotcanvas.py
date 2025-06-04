# The following has been generated automatically from src/gui/plot/qgsplotcanvas.h
try:
    QgsPlotCanvas.__attribute_docs__ = {'toolChanged': 'Emitted when the plot tool is changed.\n', 'plotAreaChanged': 'Emitted whenever the visible area of the plot is changed.\n', 'contextMenuAboutToShow': 'Emitted before the canvas context menu will be shown. Can be used to\nextend the context menu.\n', 'willBeDeleted': 'Emitted in the destructor when the canvas is about to be deleted, but is\nstill in a perfectly valid state.\n'}
    QgsPlotCanvas.__virtual_methods__ = ['crs', 'toMapCoordinates', 'toCanvasCoordinates', 'panContentsBy', 'centerPlotOn', 'scalePlot', 'zoomToRect', 'snapToPlot', 'refresh', 'wheelZoom']
    QgsPlotCanvas.__overridden_methods__ = ['event', 'keyPressEvent', 'keyReleaseEvent', 'mouseDoubleClickEvent', 'mouseMoveEvent', 'mousePressEvent', 'mouseReleaseEvent', 'wheelEvent', 'resizeEvent', 'viewportEvent']
    QgsPlotCanvas.__signal_arguments__ = {'toolChanged': ['newTool: QgsPlotTool'], 'contextMenuAboutToShow': ['menu: QMenu', 'event: QgsPlotMouseEvent']}
    QgsPlotCanvas.__group__ = ['plot']
except (NameError, AttributeError):
    pass
