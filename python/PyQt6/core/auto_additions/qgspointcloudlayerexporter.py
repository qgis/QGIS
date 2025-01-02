# The following has been generated automatically from src/core/pointcloud/qgspointcloudlayerexporter.h
# monkey patching scoped based enum
QgsPointCloudLayerExporter.ExportFormat.Memory.__doc__ = "Memory layer"
QgsPointCloudLayerExporter.ExportFormat.Las.__doc__ = "LAS/LAZ point cloud"
QgsPointCloudLayerExporter.ExportFormat.Gpkg.__doc__ = "Geopackage"
QgsPointCloudLayerExporter.ExportFormat.Shp.__doc__ = "ESRI ShapeFile"
QgsPointCloudLayerExporter.ExportFormat.Dxf.__doc__ = "AutoCAD dxf"
QgsPointCloudLayerExporter.ExportFormat.Csv.__doc__ = "Comma separated values"
QgsPointCloudLayerExporter.ExportFormat.__doc__ = """Supported export formats for point clouds

* ``Memory``: Memory layer
* ``Las``: LAS/LAZ point cloud
* ``Gpkg``: Geopackage
* ``Shp``: ESRI ShapeFile
* ``Dxf``: AutoCAD dxf
* ``Csv``: Comma separated values

"""
# --
try:
    QgsPointCloudLayerExporterTask.__attribute_docs__ = {'exportComplete': 'Emitted when exporting the layer is successfully completed.\n'}
    QgsPointCloudLayerExporterTask.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
try:
    QgsPointCloudLayerExporter.__group__ = ['pointcloud']
except (NameError, AttributeError):
    pass
