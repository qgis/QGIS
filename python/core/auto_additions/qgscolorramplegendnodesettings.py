# The following has been generated automatically from src/core/layertree/qgscolorramplegendnodesettings.h
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsColorRampLegendNodeSettings_setNumericFormat = QgsColorRampLegendNodeSettings.setNumericFormat
    def __QgsColorRampLegendNodeSettings_setNumericFormat_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsColorRampLegendNodeSettings_setNumericFormat(self, arg)
    QgsColorRampLegendNodeSettings.setNumericFormat = _functools.update_wrapper(__QgsColorRampLegendNodeSettings_setNumericFormat_wrapper, QgsColorRampLegendNodeSettings.setNumericFormat)

    QgsColorRampLegendNodeSettings.__group__ = ['layertree']
except (NameError, AttributeError):
    pass
