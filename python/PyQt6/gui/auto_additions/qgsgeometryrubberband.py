# The following has been generated automatically from src/gui/qgsgeometryrubberband.h
QgsGeometryRubberBand.ICON_NONE = QgsGeometryRubberBand.IconType.ICON_NONE
QgsGeometryRubberBand.ICON_CROSS = QgsGeometryRubberBand.IconType.ICON_CROSS
QgsGeometryRubberBand.ICON_X = QgsGeometryRubberBand.IconType.ICON_X
QgsGeometryRubberBand.ICON_BOX = QgsGeometryRubberBand.IconType.ICON_BOX
QgsGeometryRubberBand.ICON_CIRCLE = QgsGeometryRubberBand.IconType.ICON_CIRCLE
QgsGeometryRubberBand.ICON_FULL_BOX = QgsGeometryRubberBand.IconType.ICON_FULL_BOX
try:
    QgsGeometryRubberBand.__virtual_methods__ = ['setGeometry']
    QgsGeometryRubberBand.__overridden_methods__ = ['updatePosition', 'paint']
    import functools as _functools
    __wrapped_QgsGeometryRubberBand_setGeometry = QgsGeometryRubberBand.setGeometry
    def __QgsGeometryRubberBand_setGeometry_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsGeometryRubberBand_setGeometry(self, arg)
    QgsGeometryRubberBand.setGeometry = _functools.update_wrapper(__QgsGeometryRubberBand_setGeometry_wrapper, QgsGeometryRubberBand.setGeometry)

except (NameError, AttributeError):
    pass
