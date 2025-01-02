# The following has been generated automatically from src/core/providers/qgsproviderutils.h
# monkey patching scoped based enum
QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount.__doc__ = "Indicates that an unknown feature count should not be considered as incomplete"
QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType.__doc__ = "Indicates that an unknown geometry type should not be considered as incomplete"
QgsProviderUtils.SublayerCompletenessFlag.__doc__ = """Flags which control how :py:func:`QgsProviderUtils.sublayerDetailsAreIncomplete()` tests for completeness.

* ``IgnoreUnknownFeatureCount``: Indicates that an unknown feature count should not be considered as incomplete
* ``IgnoreUnknownGeometryType``: Indicates that an unknown geometry type should not be considered as incomplete

"""
# --
try:
    QgsProviderUtils.sublayerDetailsAreIncomplete = staticmethod(QgsProviderUtils.sublayerDetailsAreIncomplete)
    QgsProviderUtils.suggestLayerNameFromFilePath = staticmethod(QgsProviderUtils.suggestLayerNameFromFilePath)
    QgsProviderUtils.__group__ = ['providers']
except (NameError, AttributeError):
    pass
