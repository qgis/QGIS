# The following has been generated automatically from src/core/layout/qgsabstractreportsection.h
QgsAbstractReportSection.Header = QgsAbstractReportSection.SubSection.Header
QgsAbstractReportSection.Body = QgsAbstractReportSection.SubSection.Body
QgsAbstractReportSection.Children = QgsAbstractReportSection.SubSection.Children
QgsAbstractReportSection.Footer = QgsAbstractReportSection.SubSection.Footer
QgsAbstractReportSection.End = QgsAbstractReportSection.SubSection.End
try:
    QgsReportSectionContext.__attribute_docs__ = {'feature': 'Current feature', 'currentLayer': 'Current coverage layer', 'fieldFilters': 'Current field filters'}
    QgsReportSectionContext.__annotations__ = {'feature': 'QgsFeature', 'currentLayer': 'QgsVectorLayer', 'fieldFilters': 'Dict[str, object]'}
    QgsReportSectionContext.__group__ = ['layout']
except (NameError, AttributeError):
    pass
try:
    QgsAbstractReportSection.__virtual_methods__ = ['reset', 'prepareHeader', 'prepareFooter', 'nextBody', 'reloadSettings', 'setParentSection', 'writePropertiesToElement', 'readPropertiesFromElement']
    QgsAbstractReportSection.__abstract_methods__ = ['type', 'description', 'icon', 'clone']
    QgsAbstractReportSection.__overridden_methods__ = ['count', 'filePath', 'layout', 'beginRender', 'next', 'endRender']
    QgsAbstractReportSection.__group__ = ['layout']
except (NameError, AttributeError):
    pass
