# The following has been generated automatically from src/core/metadata/qgslayermetadata.h
try:
    QgsLayerMetadata.SpatialExtent.__attribute_docs__ = {'extentCrs': 'Coordinate reference system for spatial extent.\nThe CRS should match the CRS defined in the QgsLayerMetadata CRS property.\n\n.. seealso:: :py:func:`QgsLayerMetadata.crs`\n\n.. seealso:: :py:func:`spatial`', 'bounds': 'Geospatial extent of the resource. X and Y coordinates are in the\nCRS defined by the metadata (see extentCrs).\n\nWhile the spatial extent can include a Z dimension, this is not\ncompulsory.\n\n.. seealso:: :py:func:`extentCrs`'}
    QgsLayerMetadata.SpatialExtent.__doc__ = """Metadata spatial extent structure."""
    QgsLayerMetadata.SpatialExtent.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
try:
    QgsLayerMetadata.Constraint.__attribute_docs__ = {'type': "Constraint type. Standard values include 'access' and 'other', however any\nstring can be used for the type.", 'constraint': 'Free-form constraint string.'}
    QgsLayerMetadata.Constraint.__doc__ = """Metadata constraint structure."""
    QgsLayerMetadata.Constraint.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
try:
    QgsLayerMetadata.Extent.__doc__ = """Metadata extent structure."""
    QgsLayerMetadata.Extent.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
try:
    QgsLayerMetadata.__group__ = ['metadata']
except (NameError, AttributeError):
    pass
