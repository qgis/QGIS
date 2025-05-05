# The following has been generated automatically from src/gui/qgsgeometryrubberband.h
try:
    QgsGeometryRubberBand.__virtual_methods__ = ['setGeometry']
    QgsGeometryRubberBand.__overridden_methods__ = ['updatePosition', 'paint']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsGeometryRubberBand_setGeometry = QgsGeometryRubberBand.setGeometry
    def __QgsGeometryRubberBand_setGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometryRubberBand_setGeometry(self, arg)
    QgsGeometryRubberBand.setGeometry = _functools.update_wrapper(__QgsGeometryRubberBand_setGeometry_wrapper, QgsGeometryRubberBand.setGeometry)

except (NameError, AttributeError):
    pass
