# The following has been generated automatically from src/core/layout/qgslayoutitempage.h
QgsLayoutItemPage.Portrait = QgsLayoutItemPage.Orientation.Portrait
QgsLayoutItemPage.Landscape = QgsLayoutItemPage.Orientation.Landscape
QgsLayoutItemPage.UndoPageSymbol = QgsLayoutItemPage.UndoCommand.UndoPageSymbol
try:
    QgsLayoutItemPage.create = staticmethod(QgsLayoutItemPage.create)
    QgsLayoutItemPage.decodePageOrientation = staticmethod(QgsLayoutItemPage.decodePageOrientation)
    QgsLayoutItemPage.__overridden_methods__ = ['type', 'displayName', 'boundingRect', 'attemptResize', 'createCommand', 'exportLayerBehavior', 'accept', 'redraw', 'draw', 'drawFrame', 'drawBackground', 'writePropertiesToElement', 'readPropertiesFromElement']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemPage_setPageStyleSymbol = QgsLayoutItemPage.setPageStyleSymbol
    def __QgsLayoutItemPage_setPageStyleSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemPage_setPageStyleSymbol(self, arg)
    QgsLayoutItemPage.setPageStyleSymbol = _functools.update_wrapper(__QgsLayoutItemPage_setPageStyleSymbol_wrapper, QgsLayoutItemPage.setPageStyleSymbol)

    QgsLayoutItemPage.__group__ = ['layout']
except (NameError, AttributeError):
    pass
