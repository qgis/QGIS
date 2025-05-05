# The following has been generated automatically from src/core/symbology/qgsinvertedpolygonrenderer.h
try:
    QgsInvertedPolygonRenderer.create = staticmethod(QgsInvertedPolygonRenderer.create)
    QgsInvertedPolygonRenderer.convertFromRenderer = staticmethod(QgsInvertedPolygonRenderer.convertFromRenderer)
    QgsInvertedPolygonRenderer.__overridden_methods__ = ['clone', 'dump', 'save']
    import functools as _functools
    __wrapped_QgsInvertedPolygonRenderer_QgsInvertedPolygonRenderer = QgsInvertedPolygonRenderer.QgsInvertedPolygonRenderer
    def __QgsInvertedPolygonRenderer_QgsInvertedPolygonRenderer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsInvertedPolygonRenderer_QgsInvertedPolygonRenderer(self, arg)
    QgsInvertedPolygonRenderer.QgsInvertedPolygonRenderer = _functools.update_wrapper(__QgsInvertedPolygonRenderer_QgsInvertedPolygonRenderer_wrapper, QgsInvertedPolygonRenderer.QgsInvertedPolygonRenderer)

    QgsInvertedPolygonRenderer.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
