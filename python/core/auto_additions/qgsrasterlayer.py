# The following has been generated automatically from src/core/raster/qgsrasterlayer.h
try:
    QgsRasterLayer.__attribute_docs__ = {'SAMPLE_SIZE': 'Default sample size (number of pixels) for estimated statistics/histogram calculation', 'SINGLE_BAND_ENHANCEMENT_ALGORITHM': 'Default enhancement algorithm for single band raster', 'MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM': 'Default enhancement algorithm for multiple band raster of type Byte', 'MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM': 'Default enhancement algorithm for multiple band raster of type different from Byte', 'SINGLE_BAND_MIN_MAX_LIMITS': 'Default enhancement limits for single band raster', 'MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS': 'Default enhancement limits for multiple band raster of type Byte', 'MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS': 'Default enhancement limits for multiple band raster of type different from Byte', 'subsetStringChanged': "Emitted when the layer's subset string has changed.\n\n.. versionadded:: 3.12\n"}
    QgsRasterLayer.isValidRasterFileName = staticmethod(QgsRasterLayer.isValidRasterFileName)
    QgsRasterLayer.lastModified = staticmethod(QgsRasterLayer.lastModified)
    QgsRasterLayer.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterLayer.LayerOptions.__attribute_docs__ = {'loadDefaultStyle': 'Sets to ``True`` if the default layer style should be loaded', 'transformContext': 'Coordinate transform context\n\n.. versionadded:: 3.8', 'skipCrsValidation': "Controls whether the layer is allowed to have an invalid/unknown CRS.\n\nIf ``True``, then no validation will be performed on the layer's CRS and the layer\nlayer's :py:func:`~QgsRasterLayer.crs` may be :py:func:`~QgsRasterLayer.invalid` (i.e. the layer will have no georeferencing available\nand will be treated as having purely numerical coordinates).\n\nIf ``False`` (the default), the layer's CRS will be validated using :py:func:`QgsCoordinateReferenceSystem.validate()`,\nwhich may cause a blocking, user-facing dialog asking users to manually select the correct CRS for the\nlayer.\n\n.. versionadded:: 3.10"}
    QgsRasterLayer.LayerOptions.__doc__ = """Setting options for loading raster layers."""
    QgsRasterLayer.LayerOptions.__group__ = ['raster']
except (NameError, AttributeError):
    pass
