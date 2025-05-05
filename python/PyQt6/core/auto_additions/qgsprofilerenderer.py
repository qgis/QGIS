# The following has been generated automatically from src/core/elevation/qgsprofilerenderer.h
try:
    QgsProfilePlotRenderer.__attribute_docs__ = {'generationFinished': 'Emitted when the profile generation is finished (or canceled).\n'}
    QgsProfilePlotRenderer.defaultSubSectionsSymbol = staticmethod(QgsProfilePlotRenderer.defaultSubSectionsSymbol)
    import functools as _functools
    __wrapped_QgsProfilePlotRenderer_setSubsectionsSymbol = QgsProfilePlotRenderer.setSubsectionsSymbol
    def __QgsProfilePlotRenderer_setSubsectionsSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsProfilePlotRenderer_setSubsectionsSymbol(self, arg)
    QgsProfilePlotRenderer.setSubsectionsSymbol = _functools.update_wrapper(__QgsProfilePlotRenderer_setSubsectionsSymbol_wrapper, QgsProfilePlotRenderer.setSubsectionsSymbol)

    QgsProfilePlotRenderer.__group__ = ['elevation']
except (NameError, AttributeError):
    pass
