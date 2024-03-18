# The following has been generated automatically from src/core/qgsmaplayerelevationproperties.h
# monkey patching scoped based enum
QgsMapLayerElevationProperties.ZOffset = QgsMapLayerElevationProperties.Property.ZOffset
QgsMapLayerElevationProperties.ZOffset.is_monkey_patched = True
QgsMapLayerElevationProperties.ZOffset.__doc__ = ""
QgsMapLayerElevationProperties.ExtrusionHeight = QgsMapLayerElevationProperties.Property.ExtrusionHeight
QgsMapLayerElevationProperties.ExtrusionHeight.is_monkey_patched = True
QgsMapLayerElevationProperties.ExtrusionHeight.__doc__ = "Extrusion height"
QgsMapLayerElevationProperties.RasterPerBandLowerElevation = QgsMapLayerElevationProperties.Property.RasterPerBandLowerElevation
QgsMapLayerElevationProperties.RasterPerBandLowerElevation.is_monkey_patched = True
QgsMapLayerElevationProperties.RasterPerBandLowerElevation.__doc__ = "Lower elevation for each raster band (since QGIS 3.38)"
QgsMapLayerElevationProperties.RasterPerBandUpperElevation = QgsMapLayerElevationProperties.Property.RasterPerBandUpperElevation
QgsMapLayerElevationProperties.RasterPerBandUpperElevation.is_monkey_patched = True
QgsMapLayerElevationProperties.RasterPerBandUpperElevation.__doc__ = "Upper elevation for each raster band (since QGIS 3.38)"
QgsMapLayerElevationProperties.Property.__doc__ = "Data definable properties.\n\n.. versionadded:: 3.26\n\n" + '* ``ZOffset``: ' + QgsMapLayerElevationProperties.Property.ZOffset.__doc__ + '\n' + '* ``ExtrusionHeight``: ' + QgsMapLayerElevationProperties.Property.ExtrusionHeight.__doc__ + '\n' + '* ``RasterPerBandLowerElevation``: ' + QgsMapLayerElevationProperties.Property.RasterPerBandLowerElevation.__doc__ + '\n' + '* ``RasterPerBandUpperElevation``: ' + QgsMapLayerElevationProperties.Property.RasterPerBandUpperElevation.__doc__
# --
QgsMapLayerElevationProperties.FlagDontInvalidateCachedRendersWhenRangeChanges = QgsMapLayerElevationProperties.Flag.FlagDontInvalidateCachedRendersWhenRangeChanges
QgsMapLayerElevationProperties.Flags = lambda flags=0: QgsMapLayerElevationProperties.Flag(flags)
