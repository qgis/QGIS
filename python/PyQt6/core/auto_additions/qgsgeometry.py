# The following has been generated automatically from src/core/geometry/qgsgeometry.h
try:
    QgsGeometry.fromWkt = staticmethod(QgsGeometry.fromWkt)
    QgsGeometry.fromPointXY = staticmethod(QgsGeometry.fromPointXY)
    QgsGeometry.fromPoint = staticmethod(QgsGeometry.fromPoint)
    QgsGeometry.fromMultiPointXY = staticmethod(QgsGeometry.fromMultiPointXY)
    QgsGeometry.fromPolylineXY = staticmethod(QgsGeometry.fromPolylineXY)
    QgsGeometry.fromPolyline = staticmethod(QgsGeometry.fromPolyline)
    QgsGeometry.fromMultiPolylineXY = staticmethod(QgsGeometry.fromMultiPolylineXY)
    QgsGeometry.fromPolygonXY = staticmethod(QgsGeometry.fromPolygonXY)
    QgsGeometry.fromMultiPolygonXY = staticmethod(QgsGeometry.fromMultiPolygonXY)
    QgsGeometry.fromRect = staticmethod(QgsGeometry.fromRect)
    QgsGeometry.fromBox3D = staticmethod(QgsGeometry.fromBox3D)
    QgsGeometry.collectGeometry = staticmethod(QgsGeometry.collectGeometry)
    QgsGeometry.createWedgeBuffer = staticmethod(QgsGeometry.createWedgeBuffer)
    QgsGeometry.createWedgeBufferFromAngles = staticmethod(QgsGeometry.createWedgeBufferFromAngles)
    QgsGeometry.unaryUnion = staticmethod(QgsGeometry.unaryUnion)
    QgsGeometry.polygonize = staticmethod(QgsGeometry.polygonize)
    QgsGeometry.fromQPointF = staticmethod(QgsGeometry.fromQPointF)
    QgsGeometry.fromQPolygonF = staticmethod(QgsGeometry.fromQPolygonF)
    QgsGeometry.createPolylineFromQPolygonF = staticmethod(QgsGeometry.createPolylineFromQPolygonF)
    QgsGeometry.createPolygonFromQPolygonF = staticmethod(QgsGeometry.createPolygonFromQPolygonF)
    QgsGeometry.compare = staticmethod(QgsGeometry.compare)
    QgsGeometry.createGeometryEngine = staticmethod(QgsGeometry.createGeometryEngine)
    QgsGeometry.convertPointList = staticmethod(QgsGeometry.convertPointList)
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGeometry_QgsGeometry = QgsGeometry.QgsGeometry
    def __QgsGeometry_QgsGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometry_QgsGeometry(self, arg)
    QgsGeometry.QgsGeometry = _functools.update_wrapper(__QgsGeometry_QgsGeometry_wrapper, QgsGeometry.QgsGeometry)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGeometry_set = QgsGeometry.set
    def __QgsGeometry_set_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometry_set(self, arg)
    QgsGeometry.set = _functools.update_wrapper(__QgsGeometry_set_wrapper, QgsGeometry.set)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGeometry_addRing = QgsGeometry.addRing
    def __QgsGeometry_addRing_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometry_addRing(self, arg)
    QgsGeometry.addRing = _functools.update_wrapper(__QgsGeometry_addRing_wrapper, QgsGeometry.addRing)

    QgsGeometry.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
try:
    QgsGeometryParameters.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
try:
    QgsGeometry.Error.__group__ = ['geometry']
except (NameError, AttributeError):
    pass
