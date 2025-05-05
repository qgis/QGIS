# The following has been generated automatically from src/gui/elevation/qgselevationprofilecanvas.h
try:
    QgsElevationProfileCanvas.__attribute_docs__ = {'activeJobCountChanged': 'Emitted when the number of active background jobs changes.\n', 'canvasPointHovered': 'Emitted when the mouse hovers over the specified point (in canvas\ncoordinates).\n\nThe ``profilePoint`` argument gives the hovered profile point, which may\nbe snapped.\n'}
    QgsElevationProfileCanvas.__overridden_methods__ = ['crs', 'toMapCoordinates', 'toCanvasCoordinates', 'resizeEvent', 'paintEvent', 'panContentsBy', 'centerPlotOn', 'scalePlot', 'snapToPlot', 'zoomToRect', 'wheelZoom', 'mouseMoveEvent', 'refresh']
    QgsElevationProfileCanvas.__signal_arguments__ = {'activeJobCountChanged': ['count: int'], 'canvasPointHovered': ['point: QgsPointXY', 'profilePoint: QgsProfilePoint']}
    import functools as _functools
    __wrapped_QgsElevationProfileCanvas_setProfileCurve = QgsElevationProfileCanvas.setProfileCurve
    def __QgsElevationProfileCanvas_setProfileCurve_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsElevationProfileCanvas_setProfileCurve(self, arg)
    QgsElevationProfileCanvas.setProfileCurve = _functools.update_wrapper(__QgsElevationProfileCanvas_setProfileCurve_wrapper, QgsElevationProfileCanvas.setProfileCurve)

    import functools as _functools
    __wrapped_QgsElevationProfileCanvas_setSubsectionsSymbol = QgsElevationProfileCanvas.setSubsectionsSymbol
    def __QgsElevationProfileCanvas_setSubsectionsSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsElevationProfileCanvas_setSubsectionsSymbol(self, arg)
    QgsElevationProfileCanvas.setSubsectionsSymbol = _functools.update_wrapper(__QgsElevationProfileCanvas_setSubsectionsSymbol_wrapper, QgsElevationProfileCanvas.setSubsectionsSymbol)

    QgsElevationProfileCanvas.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
