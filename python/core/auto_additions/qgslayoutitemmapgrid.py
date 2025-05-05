# The following has been generated automatically from src/core/layout/qgslayoutitemmapgrid.h
try:
    QgsLayoutItemMapGrid.__attribute_docs__ = {'crsChanged': "Emitted whenever the grid's CRS is changed.\n\n.. versionadded:: 3.18\n"}
    QgsLayoutItemMapGrid.__overridden_methods__ = ['draw', 'writeXml', 'readXml', 'usesAdvancedEffects', 'setEnabled', 'createExpressionContext', 'accept', 'refresh']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemMapGrid_setLineSymbol = QgsLayoutItemMapGrid.setLineSymbol
    def __QgsLayoutItemMapGrid_setLineSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemMapGrid_setLineSymbol(self, arg)
    QgsLayoutItemMapGrid.setLineSymbol = _functools.update_wrapper(__QgsLayoutItemMapGrid_setLineSymbol_wrapper, QgsLayoutItemMapGrid.setLineSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemMapGrid_setMarkerSymbol = QgsLayoutItemMapGrid.setMarkerSymbol
    def __QgsLayoutItemMapGrid_setMarkerSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemMapGrid_setMarkerSymbol(self, arg)
    QgsLayoutItemMapGrid.setMarkerSymbol = _functools.update_wrapper(__QgsLayoutItemMapGrid_setMarkerSymbol_wrapper, QgsLayoutItemMapGrid.setMarkerSymbol)

    QgsLayoutItemMapGrid.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemMapGridStack.__overridden_methods__ = ['readXml']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLayoutItemMapGridStack_addGrid = QgsLayoutItemMapGridStack.addGrid
    def __QgsLayoutItemMapGridStack_addGrid_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemMapGridStack_addGrid(self, arg)
    QgsLayoutItemMapGridStack.addGrid = _functools.update_wrapper(__QgsLayoutItemMapGridStack_addGrid_wrapper, QgsLayoutItemMapGridStack.addGrid)

    QgsLayoutItemMapGridStack.__group__ = ['layout']
except (NameError, AttributeError):
    pass
