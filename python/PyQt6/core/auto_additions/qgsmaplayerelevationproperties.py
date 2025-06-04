# The following has been generated automatically from src/core/qgsmaplayerelevationproperties.h
# monkey patching scoped based enum
QgsMapLayerElevationProperties.ZOffset = QgsMapLayerElevationProperties.Property.ZOffset
QgsMapLayerElevationProperties.ZOffset.is_monkey_patched = True
QgsMapLayerElevationProperties.ZOffset.__doc__ = "Z offset"
QgsMapLayerElevationProperties.ExtrusionHeight = QgsMapLayerElevationProperties.Property.ExtrusionHeight
QgsMapLayerElevationProperties.ExtrusionHeight.is_monkey_patched = True
QgsMapLayerElevationProperties.ExtrusionHeight.__doc__ = "Extrusion height"
QgsMapLayerElevationProperties.RasterPerBandLowerElevation = QgsMapLayerElevationProperties.Property.RasterPerBandLowerElevation
QgsMapLayerElevationProperties.RasterPerBandLowerElevation.is_monkey_patched = True
QgsMapLayerElevationProperties.RasterPerBandLowerElevation.__doc__ = "Lower elevation for each raster band \n.. versionadded:: 3.38"
QgsMapLayerElevationProperties.RasterPerBandUpperElevation = QgsMapLayerElevationProperties.Property.RasterPerBandUpperElevation
QgsMapLayerElevationProperties.RasterPerBandUpperElevation.is_monkey_patched = True
QgsMapLayerElevationProperties.RasterPerBandUpperElevation.__doc__ = "Upper elevation for each raster band \n.. versionadded:: 3.38"
QgsMapLayerElevationProperties.Property.__doc__ = """Data definable properties.

.. versionadded:: 3.26

* ``ZOffset``: Z offset
* ``ExtrusionHeight``: Extrusion height
* ``RasterPerBandLowerElevation``: Lower elevation for each raster band

  .. versionadded:: 3.38

* ``RasterPerBandUpperElevation``: Upper elevation for each raster band

  .. versionadded:: 3.38


"""
# --
QgsMapLayerElevationProperties.FlagDontInvalidateCachedRendersWhenRangeChanges = QgsMapLayerElevationProperties.Flag.FlagDontInvalidateCachedRendersWhenRangeChanges
QgsMapLayerElevationProperties.Flags = lambda flags=0: QgsMapLayerElevationProperties.Flag(flags)
try:
    QgsMapLayerElevationProperties.__attribute_docs__ = {'changed': 'Emitted when any of the elevation properties have changed.\n\nSee :py:func:`~QgsMapLayerElevationProperties.renderingPropertyChanged`\nand\n:py:func:`~QgsMapLayerElevationProperties.profileGenerationPropertyChanged`\nfor more fine-grained signals.\n', 'zOffsetChanged': 'Emitted when the z offset changes.\n\n.. versionadded:: 3.26\n', 'zScaleChanged': 'Emitted when the z scale changes.\n\n.. versionadded:: 3.26\n', 'profileRenderingPropertyChanged': 'Emitted when any of the elevation properties which relate solely to\npresentation of elevation results have changed.\n\n.. seealso:: :py:func:`changed`\n\n.. seealso:: :py:func:`profileGenerationPropertyChanged`\n\n.. versionadded:: 3.26\n', 'profileGenerationPropertyChanged': 'Emitted when any of the elevation properties which relate solely to\ngeneration of elevation profiles have changed.\n\n.. seealso:: :py:func:`changed`\n\n.. seealso:: :py:func:`profileRenderingPropertyChanged`\n\n.. versionadded:: 3.26\n'}
    QgsMapLayerElevationProperties.propertyDefinitions = staticmethod(QgsMapLayerElevationProperties.propertyDefinitions)
    QgsMapLayerElevationProperties.__virtual_methods__ = ['hasElevation', 'setDefaultsFromLayer', 'htmlSummary', 'isVisibleInZRange', 'flags', 'calculateZRange', 'significantZValues', 'showByDefaultInElevationProfilePlots']
    QgsMapLayerElevationProperties.__abstract_methods__ = ['writeXml', 'readXml', 'clone']
except (NameError, AttributeError):
    pass
