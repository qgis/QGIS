# The following has been generated automatically from src/core/layout/qgsreportsectionfieldgroup.h
QgsReportSectionFieldGroup.IncludeWhenFeaturesFound = QgsReportSectionFieldGroup.SectionVisibility.IncludeWhenFeaturesFound
QgsReportSectionFieldGroup.AlwaysInclude = QgsReportSectionFieldGroup.SectionVisibility.AlwaysInclude
try:
    QgsReportSectionFieldGroup.__overridden_methods__ = ['type', 'description', 'icon', 'clone', 'beginRender', 'prepareHeader', 'prepareFooter', 'nextBody', 'reset', 'setParentSection', 'reloadSettings', 'writePropertiesToElement', 'readPropertiesFromElement']
    import functools as _functools
    __wrapped_QgsReportSectionFieldGroup_setBody = QgsReportSectionFieldGroup.setBody
    def __QgsReportSectionFieldGroup_setBody_wrapper(self, arg):
        __tracebackhide__ = True
        QgsSipUtils.verifyIsPyOwned(arg, 'you dont have ownership')
        return __wrapped_QgsReportSectionFieldGroup_setBody(self, arg)
    QgsReportSectionFieldGroup.setBody = _functools.update_wrapper(__QgsReportSectionFieldGroup_setBody_wrapper, QgsReportSectionFieldGroup.setBody)

    QgsReportSectionFieldGroup.__group__ = ['layout']
except (NameError, AttributeError):
    pass
