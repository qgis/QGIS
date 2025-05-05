# The following has been generated automatically from src/core/qgsfieldformatterregistry.h
try:
    QgsFieldFormatterRegistry.__attribute_docs__ = {'fieldFormatterAdded': 'Will be emitted after a new field formatter has been added.\n', 'fieldFormatterRemoved': 'Will be emitted just before a field formatter is removed and deleted.\n'}
    QgsFieldFormatterRegistry.__signal_arguments__ = {'fieldFormatterAdded': ['formatter: QgsFieldFormatter'], 'fieldFormatterRemoved': ['formatter: QgsFieldFormatter']}
    import functools as _functools
    from qgis.core import QgsSipUtils as _QgsSipUtils
    __wrapped_QgsFieldFormatterRegistry_addFieldFormatter = QgsFieldFormatterRegistry.addFieldFormatter
    def __QgsFieldFormatterRegistry_addFieldFormatter_wrapper(self, arg):
        __tracebackhide__ = True
        _QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsFieldFormatterRegistry_addFieldFormatter(self, arg)
    QgsFieldFormatterRegistry.addFieldFormatter = _functools.update_wrapper(__QgsFieldFormatterRegistry_addFieldFormatter_wrapper, QgsFieldFormatterRegistry.addFieldFormatter)

except (NameError, AttributeError):
    pass
