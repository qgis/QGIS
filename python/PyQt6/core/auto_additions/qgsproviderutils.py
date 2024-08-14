# The following has been generated automatically from src/core/providers/qgsproviderutils.h
# monkey patching scoped based enum
QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount.__doc__ = "Indicates that an unknown feature count should not be considered as incomplete"
QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType.__doc__ = "Indicates that an unknown geometry type should not be considered as incomplete"
QgsProviderUtils.SublayerCompletenessFlag.__doc__ = "Flags which control how :py:func:`QgsProviderUtils.sublayerDetailsAreIncomplete()` tests for completeness.\n\n" + '* ``IgnoreUnknownFeatureCount``: ' + QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount.__doc__ + '\n' + '* ``IgnoreUnknownGeometryType``: ' + QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType.__doc__
# --
QgsProviderUtils.SublayerCompletenessFlags = lambda flags=0: QgsProviderUtils.SublayerCompletenessFlag(flags)
QgsProviderUtils.sublayerDetailsAreIncomplete = staticmethod(QgsProviderUtils.sublayerDetailsAreIncomplete)
QgsProviderUtils.suggestLayerNameFromFilePath = staticmethod(QgsProviderUtils.suggestLayerNameFromFilePath)
