# The following has been generated automatically from src/gui/qgsadvanceddigitizingtoolsregistry.h
try:
    QgsAdvancedDigitizingToolAbstractMetadata.__virtual_methods__ = ['createTool']
except (NameError, AttributeError):
    pass
try:
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsAdvancedDigitizingToolsRegistry_addTool = QgsAdvancedDigitizingToolsRegistry.addTool
    def __QgsAdvancedDigitizingToolsRegistry_addTool_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsAdvancedDigitizingToolsRegistry_addTool(self, arg)
    QgsAdvancedDigitizingToolsRegistry.addTool = _functools.update_wrapper(__QgsAdvancedDigitizingToolsRegistry_addTool_wrapper, QgsAdvancedDigitizingToolsRegistry.addTool)

except (NameError, AttributeError):
    pass
