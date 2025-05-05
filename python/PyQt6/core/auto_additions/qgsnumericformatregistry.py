# The following has been generated automatically from src/core/numericformats/qgsnumericformatregistry.h
try:
    import functools as _functools
    __wrapped_QgsNumericFormatRegistry_addFormat = QgsNumericFormatRegistry.addFormat
    def __QgsNumericFormatRegistry_addFormat_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsNumericFormatRegistry_addFormat(self, arg)
    QgsNumericFormatRegistry.addFormat = _functools.update_wrapper(__QgsNumericFormatRegistry_addFormat_wrapper, QgsNumericFormatRegistry.addFormat)

    QgsNumericFormatRegistry.__group__ = ['numericformats']
except (NameError, AttributeError):
    pass
