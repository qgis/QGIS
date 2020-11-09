# The following has been generated automatically from src/core/qgsmapclippingregion.h
# monkey patching scoped based enum
QgsMapClippingRegion.FeatureClippingType.ClipToIntersection.__doc__ = "Clip the geometry of these features to the region prior to rendering (i.e. feature boundaries will follow the clip region)"
QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly.__doc__ = "Applying clipping on the painter only (i.e. feature boundaries will be unchanged, but may be invisible where the feature falls outside the clipping region)"
QgsMapClippingRegion.FeatureClippingType.NoClipping.__doc__ = "Only render features which intersect the clipping region, but do not clip these features to the region"
QgsMapClippingRegion.FeatureClippingType.__doc__ = 'Feature clipping behavior, which controls how features from vector layers\nwill be clipped.\n\n' + '* ``ClipToIntersection``: ' + QgsMapClippingRegion.FeatureClippingType.ClipToIntersection.__doc__ + '\n' + '* ``ClipPainterOnly``: ' + QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly.__doc__ + '\n' + '* ``NoClipping``: ' + QgsMapClippingRegion.FeatureClippingType.NoClipping.__doc__
# --
