# The following has been generated automatically from src/core/providers/arcgis/qgsarcgisrestutils.h
# monkey patching scoped based enum
QgsArcGisRestUtils.FeatureToJsonFlag.IncludeGeometry.__doc__ = "Whether to include the geometry definition"
QgsArcGisRestUtils.FeatureToJsonFlag.IncludeNonObjectIdAttributes.__doc__ = "Whether to include any non-objectId attributes"
QgsArcGisRestUtils.FeatureToJsonFlag.__doc__ = 'Flags which control the behavior of converting features to JSON.\n\n.. versionadded:: 3.28\n\n' + '* ``IncludeGeometry``: ' + QgsArcGisRestUtils.FeatureToJsonFlag.IncludeGeometry.__doc__ + '\n' + '* ``IncludeNonObjectIdAttributes``: ' + QgsArcGisRestUtils.FeatureToJsonFlag.IncludeNonObjectIdAttributes.__doc__
# --
QgsArcGisRestUtils.FeatureToJsonFlag.baseClass = QgsArcGisRestUtils
QgsArcGisRestUtils.FeatureToJsonFlags.baseClass = QgsArcGisRestUtils
FeatureToJsonFlags = QgsArcGisRestUtils  # dirty hack since SIP seems to introduce the flags in module
