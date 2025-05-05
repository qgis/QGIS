# The following has been generated automatically from src/core/layout/qgslayoutitemmapoverview.h
try:
    QgsLayoutItemMapOverviewStack.__overridden_methods__ = ['readXml']
    import functools as _functools
    __wrapped_QgsLayoutItemMapOverviewStack_addOverview = QgsLayoutItemMapOverviewStack.addOverview
    def __QgsLayoutItemMapOverviewStack_addOverview_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemMapOverviewStack_addOverview(self, arg)
    QgsLayoutItemMapOverviewStack.addOverview = _functools.update_wrapper(__QgsLayoutItemMapOverviewStack_addOverview_wrapper, QgsLayoutItemMapOverviewStack.addOverview)

    QgsLayoutItemMapOverviewStack.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsLayoutItemMapOverview.__overridden_methods__ = ['draw', 'writeXml', 'readXml', 'finalizeRestoreFromXml', 'usesAdvancedEffects', 'mapLayer', 'accept']
    import functools as _functools
    __wrapped_QgsLayoutItemMapOverview_setFrameSymbol = QgsLayoutItemMapOverview.setFrameSymbol
    def __QgsLayoutItemMapOverview_setFrameSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLayoutItemMapOverview_setFrameSymbol(self, arg)
    QgsLayoutItemMapOverview.setFrameSymbol = _functools.update_wrapper(__QgsLayoutItemMapOverview_setFrameSymbol_wrapper, QgsLayoutItemMapOverview.setFrameSymbol)

    QgsLayoutItemMapOverview.__group__ = ['layout']
except (NameError, AttributeError):
    pass
