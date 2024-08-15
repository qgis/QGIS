# The following has been generated automatically from src/core/metadata/qgslayermetadata.h
try:
    QgsLayerMetadata.__attribute_docs__ = {'extentCrs': 'Coordinate reference system for spatial extent.\nThe CRS should match the CRS defined in the QgsLayerMetadata CRS property.\n\n.. seealso:: :py:func:`QgsLayerMetadata.crs`\n\n.. seealso:: :py:func:`spatial`', 'bounds': 'Geospatial extent of the resource. X and Y coordinates are in the\nCRS defined by the metadata (see extentCrs).\n\nWhile the spatial extent can include a Z dimension, this is not\ncompulsory.\n\n.. seealso:: :py:func:`extentCrs`', 'type': "Constraint type. Standard values include 'access' and 'other', however any\nstring can be used for the type.", 'constraint': 'Free-form constraint string.'}
except NameError:
    pass
QgsLayerMetadata.SpatialExtent.__doc__ = """Metadata spatial extent structure."""
QgsLayerMetadata.Extent.__doc__ = """Metadata extent structure."""
QgsLayerMetadata.Constraint.__doc__ = """Metadata constraint structure."""
