# The following has been generated automatically from src/core/layout/qgsreportsectionlayout.h
try:
    QgsReportSectionLayout.__overridden_methods__ = ['type', 'description', 'icon', 'clone', 'beginRender', 'nextBody', 'reloadSettings', 'writePropertiesToElement', 'readPropertiesFromElement']
    import functools as _functools
    __wrapped_QgsReportSectionLayout_setBody = QgsReportSectionLayout.setBody
    def __QgsReportSectionLayout_setBody_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsReportSectionLayout_setBody(self, arg)
    QgsReportSectionLayout.setBody = _functools.update_wrapper(__QgsReportSectionLayout_setBody_wrapper, QgsReportSectionLayout.setBody)

    QgsReportSectionLayout.__group__ = ['layout']
except (NameError, AttributeError):
    pass
