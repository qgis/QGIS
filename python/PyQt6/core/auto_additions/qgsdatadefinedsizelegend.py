# The following has been generated automatically from src/core/qgsdatadefinedsizelegend.h
QgsDataDefinedSizeLegend.LegendSeparated = QgsDataDefinedSizeLegend.LegendType.LegendSeparated
QgsDataDefinedSizeLegend.LegendCollapsed = QgsDataDefinedSizeLegend.LegendType.LegendCollapsed
QgsDataDefinedSizeLegend.AlignCenter = QgsDataDefinedSizeLegend.VerticalAlignment.AlignCenter
QgsDataDefinedSizeLegend.AlignBottom = QgsDataDefinedSizeLegend.VerticalAlignment.AlignBottom
try:
    QgsDataDefinedSizeLegend.SizeClass.__attribute_docs__ = {'size': 'Marker size in units used by the symbol (usually millimeters). May be further scaled before rendering if size scale transformer is enabled.', 'label': 'Label to be shown with the particular symbol size'}
    QgsDataDefinedSizeLegend.SizeClass.__annotations__ = {'size': float, 'label': str}
    QgsDataDefinedSizeLegend.SizeClass.__doc__ = """Definition of one class for the legend"""
except (NameError, AttributeError):
    pass
try:
    QgsDataDefinedSizeLegend.readXml = staticmethod(QgsDataDefinedSizeLegend.readXml)
    import functools as _functools
    __wrapped_QgsDataDefinedSizeLegend_setSymbol = QgsDataDefinedSizeLegend.setSymbol
    def __QgsDataDefinedSizeLegend_setSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsDataDefinedSizeLegend_setSymbol(self, arg)
    QgsDataDefinedSizeLegend.setSymbol = _functools.update_wrapper(__QgsDataDefinedSizeLegend_setSymbol_wrapper, QgsDataDefinedSizeLegend.setSymbol)

    import functools as _functools
    __wrapped_QgsDataDefinedSizeLegend_setLineSymbol = QgsDataDefinedSizeLegend.setLineSymbol
    def __QgsDataDefinedSizeLegend_setLineSymbol_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsDataDefinedSizeLegend_setLineSymbol(self, arg)
    QgsDataDefinedSizeLegend.setLineSymbol = _functools.update_wrapper(__QgsDataDefinedSizeLegend_setLineSymbol_wrapper, QgsDataDefinedSizeLegend.setLineSymbol)

    import functools as _functools
    __wrapped_QgsDataDefinedSizeLegend_setSizeScaleTransformer = QgsDataDefinedSizeLegend.setSizeScaleTransformer
    def __QgsDataDefinedSizeLegend_setSizeScaleTransformer_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsDataDefinedSizeLegend_setSizeScaleTransformer(self, arg)
    QgsDataDefinedSizeLegend.setSizeScaleTransformer = _functools.update_wrapper(__QgsDataDefinedSizeLegend_setSizeScaleTransformer_wrapper, QgsDataDefinedSizeLegend.setSizeScaleTransformer)

except (NameError, AttributeError):
    pass
