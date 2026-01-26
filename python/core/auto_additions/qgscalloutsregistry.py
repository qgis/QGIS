# The following has been generated automatically from src/core/callouts/qgscalloutsregistry.h
try:
    QgsCalloutRegistry.defaultCallout = staticmethod(QgsCalloutRegistry.defaultCallout)
    QgsCalloutRegistry.__group__ = ['callouts']
except (NameError, AttributeError):
    pass
try:
    QgsCalloutAbstractMetadata.__virtual_methods__ = ['createCalloutWidget']
    QgsCalloutAbstractMetadata.__abstract_methods__ = ['createCallout']
    QgsCalloutAbstractMetadata.__group__ = ['callouts']
except (NameError, AttributeError):
    pass
try:
    QgsCalloutMetadata.__overridden_methods__ = ['createCallout', 'createCalloutWidget']
    QgsCalloutMetadata.__group__ = ['callouts']
except (NameError, AttributeError):
    pass
