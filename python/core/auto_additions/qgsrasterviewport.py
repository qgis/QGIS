# The following has been generated automatically from src/core/raster/qgsrasterviewport.h
try:
    QgsRasterViewPort.__attribute_docs__ = {'mTopLeftPoint': 'Coordinate (in output device coordinate system) of top left corner\nof the part of the raster that is to be rendered.', 'mBottomRightPoint': 'Coordinate (in output device coordinate system) of bottom right corner\nof the part of the raster that is to be rendered.', 'mWidth': 'Width, number of columns to be rendered', 'mHeight': 'Height, number of rows to be rendered', 'mDrawnExtent': 'Intersection of current map extent and layer extent, in map (destination) CRS', 'mSrcCRS': 'Source (layer) coordinate system', 'mDestCRS': 'Target (map) coordinate system', 'mTransformContext': 'Coordinate transform context'}
    QgsRasterViewPort.__doc__ = """This class provides details of the viewable area that a raster will
be rendered into.

The qgsrasterviewport class sets up a viewport / area of interest to be used
by rasterlayer draw functions at the point of drawing to the screen."""
    QgsRasterViewPort.__group__ = ['raster']
except (NameError, AttributeError):
    pass
