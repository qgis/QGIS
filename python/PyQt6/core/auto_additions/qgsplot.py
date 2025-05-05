# The following has been generated automatically from src/core/plot/qgsplot.h
try:
    QgsPlotDefaultSettings.axisLabelNumericFormat = staticmethod(QgsPlotDefaultSettings.axisLabelNumericFormat)
    QgsPlotDefaultSettings.axisGridMajorSymbol = staticmethod(QgsPlotDefaultSettings.axisGridMajorSymbol)
    QgsPlotDefaultSettings.axisGridMinorSymbol = staticmethod(QgsPlotDefaultSettings.axisGridMinorSymbol)
    QgsPlotDefaultSettings.chartBackgroundSymbol = staticmethod(QgsPlotDefaultSettings.chartBackgroundSymbol)
    QgsPlotDefaultSettings.chartBorderSymbol = staticmethod(QgsPlotDefaultSettings.chartBorderSymbol)
    QgsPlotDefaultSettings.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    QgsPlot.__virtual_methods__ = ['writeXml', 'readXml']
    QgsPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    Qgs2DPlot.__virtual_methods__ = ['renderContent']
    Qgs2DPlot.__overridden_methods__ = ['writeXml', 'readXml']
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_Qgs2DPlot_setChartBackgroundSymbol = Qgs2DPlot.setChartBackgroundSymbol
    def __Qgs2DPlot_setChartBackgroundSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_Qgs2DPlot_setChartBackgroundSymbol(self, arg)
    Qgs2DPlot.setChartBackgroundSymbol = _functools.update_wrapper(__Qgs2DPlot_setChartBackgroundSymbol_wrapper, Qgs2DPlot.setChartBackgroundSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_Qgs2DPlot_setChartBorderSymbol = Qgs2DPlot.setChartBorderSymbol
    def __Qgs2DPlot_setChartBorderSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_Qgs2DPlot_setChartBorderSymbol(self, arg)
    Qgs2DPlot.setChartBorderSymbol = _functools.update_wrapper(__Qgs2DPlot_setChartBorderSymbol_wrapper, Qgs2DPlot.setChartBorderSymbol)

    Qgs2DPlot.__group__ = ['plot']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPlotAxis_setGridMajorSymbol = QgsPlotAxis.setGridMajorSymbol
    def __QgsPlotAxis_setGridMajorSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPlotAxis_setGridMajorSymbol(self, arg)
    QgsPlotAxis.setGridMajorSymbol = _functools.update_wrapper(__QgsPlotAxis_setGridMajorSymbol_wrapper, QgsPlotAxis.setGridMajorSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPlotAxis_setGridMinorSymbol = QgsPlotAxis.setGridMinorSymbol
    def __QgsPlotAxis_setGridMinorSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPlotAxis_setGridMinorSymbol(self, arg)
    QgsPlotAxis.setGridMinorSymbol = _functools.update_wrapper(__QgsPlotAxis_setGridMinorSymbol_wrapper, QgsPlotAxis.setGridMinorSymbol)

    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsPlotAxis_setNumericFormat = QgsPlotAxis.setNumericFormat
    def __QgsPlotAxis_setNumericFormat_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsPlotAxis_setNumericFormat(self, arg)
    QgsPlotAxis.setNumericFormat = _functools.update_wrapper(__QgsPlotAxis_setNumericFormat_wrapper, QgsPlotAxis.setNumericFormat)

    QgsPlotAxis.__group__ = ['plot']
except (NameError, AttributeError):
    pass
