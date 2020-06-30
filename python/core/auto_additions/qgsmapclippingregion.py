# The following has been generated automatically from src/core/qgsmapclippingregion.h
# monkey patching scoped based enum
QgsMapClippingRegion.FeatureClippingType.Intersect.__doc__ = "Clip the geometry of these features to the region prior to rendering (i.e. feature boundaries will follow the clip region)"
QgsMapClippingRegion.FeatureClippingType.PainterClip.__doc__ = "Applying clipping on the painter only (i.e. feature boundaries will be unchanged, but may be invisible where the feature falls outside the clipping region)"
QgsMapClippingRegion.FeatureClippingType.Intersects.__doc__ = "Only render features which intersect the clipping region, but do not clip these features to the region"
QgsMapClippingRegion.FeatureClippingType.__doc__ = 'Feature clipping behavior.\n\n' + '* ``Intersect``: ' + QgsMapClippingRegion.FeatureClippingType.Intersect.__doc__ + '\n' + '* ``PainterClip``: ' + QgsMapClippingRegion.FeatureClippingType.PainterClip.__doc__ + '\n' + '* ``Intersects``: ' + QgsMapClippingRegion.FeatureClippingType.Intersects.__doc__
# --
