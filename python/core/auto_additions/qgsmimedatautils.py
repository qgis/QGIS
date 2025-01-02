# The following has been generated automatically from src/core/qgsmimedatautils.h
try:
    QgsMimeDataUtils.Uri.__attribute_docs__ = {'layerType': 'Type of URI.\n\nRecognized types include\n\n- "vector": vector layers\n- "raster": raster layers\n- "mesh": mesh layers\n- "pointcloud": point cloud layers\n- "vector-tile": vector tile layers\n- "tiled-scene": tiled scene layers\n- "plugin": plugin layers\n- "custom": custom types\n- "project": QGS/QGZ project file\n- "directory": directory path\n\nMime data from plugins may use additional custom layer types.', 'providerKey': 'For "vector" / "raster" type: provider id.\nFor "plugin" type: plugin layer type name.\nFor "custom" type: key of its :py:class:`QgsCustomDropHandler`\nFor "project" and "directory" types: unused', 'name': 'Human readable name to be used e.g. in layer tree', 'uri': 'Identifier of the data source recognized by its providerKey', 'layerId': 'Layer ID, if uri is associated with a layer from a :py:class:`QgsProject`.\n\n.. versionadded:: 3.8', 'pId': 'Unique ID associated with application instance. Can be used to identify\nif mime data was created inside the current application instance or not.\n\n.. versionadded:: 3.8', 'wkbType': 'WKB type, if associated with a vector layer, or :py:class:`QgsWkbTypes`.Unknown if not\nyet known.\n\n.. versionadded:: 3.8', 'filePath': 'Path to file, if uri is associated with a file.\n\n.. versionadded:: 3.22'}
except (NameError, AttributeError):
    pass
try:
    QgsMimeDataUtils.encodeUriList = staticmethod(QgsMimeDataUtils.encodeUriList)
    QgsMimeDataUtils.isUriList = staticmethod(QgsMimeDataUtils.isUriList)
    QgsMimeDataUtils.decodeUriList = staticmethod(QgsMimeDataUtils.decodeUriList)
    QgsMimeDataUtils.layerTreeNodesToUriList = staticmethod(QgsMimeDataUtils.layerTreeNodesToUriList)
    QgsMimeDataUtils.hasOriginatedFromCurrentAppInstance = staticmethod(QgsMimeDataUtils.hasOriginatedFromCurrentAppInstance)
except (NameError, AttributeError):
    pass
