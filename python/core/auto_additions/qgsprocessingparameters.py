# The following has been generated automatically from src/core/processing/qgsprocessingparameters.h
# monkey patching scoped based enum
QgsProcessingFeatureSourceDefinition.Flag.FlagOverrideDefaultGeometryCheck.__doc__ = "If set, the default geometry check method (as dictated by QgsProcessingContext) will be overridden for this source"
QgsProcessingFeatureSourceDefinition.Flag.FlagCreateIndividualOutputPerInputFeature.__doc__ = "If set, every feature processed from this source will be placed into its own individually created output destination. Support for this flag depends on how an algorithm is executed."
QgsProcessingFeatureSourceDefinition.Flag.__doc__ = 'Flags which control source behavior.\n\n.. versionadded:: 3.14\n\n' + '* ``FlagOverrideDefaultGeometryCheck``: ' + QgsProcessingFeatureSourceDefinition.Flag.FlagOverrideDefaultGeometryCheck.__doc__ + '\n' + '* ``FlagCreateIndividualOutputPerInputFeature``: ' + QgsProcessingFeatureSourceDefinition.Flag.FlagCreateIndividualOutputPerInputFeature.__doc__
# --
