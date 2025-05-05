# The following has been generated automatically from src/core/symbology/qgslegendsymbolitem.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsLegendSymbolItem_setDataDefinedSizeLegendSettings = QgsLegendSymbolItem.setDataDefinedSizeLegendSettings
    def __QgsLegendSymbolItem_setDataDefinedSizeLegendSettings_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsLegendSymbolItem_setDataDefinedSizeLegendSettings(self, arg)
    QgsLegendSymbolItem.setDataDefinedSizeLegendSettings = _functools.update_wrapper(__QgsLegendSymbolItem_setDataDefinedSizeLegendSettings_wrapper, QgsLegendSymbolItem.setDataDefinedSizeLegendSettings)

    QgsLegendSymbolItem.__group__ = ['symbology']
except (NameError, AttributeError):
    pass
