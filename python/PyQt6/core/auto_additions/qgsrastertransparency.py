# The following has been generated automatically from src/core/raster/qgsrastertransparency.h
try:
    QgsRasterTransparency.TransparentThreeValuePixel.__attribute_docs__ = {'red': 'Red pixel value.', 'green': 'Green pixel value.', 'blue': 'Blue pixel value.', 'opacity': 'Opacity for pixel, between 0 and 1.0.\n\n.. versionadded:: 3.38', 'fuzzyToleranceRed': "Fuzzy tolerance for red values.\n\nIf non zero, the pixel's red component can deviate from values specified in this object by a maximum of this tolerance amount.\n\n.. versionadded:: 3.40", 'fuzzyToleranceGreen': "Fuzzy tolerance for green values.\n\nIf non zero, the pixel's green component can deviate from values specified in this object by a maximum of this tolerance amount.\n\n.. versionadded:: 3.40", 'fuzzyToleranceBlue': "Fuzzy tolerance for blue values.\n\nIf non zero, the pixel's blue component can deviate from values specified in this object by a maximum of this tolerance amount.\n\n.. versionadded:: 3.40"}
    QgsRasterTransparency.TransparentThreeValuePixel.__doc__ = """Defines the transparency for a RGB pixel value."""
    QgsRasterTransparency.TransparentThreeValuePixel.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterTransparency.TransparentSingleValuePixel.__attribute_docs__ = {'min': 'Minimum pixel value to include in range.', 'max': 'Maximum pixel value to include in range.', 'opacity': 'Opacity for pixel, between 0 and 1.0.\n\n.. versionadded:: 3.38', 'includeMinimum': '``True`` if pixels matching the min value should be considered transparent,\nor ``False`` if only pixels greater than the min value should be transparent.\n\n.. versionadded:: 3.38', 'includeMaximum': '``True`` if pixels matching the max value should be considered transparent,\nor ``False`` if only pixels less than the max value should be transparent.\n\n.. versionadded:: 3.38'}
    QgsRasterTransparency.TransparentSingleValuePixel.__doc__ = """Defines the transparency for a range of single-band pixel values."""
    QgsRasterTransparency.TransparentSingleValuePixel.__group__ = ['raster']
except (NameError, AttributeError):
    pass
try:
    QgsRasterTransparency.__group__ = ['raster']
except (NameError, AttributeError):
    pass
