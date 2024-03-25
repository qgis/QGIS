# The following has been generated automatically from src/core/qgis.h
Qgis.Info = Qgis.MessageLevel.Info
Qgis.Warning = Qgis.MessageLevel.Warning
Qgis.Critical = Qgis.MessageLevel.Critical
Qgis.Success = Qgis.MessageLevel.Success
Qgis.NoLevel = Qgis.MessageLevel.NoLevel
Qgis.MessageLevel.baseClass = Qgis
QgsMapLayer.LayerType = Qgis.LayerType
# monkey patching scoped based enum
QgsMapLayer.VectorLayer = Qgis.LayerType.Vector
QgsMapLayer.LayerType.VectorLayer = Qgis.LayerType.Vector
QgsMapLayer.VectorLayer.is_monkey_patched = True
QgsMapLayer.VectorLayer.__doc__ = "Vector layer"
QgsMapLayer.RasterLayer = Qgis.LayerType.Raster
QgsMapLayer.LayerType.RasterLayer = Qgis.LayerType.Raster
QgsMapLayer.RasterLayer.is_monkey_patched = True
QgsMapLayer.RasterLayer.__doc__ = "Raster layer"
QgsMapLayer.PluginLayer = Qgis.LayerType.Plugin
QgsMapLayer.LayerType.PluginLayer = Qgis.LayerType.Plugin
QgsMapLayer.PluginLayer.is_monkey_patched = True
QgsMapLayer.PluginLayer.__doc__ = "Plugin based layer"
QgsMapLayer.MeshLayer = Qgis.LayerType.Mesh
QgsMapLayer.LayerType.MeshLayer = Qgis.LayerType.Mesh
QgsMapLayer.MeshLayer.is_monkey_patched = True
QgsMapLayer.MeshLayer.__doc__ = "Mesh layer. Added in QGIS 3.2"
QgsMapLayer.VectorTileLayer = Qgis.LayerType.VectorTile
QgsMapLayer.LayerType.VectorTileLayer = Qgis.LayerType.VectorTile
QgsMapLayer.VectorTileLayer.is_monkey_patched = True
QgsMapLayer.VectorTileLayer.__doc__ = "Vector tile layer. Added in QGIS 3.14"
QgsMapLayer.AnnotationLayer = Qgis.LayerType.Annotation
QgsMapLayer.LayerType.AnnotationLayer = Qgis.LayerType.Annotation
QgsMapLayer.AnnotationLayer.is_monkey_patched = True
QgsMapLayer.AnnotationLayer.__doc__ = "Contains freeform, georeferenced annotations. Added in QGIS 3.16"
QgsMapLayer.PointCloudLayer = Qgis.LayerType.PointCloud
QgsMapLayer.LayerType.PointCloudLayer = Qgis.LayerType.PointCloud
QgsMapLayer.PointCloudLayer.is_monkey_patched = True
QgsMapLayer.PointCloudLayer.__doc__ = "Point cloud layer. Added in QGIS 3.18"
QgsMapLayer.GroupLayer = Qgis.LayerType.Group
QgsMapLayer.LayerType.GroupLayer = Qgis.LayerType.Group
QgsMapLayer.GroupLayer.is_monkey_patched = True
QgsMapLayer.GroupLayer.__doc__ = "Composite group layer. Added in QGIS 3.24"
QgsMapLayer.TiledScene = Qgis.LayerType.TiledScene
QgsMapLayer.TiledScene.is_monkey_patched = True
QgsMapLayer.TiledScene.__doc__ = "Tiled scene layer. Added in QGIS 3.34"
Qgis.LayerType.__doc__ = "Types of layers that can be added to a map\n\n.. versionadded:: 3.30.\n\n" + '* ``VectorLayer``: ' + Qgis.LayerType.Vector.__doc__ + '\n' + '* ``RasterLayer``: ' + Qgis.LayerType.Raster.__doc__ + '\n' + '* ``PluginLayer``: ' + Qgis.LayerType.Plugin.__doc__ + '\n' + '* ``MeshLayer``: ' + Qgis.LayerType.Mesh.__doc__ + '\n' + '* ``VectorTileLayer``: ' + Qgis.LayerType.VectorTile.__doc__ + '\n' + '* ``AnnotationLayer``: ' + Qgis.LayerType.Annotation.__doc__ + '\n' + '* ``PointCloudLayer``: ' + Qgis.LayerType.PointCloud.__doc__ + '\n' + '* ``GroupLayer``: ' + Qgis.LayerType.Group.__doc__ + '\n' + '* ``TiledScene``: ' + Qgis.LayerType.TiledScene.__doc__
# --
Qgis.LayerType.baseClass = Qgis
QgsMapLayerProxyModel.Filter = Qgis.LayerFilter
# monkey patching scoped based enum
QgsMapLayerProxyModel.RasterLayer = Qgis.LayerFilter.RasterLayer
QgsMapLayerProxyModel.RasterLayer.is_monkey_patched = True
QgsMapLayerProxyModel.RasterLayer.__doc__ = ""
QgsMapLayerProxyModel.NoGeometry = Qgis.LayerFilter.NoGeometry
QgsMapLayerProxyModel.NoGeometry.is_monkey_patched = True
QgsMapLayerProxyModel.NoGeometry.__doc__ = ""
QgsMapLayerProxyModel.PointLayer = Qgis.LayerFilter.PointLayer
QgsMapLayerProxyModel.PointLayer.is_monkey_patched = True
QgsMapLayerProxyModel.PointLayer.__doc__ = ""
QgsMapLayerProxyModel.LineLayer = Qgis.LayerFilter.LineLayer
QgsMapLayerProxyModel.LineLayer.is_monkey_patched = True
QgsMapLayerProxyModel.LineLayer.__doc__ = ""
QgsMapLayerProxyModel.PolygonLayer = Qgis.LayerFilter.PolygonLayer
QgsMapLayerProxyModel.PolygonLayer.is_monkey_patched = True
QgsMapLayerProxyModel.PolygonLayer.__doc__ = ""
QgsMapLayerProxyModel.HasGeometry = Qgis.LayerFilter.HasGeometry
QgsMapLayerProxyModel.HasGeometry.is_monkey_patched = True
QgsMapLayerProxyModel.HasGeometry.__doc__ = ""
QgsMapLayerProxyModel.VectorLayer = Qgis.LayerFilter.VectorLayer
QgsMapLayerProxyModel.VectorLayer.is_monkey_patched = True
QgsMapLayerProxyModel.VectorLayer.__doc__ = ""
QgsMapLayerProxyModel.PluginLayer = Qgis.LayerFilter.PluginLayer
QgsMapLayerProxyModel.PluginLayer.is_monkey_patched = True
QgsMapLayerProxyModel.PluginLayer.__doc__ = ""
QgsMapLayerProxyModel.WritableLayer = Qgis.LayerFilter.WritableLayer
QgsMapLayerProxyModel.WritableLayer.is_monkey_patched = True
QgsMapLayerProxyModel.WritableLayer.__doc__ = ""
QgsMapLayerProxyModel.MeshLayer = Qgis.LayerFilter.MeshLayer
QgsMapLayerProxyModel.MeshLayer.is_monkey_patched = True
QgsMapLayerProxyModel.MeshLayer.__doc__ = "QgsMeshLayer \n.. versionadded:: 3.6"
QgsMapLayerProxyModel.VectorTileLayer = Qgis.LayerFilter.VectorTileLayer
QgsMapLayerProxyModel.VectorTileLayer.is_monkey_patched = True
QgsMapLayerProxyModel.VectorTileLayer.__doc__ = "QgsVectorTileLayer \n.. versionadded:: 3.14"
QgsMapLayerProxyModel.PointCloudLayer = Qgis.LayerFilter.PointCloudLayer
QgsMapLayerProxyModel.PointCloudLayer.is_monkey_patched = True
QgsMapLayerProxyModel.PointCloudLayer.__doc__ = "QgsPointCloudLayer \n.. versionadded:: 3.18"
QgsMapLayerProxyModel.AnnotationLayer = Qgis.LayerFilter.AnnotationLayer
QgsMapLayerProxyModel.AnnotationLayer.is_monkey_patched = True
QgsMapLayerProxyModel.AnnotationLayer.__doc__ = "QgsAnnotationLayer \n.. versionadded:: 3.22"
QgsMapLayerProxyModel.TiledSceneLayer = Qgis.LayerFilter.TiledSceneLayer
QgsMapLayerProxyModel.TiledSceneLayer.is_monkey_patched = True
QgsMapLayerProxyModel.TiledSceneLayer.__doc__ = "QgsTiledSceneLayer \n.. versionadded:: 3.34"
QgsMapLayerProxyModel.All = Qgis.LayerFilter.All
QgsMapLayerProxyModel.All.is_monkey_patched = True
QgsMapLayerProxyModel.All.__doc__ = ""
QgsMapLayerProxyModel.SpatialLayer = Qgis.LayerFilter.SpatialLayer
QgsMapLayerProxyModel.SpatialLayer.is_monkey_patched = True
QgsMapLayerProxyModel.SpatialLayer.__doc__ = ".. versionadded:: 3.24"
Qgis.LayerFilter.__doc__ = "Filter for layers\n\n.. versionadded:: 3.34.\n\n" + '* ``RasterLayer``: ' + Qgis.LayerFilter.RasterLayer.__doc__ + '\n' + '* ``NoGeometry``: ' + Qgis.LayerFilter.NoGeometry.__doc__ + '\n' + '* ``PointLayer``: ' + Qgis.LayerFilter.PointLayer.__doc__ + '\n' + '* ``LineLayer``: ' + Qgis.LayerFilter.LineLayer.__doc__ + '\n' + '* ``PolygonLayer``: ' + Qgis.LayerFilter.PolygonLayer.__doc__ + '\n' + '* ``HasGeometry``: ' + Qgis.LayerFilter.HasGeometry.__doc__ + '\n' + '* ``VectorLayer``: ' + Qgis.LayerFilter.VectorLayer.__doc__ + '\n' + '* ``PluginLayer``: ' + Qgis.LayerFilter.PluginLayer.__doc__ + '\n' + '* ``WritableLayer``: ' + Qgis.LayerFilter.WritableLayer.__doc__ + '\n' + '* ``MeshLayer``: ' + Qgis.LayerFilter.MeshLayer.__doc__ + '\n' + '* ``VectorTileLayer``: ' + Qgis.LayerFilter.VectorTileLayer.__doc__ + '\n' + '* ``PointCloudLayer``: ' + Qgis.LayerFilter.PointCloudLayer.__doc__ + '\n' + '* ``AnnotationLayer``: ' + Qgis.LayerFilter.AnnotationLayer.__doc__ + '\n' + '* ``TiledSceneLayer``: ' + Qgis.LayerFilter.TiledSceneLayer.__doc__ + '\n' + '* ``All``: ' + Qgis.LayerFilter.All.__doc__ + '\n' + '* ``SpatialLayer``: ' + Qgis.LayerFilter.SpatialLayer.__doc__
# --
Qgis.LayerFilters = lambda flags=0: Qgis.LayerFilter(flags)
Qgis.LayerFilters.baseClass = Qgis
LayerFilters = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsWkbTypes.Type = Qgis.WkbType
# monkey patching scoped based enum
QgsWkbTypes.Unknown = Qgis.WkbType.Unknown
QgsWkbTypes.Unknown.is_monkey_patched = True
QgsWkbTypes.Unknown.__doc__ = "Unknown"
QgsWkbTypes.Point = Qgis.WkbType.Point
QgsWkbTypes.Point.is_monkey_patched = True
QgsWkbTypes.Point.__doc__ = "Point"
QgsWkbTypes.LineString = Qgis.WkbType.LineString
QgsWkbTypes.LineString.is_monkey_patched = True
QgsWkbTypes.LineString.__doc__ = "LineString"
QgsWkbTypes.Polygon = Qgis.WkbType.Polygon
QgsWkbTypes.Polygon.is_monkey_patched = True
QgsWkbTypes.Polygon.__doc__ = "Polygon"
QgsWkbTypes.Triangle = Qgis.WkbType.Triangle
QgsWkbTypes.Triangle.is_monkey_patched = True
QgsWkbTypes.Triangle.__doc__ = "Triangle"
QgsWkbTypes.MultiPoint = Qgis.WkbType.MultiPoint
QgsWkbTypes.MultiPoint.is_monkey_patched = True
QgsWkbTypes.MultiPoint.__doc__ = "MultiPoint"
QgsWkbTypes.MultiLineString = Qgis.WkbType.MultiLineString
QgsWkbTypes.MultiLineString.is_monkey_patched = True
QgsWkbTypes.MultiLineString.__doc__ = "MultiLineString"
QgsWkbTypes.MultiPolygon = Qgis.WkbType.MultiPolygon
QgsWkbTypes.MultiPolygon.is_monkey_patched = True
QgsWkbTypes.MultiPolygon.__doc__ = "MultiPolygon"
QgsWkbTypes.GeometryCollection = Qgis.WkbType.GeometryCollection
QgsWkbTypes.GeometryCollection.is_monkey_patched = True
QgsWkbTypes.GeometryCollection.__doc__ = "GeometryCollection"
QgsWkbTypes.CircularString = Qgis.WkbType.CircularString
QgsWkbTypes.CircularString.is_monkey_patched = True
QgsWkbTypes.CircularString.__doc__ = "CircularString"
QgsWkbTypes.CompoundCurve = Qgis.WkbType.CompoundCurve
QgsWkbTypes.CompoundCurve.is_monkey_patched = True
QgsWkbTypes.CompoundCurve.__doc__ = "CompoundCurve"
QgsWkbTypes.CurvePolygon = Qgis.WkbType.CurvePolygon
QgsWkbTypes.CurvePolygon.is_monkey_patched = True
QgsWkbTypes.CurvePolygon.__doc__ = "CurvePolygon"
QgsWkbTypes.MultiCurve = Qgis.WkbType.MultiCurve
QgsWkbTypes.MultiCurve.is_monkey_patched = True
QgsWkbTypes.MultiCurve.__doc__ = "MultiCurve"
QgsWkbTypes.MultiSurface = Qgis.WkbType.MultiSurface
QgsWkbTypes.MultiSurface.is_monkey_patched = True
QgsWkbTypes.MultiSurface.__doc__ = "MultiSurface"
QgsWkbTypes.NoGeometry = Qgis.WkbType.NoGeometry
QgsWkbTypes.NoGeometry.is_monkey_patched = True
QgsWkbTypes.NoGeometry.__doc__ = "No geometry"
QgsWkbTypes.PointZ = Qgis.WkbType.PointZ
QgsWkbTypes.PointZ.is_monkey_patched = True
QgsWkbTypes.PointZ.__doc__ = "PointZ"
QgsWkbTypes.LineStringZ = Qgis.WkbType.LineStringZ
QgsWkbTypes.LineStringZ.is_monkey_patched = True
QgsWkbTypes.LineStringZ.__doc__ = "LineStringZ"
QgsWkbTypes.PolygonZ = Qgis.WkbType.PolygonZ
QgsWkbTypes.PolygonZ.is_monkey_patched = True
QgsWkbTypes.PolygonZ.__doc__ = "PolygonZ"
QgsWkbTypes.TriangleZ = Qgis.WkbType.TriangleZ
QgsWkbTypes.TriangleZ.is_monkey_patched = True
QgsWkbTypes.TriangleZ.__doc__ = "TriangleZ"
QgsWkbTypes.MultiPointZ = Qgis.WkbType.MultiPointZ
QgsWkbTypes.MultiPointZ.is_monkey_patched = True
QgsWkbTypes.MultiPointZ.__doc__ = "MultiPointZ"
QgsWkbTypes.MultiLineStringZ = Qgis.WkbType.MultiLineStringZ
QgsWkbTypes.MultiLineStringZ.is_monkey_patched = True
QgsWkbTypes.MultiLineStringZ.__doc__ = "MultiLineStringZ"
QgsWkbTypes.MultiPolygonZ = Qgis.WkbType.MultiPolygonZ
QgsWkbTypes.MultiPolygonZ.is_monkey_patched = True
QgsWkbTypes.MultiPolygonZ.__doc__ = "MultiPolygonZ"
QgsWkbTypes.GeometryCollectionZ = Qgis.WkbType.GeometryCollectionZ
QgsWkbTypes.GeometryCollectionZ.is_monkey_patched = True
QgsWkbTypes.GeometryCollectionZ.__doc__ = "GeometryCollectionZ"
QgsWkbTypes.CircularStringZ = Qgis.WkbType.CircularStringZ
QgsWkbTypes.CircularStringZ.is_monkey_patched = True
QgsWkbTypes.CircularStringZ.__doc__ = "CircularStringZ"
QgsWkbTypes.CompoundCurveZ = Qgis.WkbType.CompoundCurveZ
QgsWkbTypes.CompoundCurveZ.is_monkey_patched = True
QgsWkbTypes.CompoundCurveZ.__doc__ = "CompoundCurveZ"
QgsWkbTypes.CurvePolygonZ = Qgis.WkbType.CurvePolygonZ
QgsWkbTypes.CurvePolygonZ.is_monkey_patched = True
QgsWkbTypes.CurvePolygonZ.__doc__ = "CurvePolygonZ"
QgsWkbTypes.MultiCurveZ = Qgis.WkbType.MultiCurveZ
QgsWkbTypes.MultiCurveZ.is_monkey_patched = True
QgsWkbTypes.MultiCurveZ.__doc__ = "MultiCurveZ"
QgsWkbTypes.MultiSurfaceZ = Qgis.WkbType.MultiSurfaceZ
QgsWkbTypes.MultiSurfaceZ.is_monkey_patched = True
QgsWkbTypes.MultiSurfaceZ.__doc__ = "MultiSurfaceZ"
QgsWkbTypes.PointM = Qgis.WkbType.PointM
QgsWkbTypes.PointM.is_monkey_patched = True
QgsWkbTypes.PointM.__doc__ = "PointM"
QgsWkbTypes.LineStringM = Qgis.WkbType.LineStringM
QgsWkbTypes.LineStringM.is_monkey_patched = True
QgsWkbTypes.LineStringM.__doc__ = "LineStringM"
QgsWkbTypes.PolygonM = Qgis.WkbType.PolygonM
QgsWkbTypes.PolygonM.is_monkey_patched = True
QgsWkbTypes.PolygonM.__doc__ = "PolygonM"
QgsWkbTypes.TriangleM = Qgis.WkbType.TriangleM
QgsWkbTypes.TriangleM.is_monkey_patched = True
QgsWkbTypes.TriangleM.__doc__ = "TriangleM"
QgsWkbTypes.MultiPointM = Qgis.WkbType.MultiPointM
QgsWkbTypes.MultiPointM.is_monkey_patched = True
QgsWkbTypes.MultiPointM.__doc__ = "MultiPointM"
QgsWkbTypes.MultiLineStringM = Qgis.WkbType.MultiLineStringM
QgsWkbTypes.MultiLineStringM.is_monkey_patched = True
QgsWkbTypes.MultiLineStringM.__doc__ = "MultiLineStringM"
QgsWkbTypes.MultiPolygonM = Qgis.WkbType.MultiPolygonM
QgsWkbTypes.MultiPolygonM.is_monkey_patched = True
QgsWkbTypes.MultiPolygonM.__doc__ = "MultiPolygonM"
QgsWkbTypes.GeometryCollectionM = Qgis.WkbType.GeometryCollectionM
QgsWkbTypes.GeometryCollectionM.is_monkey_patched = True
QgsWkbTypes.GeometryCollectionM.__doc__ = "GeometryCollectionM"
QgsWkbTypes.CircularStringM = Qgis.WkbType.CircularStringM
QgsWkbTypes.CircularStringM.is_monkey_patched = True
QgsWkbTypes.CircularStringM.__doc__ = "CircularStringM"
QgsWkbTypes.CompoundCurveM = Qgis.WkbType.CompoundCurveM
QgsWkbTypes.CompoundCurveM.is_monkey_patched = True
QgsWkbTypes.CompoundCurveM.__doc__ = "CompoundCurveM"
QgsWkbTypes.CurvePolygonM = Qgis.WkbType.CurvePolygonM
QgsWkbTypes.CurvePolygonM.is_monkey_patched = True
QgsWkbTypes.CurvePolygonM.__doc__ = "CurvePolygonM"
QgsWkbTypes.MultiCurveM = Qgis.WkbType.MultiCurveM
QgsWkbTypes.MultiCurveM.is_monkey_patched = True
QgsWkbTypes.MultiCurveM.__doc__ = "MultiCurveM"
QgsWkbTypes.MultiSurfaceM = Qgis.WkbType.MultiSurfaceM
QgsWkbTypes.MultiSurfaceM.is_monkey_patched = True
QgsWkbTypes.MultiSurfaceM.__doc__ = "MultiSurfaceM"
QgsWkbTypes.PointZM = Qgis.WkbType.PointZM
QgsWkbTypes.PointZM.is_monkey_patched = True
QgsWkbTypes.PointZM.__doc__ = "PointZM"
QgsWkbTypes.LineStringZM = Qgis.WkbType.LineStringZM
QgsWkbTypes.LineStringZM.is_monkey_patched = True
QgsWkbTypes.LineStringZM.__doc__ = "LineStringZM"
QgsWkbTypes.PolygonZM = Qgis.WkbType.PolygonZM
QgsWkbTypes.PolygonZM.is_monkey_patched = True
QgsWkbTypes.PolygonZM.__doc__ = "PolygonZM"
QgsWkbTypes.MultiPointZM = Qgis.WkbType.MultiPointZM
QgsWkbTypes.MultiPointZM.is_monkey_patched = True
QgsWkbTypes.MultiPointZM.__doc__ = "MultiPointZM"
QgsWkbTypes.MultiLineStringZM = Qgis.WkbType.MultiLineStringZM
QgsWkbTypes.MultiLineStringZM.is_monkey_patched = True
QgsWkbTypes.MultiLineStringZM.__doc__ = "MultiLineStringZM"
QgsWkbTypes.MultiPolygonZM = Qgis.WkbType.MultiPolygonZM
QgsWkbTypes.MultiPolygonZM.is_monkey_patched = True
QgsWkbTypes.MultiPolygonZM.__doc__ = "MultiPolygonZM"
QgsWkbTypes.GeometryCollectionZM = Qgis.WkbType.GeometryCollectionZM
QgsWkbTypes.GeometryCollectionZM.is_monkey_patched = True
QgsWkbTypes.GeometryCollectionZM.__doc__ = "GeometryCollectionZM"
QgsWkbTypes.CircularStringZM = Qgis.WkbType.CircularStringZM
QgsWkbTypes.CircularStringZM.is_monkey_patched = True
QgsWkbTypes.CircularStringZM.__doc__ = "CircularStringZM"
QgsWkbTypes.CompoundCurveZM = Qgis.WkbType.CompoundCurveZM
QgsWkbTypes.CompoundCurveZM.is_monkey_patched = True
QgsWkbTypes.CompoundCurveZM.__doc__ = "CompoundCurveZM"
QgsWkbTypes.CurvePolygonZM = Qgis.WkbType.CurvePolygonZM
QgsWkbTypes.CurvePolygonZM.is_monkey_patched = True
QgsWkbTypes.CurvePolygonZM.__doc__ = "CurvePolygonZM"
QgsWkbTypes.MultiCurveZM = Qgis.WkbType.MultiCurveZM
QgsWkbTypes.MultiCurveZM.is_monkey_patched = True
QgsWkbTypes.MultiCurveZM.__doc__ = "MultiCurveZM"
QgsWkbTypes.MultiSurfaceZM = Qgis.WkbType.MultiSurfaceZM
QgsWkbTypes.MultiSurfaceZM.is_monkey_patched = True
QgsWkbTypes.MultiSurfaceZM.__doc__ = "MultiSurfaceZM"
QgsWkbTypes.TriangleZM = Qgis.WkbType.TriangleZM
QgsWkbTypes.TriangleZM.is_monkey_patched = True
QgsWkbTypes.TriangleZM.__doc__ = "TriangleZM"
QgsWkbTypes.Point25D = Qgis.WkbType.Point25D
QgsWkbTypes.Point25D.is_monkey_patched = True
QgsWkbTypes.Point25D.__doc__ = "Point25D"
QgsWkbTypes.LineString25D = Qgis.WkbType.LineString25D
QgsWkbTypes.LineString25D.is_monkey_patched = True
QgsWkbTypes.LineString25D.__doc__ = "LineString25D"
QgsWkbTypes.Polygon25D = Qgis.WkbType.Polygon25D
QgsWkbTypes.Polygon25D.is_monkey_patched = True
QgsWkbTypes.Polygon25D.__doc__ = "Polygon25D"
QgsWkbTypes.MultiPoint25D = Qgis.WkbType.MultiPoint25D
QgsWkbTypes.MultiPoint25D.is_monkey_patched = True
QgsWkbTypes.MultiPoint25D.__doc__ = "MultiPoint25D"
QgsWkbTypes.MultiLineString25D = Qgis.WkbType.MultiLineString25D
QgsWkbTypes.MultiLineString25D.is_monkey_patched = True
QgsWkbTypes.MultiLineString25D.__doc__ = "MultiLineString25D"
QgsWkbTypes.MultiPolygon25D = Qgis.WkbType.MultiPolygon25D
QgsWkbTypes.MultiPolygon25D.is_monkey_patched = True
QgsWkbTypes.MultiPolygon25D.__doc__ = "MultiPolygon25D"
Qgis.WkbType.__doc__ = "The WKB type describes the number of dimensions a geometry has\n\n- Point\n- LineString\n- Polygon\n\nas well as the number of dimensions for each individual vertex\n\n- X (always)\n- Y (always)\n- Z (optional)\n- M (measurement value, optional)\n\nit also has values for multi types, collections, unknown geometry,\nnull geometry, no geometry and curve support.\n\nThese classes of geometry are often used for data sources to\ncommunicate what kind of geometry should be expected for a given\ngeometry field. It is also used for tools or algorithms to decide\nif they should be available for a given geometry type or act in\na different mode.\n\n.. note::\n\n   Prior to 3.30 this was available as :py:class:`QgsWkbTypes`.Type.\n\n.. versionadded:: 3.30\n\n" + '* ``Unknown``: ' + Qgis.WkbType.Unknown.__doc__ + '\n' + '* ``Point``: ' + Qgis.WkbType.Point.__doc__ + '\n' + '* ``LineString``: ' + Qgis.WkbType.LineString.__doc__ + '\n' + '* ``Polygon``: ' + Qgis.WkbType.Polygon.__doc__ + '\n' + '* ``Triangle``: ' + Qgis.WkbType.Triangle.__doc__ + '\n' + '* ``MultiPoint``: ' + Qgis.WkbType.MultiPoint.__doc__ + '\n' + '* ``MultiLineString``: ' + Qgis.WkbType.MultiLineString.__doc__ + '\n' + '* ``MultiPolygon``: ' + Qgis.WkbType.MultiPolygon.__doc__ + '\n' + '* ``GeometryCollection``: ' + Qgis.WkbType.GeometryCollection.__doc__ + '\n' + '* ``CircularString``: ' + Qgis.WkbType.CircularString.__doc__ + '\n' + '* ``CompoundCurve``: ' + Qgis.WkbType.CompoundCurve.__doc__ + '\n' + '* ``CurvePolygon``: ' + Qgis.WkbType.CurvePolygon.__doc__ + '\n' + '* ``MultiCurve``: ' + Qgis.WkbType.MultiCurve.__doc__ + '\n' + '* ``MultiSurface``: ' + Qgis.WkbType.MultiSurface.__doc__ + '\n' + '* ``NoGeometry``: ' + Qgis.WkbType.NoGeometry.__doc__ + '\n' + '* ``PointZ``: ' + Qgis.WkbType.PointZ.__doc__ + '\n' + '* ``LineStringZ``: ' + Qgis.WkbType.LineStringZ.__doc__ + '\n' + '* ``PolygonZ``: ' + Qgis.WkbType.PolygonZ.__doc__ + '\n' + '* ``TriangleZ``: ' + Qgis.WkbType.TriangleZ.__doc__ + '\n' + '* ``MultiPointZ``: ' + Qgis.WkbType.MultiPointZ.__doc__ + '\n' + '* ``MultiLineStringZ``: ' + Qgis.WkbType.MultiLineStringZ.__doc__ + '\n' + '* ``MultiPolygonZ``: ' + Qgis.WkbType.MultiPolygonZ.__doc__ + '\n' + '* ``GeometryCollectionZ``: ' + Qgis.WkbType.GeometryCollectionZ.__doc__ + '\n' + '* ``CircularStringZ``: ' + Qgis.WkbType.CircularStringZ.__doc__ + '\n' + '* ``CompoundCurveZ``: ' + Qgis.WkbType.CompoundCurveZ.__doc__ + '\n' + '* ``CurvePolygonZ``: ' + Qgis.WkbType.CurvePolygonZ.__doc__ + '\n' + '* ``MultiCurveZ``: ' + Qgis.WkbType.MultiCurveZ.__doc__ + '\n' + '* ``MultiSurfaceZ``: ' + Qgis.WkbType.MultiSurfaceZ.__doc__ + '\n' + '* ``PointM``: ' + Qgis.WkbType.PointM.__doc__ + '\n' + '* ``LineStringM``: ' + Qgis.WkbType.LineStringM.__doc__ + '\n' + '* ``PolygonM``: ' + Qgis.WkbType.PolygonM.__doc__ + '\n' + '* ``TriangleM``: ' + Qgis.WkbType.TriangleM.__doc__ + '\n' + '* ``MultiPointM``: ' + Qgis.WkbType.MultiPointM.__doc__ + '\n' + '* ``MultiLineStringM``: ' + Qgis.WkbType.MultiLineStringM.__doc__ + '\n' + '* ``MultiPolygonM``: ' + Qgis.WkbType.MultiPolygonM.__doc__ + '\n' + '* ``GeometryCollectionM``: ' + Qgis.WkbType.GeometryCollectionM.__doc__ + '\n' + '* ``CircularStringM``: ' + Qgis.WkbType.CircularStringM.__doc__ + '\n' + '* ``CompoundCurveM``: ' + Qgis.WkbType.CompoundCurveM.__doc__ + '\n' + '* ``CurvePolygonM``: ' + Qgis.WkbType.CurvePolygonM.__doc__ + '\n' + '* ``MultiCurveM``: ' + Qgis.WkbType.MultiCurveM.__doc__ + '\n' + '* ``MultiSurfaceM``: ' + Qgis.WkbType.MultiSurfaceM.__doc__ + '\n' + '* ``PointZM``: ' + Qgis.WkbType.PointZM.__doc__ + '\n' + '* ``LineStringZM``: ' + Qgis.WkbType.LineStringZM.__doc__ + '\n' + '* ``PolygonZM``: ' + Qgis.WkbType.PolygonZM.__doc__ + '\n' + '* ``MultiPointZM``: ' + Qgis.WkbType.MultiPointZM.__doc__ + '\n' + '* ``MultiLineStringZM``: ' + Qgis.WkbType.MultiLineStringZM.__doc__ + '\n' + '* ``MultiPolygonZM``: ' + Qgis.WkbType.MultiPolygonZM.__doc__ + '\n' + '* ``GeometryCollectionZM``: ' + Qgis.WkbType.GeometryCollectionZM.__doc__ + '\n' + '* ``CircularStringZM``: ' + Qgis.WkbType.CircularStringZM.__doc__ + '\n' + '* ``CompoundCurveZM``: ' + Qgis.WkbType.CompoundCurveZM.__doc__ + '\n' + '* ``CurvePolygonZM``: ' + Qgis.WkbType.CurvePolygonZM.__doc__ + '\n' + '* ``MultiCurveZM``: ' + Qgis.WkbType.MultiCurveZM.__doc__ + '\n' + '* ``MultiSurfaceZM``: ' + Qgis.WkbType.MultiSurfaceZM.__doc__ + '\n' + '* ``TriangleZM``: ' + Qgis.WkbType.TriangleZM.__doc__ + '\n' + '* ``Point25D``: ' + Qgis.WkbType.Point25D.__doc__ + '\n' + '* ``LineString25D``: ' + Qgis.WkbType.LineString25D.__doc__ + '\n' + '* ``Polygon25D``: ' + Qgis.WkbType.Polygon25D.__doc__ + '\n' + '* ``MultiPoint25D``: ' + Qgis.WkbType.MultiPoint25D.__doc__ + '\n' + '* ``MultiLineString25D``: ' + Qgis.WkbType.MultiLineString25D.__doc__ + '\n' + '* ``MultiPolygon25D``: ' + Qgis.WkbType.MultiPolygon25D.__doc__
# --
Qgis.WkbType.baseClass = Qgis
QgsWkbTypes.GeometryType = Qgis.GeometryType
# monkey patching scoped based enum
QgsWkbTypes.PointGeometry = Qgis.GeometryType.Point
QgsWkbTypes.GeometryType.PointGeometry = Qgis.GeometryType.Point
QgsWkbTypes.PointGeometry.is_monkey_patched = True
QgsWkbTypes.PointGeometry.__doc__ = "Points"
QgsWkbTypes.LineGeometry = Qgis.GeometryType.Line
QgsWkbTypes.GeometryType.LineGeometry = Qgis.GeometryType.Line
QgsWkbTypes.LineGeometry.is_monkey_patched = True
QgsWkbTypes.LineGeometry.__doc__ = "Lines"
QgsWkbTypes.PolygonGeometry = Qgis.GeometryType.Polygon
QgsWkbTypes.GeometryType.PolygonGeometry = Qgis.GeometryType.Polygon
QgsWkbTypes.PolygonGeometry.is_monkey_patched = True
QgsWkbTypes.PolygonGeometry.__doc__ = "Polygons"
QgsWkbTypes.UnknownGeometry = Qgis.GeometryType.Unknown
QgsWkbTypes.GeometryType.UnknownGeometry = Qgis.GeometryType.Unknown
QgsWkbTypes.UnknownGeometry.is_monkey_patched = True
QgsWkbTypes.UnknownGeometry.__doc__ = "Unknown types"
QgsWkbTypes.NullGeometry = Qgis.GeometryType.Null
QgsWkbTypes.GeometryType.NullGeometry = Qgis.GeometryType.Null
QgsWkbTypes.NullGeometry.is_monkey_patched = True
QgsWkbTypes.NullGeometry.__doc__ = "No geometry"
Qgis.GeometryType.__doc__ = "The geometry types are used to group Qgis.WkbType in a\ncoarse way.\n\n.. note::\n\n   Prior to 3.30 this was available as :py:class:`QgsWkbTypes`.GeometryType.\n\n.. versionadded:: 3.30\n\n" + '* ``PointGeometry``: ' + Qgis.GeometryType.Point.__doc__ + '\n' + '* ``LineGeometry``: ' + Qgis.GeometryType.Line.__doc__ + '\n' + '* ``PolygonGeometry``: ' + Qgis.GeometryType.Polygon.__doc__ + '\n' + '* ``UnknownGeometry``: ' + Qgis.GeometryType.Unknown.__doc__ + '\n' + '* ``NullGeometry``: ' + Qgis.GeometryType.Null.__doc__
# --
Qgis.GeometryType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.UnknownDataType = Qgis.DataType.UnknownDataType
Qgis.UnknownDataType.is_monkey_patched = True
Qgis.UnknownDataType.__doc__ = "Unknown or unspecified type"
Qgis.Byte = Qgis.DataType.Byte
Qgis.Byte.is_monkey_patched = True
Qgis.Byte.__doc__ = "Eight bit unsigned integer (quint8)"
Qgis.Int8 = Qgis.DataType.Int8
Qgis.Int8.is_monkey_patched = True
Qgis.Int8.__doc__ = "Eight bit signed integer (qint8) (added in QGIS 3.30)"
Qgis.UInt16 = Qgis.DataType.UInt16
Qgis.UInt16.is_monkey_patched = True
Qgis.UInt16.__doc__ = "Sixteen bit unsigned integer (quint16)"
Qgis.Int16 = Qgis.DataType.Int16
Qgis.Int16.is_monkey_patched = True
Qgis.Int16.__doc__ = "Sixteen bit signed integer (qint16)"
Qgis.UInt32 = Qgis.DataType.UInt32
Qgis.UInt32.is_monkey_patched = True
Qgis.UInt32.__doc__ = "Thirty two bit unsigned integer (quint32)"
Qgis.Int32 = Qgis.DataType.Int32
Qgis.Int32.is_monkey_patched = True
Qgis.Int32.__doc__ = "Thirty two bit signed integer (qint32)"
Qgis.Float32 = Qgis.DataType.Float32
Qgis.Float32.is_monkey_patched = True
Qgis.Float32.__doc__ = "Thirty two bit floating point (float)"
Qgis.Float64 = Qgis.DataType.Float64
Qgis.Float64.is_monkey_patched = True
Qgis.Float64.__doc__ = "Sixty four bit floating point (double)"
Qgis.CInt16 = Qgis.DataType.CInt16
Qgis.CInt16.is_monkey_patched = True
Qgis.CInt16.__doc__ = "Complex Int16"
Qgis.CInt32 = Qgis.DataType.CInt32
Qgis.CInt32.is_monkey_patched = True
Qgis.CInt32.__doc__ = "Complex Int32"
Qgis.CFloat32 = Qgis.DataType.CFloat32
Qgis.CFloat32.is_monkey_patched = True
Qgis.CFloat32.__doc__ = "Complex Float32"
Qgis.CFloat64 = Qgis.DataType.CFloat64
Qgis.CFloat64.is_monkey_patched = True
Qgis.CFloat64.__doc__ = "Complex Float64"
Qgis.ARGB32 = Qgis.DataType.ARGB32
Qgis.ARGB32.is_monkey_patched = True
Qgis.ARGB32.__doc__ = "Color, alpha, red, green, blue, 4 bytes the same as QImage.Format_ARGB32"
Qgis.ARGB32_Premultiplied = Qgis.DataType.ARGB32_Premultiplied
Qgis.ARGB32_Premultiplied.is_monkey_patched = True
Qgis.ARGB32_Premultiplied.__doc__ = "Color, alpha, red, green, blue, 4 bytes  the same as QImage.Format_ARGB32_Premultiplied"
Qgis.DataType.__doc__ = "Raster data types.\nThis is modified and extended copy of GDALDataType.\n\n" + '* ``UnknownDataType``: ' + Qgis.DataType.UnknownDataType.__doc__ + '\n' + '* ``Byte``: ' + Qgis.DataType.Byte.__doc__ + '\n' + '* ``Int8``: ' + Qgis.DataType.Int8.__doc__ + '\n' + '* ``UInt16``: ' + Qgis.DataType.UInt16.__doc__ + '\n' + '* ``Int16``: ' + Qgis.DataType.Int16.__doc__ + '\n' + '* ``UInt32``: ' + Qgis.DataType.UInt32.__doc__ + '\n' + '* ``Int32``: ' + Qgis.DataType.Int32.__doc__ + '\n' + '* ``Float32``: ' + Qgis.DataType.Float32.__doc__ + '\n' + '* ``Float64``: ' + Qgis.DataType.Float64.__doc__ + '\n' + '* ``CInt16``: ' + Qgis.DataType.CInt16.__doc__ + '\n' + '* ``CInt32``: ' + Qgis.DataType.CInt32.__doc__ + '\n' + '* ``CFloat32``: ' + Qgis.DataType.CFloat32.__doc__ + '\n' + '* ``CFloat64``: ' + Qgis.DataType.CFloat64.__doc__ + '\n' + '* ``ARGB32``: ' + Qgis.DataType.ARGB32.__doc__ + '\n' + '* ``ARGB32_Premultiplied``: ' + Qgis.DataType.ARGB32_Premultiplied.__doc__
# --
Qgis.DataType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CaptureTechnique.StraightSegments.__doc__ = "Default capture mode - capture occurs with straight line segments"
Qgis.CaptureTechnique.CircularString.__doc__ = "Capture in circular strings"
Qgis.CaptureTechnique.Streaming.__doc__ = "Streaming points digitizing mode (points are automatically added as the mouse cursor moves)."
Qgis.CaptureTechnique.Shape.__doc__ = "Digitize shapes."
Qgis.CaptureTechnique.__doc__ = "Capture technique.\n\n.. versionadded:: 3.26\n\n" + '* ``StraightSegments``: ' + Qgis.CaptureTechnique.StraightSegments.__doc__ + '\n' + '* ``CircularString``: ' + Qgis.CaptureTechnique.CircularString.__doc__ + '\n' + '* ``Streaming``: ' + Qgis.CaptureTechnique.Streaming.__doc__ + '\n' + '* ``Shape``: ' + Qgis.CaptureTechnique.Shape.__doc__
# --
Qgis.CaptureTechnique.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorLayerTypeFlag.SqlQuery.__doc__ = "SQL query layer"
Qgis.VectorLayerTypeFlag.__doc__ = "Vector layer type flags.\n\n.. versionadded:: 3.24\n\n" + '* ``SqlQuery``: ' + Qgis.VectorLayerTypeFlag.SqlQuery.__doc__
# --
Qgis.VectorLayerTypeFlag.baseClass = Qgis
Qgis.VectorLayerTypeFlags = lambda flags=0: Qgis.VectorLayerTypeFlag(flags)
Qgis.VectorLayerTypeFlags.baseClass = Qgis
VectorLayerTypeFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.Never = Qgis.PythonMacroMode.Never
Qgis.Never.is_monkey_patched = True
Qgis.Never.__doc__ = "Macros are never run"
Qgis.Ask = Qgis.PythonMacroMode.Ask
Qgis.Ask.is_monkey_patched = True
Qgis.Ask.__doc__ = "User is prompt before running"
Qgis.SessionOnly = Qgis.PythonMacroMode.SessionOnly
Qgis.SessionOnly.is_monkey_patched = True
Qgis.SessionOnly.__doc__ = "Only during this session"
Qgis.Always = Qgis.PythonMacroMode.Always
Qgis.Always.is_monkey_patched = True
Qgis.Always.__doc__ = "Macros are always run"
Qgis.NotForThisSession = Qgis.PythonMacroMode.NotForThisSession
Qgis.NotForThisSession.is_monkey_patched = True
Qgis.NotForThisSession.__doc__ = "Macros will not be run for this session"
Qgis.PythonMacroMode.__doc__ = "Authorisation to run Python Macros\n\n.. versionadded:: 3.10\n\n" + '* ``Never``: ' + Qgis.PythonMacroMode.Never.__doc__ + '\n' + '* ``Ask``: ' + Qgis.PythonMacroMode.Ask.__doc__ + '\n' + '* ``SessionOnly``: ' + Qgis.PythonMacroMode.SessionOnly.__doc__ + '\n' + '* ``Always``: ' + Qgis.PythonMacroMode.Always.__doc__ + '\n' + '* ``NotForThisSession``: ' + Qgis.PythonMacroMode.NotForThisSession.__doc__
# --
Qgis.PythonMacroMode.baseClass = Qgis
QgsVectorDataProvider.FeatureCountState = Qgis.FeatureCountState
# monkey patching scoped based enum
QgsVectorDataProvider.Uncounted = Qgis.FeatureCountState.Uncounted
QgsVectorDataProvider.Uncounted.is_monkey_patched = True
QgsVectorDataProvider.Uncounted.__doc__ = "Feature count not yet computed"
QgsVectorDataProvider.UnknownCount = Qgis.FeatureCountState.UnknownCount
QgsVectorDataProvider.UnknownCount.is_monkey_patched = True
QgsVectorDataProvider.UnknownCount.__doc__ = "Provider returned an unknown feature count"
Qgis.FeatureCountState.__doc__ = "Enumeration of feature count states\n\n.. versionadded:: 3.20\n\n" + '* ``Uncounted``: ' + Qgis.FeatureCountState.Uncounted.__doc__ + '\n' + '* ``UnknownCount``: ' + Qgis.FeatureCountState.UnknownCount.__doc__
# --
Qgis.FeatureCountState.baseClass = Qgis
QgsFeatureSource.SpatialIndexPresence = Qgis.SpatialIndexPresence
# monkey patching scoped based enum
QgsFeatureSource.SpatialIndexUnknown = Qgis.SpatialIndexPresence.Unknown
QgsFeatureSource.SpatialIndexPresence.SpatialIndexUnknown = Qgis.SpatialIndexPresence.Unknown
QgsFeatureSource.SpatialIndexUnknown.is_monkey_patched = True
QgsFeatureSource.SpatialIndexUnknown.__doc__ = "Spatial index presence cannot be determined, index may or may not exist"
QgsFeatureSource.SpatialIndexNotPresent = Qgis.SpatialIndexPresence.NotPresent
QgsFeatureSource.SpatialIndexPresence.SpatialIndexNotPresent = Qgis.SpatialIndexPresence.NotPresent
QgsFeatureSource.SpatialIndexNotPresent.is_monkey_patched = True
QgsFeatureSource.SpatialIndexNotPresent.__doc__ = "No spatial index exists for the source"
QgsFeatureSource.SpatialIndexPresent = Qgis.SpatialIndexPresence.Present
QgsFeatureSource.SpatialIndexPresence.SpatialIndexPresent = Qgis.SpatialIndexPresence.Present
QgsFeatureSource.SpatialIndexPresent.is_monkey_patched = True
QgsFeatureSource.SpatialIndexPresent.__doc__ = "A valid spatial index exists for the source"
Qgis.SpatialIndexPresence.__doc__ = "Enumeration of spatial index presence states.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureSource`.SpatialIndexPresence\n\n.. versionadded:: 3.36\n\n" + '* ``SpatialIndexUnknown``: ' + Qgis.SpatialIndexPresence.Unknown.__doc__ + '\n' + '* ``SpatialIndexNotPresent``: ' + Qgis.SpatialIndexPresence.NotPresent.__doc__ + '\n' + '* ``SpatialIndexPresent``: ' + Qgis.SpatialIndexPresence.Present.__doc__
# --
Qgis.SpatialIndexPresence.baseClass = Qgis
QgsFeatureSource.FeatureAvailability = Qgis.FeatureAvailability
# monkey patching scoped based enum
QgsFeatureSource.NoFeaturesAvailable = Qgis.FeatureAvailability.NoFeaturesAvailable
QgsFeatureSource.NoFeaturesAvailable.is_monkey_patched = True
QgsFeatureSource.NoFeaturesAvailable.__doc__ = "There are certainly no features available in this source"
QgsFeatureSource.FeaturesAvailable = Qgis.FeatureAvailability.FeaturesAvailable
QgsFeatureSource.FeaturesAvailable.is_monkey_patched = True
QgsFeatureSource.FeaturesAvailable.__doc__ = "There is at least one feature available in this source"
QgsFeatureSource.FeaturesMaybeAvailable = Qgis.FeatureAvailability.FeaturesMaybeAvailable
QgsFeatureSource.FeaturesMaybeAvailable.is_monkey_patched = True
QgsFeatureSource.FeaturesMaybeAvailable.__doc__ = "There may be features available in this source"
Qgis.FeatureAvailability.__doc__ = "Possible return value for :py:func:`QgsFeatureSource.hasFeatures()` to determine if a source is empty.\n\nIt is implemented as a three-value logic, so it can return if\nthere are features available for sure, if there are no features\navailable for sure or if there might be features available but\nthere is no guarantee for this.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureSource`.FeatureAvailability\n\n.. versionadded:: 3.36\n\n" + '* ``NoFeaturesAvailable``: ' + Qgis.FeatureAvailability.NoFeaturesAvailable.__doc__ + '\n' + '* ``FeaturesAvailable``: ' + Qgis.FeatureAvailability.FeaturesAvailable.__doc__ + '\n' + '* ``FeaturesMaybeAvailable``: ' + Qgis.FeatureAvailability.FeaturesMaybeAvailable.__doc__
# --
Qgis.FeatureAvailability.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorDataProviderAttributeEditCapability.EditAlias.__doc__ = "Allows editing aliases"
Qgis.VectorDataProviderAttributeEditCapability.EditComment.__doc__ = "Allows editing comments"
Qgis.VectorDataProviderAttributeEditCapability.__doc__ = "Attribute editing capabilities which may be supported by vector data providers.\n\n.. versionadded:: 3.32\n\n" + '* ``EditAlias``: ' + Qgis.VectorDataProviderAttributeEditCapability.EditAlias.__doc__ + '\n' + '* ``EditComment``: ' + Qgis.VectorDataProviderAttributeEditCapability.EditComment.__doc__
# --
Qgis.VectorDataProviderAttributeEditCapability.baseClass = Qgis
Qgis.VectorDataProviderAttributeEditCapabilities = lambda flags=0: Qgis.VectorDataProviderAttributeEditCapability(flags)
Qgis.VectorDataProviderAttributeEditCapabilities.baseClass = Qgis
VectorDataProviderAttributeEditCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsSymbol.SymbolType = Qgis.SymbolType
# monkey patching scoped based enum
QgsSymbol.Marker = Qgis.SymbolType.Marker
QgsSymbol.Marker.is_monkey_patched = True
QgsSymbol.Marker.__doc__ = "Marker symbol"
QgsSymbol.Line = Qgis.SymbolType.Line
QgsSymbol.Line.is_monkey_patched = True
QgsSymbol.Line.__doc__ = "Line symbol"
QgsSymbol.Fill = Qgis.SymbolType.Fill
QgsSymbol.Fill.is_monkey_patched = True
QgsSymbol.Fill.__doc__ = "Fill symbol"
QgsSymbol.Hybrid = Qgis.SymbolType.Hybrid
QgsSymbol.Hybrid.is_monkey_patched = True
QgsSymbol.Hybrid.__doc__ = "Hybrid symbol"
Qgis.SymbolType.__doc__ = "Symbol types\n\n.. versionadded:: 3.20\n\n" + '* ``Marker``: ' + Qgis.SymbolType.Marker.__doc__ + '\n' + '* ``Line``: ' + Qgis.SymbolType.Line.__doc__ + '\n' + '* ``Fill``: ' + Qgis.SymbolType.Fill.__doc__ + '\n' + '* ``Hybrid``: ' + Qgis.SymbolType.Hybrid.__doc__
# --
Qgis.SymbolType.baseClass = Qgis
QgsSymbol.ScaleMethod = Qgis.ScaleMethod
# monkey patching scoped based enum
QgsSymbol.ScaleArea = Qgis.ScaleMethod.ScaleArea
QgsSymbol.ScaleArea.is_monkey_patched = True
QgsSymbol.ScaleArea.__doc__ = "Calculate scale by the area"
QgsSymbol.ScaleDiameter = Qgis.ScaleMethod.ScaleDiameter
QgsSymbol.ScaleDiameter.is_monkey_patched = True
QgsSymbol.ScaleDiameter.__doc__ = "Calculate scale by the diameter"
Qgis.ScaleMethod.__doc__ = "Scale methods\n\n.. versionadded:: 3.20\n\n" + '* ``ScaleArea``: ' + Qgis.ScaleMethod.ScaleArea.__doc__ + '\n' + '* ``ScaleDiameter``: ' + Qgis.ScaleMethod.ScaleDiameter.__doc__
# --
Qgis.ScaleMethod.baseClass = Qgis
QgsSettingsEntryBase.SettingsType = Qgis.SettingsType
# monkey patching scoped based enum
QgsSettingsEntryBase.Custom = Qgis.SettingsType.Custom
QgsSettingsEntryBase.Custom.is_monkey_patched = True
QgsSettingsEntryBase.Custom.__doc__ = "Custom implementation"
QgsSettingsEntryBase.Variant = Qgis.SettingsType.Variant
QgsSettingsEntryBase.Variant.is_monkey_patched = True
QgsSettingsEntryBase.Variant.__doc__ = "Generic variant"
QgsSettingsEntryBase.String = Qgis.SettingsType.String
QgsSettingsEntryBase.String.is_monkey_patched = True
QgsSettingsEntryBase.String.__doc__ = "String"
QgsSettingsEntryBase.StringList = Qgis.SettingsType.StringList
QgsSettingsEntryBase.StringList.is_monkey_patched = True
QgsSettingsEntryBase.StringList.__doc__ = "List of strings"
QgsSettingsEntryBase.VariantMap = Qgis.SettingsType.VariantMap
QgsSettingsEntryBase.VariantMap.is_monkey_patched = True
QgsSettingsEntryBase.VariantMap.__doc__ = "Map of strings"
QgsSettingsEntryBase.Bool = Qgis.SettingsType.Bool
QgsSettingsEntryBase.Bool.is_monkey_patched = True
QgsSettingsEntryBase.Bool.__doc__ = "Boolean"
QgsSettingsEntryBase.Integer = Qgis.SettingsType.Integer
QgsSettingsEntryBase.Integer.is_monkey_patched = True
QgsSettingsEntryBase.Integer.__doc__ = "Integer"
QgsSettingsEntryBase.Double = Qgis.SettingsType.Double
QgsSettingsEntryBase.Double.is_monkey_patched = True
QgsSettingsEntryBase.Double.__doc__ = "Double precision number"
QgsSettingsEntryBase.EnumFlag = Qgis.SettingsType.EnumFlag
QgsSettingsEntryBase.EnumFlag.is_monkey_patched = True
QgsSettingsEntryBase.EnumFlag.__doc__ = "Enum or Flag"
QgsSettingsEntryBase.Color = Qgis.SettingsType.Color
QgsSettingsEntryBase.Color.is_monkey_patched = True
QgsSettingsEntryBase.Color.__doc__ = "Color"
Qgis.SettingsType.__doc__ = "Types of settings entries\n\n.. versionadded:: 3.26\n\n" + '* ``Custom``: ' + Qgis.SettingsType.Custom.__doc__ + '\n' + '* ``Variant``: ' + Qgis.SettingsType.Variant.__doc__ + '\n' + '* ``String``: ' + Qgis.SettingsType.String.__doc__ + '\n' + '* ``StringList``: ' + Qgis.SettingsType.StringList.__doc__ + '\n' + '* ``VariantMap``: ' + Qgis.SettingsType.VariantMap.__doc__ + '\n' + '* ``Bool``: ' + Qgis.SettingsType.Bool.__doc__ + '\n' + '* ``Integer``: ' + Qgis.SettingsType.Integer.__doc__ + '\n' + '* ``Double``: ' + Qgis.SettingsType.Double.__doc__ + '\n' + '* ``EnumFlag``: ' + Qgis.SettingsType.EnumFlag.__doc__ + '\n' + '* ``Color``: ' + Qgis.SettingsType.Color.__doc__
# --
Qgis.SettingsType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SettingsTreeNodeType.Root.__doc__ = "Root Node"
Qgis.SettingsTreeNodeType.Standard.__doc__ = "Normal Node"
Qgis.SettingsTreeNodeType.NamedList.__doc__ = ""
Qgis.SettingsTreeNodeType.__doc__ = "Type of tree node\n\n.. versionadded:: 3.30\n\n" + '* ``Root``: ' + Qgis.SettingsTreeNodeType.Root.__doc__ + '\n' + '* ``Standard``: ' + Qgis.SettingsTreeNodeType.Standard.__doc__ + '\n' + '* ``NamedList``: ' + Qgis.SettingsTreeNodeType.NamedList.__doc__
# --
Qgis.SettingsTreeNodeType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SettingsTreeNodeOption.NamedListSelectedItemSetting.__doc__ = "Creates a setting to store which is the current item"
Qgis.SettingsTreeNodeOption.__doc__ = "Options for named list nodes\n\n.. versionadded:: 3.30\n\n" + '* ``NamedListSelectedItemSetting``: ' + Qgis.SettingsTreeNodeOption.NamedListSelectedItemSetting.__doc__
# --
Qgis.SettingsTreeNodeOption.baseClass = Qgis
Qgis.SettingsTreeNodeOptions = lambda flags=0: Qgis.SettingsTreeNodeOption(flags)
Qgis.SettingsTreeNodeOptions.baseClass = Qgis
SettingsTreeNodeOptions = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProperty.Type = Qgis.PropertyType
# monkey patching scoped based enum
QgsProperty.InvalidProperty = Qgis.PropertyType.Invalid
QgsProperty.Type.InvalidProperty = Qgis.PropertyType.Invalid
QgsProperty.InvalidProperty.is_monkey_patched = True
QgsProperty.InvalidProperty.__doc__ = "Invalid (not set) property"
QgsProperty.StaticProperty = Qgis.PropertyType.Static
QgsProperty.Type.StaticProperty = Qgis.PropertyType.Static
QgsProperty.StaticProperty.is_monkey_patched = True
QgsProperty.StaticProperty.__doc__ = "Static property"
QgsProperty.FieldBasedProperty = Qgis.PropertyType.Field
QgsProperty.Type.FieldBasedProperty = Qgis.PropertyType.Field
QgsProperty.FieldBasedProperty.is_monkey_patched = True
QgsProperty.FieldBasedProperty.__doc__ = "Field based property"
QgsProperty.ExpressionBasedProperty = Qgis.PropertyType.Expression
QgsProperty.Type.ExpressionBasedProperty = Qgis.PropertyType.Expression
QgsProperty.ExpressionBasedProperty.is_monkey_patched = True
QgsProperty.ExpressionBasedProperty.__doc__ = "Expression based property"
Qgis.PropertyType.__doc__ = "Property types\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProperty`.Type\n\n.. versionadded:: 3.36\n\n" + '* ``InvalidProperty``: ' + Qgis.PropertyType.Invalid.__doc__ + '\n' + '* ``StaticProperty``: ' + Qgis.PropertyType.Static.__doc__ + '\n' + '* ``FieldBasedProperty``: ' + Qgis.PropertyType.Field.__doc__ + '\n' + '* ``ExpressionBasedProperty``: ' + Qgis.PropertyType.Expression.__doc__
# --
Qgis.PropertyType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SldExportOption.NoOptions.__doc__ = "Default SLD export"
Qgis.SldExportOption.Svg.__doc__ = "Export complex styles to separate SVG files for better compatibility with OGC servers"
Qgis.SldExportOption.Png.__doc__ = "Export complex styles to separate PNG files for better compatibility with OGC servers"
Qgis.SldExportOption.__doc__ = "SLD export options\n\n.. versionadded:: 3.30\n\n" + '* ``NoOptions``: ' + Qgis.SldExportOption.NoOptions.__doc__ + '\n' + '* ``Svg``: ' + Qgis.SldExportOption.Svg.__doc__ + '\n' + '* ``Png``: ' + Qgis.SldExportOption.Png.__doc__
# --
Qgis.SldExportOption.baseClass = Qgis
Qgis.SldExportOptions = lambda flags=0: Qgis.SldExportOption(flags)
Qgis.SldExportOptions.baseClass = Qgis
SldExportOptions = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SldExportVendorExtension.NoVendorExtension.__doc__ = "No vendor extensions"
Qgis.SldExportVendorExtension.GeoServerVendorExtension.__doc__ = "Use GeoServer vendor extensions when required"
Qgis.SldExportVendorExtension.DeegreeVendorExtension.__doc__ = "Use Deegree vendor extensions when required"
Qgis.SldExportVendorExtension.__doc__ = "SLD export vendor extensions, allow the use of vendor extensions when exporting to SLD.\n\n.. versionadded:: 3.30\n\n" + '* ``NoVendorExtension``: ' + Qgis.SldExportVendorExtension.NoVendorExtension.__doc__ + '\n' + '* ``GeoServerVendorExtension``: ' + Qgis.SldExportVendorExtension.GeoServerVendorExtension.__doc__ + '\n' + '* ``DeegreeVendorExtension``: ' + Qgis.SldExportVendorExtension.DeegreeVendorExtension.__doc__
# --
Qgis.SldExportVendorExtension.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SettingsOption.SaveFormerValue.__doc__ = ""
Qgis.SettingsOption.SaveEnumFlagAsInt.__doc__ = ""
Qgis.SettingsOption.__doc__ = "Settings options\n\n.. versionadded:: 3.26\n\n" + '* ``SaveFormerValue``: ' + Qgis.SettingsOption.SaveFormerValue.__doc__ + '\n' + '* ``SaveEnumFlagAsInt``: ' + Qgis.SettingsOption.SaveEnumFlagAsInt.__doc__
# --
Qgis.SettingsOption.baseClass = Qgis
Qgis.SettingsOptions = lambda flags=0: Qgis.SettingsOption(flags)
Qgis.SettingsOptions.baseClass = Qgis
SettingsOptions = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsSnappingConfig.SnappingMode = Qgis.SnappingMode
# monkey patching scoped based enum
QgsSnappingConfig.ActiveLayer = Qgis.SnappingMode.ActiveLayer
QgsSnappingConfig.ActiveLayer.is_monkey_patched = True
QgsSnappingConfig.ActiveLayer.__doc__ = "On the active layer"
QgsSnappingConfig.AllLayers = Qgis.SnappingMode.AllLayers
QgsSnappingConfig.AllLayers.is_monkey_patched = True
QgsSnappingConfig.AllLayers.__doc__ = "On all vector layers"
QgsSnappingConfig.AdvancedConfiguration = Qgis.SnappingMode.AdvancedConfiguration
QgsSnappingConfig.AdvancedConfiguration.is_monkey_patched = True
QgsSnappingConfig.AdvancedConfiguration.__doc__ = "On a per layer configuration basis"
Qgis.SnappingMode.__doc__ = "SnappingMode defines on which layer the snapping is performed\n\n.. versionadded:: 3.26\n\n" + '* ``ActiveLayer``: ' + Qgis.SnappingMode.ActiveLayer.__doc__ + '\n' + '* ``AllLayers``: ' + Qgis.SnappingMode.AllLayers.__doc__ + '\n' + '* ``AdvancedConfiguration``: ' + Qgis.SnappingMode.AdvancedConfiguration.__doc__
# --
Qgis.SnappingMode.baseClass = Qgis
QgsSnappingConfig.SnappingTypes = Qgis.SnappingType
# monkey patching scoped based enum
QgsSnappingConfig.NoSnapFlag = Qgis.SnappingType.NoSnap
QgsSnappingConfig.SnappingTypes.NoSnapFlag = Qgis.SnappingType.NoSnap
QgsSnappingConfig.NoSnapFlag.is_monkey_patched = True
QgsSnappingConfig.NoSnapFlag.__doc__ = "No snapping"
QgsSnappingConfig.VertexFlag = Qgis.SnappingType.Vertex
QgsSnappingConfig.SnappingTypes.VertexFlag = Qgis.SnappingType.Vertex
QgsSnappingConfig.VertexFlag.is_monkey_patched = True
QgsSnappingConfig.VertexFlag.__doc__ = "On vertices"
QgsSnappingConfig.SegmentFlag = Qgis.SnappingType.Segment
QgsSnappingConfig.SnappingTypes.SegmentFlag = Qgis.SnappingType.Segment
QgsSnappingConfig.SegmentFlag.is_monkey_patched = True
QgsSnappingConfig.SegmentFlag.__doc__ = "On segments"
QgsSnappingConfig.AreaFlag = Qgis.SnappingType.Area
QgsSnappingConfig.SnappingTypes.AreaFlag = Qgis.SnappingType.Area
QgsSnappingConfig.AreaFlag.is_monkey_patched = True
QgsSnappingConfig.AreaFlag.__doc__ = "On Area"
QgsSnappingConfig.CentroidFlag = Qgis.SnappingType.Centroid
QgsSnappingConfig.SnappingTypes.CentroidFlag = Qgis.SnappingType.Centroid
QgsSnappingConfig.CentroidFlag.is_monkey_patched = True
QgsSnappingConfig.CentroidFlag.__doc__ = "On centroid"
QgsSnappingConfig.MiddleOfSegmentFlag = Qgis.SnappingType.MiddleOfSegment
QgsSnappingConfig.SnappingTypes.MiddleOfSegmentFlag = Qgis.SnappingType.MiddleOfSegment
QgsSnappingConfig.MiddleOfSegmentFlag.is_monkey_patched = True
QgsSnappingConfig.MiddleOfSegmentFlag.__doc__ = "On Middle segment"
QgsSnappingConfig.LineEndpointFlag = Qgis.SnappingType.LineEndpoint
QgsSnappingConfig.SnappingTypes.LineEndpointFlag = Qgis.SnappingType.LineEndpoint
QgsSnappingConfig.LineEndpointFlag.is_monkey_patched = True
QgsSnappingConfig.LineEndpointFlag.__doc__ = "Start or end points of lines, or first vertex in polygon rings only (since QGIS 3.20)"
Qgis.SnappingType.__doc__ = "SnappingTypeFlag defines on what object the snapping is performed\n\n.. versionadded:: 3.26\n\n" + '* ``NoSnapFlag``: ' + Qgis.SnappingType.NoSnap.__doc__ + '\n' + '* ``VertexFlag``: ' + Qgis.SnappingType.Vertex.__doc__ + '\n' + '* ``SegmentFlag``: ' + Qgis.SnappingType.Segment.__doc__ + '\n' + '* ``AreaFlag``: ' + Qgis.SnappingType.Area.__doc__ + '\n' + '* ``CentroidFlag``: ' + Qgis.SnappingType.Centroid.__doc__ + '\n' + '* ``MiddleOfSegmentFlag``: ' + Qgis.SnappingType.MiddleOfSegment.__doc__ + '\n' + '* ``LineEndpointFlag``: ' + Qgis.SnappingType.LineEndpoint.__doc__
# --
Qgis.SnappingType.baseClass = Qgis
Qgis.SnappingTypes = lambda flags=0: Qgis.SnappingType(flags)
QgsSnappingConfig.SnappingTypeFlag = Qgis.SnappingTypes
Qgis.SnappingTypes.baseClass = Qgis
SnappingTypes = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsSymbol.RenderHint = Qgis.SymbolRenderHint
# monkey patching scoped based enum
QgsSymbol.DynamicRotation = Qgis.SymbolRenderHint.DynamicRotation
QgsSymbol.DynamicRotation.is_monkey_patched = True
QgsSymbol.DynamicRotation.__doc__ = "Rotation of symbol may be changed during rendering and symbol should not be cached"
Qgis.SymbolRenderHint.__doc__ = "Flags controlling behavior of symbols during rendering\n\n.. versionadded:: 3.20\n\n" + '* ``DynamicRotation``: ' + Qgis.SymbolRenderHint.DynamicRotation.__doc__
# --
Qgis.SymbolRenderHint.baseClass = Qgis
Qgis.SymbolRenderHints = lambda flags=0: Qgis.SymbolRenderHint(flags)
QgsSymbol.RenderHints = Qgis.SymbolRenderHints
Qgis.SymbolRenderHints.baseClass = Qgis
SymbolRenderHints = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SymbolRotationMode.RespectMapRotation.__doc__ = "Entity is rotated along with the map"
Qgis.SymbolRotationMode.IgnoreMapRotation.__doc__ = "Entity ignores map rotation"
Qgis.SymbolRotationMode.__doc__ = "Modes for handling how symbol and text entity rotation is handled when maps are rotated.\n\n.. versionadded:: 3.32\n\n" + '* ``RespectMapRotation``: ' + Qgis.SymbolRotationMode.RespectMapRotation.__doc__ + '\n' + '* ``IgnoreMapRotation``: ' + Qgis.SymbolRotationMode.IgnoreMapRotation.__doc__
# --
Qgis.SymbolRotationMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SymbolFlag.RendererShouldUseSymbolLevels.__doc__ = "If present, indicates that a QgsFeatureRenderer using the symbol should use symbol levels for best results"
Qgis.SymbolFlag.__doc__ = "Flags controlling behavior of symbols\n\n.. versionadded:: 3.20\n\n" + '* ``RendererShouldUseSymbolLevels``: ' + Qgis.SymbolFlag.RendererShouldUseSymbolLevels.__doc__
# --
Qgis.SymbolFlag.baseClass = Qgis
Qgis.SymbolFlags = lambda flags=0: Qgis.SymbolFlag(flags)
Qgis.SymbolFlags.baseClass = Qgis
SymbolFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsSymbol.PreviewFlag = Qgis.SymbolPreviewFlag
# monkey patching scoped based enum
QgsSymbol.FlagIncludeCrosshairsForMarkerSymbols = Qgis.SymbolPreviewFlag.FlagIncludeCrosshairsForMarkerSymbols
QgsSymbol.FlagIncludeCrosshairsForMarkerSymbols.is_monkey_patched = True
QgsSymbol.FlagIncludeCrosshairsForMarkerSymbols.__doc__ = "Include a crosshairs reference image in the background of marker symbol previews"
Qgis.SymbolPreviewFlag.__doc__ = "Flags for controlling how symbol preview images are generated.\n\n.. versionadded:: 3.20\n\n" + '* ``FlagIncludeCrosshairsForMarkerSymbols``: ' + Qgis.SymbolPreviewFlag.FlagIncludeCrosshairsForMarkerSymbols.__doc__
# --
Qgis.SymbolPreviewFlag.baseClass = Qgis
Qgis.SymbolPreviewFlags = lambda flags=0: Qgis.SymbolPreviewFlag(flags)
QgsSymbol.SymbolPreviewFlags = Qgis.SymbolPreviewFlags
Qgis.SymbolPreviewFlags.baseClass = Qgis
SymbolPreviewFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SymbolLayerFlag.DisableFeatureClipping.__doc__ = "If present, indicates that features should never be clipped to the map extent during rendering"
Qgis.SymbolLayerFlag.__doc__ = "Flags controlling behavior of symbol layers\n\n.. note::\n\n   These differ from Qgis.SymbolLayerUserFlag in that Qgis.SymbolLayerFlag flags are used to reflect the inbuilt properties\n   of a symbol layer type, whereas Qgis.SymbolLayerUserFlag are optional, user controlled flags which can be toggled\n   for a symbol layer.\n\n.. versionadded:: 3.22\n\n" + '* ``DisableFeatureClipping``: ' + Qgis.SymbolLayerFlag.DisableFeatureClipping.__doc__
# --
Qgis.SymbolLayerFlag.baseClass = Qgis
Qgis.SymbolLayerFlags = lambda flags=0: Qgis.SymbolLayerFlag(flags)
Qgis.SymbolLayerFlags.baseClass = Qgis
SymbolLayerFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring.__doc__ = "If present, indicates that the symbol layer should not be recolored when rendering selected features"
Qgis.SymbolLayerUserFlag.__doc__ = "User-specified flags controlling behavior of symbol layers.\n\n.. note::\n\n   These differ from Qgis.SymbolLayerFlag in that Qgis.SymbolLayerFlag flags are used to reflect the inbuilt properties\n   of a symbol layer type, whereas Qgis.SymbolLayerUserFlag are optional, user controlled flags which can be toggled\n   for a symbol layer.\n\n.. versionadded:: 3.34\n\n" + '* ``DisableSelectionRecoloring``: ' + Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring.__doc__
# --
Qgis.SymbolLayerUserFlag.baseClass = Qgis
Qgis.SymbolLayerUserFlags = lambda flags=0: Qgis.SymbolLayerUserFlag(flags)
Qgis.SymbolLayerUserFlags.baseClass = Qgis
SymbolLayerUserFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsDataItem.Type = Qgis.BrowserItemType
# monkey patching scoped based enum
QgsDataItem.Collection = Qgis.BrowserItemType.Collection
QgsDataItem.Collection.is_monkey_patched = True
QgsDataItem.Collection.__doc__ = "A collection of items"
QgsDataItem.Directory = Qgis.BrowserItemType.Directory
QgsDataItem.Directory.is_monkey_patched = True
QgsDataItem.Directory.__doc__ = "Represents a file directory"
QgsDataItem.Layer = Qgis.BrowserItemType.Layer
QgsDataItem.Layer.is_monkey_patched = True
QgsDataItem.Layer.__doc__ = "Represents a map layer"
QgsDataItem.Error = Qgis.BrowserItemType.Error
QgsDataItem.Error.is_monkey_patched = True
QgsDataItem.Error.__doc__ = "Contains an error message"
QgsDataItem.Favorites = Qgis.BrowserItemType.Favorites
QgsDataItem.Favorites.is_monkey_patched = True
QgsDataItem.Favorites.__doc__ = "Represents a favorite item"
QgsDataItem.Project = Qgis.BrowserItemType.Project
QgsDataItem.Project.is_monkey_patched = True
QgsDataItem.Project.__doc__ = "Represents a QGIS project"
QgsDataItem.Custom = Qgis.BrowserItemType.Custom
QgsDataItem.Custom.is_monkey_patched = True
QgsDataItem.Custom.__doc__ = "Custom item type"
QgsDataItem.Fields = Qgis.BrowserItemType.Fields
QgsDataItem.Fields.is_monkey_patched = True
QgsDataItem.Fields.__doc__ = "Collection of fields"
QgsDataItem.Field = Qgis.BrowserItemType.Field
QgsDataItem.Field.is_monkey_patched = True
QgsDataItem.Field.__doc__ = "Vector layer field"
Qgis.BrowserItemType.__doc__ = "Browser item types.\n\n.. versionadded:: 3.20\n\n" + '* ``Collection``: ' + Qgis.BrowserItemType.Collection.__doc__ + '\n' + '* ``Directory``: ' + Qgis.BrowserItemType.Directory.__doc__ + '\n' + '* ``Layer``: ' + Qgis.BrowserItemType.Layer.__doc__ + '\n' + '* ``Error``: ' + Qgis.BrowserItemType.Error.__doc__ + '\n' + '* ``Favorites``: ' + Qgis.BrowserItemType.Favorites.__doc__ + '\n' + '* ``Project``: ' + Qgis.BrowserItemType.Project.__doc__ + '\n' + '* ``Custom``: ' + Qgis.BrowserItemType.Custom.__doc__ + '\n' + '* ``Fields``: ' + Qgis.BrowserItemType.Fields.__doc__ + '\n' + '* ``Field``: ' + Qgis.BrowserItemType.Field.__doc__
# --
Qgis.BrowserItemType.baseClass = Qgis
QgsDataItem.State = Qgis.BrowserItemState
# monkey patching scoped based enum
QgsDataItem.NotPopulated = Qgis.BrowserItemState.NotPopulated
QgsDataItem.NotPopulated.is_monkey_patched = True
QgsDataItem.NotPopulated.__doc__ = "Children not yet created"
QgsDataItem.Populating = Qgis.BrowserItemState.Populating
QgsDataItem.Populating.is_monkey_patched = True
QgsDataItem.Populating.__doc__ = "Creating children in separate thread (populating or refreshing)"
QgsDataItem.Populated = Qgis.BrowserItemState.Populated
QgsDataItem.Populated.is_monkey_patched = True
QgsDataItem.Populated.__doc__ = "Children created"
Qgis.BrowserItemState.__doc__ = "Browser item states.\n\n.. versionadded:: 3.20\n\n" + '* ``NotPopulated``: ' + Qgis.BrowserItemState.NotPopulated.__doc__ + '\n' + '* ``Populating``: ' + Qgis.BrowserItemState.Populating.__doc__ + '\n' + '* ``Populated``: ' + Qgis.BrowserItemState.Populated.__doc__
# --
Qgis.BrowserItemState.baseClass = Qgis
QgsDataItem.Capability = Qgis.BrowserItemCapability
# monkey patching scoped based enum
QgsDataItem.NoCapabilities = Qgis.BrowserItemCapability.NoCapabilities
QgsDataItem.NoCapabilities.is_monkey_patched = True
QgsDataItem.NoCapabilities.__doc__ = "Item has no capabilities"
QgsDataItem.SetCrs = Qgis.BrowserItemCapability.SetCrs
QgsDataItem.SetCrs.is_monkey_patched = True
QgsDataItem.SetCrs.__doc__ = "Can set CRS on layer or group of layers. deprecated since QGIS 3.6 -- no longer used by QGIS and will be removed in QGIS 4.0"
QgsDataItem.Fertile = Qgis.BrowserItemCapability.Fertile
QgsDataItem.Fertile.is_monkey_patched = True
QgsDataItem.Fertile.__doc__ = "Can create children. Even items without this capability may have children, but cannot create them, it means that children are created by item ancestors."
QgsDataItem.Fast = Qgis.BrowserItemCapability.Fast
QgsDataItem.Fast.is_monkey_patched = True
QgsDataItem.Fast.__doc__ = "CreateChildren() is fast enough to be run in main thread when refreshing items, most root items (wms,wfs,wcs,postgres...) are considered fast because they are reading data only from QgsSettings"
QgsDataItem.Collapse = Qgis.BrowserItemCapability.Collapse
QgsDataItem.Collapse.is_monkey_patched = True
QgsDataItem.Collapse.__doc__ = "The collapse/expand status for this items children should be ignored in order to avoid undesired network connections (wms etc.)"
QgsDataItem.Rename = Qgis.BrowserItemCapability.Rename
QgsDataItem.Rename.is_monkey_patched = True
QgsDataItem.Rename.__doc__ = "Item can be renamed"
QgsDataItem.Delete = Qgis.BrowserItemCapability.Delete
QgsDataItem.Delete.is_monkey_patched = True
QgsDataItem.Delete.__doc__ = "Item can be deleted"
QgsDataItem.ItemRepresentsFile = Qgis.BrowserItemCapability.ItemRepresentsFile
QgsDataItem.ItemRepresentsFile.is_monkey_patched = True
QgsDataItem.ItemRepresentsFile.__doc__ = "Item's path() directly represents a file on disk (since QGIS 3.22)"
QgsDataItem.RefreshChildrenWhenItemIsRefreshed = Qgis.BrowserItemCapability.RefreshChildrenWhenItemIsRefreshed
QgsDataItem.RefreshChildrenWhenItemIsRefreshed.is_monkey_patched = True
QgsDataItem.RefreshChildrenWhenItemIsRefreshed.__doc__ = "When the item is refreshed, all its populated children will also be refreshed in turn (since QGIS 3.26)"
Qgis.BrowserItemCapability.__doc__ = "Browser item capabilities.\n\n.. versionadded:: 3.20\n\n" + '* ``NoCapabilities``: ' + Qgis.BrowserItemCapability.NoCapabilities.__doc__ + '\n' + '* ``SetCrs``: ' + Qgis.BrowserItemCapability.SetCrs.__doc__ + '\n' + '* ``Fertile``: ' + Qgis.BrowserItemCapability.Fertile.__doc__ + '\n' + '* ``Fast``: ' + Qgis.BrowserItemCapability.Fast.__doc__ + '\n' + '* ``Collapse``: ' + Qgis.BrowserItemCapability.Collapse.__doc__ + '\n' + '* ``Rename``: ' + Qgis.BrowserItemCapability.Rename.__doc__ + '\n' + '* ``Delete``: ' + Qgis.BrowserItemCapability.Delete.__doc__ + '\n' + '* ``ItemRepresentsFile``: ' + Qgis.BrowserItemCapability.ItemRepresentsFile.__doc__ + '\n' + '* ``RefreshChildrenWhenItemIsRefreshed``: ' + Qgis.BrowserItemCapability.RefreshChildrenWhenItemIsRefreshed.__doc__
# --
Qgis.BrowserItemCapability.baseClass = Qgis
Qgis.BrowserItemCapabilities = lambda flags=0: Qgis.BrowserItemCapability(flags)
QgsDataItem.Capabilities = Qgis.BrowserItemCapabilities
Qgis.BrowserItemCapabilities.baseClass = Qgis
BrowserItemCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsDataProvider.DataCapability = Qgis.DataItemProviderCapability
# monkey patching scoped based enum
QgsDataProvider.NoDataCapabilities = Qgis.DataItemProviderCapability.NoCapabilities
QgsDataProvider.DataCapability.NoDataCapabilities = Qgis.DataItemProviderCapability.NoCapabilities
QgsDataProvider.NoDataCapabilities.is_monkey_patched = True
QgsDataProvider.NoDataCapabilities.__doc__ = "No capabilities"
QgsDataProvider.File = Qgis.DataItemProviderCapability.Files
QgsDataProvider.DataCapability.File = Qgis.DataItemProviderCapability.Files
QgsDataProvider.File.is_monkey_patched = True
QgsDataProvider.File.__doc__ = "Can provides items which corresponds to files"
QgsDataProvider.Dir = Qgis.DataItemProviderCapability.Directories
QgsDataProvider.DataCapability.Dir = Qgis.DataItemProviderCapability.Directories
QgsDataProvider.Dir.is_monkey_patched = True
QgsDataProvider.Dir.__doc__ = "Can provides items which corresponds to directories"
QgsDataProvider.Database = Qgis.DataItemProviderCapability.Databases
QgsDataProvider.DataCapability.Database = Qgis.DataItemProviderCapability.Databases
QgsDataProvider.Database.is_monkey_patched = True
QgsDataProvider.Database.__doc__ = "Can provides items which corresponds to databases"
QgsDataProvider.Net = Qgis.DataItemProviderCapability.NetworkSources
QgsDataProvider.DataCapability.Net = Qgis.DataItemProviderCapability.NetworkSources
QgsDataProvider.Net.is_monkey_patched = True
QgsDataProvider.Net.__doc__ = "Network/internet source"
Qgis.DataItemProviderCapability.__doc__ = "Capabilities for data item providers.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsDataProvider`.DataCapability\n\n.. versionadded:: 3.36\n\n" + '* ``NoDataCapabilities``: ' + Qgis.DataItemProviderCapability.NoCapabilities.__doc__ + '\n' + '* ``File``: ' + Qgis.DataItemProviderCapability.Files.__doc__ + '\n' + '* ``Dir``: ' + Qgis.DataItemProviderCapability.Directories.__doc__ + '\n' + '* ``Database``: ' + Qgis.DataItemProviderCapability.Databases.__doc__ + '\n' + '* ``Net``: ' + Qgis.DataItemProviderCapability.NetworkSources.__doc__
# --
Qgis.DataItemProviderCapability.baseClass = Qgis
Qgis.DataItemProviderCapabilities = lambda flags=0: Qgis.DataItemProviderCapability(flags)
QgsDataProvider.DataCapabilities = Qgis.DataItemProviderCapabilities
Qgis.DataItemProviderCapabilities.baseClass = Qgis
DataItemProviderCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsLayerItem.LayerType = Qgis.BrowserLayerType
# monkey patching scoped based enum
QgsLayerItem.NoType = Qgis.BrowserLayerType.NoType
QgsLayerItem.NoType.is_monkey_patched = True
QgsLayerItem.NoType.__doc__ = "No type"
QgsLayerItem.Vector = Qgis.BrowserLayerType.Vector
QgsLayerItem.Vector.is_monkey_patched = True
QgsLayerItem.Vector.__doc__ = "Generic vector layer"
QgsLayerItem.Raster = Qgis.BrowserLayerType.Raster
QgsLayerItem.Raster.is_monkey_patched = True
QgsLayerItem.Raster.__doc__ = "Raster layer"
QgsLayerItem.Point = Qgis.BrowserLayerType.Point
QgsLayerItem.Point.is_monkey_patched = True
QgsLayerItem.Point.__doc__ = "Vector point layer"
QgsLayerItem.Line = Qgis.BrowserLayerType.Line
QgsLayerItem.Line.is_monkey_patched = True
QgsLayerItem.Line.__doc__ = "Vector line layer"
QgsLayerItem.Polygon = Qgis.BrowserLayerType.Polygon
QgsLayerItem.Polygon.is_monkey_patched = True
QgsLayerItem.Polygon.__doc__ = "Vector polygon layer"
QgsLayerItem.TableLayer = Qgis.BrowserLayerType.TableLayer
QgsLayerItem.TableLayer.is_monkey_patched = True
QgsLayerItem.TableLayer.__doc__ = "Vector non-spatial layer"
QgsLayerItem.Database = Qgis.BrowserLayerType.Database
QgsLayerItem.Database.is_monkey_patched = True
QgsLayerItem.Database.__doc__ = "Database layer"
QgsLayerItem.Table = Qgis.BrowserLayerType.Table
QgsLayerItem.Table.is_monkey_patched = True
QgsLayerItem.Table.__doc__ = "Database table"
QgsLayerItem.Plugin = Qgis.BrowserLayerType.Plugin
QgsLayerItem.Plugin.is_monkey_patched = True
QgsLayerItem.Plugin.__doc__ = "Plugin based layer"
QgsLayerItem.Mesh = Qgis.BrowserLayerType.Mesh
QgsLayerItem.Mesh.is_monkey_patched = True
QgsLayerItem.Mesh.__doc__ = "Mesh layer"
QgsLayerItem.VectorTile = Qgis.BrowserLayerType.VectorTile
QgsLayerItem.VectorTile.is_monkey_patched = True
QgsLayerItem.VectorTile.__doc__ = "Vector tile layer"
QgsLayerItem.PointCloud = Qgis.BrowserLayerType.PointCloud
QgsLayerItem.PointCloud.is_monkey_patched = True
QgsLayerItem.PointCloud.__doc__ = "Point cloud layer"
QgsLayerItem.TiledScene = Qgis.BrowserLayerType.TiledScene
QgsLayerItem.TiledScene.is_monkey_patched = True
QgsLayerItem.TiledScene.__doc__ = "Tiled scene layer (since QGIS 3.34)"
Qgis.BrowserLayerType.__doc__ = "Browser item layer types\n\n.. versionadded:: 3.20\n\n" + '* ``NoType``: ' + Qgis.BrowserLayerType.NoType.__doc__ + '\n' + '* ``Vector``: ' + Qgis.BrowserLayerType.Vector.__doc__ + '\n' + '* ``Raster``: ' + Qgis.BrowserLayerType.Raster.__doc__ + '\n' + '* ``Point``: ' + Qgis.BrowserLayerType.Point.__doc__ + '\n' + '* ``Line``: ' + Qgis.BrowserLayerType.Line.__doc__ + '\n' + '* ``Polygon``: ' + Qgis.BrowserLayerType.Polygon.__doc__ + '\n' + '* ``TableLayer``: ' + Qgis.BrowserLayerType.TableLayer.__doc__ + '\n' + '* ``Database``: ' + Qgis.BrowserLayerType.Database.__doc__ + '\n' + '* ``Table``: ' + Qgis.BrowserLayerType.Table.__doc__ + '\n' + '* ``Plugin``: ' + Qgis.BrowserLayerType.Plugin.__doc__ + '\n' + '* ``Mesh``: ' + Qgis.BrowserLayerType.Mesh.__doc__ + '\n' + '* ``VectorTile``: ' + Qgis.BrowserLayerType.VectorTile.__doc__ + '\n' + '* ``PointCloud``: ' + Qgis.BrowserLayerType.PointCloud.__doc__ + '\n' + '* ``TiledScene``: ' + Qgis.BrowserLayerType.TiledScene.__doc__
# --
Qgis.BrowserLayerType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.BrowserDirectoryMonitoring.Default.__doc__ = "Use default logic to determine whether directory should be monitored"
Qgis.BrowserDirectoryMonitoring.NeverMonitor.__doc__ = "Never monitor the directory, regardless of the default logic"
Qgis.BrowserDirectoryMonitoring.AlwaysMonitor.__doc__ = "Always monitor the directory, regardless of the default logic"
Qgis.BrowserDirectoryMonitoring.__doc__ = "Browser directory item monitoring switches.\n\n.. versionadded:: 3.20\n\n" + '* ``Default``: ' + Qgis.BrowserDirectoryMonitoring.Default.__doc__ + '\n' + '* ``NeverMonitor``: ' + Qgis.BrowserDirectoryMonitoring.NeverMonitor.__doc__ + '\n' + '* ``AlwaysMonitor``: ' + Qgis.BrowserDirectoryMonitoring.AlwaysMonitor.__doc__
# --
Qgis.BrowserDirectoryMonitoring.baseClass = Qgis
# monkey patching scoped based enum
Qgis.HttpMethod.Get.__doc__ = "GET method"
Qgis.HttpMethod.Post.__doc__ = "POST method"
Qgis.HttpMethod.__doc__ = "Different methods of HTTP requests\n\n.. versionadded:: 3.22\n\n" + '* ``Get``: ' + Qgis.HttpMethod.Get.__doc__ + '\n' + '* ``Post``: ' + Qgis.HttpMethod.Post.__doc__
# --
Qgis.HttpMethod.baseClass = Qgis
QgsVectorLayerExporter.ExportError = Qgis.VectorExportResult
# monkey patching scoped based enum
QgsVectorLayerExporter.NoError = Qgis.VectorExportResult.Success
QgsVectorLayerExporter.ExportError.NoError = Qgis.VectorExportResult.Success
QgsVectorLayerExporter.NoError.is_monkey_patched = True
QgsVectorLayerExporter.NoError.__doc__ = "No errors were encountered"
QgsVectorLayerExporter.ErrCreateDataSource = Qgis.VectorExportResult.ErrorCreatingDataSource
QgsVectorLayerExporter.ExportError.ErrCreateDataSource = Qgis.VectorExportResult.ErrorCreatingDataSource
QgsVectorLayerExporter.ErrCreateDataSource.is_monkey_patched = True
QgsVectorLayerExporter.ErrCreateDataSource.__doc__ = "Could not create the destination data source"
QgsVectorLayerExporter.ErrCreateLayer = Qgis.VectorExportResult.ErrorCreatingLayer
QgsVectorLayerExporter.ExportError.ErrCreateLayer = Qgis.VectorExportResult.ErrorCreatingLayer
QgsVectorLayerExporter.ErrCreateLayer.is_monkey_patched = True
QgsVectorLayerExporter.ErrCreateLayer.__doc__ = "Could not create destination layer"
QgsVectorLayerExporter.ErrAttributeTypeUnsupported = Qgis.VectorExportResult.ErrorAttributeTypeUnsupported
QgsVectorLayerExporter.ExportError.ErrAttributeTypeUnsupported = Qgis.VectorExportResult.ErrorAttributeTypeUnsupported
QgsVectorLayerExporter.ErrAttributeTypeUnsupported.is_monkey_patched = True
QgsVectorLayerExporter.ErrAttributeTypeUnsupported.__doc__ = "Source layer has an attribute type which could not be handled by destination"
QgsVectorLayerExporter.ErrAttributeCreationFailed = Qgis.VectorExportResult.ErrorAttributeCreationFailed
QgsVectorLayerExporter.ExportError.ErrAttributeCreationFailed = Qgis.VectorExportResult.ErrorAttributeCreationFailed
QgsVectorLayerExporter.ErrAttributeCreationFailed.is_monkey_patched = True
QgsVectorLayerExporter.ErrAttributeCreationFailed.__doc__ = "Destination provider was unable to create an attribute"
QgsVectorLayerExporter.ErrProjection = Qgis.VectorExportResult.ErrorProjectingFeatures
QgsVectorLayerExporter.ExportError.ErrProjection = Qgis.VectorExportResult.ErrorProjectingFeatures
QgsVectorLayerExporter.ErrProjection.is_monkey_patched = True
QgsVectorLayerExporter.ErrProjection.__doc__ = "An error occurred while reprojecting features to destination CRS"
QgsVectorLayerExporter.ErrFeatureWriteFailed = Qgis.VectorExportResult.ErrorFeatureWriteFailed
QgsVectorLayerExporter.ExportError.ErrFeatureWriteFailed = Qgis.VectorExportResult.ErrorFeatureWriteFailed
QgsVectorLayerExporter.ErrFeatureWriteFailed.is_monkey_patched = True
QgsVectorLayerExporter.ErrFeatureWriteFailed.__doc__ = "An error occurred while writing a feature to the destination"
QgsVectorLayerExporter.ErrInvalidLayer = Qgis.VectorExportResult.ErrorInvalidLayer
QgsVectorLayerExporter.ExportError.ErrInvalidLayer = Qgis.VectorExportResult.ErrorInvalidLayer
QgsVectorLayerExporter.ErrInvalidLayer.is_monkey_patched = True
QgsVectorLayerExporter.ErrInvalidLayer.__doc__ = "Could not access newly created destination layer"
QgsVectorLayerExporter.ErrInvalidProvider = Qgis.VectorExportResult.ErrorInvalidProvider
QgsVectorLayerExporter.ExportError.ErrInvalidProvider = Qgis.VectorExportResult.ErrorInvalidProvider
QgsVectorLayerExporter.ErrInvalidProvider.is_monkey_patched = True
QgsVectorLayerExporter.ErrInvalidProvider.__doc__ = "Could not find a matching provider key"
QgsVectorLayerExporter.ErrProviderUnsupportedFeature = Qgis.VectorExportResult.ErrorProviderUnsupportedFeature
QgsVectorLayerExporter.ExportError.ErrProviderUnsupportedFeature = Qgis.VectorExportResult.ErrorProviderUnsupportedFeature
QgsVectorLayerExporter.ErrProviderUnsupportedFeature.is_monkey_patched = True
QgsVectorLayerExporter.ErrProviderUnsupportedFeature.__doc__ = "Provider does not support creation of empty layers"
QgsVectorLayerExporter.ErrConnectionFailed = Qgis.VectorExportResult.ErrorConnectionFailed
QgsVectorLayerExporter.ExportError.ErrConnectionFailed = Qgis.VectorExportResult.ErrorConnectionFailed
QgsVectorLayerExporter.ErrConnectionFailed.is_monkey_patched = True
QgsVectorLayerExporter.ErrConnectionFailed.__doc__ = "Could not connect to destination"
QgsVectorLayerExporter.ErrUserCanceled = Qgis.VectorExportResult.UserCanceled
QgsVectorLayerExporter.ExportError.ErrUserCanceled = Qgis.VectorExportResult.UserCanceled
QgsVectorLayerExporter.ErrUserCanceled.is_monkey_patched = True
QgsVectorLayerExporter.ErrUserCanceled.__doc__ = "User canceled the export"
Qgis.VectorExportResult.__doc__ = "Vector layer export result codes.\n\n.. versionadded:: 3.20\n\n" + '* ``NoError``: ' + Qgis.VectorExportResult.Success.__doc__ + '\n' + '* ``ErrCreateDataSource``: ' + Qgis.VectorExportResult.ErrorCreatingDataSource.__doc__ + '\n' + '* ``ErrCreateLayer``: ' + Qgis.VectorExportResult.ErrorCreatingLayer.__doc__ + '\n' + '* ``ErrAttributeTypeUnsupported``: ' + Qgis.VectorExportResult.ErrorAttributeTypeUnsupported.__doc__ + '\n' + '* ``ErrAttributeCreationFailed``: ' + Qgis.VectorExportResult.ErrorAttributeCreationFailed.__doc__ + '\n' + '* ``ErrProjection``: ' + Qgis.VectorExportResult.ErrorProjectingFeatures.__doc__ + '\n' + '* ``ErrFeatureWriteFailed``: ' + Qgis.VectorExportResult.ErrorFeatureWriteFailed.__doc__ + '\n' + '* ``ErrInvalidLayer``: ' + Qgis.VectorExportResult.ErrorInvalidLayer.__doc__ + '\n' + '* ``ErrInvalidProvider``: ' + Qgis.VectorExportResult.ErrorInvalidProvider.__doc__ + '\n' + '* ``ErrProviderUnsupportedFeature``: ' + Qgis.VectorExportResult.ErrorProviderUnsupportedFeature.__doc__ + '\n' + '* ``ErrConnectionFailed``: ' + Qgis.VectorExportResult.ErrorConnectionFailed.__doc__ + '\n' + '* ``ErrUserCanceled``: ' + Qgis.VectorExportResult.UserCanceled.__doc__
# --
Qgis.VectorExportResult.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorFileWriterCapability.FieldAliases.__doc__ = "Writer can support field aliases"
Qgis.VectorFileWriterCapability.FieldComments.__doc__ = "Writer can support field comments"
Qgis.VectorFileWriterCapability.__doc__ = "Capabilities supported by a :py:class:`QgsVectorFileWriter` object.\n\n.. versionadded:: 3.32\n\n" + '* ``FieldAliases``: ' + Qgis.VectorFileWriterCapability.FieldAliases.__doc__ + '\n' + '* ``FieldComments``: ' + Qgis.VectorFileWriterCapability.FieldComments.__doc__
# --
Qgis.VectorFileWriterCapability.baseClass = Qgis
Qgis.VectorFileWriterCapabilities = lambda flags=0: Qgis.VectorFileWriterCapability(flags)
Qgis.VectorFileWriterCapabilities.baseClass = Qgis
VectorFileWriterCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SqlLayerDefinitionCapability.SubsetStringFilter.__doc__ = "SQL layer definition supports subset string filter"
Qgis.SqlLayerDefinitionCapability.GeometryColumn.__doc__ = "SQL layer definition supports geometry column"
Qgis.SqlLayerDefinitionCapability.PrimaryKeys.__doc__ = "SQL layer definition supports primary keys"
Qgis.SqlLayerDefinitionCapability.UnstableFeatureIds.__doc__ = "SQL layer definition supports disabling select at id"
Qgis.SqlLayerDefinitionCapability.__doc__ = "SqlLayerDefinitionCapability enum lists the arguments supported by the provider when creating SQL query layers.\n\n.. versionadded:: 3.22\n\n" + '* ``SubsetStringFilter``: ' + Qgis.SqlLayerDefinitionCapability.SubsetStringFilter.__doc__ + '\n' + '* ``GeometryColumn``: ' + Qgis.SqlLayerDefinitionCapability.GeometryColumn.__doc__ + '\n' + '* ``PrimaryKeys``: ' + Qgis.SqlLayerDefinitionCapability.PrimaryKeys.__doc__ + '\n' + '* ``UnstableFeatureIds``: ' + Qgis.SqlLayerDefinitionCapability.UnstableFeatureIds.__doc__
# --
Qgis.SqlLayerDefinitionCapability.baseClass = Qgis
Qgis.SqlLayerDefinitionCapabilities = lambda flags=0: Qgis.SqlLayerDefinitionCapability(flags)
Qgis.SqlLayerDefinitionCapabilities.baseClass = Qgis
SqlLayerDefinitionCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SqlKeywordCategory.Keyword.__doc__ = "SQL keyword"
Qgis.SqlKeywordCategory.Constant.__doc__ = "SQL constant"
Qgis.SqlKeywordCategory.Function.__doc__ = "SQL generic function"
Qgis.SqlKeywordCategory.Geospatial.__doc__ = "SQL spatial function"
Qgis.SqlKeywordCategory.Operator.__doc__ = "SQL operator"
Qgis.SqlKeywordCategory.Math.__doc__ = "SQL math function"
Qgis.SqlKeywordCategory.Aggregate.__doc__ = "SQL aggregate function"
Qgis.SqlKeywordCategory.String.__doc__ = "SQL string function"
Qgis.SqlKeywordCategory.Identifier.__doc__ = "SQL identifier"
Qgis.SqlKeywordCategory.__doc__ = "SqlKeywordCategory enum represents the categories of the SQL keywords used by the SQL query editor.\n\n.. note::\n\n   The category has currently no usage, but it was planned for future uses.\n\n.. versionadded:: 3.22\n\n" + '* ``Keyword``: ' + Qgis.SqlKeywordCategory.Keyword.__doc__ + '\n' + '* ``Constant``: ' + Qgis.SqlKeywordCategory.Constant.__doc__ + '\n' + '* ``Function``: ' + Qgis.SqlKeywordCategory.Function.__doc__ + '\n' + '* ``Geospatial``: ' + Qgis.SqlKeywordCategory.Geospatial.__doc__ + '\n' + '* ``Operator``: ' + Qgis.SqlKeywordCategory.Operator.__doc__ + '\n' + '* ``Math``: ' + Qgis.SqlKeywordCategory.Math.__doc__ + '\n' + '* ``Aggregate``: ' + Qgis.SqlKeywordCategory.Aggregate.__doc__ + '\n' + '* ``String``: ' + Qgis.SqlKeywordCategory.String.__doc__ + '\n' + '* ``Identifier``: ' + Qgis.SqlKeywordCategory.Identifier.__doc__
# --
Qgis.SqlKeywordCategory.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DriveType.Unknown.__doc__ = "Unknown type"
Qgis.DriveType.Invalid.__doc__ = "Invalid path"
Qgis.DriveType.Removable.__doc__ = "Removable drive"
Qgis.DriveType.Fixed.__doc__ = "Fixed drive"
Qgis.DriveType.Remote.__doc__ = "Remote drive"
Qgis.DriveType.CdRom.__doc__ = "CD-ROM"
Qgis.DriveType.RamDisk.__doc__ = "RAM disk"
Qgis.DriveType.Cloud.__doc__ = "Cloud storage -- files may be remote or locally stored, depending on user configuration"
Qgis.DriveType.__doc__ = "Drive types\n\n.. versionadded:: 3.20\n\n" + '* ``Unknown``: ' + Qgis.DriveType.Unknown.__doc__ + '\n' + '* ``Invalid``: ' + Qgis.DriveType.Invalid.__doc__ + '\n' + '* ``Removable``: ' + Qgis.DriveType.Removable.__doc__ + '\n' + '* ``Fixed``: ' + Qgis.DriveType.Fixed.__doc__ + '\n' + '* ``Remote``: ' + Qgis.DriveType.Remote.__doc__ + '\n' + '* ``CdRom``: ' + Qgis.DriveType.CdRom.__doc__ + '\n' + '* ``RamDisk``: ' + Qgis.DriveType.RamDisk.__doc__ + '\n' + '* ``Cloud``: ' + Qgis.DriveType.Cloud.__doc__
# --
Qgis.DriveType.baseClass = Qgis
QgsNetworkContentFetcherRegistry.FetchingMode = Qgis.ActionStart
# monkey patching scoped based enum
QgsNetworkContentFetcherRegistry.DownloadLater = Qgis.ActionStart.Deferred
QgsNetworkContentFetcherRegistry.FetchingMode.DownloadLater = Qgis.ActionStart.Deferred
QgsNetworkContentFetcherRegistry.DownloadLater.is_monkey_patched = True
QgsNetworkContentFetcherRegistry.DownloadLater.__doc__ = "Do not start immediately the action"
QgsNetworkContentFetcherRegistry.DownloadImmediately = Qgis.ActionStart.Immediate
QgsNetworkContentFetcherRegistry.FetchingMode.DownloadImmediately = Qgis.ActionStart.Immediate
QgsNetworkContentFetcherRegistry.DownloadImmediately.is_monkey_patched = True
QgsNetworkContentFetcherRegistry.DownloadImmediately.__doc__ = "Action will start immediately"
Qgis.ActionStart.__doc__ = "Enum to determine when an operation would begin\n\n.. versionadded:: 3.22\n\n" + '* ``DownloadLater``: ' + Qgis.ActionStart.Deferred.__doc__ + '\n' + '* ``DownloadImmediately``: ' + Qgis.ActionStart.Immediate.__doc__
# --
Qgis.ActionStart.baseClass = Qgis
# monkey patching scoped based enum
Qgis.UnplacedLabelVisibility.FollowEngineSetting.__doc__ = "Respect the label engine setting"
Qgis.UnplacedLabelVisibility.NeverShow.__doc__ = "Never show unplaced labels, regardless of the engine setting"
Qgis.UnplacedLabelVisibility.__doc__ = "Unplaced label visibility.\n\n.. versionadded:: 3.20\n\n" + '* ``FollowEngineSetting``: ' + Qgis.UnplacedLabelVisibility.FollowEngineSetting.__doc__ + '\n' + '* ``NeverShow``: ' + Qgis.UnplacedLabelVisibility.NeverShow.__doc__
# --
Qgis.UnplacedLabelVisibility.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LabelOverlapHandling.PreventOverlap.__doc__ = "Do not allow labels to overlap other labels"
Qgis.LabelOverlapHandling.AllowOverlapIfRequired.__doc__ = "Avoids overlapping labels when possible, but permit overlaps if labels for features cannot otherwise be placed"
Qgis.LabelOverlapHandling.AllowOverlapAtNoCost.__doc__ = "Labels may freely overlap other labels, at no cost"
Qgis.LabelOverlapHandling.__doc__ = "Label overlap handling.\n\n.. versionadded:: 3.26\n\n" + '* ``PreventOverlap``: ' + Qgis.LabelOverlapHandling.PreventOverlap.__doc__ + '\n' + '* ``AllowOverlapIfRequired``: ' + Qgis.LabelOverlapHandling.AllowOverlapIfRequired.__doc__ + '\n' + '* ``AllowOverlapAtNoCost``: ' + Qgis.LabelOverlapHandling.AllowOverlapAtNoCost.__doc__
# --
Qgis.LabelOverlapHandling.baseClass = Qgis
QgsPalLayerSettings.Placement = Qgis.LabelPlacement
# monkey patching scoped based enum
QgsPalLayerSettings.AroundPoint = Qgis.LabelPlacement.AroundPoint
QgsPalLayerSettings.AroundPoint.is_monkey_patched = True
QgsPalLayerSettings.AroundPoint.__doc__ = "Arranges candidates in a circle around a point (or centroid of a polygon). Applies to point or polygon layers only."
QgsPalLayerSettings.OverPoint = Qgis.LabelPlacement.OverPoint
QgsPalLayerSettings.OverPoint.is_monkey_patched = True
QgsPalLayerSettings.OverPoint.__doc__ = "Arranges candidates over a point (or centroid of a polygon), or at a preset offset from the point. Applies to point or polygon layers only."
QgsPalLayerSettings.Line = Qgis.LabelPlacement.Line
QgsPalLayerSettings.Line.is_monkey_patched = True
QgsPalLayerSettings.Line.__doc__ = "Arranges candidates parallel to a generalised line representing the feature or parallel to a polygon's perimeter. Applies to line or polygon layers only."
QgsPalLayerSettings.Curved = Qgis.LabelPlacement.Curved
QgsPalLayerSettings.Curved.is_monkey_patched = True
QgsPalLayerSettings.Curved.__doc__ = "Arranges candidates following the curvature of a line feature. Applies to line layers only."
QgsPalLayerSettings.Horizontal = Qgis.LabelPlacement.Horizontal
QgsPalLayerSettings.Horizontal.is_monkey_patched = True
QgsPalLayerSettings.Horizontal.__doc__ = "Arranges horizontal candidates scattered throughout a polygon feature. Applies to polygon layers only."
QgsPalLayerSettings.Free = Qgis.LabelPlacement.Free
QgsPalLayerSettings.Free.is_monkey_patched = True
QgsPalLayerSettings.Free.__doc__ = "Arranges candidates scattered throughout a polygon feature. Candidates are rotated to respect the polygon's orientation. Applies to polygon layers only."
QgsPalLayerSettings.OrderedPositionsAroundPoint = Qgis.LabelPlacement.OrderedPositionsAroundPoint
QgsPalLayerSettings.OrderedPositionsAroundPoint.is_monkey_patched = True
QgsPalLayerSettings.OrderedPositionsAroundPoint.__doc__ = "Candidates are placed in predefined positions around a point. Preference is given to positions with greatest cartographic appeal, e.g., top right, bottom right, etc. Applies to point layers only."
QgsPalLayerSettings.PerimeterCurved = Qgis.LabelPlacement.PerimeterCurved
QgsPalLayerSettings.PerimeterCurved.is_monkey_patched = True
QgsPalLayerSettings.PerimeterCurved.__doc__ = "Arranges candidates following the curvature of a polygon's boundary. Applies to polygon layers only."
QgsPalLayerSettings.OutsidePolygons = Qgis.LabelPlacement.OutsidePolygons
QgsPalLayerSettings.OutsidePolygons.is_monkey_patched = True
QgsPalLayerSettings.OutsidePolygons.__doc__ = "Candidates are placed outside of polygon boundaries. Applies to polygon layers only. Since QGIS 3.14"
Qgis.LabelPlacement.__doc__ = "Placement modes which determine how label candidates are generated for a feature.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.Placement\n\n.. versionadded:: 3.26\n\n" + '* ``AroundPoint``: ' + Qgis.LabelPlacement.AroundPoint.__doc__ + '\n' + '* ``OverPoint``: ' + Qgis.LabelPlacement.OverPoint.__doc__ + '\n' + '* ``Line``: ' + Qgis.LabelPlacement.Line.__doc__ + '\n' + '* ``Curved``: ' + Qgis.LabelPlacement.Curved.__doc__ + '\n' + '* ``Horizontal``: ' + Qgis.LabelPlacement.Horizontal.__doc__ + '\n' + '* ``Free``: ' + Qgis.LabelPlacement.Free.__doc__ + '\n' + '* ``OrderedPositionsAroundPoint``: ' + Qgis.LabelPlacement.OrderedPositionsAroundPoint.__doc__ + '\n' + '* ``PerimeterCurved``: ' + Qgis.LabelPlacement.PerimeterCurved.__doc__ + '\n' + '* ``OutsidePolygons``: ' + Qgis.LabelPlacement.OutsidePolygons.__doc__
# --
Qgis.LabelPlacement.baseClass = Qgis
QgsPalLayerSettings.PredefinedPointPosition = Qgis.LabelPredefinedPointPosition
# monkey patching scoped based enum
QgsPalLayerSettings.TopLeft = Qgis.LabelPredefinedPointPosition.TopLeft
QgsPalLayerSettings.TopLeft.is_monkey_patched = True
QgsPalLayerSettings.TopLeft.__doc__ = "Label on top-left of point"
QgsPalLayerSettings.TopSlightlyLeft = Qgis.LabelPredefinedPointPosition.TopSlightlyLeft
QgsPalLayerSettings.TopSlightlyLeft.is_monkey_patched = True
QgsPalLayerSettings.TopSlightlyLeft.__doc__ = "Label on top of point, slightly left of center"
QgsPalLayerSettings.TopMiddle = Qgis.LabelPredefinedPointPosition.TopMiddle
QgsPalLayerSettings.TopMiddle.is_monkey_patched = True
QgsPalLayerSettings.TopMiddle.__doc__ = "Label directly above point"
QgsPalLayerSettings.TopSlightlyRight = Qgis.LabelPredefinedPointPosition.TopSlightlyRight
QgsPalLayerSettings.TopSlightlyRight.is_monkey_patched = True
QgsPalLayerSettings.TopSlightlyRight.__doc__ = "Label on top of point, slightly right of center"
QgsPalLayerSettings.TopRight = Qgis.LabelPredefinedPointPosition.TopRight
QgsPalLayerSettings.TopRight.is_monkey_patched = True
QgsPalLayerSettings.TopRight.__doc__ = "Label on top-right of point"
QgsPalLayerSettings.MiddleLeft = Qgis.LabelPredefinedPointPosition.MiddleLeft
QgsPalLayerSettings.MiddleLeft.is_monkey_patched = True
QgsPalLayerSettings.MiddleLeft.__doc__ = "Label on left of point"
QgsPalLayerSettings.MiddleRight = Qgis.LabelPredefinedPointPosition.MiddleRight
QgsPalLayerSettings.MiddleRight.is_monkey_patched = True
QgsPalLayerSettings.MiddleRight.__doc__ = "Label on right of point"
QgsPalLayerSettings.BottomLeft = Qgis.LabelPredefinedPointPosition.BottomLeft
QgsPalLayerSettings.BottomLeft.is_monkey_patched = True
QgsPalLayerSettings.BottomLeft.__doc__ = "Label on bottom-left of point"
QgsPalLayerSettings.BottomSlightlyLeft = Qgis.LabelPredefinedPointPosition.BottomSlightlyLeft
QgsPalLayerSettings.BottomSlightlyLeft.is_monkey_patched = True
QgsPalLayerSettings.BottomSlightlyLeft.__doc__ = "Label below point, slightly left of center"
QgsPalLayerSettings.BottomMiddle = Qgis.LabelPredefinedPointPosition.BottomMiddle
QgsPalLayerSettings.BottomMiddle.is_monkey_patched = True
QgsPalLayerSettings.BottomMiddle.__doc__ = "Label directly below point"
QgsPalLayerSettings.BottomSlightlyRight = Qgis.LabelPredefinedPointPosition.BottomSlightlyRight
QgsPalLayerSettings.BottomSlightlyRight.is_monkey_patched = True
QgsPalLayerSettings.BottomSlightlyRight.__doc__ = "Label below point, slightly right of center"
QgsPalLayerSettings.BottomRight = Qgis.LabelPredefinedPointPosition.BottomRight
QgsPalLayerSettings.BottomRight.is_monkey_patched = True
QgsPalLayerSettings.BottomRight.__doc__ = "Label on bottom right of point"
Qgis.LabelPredefinedPointPosition.__doc__ = "Positions for labels when using the Qgis.LabelPlacement.OrderedPositionsAroundPoint placement mode.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.PredefinedPointPosition\n\n.. versionadded:: 3.26\n\n" + '* ``TopLeft``: ' + Qgis.LabelPredefinedPointPosition.TopLeft.__doc__ + '\n' + '* ``TopSlightlyLeft``: ' + Qgis.LabelPredefinedPointPosition.TopSlightlyLeft.__doc__ + '\n' + '* ``TopMiddle``: ' + Qgis.LabelPredefinedPointPosition.TopMiddle.__doc__ + '\n' + '* ``TopSlightlyRight``: ' + Qgis.LabelPredefinedPointPosition.TopSlightlyRight.__doc__ + '\n' + '* ``TopRight``: ' + Qgis.LabelPredefinedPointPosition.TopRight.__doc__ + '\n' + '* ``MiddleLeft``: ' + Qgis.LabelPredefinedPointPosition.MiddleLeft.__doc__ + '\n' + '* ``MiddleRight``: ' + Qgis.LabelPredefinedPointPosition.MiddleRight.__doc__ + '\n' + '* ``BottomLeft``: ' + Qgis.LabelPredefinedPointPosition.BottomLeft.__doc__ + '\n' + '* ``BottomSlightlyLeft``: ' + Qgis.LabelPredefinedPointPosition.BottomSlightlyLeft.__doc__ + '\n' + '* ``BottomMiddle``: ' + Qgis.LabelPredefinedPointPosition.BottomMiddle.__doc__ + '\n' + '* ``BottomSlightlyRight``: ' + Qgis.LabelPredefinedPointPosition.BottomSlightlyRight.__doc__ + '\n' + '* ``BottomRight``: ' + Qgis.LabelPredefinedPointPosition.BottomRight.__doc__
# --
Qgis.LabelPredefinedPointPosition.baseClass = Qgis
QgsPalLayerSettings.OffsetType = Qgis.LabelOffsetType
# monkey patching scoped based enum
QgsPalLayerSettings.FromPoint = Qgis.LabelOffsetType.FromPoint
QgsPalLayerSettings.FromPoint.is_monkey_patched = True
QgsPalLayerSettings.FromPoint.__doc__ = "Offset distance applies from point geometry"
QgsPalLayerSettings.FromSymbolBounds = Qgis.LabelOffsetType.FromSymbolBounds
QgsPalLayerSettings.FromSymbolBounds.is_monkey_patched = True
QgsPalLayerSettings.FromSymbolBounds.__doc__ = "Offset distance applies from rendered symbol bounds"
Qgis.LabelOffsetType.__doc__ = "Behavior modifier for label offset and distance, only applies in some\nlabel placement modes.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.OffsetType\n\n.. versionadded:: 3.26\n\n" + '* ``FromPoint``: ' + Qgis.LabelOffsetType.FromPoint.__doc__ + '\n' + '* ``FromSymbolBounds``: ' + Qgis.LabelOffsetType.FromSymbolBounds.__doc__
# --
Qgis.LabelOffsetType.baseClass = Qgis
QgsPalLayerSettings.QuadrantPosition = Qgis.LabelQuadrantPosition
# monkey patching scoped based enum
QgsPalLayerSettings.QuadrantAboveLeft = Qgis.LabelQuadrantPosition.AboveLeft
QgsPalLayerSettings.QuadrantPosition.QuadrantAboveLeft = Qgis.LabelQuadrantPosition.AboveLeft
QgsPalLayerSettings.QuadrantAboveLeft.is_monkey_patched = True
QgsPalLayerSettings.QuadrantAboveLeft.__doc__ = "Above left"
QgsPalLayerSettings.QuadrantAbove = Qgis.LabelQuadrantPosition.Above
QgsPalLayerSettings.QuadrantPosition.QuadrantAbove = Qgis.LabelQuadrantPosition.Above
QgsPalLayerSettings.QuadrantAbove.is_monkey_patched = True
QgsPalLayerSettings.QuadrantAbove.__doc__ = "Above center"
QgsPalLayerSettings.QuadrantAboveRight = Qgis.LabelQuadrantPosition.AboveRight
QgsPalLayerSettings.QuadrantPosition.QuadrantAboveRight = Qgis.LabelQuadrantPosition.AboveRight
QgsPalLayerSettings.QuadrantAboveRight.is_monkey_patched = True
QgsPalLayerSettings.QuadrantAboveRight.__doc__ = "Above right"
QgsPalLayerSettings.QuadrantLeft = Qgis.LabelQuadrantPosition.Left
QgsPalLayerSettings.QuadrantPosition.QuadrantLeft = Qgis.LabelQuadrantPosition.Left
QgsPalLayerSettings.QuadrantLeft.is_monkey_patched = True
QgsPalLayerSettings.QuadrantLeft.__doc__ = "Left middle"
QgsPalLayerSettings.QuadrantOver = Qgis.LabelQuadrantPosition.Over
QgsPalLayerSettings.QuadrantPosition.QuadrantOver = Qgis.LabelQuadrantPosition.Over
QgsPalLayerSettings.QuadrantOver.is_monkey_patched = True
QgsPalLayerSettings.QuadrantOver.__doc__ = "Center middle"
QgsPalLayerSettings.QuadrantRight = Qgis.LabelQuadrantPosition.Right
QgsPalLayerSettings.QuadrantPosition.QuadrantRight = Qgis.LabelQuadrantPosition.Right
QgsPalLayerSettings.QuadrantRight.is_monkey_patched = True
QgsPalLayerSettings.QuadrantRight.__doc__ = "Right middle"
QgsPalLayerSettings.QuadrantBelowLeft = Qgis.LabelQuadrantPosition.BelowLeft
QgsPalLayerSettings.QuadrantPosition.QuadrantBelowLeft = Qgis.LabelQuadrantPosition.BelowLeft
QgsPalLayerSettings.QuadrantBelowLeft.is_monkey_patched = True
QgsPalLayerSettings.QuadrantBelowLeft.__doc__ = "Below left"
QgsPalLayerSettings.QuadrantBelow = Qgis.LabelQuadrantPosition.Below
QgsPalLayerSettings.QuadrantPosition.QuadrantBelow = Qgis.LabelQuadrantPosition.Below
QgsPalLayerSettings.QuadrantBelow.is_monkey_patched = True
QgsPalLayerSettings.QuadrantBelow.__doc__ = "Below center"
QgsPalLayerSettings.QuadrantBelowRight = Qgis.LabelQuadrantPosition.BelowRight
QgsPalLayerSettings.QuadrantPosition.QuadrantBelowRight = Qgis.LabelQuadrantPosition.BelowRight
QgsPalLayerSettings.QuadrantBelowRight.is_monkey_patched = True
QgsPalLayerSettings.QuadrantBelowRight.__doc__ = "BelowRight"
Qgis.LabelQuadrantPosition.__doc__ = "Label quadrant positions\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.QuadrantPosition\n\n.. versionadded:: 3.26\n\n" + '* ``QuadrantAboveLeft``: ' + Qgis.LabelQuadrantPosition.AboveLeft.__doc__ + '\n' + '* ``QuadrantAbove``: ' + Qgis.LabelQuadrantPosition.Above.__doc__ + '\n' + '* ``QuadrantAboveRight``: ' + Qgis.LabelQuadrantPosition.AboveRight.__doc__ + '\n' + '* ``QuadrantLeft``: ' + Qgis.LabelQuadrantPosition.Left.__doc__ + '\n' + '* ``QuadrantOver``: ' + Qgis.LabelQuadrantPosition.Over.__doc__ + '\n' + '* ``QuadrantRight``: ' + Qgis.LabelQuadrantPosition.Right.__doc__ + '\n' + '* ``QuadrantBelowLeft``: ' + Qgis.LabelQuadrantPosition.BelowLeft.__doc__ + '\n' + '* ``QuadrantBelow``: ' + Qgis.LabelQuadrantPosition.Below.__doc__ + '\n' + '* ``QuadrantBelowRight``: ' + Qgis.LabelQuadrantPosition.BelowRight.__doc__
# --
Qgis.LabelQuadrantPosition.baseClass = Qgis
QgsLabeling.LinePlacementFlag = Qgis.LabelLinePlacementFlag
# monkey patching scoped based enum
QgsLabeling.OnLine = Qgis.LabelLinePlacementFlag.OnLine
QgsLabeling.OnLine.is_monkey_patched = True
QgsLabeling.OnLine.__doc__ = "Labels can be placed directly over a line feature."
QgsLabeling.AboveLine = Qgis.LabelLinePlacementFlag.AboveLine
QgsLabeling.AboveLine.is_monkey_patched = True
QgsLabeling.AboveLine.__doc__ = "Labels can be placed above a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed below the line feature."
QgsLabeling.BelowLine = Qgis.LabelLinePlacementFlag.BelowLine
QgsLabeling.BelowLine.is_monkey_patched = True
QgsLabeling.BelowLine.__doc__ = "Labels can be placed below a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed above the line feature."
QgsLabeling.MapOrientation = Qgis.LabelLinePlacementFlag.MapOrientation
QgsLabeling.MapOrientation.is_monkey_patched = True
QgsLabeling.MapOrientation.__doc__ = "Signifies that the AboveLine and BelowLine flags should respect the map's orientation rather than the feature's orientation. For example, AboveLine will always result in label's being placed above a line, regardless of the line's direction."
Qgis.LabelLinePlacementFlag.__doc__ = "Line placement flags, which control how candidates are generated for a linear feature.\n\n.. note::\n\n   Prior to QGIS 3.32 this was available as :py:class:`QgsLabeling`.LinePlacementFlag\n\n.. versionadded:: 3.32\n\n" + '* ``OnLine``: ' + Qgis.LabelLinePlacementFlag.OnLine.__doc__ + '\n' + '* ``AboveLine``: ' + Qgis.LabelLinePlacementFlag.AboveLine.__doc__ + '\n' + '* ``BelowLine``: ' + Qgis.LabelLinePlacementFlag.BelowLine.__doc__ + '\n' + '* ``MapOrientation``: ' + Qgis.LabelLinePlacementFlag.MapOrientation.__doc__
# --
Qgis.LabelLinePlacementFlag.baseClass = Qgis
Qgis.LabelLinePlacementFlags = lambda flags=0: Qgis.LabelLinePlacementFlag(flags)
QgsLabeling.LinePlacementFlags = Qgis.LabelLinePlacementFlags
Qgis.LabelLinePlacementFlags.baseClass = Qgis
LabelLinePlacementFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsLabeling.PolygonPlacementFlag = Qgis.LabelPolygonPlacementFlag
# monkey patching scoped based enum
QgsLabeling.AllowPlacementOutsideOfPolygon = Qgis.LabelPolygonPlacementFlag.AllowPlacementOutsideOfPolygon
QgsLabeling.AllowPlacementOutsideOfPolygon.is_monkey_patched = True
QgsLabeling.AllowPlacementOutsideOfPolygon.__doc__ = "Labels can be placed outside of a polygon feature"
QgsLabeling.AllowPlacementInsideOfPolygon = Qgis.LabelPolygonPlacementFlag.AllowPlacementInsideOfPolygon
QgsLabeling.AllowPlacementInsideOfPolygon.is_monkey_patched = True
QgsLabeling.AllowPlacementInsideOfPolygon.__doc__ = "Labels can be placed inside a polygon feature"
Qgis.LabelPolygonPlacementFlag.__doc__ = "Polygon placement flags, which control how candidates are generated for a polygon feature.\n\n.. note::\n\n   Prior to QGIS 3.32 this was available as :py:class:`QgsLabeling`.PolygonPlacementFlag\n\n.. versionadded:: 3.32\n\n" + '* ``AllowPlacementOutsideOfPolygon``: ' + Qgis.LabelPolygonPlacementFlag.AllowPlacementOutsideOfPolygon.__doc__ + '\n' + '* ``AllowPlacementInsideOfPolygon``: ' + Qgis.LabelPolygonPlacementFlag.AllowPlacementInsideOfPolygon.__doc__
# --
Qgis.LabelPolygonPlacementFlag.baseClass = Qgis
Qgis.LabelPolygonPlacementFlags = lambda flags=0: Qgis.LabelPolygonPlacementFlag(flags)
Qgis.LabelPolygonPlacementFlags.baseClass = Qgis
LabelPolygonPlacementFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsPalLayerSettings.UpsideDownLabels = Qgis.UpsideDownLabelHandling
# monkey patching scoped based enum
QgsPalLayerSettings.Upright = Qgis.UpsideDownLabelHandling.FlipUpsideDownLabels
QgsPalLayerSettings.UpsideDownLabels.Upright = Qgis.UpsideDownLabelHandling.FlipUpsideDownLabels
QgsPalLayerSettings.Upright.is_monkey_patched = True
QgsPalLayerSettings.Upright.__doc__ = "Upside-down labels (90 <= angle < 270) are shown upright"
QgsPalLayerSettings.ShowDefined = Qgis.UpsideDownLabelHandling.AllowUpsideDownWhenRotationIsDefined
QgsPalLayerSettings.UpsideDownLabels.ShowDefined = Qgis.UpsideDownLabelHandling.AllowUpsideDownWhenRotationIsDefined
QgsPalLayerSettings.ShowDefined.is_monkey_patched = True
QgsPalLayerSettings.ShowDefined.__doc__ = "Show upside down when rotation is layer- or data-defined"
QgsPalLayerSettings.ShowAll = Qgis.UpsideDownLabelHandling.AlwaysAllowUpsideDown
QgsPalLayerSettings.UpsideDownLabels.ShowAll = Qgis.UpsideDownLabelHandling.AlwaysAllowUpsideDown
QgsPalLayerSettings.ShowAll.is_monkey_patched = True
QgsPalLayerSettings.ShowAll.__doc__ = "Show upside down for all labels, including dynamic ones"
Qgis.UpsideDownLabelHandling.__doc__ = "Handling techniques for upside down labels.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.UpsideDownLabels\n\n.. versionadded:: 3.26\n\n" + '* ``Upright``: ' + Qgis.UpsideDownLabelHandling.FlipUpsideDownLabels.__doc__ + '\n' + '* ``ShowDefined``: ' + Qgis.UpsideDownLabelHandling.AllowUpsideDownWhenRotationIsDefined.__doc__ + '\n' + '* ``ShowAll``: ' + Qgis.UpsideDownLabelHandling.AlwaysAllowUpsideDown.__doc__
# --
Qgis.UpsideDownLabelHandling.baseClass = Qgis
QgsPalLayerSettings.MultiLineAlign = Qgis.LabelMultiLineAlignment
# monkey patching scoped based enum
QgsPalLayerSettings.MultiLeft = Qgis.LabelMultiLineAlignment.Left
QgsPalLayerSettings.MultiLineAlign.MultiLeft = Qgis.LabelMultiLineAlignment.Left
QgsPalLayerSettings.MultiLeft.is_monkey_patched = True
QgsPalLayerSettings.MultiLeft.__doc__ = "Left align"
QgsPalLayerSettings.MultiCenter = Qgis.LabelMultiLineAlignment.Center
QgsPalLayerSettings.MultiLineAlign.MultiCenter = Qgis.LabelMultiLineAlignment.Center
QgsPalLayerSettings.MultiCenter.is_monkey_patched = True
QgsPalLayerSettings.MultiCenter.__doc__ = "Center align"
QgsPalLayerSettings.MultiRight = Qgis.LabelMultiLineAlignment.Right
QgsPalLayerSettings.MultiLineAlign.MultiRight = Qgis.LabelMultiLineAlignment.Right
QgsPalLayerSettings.MultiRight.is_monkey_patched = True
QgsPalLayerSettings.MultiRight.__doc__ = "Right align"
QgsPalLayerSettings.MultiFollowPlacement = Qgis.LabelMultiLineAlignment.FollowPlacement
QgsPalLayerSettings.MultiLineAlign.MultiFollowPlacement = Qgis.LabelMultiLineAlignment.FollowPlacement
QgsPalLayerSettings.MultiFollowPlacement.is_monkey_patched = True
QgsPalLayerSettings.MultiFollowPlacement.__doc__ = "Alignment follows placement of label, e.g., labels to the left of a feature will be drawn with right alignment"
QgsPalLayerSettings.MultiJustify = Qgis.LabelMultiLineAlignment.Justify
QgsPalLayerSettings.MultiLineAlign.MultiJustify = Qgis.LabelMultiLineAlignment.Justify
QgsPalLayerSettings.MultiJustify.is_monkey_patched = True
QgsPalLayerSettings.MultiJustify.__doc__ = "Justified"
Qgis.LabelMultiLineAlignment.__doc__ = "Text alignment for multi-line labels.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.MultiLineAlign\n\n.. versionadded:: 3.26\n\n" + '* ``MultiLeft``: ' + Qgis.LabelMultiLineAlignment.Left.__doc__ + '\n' + '* ``MultiCenter``: ' + Qgis.LabelMultiLineAlignment.Center.__doc__ + '\n' + '* ``MultiRight``: ' + Qgis.LabelMultiLineAlignment.Right.__doc__ + '\n' + '* ``MultiFollowPlacement``: ' + Qgis.LabelMultiLineAlignment.FollowPlacement.__doc__ + '\n' + '* ``MultiJustify``: ' + Qgis.LabelMultiLineAlignment.Justify.__doc__
# --
Qgis.LabelMultiLineAlignment.baseClass = Qgis
QgsProviderMetadata.FilterType = Qgis.FileFilterType
# monkey patching scoped based enum
QgsProviderMetadata.FilterVector = Qgis.FileFilterType.Vector
QgsProviderMetadata.FilterType.FilterVector = Qgis.FileFilterType.Vector
QgsProviderMetadata.FilterVector.is_monkey_patched = True
QgsProviderMetadata.FilterVector.__doc__ = "Vector layers"
QgsProviderMetadata.FilterRaster = Qgis.FileFilterType.Raster
QgsProviderMetadata.FilterType.FilterRaster = Qgis.FileFilterType.Raster
QgsProviderMetadata.FilterRaster.is_monkey_patched = True
QgsProviderMetadata.FilterRaster.__doc__ = "Raster layers"
QgsProviderMetadata.FilterMesh = Qgis.FileFilterType.Mesh
QgsProviderMetadata.FilterType.FilterMesh = Qgis.FileFilterType.Mesh
QgsProviderMetadata.FilterMesh.is_monkey_patched = True
QgsProviderMetadata.FilterMesh.__doc__ = "Mesh layers"
QgsProviderMetadata.FilterMeshDataset = Qgis.FileFilterType.MeshDataset
QgsProviderMetadata.FilterType.FilterMeshDataset = Qgis.FileFilterType.MeshDataset
QgsProviderMetadata.FilterMeshDataset.is_monkey_patched = True
QgsProviderMetadata.FilterMeshDataset.__doc__ = "Mesh datasets"
QgsProviderMetadata.FilterPointCloud = Qgis.FileFilterType.PointCloud
QgsProviderMetadata.FilterType.FilterPointCloud = Qgis.FileFilterType.PointCloud
QgsProviderMetadata.FilterPointCloud.is_monkey_patched = True
QgsProviderMetadata.FilterPointCloud.__doc__ = "Point clouds (since QGIS 3.18)"
QgsProviderMetadata.VectorTile = Qgis.FileFilterType.VectorTile
QgsProviderMetadata.VectorTile.is_monkey_patched = True
QgsProviderMetadata.VectorTile.__doc__ = "Vector tile layers (since QGIS 3.32)"
QgsProviderMetadata.TiledScene = Qgis.FileFilterType.TiledScene
QgsProviderMetadata.TiledScene.is_monkey_patched = True
QgsProviderMetadata.TiledScene.__doc__ = "Tiled scene layers (since QGIS 3.34)"
Qgis.FileFilterType.__doc__ = "Type of file filters\n\nPrior to QGIS 3.32 this was available as :py:class:`QgsProviderMetadata`.FilterType\n\n.. versionadded:: 3.32\n\n" + '* ``FilterVector``: ' + Qgis.FileFilterType.Vector.__doc__ + '\n' + '* ``FilterRaster``: ' + Qgis.FileFilterType.Raster.__doc__ + '\n' + '* ``FilterMesh``: ' + Qgis.FileFilterType.Mesh.__doc__ + '\n' + '* ``FilterMeshDataset``: ' + Qgis.FileFilterType.MeshDataset.__doc__ + '\n' + '* ``FilterPointCloud``: ' + Qgis.FileFilterType.PointCloud.__doc__ + '\n' + '* ``VectorTile``: ' + Qgis.FileFilterType.VectorTile.__doc__ + '\n' + '* ``TiledScene``: ' + Qgis.FileFilterType.TiledScene.__doc__
# --
Qgis.FileFilterType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SublayerQueryFlag.FastScan.__doc__ = "Indicates that the provider must scan for sublayers using the fastest possible approach -- e.g. by first checking that a uri has an extension which is known to be readable by the provider"
Qgis.SublayerQueryFlag.ResolveGeometryType.__doc__ = "Attempt to resolve the geometry type for vector sublayers"
Qgis.SublayerQueryFlag.CountFeatures.__doc__ = "Count features in vector sublayers"
Qgis.SublayerQueryFlag.IncludeSystemTables.__doc__ = "Include system or internal tables (these are not included by default)"
Qgis.SublayerQueryFlag.__doc__ = "Flags which control how data providers will scan for sublayers in a dataset.\n\n.. versionadded:: 3.22\n\n" + '* ``FastScan``: ' + Qgis.SublayerQueryFlag.FastScan.__doc__ + '\n' + '* ``ResolveGeometryType``: ' + Qgis.SublayerQueryFlag.ResolveGeometryType.__doc__ + '\n' + '* ``CountFeatures``: ' + Qgis.SublayerQueryFlag.CountFeatures.__doc__ + '\n' + '* ``IncludeSystemTables``: ' + Qgis.SublayerQueryFlag.IncludeSystemTables.__doc__
# --
Qgis.SublayerQueryFlags = lambda flags=0: Qgis.SublayerQueryFlag(flags)
Qgis.SublayerQueryFlag.baseClass = Qgis
Qgis.SublayerQueryFlags.baseClass = Qgis
SublayerQueryFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SublayerFlag.SystemTable.__doc__ = "Sublayer is a system or internal table, which should be hidden by default"
Qgis.SublayerFlag.__doc__ = "Flags which reflect the properties of sublayers in a dataset.\n\n.. versionadded:: 3.22\n\n" + '* ``SystemTable``: ' + Qgis.SublayerFlag.SystemTable.__doc__
# --
Qgis.SublayerFlags = lambda flags=0: Qgis.SublayerFlag(flags)
Qgis.SublayerFlag.baseClass = Qgis
Qgis.SublayerFlags.baseClass = Qgis
SublayerFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsRasterPipe.Role = Qgis.RasterPipeInterfaceRole
# monkey patching scoped based enum
QgsRasterPipe.UnknownRole = Qgis.RasterPipeInterfaceRole.Unknown
QgsRasterPipe.Role.UnknownRole = Qgis.RasterPipeInterfaceRole.Unknown
QgsRasterPipe.UnknownRole.is_monkey_patched = True
QgsRasterPipe.UnknownRole.__doc__ = "Unknown role"
QgsRasterPipe.ProviderRole = Qgis.RasterPipeInterfaceRole.Provider
QgsRasterPipe.Role.ProviderRole = Qgis.RasterPipeInterfaceRole.Provider
QgsRasterPipe.ProviderRole.is_monkey_patched = True
QgsRasterPipe.ProviderRole.__doc__ = "Data provider role"
QgsRasterPipe.RendererRole = Qgis.RasterPipeInterfaceRole.Renderer
QgsRasterPipe.Role.RendererRole = Qgis.RasterPipeInterfaceRole.Renderer
QgsRasterPipe.RendererRole.is_monkey_patched = True
QgsRasterPipe.RendererRole.__doc__ = "Raster renderer role"
QgsRasterPipe.BrightnessRole = Qgis.RasterPipeInterfaceRole.Brightness
QgsRasterPipe.Role.BrightnessRole = Qgis.RasterPipeInterfaceRole.Brightness
QgsRasterPipe.BrightnessRole.is_monkey_patched = True
QgsRasterPipe.BrightnessRole.__doc__ = "Brightness filter role"
QgsRasterPipe.ResamplerRole = Qgis.RasterPipeInterfaceRole.Resampler
QgsRasterPipe.Role.ResamplerRole = Qgis.RasterPipeInterfaceRole.Resampler
QgsRasterPipe.ResamplerRole.is_monkey_patched = True
QgsRasterPipe.ResamplerRole.__doc__ = "Resampler role"
QgsRasterPipe.ProjectorRole = Qgis.RasterPipeInterfaceRole.Projector
QgsRasterPipe.Role.ProjectorRole = Qgis.RasterPipeInterfaceRole.Projector
QgsRasterPipe.ProjectorRole.is_monkey_patched = True
QgsRasterPipe.ProjectorRole.__doc__ = "Projector role"
QgsRasterPipe.NullerRole = Qgis.RasterPipeInterfaceRole.Nuller
QgsRasterPipe.Role.NullerRole = Qgis.RasterPipeInterfaceRole.Nuller
QgsRasterPipe.NullerRole.is_monkey_patched = True
QgsRasterPipe.NullerRole.__doc__ = "Raster nuller role"
QgsRasterPipe.HueSaturationRole = Qgis.RasterPipeInterfaceRole.HueSaturation
QgsRasterPipe.Role.HueSaturationRole = Qgis.RasterPipeInterfaceRole.HueSaturation
QgsRasterPipe.HueSaturationRole.is_monkey_patched = True
QgsRasterPipe.HueSaturationRole.__doc__ = "Hue/saturation filter role (also applies grayscale/color inversion)"
Qgis.RasterPipeInterfaceRole.__doc__ = "Raster pipe interface roles.\n\n.. versionadded:: 3.22\n\n" + '* ``UnknownRole``: ' + Qgis.RasterPipeInterfaceRole.Unknown.__doc__ + '\n' + '* ``ProviderRole``: ' + Qgis.RasterPipeInterfaceRole.Provider.__doc__ + '\n' + '* ``RendererRole``: ' + Qgis.RasterPipeInterfaceRole.Renderer.__doc__ + '\n' + '* ``BrightnessRole``: ' + Qgis.RasterPipeInterfaceRole.Brightness.__doc__ + '\n' + '* ``ResamplerRole``: ' + Qgis.RasterPipeInterfaceRole.Resampler.__doc__ + '\n' + '* ``ProjectorRole``: ' + Qgis.RasterPipeInterfaceRole.Projector.__doc__ + '\n' + '* ``NullerRole``: ' + Qgis.RasterPipeInterfaceRole.Nuller.__doc__ + '\n' + '* ``HueSaturationRole``: ' + Qgis.RasterPipeInterfaceRole.HueSaturation.__doc__
# --
Qgis.RasterPipeInterfaceRole.baseClass = Qgis
QgsRasterPipe.ResamplingStage = Qgis.RasterResamplingStage
# monkey patching scoped based enum
QgsRasterPipe.ResampleFilter = Qgis.RasterResamplingStage.ResampleFilter
QgsRasterPipe.ResampleFilter.is_monkey_patched = True
QgsRasterPipe.ResampleFilter.__doc__ = ""
QgsRasterPipe.Provider = Qgis.RasterResamplingStage.Provider
QgsRasterPipe.Provider.is_monkey_patched = True
QgsRasterPipe.Provider.__doc__ = ""
Qgis.RasterResamplingStage.__doc__ = "Stage at which raster resampling occurs.\n\n.. versionadded:: 3.22\n\n" + '* ``ResampleFilter``: ' + Qgis.RasterResamplingStage.ResampleFilter.__doc__ + '\n' + '* ``Provider``: ' + Qgis.RasterResamplingStage.Provider.__doc__
# --
Qgis.RasterResamplingStage.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RasterRendererFlag.InternalLayerOpacityHandling.__doc__ = "The renderer internally handles the raster layer's opacity, so the default layer level opacity handling should not be applied."
Qgis.RasterRendererFlag.__doc__ = "Flags which control behavior of raster renderers.\n\n.. versionadded:: 3.28\n\n" + '* ``InternalLayerOpacityHandling``: ' + Qgis.RasterRendererFlag.InternalLayerOpacityHandling.__doc__
# --
Qgis.RasterRendererFlags = lambda flags=0: Qgis.RasterRendererFlag(flags)
Qgis.RasterRendererFlag.baseClass = Qgis
Qgis.RasterRendererFlags.baseClass = Qgis
RasterRendererFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.RasterRendererCapability.UsesMultipleBands.__doc__ = "The renderer utilizes multiple raster bands for color data (note that alpha bands are not considered for this capability)"
Qgis.RasterRendererCapability.__doc__ = "Raster renderer capabilities.\n\n.. versionadded:: 3.48\n\n" + '* ``UsesMultipleBands``: ' + Qgis.RasterRendererCapability.UsesMultipleBands.__doc__
# --
Qgis.RasterRendererCapability.baseClass = Qgis
Qgis.RasterRendererCapabilities = lambda flags=0: Qgis.RasterRendererCapability(flags)
Qgis.RasterRendererCapabilities.baseClass = Qgis
RasterRendererCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.RasterAttributeTableFieldUsage.Generic.__doc__ = "Field usage Generic"
Qgis.RasterAttributeTableFieldUsage.PixelCount.__doc__ = "Field usage PixelCount"
Qgis.RasterAttributeTableFieldUsage.Name.__doc__ = "Field usage Name"
Qgis.RasterAttributeTableFieldUsage.Min.__doc__ = "Field usage Min"
Qgis.RasterAttributeTableFieldUsage.Max.__doc__ = "Field usage Max"
Qgis.RasterAttributeTableFieldUsage.MinMax.__doc__ = "Field usage MinMax"
Qgis.RasterAttributeTableFieldUsage.Red.__doc__ = "Field usage Red"
Qgis.RasterAttributeTableFieldUsage.Green.__doc__ = "Field usage Green"
Qgis.RasterAttributeTableFieldUsage.Blue.__doc__ = "Field usage Blue"
Qgis.RasterAttributeTableFieldUsage.Alpha.__doc__ = "Field usage Alpha"
Qgis.RasterAttributeTableFieldUsage.RedMin.__doc__ = "Field usage RedMin"
Qgis.RasterAttributeTableFieldUsage.GreenMin.__doc__ = "Field usage GreenMin"
Qgis.RasterAttributeTableFieldUsage.BlueMin.__doc__ = "Field usage BlueMin"
Qgis.RasterAttributeTableFieldUsage.AlphaMin.__doc__ = "Field usage AlphaMin"
Qgis.RasterAttributeTableFieldUsage.RedMax.__doc__ = "Field usage RedMax"
Qgis.RasterAttributeTableFieldUsage.GreenMax.__doc__ = "Field usage GreenMax"
Qgis.RasterAttributeTableFieldUsage.BlueMax.__doc__ = "Field usage BlueMax"
Qgis.RasterAttributeTableFieldUsage.AlphaMax.__doc__ = "Field usage AlphaMax"
Qgis.RasterAttributeTableFieldUsage.MaxCount.__doc__ = "Not used by QGIS: GDAL Maximum GFU value (equals to GFU_AlphaMax+1 currently)"
Qgis.RasterAttributeTableFieldUsage.__doc__ = "The RasterAttributeTableFieldUsage enum represents the usage of a Raster Attribute Table field.\n\n.. note::\n\n   Directly mapped from GDALRATFieldUsage enum values.\n\n.. versionadded:: 3.30\n\n" + '* ``Generic``: ' + Qgis.RasterAttributeTableFieldUsage.Generic.__doc__ + '\n' + '* ``PixelCount``: ' + Qgis.RasterAttributeTableFieldUsage.PixelCount.__doc__ + '\n' + '* ``Name``: ' + Qgis.RasterAttributeTableFieldUsage.Name.__doc__ + '\n' + '* ``Min``: ' + Qgis.RasterAttributeTableFieldUsage.Min.__doc__ + '\n' + '* ``Max``: ' + Qgis.RasterAttributeTableFieldUsage.Max.__doc__ + '\n' + '* ``MinMax``: ' + Qgis.RasterAttributeTableFieldUsage.MinMax.__doc__ + '\n' + '* ``Red``: ' + Qgis.RasterAttributeTableFieldUsage.Red.__doc__ + '\n' + '* ``Green``: ' + Qgis.RasterAttributeTableFieldUsage.Green.__doc__ + '\n' + '* ``Blue``: ' + Qgis.RasterAttributeTableFieldUsage.Blue.__doc__ + '\n' + '* ``Alpha``: ' + Qgis.RasterAttributeTableFieldUsage.Alpha.__doc__ + '\n' + '* ``RedMin``: ' + Qgis.RasterAttributeTableFieldUsage.RedMin.__doc__ + '\n' + '* ``GreenMin``: ' + Qgis.RasterAttributeTableFieldUsage.GreenMin.__doc__ + '\n' + '* ``BlueMin``: ' + Qgis.RasterAttributeTableFieldUsage.BlueMin.__doc__ + '\n' + '* ``AlphaMin``: ' + Qgis.RasterAttributeTableFieldUsage.AlphaMin.__doc__ + '\n' + '* ``RedMax``: ' + Qgis.RasterAttributeTableFieldUsage.RedMax.__doc__ + '\n' + '* ``GreenMax``: ' + Qgis.RasterAttributeTableFieldUsage.GreenMax.__doc__ + '\n' + '* ``BlueMax``: ' + Qgis.RasterAttributeTableFieldUsage.BlueMax.__doc__ + '\n' + '* ``AlphaMax``: ' + Qgis.RasterAttributeTableFieldUsage.AlphaMax.__doc__ + '\n' + '* ``MaxCount``: ' + Qgis.RasterAttributeTableFieldUsage.MaxCount.__doc__
# --
Qgis.RasterAttributeTableFieldUsage.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RasterAttributeTableType.Thematic.__doc__ = ""
Qgis.RasterAttributeTableType.Athematic.__doc__ = ""
Qgis.RasterAttributeTableType.__doc__ = "The RasterAttributeTableType enum represents the type of RAT.\nnote Directly mapped from GDALRATTableType enum values.\n\n.. versionadded:: 3.30\n\n" + '* ``Thematic``: ' + Qgis.RasterAttributeTableType.Thematic.__doc__ + '\n' + '* ``Athematic``: ' + Qgis.RasterAttributeTableType.Athematic.__doc__
# --
Qgis.RasterAttributeTableType.baseClass = Qgis
QgsRasterFileWriter.Mode = Qgis.RasterExportType
# monkey patching scoped based enum
QgsRasterFileWriter.Raw = Qgis.RasterExportType.Raw
QgsRasterFileWriter.Raw.is_monkey_patched = True
QgsRasterFileWriter.Raw.__doc__ = "Raw data"
QgsRasterFileWriter.Image = Qgis.RasterExportType.RenderedImage
QgsRasterFileWriter.Mode.Image = Qgis.RasterExportType.RenderedImage
QgsRasterFileWriter.Image.is_monkey_patched = True
QgsRasterFileWriter.Image.__doc__ = "Rendered image"
Qgis.RasterExportType.__doc__ = "Raster file export types.\n\nPrior to QGIS 3.32 this was available as :py:class:`QgsRasterFileWriter`.Mode\n\n.. versionadded:: 3.32\n\n" + '* ``Raw``: ' + Qgis.RasterExportType.Raw.__doc__ + '\n' + '* ``Image``: ' + Qgis.RasterExportType.RenderedImage.__doc__
# --
Qgis.RasterExportType.baseClass = Qgis
QgsRasterFileWriter.WriterError = Qgis.RasterFileWriterResult
# monkey patching scoped based enum
QgsRasterFileWriter.NoError = Qgis.RasterFileWriterResult.Success
QgsRasterFileWriter.WriterError.NoError = Qgis.RasterFileWriterResult.Success
QgsRasterFileWriter.NoError.is_monkey_patched = True
QgsRasterFileWriter.NoError.__doc__ = "Successful export"
QgsRasterFileWriter.SourceProviderError = Qgis.RasterFileWriterResult.SourceProviderError
QgsRasterFileWriter.SourceProviderError.is_monkey_patched = True
QgsRasterFileWriter.SourceProviderError.__doc__ = "Source data provider error"
QgsRasterFileWriter.DestProviderError = Qgis.RasterFileWriterResult.DestinationProviderError
QgsRasterFileWriter.WriterError.DestProviderError = Qgis.RasterFileWriterResult.DestinationProviderError
QgsRasterFileWriter.DestProviderError.is_monkey_patched = True
QgsRasterFileWriter.DestProviderError.__doc__ = "Destination data provider error"
QgsRasterFileWriter.CreateDatasourceError = Qgis.RasterFileWriterResult.CreateDatasourceError
QgsRasterFileWriter.CreateDatasourceError.is_monkey_patched = True
QgsRasterFileWriter.CreateDatasourceError.__doc__ = "Data source creation error"
QgsRasterFileWriter.WriteError = Qgis.RasterFileWriterResult.WriteError
QgsRasterFileWriter.WriteError.is_monkey_patched = True
QgsRasterFileWriter.WriteError.__doc__ = "Write error"
QgsRasterFileWriter.NoDataConflict = Qgis.RasterFileWriterResult.NoDataConflict
QgsRasterFileWriter.NoDataConflict.is_monkey_patched = True
QgsRasterFileWriter.NoDataConflict.__doc__ = "Internal error if a value used for 'no data' was found in input"
QgsRasterFileWriter.WriteCanceled = Qgis.RasterFileWriterResult.Canceled
QgsRasterFileWriter.WriterError.WriteCanceled = Qgis.RasterFileWriterResult.Canceled
QgsRasterFileWriter.WriteCanceled.is_monkey_patched = True
QgsRasterFileWriter.WriteCanceled.__doc__ = "Writing was manually canceled"
Qgis.RasterFileWriterResult.__doc__ = "Raster file export results.\n\nPrior to QGIS 3.32 this was available as :py:class:`QgsRasterFileWriter`.WriterError\n\n.. versionadded:: 3.32\n\n" + '* ``NoError``: ' + Qgis.RasterFileWriterResult.Success.__doc__ + '\n' + '* ``SourceProviderError``: ' + Qgis.RasterFileWriterResult.SourceProviderError.__doc__ + '\n' + '* ``DestProviderError``: ' + Qgis.RasterFileWriterResult.DestinationProviderError.__doc__ + '\n' + '* ``CreateDatasourceError``: ' + Qgis.RasterFileWriterResult.CreateDatasourceError.__doc__ + '\n' + '* ``WriteError``: ' + Qgis.RasterFileWriterResult.WriteError.__doc__ + '\n' + '* ``NoDataConflict``: ' + Qgis.RasterFileWriterResult.NoDataConflict.__doc__ + '\n' + '* ``WriteCanceled``: ' + Qgis.RasterFileWriterResult.Canceled.__doc__
# --
Qgis.RasterFileWriterResult.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MeshEditingErrorType.NoError.__doc__ = "No type"
Qgis.MeshEditingErrorType.InvalidFace.__doc__ = "An error occurs due to an invalid face (for example, vertex indexes are unordered)"
Qgis.MeshEditingErrorType.TooManyVerticesInFace.__doc__ = "A face has more vertices than the maximum number supported per face"
Qgis.MeshEditingErrorType.FlatFace.__doc__ = "A flat face is present"
Qgis.MeshEditingErrorType.UniqueSharedVertex.__doc__ = "A least two faces share only one vertices"
Qgis.MeshEditingErrorType.InvalidVertex.__doc__ = "An error occurs due to an invalid vertex (for example, vertex index is out of range the available vertex)"
Qgis.MeshEditingErrorType.ManifoldFace.__doc__ = "ManifoldFace"
Qgis.MeshEditingErrorType.__doc__ = "Type of error that can occur during mesh frame editing.\n\n.. versionadded:: 3.22\n\n" + '* ``NoError``: ' + Qgis.MeshEditingErrorType.NoError.__doc__ + '\n' + '* ``InvalidFace``: ' + Qgis.MeshEditingErrorType.InvalidFace.__doc__ + '\n' + '* ``TooManyVerticesInFace``: ' + Qgis.MeshEditingErrorType.TooManyVerticesInFace.__doc__ + '\n' + '* ``FlatFace``: ' + Qgis.MeshEditingErrorType.FlatFace.__doc__ + '\n' + '* ``UniqueSharedVertex``: ' + Qgis.MeshEditingErrorType.UniqueSharedVertex.__doc__ + '\n' + '* ``InvalidVertex``: ' + Qgis.MeshEditingErrorType.InvalidVertex.__doc__ + '\n' + '* ``ManifoldFace``: ' + Qgis.MeshEditingErrorType.ManifoldFace.__doc__
# --
Qgis.MeshEditingErrorType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FilePathType.Absolute.__doc__ = "Absolute path"
Qgis.FilePathType.Relative.__doc__ = "Relative path"
Qgis.FilePathType.__doc__ = "File path types.\n\n.. versionadded:: 3.22\n\n" + '* ``Absolute``: ' + Qgis.FilePathType.Absolute.__doc__ + '\n' + '* ``Relative``: ' + Qgis.FilePathType.Relative.__doc__
# --
Qgis.FilePathType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SublayerPromptMode.AlwaysAsk.__doc__ = "Always ask users to select from available sublayers, if sublayers are present"
Qgis.SublayerPromptMode.AskExcludingRasterBands.__doc__ = "Ask users to select from available sublayers, unless only raster bands are present"
Qgis.SublayerPromptMode.NeverAskSkip.__doc__ = "Never ask users to select sublayers, instead don't load anything"
Qgis.SublayerPromptMode.NeverAskLoadAll.__doc__ = "Never ask users to select sublayers, instead automatically load all available sublayers"
Qgis.SublayerPromptMode.__doc__ = "Specifies how to handle layer sources with multiple sublayers.\n\n.. versionadded:: 3.22\n\n" + '* ``AlwaysAsk``: ' + Qgis.SublayerPromptMode.AlwaysAsk.__doc__ + '\n' + '* ``AskExcludingRasterBands``: ' + Qgis.SublayerPromptMode.AskExcludingRasterBands.__doc__ + '\n' + '* ``NeverAskSkip``: ' + Qgis.SublayerPromptMode.NeverAskSkip.__doc__ + '\n' + '* ``NeverAskLoadAll``: ' + Qgis.SublayerPromptMode.NeverAskLoadAll.__doc__
# --
Qgis.SublayerPromptMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldConfigurationFlag.NoFlag.__doc__ = "No flag is defined"
Qgis.FieldConfigurationFlag.NotSearchable.__doc__ = "Defines if the field is searchable (used in the locator search for instance)"
Qgis.FieldConfigurationFlag.HideFromWms.__doc__ = "Field is not available if layer is served as WMS from QGIS server"
Qgis.FieldConfigurationFlag.HideFromWfs.__doc__ = "Field is not available if layer is served as WFS from QGIS server"
Qgis.FieldConfigurationFlag.__doc__ = "Configuration flags for fields\nThese flags are meant to be user-configurable\nand are not describing any information from the data provider.\n\n.. note::\n\n   FieldConfigurationFlag are expressed in the negative forms so that default flags is NoFlag.\n\n.. versionadded:: 3.34\n\n" + '* ``NoFlag``: ' + Qgis.FieldConfigurationFlag.NoFlag.__doc__ + '\n' + '* ``NotSearchable``: ' + Qgis.FieldConfigurationFlag.NotSearchable.__doc__ + '\n' + '* ``HideFromWms``: ' + Qgis.FieldConfigurationFlag.HideFromWms.__doc__ + '\n' + '* ``HideFromWfs``: ' + Qgis.FieldConfigurationFlag.HideFromWfs.__doc__
# --
Qgis.FieldConfigurationFlag.baseClass = Qgis
Qgis.FieldConfigurationFlags = lambda flags=0: Qgis.FieldConfigurationFlag(flags)
Qgis.FieldConfigurationFlags.baseClass = Qgis
FieldConfigurationFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.FieldMetadataProperty.GeometryCrs.__doc__ = "Available for geometry field types with a specific associated coordinate reference system (as a QgsCoordinateReferenceSystem value)"
Qgis.FieldMetadataProperty.GeometryWkbType.__doc__ = "Available for geometry field types which accept geometries of a specific WKB type only (as a QgsWkbTypes.Type value)"
Qgis.FieldMetadataProperty.CustomProperty.__doc__ = "Starting point for custom user set properties"
Qgis.FieldMetadataProperty.__doc__ = "Standard field metadata values.\n\n.. versionadded:: 3.30\n\n" + '* ``GeometryCrs``: ' + Qgis.FieldMetadataProperty.GeometryCrs.__doc__ + '\n' + '* ``GeometryWkbType``: ' + Qgis.FieldMetadataProperty.GeometryWkbType.__doc__ + '\n' + '* ``CustomProperty``: ' + Qgis.FieldMetadataProperty.CustomProperty.__doc__
# --
Qgis.FieldMetadataProperty.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SelectionRenderingMode.Default.__doc__ = "Use default symbol and selection colors"
Qgis.SelectionRenderingMode.CustomColor.__doc__ = "Use default symbol with a custom selection color"
Qgis.SelectionRenderingMode.CustomSymbol.__doc__ = "Use a custom symbol"
Qgis.SelectionRenderingMode.__doc__ = "Specifies how a selection should be rendered.\n\n.. versionadded:: 3.34\n\n" + '* ``Default``: ' + Qgis.SelectionRenderingMode.Default.__doc__ + '\n' + '* ``CustomColor``: ' + Qgis.SelectionRenderingMode.CustomColor.__doc__ + '\n' + '* ``CustomSymbol``: ' + Qgis.SelectionRenderingMode.CustomSymbol.__doc__
# --
Qgis.SelectionRenderingMode.baseClass = Qgis
QgsVectorLayer.SelectBehavior = Qgis.SelectBehavior
# monkey patching scoped based enum
QgsVectorLayer.SetSelection = Qgis.SelectBehavior.SetSelection
QgsVectorLayer.SetSelection.is_monkey_patched = True
QgsVectorLayer.SetSelection.__doc__ = "Set selection, removing any existing selection"
QgsVectorLayer.AddToSelection = Qgis.SelectBehavior.AddToSelection
QgsVectorLayer.AddToSelection.is_monkey_patched = True
QgsVectorLayer.AddToSelection.__doc__ = "Add selection to current selection"
QgsVectorLayer.IntersectSelection = Qgis.SelectBehavior.IntersectSelection
QgsVectorLayer.IntersectSelection.is_monkey_patched = True
QgsVectorLayer.IntersectSelection.__doc__ = "Modify current selection to include only select features which match"
QgsVectorLayer.RemoveFromSelection = Qgis.SelectBehavior.RemoveFromSelection
QgsVectorLayer.RemoveFromSelection.is_monkey_patched = True
QgsVectorLayer.RemoveFromSelection.__doc__ = "Remove from current selection"
Qgis.SelectBehavior.__doc__ = "Specifies how a selection should be applied.\n\n.. versionadded:: 3.22\n\n" + '* ``SetSelection``: ' + Qgis.SelectBehavior.SetSelection.__doc__ + '\n' + '* ``AddToSelection``: ' + Qgis.SelectBehavior.AddToSelection.__doc__ + '\n' + '* ``IntersectSelection``: ' + Qgis.SelectBehavior.IntersectSelection.__doc__ + '\n' + '* ``RemoveFromSelection``: ' + Qgis.SelectBehavior.RemoveFromSelection.__doc__
# --
Qgis.SelectBehavior.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SelectGeometryRelationship.Intersect.__doc__ = "Select where features intersect the reference geometry"
Qgis.SelectGeometryRelationship.Within.__doc__ = "Select where features are within the reference geometry"
Qgis.SelectGeometryRelationship.__doc__ = "Geometry relationship test to apply for selecting features.\n\n.. versionadded:: 3.28\n\n" + '* ``Intersect``: ' + Qgis.SelectGeometryRelationship.Intersect.__doc__ + '\n' + '* ``Within``: ' + Qgis.SelectGeometryRelationship.Within.__doc__
# --
Qgis.SelectGeometryRelationship.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SelectionFlag.SingleFeatureSelection.__doc__ = "Select only a single feature, picking the \"best\" match for the selection geometry"
Qgis.SelectionFlag.ToggleSelection.__doc__ = "Enables a \"toggle\" selection mode, where previously selected matching features will be deselected and previously deselected features will be selected. This flag works only when the SingleFeatureSelection flag is also set."
Qgis.SelectionFlag.__doc__ = "Flags which control feature selection behavior.\n\n.. versionadded:: 3.28\n\n" + '* ``SingleFeatureSelection``: ' + Qgis.SelectionFlag.SingleFeatureSelection.__doc__ + '\n' + '* ``ToggleSelection``: ' + Qgis.SelectionFlag.ToggleSelection.__doc__
# --
Qgis.SelectionFlags = lambda flags=0: Qgis.SelectionFlag(flags)
Qgis.SelectionFlag.baseClass = Qgis
Qgis.SelectionFlags.baseClass = Qgis
SelectionFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsVectorLayer.EditResult = Qgis.VectorEditResult
# monkey patching scoped based enum
QgsVectorLayer.Success = Qgis.VectorEditResult.Success
QgsVectorLayer.Success.is_monkey_patched = True
QgsVectorLayer.Success.__doc__ = "Edit operation was successful"
QgsVectorLayer.EmptyGeometry = Qgis.VectorEditResult.EmptyGeometry
QgsVectorLayer.EmptyGeometry.is_monkey_patched = True
QgsVectorLayer.EmptyGeometry.__doc__ = "Edit operation resulted in an empty geometry"
QgsVectorLayer.EditFailed = Qgis.VectorEditResult.EditFailed
QgsVectorLayer.EditFailed.is_monkey_patched = True
QgsVectorLayer.EditFailed.__doc__ = "Edit operation failed"
QgsVectorLayer.FetchFeatureFailed = Qgis.VectorEditResult.FetchFeatureFailed
QgsVectorLayer.FetchFeatureFailed.is_monkey_patched = True
QgsVectorLayer.FetchFeatureFailed.__doc__ = "Unable to fetch requested feature"
QgsVectorLayer.InvalidLayer = Qgis.VectorEditResult.InvalidLayer
QgsVectorLayer.InvalidLayer.is_monkey_patched = True
QgsVectorLayer.InvalidLayer.__doc__ = "Edit failed due to invalid layer"
Qgis.VectorEditResult.__doc__ = "Specifies the result of a vector layer edit operation\n\n.. versionadded:: 3.22\n\n" + '* ``Success``: ' + Qgis.VectorEditResult.Success.__doc__ + '\n' + '* ``EmptyGeometry``: ' + Qgis.VectorEditResult.EmptyGeometry.__doc__ + '\n' + '* ``EditFailed``: ' + Qgis.VectorEditResult.EditFailed.__doc__ + '\n' + '* ``FetchFeatureFailed``: ' + Qgis.VectorEditResult.FetchFeatureFailed.__doc__ + '\n' + '* ``InvalidLayer``: ' + Qgis.VectorEditResult.InvalidLayer.__doc__
# --
Qgis.VectorEditResult.baseClass = Qgis
QgsSymbolLayerUtils.VertexMarkerType = Qgis.VertexMarkerType
# monkey patching scoped based enum
QgsSymbolLayerUtils.SemiTransparentCircle = Qgis.VertexMarkerType.SemiTransparentCircle
QgsSymbolLayerUtils.SemiTransparentCircle.is_monkey_patched = True
QgsSymbolLayerUtils.SemiTransparentCircle.__doc__ = "Semi-transparent circle marker"
QgsSymbolLayerUtils.Cross = Qgis.VertexMarkerType.Cross
QgsSymbolLayerUtils.Cross.is_monkey_patched = True
QgsSymbolLayerUtils.Cross.__doc__ = "Cross marker"
QgsSymbolLayerUtils.NoMarker = Qgis.VertexMarkerType.NoMarker
QgsSymbolLayerUtils.NoMarker.is_monkey_patched = True
QgsSymbolLayerUtils.NoMarker.__doc__ = "No marker"
Qgis.VertexMarkerType.__doc__ = "Editing vertex markers, used for showing vertices during a edit operation.\n\n.. versionadded:: 3.22\n\n" + '* ``SemiTransparentCircle``: ' + Qgis.VertexMarkerType.SemiTransparentCircle.__doc__ + '\n' + '* ``Cross``: ' + Qgis.VertexMarkerType.Cross.__doc__ + '\n' + '* ``NoMarker``: ' + Qgis.VertexMarkerType.NoMarker.__doc__
# --
Qgis.VertexMarkerType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ContentStatus.NotStarted.__doc__ = "Content fetching/storing has not started yet"
Qgis.ContentStatus.Running.__doc__ = "Content fetching/storing is in progress"
Qgis.ContentStatus.Finished.__doc__ = "Content fetching/storing is finished and successful"
Qgis.ContentStatus.Failed.__doc__ = "Content fetching/storing has failed"
Qgis.ContentStatus.Canceled.__doc__ = "Content fetching/storing has been canceled"
Qgis.ContentStatus.__doc__ = "Status for fetched or stored content\n\n.. versionadded:: 3.22\n\n" + '* ``NotStarted``: ' + Qgis.ContentStatus.NotStarted.__doc__ + '\n' + '* ``Running``: ' + Qgis.ContentStatus.Running.__doc__ + '\n' + '* ``Finished``: ' + Qgis.ContentStatus.Finished.__doc__ + '\n' + '* ``Failed``: ' + Qgis.ContentStatus.Failed.__doc__ + '\n' + '* ``Canceled``: ' + Qgis.ContentStatus.Canceled.__doc__
# --
Qgis.ContentStatus.baseClass = Qgis
# monkey patching scoped based enum
Qgis.GpsConnectionType.Automatic.__doc__ = "Automatically detected GPS device connection"
Qgis.GpsConnectionType.Internal.__doc__ = "Internal GPS device"
Qgis.GpsConnectionType.Serial.__doc__ = "Serial port GPS device"
Qgis.GpsConnectionType.Gpsd.__doc__ = "GPSD device"
Qgis.GpsConnectionType.__doc__ = "GPS connection types.\n\n.. versionadded:: 3.30\n\n" + '* ``Automatic``: ' + Qgis.GpsConnectionType.Automatic.__doc__ + '\n' + '* ``Internal``: ' + Qgis.GpsConnectionType.Internal.__doc__ + '\n' + '* ``Serial``: ' + Qgis.GpsConnectionType.Serial.__doc__ + '\n' + '* ``Gpsd``: ' + Qgis.GpsConnectionType.Gpsd.__doc__
# --
Qgis.GpsConnectionType.baseClass = Qgis
Qgis.GpsConnectionStatus = Qgis.DeviceConnectionStatus
# monkey patching scoped based enum
Qgis.Disconnected = Qgis.DeviceConnectionStatus.Disconnected
Qgis.Disconnected.is_monkey_patched = True
Qgis.Disconnected.__doc__ = "Device is disconnected"
Qgis.Connecting = Qgis.DeviceConnectionStatus.Connecting
Qgis.Connecting.is_monkey_patched = True
Qgis.Connecting.__doc__ = "Device is connecting"
Qgis.Connected = Qgis.DeviceConnectionStatus.Connected
Qgis.Connected.is_monkey_patched = True
Qgis.Connected.__doc__ = "Device is successfully connected"
Qgis.DeviceConnectionStatus.__doc__ = "GPS connection status.\n\n.. versionadded:: 3.30\n\n" + '* ``Disconnected``: ' + Qgis.DeviceConnectionStatus.Disconnected.__doc__ + '\n' + '* ``Connecting``: ' + Qgis.DeviceConnectionStatus.Connecting.__doc__ + '\n' + '* ``Connected``: ' + Qgis.DeviceConnectionStatus.Connected.__doc__
# --
Qgis.DeviceConnectionStatus.baseClass = Qgis
QgsGpsInformation.FixStatus = Qgis.GpsFixStatus
# monkey patching scoped based enum
QgsGpsInformation.NoData = Qgis.GpsFixStatus.NoData
QgsGpsInformation.NoData.is_monkey_patched = True
QgsGpsInformation.NoData.__doc__ = "No fix data available"
QgsGpsInformation.NoFix = Qgis.GpsFixStatus.NoFix
QgsGpsInformation.NoFix.is_monkey_patched = True
QgsGpsInformation.NoFix.__doc__ = "GPS is not fixed"
QgsGpsInformation.Fix2D = Qgis.GpsFixStatus.Fix2D
QgsGpsInformation.Fix2D.is_monkey_patched = True
QgsGpsInformation.Fix2D.__doc__ = "2D fix"
QgsGpsInformation.Fix3D = Qgis.GpsFixStatus.Fix3D
QgsGpsInformation.Fix3D.is_monkey_patched = True
QgsGpsInformation.Fix3D.__doc__ = "3D fix"
Qgis.GpsFixStatus.__doc__ = "GPS fix status.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsGpsInformation`.FixStatus\n\n.. versionadded:: 3.30\n\n" + '* ``NoData``: ' + Qgis.GpsFixStatus.NoData.__doc__ + '\n' + '* ``NoFix``: ' + Qgis.GpsFixStatus.NoFix.__doc__ + '\n' + '* ``Fix2D``: ' + Qgis.GpsFixStatus.Fix2D.__doc__ + '\n' + '* ``Fix3D``: ' + Qgis.GpsFixStatus.Fix3D.__doc__
# --
Qgis.GpsFixStatus.baseClass = Qgis
# monkey patching scoped based enum
Qgis.GnssConstellation.Unknown.__doc__ = "Unknown/other system"
Qgis.GnssConstellation.Gps.__doc__ = "Global Positioning System (GPS)"
Qgis.GnssConstellation.Glonass.__doc__ = "Global Navigation Satellite System (GLONASS)"
Qgis.GnssConstellation.Galileo.__doc__ = "Galileo"
Qgis.GnssConstellation.BeiDou.__doc__ = "BeiDou"
Qgis.GnssConstellation.Qzss.__doc__ = "Quasi Zenith Satellite System (QZSS)"
Qgis.GnssConstellation.Navic.__doc__ = "Indian Regional Navigation Satellite System (IRNSS) / NAVIC"
Qgis.GnssConstellation.Sbas.__doc__ = "SBAS"
Qgis.GnssConstellation.__doc__ = "GNSS constellation\n\n.. versionadded:: 3.30\n\n" + '* ``Unknown``: ' + Qgis.GnssConstellation.Unknown.__doc__ + '\n' + '* ``Gps``: ' + Qgis.GnssConstellation.Gps.__doc__ + '\n' + '* ``Glonass``: ' + Qgis.GnssConstellation.Glonass.__doc__ + '\n' + '* ``Galileo``: ' + Qgis.GnssConstellation.Galileo.__doc__ + '\n' + '* ``BeiDou``: ' + Qgis.GnssConstellation.BeiDou.__doc__ + '\n' + '* ``Qzss``: ' + Qgis.GnssConstellation.Qzss.__doc__ + '\n' + '* ``Navic``: ' + Qgis.GnssConstellation.Navic.__doc__ + '\n' + '* ``Sbas``: ' + Qgis.GnssConstellation.Sbas.__doc__
# --
Qgis.GnssConstellation.baseClass = Qgis
# monkey patching scoped based enum
Qgis.GpsQualityIndicator.Unknown.__doc__ = "Unknown"
Qgis.GpsQualityIndicator.Invalid.__doc__ = "Invalid"
Qgis.GpsQualityIndicator.GPS.__doc__ = "Standalone"
Qgis.GpsQualityIndicator.DGPS.__doc__ = "Differential GPS"
Qgis.GpsQualityIndicator.PPS.__doc__ = "PPS"
Qgis.GpsQualityIndicator.RTK.__doc__ = "Real-time-kynematic"
Qgis.GpsQualityIndicator.FloatRTK.__doc__ = "Float real-time-kynematic"
Qgis.GpsQualityIndicator.Estimated.__doc__ = "Estimated"
Qgis.GpsQualityIndicator.Manual.__doc__ = "Manual input mode"
Qgis.GpsQualityIndicator.Simulation.__doc__ = "Simulation mode"
Qgis.GpsQualityIndicator.__doc__ = "GPS signal quality indicator\n\n.. versionadded:: 3.22.6\n\n" + '* ``Unknown``: ' + Qgis.GpsQualityIndicator.Unknown.__doc__ + '\n' + '* ``Invalid``: ' + Qgis.GpsQualityIndicator.Invalid.__doc__ + '\n' + '* ``GPS``: ' + Qgis.GpsQualityIndicator.GPS.__doc__ + '\n' + '* ``DGPS``: ' + Qgis.GpsQualityIndicator.DGPS.__doc__ + '\n' + '* ``PPS``: ' + Qgis.GpsQualityIndicator.PPS.__doc__ + '\n' + '* ``RTK``: ' + Qgis.GpsQualityIndicator.RTK.__doc__ + '\n' + '* ``FloatRTK``: ' + Qgis.GpsQualityIndicator.FloatRTK.__doc__ + '\n' + '* ``Estimated``: ' + Qgis.GpsQualityIndicator.Estimated.__doc__ + '\n' + '* ``Manual``: ' + Qgis.GpsQualityIndicator.Manual.__doc__ + '\n' + '* ``Simulation``: ' + Qgis.GpsQualityIndicator.Simulation.__doc__
# --
Qgis.GpsQualityIndicator.baseClass = Qgis
# monkey patching scoped based enum
Qgis.GpsInformationComponent.Location.__doc__ = "2D location (latitude/longitude), as a QgsPointXY value"
Qgis.GpsInformationComponent.Altitude.__doc__ = "Altitude/elevation above or below the mean sea level"
Qgis.GpsInformationComponent.GroundSpeed.__doc__ = "Ground speed"
Qgis.GpsInformationComponent.Bearing.__doc__ = "Bearing measured in degrees clockwise from true north to the direction of travel"
Qgis.GpsInformationComponent.TotalTrackLength.__doc__ = "Total distance of current GPS track (available from QgsGpsLogger class only)"
Qgis.GpsInformationComponent.TrackDistanceFromStart.__doc__ = "Direct distance from first vertex in current GPS track to last vertex (available from QgsGpsLogger class only)"
Qgis.GpsInformationComponent.Pdop.__doc__ = "Dilution of precision"
Qgis.GpsInformationComponent.Hdop.__doc__ = "Horizontal dilution of precision"
Qgis.GpsInformationComponent.Vdop.__doc__ = "Vertical dilution of precision"
Qgis.GpsInformationComponent.HorizontalAccuracy.__doc__ = "Horizontal accuracy in meters"
Qgis.GpsInformationComponent.VerticalAccuracy.__doc__ = "Vertical accuracy in meters"
Qgis.GpsInformationComponent.HvAccuracy.__doc__ = "3D RMS"
Qgis.GpsInformationComponent.SatellitesUsed.__doc__ = "Count of satellites used in obtaining the fix"
Qgis.GpsInformationComponent.Timestamp.__doc__ = "Timestamp"
Qgis.GpsInformationComponent.TrackStartTime.__doc__ = "Timestamp at start of current track (available from QgsGpsLogger class only)"
Qgis.GpsInformationComponent.TrackEndTime.__doc__ = "Timestamp at end (current point) of current track (available from QgsGpsLogger class only)"
Qgis.GpsInformationComponent.TrackDistanceSinceLastPoint.__doc__ = "Distance since last recorded location (available from QgsGpsLogger class only)"
Qgis.GpsInformationComponent.TrackTimeSinceLastPoint.__doc__ = "Time since last recorded location (available from QgsGpsLogger class only)"
Qgis.GpsInformationComponent.GeoidalSeparation.__doc__ = "Geoidal separation, the difference between the WGS-84 Earth ellipsoid and mean-sea-level (geoid), \"-\" means mean-sea-level below ellipsoid"
Qgis.GpsInformationComponent.EllipsoidAltitude.__doc__ = "Altitude/elevation above or below the WGS-84 Earth ellipsoid"
Qgis.GpsInformationComponent.__doc__ = "GPS information component.\n\n.. versionadded:: 3.30\n\n" + '* ``Location``: ' + Qgis.GpsInformationComponent.Location.__doc__ + '\n' + '* ``Altitude``: ' + Qgis.GpsInformationComponent.Altitude.__doc__ + '\n' + '* ``GroundSpeed``: ' + Qgis.GpsInformationComponent.GroundSpeed.__doc__ + '\n' + '* ``Bearing``: ' + Qgis.GpsInformationComponent.Bearing.__doc__ + '\n' + '* ``TotalTrackLength``: ' + Qgis.GpsInformationComponent.TotalTrackLength.__doc__ + '\n' + '* ``TrackDistanceFromStart``: ' + Qgis.GpsInformationComponent.TrackDistanceFromStart.__doc__ + '\n' + '* ``Pdop``: ' + Qgis.GpsInformationComponent.Pdop.__doc__ + '\n' + '* ``Hdop``: ' + Qgis.GpsInformationComponent.Hdop.__doc__ + '\n' + '* ``Vdop``: ' + Qgis.GpsInformationComponent.Vdop.__doc__ + '\n' + '* ``HorizontalAccuracy``: ' + Qgis.GpsInformationComponent.HorizontalAccuracy.__doc__ + '\n' + '* ``VerticalAccuracy``: ' + Qgis.GpsInformationComponent.VerticalAccuracy.__doc__ + '\n' + '* ``HvAccuracy``: ' + Qgis.GpsInformationComponent.HvAccuracy.__doc__ + '\n' + '* ``SatellitesUsed``: ' + Qgis.GpsInformationComponent.SatellitesUsed.__doc__ + '\n' + '* ``Timestamp``: ' + Qgis.GpsInformationComponent.Timestamp.__doc__ + '\n' + '* ``TrackStartTime``: ' + Qgis.GpsInformationComponent.TrackStartTime.__doc__ + '\n' + '* ``TrackEndTime``: ' + Qgis.GpsInformationComponent.TrackEndTime.__doc__ + '\n' + '* ``TrackDistanceSinceLastPoint``: ' + Qgis.GpsInformationComponent.TrackDistanceSinceLastPoint.__doc__ + '\n' + '* ``TrackTimeSinceLastPoint``: ' + Qgis.GpsInformationComponent.TrackTimeSinceLastPoint.__doc__ + '\n' + '* ``GeoidalSeparation``: ' + Qgis.GpsInformationComponent.GeoidalSeparation.__doc__ + '\n' + '* ``EllipsoidAltitude``: ' + Qgis.GpsInformationComponent.EllipsoidAltitude.__doc__
# --
Qgis.GpsInformationComponents = lambda flags=0: Qgis.GpsInformationComponent(flags)
Qgis.GpsInformationComponent.baseClass = Qgis
Qgis.GpsInformationComponents.baseClass = Qgis
GpsInformationComponents = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.BabelFormatCapability.Import.__doc__ = "Format supports importing"
Qgis.BabelFormatCapability.Export.__doc__ = "Format supports exporting"
Qgis.BabelFormatCapability.Waypoints.__doc__ = "Format supports waypoints"
Qgis.BabelFormatCapability.Routes.__doc__ = "Format supports routes"
Qgis.BabelFormatCapability.Tracks.__doc__ = "Format supports tracks"
Qgis.BabelFormatCapability.__doc__ = "Babel GPS format capabilities.\n\n.. versionadded:: 3.22\n\n" + '* ``Import``: ' + Qgis.BabelFormatCapability.Import.__doc__ + '\n' + '* ``Export``: ' + Qgis.BabelFormatCapability.Export.__doc__ + '\n' + '* ``Waypoints``: ' + Qgis.BabelFormatCapability.Waypoints.__doc__ + '\n' + '* ``Routes``: ' + Qgis.BabelFormatCapability.Routes.__doc__ + '\n' + '* ``Tracks``: ' + Qgis.BabelFormatCapability.Tracks.__doc__
# --
Qgis.BabelFormatCapabilities = lambda flags=0: Qgis.BabelFormatCapability(flags)
Qgis.BabelFormatCapability.baseClass = Qgis
Qgis.BabelFormatCapabilities.baseClass = Qgis
BabelFormatCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.BabelCommandFlag.QuoteFilePaths.__doc__ = "File paths should be enclosed in quotations and escaped"
Qgis.BabelCommandFlag.__doc__ = "Babel command flags, which control how commands and arguments\nare generated for executing GPSBabel processes.\n\n.. versionadded:: 3.22\n\n" + '* ``QuoteFilePaths``: ' + Qgis.BabelCommandFlag.QuoteFilePaths.__doc__
# --
Qgis.BabelCommandFlags = lambda flags=0: Qgis.BabelCommandFlag(flags)
Qgis.BabelCommandFlag.baseClass = Qgis
Qgis.BabelCommandFlags.baseClass = Qgis
BabelCommandFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.GpsFeatureType.Waypoint.__doc__ = "Waypoint"
Qgis.GpsFeatureType.Route.__doc__ = "Route"
Qgis.GpsFeatureType.Track.__doc__ = "Track"
Qgis.GpsFeatureType.__doc__ = "GPS feature types.\n\n.. versionadded:: 3.22\n\n" + '* ``Waypoint``: ' + Qgis.GpsFeatureType.Waypoint.__doc__ + '\n' + '* ``Route``: ' + Qgis.GpsFeatureType.Route.__doc__ + '\n' + '* ``Track``: ' + Qgis.GpsFeatureType.Track.__doc__
# --
Qgis.GpsFeatureType.baseClass = Qgis
QgsGeometry.OperationResult = Qgis.GeometryOperationResult
# monkey patching scoped based enum
QgsGeometry.Success = Qgis.GeometryOperationResult.Success
QgsGeometry.Success.is_monkey_patched = True
QgsGeometry.Success.__doc__ = "Operation succeeded"
QgsGeometry.NothingHappened = Qgis.GeometryOperationResult.NothingHappened
QgsGeometry.NothingHappened.is_monkey_patched = True
QgsGeometry.NothingHappened.__doc__ = "Nothing happened, without any error"
QgsGeometry.InvalidBaseGeometry = Qgis.GeometryOperationResult.InvalidBaseGeometry
QgsGeometry.InvalidBaseGeometry.is_monkey_patched = True
QgsGeometry.InvalidBaseGeometry.__doc__ = "The base geometry on which the operation is done is invalid or empty"
QgsGeometry.InvalidInputGeometryType = Qgis.GeometryOperationResult.InvalidInputGeometryType
QgsGeometry.InvalidInputGeometryType.is_monkey_patched = True
QgsGeometry.InvalidInputGeometryType.__doc__ = "The input geometry (ring, part, split line, etc.) has not the correct geometry type"
QgsGeometry.SelectionIsEmpty = Qgis.GeometryOperationResult.SelectionIsEmpty
QgsGeometry.SelectionIsEmpty.is_monkey_patched = True
QgsGeometry.SelectionIsEmpty.__doc__ = "No features were selected"
QgsGeometry.SelectionIsGreaterThanOne = Qgis.GeometryOperationResult.SelectionIsGreaterThanOne
QgsGeometry.SelectionIsGreaterThanOne.is_monkey_patched = True
QgsGeometry.SelectionIsGreaterThanOne.__doc__ = "More than one features were selected"
QgsGeometry.GeometryEngineError = Qgis.GeometryOperationResult.GeometryEngineError
QgsGeometry.GeometryEngineError.is_monkey_patched = True
QgsGeometry.GeometryEngineError.__doc__ = "Geometry engine misses a method implemented or an error occurred in the geometry engine"
QgsGeometry.LayerNotEditable = Qgis.GeometryOperationResult.LayerNotEditable
QgsGeometry.LayerNotEditable.is_monkey_patched = True
QgsGeometry.LayerNotEditable.__doc__ = "Cannot edit layer"
QgsGeometry.AddPartSelectedGeometryNotFound = Qgis.GeometryOperationResult.AddPartSelectedGeometryNotFound
QgsGeometry.AddPartSelectedGeometryNotFound.is_monkey_patched = True
QgsGeometry.AddPartSelectedGeometryNotFound.__doc__ = "The selected geometry cannot be found"
QgsGeometry.AddPartNotMultiGeometry = Qgis.GeometryOperationResult.AddPartNotMultiGeometry
QgsGeometry.AddPartNotMultiGeometry.is_monkey_patched = True
QgsGeometry.AddPartNotMultiGeometry.__doc__ = "The source geometry is not multi"
QgsGeometry.AddRingNotClosed = Qgis.GeometryOperationResult.AddRingNotClosed
QgsGeometry.AddRingNotClosed.is_monkey_patched = True
QgsGeometry.AddRingNotClosed.__doc__ = "The input ring is not closed"
QgsGeometry.AddRingNotValid = Qgis.GeometryOperationResult.AddRingNotValid
QgsGeometry.AddRingNotValid.is_monkey_patched = True
QgsGeometry.AddRingNotValid.__doc__ = "The input ring is not valid"
QgsGeometry.AddRingCrossesExistingRings = Qgis.GeometryOperationResult.AddRingCrossesExistingRings
QgsGeometry.AddRingCrossesExistingRings.is_monkey_patched = True
QgsGeometry.AddRingCrossesExistingRings.__doc__ = "The input ring crosses existing rings (it is not disjoint)"
QgsGeometry.AddRingNotInExistingFeature = Qgis.GeometryOperationResult.AddRingNotInExistingFeature
QgsGeometry.AddRingNotInExistingFeature.is_monkey_patched = True
QgsGeometry.AddRingNotInExistingFeature.__doc__ = "The input ring doesn't have any existing ring to fit into"
QgsGeometry.SplitCannotSplitPoint = Qgis.GeometryOperationResult.SplitCannotSplitPoint
QgsGeometry.SplitCannotSplitPoint.is_monkey_patched = True
QgsGeometry.SplitCannotSplitPoint.__doc__ = "Cannot split points"
QgsGeometry.GeometryTypeHasChanged = Qgis.GeometryOperationResult.GeometryTypeHasChanged
QgsGeometry.GeometryTypeHasChanged.is_monkey_patched = True
QgsGeometry.GeometryTypeHasChanged.__doc__ = "Operation has changed geometry type"
Qgis.GeometryOperationResult.__doc__ = "Success or failure of a geometry operation.\n\nThis enum gives details about cause of failure.\n\n.. versionadded:: 3.22\n\n" + '* ``Success``: ' + Qgis.GeometryOperationResult.Success.__doc__ + '\n' + '* ``NothingHappened``: ' + Qgis.GeometryOperationResult.NothingHappened.__doc__ + '\n' + '* ``InvalidBaseGeometry``: ' + Qgis.GeometryOperationResult.InvalidBaseGeometry.__doc__ + '\n' + '* ``InvalidInputGeometryType``: ' + Qgis.GeometryOperationResult.InvalidInputGeometryType.__doc__ + '\n' + '* ``SelectionIsEmpty``: ' + Qgis.GeometryOperationResult.SelectionIsEmpty.__doc__ + '\n' + '* ``SelectionIsGreaterThanOne``: ' + Qgis.GeometryOperationResult.SelectionIsGreaterThanOne.__doc__ + '\n' + '* ``GeometryEngineError``: ' + Qgis.GeometryOperationResult.GeometryEngineError.__doc__ + '\n' + '* ``LayerNotEditable``: ' + Qgis.GeometryOperationResult.LayerNotEditable.__doc__ + '\n' + '* ``AddPartSelectedGeometryNotFound``: ' + Qgis.GeometryOperationResult.AddPartSelectedGeometryNotFound.__doc__ + '\n' + '* ``AddPartNotMultiGeometry``: ' + Qgis.GeometryOperationResult.AddPartNotMultiGeometry.__doc__ + '\n' + '* ``AddRingNotClosed``: ' + Qgis.GeometryOperationResult.AddRingNotClosed.__doc__ + '\n' + '* ``AddRingNotValid``: ' + Qgis.GeometryOperationResult.AddRingNotValid.__doc__ + '\n' + '* ``AddRingCrossesExistingRings``: ' + Qgis.GeometryOperationResult.AddRingCrossesExistingRings.__doc__ + '\n' + '* ``AddRingNotInExistingFeature``: ' + Qgis.GeometryOperationResult.AddRingNotInExistingFeature.__doc__ + '\n' + '* ``SplitCannotSplitPoint``: ' + Qgis.GeometryOperationResult.SplitCannotSplitPoint.__doc__ + '\n' + '* ``GeometryTypeHasChanged``: ' + Qgis.GeometryOperationResult.GeometryTypeHasChanged.__doc__
# --
Qgis.GeometryOperationResult.baseClass = Qgis
QgsGeometry.ValidityFlag = Qgis.GeometryValidityFlag
# monkey patching scoped based enum
QgsGeometry.FlagAllowSelfTouchingHoles = Qgis.GeometryValidityFlag.AllowSelfTouchingHoles
QgsGeometry.ValidityFlag.FlagAllowSelfTouchingHoles = Qgis.GeometryValidityFlag.AllowSelfTouchingHoles
QgsGeometry.FlagAllowSelfTouchingHoles.is_monkey_patched = True
QgsGeometry.FlagAllowSelfTouchingHoles.__doc__ = "Indicates that self-touching holes are permitted. OGC validity states that self-touching holes are NOT permitted, whilst other vendor validity checks (e.g. ESRI) permit self-touching holes."
Qgis.GeometryValidityFlag.__doc__ = "Geometry validity check flags.\n\n.. versionadded:: 3.22\n\n" + '* ``FlagAllowSelfTouchingHoles``: ' + Qgis.GeometryValidityFlag.AllowSelfTouchingHoles.__doc__
# --
Qgis.GeometryValidityFlags = lambda flags=0: Qgis.GeometryValidityFlag(flags)
QgsGeometry.ValidityFlags = Qgis.GeometryValidityFlags
Qgis.GeometryValidityFlag.baseClass = Qgis
Qgis.GeometryValidityFlags.baseClass = Qgis
GeometryValidityFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsGeometry.ValidationMethod = Qgis.GeometryValidationEngine
# monkey patching scoped based enum
QgsGeometry.ValidatorQgisInternal = Qgis.GeometryValidationEngine.QgisInternal
QgsGeometry.ValidationMethod.ValidatorQgisInternal = Qgis.GeometryValidationEngine.QgisInternal
QgsGeometry.ValidatorQgisInternal.is_monkey_patched = True
QgsGeometry.ValidatorQgisInternal.__doc__ = "Use internal QgsGeometryValidator method"
QgsGeometry.ValidatorGeos = Qgis.GeometryValidationEngine.Geos
QgsGeometry.ValidationMethod.ValidatorGeos = Qgis.GeometryValidationEngine.Geos
QgsGeometry.ValidatorGeos.is_monkey_patched = True
QgsGeometry.ValidatorGeos.__doc__ = "Use GEOS validation methods"
Qgis.GeometryValidationEngine.__doc__ = "Available engines for validating geometries.\n\n.. versionadded:: 3.22\n\n" + '* ``ValidatorQgisInternal``: ' + Qgis.GeometryValidationEngine.QgisInternal.__doc__ + '\n' + '* ``ValidatorGeos``: ' + Qgis.GeometryValidationEngine.Geos.__doc__
# --
Qgis.GeometryValidationEngine.baseClass = Qgis
QgsGeometry.BufferSide = Qgis.BufferSide
# monkey patching scoped based enum
QgsGeometry.SideLeft = Qgis.BufferSide.Left
QgsGeometry.BufferSide.SideLeft = Qgis.BufferSide.Left
QgsGeometry.SideLeft.is_monkey_patched = True
QgsGeometry.SideLeft.__doc__ = "Buffer to left of line"
QgsGeometry.SideRight = Qgis.BufferSide.Right
QgsGeometry.BufferSide.SideRight = Qgis.BufferSide.Right
QgsGeometry.SideRight.is_monkey_patched = True
QgsGeometry.SideRight.__doc__ = "Buffer to right of line"
Qgis.BufferSide.__doc__ = "Side of line to buffer.\n\n.. versionadded:: 3.22\n\n" + '* ``SideLeft``: ' + Qgis.BufferSide.Left.__doc__ + '\n' + '* ``SideRight``: ' + Qgis.BufferSide.Right.__doc__
# --
Qgis.BufferSide.baseClass = Qgis
QgsGeometry.EndCapStyle = Qgis.EndCapStyle
# monkey patching scoped based enum
QgsGeometry.CapRound = Qgis.EndCapStyle.Round
QgsGeometry.EndCapStyle.CapRound = Qgis.EndCapStyle.Round
QgsGeometry.CapRound.is_monkey_patched = True
QgsGeometry.CapRound.__doc__ = "Round cap"
QgsGeometry.CapFlat = Qgis.EndCapStyle.Flat
QgsGeometry.EndCapStyle.CapFlat = Qgis.EndCapStyle.Flat
QgsGeometry.CapFlat.is_monkey_patched = True
QgsGeometry.CapFlat.__doc__ = "Flat cap (in line with start/end of line)"
QgsGeometry.CapSquare = Qgis.EndCapStyle.Square
QgsGeometry.EndCapStyle.CapSquare = Qgis.EndCapStyle.Square
QgsGeometry.CapSquare.is_monkey_patched = True
QgsGeometry.CapSquare.__doc__ = "Square cap (extends past start/end of line by buffer distance)"
Qgis.EndCapStyle.__doc__ = "End cap styles for buffers.\n\n.. versionadded:: 3.22\n\n" + '* ``CapRound``: ' + Qgis.EndCapStyle.Round.__doc__ + '\n' + '* ``CapFlat``: ' + Qgis.EndCapStyle.Flat.__doc__ + '\n' + '* ``CapSquare``: ' + Qgis.EndCapStyle.Square.__doc__
# --
Qgis.EndCapStyle.baseClass = Qgis
QgsGeometry.JoinStyle = Qgis.JoinStyle
# monkey patching scoped based enum
QgsGeometry.JoinStyleRound = Qgis.JoinStyle.Round
QgsGeometry.JoinStyle.JoinStyleRound = Qgis.JoinStyle.Round
QgsGeometry.JoinStyleRound.is_monkey_patched = True
QgsGeometry.JoinStyleRound.__doc__ = "Use rounded joins"
QgsGeometry.JoinStyleMiter = Qgis.JoinStyle.Miter
QgsGeometry.JoinStyle.JoinStyleMiter = Qgis.JoinStyle.Miter
QgsGeometry.JoinStyleMiter.is_monkey_patched = True
QgsGeometry.JoinStyleMiter.__doc__ = "Use mitered joins"
QgsGeometry.JoinStyleBevel = Qgis.JoinStyle.Bevel
QgsGeometry.JoinStyle.JoinStyleBevel = Qgis.JoinStyle.Bevel
QgsGeometry.JoinStyleBevel.is_monkey_patched = True
QgsGeometry.JoinStyleBevel.__doc__ = "Use beveled joins"
Qgis.JoinStyle.__doc__ = "Join styles for buffers.\n\n.. versionadded:: 3.22\n\n" + '* ``JoinStyleRound``: ' + Qgis.JoinStyle.Round.__doc__ + '\n' + '* ``JoinStyleMiter``: ' + Qgis.JoinStyle.Miter.__doc__ + '\n' + '* ``JoinStyleBevel``: ' + Qgis.JoinStyle.Bevel.__doc__
# --
Qgis.JoinStyle.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CoverageValidityResult.Invalid.__doc__ = "Coverage is invalid. Invalidity includes polygons that overlap, that have gaps smaller than the gap width, or non-polygonal entries in the input collection."
Qgis.CoverageValidityResult.Valid.__doc__ = "Coverage is valid"
Qgis.CoverageValidityResult.Error.__doc__ = "An exception occurred while determining validity"
Qgis.CoverageValidityResult.__doc__ = "Coverage validity results.\n\n.. versionadded:: 3.36\n\n" + '* ``Invalid``: ' + Qgis.CoverageValidityResult.Invalid.__doc__ + '\n' + '* ``Valid``: ' + Qgis.CoverageValidityResult.Valid.__doc__ + '\n' + '* ``Error``: ' + Qgis.CoverageValidityResult.Error.__doc__
# --
Qgis.CoverageValidityResult.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MakeValidMethod.Linework.__doc__ = "Combines all rings into a set of noded lines and then extracts valid polygons from that linework."
Qgis.MakeValidMethod.Structure.__doc__ = "Structured method, first makes all rings valid and then merges shells and subtracts holes from shells to generate valid result. Assumes that holes and shells are correctly categorized. Requires GEOS 3.10+."
Qgis.MakeValidMethod.__doc__ = "Algorithms to use when repairing invalid geometries.\n\n.. versionadded:: 3.28\n\n" + '* ``Linework``: ' + Qgis.MakeValidMethod.Linework.__doc__ + '\n' + '* ``Structure``: ' + Qgis.MakeValidMethod.Structure.__doc__
# --
Qgis.MakeValidMethod.baseClass = Qgis
QgsFeatureRequest.Flag = Qgis.FeatureRequestFlag
# monkey patching scoped based enum
QgsFeatureRequest.NoFlags = Qgis.FeatureRequestFlag.NoFlags
QgsFeatureRequest.NoFlags.is_monkey_patched = True
QgsFeatureRequest.NoFlags.__doc__ = "No flags are set"
QgsFeatureRequest.NoGeometry = Qgis.FeatureRequestFlag.NoGeometry
QgsFeatureRequest.NoGeometry.is_monkey_patched = True
QgsFeatureRequest.NoGeometry.__doc__ = "Geometry is not required. It may still be returned if e.g. required for a filter condition."
QgsFeatureRequest.SubsetOfAttributes = Qgis.FeatureRequestFlag.SubsetOfAttributes
QgsFeatureRequest.SubsetOfAttributes.is_monkey_patched = True
QgsFeatureRequest.SubsetOfAttributes.__doc__ = "Fetch only a subset of attributes (setSubsetOfAttributes sets this flag)"
QgsFeatureRequest.ExactIntersect = Qgis.FeatureRequestFlag.ExactIntersect
QgsFeatureRequest.ExactIntersect.is_monkey_patched = True
QgsFeatureRequest.ExactIntersect.__doc__ = "Use exact geometry intersection (slower) instead of bounding boxes"
QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation = Qgis.FeatureRequestFlag.IgnoreStaticNodesDuringExpressionCompilation
QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation.is_monkey_patched = True
QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation.__doc__ = "If a feature request uses a filter expression which can be partially precalculated due to static nodes in the expression, setting this flag will prevent these precalculated values from being utilized during compilation of the filter for the backend provider. This flag significantly slows down feature requests and should be used for debugging purposes only. (Since QGIS 3.18)"
QgsFeatureRequest.EmbeddedSymbols = Qgis.FeatureRequestFlag.EmbeddedSymbols
QgsFeatureRequest.EmbeddedSymbols.is_monkey_patched = True
QgsFeatureRequest.EmbeddedSymbols.__doc__ = "Retrieve any embedded feature symbology (since QGIS 3.20)"
Qgis.FeatureRequestFlag.__doc__ = "Flags for controlling feature requests.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureRequest`.Flag\n\n.. versionadded:: 3.36\n\n" + '* ``NoFlags``: ' + Qgis.FeatureRequestFlag.NoFlags.__doc__ + '\n' + '* ``NoGeometry``: ' + Qgis.FeatureRequestFlag.NoGeometry.__doc__ + '\n' + '* ``SubsetOfAttributes``: ' + Qgis.FeatureRequestFlag.SubsetOfAttributes.__doc__ + '\n' + '* ``ExactIntersect``: ' + Qgis.FeatureRequestFlag.ExactIntersect.__doc__ + '\n' + '* ``IgnoreStaticNodesDuringExpressionCompilation``: ' + Qgis.FeatureRequestFlag.IgnoreStaticNodesDuringExpressionCompilation.__doc__ + '\n' + '* ``EmbeddedSymbols``: ' + Qgis.FeatureRequestFlag.EmbeddedSymbols.__doc__
# --
Qgis.FeatureRequestFlag.baseClass = Qgis
Qgis.FeatureRequestFlags = lambda flags=0: Qgis.FeatureRequestFlag(flags)
QgsFeatureRequest.Flags = Qgis.FeatureRequestFlags
Qgis.FeatureRequestFlags.baseClass = Qgis
FeatureRequestFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsFeatureRequest.FilterType = Qgis.FeatureRequestFilterType
# monkey patching scoped based enum
QgsFeatureRequest.FilterNone = Qgis.FeatureRequestFilterType.NoFilter
QgsFeatureRequest.FilterType.FilterNone = Qgis.FeatureRequestFilterType.NoFilter
QgsFeatureRequest.FilterNone.is_monkey_patched = True
QgsFeatureRequest.FilterNone.__doc__ = "No filter is applied"
QgsFeatureRequest.FilterFid = Qgis.FeatureRequestFilterType.Fid
QgsFeatureRequest.FilterType.FilterFid = Qgis.FeatureRequestFilterType.Fid
QgsFeatureRequest.FilterFid.is_monkey_patched = True
QgsFeatureRequest.FilterFid.__doc__ = "Filter using feature ID"
QgsFeatureRequest.FilterExpression = Qgis.FeatureRequestFilterType.Expression
QgsFeatureRequest.FilterType.FilterExpression = Qgis.FeatureRequestFilterType.Expression
QgsFeatureRequest.FilterExpression.is_monkey_patched = True
QgsFeatureRequest.FilterExpression.__doc__ = "Filter using expression"
QgsFeatureRequest.FilterFids = Qgis.FeatureRequestFilterType.Fids
QgsFeatureRequest.FilterType.FilterFids = Qgis.FeatureRequestFilterType.Fids
QgsFeatureRequest.FilterFids.is_monkey_patched = True
QgsFeatureRequest.FilterFids.__doc__ = "Filter using feature IDs"
Qgis.FeatureRequestFilterType.__doc__ = "Types of feature request filters.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureRequest`.FilterType\n\n.. versionadded:: 3.36\n\n" + '* ``FilterNone``: ' + Qgis.FeatureRequestFilterType.NoFilter.__doc__ + '\n' + '* ``FilterFid``: ' + Qgis.FeatureRequestFilterType.Fid.__doc__ + '\n' + '* ``FilterExpression``: ' + Qgis.FeatureRequestFilterType.Expression.__doc__ + '\n' + '* ``FilterFids``: ' + Qgis.FeatureRequestFilterType.Fids.__doc__
# --
Qgis.FeatureRequestFilterType.baseClass = Qgis
QgsFeatureRequest.InvalidGeometryCheck = Qgis.InvalidGeometryCheck
# monkey patching scoped based enum
QgsFeatureRequest.GeometryNoCheck = Qgis.InvalidGeometryCheck.NoCheck
QgsFeatureRequest.InvalidGeometryCheck.GeometryNoCheck = Qgis.InvalidGeometryCheck.NoCheck
QgsFeatureRequest.GeometryNoCheck.is_monkey_patched = True
QgsFeatureRequest.GeometryNoCheck.__doc__ = "No invalid geometry checking"
QgsFeatureRequest.GeometrySkipInvalid = Qgis.InvalidGeometryCheck.SkipInvalid
QgsFeatureRequest.InvalidGeometryCheck.GeometrySkipInvalid = Qgis.InvalidGeometryCheck.SkipInvalid
QgsFeatureRequest.GeometrySkipInvalid.is_monkey_patched = True
QgsFeatureRequest.GeometrySkipInvalid.__doc__ = "Skip any features with invalid geometry. This requires a slow geometry validity check for every feature."
QgsFeatureRequest.GeometryAbortOnInvalid = Qgis.InvalidGeometryCheck.AbortOnInvalid
QgsFeatureRequest.InvalidGeometryCheck.GeometryAbortOnInvalid = Qgis.InvalidGeometryCheck.AbortOnInvalid
QgsFeatureRequest.GeometryAbortOnInvalid.is_monkey_patched = True
QgsFeatureRequest.GeometryAbortOnInvalid.__doc__ = "Close iterator on encountering any features with invalid geometry. This requires a slow geometry validity check for every feature."
Qgis.InvalidGeometryCheck.__doc__ = "Methods for handling of features with invalid geometries\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureRequest`.InvalidGeometryCheck\n\n.. versionadded:: 3.36\n\n" + '* ``GeometryNoCheck``: ' + Qgis.InvalidGeometryCheck.NoCheck.__doc__ + '\n' + '* ``GeometrySkipInvalid``: ' + Qgis.InvalidGeometryCheck.SkipInvalid.__doc__ + '\n' + '* ``GeometryAbortOnInvalid``: ' + Qgis.InvalidGeometryCheck.AbortOnInvalid.__doc__
# --
Qgis.InvalidGeometryCheck.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SpatialFilterType.NoFilter.__doc__ = "No spatial filtering of features"
Qgis.SpatialFilterType.BoundingBox.__doc__ = "Filter using a bounding box"
Qgis.SpatialFilterType.DistanceWithin.__doc__ = "Filter by distance to reference geometry"
Qgis.SpatialFilterType.__doc__ = "Feature request spatial filter types.\n\n.. versionadded:: 3.22\n\n" + '* ``NoFilter``: ' + Qgis.SpatialFilterType.NoFilter.__doc__ + '\n' + '* ``BoundingBox``: ' + Qgis.SpatialFilterType.BoundingBox.__doc__ + '\n' + '* ``DistanceWithin``: ' + Qgis.SpatialFilterType.DistanceWithin.__doc__
# --
Qgis.SpatialFilterType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FileOperationFlag.IncludeMetadataFile.__doc__ = "Indicates that any associated .qmd metadata file should be included with the operation"
Qgis.FileOperationFlag.IncludeStyleFile.__doc__ = "Indicates that any associated .qml styling file should be included with the operation"
Qgis.FileOperationFlag.__doc__ = "File operation flags.\n\n.. versionadded:: 3.22\n\n" + '* ``IncludeMetadataFile``: ' + Qgis.FileOperationFlag.IncludeMetadataFile.__doc__ + '\n' + '* ``IncludeStyleFile``: ' + Qgis.FileOperationFlag.IncludeStyleFile.__doc__
# --
Qgis.FileOperationFlags = lambda flags=0: Qgis.FileOperationFlag(flags)
Qgis.FileOperationFlag.baseClass = Qgis
Qgis.FileOperationFlags.baseClass = Qgis
FileOperationFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.MapLayerProperty.UsersCannotToggleEditing.__doc__ = "Indicates that users are not allowed to toggle editing for this layer. Note that this does not imply that the layer is non-editable (see isEditable(), supportsEditing() ), rather that the editable status of the layer cannot be changed by users manually. Since QGIS 3.22."
Qgis.MapLayerProperty.IsBasemapLayer.__doc__ = "Layer is considered a 'basemap' layer, and certain properties of the layer should be ignored when calculating project-level properties. For instance, the extent of basemap layers is ignored when calculating the extent of a project, as these layers are typically global and extend outside of a project's area of interest. Since QGIS 3.26."
Qgis.MapLayerProperty.__doc__ = "Generic map layer properties.\n\n.. versionadded:: 3.22\n\n" + '* ``UsersCannotToggleEditing``: ' + Qgis.MapLayerProperty.UsersCannotToggleEditing.__doc__ + '\n' + '* ``IsBasemapLayer``: ' + Qgis.MapLayerProperty.IsBasemapLayer.__doc__
# --
Qgis.MapLayerProperties = lambda flags=0: Qgis.MapLayerProperty(flags)
Qgis.MapLayerProperty.baseClass = Qgis
Qgis.MapLayerProperties.baseClass = Qgis
MapLayerProperties = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.AutoRefreshMode.Disabled.__doc__ = "Automatic refreshing is disabled"
Qgis.AutoRefreshMode.ReloadData.__doc__ = "Reload data (and draw the new data)"
Qgis.AutoRefreshMode.RedrawOnly.__doc__ = "Redraw current data only"
Qgis.AutoRefreshMode.__doc__ = "Map layer automatic refresh modes.\n\n.. versionadded:: 3.34\n\n" + '* ``Disabled``: ' + Qgis.AutoRefreshMode.Disabled.__doc__ + '\n' + '* ``ReloadData``: ' + Qgis.AutoRefreshMode.ReloadData.__doc__ + '\n' + '* ``RedrawOnly``: ' + Qgis.AutoRefreshMode.RedrawOnly.__doc__
# --
Qgis.AutoRefreshMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DataProviderFlag.IsBasemapSource.__doc__ = "Associated source should be considered a 'basemap' layer. See Qgis.MapLayerProperty.IsBasemapLayer."
Qgis.DataProviderFlag.FastExtent2D.__doc__ = "Provider's 2D extent retrieval via QgsDataProvider.extent() is always guaranteed to be trivial/fast to calculate. Since QGIS 3.38."
Qgis.DataProviderFlag.FastExtent3D.__doc__ = "Provider's 3D extent retrieval via QgsDataProvider.extent3D() is always guaranteed to be trivial/fast to calculate. Since QGIS 3.38."
Qgis.DataProviderFlag.__doc__ = "Generic data provider flags.\n\n.. versionadded:: 3.26\n\n" + '* ``IsBasemapSource``: ' + Qgis.DataProviderFlag.IsBasemapSource.__doc__ + '\n' + '* ``FastExtent2D``: ' + Qgis.DataProviderFlag.FastExtent2D.__doc__ + '\n' + '* ``FastExtent3D``: ' + Qgis.DataProviderFlag.FastExtent3D.__doc__
# --
Qgis.DataProviderFlags = lambda flags=0: Qgis.DataProviderFlag(flags)
Qgis.DataProviderFlag.baseClass = Qgis
Qgis.DataProviderFlags.baseClass = Qgis
DataProviderFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.CrsType.Unknown.__doc__ = "Unknown type"
Qgis.CrsType.Geodetic.__doc__ = "Geodetic CRS"
Qgis.CrsType.Geocentric.__doc__ = "Geocentric CRS"
Qgis.CrsType.Geographic2d.__doc__ = "2D geographic CRS"
Qgis.CrsType.Geographic3d.__doc__ = "3D geopraphic CRS"
Qgis.CrsType.Vertical.__doc__ = "Vertical CRS"
Qgis.CrsType.Projected.__doc__ = "Projected CRS"
Qgis.CrsType.Compound.__doc__ = "Compound (horizontal + vertical) CRS"
Qgis.CrsType.Temporal.__doc__ = "Temporal CRS"
Qgis.CrsType.Engineering.__doc__ = "Engineering CRS"
Qgis.CrsType.Bound.__doc__ = "Bound CRS"
Qgis.CrsType.Other.__doc__ = "Other type"
Qgis.CrsType.DerivedProjected.__doc__ = "Derived projected CRS"
Qgis.CrsType.__doc__ = "Coordinate reference system types.\n\nContains a subset of Proj's PJ_TYPE enum, specifically the types which relate to CRS types.\n\n.. versionadded:: 3.34\n\n" + '* ``Unknown``: ' + Qgis.CrsType.Unknown.__doc__ + '\n' + '* ``Geodetic``: ' + Qgis.CrsType.Geodetic.__doc__ + '\n' + '* ``Geocentric``: ' + Qgis.CrsType.Geocentric.__doc__ + '\n' + '* ``Geographic2d``: ' + Qgis.CrsType.Geographic2d.__doc__ + '\n' + '* ``Geographic3d``: ' + Qgis.CrsType.Geographic3d.__doc__ + '\n' + '* ``Vertical``: ' + Qgis.CrsType.Vertical.__doc__ + '\n' + '* ``Projected``: ' + Qgis.CrsType.Projected.__doc__ + '\n' + '* ``Compound``: ' + Qgis.CrsType.Compound.__doc__ + '\n' + '* ``Temporal``: ' + Qgis.CrsType.Temporal.__doc__ + '\n' + '* ``Engineering``: ' + Qgis.CrsType.Engineering.__doc__ + '\n' + '* ``Bound``: ' + Qgis.CrsType.Bound.__doc__ + '\n' + '* ``Other``: ' + Qgis.CrsType.Other.__doc__ + '\n' + '* ``DerivedProjected``: ' + Qgis.CrsType.DerivedProjected.__doc__
# --
Qgis.CrsType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CrsAxisDirection.North.__doc__ = "North"
Qgis.CrsAxisDirection.NorthNorthEast.__doc__ = "North North East"
Qgis.CrsAxisDirection.NorthEast.__doc__ = "North East"
Qgis.CrsAxisDirection.EastNorthEast.__doc__ = "East North East"
Qgis.CrsAxisDirection.East.__doc__ = "East"
Qgis.CrsAxisDirection.EastSouthEast.__doc__ = "East South East"
Qgis.CrsAxisDirection.SouthEast.__doc__ = "South East"
Qgis.CrsAxisDirection.SouthSouthEast.__doc__ = "South South East"
Qgis.CrsAxisDirection.South.__doc__ = "South"
Qgis.CrsAxisDirection.SouthSouthWest.__doc__ = "South South West"
Qgis.CrsAxisDirection.SouthWest.__doc__ = "South West"
Qgis.CrsAxisDirection.WestSouthWest.__doc__ = "West South West"
Qgis.CrsAxisDirection.West.__doc__ = "West"
Qgis.CrsAxisDirection.WestNorthWest.__doc__ = "West North West"
Qgis.CrsAxisDirection.NorthWest.__doc__ = "North West"
Qgis.CrsAxisDirection.NorthNorthWest.__doc__ = "North North West"
Qgis.CrsAxisDirection.GeocentricX.__doc__ = "Geocentric (X)"
Qgis.CrsAxisDirection.GeocentricY.__doc__ = "Geocentric (Y)"
Qgis.CrsAxisDirection.GeocentricZ.__doc__ = "Geocentric (Z)"
Qgis.CrsAxisDirection.Up.__doc__ = "Up"
Qgis.CrsAxisDirection.Down.__doc__ = "Down"
Qgis.CrsAxisDirection.Forward.__doc__ = "Forward"
Qgis.CrsAxisDirection.Aft.__doc__ = "Aft"
Qgis.CrsAxisDirection.Port.__doc__ = "Port"
Qgis.CrsAxisDirection.Starboard.__doc__ = "Starboard"
Qgis.CrsAxisDirection.Clockwise.__doc__ = "Clockwise"
Qgis.CrsAxisDirection.CounterClockwise.__doc__ = "Counter clockwise"
Qgis.CrsAxisDirection.ColumnPositive.__doc__ = "Column positive"
Qgis.CrsAxisDirection.ColumnNegative.__doc__ = "Column negative"
Qgis.CrsAxisDirection.RowPositive.__doc__ = "Row positive"
Qgis.CrsAxisDirection.RowNegative.__doc__ = "Row negative"
Qgis.CrsAxisDirection.DisplayRight.__doc__ = "Display right"
Qgis.CrsAxisDirection.DisplayLeft.__doc__ = "Display left"
Qgis.CrsAxisDirection.DisplayUp.__doc__ = "Display up"
Qgis.CrsAxisDirection.DisplayDown.__doc__ = "Display down"
Qgis.CrsAxisDirection.Future.__doc__ = "Future"
Qgis.CrsAxisDirection.Past.__doc__ = "Past"
Qgis.CrsAxisDirection.Towards.__doc__ = "Towards"
Qgis.CrsAxisDirection.AwayFrom.__doc__ = "Away from"
Qgis.CrsAxisDirection.Unspecified.__doc__ = "Unspecified"
Qgis.CrsAxisDirection.__doc__ = "Coordinate reference system axis directions.\n\nFrom \"Geographic information — Well-known text representation of coordinate reference systems\", section 7.5.1.\n\n.. versionadded:: 3.26\n\n" + '* ``North``: ' + Qgis.CrsAxisDirection.North.__doc__ + '\n' + '* ``NorthNorthEast``: ' + Qgis.CrsAxisDirection.NorthNorthEast.__doc__ + '\n' + '* ``NorthEast``: ' + Qgis.CrsAxisDirection.NorthEast.__doc__ + '\n' + '* ``EastNorthEast``: ' + Qgis.CrsAxisDirection.EastNorthEast.__doc__ + '\n' + '* ``East``: ' + Qgis.CrsAxisDirection.East.__doc__ + '\n' + '* ``EastSouthEast``: ' + Qgis.CrsAxisDirection.EastSouthEast.__doc__ + '\n' + '* ``SouthEast``: ' + Qgis.CrsAxisDirection.SouthEast.__doc__ + '\n' + '* ``SouthSouthEast``: ' + Qgis.CrsAxisDirection.SouthSouthEast.__doc__ + '\n' + '* ``South``: ' + Qgis.CrsAxisDirection.South.__doc__ + '\n' + '* ``SouthSouthWest``: ' + Qgis.CrsAxisDirection.SouthSouthWest.__doc__ + '\n' + '* ``SouthWest``: ' + Qgis.CrsAxisDirection.SouthWest.__doc__ + '\n' + '* ``WestSouthWest``: ' + Qgis.CrsAxisDirection.WestSouthWest.__doc__ + '\n' + '* ``West``: ' + Qgis.CrsAxisDirection.West.__doc__ + '\n' + '* ``WestNorthWest``: ' + Qgis.CrsAxisDirection.WestNorthWest.__doc__ + '\n' + '* ``NorthWest``: ' + Qgis.CrsAxisDirection.NorthWest.__doc__ + '\n' + '* ``NorthNorthWest``: ' + Qgis.CrsAxisDirection.NorthNorthWest.__doc__ + '\n' + '* ``GeocentricX``: ' + Qgis.CrsAxisDirection.GeocentricX.__doc__ + '\n' + '* ``GeocentricY``: ' + Qgis.CrsAxisDirection.GeocentricY.__doc__ + '\n' + '* ``GeocentricZ``: ' + Qgis.CrsAxisDirection.GeocentricZ.__doc__ + '\n' + '* ``Up``: ' + Qgis.CrsAxisDirection.Up.__doc__ + '\n' + '* ``Down``: ' + Qgis.CrsAxisDirection.Down.__doc__ + '\n' + '* ``Forward``: ' + Qgis.CrsAxisDirection.Forward.__doc__ + '\n' + '* ``Aft``: ' + Qgis.CrsAxisDirection.Aft.__doc__ + '\n' + '* ``Port``: ' + Qgis.CrsAxisDirection.Port.__doc__ + '\n' + '* ``Starboard``: ' + Qgis.CrsAxisDirection.Starboard.__doc__ + '\n' + '* ``Clockwise``: ' + Qgis.CrsAxisDirection.Clockwise.__doc__ + '\n' + '* ``CounterClockwise``: ' + Qgis.CrsAxisDirection.CounterClockwise.__doc__ + '\n' + '* ``ColumnPositive``: ' + Qgis.CrsAxisDirection.ColumnPositive.__doc__ + '\n' + '* ``ColumnNegative``: ' + Qgis.CrsAxisDirection.ColumnNegative.__doc__ + '\n' + '* ``RowPositive``: ' + Qgis.CrsAxisDirection.RowPositive.__doc__ + '\n' + '* ``RowNegative``: ' + Qgis.CrsAxisDirection.RowNegative.__doc__ + '\n' + '* ``DisplayRight``: ' + Qgis.CrsAxisDirection.DisplayRight.__doc__ + '\n' + '* ``DisplayLeft``: ' + Qgis.CrsAxisDirection.DisplayLeft.__doc__ + '\n' + '* ``DisplayUp``: ' + Qgis.CrsAxisDirection.DisplayUp.__doc__ + '\n' + '* ``DisplayDown``: ' + Qgis.CrsAxisDirection.DisplayDown.__doc__ + '\n' + '* ``Future``: ' + Qgis.CrsAxisDirection.Future.__doc__ + '\n' + '* ``Past``: ' + Qgis.CrsAxisDirection.Past.__doc__ + '\n' + '* ``Towards``: ' + Qgis.CrsAxisDirection.Towards.__doc__ + '\n' + '* ``AwayFrom``: ' + Qgis.CrsAxisDirection.AwayFrom.__doc__ + '\n' + '* ``Unspecified``: ' + Qgis.CrsAxisDirection.Unspecified.__doc__
# --
Qgis.CrsAxisDirection.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CoordinateOrder.Default.__doc__ = "Respect the default axis ordering for the CRS, as defined in the CRS's parameters"
Qgis.CoordinateOrder.XY.__doc__ = "Easting/Northing (or Longitude/Latitude for geographic CRS)"
Qgis.CoordinateOrder.YX.__doc__ = "Northing/Easting (or Latitude/Longitude for geographic CRS)"
Qgis.CoordinateOrder.__doc__ = "Order of coordinates.\n\n.. versionadded:: 3.26\n\n" + '* ``Default``: ' + Qgis.CoordinateOrder.Default.__doc__ + '\n' + '* ``XY``: ' + Qgis.CoordinateOrder.XY.__doc__ + '\n' + '* ``YX``: ' + Qgis.CoordinateOrder.YX.__doc__
# --
Qgis.CoordinateOrder.baseClass = Qgis
QgsCoordinateReferenceSystem.IdentifierType = Qgis.CrsIdentifierType
# monkey patching scoped based enum
QgsCoordinateReferenceSystem.ShortString = Qgis.CrsIdentifierType.ShortString
QgsCoordinateReferenceSystem.ShortString.is_monkey_patched = True
QgsCoordinateReferenceSystem.ShortString.__doc__ = "A heavily abbreviated string, for use when a compact representation is required"
QgsCoordinateReferenceSystem.MediumString = Qgis.CrsIdentifierType.MediumString
QgsCoordinateReferenceSystem.MediumString.is_monkey_patched = True
QgsCoordinateReferenceSystem.MediumString.__doc__ = "A medium-length string, recommended for general purpose use"
QgsCoordinateReferenceSystem.FullString = Qgis.CrsIdentifierType.FullString
QgsCoordinateReferenceSystem.FullString.is_monkey_patched = True
QgsCoordinateReferenceSystem.FullString.__doc__ = "Full definition -- possibly a very lengthy string, e.g. with no truncation of custom WKT definitions"
Qgis.CrsIdentifierType.__doc__ = "Available identifier string types for representing coordinate reference systems\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsCoordinateReferenceSystem`.IdentifierType\n\n.. versionadded:: 3.36\n\n" + '* ``ShortString``: ' + Qgis.CrsIdentifierType.ShortString.__doc__ + '\n' + '* ``MediumString``: ' + Qgis.CrsIdentifierType.MediumString.__doc__ + '\n' + '* ``FullString``: ' + Qgis.CrsIdentifierType.FullString.__doc__
# --
Qgis.CrsIdentifierType.baseClass = Qgis
QgsCoordinateReferenceSystem.WktVariant = Qgis.CrsWktVariant
# monkey patching scoped based enum
QgsCoordinateReferenceSystem.WKT1_GDAL = Qgis.CrsWktVariant.Wkt1Gdal
QgsCoordinateReferenceSystem.WktVariant.WKT1_GDAL = Qgis.CrsWktVariant.Wkt1Gdal
QgsCoordinateReferenceSystem.WKT1_GDAL.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT1_GDAL.__doc__ = "WKT1 as traditionally output by GDAL, deriving from OGC 01-009. A notable departure from WKT1_GDAL with respect to OGC 01-009 is that in WKT1_GDAL, the unit of the PRIMEM value is always degrees."
QgsCoordinateReferenceSystem.WKT1_ESRI = Qgis.CrsWktVariant.Wkt1Esri
QgsCoordinateReferenceSystem.WktVariant.WKT1_ESRI = Qgis.CrsWktVariant.Wkt1Esri
QgsCoordinateReferenceSystem.WKT1_ESRI.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT1_ESRI.__doc__ = "WKT1 as traditionally output by ESRI software, deriving from OGC 99-049."
QgsCoordinateReferenceSystem.WKT2_2015 = Qgis.CrsWktVariant.Wkt2_2015
QgsCoordinateReferenceSystem.WktVariant.WKT2_2015 = Qgis.CrsWktVariant.Wkt2_2015
QgsCoordinateReferenceSystem.WKT2_2015.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT2_2015.__doc__ = "Full WKT2 string, conforming to ISO 19162:2015(E) / OGC 12-063r5 with all possible nodes and new keyword names."
QgsCoordinateReferenceSystem.WKT2_2015_SIMPLIFIED = Qgis.CrsWktVariant.Wkt2_2015Simplified
QgsCoordinateReferenceSystem.WktVariant.WKT2_2015_SIMPLIFIED = Qgis.CrsWktVariant.Wkt2_2015Simplified
QgsCoordinateReferenceSystem.WKT2_2015_SIMPLIFIED.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT2_2015_SIMPLIFIED.__doc__ = "Same as WKT2_2015 with the following exceptions: UNIT keyword used. ID node only on top element. No ORDER element in AXIS element. PRIMEM node omitted if it is Greenwich.  ELLIPSOID.UNIT node omitted if it is UnitOfMeasure.METRE. PARAMETER.UNIT / PRIMEM.UNIT omitted if same as AXIS. AXIS.UNIT omitted and replaced by a common GEODCRS.UNIT if they are all the same on all axis."
QgsCoordinateReferenceSystem.WKT2_2019 = Qgis.CrsWktVariant.Wkt2_2019
QgsCoordinateReferenceSystem.WktVariant.WKT2_2019 = Qgis.CrsWktVariant.Wkt2_2019
QgsCoordinateReferenceSystem.WKT2_2019.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT2_2019.__doc__ = "Full WKT2 string, conforming to ISO 19162:2019 / OGC 18-010, with all possible nodes and new keyword names. Non-normative list of differences: WKT2_2019 uses GEOGCRS / BASEGEOGCRS keywords for GeographicCRS."
QgsCoordinateReferenceSystem.WKT2_2019_SIMPLIFIED = Qgis.CrsWktVariant.Wkt2_2019Simplified
QgsCoordinateReferenceSystem.WktVariant.WKT2_2019_SIMPLIFIED = Qgis.CrsWktVariant.Wkt2_2019Simplified
QgsCoordinateReferenceSystem.WKT2_2019_SIMPLIFIED.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT2_2019_SIMPLIFIED.__doc__ = "WKT2_2019 with the simplification rule of WKT2_SIMPLIFIED"
QgsCoordinateReferenceSystem.WKT_PREFERRED = Qgis.CrsWktVariant.Preferred
QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED = Qgis.CrsWktVariant.Preferred
QgsCoordinateReferenceSystem.WKT_PREFERRED.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT_PREFERRED.__doc__ = "Preferred format, matching the most recent WKT ISO standard. Currently an alias to WKT2_2019, but may change in future versions."
QgsCoordinateReferenceSystem.WKT_PREFERRED_SIMPLIFIED = Qgis.CrsWktVariant.PreferredSimplified
QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED_SIMPLIFIED = Qgis.CrsWktVariant.PreferredSimplified
QgsCoordinateReferenceSystem.WKT_PREFERRED_SIMPLIFIED.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT_PREFERRED_SIMPLIFIED.__doc__ = "Preferred simplified format, matching the most recent WKT ISO standard. Currently an alias to WKT2_2019_SIMPLIFIED, but may change in future versions."
QgsCoordinateReferenceSystem.WKT_PREFERRED_GDAL = Qgis.CrsWktVariant.PreferredGdal
QgsCoordinateReferenceSystem.WktVariant.WKT_PREFERRED_GDAL = Qgis.CrsWktVariant.PreferredGdal
QgsCoordinateReferenceSystem.WKT_PREFERRED_GDAL.is_monkey_patched = True
QgsCoordinateReferenceSystem.WKT_PREFERRED_GDAL.__doc__ = "Preferred format for conversion of CRS to WKT for use with the GDAL library."
Qgis.CrsWktVariant.__doc__ = "Coordinate reference system WKT formatting variants.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsCoordinateReferenceSystem`.WktVariant\n\n.. versionadded:: 3.36\n\n" + '* ``WKT1_GDAL``: ' + Qgis.CrsWktVariant.Wkt1Gdal.__doc__ + '\n' + '* ``WKT1_ESRI``: ' + Qgis.CrsWktVariant.Wkt1Esri.__doc__ + '\n' + '* ``WKT2_2015``: ' + Qgis.CrsWktVariant.Wkt2_2015.__doc__ + '\n' + '* ``WKT2_2015_SIMPLIFIED``: ' + Qgis.CrsWktVariant.Wkt2_2015Simplified.__doc__ + '\n' + '* ``WKT2_2019``: ' + Qgis.CrsWktVariant.Wkt2_2019.__doc__ + '\n' + '* ``WKT2_2019_SIMPLIFIED``: ' + Qgis.CrsWktVariant.Wkt2_2019Simplified.__doc__ + '\n' + '* ``WKT_PREFERRED``: ' + Qgis.CrsWktVariant.Preferred.__doc__ + '\n' + '* ``WKT_PREFERRED_SIMPLIFIED``: ' + Qgis.CrsWktVariant.PreferredSimplified.__doc__ + '\n' + '* ``WKT_PREFERRED_GDAL``: ' + Qgis.CrsWktVariant.PreferredGdal.__doc__
# --
Qgis.CrsWktVariant.baseClass = Qgis
# monkey patching scoped based enum
Qgis.Axis.X.__doc__ = "X-axis"
Qgis.Axis.Y.__doc__ = "Y-axis"
Qgis.Axis.Z.__doc__ = "Z-axis"
Qgis.Axis.__doc__ = "Cartesian axes.\n\n.. versionadded:: 3.34\n\n" + '* ``X``: ' + Qgis.Axis.X.__doc__ + '\n' + '* ``Y``: ' + Qgis.Axis.Y.__doc__ + '\n' + '* ``Z``: ' + Qgis.Axis.Z.__doc__
# --
Qgis.Axis.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AnnotationItemFlag.ScaleDependentBoundingBox.__doc__ = "Item's bounding box will vary depending on map scale"
Qgis.AnnotationItemFlag.__doc__ = "Flags for annotation items.\n\n.. versionadded:: 3.22\n\n" + '* ``ScaleDependentBoundingBox``: ' + Qgis.AnnotationItemFlag.ScaleDependentBoundingBox.__doc__
# --
Qgis.AnnotationItemFlags = lambda flags=0: Qgis.AnnotationItemFlag(flags)
Qgis.AnnotationItemFlag.baseClass = Qgis
Qgis.AnnotationItemFlags.baseClass = Qgis
AnnotationItemFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.AnnotationItemGuiFlag.FlagNoCreationTools.__doc__ = "Do not show item creation tools for the item type"
Qgis.AnnotationItemGuiFlag.__doc__ = "Flags for controlling how an annotation item behaves in the GUI.\n\n.. versionadded:: 3.22\n\n" + '* ``FlagNoCreationTools``: ' + Qgis.AnnotationItemGuiFlag.FlagNoCreationTools.__doc__
# --
Qgis.AnnotationItemGuiFlags = lambda flags=0: Qgis.AnnotationItemGuiFlag(flags)
Qgis.AnnotationItemGuiFlag.baseClass = Qgis
Qgis.AnnotationItemGuiFlags.baseClass = Qgis
AnnotationItemGuiFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.AnnotationItemNodeType.VertexHandle.__doc__ = "Node is a handle for manipulating vertices"
Qgis.AnnotationItemNodeType.__doc__ = "Annotation item node types.\n\n.. versionadded:: 3.22\n\n" + '* ``VertexHandle``: ' + Qgis.AnnotationItemNodeType.VertexHandle.__doc__
# --
Qgis.AnnotationItemNodeType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AnnotationItemEditOperationResult.Success.__doc__ = "Item was modified successfully"
Qgis.AnnotationItemEditOperationResult.Invalid.__doc__ = "Operation has invalid parameters for the item, no change occurred"
Qgis.AnnotationItemEditOperationResult.ItemCleared.__doc__ = "The operation results in the item being cleared, and the item should be removed from the layer as a result"
Qgis.AnnotationItemEditOperationResult.__doc__ = "Results from an edit operation on an annotation item.\n\n.. versionadded:: 3.22\n\n" + '* ``Success``: ' + Qgis.AnnotationItemEditOperationResult.Success.__doc__ + '\n' + '* ``Invalid``: ' + Qgis.AnnotationItemEditOperationResult.Invalid.__doc__ + '\n' + '* ``ItemCleared``: ' + Qgis.AnnotationItemEditOperationResult.ItemCleared.__doc__
# --
Qgis.AnnotationItemEditOperationResult.baseClass = Qgis
QgsTemporalNavigationObject.NavigationMode = Qgis.TemporalNavigationMode
# monkey patching scoped based enum
QgsTemporalNavigationObject.NavigationOff = Qgis.TemporalNavigationMode.Disabled
QgsTemporalNavigationObject.NavigationMode.NavigationOff = Qgis.TemporalNavigationMode.Disabled
QgsTemporalNavigationObject.NavigationOff.is_monkey_patched = True
QgsTemporalNavigationObject.NavigationOff.__doc__ = "Temporal navigation is disabled"
QgsTemporalNavigationObject.Animated = Qgis.TemporalNavigationMode.Animated
QgsTemporalNavigationObject.Animated.is_monkey_patched = True
QgsTemporalNavigationObject.Animated.__doc__ = "Temporal navigation relies on frames within a datetime range"
QgsTemporalNavigationObject.FixedRange = Qgis.TemporalNavigationMode.FixedRange
QgsTemporalNavigationObject.FixedRange.is_monkey_patched = True
QgsTemporalNavigationObject.FixedRange.__doc__ = "Temporal navigation relies on a fixed datetime range"
QgsTemporalNavigationObject.Movie = Qgis.TemporalNavigationMode.Movie
QgsTemporalNavigationObject.Movie.is_monkey_patched = True
QgsTemporalNavigationObject.Movie.__doc__ = "Movie mode -- behaves like a video player, with a fixed frame duration and no temporal range (since QGIS 3.36)"
Qgis.TemporalNavigationMode.__doc__ = "Temporal navigation modes.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsTemporalNavigationObject`.NavigationMode\n\n.. versionadded:: 3.36\n\n" + '* ``NavigationOff``: ' + Qgis.TemporalNavigationMode.Disabled.__doc__ + '\n' + '* ``Animated``: ' + Qgis.TemporalNavigationMode.Animated.__doc__ + '\n' + '* ``FixedRange``: ' + Qgis.TemporalNavigationMode.FixedRange.__doc__ + '\n' + '* ``Movie``: ' + Qgis.TemporalNavigationMode.Movie.__doc__
# --
Qgis.TemporalNavigationMode.baseClass = Qgis
QgsTemporalNavigationObject.AnimationState = Qgis.AnimationState
# monkey patching scoped based enum
QgsTemporalNavigationObject.Forward = Qgis.AnimationState.Forward
QgsTemporalNavigationObject.Forward.is_monkey_patched = True
QgsTemporalNavigationObject.Forward.__doc__ = "Animation is playing forward."
QgsTemporalNavigationObject.Reverse = Qgis.AnimationState.Reverse
QgsTemporalNavigationObject.Reverse.is_monkey_patched = True
QgsTemporalNavigationObject.Reverse.__doc__ = "Animation is playing in reverse."
QgsTemporalNavigationObject.Idle = Qgis.AnimationState.Idle
QgsTemporalNavigationObject.Idle.is_monkey_patched = True
QgsTemporalNavigationObject.Idle.__doc__ = "Animation is paused."
Qgis.AnimationState.__doc__ = "Animation states.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsTemporalNavigationObject`.AnimationState\n\n.. versionadded:: 3.36\n\n" + '* ``Forward``: ' + Qgis.AnimationState.Forward.__doc__ + '\n' + '* ``Reverse``: ' + Qgis.AnimationState.Reverse.__doc__ + '\n' + '* ``Idle``: ' + Qgis.AnimationState.Idle.__doc__
# --
Qgis.AnimationState.baseClass = Qgis
# monkey patching scoped based enum
Qgis.PlaybackOperation.SkipToStart.__doc__ = "Jump to start of playback"
Qgis.PlaybackOperation.PreviousFrame.__doc__ = "Step to previous frame"
Qgis.PlaybackOperation.PlayReverse.__doc__ = "Play in reverse"
Qgis.PlaybackOperation.Pause.__doc__ = "Pause playback"
Qgis.PlaybackOperation.PlayForward.__doc__ = "Play forward"
Qgis.PlaybackOperation.NextFrame.__doc__ = "Step to next frame"
Qgis.PlaybackOperation.SkipToEnd.__doc__ = "Jump to end of playback"
Qgis.PlaybackOperation.__doc__ = "Media playback operations.\n\n.. versionadded:: 3.36\n\n" + '* ``SkipToStart``: ' + Qgis.PlaybackOperation.SkipToStart.__doc__ + '\n' + '* ``PreviousFrame``: ' + Qgis.PlaybackOperation.PreviousFrame.__doc__ + '\n' + '* ``PlayReverse``: ' + Qgis.PlaybackOperation.PlayReverse.__doc__ + '\n' + '* ``Pause``: ' + Qgis.PlaybackOperation.Pause.__doc__ + '\n' + '* ``PlayForward``: ' + Qgis.PlaybackOperation.PlayForward.__doc__ + '\n' + '* ``NextFrame``: ' + Qgis.PlaybackOperation.NextFrame.__doc__ + '\n' + '* ``SkipToEnd``: ' + Qgis.PlaybackOperation.SkipToEnd.__doc__
# --
Qgis.PlaybackOperation.baseClass = Qgis
QgsVectorLayerTemporalProperties.TemporalMode = Qgis.VectorTemporalMode
# monkey patching scoped based enum
QgsVectorLayerTemporalProperties.ModeFixedTemporalRange = Qgis.VectorTemporalMode.FixedTemporalRange
QgsVectorLayerTemporalProperties.TemporalMode.ModeFixedTemporalRange = Qgis.VectorTemporalMode.FixedTemporalRange
QgsVectorLayerTemporalProperties.ModeFixedTemporalRange.is_monkey_patched = True
QgsVectorLayerTemporalProperties.ModeFixedTemporalRange.__doc__ = "Mode when temporal properties have fixed start and end datetimes."
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField = Qgis.VectorTemporalMode.FeatureDateTimeInstantFromField
QgsVectorLayerTemporalProperties.TemporalMode.ModeFeatureDateTimeInstantFromField = Qgis.VectorTemporalMode.FeatureDateTimeInstantFromField
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField.is_monkey_patched = True
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField.__doc__ = "Mode when features have a datetime instant taken from a single field"
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields = Qgis.VectorTemporalMode.FeatureDateTimeStartAndEndFromFields
QgsVectorLayerTemporalProperties.TemporalMode.ModeFeatureDateTimeStartAndEndFromFields = Qgis.VectorTemporalMode.FeatureDateTimeStartAndEndFromFields
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields.is_monkey_patched = True
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields.__doc__ = "Mode when features have separate fields for start and end times"
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndDurationFromFields = Qgis.VectorTemporalMode.FeatureDateTimeStartAndDurationFromFields
QgsVectorLayerTemporalProperties.TemporalMode.ModeFeatureDateTimeStartAndDurationFromFields = Qgis.VectorTemporalMode.FeatureDateTimeStartAndDurationFromFields
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndDurationFromFields.is_monkey_patched = True
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndDurationFromFields.__doc__ = "Mode when features have a field for start time and a field for event duration"
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromExpressions = Qgis.VectorTemporalMode.FeatureDateTimeStartAndEndFromExpressions
QgsVectorLayerTemporalProperties.TemporalMode.ModeFeatureDateTimeStartAndEndFromExpressions = Qgis.VectorTemporalMode.FeatureDateTimeStartAndEndFromExpressions
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromExpressions.is_monkey_patched = True
QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromExpressions.__doc__ = "Mode when features use expressions for start and end times"
QgsVectorLayerTemporalProperties.ModeRedrawLayerOnly = Qgis.VectorTemporalMode.RedrawLayerOnly
QgsVectorLayerTemporalProperties.TemporalMode.ModeRedrawLayerOnly = Qgis.VectorTemporalMode.RedrawLayerOnly
QgsVectorLayerTemporalProperties.ModeRedrawLayerOnly.is_monkey_patched = True
QgsVectorLayerTemporalProperties.ModeRedrawLayerOnly.__doc__ = "Redraw the layer when temporal range changes, but don't apply any filtering. Useful when symbology or rule based renderer expressions depend on the time range."
Qgis.VectorTemporalMode.__doc__ = "Vector layer temporal feature modes\n\n.. versionadded:: 3.22\n\n" + '* ``ModeFixedTemporalRange``: ' + Qgis.VectorTemporalMode.FixedTemporalRange.__doc__ + '\n' + '* ``ModeFeatureDateTimeInstantFromField``: ' + Qgis.VectorTemporalMode.FeatureDateTimeInstantFromField.__doc__ + '\n' + '* ``ModeFeatureDateTimeStartAndEndFromFields``: ' + Qgis.VectorTemporalMode.FeatureDateTimeStartAndEndFromFields.__doc__ + '\n' + '* ``ModeFeatureDateTimeStartAndDurationFromFields``: ' + Qgis.VectorTemporalMode.FeatureDateTimeStartAndDurationFromFields.__doc__ + '\n' + '* ``ModeFeatureDateTimeStartAndEndFromExpressions``: ' + Qgis.VectorTemporalMode.FeatureDateTimeStartAndEndFromExpressions.__doc__ + '\n' + '* ``ModeRedrawLayerOnly``: ' + Qgis.VectorTemporalMode.RedrawLayerOnly.__doc__
# --
Qgis.VectorTemporalMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd.__doc__ = "Default mode: include the Begin limit, but exclude the End limit"
Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd.__doc__ = "Mode to include both limits of the filtering timeframe"
Qgis.VectorTemporalLimitMode.__doc__ = "Mode for the handling of the limits of the filtering timeframe for vector features\n\n.. versionadded:: 3.22\n\n" + '* ``IncludeBeginExcludeEnd``: ' + Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd.__doc__ + '\n' + '* ``IncludeBeginIncludeEnd``: ' + Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd.__doc__
# --
Qgis.VectorTemporalLimitMode.baseClass = Qgis
QgsVectorDataProviderTemporalCapabilities.TemporalMode = Qgis.VectorDataProviderTemporalMode
# monkey patching scoped based enum
QgsVectorDataProviderTemporalCapabilities.ProviderHasFixedTemporalRange = Qgis.VectorDataProviderTemporalMode.HasFixedTemporalRange
QgsVectorDataProviderTemporalCapabilities.TemporalMode.ProviderHasFixedTemporalRange = Qgis.VectorDataProviderTemporalMode.HasFixedTemporalRange
QgsVectorDataProviderTemporalCapabilities.ProviderHasFixedTemporalRange.is_monkey_patched = True
QgsVectorDataProviderTemporalCapabilities.ProviderHasFixedTemporalRange.__doc__ = "Entire dataset from provider has a fixed start and end datetime."
QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeInstantInField = Qgis.VectorDataProviderTemporalMode.StoresFeatureDateTimeInstantInField
QgsVectorDataProviderTemporalCapabilities.TemporalMode.ProviderStoresFeatureDateTimeInstantInField = Qgis.VectorDataProviderTemporalMode.StoresFeatureDateTimeInstantInField
QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeInstantInField.is_monkey_patched = True
QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeInstantInField.__doc__ = "Dataset has feature datetime instants stored in a single field"
QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeStartAndEndInSeparateFields = Qgis.VectorDataProviderTemporalMode.StoresFeatureDateTimeStartAndEndInSeparateFields
QgsVectorDataProviderTemporalCapabilities.TemporalMode.ProviderStoresFeatureDateTimeStartAndEndInSeparateFields = Qgis.VectorDataProviderTemporalMode.StoresFeatureDateTimeStartAndEndInSeparateFields
QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeStartAndEndInSeparateFields.is_monkey_patched = True
QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeStartAndEndInSeparateFields.__doc__ = "Dataset stores feature start and end datetimes in separate fields"
Qgis.VectorDataProviderTemporalMode.__doc__ = "Vector data provider temporal handling modes.\n\n.. versionadded:: 3.22\n\n" + '* ``ProviderHasFixedTemporalRange``: ' + Qgis.VectorDataProviderTemporalMode.HasFixedTemporalRange.__doc__ + '\n' + '* ``ProviderStoresFeatureDateTimeInstantInField``: ' + Qgis.VectorDataProviderTemporalMode.StoresFeatureDateTimeInstantInField.__doc__ + '\n' + '* ``ProviderStoresFeatureDateTimeStartAndEndInSeparateFields``: ' + Qgis.VectorDataProviderTemporalMode.StoresFeatureDateTimeStartAndEndInSeparateFields.__doc__
# --
Qgis.VectorDataProviderTemporalMode.baseClass = Qgis
QgsRasterLayerTemporalProperties.TemporalMode = Qgis.RasterTemporalMode
# monkey patching scoped based enum
QgsRasterLayerTemporalProperties.ModeFixedTemporalRange = Qgis.RasterTemporalMode.FixedTemporalRange
QgsRasterLayerTemporalProperties.TemporalMode.ModeFixedTemporalRange = Qgis.RasterTemporalMode.FixedTemporalRange
QgsRasterLayerTemporalProperties.ModeFixedTemporalRange.is_monkey_patched = True
QgsRasterLayerTemporalProperties.ModeFixedTemporalRange.__doc__ = "Mode when temporal properties have fixed start and end datetimes."
QgsRasterLayerTemporalProperties.ModeTemporalRangeFromDataProvider = Qgis.RasterTemporalMode.TemporalRangeFromDataProvider
QgsRasterLayerTemporalProperties.TemporalMode.ModeTemporalRangeFromDataProvider = Qgis.RasterTemporalMode.TemporalRangeFromDataProvider
QgsRasterLayerTemporalProperties.ModeTemporalRangeFromDataProvider.is_monkey_patched = True
QgsRasterLayerTemporalProperties.ModeTemporalRangeFromDataProvider.__doc__ = "Mode when raster layer delegates temporal range handling to the dataprovider."
QgsRasterLayerTemporalProperties.ModeRedrawLayerOnly = Qgis.RasterTemporalMode.RedrawLayerOnly
QgsRasterLayerTemporalProperties.TemporalMode.ModeRedrawLayerOnly = Qgis.RasterTemporalMode.RedrawLayerOnly
QgsRasterLayerTemporalProperties.ModeRedrawLayerOnly.is_monkey_patched = True
QgsRasterLayerTemporalProperties.ModeRedrawLayerOnly.__doc__ = "Redraw the layer when temporal range changes, but don't apply any filtering. Useful when raster symbology expressions depend on the time range. (since QGIS 3.22)"
QgsRasterLayerTemporalProperties.FixedRangePerBand = Qgis.RasterTemporalMode.FixedRangePerBand
QgsRasterLayerTemporalProperties.FixedRangePerBand.is_monkey_patched = True
QgsRasterLayerTemporalProperties.FixedRangePerBand.__doc__ = "Layer has a fixed temporal range per band (since QGIS 3.38)"
Qgis.RasterTemporalMode.__doc__ = "Raster layer temporal modes\n\n.. versionadded:: 3.22\n\n" + '* ``ModeFixedTemporalRange``: ' + Qgis.RasterTemporalMode.FixedTemporalRange.__doc__ + '\n' + '* ``ModeTemporalRangeFromDataProvider``: ' + Qgis.RasterTemporalMode.TemporalRangeFromDataProvider.__doc__ + '\n' + '* ``ModeRedrawLayerOnly``: ' + Qgis.RasterTemporalMode.RedrawLayerOnly.__doc__ + '\n' + '* ``FixedRangePerBand``: ' + Qgis.RasterTemporalMode.FixedRangePerBand.__doc__
# --
Qgis.RasterTemporalMode.baseClass = Qgis
QgsRasterDataProviderTemporalCapabilities.IntervalHandlingMethod = Qgis.TemporalIntervalMatchMethod
# monkey patching scoped based enum
QgsRasterDataProviderTemporalCapabilities.MatchUsingWholeRange = Qgis.TemporalIntervalMatchMethod.MatchUsingWholeRange
QgsRasterDataProviderTemporalCapabilities.MatchUsingWholeRange.is_monkey_patched = True
QgsRasterDataProviderTemporalCapabilities.MatchUsingWholeRange.__doc__ = "Use an exact match to the whole temporal range"
QgsRasterDataProviderTemporalCapabilities.MatchExactUsingStartOfRange = Qgis.TemporalIntervalMatchMethod.MatchExactUsingStartOfRange
QgsRasterDataProviderTemporalCapabilities.MatchExactUsingStartOfRange.is_monkey_patched = True
QgsRasterDataProviderTemporalCapabilities.MatchExactUsingStartOfRange.__doc__ = "Match the start of the temporal range to a corresponding layer or band, and only use exact matching results"
QgsRasterDataProviderTemporalCapabilities.MatchExactUsingEndOfRange = Qgis.TemporalIntervalMatchMethod.MatchExactUsingEndOfRange
QgsRasterDataProviderTemporalCapabilities.MatchExactUsingEndOfRange.is_monkey_patched = True
QgsRasterDataProviderTemporalCapabilities.MatchExactUsingEndOfRange.__doc__ = "Match the end of the temporal range to a corresponding layer or band, and only use exact matching results"
QgsRasterDataProviderTemporalCapabilities.FindClosestMatchToStartOfRange = Qgis.TemporalIntervalMatchMethod.FindClosestMatchToStartOfRange
QgsRasterDataProviderTemporalCapabilities.FindClosestMatchToStartOfRange.is_monkey_patched = True
QgsRasterDataProviderTemporalCapabilities.FindClosestMatchToStartOfRange.__doc__ = "Match the start of the temporal range to the least previous closest datetime."
QgsRasterDataProviderTemporalCapabilities.FindClosestMatchToEndOfRange = Qgis.TemporalIntervalMatchMethod.FindClosestMatchToEndOfRange
QgsRasterDataProviderTemporalCapabilities.FindClosestMatchToEndOfRange.is_monkey_patched = True
QgsRasterDataProviderTemporalCapabilities.FindClosestMatchToEndOfRange.__doc__ = "Match the end of the temporal range to the least previous closest datetime."
Qgis.TemporalIntervalMatchMethod.__doc__ = "Method to use when resolving a temporal range to a data provider layer or band.\n\n.. versionadded:: 3.22\n\n" + '* ``MatchUsingWholeRange``: ' + Qgis.TemporalIntervalMatchMethod.MatchUsingWholeRange.__doc__ + '\n' + '* ``MatchExactUsingStartOfRange``: ' + Qgis.TemporalIntervalMatchMethod.MatchExactUsingStartOfRange.__doc__ + '\n' + '* ``MatchExactUsingEndOfRange``: ' + Qgis.TemporalIntervalMatchMethod.MatchExactUsingEndOfRange.__doc__ + '\n' + '* ``FindClosestMatchToStartOfRange``: ' + Qgis.TemporalIntervalMatchMethod.FindClosestMatchToStartOfRange.__doc__ + '\n' + '* ``FindClosestMatchToEndOfRange``: ' + Qgis.TemporalIntervalMatchMethod.FindClosestMatchToEndOfRange.__doc__
# --
Qgis.TemporalIntervalMatchMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RasterTemporalCapabilityFlag.RequestedTimesMustExactlyMatchAllAvailableTemporalRanges.__doc__ = "If present, indicates that the provider must only request temporal values which are exact matches for the values present in QgsRasterDataProviderTemporalCapabilities.allAvailableTemporalRanges()."
Qgis.RasterTemporalCapabilityFlag.__doc__ = "Flags for raster layer temporal capabilities.\n\n.. versionadded:: 3.28\n\n" + '* ``RequestedTimesMustExactlyMatchAllAvailableTemporalRanges``: ' + Qgis.RasterTemporalCapabilityFlag.RequestedTimesMustExactlyMatchAllAvailableTemporalRanges.__doc__
# --
Qgis.RasterTemporalCapabilityFlag.baseClass = Qgis
Qgis.RasterTemporalCapabilityFlags = lambda flags=0: Qgis.RasterTemporalCapabilityFlag(flags)
Qgis.RasterTemporalCapabilityFlags.baseClass = Qgis
RasterTemporalCapabilityFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsCoordinateTransform.TransformDirection = Qgis.TransformDirection
# monkey patching scoped based enum
QgsCoordinateTransform.ForwardTransform = Qgis.TransformDirection.Forward
QgsCoordinateTransform.TransformDirection.ForwardTransform = Qgis.TransformDirection.Forward
QgsCoordinateTransform.ForwardTransform.is_monkey_patched = True
QgsCoordinateTransform.ForwardTransform.__doc__ = "Forward transform (from source to destination)"
QgsCoordinateTransform.ReverseTransform = Qgis.TransformDirection.Reverse
QgsCoordinateTransform.TransformDirection.ReverseTransform = Qgis.TransformDirection.Reverse
QgsCoordinateTransform.ReverseTransform.is_monkey_patched = True
QgsCoordinateTransform.ReverseTransform.__doc__ = "Reverse/inverse transform (from destination to source)"
Qgis.TransformDirection.__doc__ = "Indicates the direction (forward or inverse) of a transform.\n\n.. versionadded:: 3.22\n\n" + '* ``ForwardTransform``: ' + Qgis.TransformDirection.Forward.__doc__ + '\n' + '* ``ReverseTransform``: ' + Qgis.TransformDirection.Reverse.__doc__
# --
Qgis.TransformDirection.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CoordinateTransformationFlag.BallparkTransformsAreAppropriate.__doc__ = "Indicates that approximate \"ballpark\" results are appropriate for this coordinate transform. See QgsCoordinateTransform.setBallparkTransformsAreAppropriate() for further details."
Qgis.CoordinateTransformationFlag.IgnoreImpossibleTransformations.__doc__ = "Indicates that impossible transformations (such as those which attempt to transform between two different celestial bodies) should be silently handled and marked as invalid. See QgsCoordinateTransform.isTransformationPossible() and QgsCoordinateTransform.isValid()."
Qgis.CoordinateTransformationFlag.__doc__ = "Flags which adjust the coordinate transformations behave.\n\n.. versionadded:: 3.26\n\n" + '* ``BallparkTransformsAreAppropriate``: ' + Qgis.CoordinateTransformationFlag.BallparkTransformsAreAppropriate.__doc__ + '\n' + '* ``IgnoreImpossibleTransformations``: ' + Qgis.CoordinateTransformationFlag.IgnoreImpossibleTransformations.__doc__
# --
Qgis.CoordinateTransformationFlag.baseClass = Qgis
Qgis.CoordinateTransformationFlags = lambda flags=0: Qgis.CoordinateTransformationFlag(flags)
Qgis.CoordinateTransformationFlags.baseClass = Qgis
CoordinateTransformationFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsMapSettings.Flag = Qgis.MapSettingsFlag
# monkey patching scoped based enum
QgsMapSettings.Antialiasing = Qgis.MapSettingsFlag.Antialiasing
QgsMapSettings.Antialiasing.is_monkey_patched = True
QgsMapSettings.Antialiasing.__doc__ = "Enable anti-aliasing for map rendering"
QgsMapSettings.DrawEditingInfo = Qgis.MapSettingsFlag.DrawEditingInfo
QgsMapSettings.DrawEditingInfo.is_monkey_patched = True
QgsMapSettings.DrawEditingInfo.__doc__ = "Enable drawing of vertex markers for layers in editing mode"
QgsMapSettings.ForceVectorOutput = Qgis.MapSettingsFlag.ForceVectorOutput
QgsMapSettings.ForceVectorOutput.is_monkey_patched = True
QgsMapSettings.ForceVectorOutput.__doc__ = "Vector graphics should not be cached and drawn as raster images"
QgsMapSettings.UseAdvancedEffects = Qgis.MapSettingsFlag.UseAdvancedEffects
QgsMapSettings.UseAdvancedEffects.is_monkey_patched = True
QgsMapSettings.UseAdvancedEffects.__doc__ = "Enable layer opacity and blending effects"
QgsMapSettings.DrawLabeling = Qgis.MapSettingsFlag.DrawLabeling
QgsMapSettings.DrawLabeling.is_monkey_patched = True
QgsMapSettings.DrawLabeling.__doc__ = "Enable drawing of labels on top of the map"
QgsMapSettings.UseRenderingOptimization = Qgis.MapSettingsFlag.UseRenderingOptimization
QgsMapSettings.UseRenderingOptimization.is_monkey_patched = True
QgsMapSettings.UseRenderingOptimization.__doc__ = "Enable vector simplification and other rendering optimizations"
QgsMapSettings.DrawSelection = Qgis.MapSettingsFlag.DrawSelection
QgsMapSettings.DrawSelection.is_monkey_patched = True
QgsMapSettings.DrawSelection.__doc__ = "Whether vector selections should be shown in the rendered map"
QgsMapSettings.DrawSymbolBounds = Qgis.MapSettingsFlag.DrawSymbolBounds
QgsMapSettings.DrawSymbolBounds.is_monkey_patched = True
QgsMapSettings.DrawSymbolBounds.__doc__ = "Draw bounds of symbols (for debugging/testing)"
QgsMapSettings.RenderMapTile = Qgis.MapSettingsFlag.RenderMapTile
QgsMapSettings.RenderMapTile.is_monkey_patched = True
QgsMapSettings.RenderMapTile.__doc__ = "Draw map such that there are no problems between adjacent tiles"
QgsMapSettings.RenderPartialOutput = Qgis.MapSettingsFlag.RenderPartialOutput
QgsMapSettings.RenderPartialOutput.is_monkey_patched = True
QgsMapSettings.RenderPartialOutput.__doc__ = "Whether to make extra effort to update map image with partially rendered layers (better for interactive map canvas). Added in QGIS 3.0"
QgsMapSettings.RenderPreviewJob = Qgis.MapSettingsFlag.RenderPreviewJob
QgsMapSettings.RenderPreviewJob.is_monkey_patched = True
QgsMapSettings.RenderPreviewJob.__doc__ = "Render is a 'canvas preview' render, and shortcuts should be taken to ensure fast rendering"
QgsMapSettings.RenderBlocking = Qgis.MapSettingsFlag.RenderBlocking
QgsMapSettings.RenderBlocking.is_monkey_patched = True
QgsMapSettings.RenderBlocking.__doc__ = "Render and load remote sources in the same thread to ensure rendering remote sources (svg and images). WARNING: this flag must NEVER be used from GUI based applications (like the main QGIS application) or crashes will result. Only for use in external scripts or QGIS server."
QgsMapSettings.LosslessImageRendering = Qgis.MapSettingsFlag.LosslessImageRendering
QgsMapSettings.LosslessImageRendering.is_monkey_patched = True
QgsMapSettings.LosslessImageRendering.__doc__ = "Render images losslessly whenever possible, instead of the default lossy jpeg rendering used for some destination devices (e.g. PDF). This flag only works with builds based on Qt 5.13 or later."
QgsMapSettings.Render3DMap = Qgis.MapSettingsFlag.Render3DMap
QgsMapSettings.Render3DMap.is_monkey_patched = True
QgsMapSettings.Render3DMap.__doc__ = "Render is for a 3D map"
QgsMapSettings.HighQualityImageTransforms = Qgis.MapSettingsFlag.HighQualityImageTransforms
QgsMapSettings.HighQualityImageTransforms.is_monkey_patched = True
QgsMapSettings.HighQualityImageTransforms.__doc__ = "Enable high quality image transformations, which results in better appearance of scaled or rotated raster components of a map (since QGIS 3.24)"
QgsMapSettings.SkipSymbolRendering = Qgis.MapSettingsFlag.SkipSymbolRendering
QgsMapSettings.SkipSymbolRendering.is_monkey_patched = True
QgsMapSettings.SkipSymbolRendering.__doc__ = "Disable symbol rendering while still drawing labels if enabled (since QGIS 3.24)"
QgsMapSettings.ForceRasterMasks = Qgis.MapSettingsFlag.ForceRasterMasks
QgsMapSettings.ForceRasterMasks.is_monkey_patched = True
QgsMapSettings.ForceRasterMasks.__doc__ = "Force symbol masking to be applied using a raster method. This is considerably faster when compared to the vector method, but results in a inferior quality output. (since QGIS 3.26.1)"
QgsMapSettings.RecordProfile = Qgis.MapSettingsFlag.RecordProfile
QgsMapSettings.RecordProfile.is_monkey_patched = True
QgsMapSettings.RecordProfile.__doc__ = "Enable run-time profiling while rendering (since QGIS 3.34)"
Qgis.MapSettingsFlag.__doc__ = "Flags which adjust the way maps are rendered.\n\n.. versionadded:: 3.22\n\n" + '* ``Antialiasing``: ' + Qgis.MapSettingsFlag.Antialiasing.__doc__ + '\n' + '* ``DrawEditingInfo``: ' + Qgis.MapSettingsFlag.DrawEditingInfo.__doc__ + '\n' + '* ``ForceVectorOutput``: ' + Qgis.MapSettingsFlag.ForceVectorOutput.__doc__ + '\n' + '* ``UseAdvancedEffects``: ' + Qgis.MapSettingsFlag.UseAdvancedEffects.__doc__ + '\n' + '* ``DrawLabeling``: ' + Qgis.MapSettingsFlag.DrawLabeling.__doc__ + '\n' + '* ``UseRenderingOptimization``: ' + Qgis.MapSettingsFlag.UseRenderingOptimization.__doc__ + '\n' + '* ``DrawSelection``: ' + Qgis.MapSettingsFlag.DrawSelection.__doc__ + '\n' + '* ``DrawSymbolBounds``: ' + Qgis.MapSettingsFlag.DrawSymbolBounds.__doc__ + '\n' + '* ``RenderMapTile``: ' + Qgis.MapSettingsFlag.RenderMapTile.__doc__ + '\n' + '* ``RenderPartialOutput``: ' + Qgis.MapSettingsFlag.RenderPartialOutput.__doc__ + '\n' + '* ``RenderPreviewJob``: ' + Qgis.MapSettingsFlag.RenderPreviewJob.__doc__ + '\n' + '* ``RenderBlocking``: ' + Qgis.MapSettingsFlag.RenderBlocking.__doc__ + '\n' + '* ``LosslessImageRendering``: ' + Qgis.MapSettingsFlag.LosslessImageRendering.__doc__ + '\n' + '* ``Render3DMap``: ' + Qgis.MapSettingsFlag.Render3DMap.__doc__ + '\n' + '* ``HighQualityImageTransforms``: ' + Qgis.MapSettingsFlag.HighQualityImageTransforms.__doc__ + '\n' + '* ``SkipSymbolRendering``: ' + Qgis.MapSettingsFlag.SkipSymbolRendering.__doc__ + '\n' + '* ``ForceRasterMasks``: ' + Qgis.MapSettingsFlag.ForceRasterMasks.__doc__ + '\n' + '* ``RecordProfile``: ' + Qgis.MapSettingsFlag.RecordProfile.__doc__
# --
Qgis.MapSettingsFlags = lambda flags=0: Qgis.MapSettingsFlag(flags)
QgsMapSettings.Flags = Qgis.MapSettingsFlags
Qgis.MapSettingsFlag.baseClass = Qgis
Qgis.MapSettingsFlags.baseClass = Qgis
MapSettingsFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsRenderContext.Flag = Qgis.RenderContextFlag
# monkey patching scoped based enum
QgsRenderContext.DrawEditingInfo = Qgis.RenderContextFlag.DrawEditingInfo
QgsRenderContext.DrawEditingInfo.is_monkey_patched = True
QgsRenderContext.DrawEditingInfo.__doc__ = "Enable drawing of vertex markers for layers in editing mode"
QgsRenderContext.ForceVectorOutput = Qgis.RenderContextFlag.ForceVectorOutput
QgsRenderContext.ForceVectorOutput.is_monkey_patched = True
QgsRenderContext.ForceVectorOutput.__doc__ = "Vector graphics should not be cached and drawn as raster images"
QgsRenderContext.UseAdvancedEffects = Qgis.RenderContextFlag.UseAdvancedEffects
QgsRenderContext.UseAdvancedEffects.is_monkey_patched = True
QgsRenderContext.UseAdvancedEffects.__doc__ = "Enable layer opacity and blending effects"
QgsRenderContext.UseRenderingOptimization = Qgis.RenderContextFlag.UseRenderingOptimization
QgsRenderContext.UseRenderingOptimization.is_monkey_patched = True
QgsRenderContext.UseRenderingOptimization.__doc__ = "Enable vector simplification and other rendering optimizations"
QgsRenderContext.DrawSelection = Qgis.RenderContextFlag.DrawSelection
QgsRenderContext.DrawSelection.is_monkey_patched = True
QgsRenderContext.DrawSelection.__doc__ = "Whether vector selections should be shown in the rendered map"
QgsRenderContext.DrawSymbolBounds = Qgis.RenderContextFlag.DrawSymbolBounds
QgsRenderContext.DrawSymbolBounds.is_monkey_patched = True
QgsRenderContext.DrawSymbolBounds.__doc__ = "Draw bounds of symbols (for debugging/testing)"
QgsRenderContext.RenderMapTile = Qgis.RenderContextFlag.RenderMapTile
QgsRenderContext.RenderMapTile.is_monkey_patched = True
QgsRenderContext.RenderMapTile.__doc__ = "Draw map such that there are no problems between adjacent tiles"
QgsRenderContext.Antialiasing = Qgis.RenderContextFlag.Antialiasing
QgsRenderContext.Antialiasing.is_monkey_patched = True
QgsRenderContext.Antialiasing.__doc__ = "Use antialiasing while drawing"
QgsRenderContext.RenderPartialOutput = Qgis.RenderContextFlag.RenderPartialOutput
QgsRenderContext.RenderPartialOutput.is_monkey_patched = True
QgsRenderContext.RenderPartialOutput.__doc__ = "Whether to make extra effort to update map image with partially rendered layers (better for interactive map canvas). Added in QGIS 3.0"
QgsRenderContext.RenderPreviewJob = Qgis.RenderContextFlag.RenderPreviewJob
QgsRenderContext.RenderPreviewJob.is_monkey_patched = True
QgsRenderContext.RenderPreviewJob.__doc__ = "Render is a 'canvas preview' render, and shortcuts should be taken to ensure fast rendering"
QgsRenderContext.RenderBlocking = Qgis.RenderContextFlag.RenderBlocking
QgsRenderContext.RenderBlocking.is_monkey_patched = True
QgsRenderContext.RenderBlocking.__doc__ = "Render and load remote sources in the same thread to ensure rendering remote sources (svg and images). WARNING: this flag must NEVER be used from GUI based applications (like the main QGIS application) or crashes will result. Only for use in external scripts or QGIS server."
QgsRenderContext.RenderSymbolPreview = Qgis.RenderContextFlag.RenderSymbolPreview
QgsRenderContext.RenderSymbolPreview.is_monkey_patched = True
QgsRenderContext.RenderSymbolPreview.__doc__ = "The render is for a symbol preview only and map based properties may not be available, so care should be taken to handle map unit based sizes in an appropriate way."
QgsRenderContext.LosslessImageRendering = Qgis.RenderContextFlag.LosslessImageRendering
QgsRenderContext.LosslessImageRendering.is_monkey_patched = True
QgsRenderContext.LosslessImageRendering.__doc__ = "Render images losslessly whenever possible, instead of the default lossy jpeg rendering used for some destination devices (e.g. PDF). This flag only works with builds based on Qt 5.13 or later."
QgsRenderContext.ApplyScalingWorkaroundForTextRendering = Qgis.RenderContextFlag.ApplyScalingWorkaroundForTextRendering
QgsRenderContext.ApplyScalingWorkaroundForTextRendering.is_monkey_patched = True
QgsRenderContext.ApplyScalingWorkaroundForTextRendering.__doc__ = "Whether a scaling workaround designed to stablise the rendering of small font sizes (or for painters scaled out by a large amount) when rendering text. Generally this is recommended, but it may incur some performance cost."
QgsRenderContext.Render3DMap = Qgis.RenderContextFlag.Render3DMap
QgsRenderContext.Render3DMap.is_monkey_patched = True
QgsRenderContext.Render3DMap.__doc__ = "Render is for a 3D map"
QgsRenderContext.ApplyClipAfterReprojection = Qgis.RenderContextFlag.ApplyClipAfterReprojection
QgsRenderContext.ApplyClipAfterReprojection.is_monkey_patched = True
QgsRenderContext.ApplyClipAfterReprojection.__doc__ = "Feature geometry clipping to mapExtent() must be performed after the geometries are transformed using coordinateTransform(). Usually feature geometry clipping occurs using the extent() in the layer's CRS prior to geometry transformation, but in some cases when extent() could not be accurately calculated it is necessary to clip geometries to mapExtent() AFTER transforming them using coordinateTransform()."
QgsRenderContext.RenderingSubSymbol = Qgis.RenderContextFlag.RenderingSubSymbol
QgsRenderContext.RenderingSubSymbol.is_monkey_patched = True
QgsRenderContext.RenderingSubSymbol.__doc__ = "Set whenever a sub-symbol of a parent symbol is currently being rendered. Can be used during symbol and symbol layer rendering to determine whether the symbol being rendered is a subsymbol. (Since QGIS 3.24)"
QgsRenderContext.HighQualityImageTransforms = Qgis.RenderContextFlag.HighQualityImageTransforms
QgsRenderContext.HighQualityImageTransforms.is_monkey_patched = True
QgsRenderContext.HighQualityImageTransforms.__doc__ = "Enable high quality image transformations, which results in better appearance of scaled or rotated raster components of a map (since QGIS 3.24)"
QgsRenderContext.SkipSymbolRendering = Qgis.RenderContextFlag.SkipSymbolRendering
QgsRenderContext.SkipSymbolRendering.is_monkey_patched = True
QgsRenderContext.SkipSymbolRendering.__doc__ = "Disable symbol rendering while still drawing labels if enabled (since QGIS 3.24)"
QgsRenderContext.RecordProfile = Qgis.RenderContextFlag.RecordProfile
QgsRenderContext.RecordProfile.is_monkey_patched = True
QgsRenderContext.RecordProfile.__doc__ = "Enable run-time profiling while rendering (since QGIS 3.34)"
Qgis.RenderContextFlag.__doc__ = "Flags which affect rendering operations.\n\n.. versionadded:: 3.22\n\n" + '* ``DrawEditingInfo``: ' + Qgis.RenderContextFlag.DrawEditingInfo.__doc__ + '\n' + '* ``ForceVectorOutput``: ' + Qgis.RenderContextFlag.ForceVectorOutput.__doc__ + '\n' + '* ``UseAdvancedEffects``: ' + Qgis.RenderContextFlag.UseAdvancedEffects.__doc__ + '\n' + '* ``UseRenderingOptimization``: ' + Qgis.RenderContextFlag.UseRenderingOptimization.__doc__ + '\n' + '* ``DrawSelection``: ' + Qgis.RenderContextFlag.DrawSelection.__doc__ + '\n' + '* ``DrawSymbolBounds``: ' + Qgis.RenderContextFlag.DrawSymbolBounds.__doc__ + '\n' + '* ``RenderMapTile``: ' + Qgis.RenderContextFlag.RenderMapTile.__doc__ + '\n' + '* ``Antialiasing``: ' + Qgis.RenderContextFlag.Antialiasing.__doc__ + '\n' + '* ``RenderPartialOutput``: ' + Qgis.RenderContextFlag.RenderPartialOutput.__doc__ + '\n' + '* ``RenderPreviewJob``: ' + Qgis.RenderContextFlag.RenderPreviewJob.__doc__ + '\n' + '* ``RenderBlocking``: ' + Qgis.RenderContextFlag.RenderBlocking.__doc__ + '\n' + '* ``RenderSymbolPreview``: ' + Qgis.RenderContextFlag.RenderSymbolPreview.__doc__ + '\n' + '* ``LosslessImageRendering``: ' + Qgis.RenderContextFlag.LosslessImageRendering.__doc__ + '\n' + '* ``ApplyScalingWorkaroundForTextRendering``: ' + Qgis.RenderContextFlag.ApplyScalingWorkaroundForTextRendering.__doc__ + '\n' + '* ``Render3DMap``: ' + Qgis.RenderContextFlag.Render3DMap.__doc__ + '\n' + '* ``ApplyClipAfterReprojection``: ' + Qgis.RenderContextFlag.ApplyClipAfterReprojection.__doc__ + '\n' + '* ``RenderingSubSymbol``: ' + Qgis.RenderContextFlag.RenderingSubSymbol.__doc__ + '\n' + '* ``HighQualityImageTransforms``: ' + Qgis.RenderContextFlag.HighQualityImageTransforms.__doc__ + '\n' + '* ``SkipSymbolRendering``: ' + Qgis.RenderContextFlag.SkipSymbolRendering.__doc__ + '\n' + '* ``RecordProfile``: ' + Qgis.RenderContextFlag.RecordProfile.__doc__
# --
Qgis.RenderContextFlags = lambda flags=0: Qgis.RenderContextFlag(flags)
QgsRenderContext.Flags = Qgis.RenderContextFlags
Qgis.RenderContextFlag.baseClass = Qgis
Qgis.RenderContextFlags.baseClass = Qgis
RenderContextFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.MapLayerRendererFlag.RenderPartialOutputs.__doc__ = "The renderer benefits from rendering temporary in-progress preview renders. These are temporary results which will be used for the layer during rendering in-progress compositions, which will differ from the final layer render. They can be used for showing overlays or other information to users which help inform them about what is actually occurring during a slow layer render, but where these overlays and additional content is not wanted in the final layer renders. Another use case is rendering unsorted results as soon as they are available, before doing a final sorted render of the entire layer contents."
Qgis.MapLayerRendererFlag.RenderPartialOutputOverPreviousCachedImage.__doc__ = "When rendering temporary in-progress preview renders, these preview renders can be drawn over any previously cached layer render we have for the same region. This can allow eg a low-resolution zoomed in version of the last map render to be used as a base painting surface to overdraw with incremental preview render outputs. If not set, an empty image will be used as the starting point for the render preview image."
Qgis.MapLayerRendererFlag.__doc__ = "Flags which control how map layer renderers behave.\n\n.. versionadded:: 3.34\n\n" + '* ``RenderPartialOutputs``: ' + Qgis.MapLayerRendererFlag.RenderPartialOutputs.__doc__ + '\n' + '* ``RenderPartialOutputOverPreviousCachedImage``: ' + Qgis.MapLayerRendererFlag.RenderPartialOutputOverPreviousCachedImage.__doc__
# --
Qgis.MapLayerRendererFlag.baseClass = Qgis
Qgis.MapLayerRendererFlags = lambda flags=0: Qgis.MapLayerRendererFlag(flags)
Qgis.MapLayerRendererFlags.baseClass = Qgis
MapLayerRendererFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsRenderContext.TextRenderFormat = Qgis.TextRenderFormat
# monkey patching scoped based enum
QgsRenderContext.TextFormatAlwaysOutlines = Qgis.TextRenderFormat.AlwaysOutlines
QgsRenderContext.TextRenderFormat.TextFormatAlwaysOutlines = Qgis.TextRenderFormat.AlwaysOutlines
QgsRenderContext.TextFormatAlwaysOutlines.is_monkey_patched = True
QgsRenderContext.TextFormatAlwaysOutlines.__doc__ = "Always render text using path objects (AKA outlines/curves). This setting guarantees the best quality rendering, even when using a raster paint surface (where sub-pixel path based text rendering is superior to sub-pixel text-based rendering). The downside is that text is converted to paths only, so users cannot open created vector outputs for post-processing in other applications and retain text editability.  This setting also guarantees complete compatibility with the full range of formatting options available through QgsTextRenderer and QgsTextFormat, some of which may not be possible to reproduce when using a vector-based paint surface and TextFormatAlwaysText mode. A final benefit to this setting is that vector exports created using text as outlines do not require all users to have the original fonts installed in order to display the text in its original style."
QgsRenderContext.TextFormatAlwaysText = Qgis.TextRenderFormat.AlwaysText
QgsRenderContext.TextRenderFormat.TextFormatAlwaysText = Qgis.TextRenderFormat.AlwaysText
QgsRenderContext.TextFormatAlwaysText.is_monkey_patched = True
QgsRenderContext.TextFormatAlwaysText.__doc__ = "Always render text as text objects. While this mode preserves text objects as text for post-processing in external vector editing applications, it can result in rendering artifacts or poor quality rendering, depending on the text format settings. Even with raster based paint devices, TextFormatAlwaysText can result in inferior rendering quality to TextFormatAlwaysOutlines. When rendering using TextFormatAlwaysText to a vector based device (e.g. PDF or SVG), care must be taken to ensure that the required fonts are available to users when opening the created files, or default fallback fonts will be used to display the output instead. (Although PDF exports MAY automatically embed some fonts when possible, depending on the user's platform)."
Qgis.TextRenderFormat.__doc__ = "Options for rendering text.\n\n.. versionadded:: 3.22\n\n" + '* ``TextFormatAlwaysOutlines``: ' + Qgis.TextRenderFormat.AlwaysOutlines.__doc__ + '\n' + '* ``TextFormatAlwaysText``: ' + Qgis.TextRenderFormat.AlwaysText.__doc__
# --
Qgis.TextRenderFormat.baseClass = Qgis
QgsLabelingEngineSettings.Flag = Qgis.LabelingFlag
# monkey patching scoped based enum
QgsLabelingEngineSettings.UseAllLabels = Qgis.LabelingFlag.UseAllLabels
QgsLabelingEngineSettings.UseAllLabels.is_monkey_patched = True
QgsLabelingEngineSettings.UseAllLabels.__doc__ = "Whether to draw all labels even if there would be collisions"
QgsLabelingEngineSettings.UsePartialCandidates = Qgis.LabelingFlag.UsePartialCandidates
QgsLabelingEngineSettings.UsePartialCandidates.is_monkey_patched = True
QgsLabelingEngineSettings.UsePartialCandidates.__doc__ = "Whether to use also label candidates that are partially outside of the map view"
QgsLabelingEngineSettings.RenderOutlineLabels = Qgis.LabelingFlag.RenderOutlineLabels
QgsLabelingEngineSettings.RenderOutlineLabels.is_monkey_patched = True
QgsLabelingEngineSettings.RenderOutlineLabels.__doc__ = "Whether to render labels as text or outlines. Deprecated and of QGIS 3.4.3 - use defaultTextRenderFormat() instead."
QgsLabelingEngineSettings.DrawLabelRectOnly = Qgis.LabelingFlag.DrawLabelRectOnly
QgsLabelingEngineSettings.DrawLabelRectOnly.is_monkey_patched = True
QgsLabelingEngineSettings.DrawLabelRectOnly.__doc__ = "Whether to only draw the label rect and not the actual label text (used for unit tests)"
QgsLabelingEngineSettings.DrawCandidates = Qgis.LabelingFlag.DrawCandidates
QgsLabelingEngineSettings.DrawCandidates.is_monkey_patched = True
QgsLabelingEngineSettings.DrawCandidates.__doc__ = "Whether to draw rectangles of generated candidates (good for debugging)"
QgsLabelingEngineSettings.DrawUnplacedLabels = Qgis.LabelingFlag.DrawUnplacedLabels
QgsLabelingEngineSettings.DrawUnplacedLabels.is_monkey_patched = True
QgsLabelingEngineSettings.DrawUnplacedLabels.__doc__ = "Whether to render unplaced labels as an indicator/warning for users"
QgsLabelingEngineSettings.CollectUnplacedLabels = Qgis.LabelingFlag.CollectUnplacedLabels
QgsLabelingEngineSettings.CollectUnplacedLabels.is_monkey_patched = True
QgsLabelingEngineSettings.CollectUnplacedLabels.__doc__ = "Whether unplaced labels should be collected in the labeling results (regardless of whether they are being rendered). Since QGIS 3.20"
QgsLabelingEngineSettings.DrawLabelMetrics = Qgis.LabelingFlag.DrawLabelMetrics
QgsLabelingEngineSettings.DrawLabelMetrics.is_monkey_patched = True
QgsLabelingEngineSettings.DrawLabelMetrics.__doc__ = "Whether to render label metric guides (for debugging). Since QGIS 3.30"
Qgis.LabelingFlag.__doc__ = "Various flags that affect drawing and placement of labels.\n\nPrior to QGIS 3.30 this was available as :py:class:`QgsLabelingEngineSettings`.Flag\n\n.. versionadded:: 3.30\n\n" + '* ``UseAllLabels``: ' + Qgis.LabelingFlag.UseAllLabels.__doc__ + '\n' + '* ``UsePartialCandidates``: ' + Qgis.LabelingFlag.UsePartialCandidates.__doc__ + '\n' + '* ``RenderOutlineLabels``: ' + Qgis.LabelingFlag.RenderOutlineLabels.__doc__ + '\n' + '* ``DrawLabelRectOnly``: ' + Qgis.LabelingFlag.DrawLabelRectOnly.__doc__ + '\n' + '* ``DrawCandidates``: ' + Qgis.LabelingFlag.DrawCandidates.__doc__ + '\n' + '* ``DrawUnplacedLabels``: ' + Qgis.LabelingFlag.DrawUnplacedLabels.__doc__ + '\n' + '* ``CollectUnplacedLabels``: ' + Qgis.LabelingFlag.CollectUnplacedLabels.__doc__ + '\n' + '* ``DrawLabelMetrics``: ' + Qgis.LabelingFlag.DrawLabelMetrics.__doc__
# --
Qgis.LabelingFlag.baseClass = Qgis
Qgis.LabelingFlags = lambda flags=0: Qgis.LabelingFlag(flags)
QgsLabelingEngineSettings.Flags = Qgis.LabelingFlags
Qgis.LabelingFlags.baseClass = Qgis
LabelingFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsLabelingEngineSettings.PlacementEngineVersion = Qgis.LabelPlacementEngineVersion
# monkey patching scoped based enum
QgsLabelingEngineSettings.PlacementEngineVersion1 = Qgis.LabelPlacementEngineVersion.Version1
QgsLabelingEngineSettings.PlacementEngineVersion.PlacementEngineVersion1 = Qgis.LabelPlacementEngineVersion.Version1
QgsLabelingEngineSettings.PlacementEngineVersion1.is_monkey_patched = True
QgsLabelingEngineSettings.PlacementEngineVersion1.__doc__ = "Version 1, matches placement from QGIS <= 3.10.1"
QgsLabelingEngineSettings.PlacementEngineVersion2 = Qgis.LabelPlacementEngineVersion.Version2
QgsLabelingEngineSettings.PlacementEngineVersion.PlacementEngineVersion2 = Qgis.LabelPlacementEngineVersion.Version2
QgsLabelingEngineSettings.PlacementEngineVersion2.is_monkey_patched = True
QgsLabelingEngineSettings.PlacementEngineVersion2.__doc__ = "Version 2 (default for new projects since QGIS 3.12)"
Qgis.LabelPlacementEngineVersion.__doc__ = "Labeling placement engine version.\n\nPrior to QGIS 3.30 this was available as :py:class:`QgsLabelingEngineSettings`.PlacementEngineVersion\n\n.. versionadded:: 3.30\n\n" + '* ``PlacementEngineVersion1``: ' + Qgis.LabelPlacementEngineVersion.Version1.__doc__ + '\n' + '* ``PlacementEngineVersion2``: ' + Qgis.LabelPlacementEngineVersion.Version2.__doc__
# --
Qgis.LabelPlacementEngineVersion.baseClass = Qgis
QgsTextFormat.TextOrientation = Qgis.TextOrientation
# monkey patching scoped based enum
QgsTextFormat.HorizontalOrientation = Qgis.TextOrientation.Horizontal
QgsTextFormat.TextOrientation.HorizontalOrientation = Qgis.TextOrientation.Horizontal
QgsTextFormat.HorizontalOrientation.is_monkey_patched = True
QgsTextFormat.HorizontalOrientation.__doc__ = "Horizontally oriented text"
QgsTextFormat.VerticalOrientation = Qgis.TextOrientation.Vertical
QgsTextFormat.TextOrientation.VerticalOrientation = Qgis.TextOrientation.Vertical
QgsTextFormat.VerticalOrientation.is_monkey_patched = True
QgsTextFormat.VerticalOrientation.__doc__ = "Vertically oriented text"
QgsTextFormat.RotationBasedOrientation = Qgis.TextOrientation.RotationBased
QgsTextFormat.TextOrientation.RotationBasedOrientation = Qgis.TextOrientation.RotationBased
QgsTextFormat.RotationBasedOrientation.is_monkey_patched = True
QgsTextFormat.RotationBasedOrientation.__doc__ = "Horizontally or vertically oriented text based on rotation (only available for map labeling)"
Qgis.TextOrientation.__doc__ = "Text orientations.\n\n.. note::\n\n   Prior to QGIS 3.28 this was available as :py:class:`QgsTextFormat`.TextOrientation\n\n.. versionadded:: 3.28\n\n" + '* ``HorizontalOrientation``: ' + Qgis.TextOrientation.Horizontal.__doc__ + '\n' + '* ``VerticalOrientation``: ' + Qgis.TextOrientation.Vertical.__doc__ + '\n' + '* ``RotationBasedOrientation``: ' + Qgis.TextOrientation.RotationBased.__doc__
# --
Qgis.TextOrientation.baseClass = Qgis
QgsTextRenderer.DrawMode = Qgis.TextLayoutMode
# monkey patching scoped based enum
QgsTextRenderer.Rect = Qgis.TextLayoutMode.Rectangle
QgsTextRenderer.DrawMode.Rect = Qgis.TextLayoutMode.Rectangle
QgsTextRenderer.Rect.is_monkey_patched = True
QgsTextRenderer.Rect.__doc__ = "Text within rectangle layout mode"
QgsTextRenderer.Point = Qgis.TextLayoutMode.Point
QgsTextRenderer.Point.is_monkey_patched = True
QgsTextRenderer.Point.__doc__ = "Text at point of origin layout mode"
QgsTextRenderer.Label = Qgis.TextLayoutMode.Labeling
QgsTextRenderer.DrawMode.Label = Qgis.TextLayoutMode.Labeling
QgsTextRenderer.Label.is_monkey_patched = True
QgsTextRenderer.Label.__doc__ = "Labeling-specific layout mode"
QgsTextRenderer.RectangleCapHeightBased = Qgis.TextLayoutMode.RectangleCapHeightBased
QgsTextRenderer.RectangleCapHeightBased.is_monkey_patched = True
QgsTextRenderer.RectangleCapHeightBased.__doc__ = "Similar to Rectangle mode, but uses cap height only when calculating font heights for the first line of text, and cap height + descent for subsequent lines of text (since QGIS 3.30)"
QgsTextRenderer.RectangleAscentBased = Qgis.TextLayoutMode.RectangleAscentBased
QgsTextRenderer.RectangleAscentBased.is_monkey_patched = True
QgsTextRenderer.RectangleAscentBased.__doc__ = "Similar to Rectangle mode, but uses ascents only when calculating font and line heights. (since QGIS 3.30)"
Qgis.TextLayoutMode.__doc__ = "Text layout modes.\n\n.. note::\n\n   Prior to QGIS 3.28 this was available as :py:class:`QgsTextRenderer`.DrawMode\n\n.. versionadded:: 3.28\n\n" + '* ``Rect``: ' + Qgis.TextLayoutMode.Rectangle.__doc__ + '\n' + '* ``Point``: ' + Qgis.TextLayoutMode.Point.__doc__ + '\n' + '* ``Label``: ' + Qgis.TextLayoutMode.Labeling.__doc__ + '\n' + '* ``RectangleCapHeightBased``: ' + Qgis.TextLayoutMode.RectangleCapHeightBased.__doc__ + '\n' + '* ``RectangleAscentBased``: ' + Qgis.TextLayoutMode.RectangleAscentBased.__doc__
# --
Qgis.TextLayoutMode.baseClass = Qgis
QgsTextRenderer.TextPart = Qgis.TextComponent
# monkey patching scoped based enum
QgsTextRenderer.Text = Qgis.TextComponent.Text
QgsTextRenderer.Text.is_monkey_patched = True
QgsTextRenderer.Text.__doc__ = "Text component"
QgsTextRenderer.Buffer = Qgis.TextComponent.Buffer
QgsTextRenderer.Buffer.is_monkey_patched = True
QgsTextRenderer.Buffer.__doc__ = "Buffer component"
QgsTextRenderer.Background = Qgis.TextComponent.Background
QgsTextRenderer.Background.is_monkey_patched = True
QgsTextRenderer.Background.__doc__ = "Background shape"
QgsTextRenderer.Shadow = Qgis.TextComponent.Shadow
QgsTextRenderer.Shadow.is_monkey_patched = True
QgsTextRenderer.Shadow.__doc__ = "Drop shadow"
Qgis.TextComponent.__doc__ = "Text components.\n\n.. note::\n\n   Prior to QGIS 3.28 this was available as :py:class:`QgsTextRenderer`.TextPart\n\n.. versionadded:: 3.28\n\n" + '* ``Text``: ' + Qgis.TextComponent.Text.__doc__ + '\n' + '* ``Buffer``: ' + Qgis.TextComponent.Buffer.__doc__ + '\n' + '* ``Background``: ' + Qgis.TextComponent.Background.__doc__ + '\n' + '* ``Shadow``: ' + Qgis.TextComponent.Shadow.__doc__
# --
Qgis.TextComponent.baseClass = Qgis
QgsTextRenderer.HAlignment = Qgis.TextHorizontalAlignment
# monkey patching scoped based enum
QgsTextRenderer.AlignLeft = Qgis.TextHorizontalAlignment.Left
QgsTextRenderer.HAlignment.AlignLeft = Qgis.TextHorizontalAlignment.Left
QgsTextRenderer.AlignLeft.is_monkey_patched = True
QgsTextRenderer.AlignLeft.__doc__ = "Left align"
QgsTextRenderer.AlignCenter = Qgis.TextHorizontalAlignment.Center
QgsTextRenderer.HAlignment.AlignCenter = Qgis.TextHorizontalAlignment.Center
QgsTextRenderer.AlignCenter.is_monkey_patched = True
QgsTextRenderer.AlignCenter.__doc__ = "Center align"
QgsTextRenderer.AlignRight = Qgis.TextHorizontalAlignment.Right
QgsTextRenderer.HAlignment.AlignRight = Qgis.TextHorizontalAlignment.Right
QgsTextRenderer.AlignRight.is_monkey_patched = True
QgsTextRenderer.AlignRight.__doc__ = "Right align"
QgsTextRenderer.AlignJustify = Qgis.TextHorizontalAlignment.Justify
QgsTextRenderer.HAlignment.AlignJustify = Qgis.TextHorizontalAlignment.Justify
QgsTextRenderer.AlignJustify.is_monkey_patched = True
QgsTextRenderer.AlignJustify.__doc__ = "Justify align"
Qgis.TextHorizontalAlignment.__doc__ = "Text horizontal alignment.\n\n.. note::\n\n   Prior to QGIS 3.28 this was available as :py:class:`QgsTextRenderer`.HAlignment\n\n.. versionadded:: 3.28\n\n" + '* ``AlignLeft``: ' + Qgis.TextHorizontalAlignment.Left.__doc__ + '\n' + '* ``AlignCenter``: ' + Qgis.TextHorizontalAlignment.Center.__doc__ + '\n' + '* ``AlignRight``: ' + Qgis.TextHorizontalAlignment.Right.__doc__ + '\n' + '* ``AlignJustify``: ' + Qgis.TextHorizontalAlignment.Justify.__doc__
# --
Qgis.TextHorizontalAlignment.baseClass = Qgis
QgsTextRenderer.VAlignment = Qgis.TextVerticalAlignment
# monkey patching scoped based enum
QgsTextRenderer.AlignTop = Qgis.TextVerticalAlignment.Top
QgsTextRenderer.VAlignment.AlignTop = Qgis.TextVerticalAlignment.Top
QgsTextRenderer.AlignTop.is_monkey_patched = True
QgsTextRenderer.AlignTop.__doc__ = "Align to top"
QgsTextRenderer.AlignVCenter = Qgis.TextVerticalAlignment.VerticalCenter
QgsTextRenderer.VAlignment.AlignVCenter = Qgis.TextVerticalAlignment.VerticalCenter
QgsTextRenderer.AlignVCenter.is_monkey_patched = True
QgsTextRenderer.AlignVCenter.__doc__ = "Center align"
QgsTextRenderer.AlignBottom = Qgis.TextVerticalAlignment.Bottom
QgsTextRenderer.VAlignment.AlignBottom = Qgis.TextVerticalAlignment.Bottom
QgsTextRenderer.AlignBottom.is_monkey_patched = True
QgsTextRenderer.AlignBottom.__doc__ = "Align to bottom"
Qgis.TextVerticalAlignment.__doc__ = "Text vertical alignment.\n\nThis enum controls vertical alignment of text in a predefined rectangular\nbounding box. See also Qgis.TextCharacterVerticalAlignment.\n\n.. note::\n\n   Prior to QGIS 3.28 this was available as :py:class:`QgsTextRenderer`.VAlignment\n\n.. versionadded:: 3.28\n\n" + '* ``AlignTop``: ' + Qgis.TextVerticalAlignment.Top.__doc__ + '\n' + '* ``AlignVCenter``: ' + Qgis.TextVerticalAlignment.VerticalCenter.__doc__ + '\n' + '* ``AlignBottom``: ' + Qgis.TextVerticalAlignment.Bottom.__doc__
# --
Qgis.TextVerticalAlignment.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TextCharacterVerticalAlignment.Normal.__doc__ = "Adjacent characters are positioned in the standard way for text in the writing system in use"
Qgis.TextCharacterVerticalAlignment.SuperScript.__doc__ = "Characters are placed above the base line for normal text."
Qgis.TextCharacterVerticalAlignment.SubScript.__doc__ = "Characters are placed below the base line for normal text."
Qgis.TextCharacterVerticalAlignment.__doc__ = "Text vertical alignment for characters.\n\nThis enum controls vertical alignment of individual characters within a block\nof text.\n\n.. versionadded:: 3.30\n\n" + '* ``Normal``: ' + Qgis.TextCharacterVerticalAlignment.Normal.__doc__ + '\n' + '* ``SuperScript``: ' + Qgis.TextCharacterVerticalAlignment.SuperScript.__doc__ + '\n' + '* ``SubScript``: ' + Qgis.TextCharacterVerticalAlignment.SubScript.__doc__
# --
Qgis.TextCharacterVerticalAlignment.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RenderSubcomponentProperty.Generic.__doc__ = "Generic subcomponent property"
Qgis.RenderSubcomponentProperty.ShadowOffset.__doc__ = "Shadow offset"
Qgis.RenderSubcomponentProperty.BlurSize.__doc__ = "Blur size"
Qgis.RenderSubcomponentProperty.GlowSpread.__doc__ = "Glow spread size"
Qgis.RenderSubcomponentProperty.__doc__ = "Rendering subcomponent properties.\n\n.. versionadded:: 3.22\n\n" + '* ``Generic``: ' + Qgis.RenderSubcomponentProperty.Generic.__doc__ + '\n' + '* ``ShadowOffset``: ' + Qgis.RenderSubcomponentProperty.ShadowOffset.__doc__ + '\n' + '* ``BlurSize``: ' + Qgis.RenderSubcomponentProperty.BlurSize.__doc__ + '\n' + '* ``GlowSpread``: ' + Qgis.RenderSubcomponentProperty.GlowSpread.__doc__
# --
Qgis.RenderSubcomponentProperty.baseClass = Qgis
QgsVertexId.VertexType = Qgis.VertexType
# monkey patching scoped based enum
QgsVertexId.SegmentVertex = Qgis.VertexType.Segment
QgsVertexId.VertexType.SegmentVertex = Qgis.VertexType.Segment
QgsVertexId.SegmentVertex.is_monkey_patched = True
QgsVertexId.SegmentVertex.__doc__ = "The actual start or end point of a segment"
QgsVertexId.CurveVertex = Qgis.VertexType.Curve
QgsVertexId.VertexType.CurveVertex = Qgis.VertexType.Curve
QgsVertexId.CurveVertex.is_monkey_patched = True
QgsVertexId.CurveVertex.__doc__ = "An intermediate point on a segment defining the curvature of the segment"
Qgis.VertexType.__doc__ = "Types of vertex.\n\n.. versionadded:: 3.22\n\n" + '* ``SegmentVertex``: ' + Qgis.VertexType.Segment.__doc__ + '\n' + '* ``CurveVertex``: ' + Qgis.VertexType.Curve.__doc__
# --
Qgis.VertexType.baseClass = Qgis
QgsSimpleMarkerSymbolLayerBase.Shape = Qgis.MarkerShape
# monkey patching scoped based enum
QgsSimpleMarkerSymbolLayerBase.Square = Qgis.MarkerShape.Square
QgsSimpleMarkerSymbolLayerBase.Square.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Square.__doc__ = "Square"
QgsSimpleMarkerSymbolLayerBase.Diamond = Qgis.MarkerShape.Diamond
QgsSimpleMarkerSymbolLayerBase.Diamond.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Diamond.__doc__ = "Diamond"
QgsSimpleMarkerSymbolLayerBase.Pentagon = Qgis.MarkerShape.Pentagon
QgsSimpleMarkerSymbolLayerBase.Pentagon.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Pentagon.__doc__ = "Pentagon"
QgsSimpleMarkerSymbolLayerBase.Hexagon = Qgis.MarkerShape.Hexagon
QgsSimpleMarkerSymbolLayerBase.Hexagon.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Hexagon.__doc__ = "Hexagon"
QgsSimpleMarkerSymbolLayerBase.Triangle = Qgis.MarkerShape.Triangle
QgsSimpleMarkerSymbolLayerBase.Triangle.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Triangle.__doc__ = "Triangle"
QgsSimpleMarkerSymbolLayerBase.EquilateralTriangle = Qgis.MarkerShape.EquilateralTriangle
QgsSimpleMarkerSymbolLayerBase.EquilateralTriangle.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.EquilateralTriangle.__doc__ = "Equilateral triangle"
QgsSimpleMarkerSymbolLayerBase.Star = Qgis.MarkerShape.Star
QgsSimpleMarkerSymbolLayerBase.Star.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Star.__doc__ = "Star"
QgsSimpleMarkerSymbolLayerBase.Arrow = Qgis.MarkerShape.Arrow
QgsSimpleMarkerSymbolLayerBase.Arrow.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Arrow.__doc__ = "Arrow"
QgsSimpleMarkerSymbolLayerBase.Circle = Qgis.MarkerShape.Circle
QgsSimpleMarkerSymbolLayerBase.Circle.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Circle.__doc__ = "Circle"
QgsSimpleMarkerSymbolLayerBase.Cross = Qgis.MarkerShape.Cross
QgsSimpleMarkerSymbolLayerBase.Cross.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Cross.__doc__ = "Cross (lines only)"
QgsSimpleMarkerSymbolLayerBase.CrossFill = Qgis.MarkerShape.CrossFill
QgsSimpleMarkerSymbolLayerBase.CrossFill.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.CrossFill.__doc__ = "Solid filled cross"
QgsSimpleMarkerSymbolLayerBase.Cross2 = Qgis.MarkerShape.Cross2
QgsSimpleMarkerSymbolLayerBase.Cross2.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Cross2.__doc__ = "Rotated cross (lines only), 'x' shape"
QgsSimpleMarkerSymbolLayerBase.Line = Qgis.MarkerShape.Line
QgsSimpleMarkerSymbolLayerBase.Line.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Line.__doc__ = "Vertical line"
QgsSimpleMarkerSymbolLayerBase.ArrowHead = Qgis.MarkerShape.ArrowHead
QgsSimpleMarkerSymbolLayerBase.ArrowHead.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ArrowHead.__doc__ = "Right facing arrow head (unfilled, lines only)"
QgsSimpleMarkerSymbolLayerBase.ArrowHeadFilled = Qgis.MarkerShape.ArrowHeadFilled
QgsSimpleMarkerSymbolLayerBase.ArrowHeadFilled.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ArrowHeadFilled.__doc__ = "Right facing filled arrow head"
QgsSimpleMarkerSymbolLayerBase.SemiCircle = Qgis.MarkerShape.SemiCircle
QgsSimpleMarkerSymbolLayerBase.SemiCircle.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.SemiCircle.__doc__ = "Semi circle (top half)"
QgsSimpleMarkerSymbolLayerBase.ThirdCircle = Qgis.MarkerShape.ThirdCircle
QgsSimpleMarkerSymbolLayerBase.ThirdCircle.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ThirdCircle.__doc__ = "One third circle (top left third)"
QgsSimpleMarkerSymbolLayerBase.QuarterCircle = Qgis.MarkerShape.QuarterCircle
QgsSimpleMarkerSymbolLayerBase.QuarterCircle.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.QuarterCircle.__doc__ = "Quarter circle (top left quarter)"
QgsSimpleMarkerSymbolLayerBase.QuarterSquare = Qgis.MarkerShape.QuarterSquare
QgsSimpleMarkerSymbolLayerBase.QuarterSquare.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.QuarterSquare.__doc__ = "Quarter square (top left quarter)"
QgsSimpleMarkerSymbolLayerBase.HalfSquare = Qgis.MarkerShape.HalfSquare
QgsSimpleMarkerSymbolLayerBase.HalfSquare.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.HalfSquare.__doc__ = "Half square (left half)"
QgsSimpleMarkerSymbolLayerBase.DiagonalHalfSquare = Qgis.MarkerShape.DiagonalHalfSquare
QgsSimpleMarkerSymbolLayerBase.DiagonalHalfSquare.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.DiagonalHalfSquare.__doc__ = "Diagonal half square (bottom left half)"
QgsSimpleMarkerSymbolLayerBase.RightHalfTriangle = Qgis.MarkerShape.RightHalfTriangle
QgsSimpleMarkerSymbolLayerBase.RightHalfTriangle.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.RightHalfTriangle.__doc__ = "Right half of triangle"
QgsSimpleMarkerSymbolLayerBase.LeftHalfTriangle = Qgis.MarkerShape.LeftHalfTriangle
QgsSimpleMarkerSymbolLayerBase.LeftHalfTriangle.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.LeftHalfTriangle.__doc__ = "Left half of triangle"
QgsSimpleMarkerSymbolLayerBase.Octagon = Qgis.MarkerShape.Octagon
QgsSimpleMarkerSymbolLayerBase.Octagon.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Octagon.__doc__ = "Octagon (since QGIS 3.18)"
QgsSimpleMarkerSymbolLayerBase.SquareWithCorners = Qgis.MarkerShape.SquareWithCorners
QgsSimpleMarkerSymbolLayerBase.SquareWithCorners.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.SquareWithCorners.__doc__ = "A square with diagonal corners (since QGIS 3.18)"
QgsSimpleMarkerSymbolLayerBase.AsteriskFill = Qgis.MarkerShape.AsteriskFill
QgsSimpleMarkerSymbolLayerBase.AsteriskFill.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.AsteriskFill.__doc__ = "A filled asterisk shape (since QGIS 3.18)"
QgsSimpleMarkerSymbolLayerBase.HalfArc = Qgis.MarkerShape.HalfArc
QgsSimpleMarkerSymbolLayerBase.HalfArc.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.HalfArc.__doc__ = "A line-only half arc (since QGIS 3.20)"
QgsSimpleMarkerSymbolLayerBase.ThirdArc = Qgis.MarkerShape.ThirdArc
QgsSimpleMarkerSymbolLayerBase.ThirdArc.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ThirdArc.__doc__ = "A line-only one third arc (since QGIS 3.20)"
QgsSimpleMarkerSymbolLayerBase.QuarterArc = Qgis.MarkerShape.QuarterArc
QgsSimpleMarkerSymbolLayerBase.QuarterArc.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.QuarterArc.__doc__ = "A line-only one quarter arc (since QGIS 3.20)"
QgsSimpleMarkerSymbolLayerBase.ParallelogramRight = Qgis.MarkerShape.ParallelogramRight
QgsSimpleMarkerSymbolLayerBase.ParallelogramRight.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ParallelogramRight.__doc__ = "Parallelogram that slants right (since QGIS 3.28)"
QgsSimpleMarkerSymbolLayerBase.ParallelogramLeft = Qgis.MarkerShape.ParallelogramLeft
QgsSimpleMarkerSymbolLayerBase.ParallelogramLeft.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ParallelogramLeft.__doc__ = "Parallelogram that slants left (since QGIS 3.28)"
QgsSimpleMarkerSymbolLayerBase.Trapezoid = Qgis.MarkerShape.Trapezoid
QgsSimpleMarkerSymbolLayerBase.Trapezoid.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Trapezoid.__doc__ = "Trapezoid (since QGIS 3.28)"
QgsSimpleMarkerSymbolLayerBase.Shield = Qgis.MarkerShape.Shield
QgsSimpleMarkerSymbolLayerBase.Shield.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Shield.__doc__ = "A shape consisting of a triangle attached to a rectangle (since QGIS 3.28)"
QgsSimpleMarkerSymbolLayerBase.DiamondStar = Qgis.MarkerShape.DiamondStar
QgsSimpleMarkerSymbolLayerBase.DiamondStar.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.DiamondStar.__doc__ = "A 4-sided star (since QGIS 3.28)"
QgsSimpleMarkerSymbolLayerBase.Heart = Qgis.MarkerShape.Heart
QgsSimpleMarkerSymbolLayerBase.Heart.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Heart.__doc__ = "Heart (since QGIS 3.28)"
QgsSimpleMarkerSymbolLayerBase.Decagon = Qgis.MarkerShape.Decagon
QgsSimpleMarkerSymbolLayerBase.Decagon.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Decagon.__doc__ = "Decagon (since QGIS 3.28)"
QgsSimpleMarkerSymbolLayerBase.RoundedSquare = Qgis.MarkerShape.RoundedSquare
QgsSimpleMarkerSymbolLayerBase.RoundedSquare.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.RoundedSquare.__doc__ = "A square with rounded corners (since QGIS 3.28)"
Qgis.MarkerShape.__doc__ = "Marker shapes.\n\n.. note::\n\n   Prior to QGIS 3.24 this was available as :py:class:`QgsSimpleMarkerSymbolLayerBase`.Shape\n\n.. versionadded:: 3.24\n\n" + '* ``Square``: ' + Qgis.MarkerShape.Square.__doc__ + '\n' + '* ``Diamond``: ' + Qgis.MarkerShape.Diamond.__doc__ + '\n' + '* ``Pentagon``: ' + Qgis.MarkerShape.Pentagon.__doc__ + '\n' + '* ``Hexagon``: ' + Qgis.MarkerShape.Hexagon.__doc__ + '\n' + '* ``Triangle``: ' + Qgis.MarkerShape.Triangle.__doc__ + '\n' + '* ``EquilateralTriangle``: ' + Qgis.MarkerShape.EquilateralTriangle.__doc__ + '\n' + '* ``Star``: ' + Qgis.MarkerShape.Star.__doc__ + '\n' + '* ``Arrow``: ' + Qgis.MarkerShape.Arrow.__doc__ + '\n' + '* ``Circle``: ' + Qgis.MarkerShape.Circle.__doc__ + '\n' + '* ``Cross``: ' + Qgis.MarkerShape.Cross.__doc__ + '\n' + '* ``CrossFill``: ' + Qgis.MarkerShape.CrossFill.__doc__ + '\n' + '* ``Cross2``: ' + Qgis.MarkerShape.Cross2.__doc__ + '\n' + '* ``Line``: ' + Qgis.MarkerShape.Line.__doc__ + '\n' + '* ``ArrowHead``: ' + Qgis.MarkerShape.ArrowHead.__doc__ + '\n' + '* ``ArrowHeadFilled``: ' + Qgis.MarkerShape.ArrowHeadFilled.__doc__ + '\n' + '* ``SemiCircle``: ' + Qgis.MarkerShape.SemiCircle.__doc__ + '\n' + '* ``ThirdCircle``: ' + Qgis.MarkerShape.ThirdCircle.__doc__ + '\n' + '* ``QuarterCircle``: ' + Qgis.MarkerShape.QuarterCircle.__doc__ + '\n' + '* ``QuarterSquare``: ' + Qgis.MarkerShape.QuarterSquare.__doc__ + '\n' + '* ``HalfSquare``: ' + Qgis.MarkerShape.HalfSquare.__doc__ + '\n' + '* ``DiagonalHalfSquare``: ' + Qgis.MarkerShape.DiagonalHalfSquare.__doc__ + '\n' + '* ``RightHalfTriangle``: ' + Qgis.MarkerShape.RightHalfTriangle.__doc__ + '\n' + '* ``LeftHalfTriangle``: ' + Qgis.MarkerShape.LeftHalfTriangle.__doc__ + '\n' + '* ``Octagon``: ' + Qgis.MarkerShape.Octagon.__doc__ + '\n' + '* ``SquareWithCorners``: ' + Qgis.MarkerShape.SquareWithCorners.__doc__ + '\n' + '* ``AsteriskFill``: ' + Qgis.MarkerShape.AsteriskFill.__doc__ + '\n' + '* ``HalfArc``: ' + Qgis.MarkerShape.HalfArc.__doc__ + '\n' + '* ``ThirdArc``: ' + Qgis.MarkerShape.ThirdArc.__doc__ + '\n' + '* ``QuarterArc``: ' + Qgis.MarkerShape.QuarterArc.__doc__ + '\n' + '* ``ParallelogramRight``: ' + Qgis.MarkerShape.ParallelogramRight.__doc__ + '\n' + '* ``ParallelogramLeft``: ' + Qgis.MarkerShape.ParallelogramLeft.__doc__ + '\n' + '* ``Trapezoid``: ' + Qgis.MarkerShape.Trapezoid.__doc__ + '\n' + '* ``Shield``: ' + Qgis.MarkerShape.Shield.__doc__ + '\n' + '* ``DiamondStar``: ' + Qgis.MarkerShape.DiamondStar.__doc__ + '\n' + '* ``Heart``: ' + Qgis.MarkerShape.Heart.__doc__ + '\n' + '* ``Decagon``: ' + Qgis.MarkerShape.Decagon.__doc__ + '\n' + '* ``RoundedSquare``: ' + Qgis.MarkerShape.RoundedSquare.__doc__
# --
Qgis.MarkerShape.baseClass = Qgis
QgsTemplatedLineSymbolLayerBase.Placement = Qgis.MarkerLinePlacement
# monkey patching scoped based enum
QgsTemplatedLineSymbolLayerBase.Interval = Qgis.MarkerLinePlacement.Interval
QgsTemplatedLineSymbolLayerBase.Interval.is_monkey_patched = True
QgsTemplatedLineSymbolLayerBase.Interval.__doc__ = "Place symbols at regular intervals"
QgsTemplatedLineSymbolLayerBase.Vertex = Qgis.MarkerLinePlacement.Vertex
QgsTemplatedLineSymbolLayerBase.Vertex.is_monkey_patched = True
QgsTemplatedLineSymbolLayerBase.Vertex.__doc__ = "Place symbols on every vertex in the line"
QgsTemplatedLineSymbolLayerBase.LastVertex = Qgis.MarkerLinePlacement.LastVertex
QgsTemplatedLineSymbolLayerBase.LastVertex.is_monkey_patched = True
QgsTemplatedLineSymbolLayerBase.LastVertex.__doc__ = "Place symbols on the last vertex in the line"
QgsTemplatedLineSymbolLayerBase.FirstVertex = Qgis.MarkerLinePlacement.FirstVertex
QgsTemplatedLineSymbolLayerBase.FirstVertex.is_monkey_patched = True
QgsTemplatedLineSymbolLayerBase.FirstVertex.__doc__ = "Place symbols on the first vertex in the line"
QgsTemplatedLineSymbolLayerBase.CentralPoint = Qgis.MarkerLinePlacement.CentralPoint
QgsTemplatedLineSymbolLayerBase.CentralPoint.is_monkey_patched = True
QgsTemplatedLineSymbolLayerBase.CentralPoint.__doc__ = "Place symbols at the mid point of the line"
QgsTemplatedLineSymbolLayerBase.CurvePoint = Qgis.MarkerLinePlacement.CurvePoint
QgsTemplatedLineSymbolLayerBase.CurvePoint.is_monkey_patched = True
QgsTemplatedLineSymbolLayerBase.CurvePoint.__doc__ = "Place symbols at every virtual curve point in the line (used when rendering curved geometry types only)"
QgsTemplatedLineSymbolLayerBase.SegmentCenter = Qgis.MarkerLinePlacement.SegmentCenter
QgsTemplatedLineSymbolLayerBase.SegmentCenter.is_monkey_patched = True
QgsTemplatedLineSymbolLayerBase.SegmentCenter.__doc__ = "Place symbols at the center of every line segment"
QgsTemplatedLineSymbolLayerBase.InnerVertices = Qgis.MarkerLinePlacement.InnerVertices
QgsTemplatedLineSymbolLayerBase.InnerVertices.is_monkey_patched = True
QgsTemplatedLineSymbolLayerBase.InnerVertices.__doc__ = "Inner vertices (i.e. all vertices except the first and last vertex) (since QGIS 3.24)"
Qgis.MarkerLinePlacement.__doc__ = "Defines how/where the symbols should be placed on a line.\n\n.. note::\n\n   Prior to QGIS 3.24 this was available as :py:class:`QgsTemplatedLineSymbolLayerBase`.Placement\n\n.. versionadded:: 3.24\n\n" + '* ``Interval``: ' + Qgis.MarkerLinePlacement.Interval.__doc__ + '\n' + '* ``Vertex``: ' + Qgis.MarkerLinePlacement.Vertex.__doc__ + '\n' + '* ``LastVertex``: ' + Qgis.MarkerLinePlacement.LastVertex.__doc__ + '\n' + '* ``FirstVertex``: ' + Qgis.MarkerLinePlacement.FirstVertex.__doc__ + '\n' + '* ``CentralPoint``: ' + Qgis.MarkerLinePlacement.CentralPoint.__doc__ + '\n' + '* ``CurvePoint``: ' + Qgis.MarkerLinePlacement.CurvePoint.__doc__ + '\n' + '* ``SegmentCenter``: ' + Qgis.MarkerLinePlacement.SegmentCenter.__doc__ + '\n' + '* ``InnerVertices``: ' + Qgis.MarkerLinePlacement.InnerVertices.__doc__
# --
Qgis.MarkerLinePlacement.baseClass = Qgis
Qgis.MarkerLinePlacements = lambda flags=0: Qgis.MarkerLinePlacement(flags)
Qgis.MarkerLinePlacements.baseClass = Qgis
MarkerLinePlacements = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsGradientFillSymbolLayer.GradientColorType = Qgis.GradientColorSource
# monkey patching scoped based enum
QgsGradientFillSymbolLayer.SimpleTwoColor = Qgis.GradientColorSource.SimpleTwoColor
QgsGradientFillSymbolLayer.SimpleTwoColor.is_monkey_patched = True
QgsGradientFillSymbolLayer.SimpleTwoColor.__doc__ = "Simple two color gradient"
QgsGradientFillSymbolLayer.ColorRamp = Qgis.GradientColorSource.ColorRamp
QgsGradientFillSymbolLayer.ColorRamp.is_monkey_patched = True
QgsGradientFillSymbolLayer.ColorRamp.__doc__ = "Gradient color ramp"
Qgis.GradientColorSource.__doc__ = "Gradient color sources.\n\n.. note::\n\n   Prior to QGIS 3.24 this was available as :py:class:`QgsGradientFillSymbolLayer`.GradientColorType\n\n.. versionadded:: 3.24\n\n" + '* ``SimpleTwoColor``: ' + Qgis.GradientColorSource.SimpleTwoColor.__doc__ + '\n' + '* ``ColorRamp``: ' + Qgis.GradientColorSource.ColorRamp.__doc__
# --
Qgis.GradientColorSource.baseClass = Qgis
QgsGradientFillSymbolLayer.GradientType = Qgis.GradientType
# monkey patching scoped based enum
QgsGradientFillSymbolLayer.Linear = Qgis.GradientType.Linear
QgsGradientFillSymbolLayer.Linear.is_monkey_patched = True
QgsGradientFillSymbolLayer.Linear.__doc__ = "Linear gradient"
QgsGradientFillSymbolLayer.Radial = Qgis.GradientType.Radial
QgsGradientFillSymbolLayer.Radial.is_monkey_patched = True
QgsGradientFillSymbolLayer.Radial.__doc__ = "Radial (circular) gradient"
QgsGradientFillSymbolLayer.Conical = Qgis.GradientType.Conical
QgsGradientFillSymbolLayer.Conical.is_monkey_patched = True
QgsGradientFillSymbolLayer.Conical.__doc__ = "Conical (polar) gradient"
Qgis.GradientType.__doc__ = "Gradient types.\n\n.. note::\n\n   Prior to QGIS 3.24 this was available as :py:class:`QgsGradientFillSymbolLayer`.GradientType\n\n.. versionadded:: 3.24\n\n" + '* ``Linear``: ' + Qgis.GradientType.Linear.__doc__ + '\n' + '* ``Radial``: ' + Qgis.GradientType.Radial.__doc__ + '\n' + '* ``Conical``: ' + Qgis.GradientType.Conical.__doc__
# --
Qgis.GradientType.baseClass = Qgis
QgsGradientFillSymbolLayer.GradientCoordinateMode = Qgis.SymbolCoordinateReference
# monkey patching scoped based enum
QgsGradientFillSymbolLayer.Feature = Qgis.SymbolCoordinateReference.Feature
QgsGradientFillSymbolLayer.Feature.is_monkey_patched = True
QgsGradientFillSymbolLayer.Feature.__doc__ = "Relative to feature/shape being rendered"
QgsGradientFillSymbolLayer.Viewport = Qgis.SymbolCoordinateReference.Viewport
QgsGradientFillSymbolLayer.Viewport.is_monkey_patched = True
QgsGradientFillSymbolLayer.Viewport.__doc__ = "Relative to the whole viewport/output device"
Qgis.SymbolCoordinateReference.__doc__ = "Symbol coordinate reference modes.\n\n.. note::\n\n   Prior to QGIS 3.24 this was available as :py:class:`QgsGradientFillSymbolLayer`.GradientCoordinateMode\n\n.. versionadded:: 3.24\n\n" + '* ``Feature``: ' + Qgis.SymbolCoordinateReference.Feature.__doc__ + '\n' + '* ``Viewport``: ' + Qgis.SymbolCoordinateReference.Viewport.__doc__
# --
Qgis.SymbolCoordinateReference.baseClass = Qgis
QgsGradientFillSymbolLayer.GradientSpread = Qgis.GradientSpread
# monkey patching scoped based enum
QgsGradientFillSymbolLayer.Pad = Qgis.GradientSpread.Pad
QgsGradientFillSymbolLayer.Pad.is_monkey_patched = True
QgsGradientFillSymbolLayer.Pad.__doc__ = "Pad out gradient using colors at endpoint of gradient"
QgsGradientFillSymbolLayer.Reflect = Qgis.GradientSpread.Reflect
QgsGradientFillSymbolLayer.Reflect.is_monkey_patched = True
QgsGradientFillSymbolLayer.Reflect.__doc__ = "Reflect gradient"
QgsGradientFillSymbolLayer.Repeat = Qgis.GradientSpread.Repeat
QgsGradientFillSymbolLayer.Repeat.is_monkey_patched = True
QgsGradientFillSymbolLayer.Repeat.__doc__ = "Repeat gradient"
Qgis.GradientSpread.__doc__ = "Gradient spread options, which control how gradients are rendered outside of their\nstart and end points.\n\n.. note::\n\n   Prior to QGIS 3.24 this was available as :py:class:`QgsGradientFillSymbolLayer`.GradientSpread\n\n.. versionadded:: 3.24\n\n" + '* ``Pad``: ' + Qgis.GradientSpread.Pad.__doc__ + '\n' + '* ``Reflect``: ' + Qgis.GradientSpread.Reflect.__doc__ + '\n' + '* ``Repeat``: ' + Qgis.GradientSpread.Repeat.__doc__
# --
Qgis.GradientSpread.baseClass = Qgis
QgsRandomMarkerFillSymbolLayer.CountMethod = Qgis.PointCountMethod
# monkey patching scoped based enum
QgsRandomMarkerFillSymbolLayer.AbsoluteCount = Qgis.PointCountMethod.Absolute
QgsRandomMarkerFillSymbolLayer.CountMethod.AbsoluteCount = Qgis.PointCountMethod.Absolute
QgsRandomMarkerFillSymbolLayer.AbsoluteCount.is_monkey_patched = True
QgsRandomMarkerFillSymbolLayer.AbsoluteCount.__doc__ = "The point count is used as an absolute count of markers"
QgsRandomMarkerFillSymbolLayer.DensityBasedCount = Qgis.PointCountMethod.DensityBased
QgsRandomMarkerFillSymbolLayer.CountMethod.DensityBasedCount = Qgis.PointCountMethod.DensityBased
QgsRandomMarkerFillSymbolLayer.DensityBasedCount.is_monkey_patched = True
QgsRandomMarkerFillSymbolLayer.DensityBasedCount.__doc__ = "The point count is part of a marker density count"
Qgis.PointCountMethod.__doc__ = "Methods which define the number of points randomly filling a polygon.\n\n.. note::\n\n   Prior to QGIS 3.24 this was available as :py:class:`QgsRandomMarkerFillSymbolLayer`.CountMethod\n\n.. versionadded:: 3.24\n\n" + '* ``AbsoluteCount``: ' + Qgis.PointCountMethod.Absolute.__doc__ + '\n' + '* ``DensityBasedCount``: ' + Qgis.PointCountMethod.DensityBased.__doc__
# --
Qgis.PointCountMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MarkerClipMode.NoClipping.__doc__ = "No clipping, render complete markers"
Qgis.MarkerClipMode.Shape.__doc__ = "Clip to polygon shape"
Qgis.MarkerClipMode.CentroidWithin.__doc__ = "Render complete markers wherever their centroid falls within the polygon shape"
Qgis.MarkerClipMode.CompletelyWithin.__doc__ = "Render complete markers wherever the completely fall within the polygon shape"
Qgis.MarkerClipMode.__doc__ = "Marker clipping modes.\n\n.. versionadded:: 3.24\n\n" + '* ``NoClipping``: ' + Qgis.MarkerClipMode.NoClipping.__doc__ + '\n' + '* ``Shape``: ' + Qgis.MarkerClipMode.Shape.__doc__ + '\n' + '* ``CentroidWithin``: ' + Qgis.MarkerClipMode.CentroidWithin.__doc__ + '\n' + '* ``CompletelyWithin``: ' + Qgis.MarkerClipMode.CompletelyWithin.__doc__
# --
Qgis.MarkerClipMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LineClipMode.ClipPainterOnly.__doc__ = "Applying clipping on the painter only (i.e. line endpoints will coincide with polygon bounding box, but will not be part of the visible portion of the line)"
Qgis.LineClipMode.ClipToIntersection.__doc__ = "Clip lines to intersection with polygon shape (slower) (i.e. line endpoints will coincide with polygon exterior)"
Qgis.LineClipMode.NoClipping.__doc__ = "Lines are not clipped, will extend to shape's bounding box."
Qgis.LineClipMode.__doc__ = "Line clipping modes.\n\n.. versionadded:: 3.24\n\n" + '* ``ClipPainterOnly``: ' + Qgis.LineClipMode.ClipPainterOnly.__doc__ + '\n' + '* ``ClipToIntersection``: ' + Qgis.LineClipMode.ClipToIntersection.__doc__ + '\n' + '* ``NoClipping``: ' + Qgis.LineClipMode.NoClipping.__doc__
# --
Qgis.LineClipMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DashPatternLineEndingRule.NoRule.__doc__ = "No special rule"
Qgis.DashPatternLineEndingRule.FullDash.__doc__ = "Start or finish the pattern with a full dash"
Qgis.DashPatternLineEndingRule.HalfDash.__doc__ = "Start or finish the pattern with a half length dash"
Qgis.DashPatternLineEndingRule.FullGap.__doc__ = "Start or finish the pattern with a full gap"
Qgis.DashPatternLineEndingRule.HalfGap.__doc__ = "Start or finish the pattern with a half length gap"
Qgis.DashPatternLineEndingRule.__doc__ = "Dash pattern line ending rules.\n\n.. versionadded:: 3.24\n\n" + '* ``NoRule``: ' + Qgis.DashPatternLineEndingRule.NoRule.__doc__ + '\n' + '* ``FullDash``: ' + Qgis.DashPatternLineEndingRule.FullDash.__doc__ + '\n' + '* ``HalfDash``: ' + Qgis.DashPatternLineEndingRule.HalfDash.__doc__ + '\n' + '* ``FullGap``: ' + Qgis.DashPatternLineEndingRule.FullGap.__doc__ + '\n' + '* ``HalfGap``: ' + Qgis.DashPatternLineEndingRule.HalfGap.__doc__
# --
Qgis.DashPatternLineEndingRule.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DashPatternSizeAdjustment.ScaleBothDashAndGap.__doc__ = "Both the dash and gap lengths are adjusted equally"
Qgis.DashPatternSizeAdjustment.ScaleDashOnly.__doc__ = "Only dash lengths are adjusted"
Qgis.DashPatternSizeAdjustment.ScaleGapOnly.__doc__ = "Only gap lengths are adjusted"
Qgis.DashPatternSizeAdjustment.__doc__ = "Dash pattern size adjustment options.\n\n.. versionadded:: 3.24\n\n" + '* ``ScaleBothDashAndGap``: ' + Qgis.DashPatternSizeAdjustment.ScaleBothDashAndGap.__doc__ + '\n' + '* ``ScaleDashOnly``: ' + Qgis.DashPatternSizeAdjustment.ScaleDashOnly.__doc__ + '\n' + '* ``ScaleGapOnly``: ' + Qgis.DashPatternSizeAdjustment.ScaleGapOnly.__doc__
# --
Qgis.DashPatternSizeAdjustment.baseClass = Qgis
QgsGraduatedSymbolRenderer.GraduatedMethod = Qgis.GraduatedMethod
# monkey patching scoped based enum
QgsGraduatedSymbolRenderer.GraduatedColor = Qgis.GraduatedMethod.Color
QgsGraduatedSymbolRenderer.GraduatedMethod.GraduatedColor = Qgis.GraduatedMethod.Color
QgsGraduatedSymbolRenderer.GraduatedColor.is_monkey_patched = True
QgsGraduatedSymbolRenderer.GraduatedColor.__doc__ = "Alter color of symbols"
QgsGraduatedSymbolRenderer.GraduatedSize = Qgis.GraduatedMethod.Size
QgsGraduatedSymbolRenderer.GraduatedMethod.GraduatedSize = Qgis.GraduatedMethod.Size
QgsGraduatedSymbolRenderer.GraduatedSize.is_monkey_patched = True
QgsGraduatedSymbolRenderer.GraduatedSize.__doc__ = "Alter size of symbols"
Qgis.GraduatedMethod.__doc__ = "Methods for modifying symbols by range in a graduated symbol renderer.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsGraduatedSymbolRenderer`.GraduatedMethod\n\n.. versionadded:: 3.26\n\n" + '* ``GraduatedColor``: ' + Qgis.GraduatedMethod.Color.__doc__ + '\n' + '* ``GraduatedSize``: ' + Qgis.GraduatedMethod.Size.__doc__
# --
Qgis.GraduatedMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.PlotAxisSuffixPlacement.NoLabels.__doc__ = "Do not place suffixes"
Qgis.PlotAxisSuffixPlacement.EveryLabel.__doc__ = "Place suffix after every value label"
Qgis.PlotAxisSuffixPlacement.FirstLabel.__doc__ = "Place suffix after the first label value only"
Qgis.PlotAxisSuffixPlacement.LastLabel.__doc__ = "Place suffix after the last label value only"
Qgis.PlotAxisSuffixPlacement.FirstAndLastLabels.__doc__ = "Place suffix after the first and last label values only"
Qgis.PlotAxisSuffixPlacement.__doc__ = "Placement options for suffixes in the labels for axis of plots.\n\n.. versionadded:: 3.32\n\n" + '* ``NoLabels``: ' + Qgis.PlotAxisSuffixPlacement.NoLabels.__doc__ + '\n' + '* ``EveryLabel``: ' + Qgis.PlotAxisSuffixPlacement.EveryLabel.__doc__ + '\n' + '* ``FirstLabel``: ' + Qgis.PlotAxisSuffixPlacement.FirstLabel.__doc__ + '\n' + '* ``LastLabel``: ' + Qgis.PlotAxisSuffixPlacement.LastLabel.__doc__ + '\n' + '* ``FirstAndLastLabels``: ' + Qgis.PlotAxisSuffixPlacement.FirstAndLastLabels.__doc__
# --
Qgis.PlotAxisSuffixPlacement.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DpiMode.All.__doc__ = "All"
Qgis.DpiMode.Off.__doc__ = "Off"
Qgis.DpiMode.QGIS.__doc__ = "QGIS"
Qgis.DpiMode.UMN.__doc__ = "UMN"
Qgis.DpiMode.GeoServer.__doc__ = "GeoServer"
Qgis.DpiMode.__doc__ = "DpiMode enum\n\n.. versionadded:: 3.26\n\n" + '* ``All``: ' + Qgis.DpiMode.All.__doc__ + '\n' + '* ``Off``: ' + Qgis.DpiMode.Off.__doc__ + '\n' + '* ``QGIS``: ' + Qgis.DpiMode.QGIS.__doc__ + '\n' + '* ``UMN``: ' + Qgis.DpiMode.UMN.__doc__ + '\n' + '* ``GeoServer``: ' + Qgis.DpiMode.GeoServer.__doc__
# --
Qgis.DpiMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TilePixelRatio.Undefined.__doc__ = "Undefined (not scale)"
Qgis.TilePixelRatio.StandardDpi.__doc__ = "Standard (96 DPI)"
Qgis.TilePixelRatio.HighDpi.__doc__ = "High (192 DPI)"
Qgis.TilePixelRatio.__doc__ = "DpiMode enum\n\n.. versionadded:: 3.30\n\n" + '* ``Undefined``: ' + Qgis.TilePixelRatio.Undefined.__doc__ + '\n' + '* ``StandardDpi``: ' + Qgis.TilePixelRatio.StandardDpi.__doc__ + '\n' + '* ``HighDpi``: ' + Qgis.TilePixelRatio.HighDpi.__doc__
# --
Qgis.TilePixelRatio.baseClass = Qgis
QgsStringUtils.Capitalization = Qgis.Capitalization
# monkey patching scoped based enum
QgsStringUtils.MixedCase = Qgis.Capitalization.MixedCase
QgsStringUtils.MixedCase.is_monkey_patched = True
QgsStringUtils.MixedCase.__doc__ = "Mixed case, ie no change"
QgsStringUtils.AllUppercase = Qgis.Capitalization.AllUppercase
QgsStringUtils.AllUppercase.is_monkey_patched = True
QgsStringUtils.AllUppercase.__doc__ = "Convert all characters to uppercase"
QgsStringUtils.AllLowercase = Qgis.Capitalization.AllLowercase
QgsStringUtils.AllLowercase.is_monkey_patched = True
QgsStringUtils.AllLowercase.__doc__ = "Convert all characters to lowercase"
QgsStringUtils.ForceFirstLetterToCapital = Qgis.Capitalization.ForceFirstLetterToCapital
QgsStringUtils.ForceFirstLetterToCapital.is_monkey_patched = True
QgsStringUtils.ForceFirstLetterToCapital.__doc__ = "Convert just the first letter of each word to uppercase, leave the rest untouched"
QgsStringUtils.SmallCaps = Qgis.Capitalization.SmallCaps
QgsStringUtils.SmallCaps.is_monkey_patched = True
QgsStringUtils.SmallCaps.__doc__ = "Mixed case small caps (since QGIS 3.24)"
QgsStringUtils.TitleCase = Qgis.Capitalization.TitleCase
QgsStringUtils.TitleCase.is_monkey_patched = True
QgsStringUtils.TitleCase.__doc__ = "Simple title case conversion - does not fully grammatically parse the text and uses simple rules only. Note that this method does not convert any characters to lowercase, it only uppercases required letters. Callers must ensure that input strings are already lowercased."
QgsStringUtils.UpperCamelCase = Qgis.Capitalization.UpperCamelCase
QgsStringUtils.UpperCamelCase.is_monkey_patched = True
QgsStringUtils.UpperCamelCase.__doc__ = "Convert the string to upper camel case. Note that this method does not unaccent characters."
QgsStringUtils.AllSmallCaps = Qgis.Capitalization.AllSmallCaps
QgsStringUtils.AllSmallCaps.is_monkey_patched = True
QgsStringUtils.AllSmallCaps.__doc__ = "Force all characters to small caps (since QGIS 3.24)"
Qgis.Capitalization.__doc__ = "String capitalization options.\n\n.. note::\n\n   Prior to QGIS 3.24 this was available as :py:class:`QgsStringUtils`.Capitalization\n\n.. versionadded:: 3.24\n\n" + '* ``MixedCase``: ' + Qgis.Capitalization.MixedCase.__doc__ + '\n' + '* ``AllUppercase``: ' + Qgis.Capitalization.AllUppercase.__doc__ + '\n' + '* ``AllLowercase``: ' + Qgis.Capitalization.AllLowercase.__doc__ + '\n' + '* ``ForceFirstLetterToCapital``: ' + Qgis.Capitalization.ForceFirstLetterToCapital.__doc__ + '\n' + '* ``SmallCaps``: ' + Qgis.Capitalization.SmallCaps.__doc__ + '\n' + '* ``TitleCase``: ' + Qgis.Capitalization.TitleCase.__doc__ + '\n' + '* ``UpperCamelCase``: ' + Qgis.Capitalization.UpperCamelCase.__doc__ + '\n' + '* ``AllSmallCaps``: ' + Qgis.Capitalization.AllSmallCaps.__doc__
# --
Qgis.Capitalization.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TextRendererFlag.WrapLines.__doc__ = "Automatically wrap long lines of text"
Qgis.TextRendererFlag.__doc__ = "Flags which control the behavior of rendering text.\n\n.. versionadded:: 3.24\n\n" + '* ``WrapLines``: ' + Qgis.TextRendererFlag.WrapLines.__doc__
# --
Qgis.TextRendererFlag.baseClass = Qgis
Qgis.TextRendererFlags = lambda flags=0: Qgis.TextRendererFlag(flags)
Qgis.TextRendererFlags.baseClass = Qgis
TextRendererFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ScaleToTileZoomLevelMethod.MapBox.__doc__ = "Uses a scale doubling approach to account for hi-DPI tiles, and rounds to the nearest tile level for the map scale"
Qgis.ScaleToTileZoomLevelMethod.Esri.__doc__ = "No scale doubling, always rounds down when matching to available tile levels"
Qgis.ScaleToTileZoomLevelMethod.__doc__ = "Available methods for converting map scales to tile zoom levels.\n\n.. versionadded:: 3.26\n\n" + '* ``MapBox``: ' + Qgis.ScaleToTileZoomLevelMethod.MapBox.__doc__ + '\n' + '* ``Esri``: ' + Qgis.ScaleToTileZoomLevelMethod.Esri.__doc__
# --
Qgis.ScaleToTileZoomLevelMethod.baseClass = Qgis
QgsCurve.Orientation = Qgis.AngularDirection
# monkey patching scoped based enum
QgsCurve.Clockwise = Qgis.AngularDirection.Clockwise
QgsCurve.Clockwise.is_monkey_patched = True
QgsCurve.Clockwise.__doc__ = "Clockwise direction"
QgsCurve.CounterClockwise = Qgis.AngularDirection.CounterClockwise
QgsCurve.CounterClockwise.is_monkey_patched = True
QgsCurve.CounterClockwise.__doc__ = "Counter-clockwise direction"
QgsCurve.NoOrientation = Qgis.AngularDirection.NoOrientation
QgsCurve.NoOrientation.is_monkey_patched = True
QgsCurve.NoOrientation.__doc__ = "Unknown orientation or sentinel value"
Qgis.AngularDirection.__doc__ = "Angular directions.\n\n.. versionadded:: 3.24\n\n" + '* ``Clockwise``: ' + Qgis.AngularDirection.Clockwise.__doc__ + '\n' + '* ``CounterClockwise``: ' + Qgis.AngularDirection.CounterClockwise.__doc__ + '\n' + '* ``NoOrientation``: ' + Qgis.AngularDirection.NoOrientation.__doc__
# --
Qgis.AngularDirection.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RendererUsage.View.__doc__ = "Renderer used for displaying on screen"
Qgis.RendererUsage.Export.__doc__ = "Renderer used for printing or exporting to a file"
Qgis.RendererUsage.Unknown.__doc__ = "Renderer used for unknown usage"
Qgis.RendererUsage.__doc__ = "Usage of the renderer.\n\n.. versionadded:: 3.24\n\n" + '* ``View``: ' + Qgis.RendererUsage.View.__doc__ + '\n' + '* ``Export``: ' + Qgis.RendererUsage.Export.__doc__ + '\n' + '* ``Unknown``: ' + Qgis.RendererUsage.Unknown.__doc__
# --
Qgis.RendererUsage.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ViewSyncModeFlag.Sync3DTo2D.__doc__ = "Synchronize 3D view camera to the main map canvas extent"
Qgis.ViewSyncModeFlag.Sync2DTo3D.__doc__ = "Update the 2D main canvas extent to include the viewed area from the 3D view"
Qgis.ViewSyncModeFlag.__doc__ = "Synchronization of 2D map canvas and 3D view\n\n.. versionadded:: 3.26\n\n" + '* ``Sync3DTo2D``: ' + Qgis.ViewSyncModeFlag.Sync3DTo2D.__doc__ + '\n' + '* ``Sync2DTo3D``: ' + Qgis.ViewSyncModeFlag.Sync2DTo3D.__doc__
# --
Qgis.ViewSyncModeFlag.baseClass = Qgis
Qgis.ViewSyncModeFlags = lambda flags=0: Qgis.ViewSyncModeFlag(flags)
# monkey patching scoped based enum
Qgis.MapRecenteringMode.Always.__doc__ = "Always recenter map"
Qgis.MapRecenteringMode.WhenOutsideVisibleExtent.__doc__ = "Only recenter map when new center would be outside of current visible extent"
Qgis.MapRecenteringMode.Never.__doc__ = "Never recenter map"
Qgis.MapRecenteringMode.__doc__ = "Modes for recentering map canvases.\n\n.. versionadded:: 3.30\n\n" + '* ``Always``: ' + Qgis.MapRecenteringMode.Always.__doc__ + '\n' + '* ``WhenOutsideVisibleExtent``: ' + Qgis.MapRecenteringMode.WhenOutsideVisibleExtent.__doc__ + '\n' + '* ``Never``: ' + Qgis.MapRecenteringMode.Never.__doc__
# --
Qgis.MapRecenteringMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.HistoryProviderBackend.LocalProfile.__doc__ = "Local profile"
Qgis.HistoryProviderBackend.__doc__ = "History provider backends.\n\n.. versionadded:: 3.24\n\n" + '* ``LocalProfile``: ' + Qgis.HistoryProviderBackend.LocalProfile.__doc__
# --
Qgis.HistoryProviderBackend.baseClass = Qgis
Qgis.HistoryProviderBackends = lambda flags=0: Qgis.HistoryProviderBackend(flags)
Qgis.HistoryProviderBackends.baseClass = Qgis
HistoryProviderBackends = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessing.SourceType = Qgis.ProcessingSourceType
# monkey patching scoped based enum
QgsProcessing.TypeMapLayer = Qgis.ProcessingSourceType.MapLayer
QgsProcessing.SourceType.TypeMapLayer = Qgis.ProcessingSourceType.MapLayer
QgsProcessing.TypeMapLayer.is_monkey_patched = True
QgsProcessing.TypeMapLayer.__doc__ = "Any map layer type (raster, vector, mesh, point cloud, annotation or plugin layer)"
QgsProcessing.TypeVectorAnyGeometry = Qgis.ProcessingSourceType.VectorAnyGeometry
QgsProcessing.SourceType.TypeVectorAnyGeometry = Qgis.ProcessingSourceType.VectorAnyGeometry
QgsProcessing.TypeVectorAnyGeometry.is_monkey_patched = True
QgsProcessing.TypeVectorAnyGeometry.__doc__ = "Any vector layer with geometry"
QgsProcessing.TypeVectorPoint = Qgis.ProcessingSourceType.VectorPoint
QgsProcessing.SourceType.TypeVectorPoint = Qgis.ProcessingSourceType.VectorPoint
QgsProcessing.TypeVectorPoint.is_monkey_patched = True
QgsProcessing.TypeVectorPoint.__doc__ = "Vector point layers"
QgsProcessing.TypeVectorLine = Qgis.ProcessingSourceType.VectorLine
QgsProcessing.SourceType.TypeVectorLine = Qgis.ProcessingSourceType.VectorLine
QgsProcessing.TypeVectorLine.is_monkey_patched = True
QgsProcessing.TypeVectorLine.__doc__ = "Vector line layers"
QgsProcessing.TypeVectorPolygon = Qgis.ProcessingSourceType.VectorPolygon
QgsProcessing.SourceType.TypeVectorPolygon = Qgis.ProcessingSourceType.VectorPolygon
QgsProcessing.TypeVectorPolygon.is_monkey_patched = True
QgsProcessing.TypeVectorPolygon.__doc__ = "Vector polygon layers"
QgsProcessing.TypeRaster = Qgis.ProcessingSourceType.Raster
QgsProcessing.SourceType.TypeRaster = Qgis.ProcessingSourceType.Raster
QgsProcessing.TypeRaster.is_monkey_patched = True
QgsProcessing.TypeRaster.__doc__ = "Raster layers"
QgsProcessing.TypeFile = Qgis.ProcessingSourceType.File
QgsProcessing.SourceType.TypeFile = Qgis.ProcessingSourceType.File
QgsProcessing.TypeFile.is_monkey_patched = True
QgsProcessing.TypeFile.__doc__ = "Files (i.e. non map layer sources, such as text files)"
QgsProcessing.TypeVector = Qgis.ProcessingSourceType.Vector
QgsProcessing.SourceType.TypeVector = Qgis.ProcessingSourceType.Vector
QgsProcessing.TypeVector.is_monkey_patched = True
QgsProcessing.TypeVector.__doc__ = "Tables (i.e. vector layers with or without geometry). When used for a sink this indicates the sink has no geometry."
QgsProcessing.TypeMesh = Qgis.ProcessingSourceType.Mesh
QgsProcessing.SourceType.TypeMesh = Qgis.ProcessingSourceType.Mesh
QgsProcessing.TypeMesh.is_monkey_patched = True
QgsProcessing.TypeMesh.__doc__ = "Mesh layers \n.. versionadded:: 3.6"
QgsProcessing.TypePlugin = Qgis.ProcessingSourceType.Plugin
QgsProcessing.SourceType.TypePlugin = Qgis.ProcessingSourceType.Plugin
QgsProcessing.TypePlugin.is_monkey_patched = True
QgsProcessing.TypePlugin.__doc__ = "Plugin layers \n.. versionadded:: 3.22"
QgsProcessing.TypePointCloud = Qgis.ProcessingSourceType.PointCloud
QgsProcessing.SourceType.TypePointCloud = Qgis.ProcessingSourceType.PointCloud
QgsProcessing.TypePointCloud.is_monkey_patched = True
QgsProcessing.TypePointCloud.__doc__ = "Point cloud layers \n.. versionadded:: 3.22"
QgsProcessing.TypeAnnotation = Qgis.ProcessingSourceType.Annotation
QgsProcessing.SourceType.TypeAnnotation = Qgis.ProcessingSourceType.Annotation
QgsProcessing.TypeAnnotation.is_monkey_patched = True
QgsProcessing.TypeAnnotation.__doc__ = "Annotation layers \n.. versionadded:: 3.22"
QgsProcessing.TypeVectorTile = Qgis.ProcessingSourceType.VectorTile
QgsProcessing.SourceType.TypeVectorTile = Qgis.ProcessingSourceType.VectorTile
QgsProcessing.TypeVectorTile.is_monkey_patched = True
QgsProcessing.TypeVectorTile.__doc__ = "Vector tile layers \n.. versionadded:: 3.32"
Qgis.ProcessingSourceType.__doc__ = "Processing data source types.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessing`.SourceType\n\n.. versionadded:: 3.36\n\n" + '* ``TypeMapLayer``: ' + Qgis.ProcessingSourceType.MapLayer.__doc__ + '\n' + '* ``TypeVectorAnyGeometry``: ' + Qgis.ProcessingSourceType.VectorAnyGeometry.__doc__ + '\n' + '* ``TypeVectorPoint``: ' + Qgis.ProcessingSourceType.VectorPoint.__doc__ + '\n' + '* ``TypeVectorLine``: ' + Qgis.ProcessingSourceType.VectorLine.__doc__ + '\n' + '* ``TypeVectorPolygon``: ' + Qgis.ProcessingSourceType.VectorPolygon.__doc__ + '\n' + '* ``TypeRaster``: ' + Qgis.ProcessingSourceType.Raster.__doc__ + '\n' + '* ``TypeFile``: ' + Qgis.ProcessingSourceType.File.__doc__ + '\n' + '* ``TypeVector``: ' + Qgis.ProcessingSourceType.Vector.__doc__ + '\n' + '* ``TypeMesh``: ' + Qgis.ProcessingSourceType.Mesh.__doc__ + '\n' + '* ``TypePlugin``: ' + Qgis.ProcessingSourceType.Plugin.__doc__ + '\n' + '* ``TypePointCloud``: ' + Qgis.ProcessingSourceType.PointCloud.__doc__ + '\n' + '* ``TypeAnnotation``: ' + Qgis.ProcessingSourceType.Annotation.__doc__ + '\n' + '* ``TypeVectorTile``: ' + Qgis.ProcessingSourceType.VectorTile.__doc__
# --
Qgis.ProcessingSourceType.baseClass = Qgis
QgsProcessingProvider.Flag = Qgis.ProcessingProviderFlag
# monkey patching scoped based enum
QgsProcessingProvider.FlagDeemphasiseSearchResults = Qgis.ProcessingProviderFlag.DeemphasiseSearchResults
QgsProcessingProvider.Flag.FlagDeemphasiseSearchResults = Qgis.ProcessingProviderFlag.DeemphasiseSearchResults
QgsProcessingProvider.FlagDeemphasiseSearchResults.is_monkey_patched = True
QgsProcessingProvider.FlagDeemphasiseSearchResults.__doc__ = "Algorithms should be de-emphasised in the search results when searching for algorithms. Use for low-priority providers or those with substantial known issues."
QgsProcessingProvider.FlagCompatibleWithVirtualRaster = Qgis.ProcessingProviderFlag.CompatibleWithVirtualRaster
QgsProcessingProvider.Flag.FlagCompatibleWithVirtualRaster = Qgis.ProcessingProviderFlag.CompatibleWithVirtualRaster
QgsProcessingProvider.FlagCompatibleWithVirtualRaster.is_monkey_patched = True
QgsProcessingProvider.FlagCompatibleWithVirtualRaster.__doc__ = "The processing provider's algorithms can work with QGIS virtualraster data provider. Since QGIS 3.36"
Qgis.ProcessingProviderFlag.__doc__ = "Flags indicating how and when an processing provider operates and should be exposed to users.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingProvider`.Flag\n\n.. versionadded:: 3.36\n\n" + '* ``FlagDeemphasiseSearchResults``: ' + Qgis.ProcessingProviderFlag.DeemphasiseSearchResults.__doc__ + '\n' + '* ``FlagCompatibleWithVirtualRaster``: ' + Qgis.ProcessingProviderFlag.CompatibleWithVirtualRaster.__doc__
# --
Qgis.ProcessingProviderFlag.baseClass = Qgis
Qgis.ProcessingProviderFlags = lambda flags=0: Qgis.ProcessingProviderFlag(flags)
QgsProcessingProvider.Flags = Qgis.ProcessingProviderFlags
Qgis.ProcessingProviderFlags.baseClass = Qgis
ProcessingProviderFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingAlgorithm.Flag = Qgis.ProcessingAlgorithmFlag
# monkey patching scoped based enum
QgsProcessingAlgorithm.FlagHideFromToolbox = Qgis.ProcessingAlgorithmFlag.HideFromToolbox
QgsProcessingAlgorithm.Flag.FlagHideFromToolbox = Qgis.ProcessingAlgorithmFlag.HideFromToolbox
QgsProcessingAlgorithm.FlagHideFromToolbox.is_monkey_patched = True
QgsProcessingAlgorithm.FlagHideFromToolbox.__doc__ = "Algorithm should be hidden from the toolbox"
QgsProcessingAlgorithm.FlagHideFromModeler = Qgis.ProcessingAlgorithmFlag.HideFromModeler
QgsProcessingAlgorithm.Flag.FlagHideFromModeler = Qgis.ProcessingAlgorithmFlag.HideFromModeler
QgsProcessingAlgorithm.FlagHideFromModeler.is_monkey_patched = True
QgsProcessingAlgorithm.FlagHideFromModeler.__doc__ = "Algorithm should be hidden from the modeler"
QgsProcessingAlgorithm.FlagSupportsBatch = Qgis.ProcessingAlgorithmFlag.SupportsBatch
QgsProcessingAlgorithm.Flag.FlagSupportsBatch = Qgis.ProcessingAlgorithmFlag.SupportsBatch
QgsProcessingAlgorithm.FlagSupportsBatch.is_monkey_patched = True
QgsProcessingAlgorithm.FlagSupportsBatch.__doc__ = "Algorithm supports batch mode"
QgsProcessingAlgorithm.FlagCanCancel = Qgis.ProcessingAlgorithmFlag.CanCancel
QgsProcessingAlgorithm.Flag.FlagCanCancel = Qgis.ProcessingAlgorithmFlag.CanCancel
QgsProcessingAlgorithm.FlagCanCancel.is_monkey_patched = True
QgsProcessingAlgorithm.FlagCanCancel.__doc__ = "Algorithm can be canceled"
QgsProcessingAlgorithm.FlagRequiresMatchingCrs = Qgis.ProcessingAlgorithmFlag.RequiresMatchingCrs
QgsProcessingAlgorithm.Flag.FlagRequiresMatchingCrs = Qgis.ProcessingAlgorithmFlag.RequiresMatchingCrs
QgsProcessingAlgorithm.FlagRequiresMatchingCrs.is_monkey_patched = True
QgsProcessingAlgorithm.FlagRequiresMatchingCrs.__doc__ = "Algorithm requires that all input layers have matching coordinate reference systems"
QgsProcessingAlgorithm.FlagNoThreading = Qgis.ProcessingAlgorithmFlag.NoThreading
QgsProcessingAlgorithm.Flag.FlagNoThreading = Qgis.ProcessingAlgorithmFlag.NoThreading
QgsProcessingAlgorithm.FlagNoThreading.is_monkey_patched = True
QgsProcessingAlgorithm.FlagNoThreading.__doc__ = "Algorithm is not thread safe and cannot be run in a background thread, e.g. for algorithms which manipulate the current project, layer selections, or with external dependencies which are not thread-safe."
QgsProcessingAlgorithm.FlagDisplayNameIsLiteral = Qgis.ProcessingAlgorithmFlag.DisplayNameIsLiteral
QgsProcessingAlgorithm.Flag.FlagDisplayNameIsLiteral = Qgis.ProcessingAlgorithmFlag.DisplayNameIsLiteral
QgsProcessingAlgorithm.FlagDisplayNameIsLiteral.is_monkey_patched = True
QgsProcessingAlgorithm.FlagDisplayNameIsLiteral.__doc__ = "Algorithm's display name is a static literal string, and should not be translated or automatically formatted. For use with algorithms named after commands, e.g. GRASS 'v.in.ogr'."
QgsProcessingAlgorithm.FlagSupportsInPlaceEdits = Qgis.ProcessingAlgorithmFlag.SupportsInPlaceEdits
QgsProcessingAlgorithm.Flag.FlagSupportsInPlaceEdits = Qgis.ProcessingAlgorithmFlag.SupportsInPlaceEdits
QgsProcessingAlgorithm.FlagSupportsInPlaceEdits.is_monkey_patched = True
QgsProcessingAlgorithm.FlagSupportsInPlaceEdits.__doc__ = "Algorithm supports in-place editing"
QgsProcessingAlgorithm.FlagKnownIssues = Qgis.ProcessingAlgorithmFlag.KnownIssues
QgsProcessingAlgorithm.Flag.FlagKnownIssues = Qgis.ProcessingAlgorithmFlag.KnownIssues
QgsProcessingAlgorithm.FlagKnownIssues.is_monkey_patched = True
QgsProcessingAlgorithm.FlagKnownIssues.__doc__ = "Algorithm has known issues"
QgsProcessingAlgorithm.FlagCustomException = Qgis.ProcessingAlgorithmFlag.CustomException
QgsProcessingAlgorithm.Flag.FlagCustomException = Qgis.ProcessingAlgorithmFlag.CustomException
QgsProcessingAlgorithm.FlagCustomException.is_monkey_patched = True
QgsProcessingAlgorithm.FlagCustomException.__doc__ = "Algorithm raises custom exception notices, don't use the standard ones"
QgsProcessingAlgorithm.FlagPruneModelBranchesBasedOnAlgorithmResults = Qgis.ProcessingAlgorithmFlag.PruneModelBranchesBasedOnAlgorithmResults
QgsProcessingAlgorithm.Flag.FlagPruneModelBranchesBasedOnAlgorithmResults = Qgis.ProcessingAlgorithmFlag.PruneModelBranchesBasedOnAlgorithmResults
QgsProcessingAlgorithm.FlagPruneModelBranchesBasedOnAlgorithmResults.is_monkey_patched = True
QgsProcessingAlgorithm.FlagPruneModelBranchesBasedOnAlgorithmResults.__doc__ = "Algorithm results will cause remaining model branches to be pruned based on the results of running the algorithm"
QgsProcessingAlgorithm.FlagSkipGenericModelLogging = Qgis.ProcessingAlgorithmFlag.SkipGenericModelLogging
QgsProcessingAlgorithm.Flag.FlagSkipGenericModelLogging = Qgis.ProcessingAlgorithmFlag.SkipGenericModelLogging
QgsProcessingAlgorithm.FlagSkipGenericModelLogging.is_monkey_patched = True
QgsProcessingAlgorithm.FlagSkipGenericModelLogging.__doc__ = "When running as part of a model, the generic algorithm setup and results logging should be skipped"
QgsProcessingAlgorithm.FlagNotAvailableInStandaloneTool = Qgis.ProcessingAlgorithmFlag.NotAvailableInStandaloneTool
QgsProcessingAlgorithm.Flag.FlagNotAvailableInStandaloneTool = Qgis.ProcessingAlgorithmFlag.NotAvailableInStandaloneTool
QgsProcessingAlgorithm.FlagNotAvailableInStandaloneTool.is_monkey_patched = True
QgsProcessingAlgorithm.FlagNotAvailableInStandaloneTool.__doc__ = "Algorithm should not be available from the standalone \"qgis_process\" tool. Used to flag algorithms which make no sense outside of the QGIS application, such as \"select by...\" style algorithms."
QgsProcessingAlgorithm.FlagRequiresProject = Qgis.ProcessingAlgorithmFlag.RequiresProject
QgsProcessingAlgorithm.Flag.FlagRequiresProject = Qgis.ProcessingAlgorithmFlag.RequiresProject
QgsProcessingAlgorithm.FlagRequiresProject.is_monkey_patched = True
QgsProcessingAlgorithm.FlagRequiresProject.__doc__ = "The algorithm requires that a valid QgsProject is available from the processing context in order to execute"
QgsProcessingAlgorithm.FlagDeprecated = Qgis.ProcessingAlgorithmFlag.Deprecated
QgsProcessingAlgorithm.Flag.FlagDeprecated = Qgis.ProcessingAlgorithmFlag.Deprecated
QgsProcessingAlgorithm.FlagDeprecated.is_monkey_patched = True
QgsProcessingAlgorithm.FlagDeprecated.__doc__ = "Algorithm is deprecated"
Qgis.ProcessingAlgorithmFlag.__doc__ = "Flags indicating how and when an algorithm operates and should be exposed to users.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingAlgorithm`.Flag\n\n.. versionadded:: 3.36\n\n" + '* ``FlagHideFromToolbox``: ' + Qgis.ProcessingAlgorithmFlag.HideFromToolbox.__doc__ + '\n' + '* ``FlagHideFromModeler``: ' + Qgis.ProcessingAlgorithmFlag.HideFromModeler.__doc__ + '\n' + '* ``FlagSupportsBatch``: ' + Qgis.ProcessingAlgorithmFlag.SupportsBatch.__doc__ + '\n' + '* ``FlagCanCancel``: ' + Qgis.ProcessingAlgorithmFlag.CanCancel.__doc__ + '\n' + '* ``FlagRequiresMatchingCrs``: ' + Qgis.ProcessingAlgorithmFlag.RequiresMatchingCrs.__doc__ + '\n' + '* ``FlagNoThreading``: ' + Qgis.ProcessingAlgorithmFlag.NoThreading.__doc__ + '\n' + '* ``FlagDisplayNameIsLiteral``: ' + Qgis.ProcessingAlgorithmFlag.DisplayNameIsLiteral.__doc__ + '\n' + '* ``FlagSupportsInPlaceEdits``: ' + Qgis.ProcessingAlgorithmFlag.SupportsInPlaceEdits.__doc__ + '\n' + '* ``FlagKnownIssues``: ' + Qgis.ProcessingAlgorithmFlag.KnownIssues.__doc__ + '\n' + '* ``FlagCustomException``: ' + Qgis.ProcessingAlgorithmFlag.CustomException.__doc__ + '\n' + '* ``FlagPruneModelBranchesBasedOnAlgorithmResults``: ' + Qgis.ProcessingAlgorithmFlag.PruneModelBranchesBasedOnAlgorithmResults.__doc__ + '\n' + '* ``FlagSkipGenericModelLogging``: ' + Qgis.ProcessingAlgorithmFlag.SkipGenericModelLogging.__doc__ + '\n' + '* ``FlagNotAvailableInStandaloneTool``: ' + Qgis.ProcessingAlgorithmFlag.NotAvailableInStandaloneTool.__doc__ + '\n' + '* ``FlagRequiresProject``: ' + Qgis.ProcessingAlgorithmFlag.RequiresProject.__doc__ + '\n' + '* ``FlagDeprecated``: ' + Qgis.ProcessingAlgorithmFlag.Deprecated.__doc__
# --
Qgis.ProcessingAlgorithmFlag.baseClass = Qgis
Qgis.ProcessingAlgorithmFlags = lambda flags=0: Qgis.ProcessingAlgorithmFlag(flags)
QgsProcessingAlgorithm.Flags = Qgis.ProcessingAlgorithmFlags
Qgis.ProcessingAlgorithmFlags.baseClass = Qgis
ProcessingAlgorithmFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingAlgorithm.PropertyAvailability = Qgis.ProcessingPropertyAvailability
# monkey patching scoped based enum
QgsProcessingAlgorithm.NotAvailable = Qgis.ProcessingPropertyAvailability.NotAvailable
QgsProcessingAlgorithm.NotAvailable.is_monkey_patched = True
QgsProcessingAlgorithm.NotAvailable.__doc__ = "Properties are not available"
QgsProcessingAlgorithm.Available = Qgis.ProcessingPropertyAvailability.Available
QgsProcessingAlgorithm.Available.is_monkey_patched = True
QgsProcessingAlgorithm.Available.__doc__ = "Properties are available"
Qgis.ProcessingPropertyAvailability.__doc__ = "Property availability, used for :py:class:`QgsProcessingAlgorithm`.VectorProperties\nin order to determine if properties are available or not.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingAlgorithm`.PropertyAvailability\n\n.. versionadded:: 3.36\n\n" + '* ``NotAvailable``: ' + Qgis.ProcessingPropertyAvailability.NotAvailable.__doc__ + '\n' + '* ``Available``: ' + Qgis.ProcessingPropertyAvailability.Available.__doc__
# --
Qgis.ProcessingPropertyAvailability.baseClass = Qgis
QgsProcessingContext.LogLevel = Qgis.ProcessingLogLevel
# monkey patching scoped based enum
QgsProcessingContext.DefaultLevel = Qgis.ProcessingLogLevel.DefaultLevel
QgsProcessingContext.DefaultLevel.is_monkey_patched = True
QgsProcessingContext.DefaultLevel.__doc__ = "Default logging level"
QgsProcessingContext.Verbose = Qgis.ProcessingLogLevel.Verbose
QgsProcessingContext.Verbose.is_monkey_patched = True
QgsProcessingContext.Verbose.__doc__ = "Verbose logging"
QgsProcessingContext.ModelDebug = Qgis.ProcessingLogLevel.ModelDebug
QgsProcessingContext.ModelDebug.is_monkey_patched = True
QgsProcessingContext.ModelDebug.__doc__ = "Model debug level logging. Includes verbose logging and other outputs useful for debugging models (since QGIS 3.34)."
Qgis.ProcessingLogLevel.__doc__ = "Logging level for algorithms to use when pushing feedback messages.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingContext`.LogLevel\n\n.. versionadded:: 3.36\n\n" + '* ``DefaultLevel``: ' + Qgis.ProcessingLogLevel.DefaultLevel.__doc__ + '\n' + '* ``Verbose``: ' + Qgis.ProcessingLogLevel.Verbose.__doc__ + '\n' + '* ``ModelDebug``: ' + Qgis.ProcessingLogLevel.ModelDebug.__doc__
# --
Qgis.ProcessingLogLevel.baseClass = Qgis
QgsProcessingFeatureSourceDefinition.Flag = Qgis.ProcessingFeatureSourceDefinitionFlag
# monkey patching scoped based enum
QgsProcessingFeatureSourceDefinition.FlagOverrideDefaultGeometryCheck = Qgis.ProcessingFeatureSourceDefinitionFlag.OverrideDefaultGeometryCheck
QgsProcessingFeatureSourceDefinition.Flag.FlagOverrideDefaultGeometryCheck = Qgis.ProcessingFeatureSourceDefinitionFlag.OverrideDefaultGeometryCheck
QgsProcessingFeatureSourceDefinition.FlagOverrideDefaultGeometryCheck.is_monkey_patched = True
QgsProcessingFeatureSourceDefinition.FlagOverrideDefaultGeometryCheck.__doc__ = "If set, the default geometry check method (as dictated by QgsProcessingContext) will be overridden for this source"
QgsProcessingFeatureSourceDefinition.FlagCreateIndividualOutputPerInputFeature = Qgis.ProcessingFeatureSourceDefinitionFlag.CreateIndividualOutputPerInputFeature
QgsProcessingFeatureSourceDefinition.Flag.FlagCreateIndividualOutputPerInputFeature = Qgis.ProcessingFeatureSourceDefinitionFlag.CreateIndividualOutputPerInputFeature
QgsProcessingFeatureSourceDefinition.FlagCreateIndividualOutputPerInputFeature.is_monkey_patched = True
QgsProcessingFeatureSourceDefinition.FlagCreateIndividualOutputPerInputFeature.__doc__ = "If set, every feature processed from this source will be placed into its own individually created output destination. Support for this flag depends on how an algorithm is executed."
Qgis.ProcessingFeatureSourceDefinitionFlag.__doc__ = "Flags which control behavior for a Processing feature source.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingFeatureSourceDefinition`.Flag\n\n.. versionadded:: 3.36\n\n" + '* ``FlagOverrideDefaultGeometryCheck``: ' + Qgis.ProcessingFeatureSourceDefinitionFlag.OverrideDefaultGeometryCheck.__doc__ + '\n' + '* ``FlagCreateIndividualOutputPerInputFeature``: ' + Qgis.ProcessingFeatureSourceDefinitionFlag.CreateIndividualOutputPerInputFeature.__doc__
# --
Qgis.ProcessingFeatureSourceDefinitionFlag.baseClass = Qgis
Qgis.ProcessingFeatureSourceDefinitionFlags = lambda flags=0: Qgis.ProcessingFeatureSourceDefinitionFlag(flags)
QgsProcessingFeatureSourceDefinition.Flags = Qgis.ProcessingFeatureSourceDefinitionFlags
Qgis.ProcessingFeatureSourceDefinitionFlags.baseClass = Qgis
ProcessingFeatureSourceDefinitionFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingFeatureSource.Flag = Qgis.ProcessingFeatureSourceFlag
# monkey patching scoped based enum
QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks = Qgis.ProcessingFeatureSourceFlag.SkipGeometryValidityChecks
QgsProcessingFeatureSource.Flag.FlagSkipGeometryValidityChecks = Qgis.ProcessingFeatureSourceFlag.SkipGeometryValidityChecks
QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks.is_monkey_patched = True
QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks.__doc__ = "Invalid geometry checks should always be skipped. This flag can be useful for algorithms which always require invalid geometries, regardless of any user settings (e.g. \"repair geometry\" type algorithms)."
Qgis.ProcessingFeatureSourceFlag.__doc__ = "Flags which control how :py:class:`QgsProcessingFeatureSource` fetches features.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingFeatureSource`.Flag\n\n.. versionadded:: 3.36\n\n" + '* ``FlagSkipGeometryValidityChecks``: ' + Qgis.ProcessingFeatureSourceFlag.SkipGeometryValidityChecks.__doc__
# --
Qgis.ProcessingFeatureSourceFlag.baseClass = Qgis
Qgis.ProcessingFeatureSourceFlags = lambda flags=0: Qgis.ProcessingFeatureSourceFlag(flags)
QgsProcessingFeatureSource.Flags = Qgis.ProcessingFeatureSourceFlags
Qgis.ProcessingFeatureSourceFlags.baseClass = Qgis
ProcessingFeatureSourceFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingParameterType.ParameterFlag = Qgis.ProcessingParameterTypeFlag
# monkey patching scoped based enum
QgsProcessingParameterType.ExposeToModeler = Qgis.ProcessingParameterTypeFlag.ExposeToModeler
QgsProcessingParameterType.ExposeToModeler.is_monkey_patched = True
QgsProcessingParameterType.ExposeToModeler.__doc__ = "Is this parameter available in the modeler. Is set to on by default."
Qgis.ProcessingParameterTypeFlag.__doc__ = "Flags which dictate the behavior of Processing parameter types.\n\nEach parameter type can offer a number of additional flags to fine tune its behavior\nand capabilities.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterType`.ParameterFlag\n\n.. versionadded:: 3.36\n\n" + '* ``ExposeToModeler``: ' + Qgis.ProcessingParameterTypeFlag.ExposeToModeler.__doc__
# --
Qgis.ProcessingParameterTypeFlag.baseClass = Qgis
Qgis.ProcessingParameterTypeFlags = lambda flags=0: Qgis.ProcessingParameterTypeFlag(flags)
QgsProcessingParameterType.ParameterFlags = Qgis.ProcessingParameterTypeFlags
Qgis.ProcessingParameterTypeFlags.baseClass = Qgis
ProcessingParameterTypeFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingParameterDefinition.Flag = Qgis.ProcessingParameterFlag
# monkey patching scoped based enum
QgsProcessingParameterDefinition.FlagAdvanced = Qgis.ProcessingParameterFlag.Advanced
QgsProcessingParameterDefinition.Flag.FlagAdvanced = Qgis.ProcessingParameterFlag.Advanced
QgsProcessingParameterDefinition.FlagAdvanced.is_monkey_patched = True
QgsProcessingParameterDefinition.FlagAdvanced.__doc__ = "Parameter is an advanced parameter which should be hidden from users by default"
QgsProcessingParameterDefinition.FlagHidden = Qgis.ProcessingParameterFlag.Hidden
QgsProcessingParameterDefinition.Flag.FlagHidden = Qgis.ProcessingParameterFlag.Hidden
QgsProcessingParameterDefinition.FlagHidden.is_monkey_patched = True
QgsProcessingParameterDefinition.FlagHidden.__doc__ = "Parameter is hidden and should not be shown to users"
QgsProcessingParameterDefinition.FlagOptional = Qgis.ProcessingParameterFlag.Optional
QgsProcessingParameterDefinition.Flag.FlagOptional = Qgis.ProcessingParameterFlag.Optional
QgsProcessingParameterDefinition.FlagOptional.is_monkey_patched = True
QgsProcessingParameterDefinition.FlagOptional.__doc__ = "Parameter is optional"
QgsProcessingParameterDefinition.FlagIsModelOutput = Qgis.ProcessingParameterFlag.IsModelOutput
QgsProcessingParameterDefinition.Flag.FlagIsModelOutput = Qgis.ProcessingParameterFlag.IsModelOutput
QgsProcessingParameterDefinition.FlagIsModelOutput.is_monkey_patched = True
QgsProcessingParameterDefinition.FlagIsModelOutput.__doc__ = "Destination parameter is final output. The parameter name will be used."
Qgis.ProcessingParameterFlag.__doc__ = "Flags which dictate the behavior of Processing parameters.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterDefinition`.Flag\n\n.. versionadded:: 3.36\n\n" + '* ``FlagAdvanced``: ' + Qgis.ProcessingParameterFlag.Advanced.__doc__ + '\n' + '* ``FlagHidden``: ' + Qgis.ProcessingParameterFlag.Hidden.__doc__ + '\n' + '* ``FlagOptional``: ' + Qgis.ProcessingParameterFlag.Optional.__doc__ + '\n' + '* ``FlagIsModelOutput``: ' + Qgis.ProcessingParameterFlag.IsModelOutput.__doc__
# --
Qgis.ProcessingParameterFlag.baseClass = Qgis
Qgis.ProcessingParameterFlags = lambda flags=0: Qgis.ProcessingParameterFlag(flags)
QgsProcessingParameterDefinition.Flags = Qgis.ProcessingParameterFlags
Qgis.ProcessingParameterFlags.baseClass = Qgis
ProcessingParameterFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingParameterFile.Behavior = Qgis.ProcessingFileParameterBehavior
# monkey patching scoped based enum
QgsProcessingParameterFile.File = Qgis.ProcessingFileParameterBehavior.File
QgsProcessingParameterFile.File.is_monkey_patched = True
QgsProcessingParameterFile.File.__doc__ = "Parameter is a single file"
QgsProcessingParameterFile.Folder = Qgis.ProcessingFileParameterBehavior.Folder
QgsProcessingParameterFile.Folder.is_monkey_patched = True
QgsProcessingParameterFile.Folder.__doc__ = "Parameter is a folder"
Qgis.ProcessingFileParameterBehavior.__doc__ = "Flags which dictate the behavior of :py:class:`QgsProcessingParameterFile`.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterFile`.Behavior\n\n.. versionadded:: 3.36\n\n" + '* ``File``: ' + Qgis.ProcessingFileParameterBehavior.File.__doc__ + '\n' + '* ``Folder``: ' + Qgis.ProcessingFileParameterBehavior.Folder.__doc__
# --
Qgis.ProcessingFileParameterBehavior.baseClass = Qgis
QgsProcessingParameterNumber.Type = Qgis.ProcessingNumberParameterType
# monkey patching scoped based enum
QgsProcessingParameterNumber.Integer = Qgis.ProcessingNumberParameterType.Integer
QgsProcessingParameterNumber.Integer.is_monkey_patched = True
QgsProcessingParameterNumber.Integer.__doc__ = "Integer values"
QgsProcessingParameterNumber.Double = Qgis.ProcessingNumberParameterType.Double
QgsProcessingParameterNumber.Double.is_monkey_patched = True
QgsProcessingParameterNumber.Double.__doc__ = "Double/float values"
Qgis.ProcessingNumberParameterType.__doc__ = "Processing numeric parameter data types.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterNumber`.Type\n\n.. versionadded:: 3.36\n\n" + '* ``Integer``: ' + Qgis.ProcessingNumberParameterType.Integer.__doc__ + '\n' + '* ``Double``: ' + Qgis.ProcessingNumberParameterType.Double.__doc__
# --
Qgis.ProcessingNumberParameterType.baseClass = Qgis
QgsProcessingParameterField.DataType = Qgis.ProcessingFieldParameterDataType
# monkey patching scoped based enum
QgsProcessingParameterField.Any = Qgis.ProcessingFieldParameterDataType.Any
QgsProcessingParameterField.Any.is_monkey_patched = True
QgsProcessingParameterField.Any.__doc__ = "Accepts any field"
QgsProcessingParameterField.Numeric = Qgis.ProcessingFieldParameterDataType.Numeric
QgsProcessingParameterField.Numeric.is_monkey_patched = True
QgsProcessingParameterField.Numeric.__doc__ = "Accepts numeric fields"
QgsProcessingParameterField.String = Qgis.ProcessingFieldParameterDataType.String
QgsProcessingParameterField.String.is_monkey_patched = True
QgsProcessingParameterField.String.__doc__ = "Accepts string fields"
QgsProcessingParameterField.DateTime = Qgis.ProcessingFieldParameterDataType.DateTime
QgsProcessingParameterField.DateTime.is_monkey_patched = True
QgsProcessingParameterField.DateTime.__doc__ = "Accepts datetime fields"
QgsProcessingParameterField.Binary = Qgis.ProcessingFieldParameterDataType.Binary
QgsProcessingParameterField.Binary.is_monkey_patched = True
QgsProcessingParameterField.Binary.__doc__ = "Accepts binary fields, since QGIS 3.34"
QgsProcessingParameterField.Boolean = Qgis.ProcessingFieldParameterDataType.Boolean
QgsProcessingParameterField.Boolean.is_monkey_patched = True
QgsProcessingParameterField.Boolean.__doc__ = "Accepts boolean fields, since QGIS 3.34"
Qgis.ProcessingFieldParameterDataType.__doc__ = "Processing field parameter data types.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterField`.DataType\n\n.. versionadded:: 3.36\n\n" + '* ``Any``: ' + Qgis.ProcessingFieldParameterDataType.Any.__doc__ + '\n' + '* ``Numeric``: ' + Qgis.ProcessingFieldParameterDataType.Numeric.__doc__ + '\n' + '* ``String``: ' + Qgis.ProcessingFieldParameterDataType.String.__doc__ + '\n' + '* ``DateTime``: ' + Qgis.ProcessingFieldParameterDataType.DateTime.__doc__ + '\n' + '* ``Binary``: ' + Qgis.ProcessingFieldParameterDataType.Binary.__doc__ + '\n' + '* ``Boolean``: ' + Qgis.ProcessingFieldParameterDataType.Boolean.__doc__
# --
Qgis.ProcessingFieldParameterDataType.baseClass = Qgis
QgsProcessingParameterDateTime.Type = Qgis.ProcessingDateTimeParameterDataType
# monkey patching scoped based enum
QgsProcessingParameterDateTime.DateTime = Qgis.ProcessingDateTimeParameterDataType.DateTime
QgsProcessingParameterDateTime.DateTime.is_monkey_patched = True
QgsProcessingParameterDateTime.DateTime.__doc__ = "Datetime values"
QgsProcessingParameterDateTime.Date = Qgis.ProcessingDateTimeParameterDataType.Date
QgsProcessingParameterDateTime.Date.is_monkey_patched = True
QgsProcessingParameterDateTime.Date.__doc__ = "Date values"
QgsProcessingParameterDateTime.Time = Qgis.ProcessingDateTimeParameterDataType.Time
QgsProcessingParameterDateTime.Time.is_monkey_patched = True
QgsProcessingParameterDateTime.Time.__doc__ = "Time values"
Qgis.ProcessingDateTimeParameterDataType.__doc__ = "Processing date time parameter data types.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterDateTime`.Type\n\n.. versionadded:: 3.36\n\n" + '* ``DateTime``: ' + Qgis.ProcessingDateTimeParameterDataType.DateTime.__doc__ + '\n' + '* ``Date``: ' + Qgis.ProcessingDateTimeParameterDataType.Date.__doc__ + '\n' + '* ``Time``: ' + Qgis.ProcessingDateTimeParameterDataType.Time.__doc__
# --
Qgis.ProcessingDateTimeParameterDataType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProcessingModelChildParameterSource.ModelParameter.__doc__ = "Parameter value is taken from a parent model parameter"
Qgis.ProcessingModelChildParameterSource.ChildOutput.__doc__ = "Parameter value is taken from an output generated by a child algorithm"
Qgis.ProcessingModelChildParameterSource.StaticValue.__doc__ = "Parameter value is a static value"
Qgis.ProcessingModelChildParameterSource.Expression.__doc__ = "Parameter value is taken from an expression, evaluated just before the algorithm runs"
Qgis.ProcessingModelChildParameterSource.ExpressionText.__doc__ = "Parameter value is taken from a text with expressions, evaluated just before the algorithm runs"
Qgis.ProcessingModelChildParameterSource.ModelOutput.__doc__ = "Parameter value is linked to an output parameter for the model"
Qgis.ProcessingModelChildParameterSource.__doc__ = "Processing model child parameter sources.\n\n.. versionadded:: 3.34\n\n" + '* ``ModelParameter``: ' + Qgis.ProcessingModelChildParameterSource.ModelParameter.__doc__ + '\n' + '* ``ChildOutput``: ' + Qgis.ProcessingModelChildParameterSource.ChildOutput.__doc__ + '\n' + '* ``StaticValue``: ' + Qgis.ProcessingModelChildParameterSource.StaticValue.__doc__ + '\n' + '* ``Expression``: ' + Qgis.ProcessingModelChildParameterSource.Expression.__doc__ + '\n' + '* ``ExpressionText``: ' + Qgis.ProcessingModelChildParameterSource.ExpressionText.__doc__ + '\n' + '* ``ModelOutput``: ' + Qgis.ProcessingModelChildParameterSource.ModelOutput.__doc__
# --
Qgis.ProcessingModelChildParameterSource.baseClass = Qgis
QgsProcessingParameterTinInputLayers.Type = Qgis.ProcessingTinInputLayerType
# monkey patching scoped based enum
QgsProcessingParameterTinInputLayers.Vertices = Qgis.ProcessingTinInputLayerType.Vertices
QgsProcessingParameterTinInputLayers.Vertices.is_monkey_patched = True
QgsProcessingParameterTinInputLayers.Vertices.__doc__ = "Input that adds only vertices"
QgsProcessingParameterTinInputLayers.StructureLines = Qgis.ProcessingTinInputLayerType.StructureLines
QgsProcessingParameterTinInputLayers.StructureLines.is_monkey_patched = True
QgsProcessingParameterTinInputLayers.StructureLines.__doc__ = "Input that adds add structure lines"
QgsProcessingParameterTinInputLayers.BreakLines = Qgis.ProcessingTinInputLayerType.BreakLines
QgsProcessingParameterTinInputLayers.BreakLines.is_monkey_patched = True
QgsProcessingParameterTinInputLayers.BreakLines.__doc__ = "Input that adds vertices and break lines"
Qgis.ProcessingTinInputLayerType.__doc__ = "Defines the type of input layer for a Processing TIN input.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterTinInputLayers`.Type\n\n.. versionadded:: 3.36\n\n" + '* ``Vertices``: ' + Qgis.ProcessingTinInputLayerType.Vertices.__doc__ + '\n' + '* ``StructureLines``: ' + Qgis.ProcessingTinInputLayerType.StructureLines.__doc__ + '\n' + '* ``BreakLines``: ' + Qgis.ProcessingTinInputLayerType.BreakLines.__doc__
# --
Qgis.ProcessingTinInputLayerType.baseClass = Qgis
QgsCoordinateReferenceSystem.Format = Qgis.CrsDefinitionFormat
# monkey patching scoped based enum
QgsCoordinateReferenceSystem.FormatWkt = Qgis.CrsDefinitionFormat.Wkt
QgsCoordinateReferenceSystem.Format.FormatWkt = Qgis.CrsDefinitionFormat.Wkt
QgsCoordinateReferenceSystem.FormatWkt.is_monkey_patched = True
QgsCoordinateReferenceSystem.FormatWkt.__doc__ = "WKT format (always recommended over proj string format)"
QgsCoordinateReferenceSystem.FormatProj = Qgis.CrsDefinitionFormat.Proj
QgsCoordinateReferenceSystem.Format.FormatProj = Qgis.CrsDefinitionFormat.Proj
QgsCoordinateReferenceSystem.FormatProj.is_monkey_patched = True
QgsCoordinateReferenceSystem.FormatProj.__doc__ = "Proj string format"
Qgis.CrsDefinitionFormat.__doc__ = "CRS definition formats.\n\n.. versionadded:: 3.24\n\n" + '* ``FormatWkt``: ' + Qgis.CrsDefinitionFormat.Wkt.__doc__ + '\n' + '* ``FormatProj``: ' + Qgis.CrsDefinitionFormat.Proj.__doc__
# --
Qgis.CrsDefinitionFormat.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldDomainSplitPolicy.DefaultValue.__doc__ = "Use default field value"
Qgis.FieldDomainSplitPolicy.Duplicate.__doc__ = "Duplicate original value"
Qgis.FieldDomainSplitPolicy.GeometryRatio.__doc__ = "New values are computed by the ratio of their area/length compared to the area/length of the original feature"
Qgis.FieldDomainSplitPolicy.UnsetField.__doc__ = "Clears the field value so that the data provider backend will populate using any backend triggers or similar logic (since QGIS 3.30)"
Qgis.FieldDomainSplitPolicy.__doc__ = "Split policy for field domains.\n\nWhen a feature is split into multiple parts, defines how the value of attributes\nfollowing the domain are computed.\n\n.. versionadded:: 3.26\n\n" + '* ``DefaultValue``: ' + Qgis.FieldDomainSplitPolicy.DefaultValue.__doc__ + '\n' + '* ``Duplicate``: ' + Qgis.FieldDomainSplitPolicy.Duplicate.__doc__ + '\n' + '* ``GeometryRatio``: ' + Qgis.FieldDomainSplitPolicy.GeometryRatio.__doc__ + '\n' + '* ``UnsetField``: ' + Qgis.FieldDomainSplitPolicy.UnsetField.__doc__
# --
Qgis.FieldDomainSplitPolicy.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldDomainMergePolicy.DefaultValue.__doc__ = "Use default field value"
Qgis.FieldDomainMergePolicy.Sum.__doc__ = "Sum of values"
Qgis.FieldDomainMergePolicy.GeometryWeighted.__doc__ = "New values are computed as the weighted average of the source values"
Qgis.FieldDomainMergePolicy.__doc__ = "Merge policy for field domains.\n\nWhen a feature is built by merging multiple features, defines how the value of\nattributes following the domain are computed.\n\n.. versionadded:: 3.26\n\n" + '* ``DefaultValue``: ' + Qgis.FieldDomainMergePolicy.DefaultValue.__doc__ + '\n' + '* ``Sum``: ' + Qgis.FieldDomainMergePolicy.Sum.__doc__ + '\n' + '* ``GeometryWeighted``: ' + Qgis.FieldDomainMergePolicy.GeometryWeighted.__doc__
# --
Qgis.FieldDomainMergePolicy.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldDomainType.Coded.__doc__ = "Coded field domain"
Qgis.FieldDomainType.Range.__doc__ = "Numeric range field domain (min/max)"
Qgis.FieldDomainType.Glob.__doc__ = "Glob string pattern field domain"
Qgis.FieldDomainType.__doc__ = "Types of field domain\n\n.. versionadded:: 3.26\n\n" + '* ``Coded``: ' + Qgis.FieldDomainType.Coded.__doc__ + '\n' + '* ``Range``: ' + Qgis.FieldDomainType.Range.__doc__ + '\n' + '* ``Glob``: ' + Qgis.FieldDomainType.Glob.__doc__
# --
Qgis.FieldDomainType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TransactionMode.Disabled.__doc__ = "Edits are buffered locally and sent to the provider when toggling layer editing mode."
Qgis.TransactionMode.AutomaticGroups.__doc__ = "Automatic transactional editing means that on supported datasources (postgres and geopackage databases) the edit state of all tables that originate from the same database are synchronized and executed in a server side transaction."
Qgis.TransactionMode.BufferedGroups.__doc__ = "Buffered transactional editing means that all editable layers in the buffered transaction group are toggled synchronously and all edits are saved in a local edit buffer. Saving changes is executed within a single transaction on all layers (per provider)."
Qgis.TransactionMode.__doc__ = "Transaction mode.\n\n.. versionadded:: 3.26\n\n" + '* ``Disabled``: ' + Qgis.TransactionMode.Disabled.__doc__ + '\n' + '* ``AutomaticGroups``: ' + Qgis.TransactionMode.AutomaticGroups.__doc__ + '\n' + '* ``BufferedGroups``: ' + Qgis.TransactionMode.BufferedGroups.__doc__
# --
Qgis.TransactionMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AltitudeClamping.Absolute.__doc__ = "Elevation is taken directly from feature and is independent of terrain height (final elevation = feature elevation)"
Qgis.AltitudeClamping.Relative.__doc__ = "Elevation is relative to terrain height (final elevation = terrain elevation + feature elevation)"
Qgis.AltitudeClamping.Terrain.__doc__ = "Elevation is clamped to terrain (final elevation = terrain elevation)"
Qgis.AltitudeClamping.__doc__ = "Altitude clamping.\n\n.. versionadded:: 3.26\n\n" + '* ``Absolute``: ' + Qgis.AltitudeClamping.Absolute.__doc__ + '\n' + '* ``Relative``: ' + Qgis.AltitudeClamping.Relative.__doc__ + '\n' + '* ``Terrain``: ' + Qgis.AltitudeClamping.Terrain.__doc__
# --
Qgis.AltitudeClamping.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AltitudeBinding.Vertex.__doc__ = "Clamp every vertex of feature"
Qgis.AltitudeBinding.Centroid.__doc__ = "Clamp just centroid of feature"
Qgis.AltitudeBinding.__doc__ = "Altitude binding.\n\n.. versionadded:: 3.26\n\n" + '* ``Vertex``: ' + Qgis.AltitudeBinding.Vertex.__doc__ + '\n' + '* ``Centroid``: ' + Qgis.AltitudeBinding.Centroid.__doc__
# --
Qgis.AltitudeBinding.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RangeLimits.IncludeBoth.__doc__ = "Both lower and upper values are included in the range"
Qgis.RangeLimits.IncludeLowerExcludeUpper.__doc__ = "Lower value is included in the range, upper value is excluded"
Qgis.RangeLimits.ExcludeLowerIncludeUpper.__doc__ = "Lower value is excluded from the range, upper value in inccluded"
Qgis.RangeLimits.ExcludeBoth.__doc__ = "Both lower and upper values are excluded from the range"
Qgis.RangeLimits.__doc__ = "Describes how the limits of a range are handled.\n\n.. versionadded:: 3.38\n\n" + '* ``IncludeBoth``: ' + Qgis.RangeLimits.IncludeBoth.__doc__ + '\n' + '* ``IncludeLowerExcludeUpper``: ' + Qgis.RangeLimits.IncludeLowerExcludeUpper.__doc__ + '\n' + '* ``ExcludeLowerIncludeUpper``: ' + Qgis.RangeLimits.ExcludeLowerIncludeUpper.__doc__ + '\n' + '* ``ExcludeBoth``: ' + Qgis.RangeLimits.ExcludeBoth.__doc__
# --
Qgis.RangeLimits.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RasterElevationMode.FixedElevationRange.__doc__ = "Layer has a fixed elevation range"
Qgis.RasterElevationMode.RepresentsElevationSurface.__doc__ = "Pixel values represent an elevation surface"
Qgis.RasterElevationMode.FixedRangePerBand.__doc__ = "Layer has a fixed (manually specified) elevation range per band"
Qgis.RasterElevationMode.DynamicRangePerBand.__doc__ = "Layer has a elevation range per band, calculated dynamically from an expression"
Qgis.RasterElevationMode.__doc__ = "Raster layer elevation modes.\n\n.. versionadded:: 3.38\n\n" + '* ``FixedElevationRange``: ' + Qgis.RasterElevationMode.FixedElevationRange.__doc__ + '\n' + '* ``RepresentsElevationSurface``: ' + Qgis.RasterElevationMode.RepresentsElevationSurface.__doc__ + '\n' + '* ``FixedRangePerBand``: ' + Qgis.RasterElevationMode.FixedRangePerBand.__doc__ + '\n' + '* ``DynamicRangePerBand``: ' + Qgis.RasterElevationMode.DynamicRangePerBand.__doc__
# --
Qgis.RasterElevationMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MeshElevationMode.FixedElevationRange.__doc__ = "Layer has a fixed elevation range"
Qgis.MeshElevationMode.FromVertices.__doc__ = "Elevation should be taken from mesh vertices"
Qgis.MeshElevationMode.__doc__ = "Mesh layer elevation modes.\n\n.. versionadded:: 3.38\n\n" + '* ``FixedElevationRange``: ' + Qgis.MeshElevationMode.FixedElevationRange.__doc__ + '\n' + '* ``FromVertices``: ' + Qgis.MeshElevationMode.FromVertices.__doc__
# --
Qgis.MeshElevationMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.NoConstraint = Qgis.BetweenLineConstraint.NoConstraint
Qgis.NoConstraint.is_monkey_patched = True
Qgis.BetweenLineConstraint.NoConstraint.__doc__ = "No additional constraint"
Qgis.Perpendicular = Qgis.BetweenLineConstraint.Perpendicular
Qgis.Perpendicular.is_monkey_patched = True
Qgis.BetweenLineConstraint.Perpendicular.__doc__ = "Perpendicular"
Qgis.Parallel = Qgis.BetweenLineConstraint.Parallel
Qgis.Parallel.is_monkey_patched = True
Qgis.BetweenLineConstraint.Parallel.__doc__ = "Parallel"
Qgis.BetweenLineConstraint.__doc__ = "Between line constraints which can be enabled\n\n.. versionadded:: 3.26\n\n" + '* ``NoConstraint``: ' + Qgis.BetweenLineConstraint.NoConstraint.__doc__ + '\n' + '* ``Perpendicular``: ' + Qgis.BetweenLineConstraint.Perpendicular.__doc__ + '\n' + '* ``Parallel``: ' + Qgis.BetweenLineConstraint.Parallel.__doc__
# --
Qgis.BetweenLineConstraint.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LineExtensionSide.BeforeVertex.__doc__ = "Lock to previous vertex"
Qgis.LineExtensionSide.AfterVertex.__doc__ = "Lock to next vertex"
Qgis.LineExtensionSide.NoVertex.__doc__ = "Don't lock to vertex"
Qgis.LineExtensionSide.__doc__ = "Designates whether the line extension constraint is currently soft locked\nwith the previous or next vertex of the locked one.\n\n.. versionadded:: 3.26\n\n" + '* ``BeforeVertex``: ' + Qgis.LineExtensionSide.BeforeVertex.__doc__ + '\n' + '* ``AfterVertex``: ' + Qgis.LineExtensionSide.AfterVertex.__doc__ + '\n' + '* ``NoVertex``: ' + Qgis.LineExtensionSide.NoVertex.__doc__
# --
Qgis.LineExtensionSide.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CadConstraintType.Generic.__doc__ = "Generic value"
Qgis.CadConstraintType.Angle.__doc__ = "Angle value"
Qgis.CadConstraintType.Distance.__doc__ = "Distance value"
Qgis.CadConstraintType.XCoordinate.__doc__ = "X Coordinate value"
Qgis.CadConstraintType.YCoordinate.__doc__ = "Y Coordinate value"
Qgis.CadConstraintType.ZValue.__doc__ = "Z value"
Qgis.CadConstraintType.MValue.__doc__ = "M value"
Qgis.CadConstraintType.__doc__ = "Advanced digitizing constraint type.\n\n.. versionadded:: 3.32\n\n" + '* ``Generic``: ' + Qgis.CadConstraintType.Generic.__doc__ + '\n' + '* ``Angle``: ' + Qgis.CadConstraintType.Angle.__doc__ + '\n' + '* ``Distance``: ' + Qgis.CadConstraintType.Distance.__doc__ + '\n' + '* ``XCoordinate``: ' + Qgis.CadConstraintType.XCoordinate.__doc__ + '\n' + '* ``YCoordinate``: ' + Qgis.CadConstraintType.YCoordinate.__doc__ + '\n' + '* ``ZValue``: ' + Qgis.CadConstraintType.ZValue.__doc__ + '\n' + '* ``MValue``: ' + Qgis.CadConstraintType.MValue.__doc__
# --
Qgis.CadConstraintType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProjectFlag.EvaluateDefaultValuesOnProviderSide.__doc__ = "If set, default values for fields will be evaluated on the provider side when features from the project are created instead of when they are committed."
Qgis.ProjectFlag.TrustStoredLayerStatistics.__doc__ = "If set, then layer statistics (such as the layer extent) will be read from values stored in the project instead of requesting updated values from the data provider. Additionally, when this flag is set, primary key unicity is not checked for views and materialized views with Postgres provider."
Qgis.ProjectFlag.RememberLayerEditStatusBetweenSessions.__doc__ = "If set, then any layers set to be editable will be stored in the project and immediately made editable whenever that project is restored"
Qgis.ProjectFlag.RememberAttributeTableWindowsBetweenSessions.__doc__ = "If set, then any open attribute tables will be stored in the project and immediately reopened when the project is restored"
Qgis.ProjectFlag.__doc__ = "Flags which control the behavior of :py:class:`QgsProjects`.\n\n.. versionadded:: 3.26\n\n" + '* ``EvaluateDefaultValuesOnProviderSide``: ' + Qgis.ProjectFlag.EvaluateDefaultValuesOnProviderSide.__doc__ + '\n' + '* ``TrustStoredLayerStatistics``: ' + Qgis.ProjectFlag.TrustStoredLayerStatistics.__doc__ + '\n' + '* ``RememberLayerEditStatusBetweenSessions``: ' + Qgis.ProjectFlag.RememberLayerEditStatusBetweenSessions.__doc__ + '\n' + '* ``RememberAttributeTableWindowsBetweenSessions``: ' + Qgis.ProjectFlag.RememberAttributeTableWindowsBetweenSessions.__doc__
# --
Qgis.ProjectFlag.baseClass = Qgis
Qgis.ProjectFlags = lambda flags=0: Qgis.ProjectFlag(flags)
Qgis.ProjectFlags.baseClass = Qgis
ProjectFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.PlotToolFlag.ShowContextMenu.__doc__ = "Show a context menu when right-clicking with the tool."
Qgis.PlotToolFlag.__doc__ = "Flags that control the way the :py:class:`QgsPlotTools` operate.\n\n.. versionadded:: 3.26\n\n" + '* ``ShowContextMenu``: ' + Qgis.PlotToolFlag.ShowContextMenu.__doc__
# --
Qgis.PlotToolFlag.baseClass = Qgis
Qgis.PlotToolFlags = lambda flags=0: Qgis.PlotToolFlag(flags)
Qgis.PlotToolFlags.baseClass = Qgis
PlotToolFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.Point3DShape.Cylinder.__doc__ = "Cylinder"
Qgis.Point3DShape.Sphere.__doc__ = "Sphere"
Qgis.Point3DShape.Cone.__doc__ = "Cone"
Qgis.Point3DShape.Cube.__doc__ = "Cube"
Qgis.Point3DShape.Torus.__doc__ = "Torus"
Qgis.Point3DShape.Plane.__doc__ = "Flat plane"
Qgis.Point3DShape.ExtrudedText.__doc__ = "Extruded text"
Qgis.Point3DShape.Model.__doc__ = "Model"
Qgis.Point3DShape.Billboard.__doc__ = "Billboard"
Qgis.Point3DShape.__doc__ = "3D point shape types.\n\n.. note::\n\n   Prior to QGIS 3.36 this was available as :py:class:`QgsPoint3DSymbol`.Shape\n\n.. versionadded:: 3.36\n\n" + '* ``Cylinder``: ' + Qgis.Point3DShape.Cylinder.__doc__ + '\n' + '* ``Sphere``: ' + Qgis.Point3DShape.Sphere.__doc__ + '\n' + '* ``Cone``: ' + Qgis.Point3DShape.Cone.__doc__ + '\n' + '* ``Cube``: ' + Qgis.Point3DShape.Cube.__doc__ + '\n' + '* ``Torus``: ' + Qgis.Point3DShape.Torus.__doc__ + '\n' + '* ``Plane``: ' + Qgis.Point3DShape.Plane.__doc__ + '\n' + '* ``ExtrudedText``: ' + Qgis.Point3DShape.ExtrudedText.__doc__ + '\n' + '* ``Model``: ' + Qgis.Point3DShape.Model.__doc__ + '\n' + '* ``Billboard``: ' + Qgis.Point3DShape.Billboard.__doc__
# --
Qgis.Point3DShape.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LightSourceType.Point.__doc__ = "Point light source"
Qgis.LightSourceType.Directional.__doc__ = "Directional light source"
Qgis.LightSourceType.__doc__ = "Light source types for 3D scenes.\n\n.. versionadded:: 3.26\n\n" + '* ``Point``: ' + Qgis.LightSourceType.Point.__doc__ + '\n' + '* ``Directional``: ' + Qgis.LightSourceType.Directional.__doc__
# --
Qgis.LightSourceType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.NavigationMode.TerrainBased.__doc__ = "The default navigation based on the terrain"
Qgis.NavigationMode.Walk.__doc__ = "Uses WASD keys or arrows to navigate in walking (first person) manner"
Qgis.NavigationMode.__doc__ = "The navigation mode used by 3D cameras.\n\n.. versionadded:: 3.30\n\n" + '* ``TerrainBased``: ' + Qgis.NavigationMode.TerrainBased.__doc__ + '\n' + '* ``Walk``: ' + Qgis.NavigationMode.Walk.__doc__
# --
Qgis.NavigationMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VerticalAxisInversion.Never.__doc__ = "Never invert vertical axis movements"
Qgis.VerticalAxisInversion.WhenDragging.__doc__ = "Invert vertical axis movements when dragging in first person modes"
Qgis.VerticalAxisInversion.Always.__doc__ = "Always invert vertical axis movements"
Qgis.VerticalAxisInversion.__doc__ = "Vertical axis inversion options for 3D views.\n\n.. versionadded:: 3.30\n\n" + '* ``Never``: ' + Qgis.VerticalAxisInversion.Never.__doc__ + '\n' + '* ``WhenDragging``: ' + Qgis.VerticalAxisInversion.WhenDragging.__doc__ + '\n' + '* ``Always``: ' + Qgis.VerticalAxisInversion.Always.__doc__
# --
Qgis.VerticalAxisInversion.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProfileSurfaceSymbology.Line.__doc__ = "The elevation surface will be rendered using a line symbol"
Qgis.ProfileSurfaceSymbology.FillBelow.__doc__ = "The elevation surface will be rendered using a fill symbol below the surface level"
Qgis.ProfileSurfaceSymbology.FillAbove.__doc__ = "The elevation surface will be rendered using a fill symbol above the surface level (since QGIS 3.32)"
Qgis.ProfileSurfaceSymbology.__doc__ = "Surface symbology type for elevation profile plots.\n\n.. versionadded:: 3.26\n\n" + '* ``Line``: ' + Qgis.ProfileSurfaceSymbology.Line.__doc__ + '\n' + '* ``FillBelow``: ' + Qgis.ProfileSurfaceSymbology.FillBelow.__doc__ + '\n' + '* ``FillAbove``: ' + Qgis.ProfileSurfaceSymbology.FillAbove.__doc__
# --
Qgis.ProfileSurfaceSymbology.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorProfileType.IndividualFeatures.__doc__ = "Treat each feature as an individual object (eg buildings)"
Qgis.VectorProfileType.ContinuousSurface.__doc__ = "The features should be treated as representing values on a continuous surface (eg contour lines)"
Qgis.VectorProfileType.__doc__ = "Types of elevation profiles to generate for vector sources.\n\n.. versionadded:: 3.26\n\n" + '* ``IndividualFeatures``: ' + Qgis.VectorProfileType.IndividualFeatures.__doc__ + '\n' + '* ``ContinuousSurface``: ' + Qgis.VectorProfileType.ContinuousSurface.__doc__
# --
Qgis.VectorProfileType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProfileGeneratorFlag.RespectsMaximumErrorMapUnit.__doc__ = "Generated profile respects the QgsProfileGenerationContext.maximumErrorMapUnits() property."
Qgis.ProfileGeneratorFlag.RespectsDistanceRange.__doc__ = "Generated profile respects the QgsProfileGenerationContext.distanceRange() property."
Qgis.ProfileGeneratorFlag.RespectsElevationRange.__doc__ = "Generated profile respects the QgsProfileGenerationContext.elevationRange() property."
Qgis.ProfileGeneratorFlag.__doc__ = "Flags that control the way the :py:class:`QgsAbstractProfileGenerator` operate.\n\n.. versionadded:: 3.26\n\n" + '* ``RespectsMaximumErrorMapUnit``: ' + Qgis.ProfileGeneratorFlag.RespectsMaximumErrorMapUnit.__doc__ + '\n' + '* ``RespectsDistanceRange``: ' + Qgis.ProfileGeneratorFlag.RespectsDistanceRange.__doc__ + '\n' + '* ``RespectsElevationRange``: ' + Qgis.ProfileGeneratorFlag.RespectsElevationRange.__doc__
# --
Qgis.ProfileGeneratorFlag.baseClass = Qgis
Qgis.ProfileGeneratorFlags = lambda flags=0: Qgis.ProfileGeneratorFlag(flags)
Qgis.ProfileGeneratorFlags.baseClass = Qgis
ProfileGeneratorFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ProfileExportType.Features3D.__doc__ = "Export profiles as 3D features, with elevation values stored in exported geometry Z values"
Qgis.ProfileExportType.Profile2D.__doc__ = "Export profiles as 2D profile lines, with elevation stored in exported geometry Y dimension and distance in X dimension"
Qgis.ProfileExportType.DistanceVsElevationTable.__doc__ = "Export profiles as a table of sampled distance vs elevation values"
Qgis.ProfileExportType.__doc__ = "Types of export for elevation profiles.\n\n.. versionadded:: 3.32\n\n" + '* ``Features3D``: ' + Qgis.ProfileExportType.Features3D.__doc__ + '\n' + '* ``Profile2D``: ' + Qgis.ProfileExportType.Profile2D.__doc__ + '\n' + '* ``DistanceVsElevationTable``: ' + Qgis.ProfileExportType.DistanceVsElevationTable.__doc__
# --
Qgis.ProfileExportType.baseClass = Qgis
QgsPointCloudRenderer.PointSymbol = Qgis.PointCloudSymbol
# monkey patching scoped based enum
QgsPointCloudRenderer.Square = Qgis.PointCloudSymbol.Square
QgsPointCloudRenderer.Square.is_monkey_patched = True
QgsPointCloudRenderer.Square.__doc__ = "Renders points as squares"
QgsPointCloudRenderer.Circle = Qgis.PointCloudSymbol.Circle
QgsPointCloudRenderer.Circle.is_monkey_patched = True
QgsPointCloudRenderer.Circle.__doc__ = "Renders points as circles"
Qgis.PointCloudSymbol.__doc__ = "Rendering symbols for point cloud points.\n\n.. versionadded:: 3.26\n\n" + '* ``Square``: ' + Qgis.PointCloudSymbol.Square.__doc__ + '\n' + '* ``Circle``: ' + Qgis.PointCloudSymbol.Circle.__doc__
# --
Qgis.PointCloudSymbol.baseClass = Qgis
QgsPointCloudRenderer.DrawOrder = Qgis.PointCloudDrawOrder
# monkey patching scoped based enum
QgsPointCloudRenderer.Default = Qgis.PointCloudDrawOrder.Default
QgsPointCloudRenderer.Default.is_monkey_patched = True
QgsPointCloudRenderer.Default.__doc__ = "Draw points in the order they are stored"
QgsPointCloudRenderer.BottomToTop = Qgis.PointCloudDrawOrder.BottomToTop
QgsPointCloudRenderer.BottomToTop.is_monkey_patched = True
QgsPointCloudRenderer.BottomToTop.__doc__ = "Draw points with larger Z values last"
QgsPointCloudRenderer.TopToBottom = Qgis.PointCloudDrawOrder.TopToBottom
QgsPointCloudRenderer.TopToBottom.is_monkey_patched = True
QgsPointCloudRenderer.TopToBottom.__doc__ = "Draw points with larger Z values first"
Qgis.PointCloudDrawOrder.__doc__ = "Pointcloud rendering order for 2d views\n\n/since QGIS 3.26\n\n" + '* ``Default``: ' + Qgis.PointCloudDrawOrder.Default.__doc__ + '\n' + '* ``BottomToTop``: ' + Qgis.PointCloudDrawOrder.BottomToTop.__doc__ + '\n' + '* ``TopToBottom``: ' + Qgis.PointCloudDrawOrder.TopToBottom.__doc__
# --
Qgis.PointCloudDrawOrder.baseClass = Qgis
QgsProject.AvoidIntersectionsMode = Qgis.AvoidIntersectionsMode
# monkey patching scoped based enum
QgsProject.AllowIntersections = Qgis.AvoidIntersectionsMode.AllowIntersections
QgsProject.AllowIntersections.is_monkey_patched = True
QgsProject.AllowIntersections.__doc__ = "Overlap with any feature allowed when digitizing new features"
QgsProject.AvoidIntersectionsCurrentLayer = Qgis.AvoidIntersectionsMode.AvoidIntersectionsCurrentLayer
QgsProject.AvoidIntersectionsCurrentLayer.is_monkey_patched = True
QgsProject.AvoidIntersectionsCurrentLayer.__doc__ = "Overlap with features from the active layer when digitizing new features not allowed"
QgsProject.AvoidIntersectionsLayers = Qgis.AvoidIntersectionsMode.AvoidIntersectionsLayers
QgsProject.AvoidIntersectionsLayers.is_monkey_patched = True
QgsProject.AvoidIntersectionsLayers.__doc__ = "Overlap with features from a specified list of layers when digitizing new features not allowed"
Qgis.AvoidIntersectionsMode.__doc__ = "Flags which control how intersections of pre-existing feature are handled when digitizing new features.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsProject`.AvoidIntersectionsMode\n\n.. versionadded:: 3.26\n\n" + '* ``AllowIntersections``: ' + Qgis.AvoidIntersectionsMode.AllowIntersections.__doc__ + '\n' + '* ``AvoidIntersectionsCurrentLayer``: ' + Qgis.AvoidIntersectionsMode.AvoidIntersectionsCurrentLayer.__doc__ + '\n' + '* ``AvoidIntersectionsLayers``: ' + Qgis.AvoidIntersectionsMode.AvoidIntersectionsLayers.__doc__
# --
Qgis.AvoidIntersectionsMode.baseClass = Qgis
QgsProject.FileFormat = Qgis.ProjectFileFormat
# monkey patching scoped based enum
QgsProject.Qgz = Qgis.ProjectFileFormat.Qgz
QgsProject.Qgz.is_monkey_patched = True
QgsProject.Qgz.__doc__ = "Archive file format, supports auxiliary data"
QgsProject.Qgs = Qgis.ProjectFileFormat.Qgs
QgsProject.Qgs.is_monkey_patched = True
QgsProject.Qgs.__doc__ = "Project saved in a clear text, does not support auxiliary data"
Qgis.ProjectFileFormat.__doc__ = "Flags which control project read behavior.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsProject`.FileFormat\n\n.. versionadded:: 3.26\n\n" + '* ``Qgz``: ' + Qgis.ProjectFileFormat.Qgz.__doc__ + '\n' + '* ``Qgs``: ' + Qgis.ProjectFileFormat.Qgs.__doc__
# --
Qgis.ProjectFileFormat.baseClass = Qgis
QgsProject.ReadFlag = Qgis.ProjectReadFlag
# monkey patching scoped based enum
QgsProject.FlagDontResolveLayers = Qgis.ProjectReadFlag.DontResolveLayers
QgsProject.ReadFlag.FlagDontResolveLayers = Qgis.ProjectReadFlag.DontResolveLayers
QgsProject.FlagDontResolveLayers.is_monkey_patched = True
QgsProject.FlagDontResolveLayers.__doc__ = "Don't resolve layer paths (i.e. don't load any layer content). Dramatically improves project read time if the actual data from the layers is not required."
QgsProject.FlagDontLoadLayouts = Qgis.ProjectReadFlag.DontLoadLayouts
QgsProject.ReadFlag.FlagDontLoadLayouts = Qgis.ProjectReadFlag.DontLoadLayouts
QgsProject.FlagDontLoadLayouts.is_monkey_patched = True
QgsProject.FlagDontLoadLayouts.__doc__ = "Don't load print layouts. Improves project read time if layouts are not required, and allows projects to be safely read in background threads (since print layouts are not thread safe)."
QgsProject.FlagTrustLayerMetadata = Qgis.ProjectReadFlag.TrustLayerMetadata
QgsProject.ReadFlag.FlagTrustLayerMetadata = Qgis.ProjectReadFlag.TrustLayerMetadata
QgsProject.FlagTrustLayerMetadata.is_monkey_patched = True
QgsProject.FlagTrustLayerMetadata.__doc__ = "Trust layer metadata. Improves project read time. Do not use it if layers' extent is not fixed during the project's use by QGIS and QGIS Server."
QgsProject.FlagDontStoreOriginalStyles = Qgis.ProjectReadFlag.DontStoreOriginalStyles
QgsProject.ReadFlag.FlagDontStoreOriginalStyles = Qgis.ProjectReadFlag.DontStoreOriginalStyles
QgsProject.FlagDontStoreOriginalStyles.is_monkey_patched = True
QgsProject.FlagDontStoreOriginalStyles.__doc__ = "Skip the initial XML style storage for layers. Useful for minimising project load times in non-interactive contexts."
QgsProject.FlagDontLoad3DViews = Qgis.ProjectReadFlag.DontLoad3DViews
QgsProject.ReadFlag.FlagDontLoad3DViews = Qgis.ProjectReadFlag.DontLoad3DViews
QgsProject.FlagDontLoad3DViews.is_monkey_patched = True
QgsProject.FlagDontLoad3DViews.__doc__ = "Skip loading 3D views (since QGIS 3.26)"
QgsProject.DontLoadProjectStyles = Qgis.ProjectReadFlag.DontLoadProjectStyles
QgsProject.DontLoadProjectStyles.is_monkey_patched = True
QgsProject.DontLoadProjectStyles.__doc__ = "Skip loading project style databases (deprecated -- use ProjectCapability.ProjectStyles flag instead)"
QgsProject.ForceReadOnlyLayers = Qgis.ProjectReadFlag.ForceReadOnlyLayers
QgsProject.ForceReadOnlyLayers.is_monkey_patched = True
QgsProject.ForceReadOnlyLayers.__doc__ = "Open layers in a read-only mode. (since QGIS 3.28)"
Qgis.ProjectReadFlag.__doc__ = "Flags which control project read behavior.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsProject`.ReadFlag\n\n.. versionadded:: 3.26\n\n" + '* ``FlagDontResolveLayers``: ' + Qgis.ProjectReadFlag.DontResolveLayers.__doc__ + '\n' + '* ``FlagDontLoadLayouts``: ' + Qgis.ProjectReadFlag.DontLoadLayouts.__doc__ + '\n' + '* ``FlagTrustLayerMetadata``: ' + Qgis.ProjectReadFlag.TrustLayerMetadata.__doc__ + '\n' + '* ``FlagDontStoreOriginalStyles``: ' + Qgis.ProjectReadFlag.DontStoreOriginalStyles.__doc__ + '\n' + '* ``FlagDontLoad3DViews``: ' + Qgis.ProjectReadFlag.DontLoad3DViews.__doc__ + '\n' + '* ``DontLoadProjectStyles``: ' + Qgis.ProjectReadFlag.DontLoadProjectStyles.__doc__ + '\n' + '* ``ForceReadOnlyLayers``: ' + Qgis.ProjectReadFlag.ForceReadOnlyLayers.__doc__
# --
Qgis.ProjectReadFlag.baseClass = Qgis
Qgis.ProjectReadFlags = lambda flags=0: Qgis.ProjectReadFlag(flags)
QgsProject.ReadFlags = Qgis.ProjectReadFlags
Qgis.ProjectReadFlags.baseClass = Qgis
ProjectReadFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ProjectCapability.ProjectStyles.__doc__ = "Enable the project embedded style library. Enabling this flag can increase the time required to clear and load projects."
Qgis.ProjectCapability.__doc__ = "Flags which control project capabilities.\n\nThese flags are specific upfront on creation of a :py:class:`QgsProject` object, and can\nbe used to selectively enable potentially costly functionality for the project.\n\n.. versionadded:: 3.26.1\n\n" + '* ``ProjectStyles``: ' + Qgis.ProjectCapability.ProjectStyles.__doc__
# --
Qgis.ProjectCapability.baseClass = Qgis
Qgis.ProjectCapabilities = lambda flags=0: Qgis.ProjectCapability(flags)
Qgis.ProjectCapabilities.baseClass = Qgis
ProjectCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.MapBoxGlStyleSourceType.Vector.__doc__ = "Vector source"
Qgis.MapBoxGlStyleSourceType.Raster.__doc__ = "Raster source"
Qgis.MapBoxGlStyleSourceType.RasterDem.__doc__ = "Raster DEM source"
Qgis.MapBoxGlStyleSourceType.GeoJson.__doc__ = "GeoJSON source"
Qgis.MapBoxGlStyleSourceType.Image.__doc__ = "Image source"
Qgis.MapBoxGlStyleSourceType.Video.__doc__ = "Video source"
Qgis.MapBoxGlStyleSourceType.Unknown.__doc__ = "Other/unknown source type"
Qgis.MapBoxGlStyleSourceType.__doc__ = "Available MapBox GL style source types.\n\n.. versionadded:: 3.28\n\n" + '* ``Vector``: ' + Qgis.MapBoxGlStyleSourceType.Vector.__doc__ + '\n' + '* ``Raster``: ' + Qgis.MapBoxGlStyleSourceType.Raster.__doc__ + '\n' + '* ``RasterDem``: ' + Qgis.MapBoxGlStyleSourceType.RasterDem.__doc__ + '\n' + '* ``GeoJson``: ' + Qgis.MapBoxGlStyleSourceType.GeoJson.__doc__ + '\n' + '* ``Image``: ' + Qgis.MapBoxGlStyleSourceType.Image.__doc__ + '\n' + '* ``Video``: ' + Qgis.MapBoxGlStyleSourceType.Video.__doc__ + '\n' + '* ``Unknown``: ' + Qgis.MapBoxGlStyleSourceType.Unknown.__doc__
# --
Qgis.MapBoxGlStyleSourceType.baseClass = Qgis
QgsArcGisPortalUtils.ItemType = Qgis.ArcGisRestServiceType
# monkey patching scoped based enum
QgsArcGisPortalUtils.FeatureService = Qgis.ArcGisRestServiceType.FeatureServer
QgsArcGisPortalUtils.ItemType.FeatureService = Qgis.ArcGisRestServiceType.FeatureServer
QgsArcGisPortalUtils.FeatureService.is_monkey_patched = True
QgsArcGisPortalUtils.FeatureService.__doc__ = "FeatureServer"
QgsArcGisPortalUtils.MapService = Qgis.ArcGisRestServiceType.MapServer
QgsArcGisPortalUtils.ItemType.MapService = Qgis.ArcGisRestServiceType.MapServer
QgsArcGisPortalUtils.MapService.is_monkey_patched = True
QgsArcGisPortalUtils.MapService.__doc__ = "MapServer"
QgsArcGisPortalUtils.ImageService = Qgis.ArcGisRestServiceType.ImageServer
QgsArcGisPortalUtils.ItemType.ImageService = Qgis.ArcGisRestServiceType.ImageServer
QgsArcGisPortalUtils.ImageService.is_monkey_patched = True
QgsArcGisPortalUtils.ImageService.__doc__ = "ImageServer"
QgsArcGisPortalUtils.GlobeServer = Qgis.ArcGisRestServiceType.GlobeServer
QgsArcGisPortalUtils.GlobeServer.is_monkey_patched = True
QgsArcGisPortalUtils.GlobeServer.__doc__ = "GlobeServer"
QgsArcGisPortalUtils.GPServer = Qgis.ArcGisRestServiceType.GPServer
QgsArcGisPortalUtils.GPServer.is_monkey_patched = True
QgsArcGisPortalUtils.GPServer.__doc__ = "GPServer"
QgsArcGisPortalUtils.GeocodeServer = Qgis.ArcGisRestServiceType.GeocodeServer
QgsArcGisPortalUtils.GeocodeServer.is_monkey_patched = True
QgsArcGisPortalUtils.GeocodeServer.__doc__ = "GeocodeServer"
QgsArcGisPortalUtils.Unknown = Qgis.ArcGisRestServiceType.Unknown
QgsArcGisPortalUtils.Unknown.is_monkey_patched = True
QgsArcGisPortalUtils.Unknown.__doc__ = "Other unknown/unsupported type"
Qgis.ArcGisRestServiceType.__doc__ = "Available ArcGIS REST service types.\n\n.. note::\n\n   Prior to QGIS 3.26 this was available as :py:class:`QgsArcGisPortalUtils`.ItemType.\n\n.. versionadded:: 3.28\n\n" + '* ``FeatureService``: ' + Qgis.ArcGisRestServiceType.FeatureServer.__doc__ + '\n' + '* ``MapService``: ' + Qgis.ArcGisRestServiceType.MapServer.__doc__ + '\n' + '* ``ImageService``: ' + Qgis.ArcGisRestServiceType.ImageServer.__doc__ + '\n' + '* ``GlobeServer``: ' + Qgis.ArcGisRestServiceType.GlobeServer.__doc__ + '\n' + '* ``GPServer``: ' + Qgis.ArcGisRestServiceType.GPServer.__doc__ + '\n' + '* ``GeocodeServer``: ' + Qgis.ArcGisRestServiceType.GeocodeServer.__doc__ + '\n' + '* ``Unknown``: ' + Qgis.ArcGisRestServiceType.Unknown.__doc__
# --
Qgis.ArcGisRestServiceType.baseClass = Qgis
QgsRelation.RelationType = Qgis.RelationshipType
# monkey patching scoped based enum
QgsRelation.Normal = Qgis.RelationshipType.Normal
QgsRelation.Normal.is_monkey_patched = True
QgsRelation.Normal.__doc__ = "A normal relation"
QgsRelation.Generated = Qgis.RelationshipType.Generated
QgsRelation.Generated.is_monkey_patched = True
QgsRelation.Generated.__doc__ = "A generated relation is a child of a polymorphic relation"
Qgis.RelationshipType.__doc__ = "Relationship types.\n\n.. note::\n\n   Prior to QGIS 3.28 this was available as :py:class:`QgsRelation`.RelationType.\n\n.. versionadded:: 3.28\n\n" + '* ``Normal``: ' + Qgis.RelationshipType.Normal.__doc__ + '\n' + '* ``Generated``: ' + Qgis.RelationshipType.Generated.__doc__
# --
Qgis.RelationshipType.baseClass = Qgis
QgsRelation.RelationStrength = Qgis.RelationshipStrength
# monkey patching scoped based enum
QgsRelation.Association = Qgis.RelationshipStrength.Association
QgsRelation.Association.is_monkey_patched = True
QgsRelation.Association.__doc__ = "Loose relation, related elements are not part of the parent and a parent copy will not copy any children."
QgsRelation.Composition = Qgis.RelationshipStrength.Composition
QgsRelation.Composition.is_monkey_patched = True
QgsRelation.Composition.__doc__ = "Fix relation, related elements are part of the parent and a parent copy will copy any children or delete of parent will delete children"
Qgis.RelationshipStrength.__doc__ = "Relationship strength.\n\n.. note::\n\n   Prior to QGIS 3.28 this was available as :py:class:`QgsRelation`.RelationStrength.\n\n.. versionadded:: 3.28\n\n" + '* ``Association``: ' + Qgis.RelationshipStrength.Association.__doc__ + '\n' + '* ``Composition``: ' + Qgis.RelationshipStrength.Composition.__doc__
# --
Qgis.RelationshipStrength.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RelationshipCardinality.OneToOne.__doc__ = "One to one relationship"
Qgis.RelationshipCardinality.OneToMany.__doc__ = "One to many relationship"
Qgis.RelationshipCardinality.ManyToOne.__doc__ = "Many to one relationship"
Qgis.RelationshipCardinality.ManyToMany.__doc__ = "Many to many relationship"
Qgis.RelationshipCardinality.__doc__ = "Relationship cardinality.\n\n.. versionadded:: 3.28\n\n" + '* ``OneToOne``: ' + Qgis.RelationshipCardinality.OneToOne.__doc__ + '\n' + '* ``OneToMany``: ' + Qgis.RelationshipCardinality.OneToMany.__doc__ + '\n' + '* ``ManyToOne``: ' + Qgis.RelationshipCardinality.ManyToOne.__doc__ + '\n' + '* ``ManyToMany``: ' + Qgis.RelationshipCardinality.ManyToMany.__doc__
# --
Qgis.RelationshipCardinality.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RelationshipCapability.MultipleFieldKeys.__doc__ = "Supports multiple field keys (as opposed to a singular field)"
Qgis.RelationshipCapability.ForwardPathLabel.__doc__ = "Supports forward path labels"
Qgis.RelationshipCapability.BackwardPathLabel.__doc__ = "Supports backward path labels"
Qgis.RelationshipCapability.__doc__ = "Relationship capabilities.\n\n.. versionadded:: 3.30\n\n" + '* ``MultipleFieldKeys``: ' + Qgis.RelationshipCapability.MultipleFieldKeys.__doc__ + '\n' + '* ``ForwardPathLabel``: ' + Qgis.RelationshipCapability.ForwardPathLabel.__doc__ + '\n' + '* ``BackwardPathLabel``: ' + Qgis.RelationshipCapability.BackwardPathLabel.__doc__
# --
Qgis.RelationshipCapability.baseClass = Qgis
Qgis.RelationshipCapabilities = lambda flags=0: Qgis.RelationshipCapability(flags)
Qgis.RelationshipCapabilities.baseClass = Qgis
RelationshipCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.CoordinateDisplayType.MapCrs.__doc__ = "Map CRS"
Qgis.CoordinateDisplayType.MapGeographic.__doc__ = "Map Geographic CRS equivalent (stays unchanged if the map CRS is geographic)"
Qgis.CoordinateDisplayType.CustomCrs.__doc__ = "Custom CRS"
Qgis.CoordinateDisplayType.__doc__ = "Formats for displaying coordinates\n\n.. versionadded:: 3.28\n\n" + '* ``MapCrs``: ' + Qgis.CoordinateDisplayType.MapCrs.__doc__ + '\n' + '* ``MapGeographic``: ' + Qgis.CoordinateDisplayType.MapGeographic.__doc__ + '\n' + '* ``CustomCrs``: ' + Qgis.CoordinateDisplayType.CustomCrs.__doc__
# --
Qgis.CoordinateDisplayType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SettingsOrigin.Any.__doc__ = "From any origin"
Qgis.SettingsOrigin.Global.__doc__ = "Global settings are stored in `qgis_global_settings.ini`"
Qgis.SettingsOrigin.Local.__doc__ = "Local settings are stored in the user profile"
Qgis.SettingsOrigin.__doc__ = "The setting origin describes where a setting is stored.\n\n.. versionadded:: 3.30\n\n" + '* ``Any``: ' + Qgis.SettingsOrigin.Any.__doc__ + '\n' + '* ``Global``: ' + Qgis.SettingsOrigin.Global.__doc__ + '\n' + '* ``Local``: ' + Qgis.SettingsOrigin.Local.__doc__
# --
Qgis.SettingsOrigin.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ScriptLanguage.Css.__doc__ = "CSS"
Qgis.ScriptLanguage.QgisExpression.__doc__ = "QGIS expressions"
Qgis.ScriptLanguage.Html.__doc__ = "HTML"
Qgis.ScriptLanguage.JavaScript.__doc__ = "JavaScript"
Qgis.ScriptLanguage.Json.__doc__ = "JSON"
Qgis.ScriptLanguage.Python.__doc__ = "Python"
Qgis.ScriptLanguage.R.__doc__ = "R Stats"
Qgis.ScriptLanguage.Sql.__doc__ = "SQL"
Qgis.ScriptLanguage.Batch.__doc__ = "Windows batch files"
Qgis.ScriptLanguage.Bash.__doc__ = "Bash scripts"
Qgis.ScriptLanguage.Unknown.__doc__ = "Unknown/other language"
Qgis.ScriptLanguage.__doc__ = "Scripting languages.\n\n.. versionadded:: 3.30\n\n" + '* ``Css``: ' + Qgis.ScriptLanguage.Css.__doc__ + '\n' + '* ``QgisExpression``: ' + Qgis.ScriptLanguage.QgisExpression.__doc__ + '\n' + '* ``Html``: ' + Qgis.ScriptLanguage.Html.__doc__ + '\n' + '* ``JavaScript``: ' + Qgis.ScriptLanguage.JavaScript.__doc__ + '\n' + '* ``Json``: ' + Qgis.ScriptLanguage.Json.__doc__ + '\n' + '* ``Python``: ' + Qgis.ScriptLanguage.Python.__doc__ + '\n' + '* ``R``: ' + Qgis.ScriptLanguage.R.__doc__ + '\n' + '* ``Sql``: ' + Qgis.ScriptLanguage.Sql.__doc__ + '\n' + '* ``Batch``: ' + Qgis.ScriptLanguage.Batch.__doc__ + '\n' + '* ``Bash``: ' + Qgis.ScriptLanguage.Bash.__doc__ + '\n' + '* ``Unknown``: ' + Qgis.ScriptLanguage.Unknown.__doc__
# --
Qgis.ScriptLanguage.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ScriptLanguageCapability.Reformat.__doc__ = "Language supports automatic code reformatting"
Qgis.ScriptLanguageCapability.CheckSyntax.__doc__ = "Language supports syntax checking"
Qgis.ScriptLanguageCapability.ToggleComment.__doc__ = "Language supports comment toggling"
Qgis.ScriptLanguageCapability.__doc__ = "Script language capabilities.\n\nThe flags reflect the support capabilities of a scripting language.\n\n.. versionadded:: 3.32\n\n" + '* ``Reformat``: ' + Qgis.ScriptLanguageCapability.Reformat.__doc__ + '\n' + '* ``CheckSyntax``: ' + Qgis.ScriptLanguageCapability.CheckSyntax.__doc__ + '\n' + '* ``ToggleComment``: ' + Qgis.ScriptLanguageCapability.ToggleComment.__doc__
# --
Qgis.ScriptLanguageCapability.baseClass = Qgis
Qgis.ScriptLanguageCapabilities = lambda flags=0: Qgis.ScriptLanguageCapability(flags)
Qgis.ScriptLanguageCapabilities.baseClass = Qgis
ScriptLanguageCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.LayerTreeInsertionMethod.AboveInsertionPoint.__doc__ = "Layers are added in the tree above the insertion point"
Qgis.LayerTreeInsertionMethod.TopOfTree.__doc__ = "Layers are added at the top of the layer tree"
Qgis.LayerTreeInsertionMethod.OptimalInInsertionGroup.__doc__ = "Layers are added at optimal locations across the insertion point's group"
Qgis.LayerTreeInsertionMethod.__doc__ = "Layer tree insertion methods\n\n.. versionadded:: 3.30\n\n" + '* ``AboveInsertionPoint``: ' + Qgis.LayerTreeInsertionMethod.AboveInsertionPoint.__doc__ + '\n' + '* ``TopOfTree``: ' + Qgis.LayerTreeInsertionMethod.TopOfTree.__doc__ + '\n' + '* ``OptimalInInsertionGroup``: ' + Qgis.LayerTreeInsertionMethod.OptimalInInsertionGroup.__doc__
# --
Qgis.LayerTreeInsertionMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LayerTreeFilterFlag.SkipVisibilityCheck.__doc__ = "If set, the standard visibility check should be skipped"
Qgis.LayerTreeFilterFlag.__doc__ = "Layer tree filter flags.\n\n.. versionadded:: 3.32\n\n" + '* ``SkipVisibilityCheck``: ' + Qgis.LayerTreeFilterFlag.SkipVisibilityCheck.__doc__
# --
Qgis.LayerTreeFilterFlag.baseClass = Qgis
Qgis.LayerTreeFilterFlags = lambda flags=0: Qgis.LayerTreeFilterFlag(flags)
Qgis.LayerTreeFilterFlags.baseClass = Qgis
LayerTreeFilterFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.LegendJsonRenderFlag.ShowRuleDetails.__doc__ = "If set, the rule expression of a rule based renderer legend item will be added to the JSON"
Qgis.LegendJsonRenderFlag.__doc__ = "Legend JSON export flags.\n\nFlags to control JSON attributes when exporting a legend in JSON format.\n\n.. versionadded:: 3.36\n\n" + '* ``ShowRuleDetails``: ' + Qgis.LegendJsonRenderFlag.ShowRuleDetails.__doc__
# --
Qgis.LegendJsonRenderFlag.baseClass = Qgis
Qgis.LegendJsonRenderFlags = lambda flags=0: Qgis.LegendJsonRenderFlag(flags)
Qgis.LegendJsonRenderFlags.baseClass = Qgis
LegendJsonRenderFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ActionType.Invalid.__doc__ = "Invalid"
Qgis.ActionType.MapLayerAction.__doc__ = "Standard actions (defined by core or plugins), corresponds to QgsMapLayerAction class."
Qgis.ActionType.AttributeAction.__doc__ = "Custom actions (manually defined in layer properties), corresponds to QgsAction class."
Qgis.ActionType.__doc__ = "Action types.\n\nPrior to QGIS 3.30 this was available as :py:class:`QgsActionMenu`.ActionType\n\n.. versionadded:: 3.30\n\n" + '* ``Invalid``: ' + Qgis.ActionType.Invalid.__doc__ + '\n' + '* ``MapLayerAction``: ' + Qgis.ActionType.MapLayerAction.__doc__ + '\n' + '* ``AttributeAction``: ' + Qgis.ActionType.AttributeAction.__doc__
# --
Qgis.ActionType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MapLayerActionTarget.Layer.__doc__ = "Action targets a complete layer"
Qgis.MapLayerActionTarget.SingleFeature.__doc__ = "Action targets a single feature from a layer"
Qgis.MapLayerActionTarget.MultipleFeatures.__doc__ = "Action targets multiple features from a layer"
Qgis.MapLayerActionTarget.AllActions.__doc__ = ""
Qgis.MapLayerActionTarget.__doc__ = "Map layer action targets.\n\nPrior to QGIS 3.30 this was available as :py:class:`QgsMapLayerAction`.Target\n\n.. versionadded:: 3.30\n\n" + '* ``Layer``: ' + Qgis.MapLayerActionTarget.Layer.__doc__ + '\n' + '* ``SingleFeature``: ' + Qgis.MapLayerActionTarget.SingleFeature.__doc__ + '\n' + '* ``MultipleFeatures``: ' + Qgis.MapLayerActionTarget.MultipleFeatures.__doc__ + '\n' + '* ``AllActions``: ' + Qgis.MapLayerActionTarget.AllActions.__doc__
# --
Qgis.MapLayerActionTarget.baseClass = Qgis
Qgis.MapLayerActionTargets = lambda flags=0: Qgis.MapLayerActionTarget(flags)
Qgis.MapLayerActionTargets.baseClass = Qgis
MapLayerActionTargets = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.MapLayerActionFlag.EnabledOnlyWhenEditable.__doc__ = "Action should be shown only for editable layers"
Qgis.MapLayerActionFlag.__doc__ = "Map layer action flags.\n\nPrior to QGIS 3.30 this was available as :py:class:`QgsMapLayerAction`.Flag\n\n.. versionadded:: 3.30\n\n" + '* ``EnabledOnlyWhenEditable``: ' + Qgis.MapLayerActionFlag.EnabledOnlyWhenEditable.__doc__
# --
Qgis.MapLayerActionFlag.baseClass = Qgis
Qgis.MapLayerActionFlags = lambda flags=0: Qgis.MapLayerActionFlag(flags)
Qgis.MapLayerActionFlags.baseClass = Qgis
MapLayerActionFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsAction.ActionType = Qgis.AttributeActionType
# monkey patching scoped based enum
QgsAction.Generic = Qgis.AttributeActionType.Generic
QgsAction.Generic.is_monkey_patched = True
QgsAction.Generic.__doc__ = "Generic"
QgsAction.GenericPython = Qgis.AttributeActionType.GenericPython
QgsAction.GenericPython.is_monkey_patched = True
QgsAction.GenericPython.__doc__ = "Python"
QgsAction.Mac = Qgis.AttributeActionType.Mac
QgsAction.Mac.is_monkey_patched = True
QgsAction.Mac.__doc__ = "MacOS specific"
QgsAction.Windows = Qgis.AttributeActionType.Windows
QgsAction.Windows.is_monkey_patched = True
QgsAction.Windows.__doc__ = "Windows specific"
QgsAction.Unix = Qgis.AttributeActionType.Unix
QgsAction.Unix.is_monkey_patched = True
QgsAction.Unix.__doc__ = "Unix specific"
QgsAction.OpenUrl = Qgis.AttributeActionType.OpenUrl
QgsAction.OpenUrl.is_monkey_patched = True
QgsAction.OpenUrl.__doc__ = "Open URL action"
QgsAction.SubmitUrlEncoded = Qgis.AttributeActionType.SubmitUrlEncoded
QgsAction.SubmitUrlEncoded.is_monkey_patched = True
QgsAction.SubmitUrlEncoded.__doc__ = "POST data to an URL, using \"application/x-www-form-urlencoded\" or \"application/json\" if the body is valid JSON \n.. versionadded:: 3.24"
QgsAction.SubmitUrlMultipart = Qgis.AttributeActionType.SubmitUrlMultipart
QgsAction.SubmitUrlMultipart.is_monkey_patched = True
QgsAction.SubmitUrlMultipart.__doc__ = "POST data to an URL using \"multipart/form-data\"  \n.. versionadded:: 3.24"
Qgis.AttributeActionType.__doc__ = "Attribute action types.\n\nPrior to QGIS 3.30 this was available as :py:class:`QgsAction`.ActionType\n\n.. versionadded:: 3.30\n\n" + '* ``Generic``: ' + Qgis.AttributeActionType.Generic.__doc__ + '\n' + '* ``GenericPython``: ' + Qgis.AttributeActionType.GenericPython.__doc__ + '\n' + '* ``Mac``: ' + Qgis.AttributeActionType.Mac.__doc__ + '\n' + '* ``Windows``: ' + Qgis.AttributeActionType.Windows.__doc__ + '\n' + '* ``Unix``: ' + Qgis.AttributeActionType.Unix.__doc__ + '\n' + '* ``OpenUrl``: ' + Qgis.AttributeActionType.OpenUrl.__doc__ + '\n' + '* ``SubmitUrlEncoded``: ' + Qgis.AttributeActionType.SubmitUrlEncoded.__doc__ + '\n' + '* ``SubmitUrlMultipart``: ' + Qgis.AttributeActionType.SubmitUrlMultipart.__doc__
# --
Qgis.AttributeActionType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MetadataDateType.Created.__doc__ = "Date created"
Qgis.MetadataDateType.Published.__doc__ = "Date published"
Qgis.MetadataDateType.Revised.__doc__ = "Date revised"
Qgis.MetadataDateType.Superseded.__doc__ = "Date superseded"
Qgis.MetadataDateType.__doc__ = "Date types for metadata.\n\n.. versionadded:: 3.30\n\n" + '* ``Created``: ' + Qgis.MetadataDateType.Created.__doc__ + '\n' + '* ``Published``: ' + Qgis.MetadataDateType.Published.__doc__ + '\n' + '* ``Revised``: ' + Qgis.MetadataDateType.Revised.__doc__ + '\n' + '* ``Superseded``: ' + Qgis.MetadataDateType.Superseded.__doc__
# --
Qgis.MetadataDateType.baseClass = Qgis
QgsRaster.ColorInterpretation = Qgis.RasterColorInterpretation
# monkey patching scoped based enum
QgsRaster.UndefinedColorInterpretation = Qgis.RasterColorInterpretation.Undefined
QgsRaster.ColorInterpretation.UndefinedColorInterpretation = Qgis.RasterColorInterpretation.Undefined
QgsRaster.UndefinedColorInterpretation.is_monkey_patched = True
QgsRaster.UndefinedColorInterpretation.__doc__ = "Undefined"
QgsRaster.GrayIndex = Qgis.RasterColorInterpretation.GrayIndex
QgsRaster.GrayIndex.is_monkey_patched = True
QgsRaster.GrayIndex.__doc__ = "Grayscale"
QgsRaster.PaletteIndex = Qgis.RasterColorInterpretation.PaletteIndex
QgsRaster.PaletteIndex.is_monkey_patched = True
QgsRaster.PaletteIndex.__doc__ = "Paletted (see associated color table)"
QgsRaster.RedBand = Qgis.RasterColorInterpretation.RedBand
QgsRaster.RedBand.is_monkey_patched = True
QgsRaster.RedBand.__doc__ = "Red band of RGBA image"
QgsRaster.GreenBand = Qgis.RasterColorInterpretation.GreenBand
QgsRaster.GreenBand.is_monkey_patched = True
QgsRaster.GreenBand.__doc__ = "Green band of RGBA image"
QgsRaster.BlueBand = Qgis.RasterColorInterpretation.BlueBand
QgsRaster.BlueBand.is_monkey_patched = True
QgsRaster.BlueBand.__doc__ = "Blue band of RGBA image"
QgsRaster.AlphaBand = Qgis.RasterColorInterpretation.AlphaBand
QgsRaster.AlphaBand.is_monkey_patched = True
QgsRaster.AlphaBand.__doc__ = "Alpha (0=transparent, 255=opaque)"
QgsRaster.HueBand = Qgis.RasterColorInterpretation.HueBand
QgsRaster.HueBand.is_monkey_patched = True
QgsRaster.HueBand.__doc__ = "Hue band of HLS image"
QgsRaster.SaturationBand = Qgis.RasterColorInterpretation.SaturationBand
QgsRaster.SaturationBand.is_monkey_patched = True
QgsRaster.SaturationBand.__doc__ = "Saturation band of HLS image"
QgsRaster.LightnessBand = Qgis.RasterColorInterpretation.LightnessBand
QgsRaster.LightnessBand.is_monkey_patched = True
QgsRaster.LightnessBand.__doc__ = "Lightness band of HLS image"
QgsRaster.CyanBand = Qgis.RasterColorInterpretation.CyanBand
QgsRaster.CyanBand.is_monkey_patched = True
QgsRaster.CyanBand.__doc__ = "Cyan band of CMYK image"
QgsRaster.MagentaBand = Qgis.RasterColorInterpretation.MagentaBand
QgsRaster.MagentaBand.is_monkey_patched = True
QgsRaster.MagentaBand.__doc__ = "Magenta band of CMYK image"
QgsRaster.YellowBand = Qgis.RasterColorInterpretation.YellowBand
QgsRaster.YellowBand.is_monkey_patched = True
QgsRaster.YellowBand.__doc__ = "Yellow band of CMYK image"
QgsRaster.BlackBand = Qgis.RasterColorInterpretation.BlackBand
QgsRaster.BlackBand.is_monkey_patched = True
QgsRaster.BlackBand.__doc__ = "Black band of CMLY image"
QgsRaster.YCbCr_YBand = Qgis.RasterColorInterpretation.YCbCr_YBand
QgsRaster.YCbCr_YBand.is_monkey_patched = True
QgsRaster.YCbCr_YBand.__doc__ = "Y Luminance"
QgsRaster.YCbCr_CbBand = Qgis.RasterColorInterpretation.YCbCr_CbBand
QgsRaster.YCbCr_CbBand.is_monkey_patched = True
QgsRaster.YCbCr_CbBand.__doc__ = "Cb Chroma"
QgsRaster.YCbCr_CrBand = Qgis.RasterColorInterpretation.YCbCr_CrBand
QgsRaster.YCbCr_CrBand.is_monkey_patched = True
QgsRaster.YCbCr_CrBand.__doc__ = "Cr Chroma"
QgsRaster.ContinuousPalette = Qgis.RasterColorInterpretation.ContinuousPalette
QgsRaster.ContinuousPalette.is_monkey_patched = True
QgsRaster.ContinuousPalette.__doc__ = "Continuous palette, QGIS addition, GRASS"
Qgis.RasterColorInterpretation.__doc__ = "Raster color interpretation.\n\nThis is a modified copy of the GDAL GDALColorInterp enum.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.ColorInterpretation\n\n.. versionadded:: 3.30\n\n" + '* ``UndefinedColorInterpretation``: ' + Qgis.RasterColorInterpretation.Undefined.__doc__ + '\n' + '* ``GrayIndex``: ' + Qgis.RasterColorInterpretation.GrayIndex.__doc__ + '\n' + '* ``PaletteIndex``: ' + Qgis.RasterColorInterpretation.PaletteIndex.__doc__ + '\n' + '* ``RedBand``: ' + Qgis.RasterColorInterpretation.RedBand.__doc__ + '\n' + '* ``GreenBand``: ' + Qgis.RasterColorInterpretation.GreenBand.__doc__ + '\n' + '* ``BlueBand``: ' + Qgis.RasterColorInterpretation.BlueBand.__doc__ + '\n' + '* ``AlphaBand``: ' + Qgis.RasterColorInterpretation.AlphaBand.__doc__ + '\n' + '* ``HueBand``: ' + Qgis.RasterColorInterpretation.HueBand.__doc__ + '\n' + '* ``SaturationBand``: ' + Qgis.RasterColorInterpretation.SaturationBand.__doc__ + '\n' + '* ``LightnessBand``: ' + Qgis.RasterColorInterpretation.LightnessBand.__doc__ + '\n' + '* ``CyanBand``: ' + Qgis.RasterColorInterpretation.CyanBand.__doc__ + '\n' + '* ``MagentaBand``: ' + Qgis.RasterColorInterpretation.MagentaBand.__doc__ + '\n' + '* ``YellowBand``: ' + Qgis.RasterColorInterpretation.YellowBand.__doc__ + '\n' + '* ``BlackBand``: ' + Qgis.RasterColorInterpretation.BlackBand.__doc__ + '\n' + '* ``YCbCr_YBand``: ' + Qgis.RasterColorInterpretation.YCbCr_YBand.__doc__ + '\n' + '* ``YCbCr_CbBand``: ' + Qgis.RasterColorInterpretation.YCbCr_CbBand.__doc__ + '\n' + '* ``YCbCr_CrBand``: ' + Qgis.RasterColorInterpretation.YCbCr_CrBand.__doc__ + '\n' + '* ``ContinuousPalette``: ' + Qgis.RasterColorInterpretation.ContinuousPalette.__doc__
# --
Qgis.RasterColorInterpretation.baseClass = Qgis
QgsRasterLayer.LayerType = Qgis.RasterLayerType
# monkey patching scoped based enum
QgsRasterLayer.GrayOrUndefined = Qgis.RasterLayerType.GrayOrUndefined
QgsRasterLayer.GrayOrUndefined.is_monkey_patched = True
QgsRasterLayer.GrayOrUndefined.__doc__ = "Gray or undefined"
QgsRasterLayer.Palette = Qgis.RasterLayerType.Palette
QgsRasterLayer.Palette.is_monkey_patched = True
QgsRasterLayer.Palette.__doc__ = "Palette"
QgsRasterLayer.Multiband = Qgis.RasterLayerType.MultiBand
QgsRasterLayer.LayerType.Multiband = Qgis.RasterLayerType.MultiBand
QgsRasterLayer.Multiband.is_monkey_patched = True
QgsRasterLayer.Multiband.__doc__ = "Multi band"
QgsRasterLayer.ColorLayer = Qgis.RasterLayerType.SingleBandColorData
QgsRasterLayer.LayerType.ColorLayer = Qgis.RasterLayerType.SingleBandColorData
QgsRasterLayer.ColorLayer.is_monkey_patched = True
QgsRasterLayer.ColorLayer.__doc__ = "Single band containing color data"
Qgis.RasterLayerType.__doc__ = "Raster layer types.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsRasterLayer`.LayerType\n\n.. versionadded:: 3.30\n\n" + '* ``GrayOrUndefined``: ' + Qgis.RasterLayerType.GrayOrUndefined.__doc__ + '\n' + '* ``Palette``: ' + Qgis.RasterLayerType.Palette.__doc__ + '\n' + '* ``Multiband``: ' + Qgis.RasterLayerType.MultiBand.__doc__ + '\n' + '* ``ColorLayer``: ' + Qgis.RasterLayerType.SingleBandColorData.__doc__
# --
Qgis.RasterLayerType.baseClass = Qgis
QgsRaster.DrawingStyle = Qgis.RasterDrawingStyle
# monkey patching scoped based enum
QgsRaster.UndefinedDrawingStyle = Qgis.RasterDrawingStyle.Undefined
QgsRaster.DrawingStyle.UndefinedDrawingStyle = Qgis.RasterDrawingStyle.Undefined
QgsRaster.UndefinedDrawingStyle.is_monkey_patched = True
QgsRaster.UndefinedDrawingStyle.__doc__ = "Undefined"
QgsRaster.SingleBandGray = Qgis.RasterDrawingStyle.SingleBandGray
QgsRaster.SingleBandGray.is_monkey_patched = True
QgsRaster.SingleBandGray.__doc__ = "A single band image drawn as a range of gray colors"
QgsRaster.SingleBandPseudoColor = Qgis.RasterDrawingStyle.SingleBandPseudoColor
QgsRaster.SingleBandPseudoColor.is_monkey_patched = True
QgsRaster.SingleBandPseudoColor.__doc__ = "A single band image drawn using a pseudocolor algorithm"
QgsRaster.PalettedColor = Qgis.RasterDrawingStyle.PalettedColor
QgsRaster.PalettedColor.is_monkey_patched = True
QgsRaster.PalettedColor.__doc__ = "A \"Palette\" image drawn using color table"
QgsRaster.PalettedSingleBandGray = Qgis.RasterDrawingStyle.PalettedSingleBandGray
QgsRaster.PalettedSingleBandGray.is_monkey_patched = True
QgsRaster.PalettedSingleBandGray.__doc__ = "A \"Palette\" layer drawn in gray scale"
QgsRaster.PalettedSingleBandPseudoColor = Qgis.RasterDrawingStyle.PalettedSingleBandPseudoColor
QgsRaster.PalettedSingleBandPseudoColor.is_monkey_patched = True
QgsRaster.PalettedSingleBandPseudoColor.__doc__ = "A \"Palette\" layerdrawn using a pseudocolor algorithm"
QgsRaster.PalettedMultiBandColor = Qgis.RasterDrawingStyle.PalettedMultiBandColor
QgsRaster.PalettedMultiBandColor.is_monkey_patched = True
QgsRaster.PalettedMultiBandColor.__doc__ = "Currently not supported"
QgsRaster.MultiBandSingleBandGray = Qgis.RasterDrawingStyle.MultiBandSingleBandGray
QgsRaster.MultiBandSingleBandGray.is_monkey_patched = True
QgsRaster.MultiBandSingleBandGray.__doc__ = "A layer containing 2 or more bands, but a single band drawn as a range of gray colors"
QgsRaster.MultiBandSingleBandPseudoColor = Qgis.RasterDrawingStyle.MultiBandSingleBandPseudoColor
QgsRaster.MultiBandSingleBandPseudoColor.is_monkey_patched = True
QgsRaster.MultiBandSingleBandPseudoColor.__doc__ = "A layer containing 2 or more bands, but a single band drawn using a pseudocolor algorithm"
QgsRaster.MultiBandColor = Qgis.RasterDrawingStyle.MultiBandColor
QgsRaster.MultiBandColor.is_monkey_patched = True
QgsRaster.MultiBandColor.__doc__ = "A layer containing 2 or more bands, mapped to RGB color space. In the case of a multiband with only two bands, one band will be mapped to more than one color."
QgsRaster.SingleBandColorDataStyle = Qgis.RasterDrawingStyle.SingleBandColorData
QgsRaster.DrawingStyle.SingleBandColorDataStyle = Qgis.RasterDrawingStyle.SingleBandColorData
QgsRaster.SingleBandColorDataStyle.is_monkey_patched = True
QgsRaster.SingleBandColorDataStyle.__doc__ = "ARGB values rendered directly"
Qgis.RasterDrawingStyle.__doc__ = "Raster drawing styles.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.DrawingStyle\n\n.. versionadded:: 3.30\n\n" + '* ``UndefinedDrawingStyle``: ' + Qgis.RasterDrawingStyle.Undefined.__doc__ + '\n' + '* ``SingleBandGray``: ' + Qgis.RasterDrawingStyle.SingleBandGray.__doc__ + '\n' + '* ``SingleBandPseudoColor``: ' + Qgis.RasterDrawingStyle.SingleBandPseudoColor.__doc__ + '\n' + '* ``PalettedColor``: ' + Qgis.RasterDrawingStyle.PalettedColor.__doc__ + '\n' + '* ``PalettedSingleBandGray``: ' + Qgis.RasterDrawingStyle.PalettedSingleBandGray.__doc__ + '\n' + '* ``PalettedSingleBandPseudoColor``: ' + Qgis.RasterDrawingStyle.PalettedSingleBandPseudoColor.__doc__ + '\n' + '* ``PalettedMultiBandColor``: ' + Qgis.RasterDrawingStyle.PalettedMultiBandColor.__doc__ + '\n' + '* ``MultiBandSingleBandGray``: ' + Qgis.RasterDrawingStyle.MultiBandSingleBandGray.__doc__ + '\n' + '* ``MultiBandSingleBandPseudoColor``: ' + Qgis.RasterDrawingStyle.MultiBandSingleBandPseudoColor.__doc__ + '\n' + '* ``MultiBandColor``: ' + Qgis.RasterDrawingStyle.MultiBandColor.__doc__ + '\n' + '* ``SingleBandColorDataStyle``: ' + Qgis.RasterDrawingStyle.SingleBandColorData.__doc__
# --
Qgis.RasterDrawingStyle.baseClass = Qgis
QgsRaster.RasterPyramidsFormat = Qgis.RasterPyramidFormat
# monkey patching scoped based enum
QgsRaster.PyramidsGTiff = Qgis.RasterPyramidFormat.GeoTiff
QgsRaster.RasterPyramidsFormat.PyramidsGTiff = Qgis.RasterPyramidFormat.GeoTiff
QgsRaster.PyramidsGTiff.is_monkey_patched = True
QgsRaster.PyramidsGTiff.__doc__ = "Geotiff .ovr (external)"
QgsRaster.PyramidsInternal = Qgis.RasterPyramidFormat.Internal
QgsRaster.RasterPyramidsFormat.PyramidsInternal = Qgis.RasterPyramidFormat.Internal
QgsRaster.PyramidsInternal.is_monkey_patched = True
QgsRaster.PyramidsInternal.__doc__ = "Internal"
QgsRaster.PyramidsErdas = Qgis.RasterPyramidFormat.Erdas
QgsRaster.RasterPyramidsFormat.PyramidsErdas = Qgis.RasterPyramidFormat.Erdas
QgsRaster.PyramidsErdas.is_monkey_patched = True
QgsRaster.PyramidsErdas.__doc__ = "Erdas Image .aux (external)"
Qgis.RasterPyramidFormat.__doc__ = "Raster pyramid formats.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.RasterPyramidsFormat\n\n.. versionadded:: 3.30\n\n" + '* ``PyramidsGTiff``: ' + Qgis.RasterPyramidFormat.GeoTiff.__doc__ + '\n' + '* ``PyramidsInternal``: ' + Qgis.RasterPyramidFormat.Internal.__doc__ + '\n' + '* ``PyramidsErdas``: ' + Qgis.RasterPyramidFormat.Erdas.__doc__
# --
Qgis.RasterPyramidFormat.baseClass = Qgis
QgsRaster.RasterBuildPyramids = Qgis.RasterBuildPyramidOption
# monkey patching scoped based enum
QgsRaster.PyramidsFlagNo = Qgis.RasterBuildPyramidOption.No
QgsRaster.RasterBuildPyramids.PyramidsFlagNo = Qgis.RasterBuildPyramidOption.No
QgsRaster.PyramidsFlagNo.is_monkey_patched = True
QgsRaster.PyramidsFlagNo.__doc__ = "Never"
QgsRaster.PyramidsFlagYes = Qgis.RasterBuildPyramidOption.Yes
QgsRaster.RasterBuildPyramids.PyramidsFlagYes = Qgis.RasterBuildPyramidOption.Yes
QgsRaster.PyramidsFlagYes.is_monkey_patched = True
QgsRaster.PyramidsFlagYes.__doc__ = "Yes"
QgsRaster.PyramidsCopyExisting = Qgis.RasterBuildPyramidOption.CopyExisting
QgsRaster.RasterBuildPyramids.PyramidsCopyExisting = Qgis.RasterBuildPyramidOption.CopyExisting
QgsRaster.PyramidsCopyExisting.is_monkey_patched = True
QgsRaster.PyramidsCopyExisting.__doc__ = "Copy existing"
Qgis.RasterBuildPyramidOption.__doc__ = "Raster pyramid building options.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.RasterBuildPyramids\n\n.. versionadded:: 3.30\n\n" + '* ``PyramidsFlagNo``: ' + Qgis.RasterBuildPyramidOption.No.__doc__ + '\n' + '* ``PyramidsFlagYes``: ' + Qgis.RasterBuildPyramidOption.Yes.__doc__ + '\n' + '* ``PyramidsCopyExisting``: ' + Qgis.RasterBuildPyramidOption.CopyExisting.__doc__
# --
Qgis.RasterBuildPyramidOption.baseClass = Qgis
QgsRaster.IdentifyFormat = Qgis.RasterIdentifyFormat
# monkey patching scoped based enum
QgsRaster.IdentifyFormatUndefined = Qgis.RasterIdentifyFormat.Undefined
QgsRaster.IdentifyFormat.IdentifyFormatUndefined = Qgis.RasterIdentifyFormat.Undefined
QgsRaster.IdentifyFormatUndefined.is_monkey_patched = True
QgsRaster.IdentifyFormatUndefined.__doc__ = "Undefined"
QgsRaster.IdentifyFormatValue = Qgis.RasterIdentifyFormat.Value
QgsRaster.IdentifyFormat.IdentifyFormatValue = Qgis.RasterIdentifyFormat.Value
QgsRaster.IdentifyFormatValue.is_monkey_patched = True
QgsRaster.IdentifyFormatValue.__doc__ = "Numerical pixel value"
QgsRaster.IdentifyFormatText = Qgis.RasterIdentifyFormat.Text
QgsRaster.IdentifyFormat.IdentifyFormatText = Qgis.RasterIdentifyFormat.Text
QgsRaster.IdentifyFormatText.is_monkey_patched = True
QgsRaster.IdentifyFormatText.__doc__ = "WMS text"
QgsRaster.IdentifyFormatHtml = Qgis.RasterIdentifyFormat.Html
QgsRaster.IdentifyFormat.IdentifyFormatHtml = Qgis.RasterIdentifyFormat.Html
QgsRaster.IdentifyFormatHtml.is_monkey_patched = True
QgsRaster.IdentifyFormatHtml.__doc__ = "WMS HTML"
QgsRaster.IdentifyFormatFeature = Qgis.RasterIdentifyFormat.Feature
QgsRaster.IdentifyFormat.IdentifyFormatFeature = Qgis.RasterIdentifyFormat.Feature
QgsRaster.IdentifyFormatFeature.is_monkey_patched = True
QgsRaster.IdentifyFormatFeature.__doc__ = "WMS GML/JSON -> feature"
Qgis.RasterIdentifyFormat.__doc__ = "Raster identify formats.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.IdentifyFormat\n\n.. versionadded:: 3.30\n\n" + '* ``IdentifyFormatUndefined``: ' + Qgis.RasterIdentifyFormat.Undefined.__doc__ + '\n' + '* ``IdentifyFormatValue``: ' + Qgis.RasterIdentifyFormat.Value.__doc__ + '\n' + '* ``IdentifyFormatText``: ' + Qgis.RasterIdentifyFormat.Text.__doc__ + '\n' + '* ``IdentifyFormatHtml``: ' + Qgis.RasterIdentifyFormat.Html.__doc__ + '\n' + '* ``IdentifyFormatFeature``: ' + Qgis.RasterIdentifyFormat.Feature.__doc__
# --
Qgis.RasterIdentifyFormat.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ElevationMapCombineMethod.HighestElevation.__doc__ = "Keep the highest elevation if it is not null"
Qgis.ElevationMapCombineMethod.NewerElevation.__doc__ = "Keep the new elevation regardless of its value if it is not null"
Qgis.ElevationMapCombineMethod.__doc__ = "Methods used to select the elevation when two elevation maps are combined\n\n.. versionadded:: 3.30\n\n" + '* ``HighestElevation``: ' + Qgis.ElevationMapCombineMethod.HighestElevation.__doc__ + '\n' + '* ``NewerElevation``: ' + Qgis.ElevationMapCombineMethod.NewerElevation.__doc__
# --
Qgis.ElevationMapCombineMethod.baseClass = Qgis
QgsPainting.BlendMode = Qgis.BlendMode
# monkey patching scoped based enum
QgsPainting.BlendNormal = Qgis.BlendMode.Normal
QgsPainting.BlendMode.BlendNormal = Qgis.BlendMode.Normal
QgsPainting.BlendNormal.is_monkey_patched = True
QgsPainting.BlendNormal.__doc__ = "Normal"
QgsPainting.BlendLighten = Qgis.BlendMode.Lighten
QgsPainting.BlendMode.BlendLighten = Qgis.BlendMode.Lighten
QgsPainting.BlendLighten.is_monkey_patched = True
QgsPainting.BlendLighten.__doc__ = "Lighten"
QgsPainting.BlendScreen = Qgis.BlendMode.Screen
QgsPainting.BlendMode.BlendScreen = Qgis.BlendMode.Screen
QgsPainting.BlendScreen.is_monkey_patched = True
QgsPainting.BlendScreen.__doc__ = "Screen"
QgsPainting.BlendDodge = Qgis.BlendMode.Dodge
QgsPainting.BlendMode.BlendDodge = Qgis.BlendMode.Dodge
QgsPainting.BlendDodge.is_monkey_patched = True
QgsPainting.BlendDodge.__doc__ = "Dodge"
QgsPainting.BlendAddition = Qgis.BlendMode.Addition
QgsPainting.BlendMode.BlendAddition = Qgis.BlendMode.Addition
QgsPainting.BlendAddition.is_monkey_patched = True
QgsPainting.BlendAddition.__doc__ = "Addition"
QgsPainting.BlendDarken = Qgis.BlendMode.Darken
QgsPainting.BlendMode.BlendDarken = Qgis.BlendMode.Darken
QgsPainting.BlendDarken.is_monkey_patched = True
QgsPainting.BlendDarken.__doc__ = "Darken"
QgsPainting.BlendMultiply = Qgis.BlendMode.Multiply
QgsPainting.BlendMode.BlendMultiply = Qgis.BlendMode.Multiply
QgsPainting.BlendMultiply.is_monkey_patched = True
QgsPainting.BlendMultiply.__doc__ = "Multiple"
QgsPainting.BlendBurn = Qgis.BlendMode.Burn
QgsPainting.BlendMode.BlendBurn = Qgis.BlendMode.Burn
QgsPainting.BlendBurn.is_monkey_patched = True
QgsPainting.BlendBurn.__doc__ = "Burn"
QgsPainting.BlendOverlay = Qgis.BlendMode.Overlay
QgsPainting.BlendMode.BlendOverlay = Qgis.BlendMode.Overlay
QgsPainting.BlendOverlay.is_monkey_patched = True
QgsPainting.BlendOverlay.__doc__ = "Overlay"
QgsPainting.BlendSoftLight = Qgis.BlendMode.SoftLight
QgsPainting.BlendMode.BlendSoftLight = Qgis.BlendMode.SoftLight
QgsPainting.BlendSoftLight.is_monkey_patched = True
QgsPainting.BlendSoftLight.__doc__ = "Soft light"
QgsPainting.BlendHardLight = Qgis.BlendMode.HardLight
QgsPainting.BlendMode.BlendHardLight = Qgis.BlendMode.HardLight
QgsPainting.BlendHardLight.is_monkey_patched = True
QgsPainting.BlendHardLight.__doc__ = "Hard light"
QgsPainting.BlendDifference = Qgis.BlendMode.Difference
QgsPainting.BlendMode.BlendDifference = Qgis.BlendMode.Difference
QgsPainting.BlendDifference.is_monkey_patched = True
QgsPainting.BlendDifference.__doc__ = "Difference"
QgsPainting.BlendSubtract = Qgis.BlendMode.Subtract
QgsPainting.BlendMode.BlendSubtract = Qgis.BlendMode.Subtract
QgsPainting.BlendSubtract.is_monkey_patched = True
QgsPainting.BlendSubtract.__doc__ = "Subtract"
QgsPainting.BlendSource = Qgis.BlendMode.Source
QgsPainting.BlendMode.BlendSource = Qgis.BlendMode.Source
QgsPainting.BlendSource.is_monkey_patched = True
QgsPainting.BlendSource.__doc__ = "Source"
QgsPainting.BlendDestinationOver = Qgis.BlendMode.DestinationOver
QgsPainting.BlendMode.BlendDestinationOver = Qgis.BlendMode.DestinationOver
QgsPainting.BlendDestinationOver.is_monkey_patched = True
QgsPainting.BlendDestinationOver.__doc__ = "Destination over"
QgsPainting.BlendClear = Qgis.BlendMode.Clear
QgsPainting.BlendMode.BlendClear = Qgis.BlendMode.Clear
QgsPainting.BlendClear.is_monkey_patched = True
QgsPainting.BlendClear.__doc__ = "Clear"
QgsPainting.BlendDestination = Qgis.BlendMode.Destination
QgsPainting.BlendMode.BlendDestination = Qgis.BlendMode.Destination
QgsPainting.BlendDestination.is_monkey_patched = True
QgsPainting.BlendDestination.__doc__ = "Destination"
QgsPainting.BlendSourceIn = Qgis.BlendMode.SourceIn
QgsPainting.BlendMode.BlendSourceIn = Qgis.BlendMode.SourceIn
QgsPainting.BlendSourceIn.is_monkey_patched = True
QgsPainting.BlendSourceIn.__doc__ = "Source in"
QgsPainting.BlendDestinationIn = Qgis.BlendMode.DestinationIn
QgsPainting.BlendMode.BlendDestinationIn = Qgis.BlendMode.DestinationIn
QgsPainting.BlendDestinationIn.is_monkey_patched = True
QgsPainting.BlendDestinationIn.__doc__ = "Destination in"
QgsPainting.BlendSourceOut = Qgis.BlendMode.SourceOut
QgsPainting.BlendMode.BlendSourceOut = Qgis.BlendMode.SourceOut
QgsPainting.BlendSourceOut.is_monkey_patched = True
QgsPainting.BlendSourceOut.__doc__ = "Source out"
QgsPainting.BlendDestinationOut = Qgis.BlendMode.DestinationOut
QgsPainting.BlendMode.BlendDestinationOut = Qgis.BlendMode.DestinationOut
QgsPainting.BlendDestinationOut.is_monkey_patched = True
QgsPainting.BlendDestinationOut.__doc__ = "Destination out"
QgsPainting.BlendSourceAtop = Qgis.BlendMode.SourceAtop
QgsPainting.BlendMode.BlendSourceAtop = Qgis.BlendMode.SourceAtop
QgsPainting.BlendSourceAtop.is_monkey_patched = True
QgsPainting.BlendSourceAtop.__doc__ = "Source atop"
QgsPainting.BlendDestinationAtop = Qgis.BlendMode.DestinationAtop
QgsPainting.BlendMode.BlendDestinationAtop = Qgis.BlendMode.DestinationAtop
QgsPainting.BlendDestinationAtop.is_monkey_patched = True
QgsPainting.BlendDestinationAtop.__doc__ = "Destination atop"
QgsPainting.BlendXor = Qgis.BlendMode.Xor
QgsPainting.BlendMode.BlendXor = Qgis.BlendMode.Xor
QgsPainting.BlendXor.is_monkey_patched = True
QgsPainting.BlendXor.__doc__ = "XOR"
Qgis.BlendMode.__doc__ = "Blending modes defining the available composition modes that can\nbe used when painting.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsPainting`.BlendMode.\n\n.. versionadded:: 3.30\n\n" + '* ``BlendNormal``: ' + Qgis.BlendMode.Normal.__doc__ + '\n' + '* ``BlendLighten``: ' + Qgis.BlendMode.Lighten.__doc__ + '\n' + '* ``BlendScreen``: ' + Qgis.BlendMode.Screen.__doc__ + '\n' + '* ``BlendDodge``: ' + Qgis.BlendMode.Dodge.__doc__ + '\n' + '* ``BlendAddition``: ' + Qgis.BlendMode.Addition.__doc__ + '\n' + '* ``BlendDarken``: ' + Qgis.BlendMode.Darken.__doc__ + '\n' + '* ``BlendMultiply``: ' + Qgis.BlendMode.Multiply.__doc__ + '\n' + '* ``BlendBurn``: ' + Qgis.BlendMode.Burn.__doc__ + '\n' + '* ``BlendOverlay``: ' + Qgis.BlendMode.Overlay.__doc__ + '\n' + '* ``BlendSoftLight``: ' + Qgis.BlendMode.SoftLight.__doc__ + '\n' + '* ``BlendHardLight``: ' + Qgis.BlendMode.HardLight.__doc__ + '\n' + '* ``BlendDifference``: ' + Qgis.BlendMode.Difference.__doc__ + '\n' + '* ``BlendSubtract``: ' + Qgis.BlendMode.Subtract.__doc__ + '\n' + '* ``BlendSource``: ' + Qgis.BlendMode.Source.__doc__ + '\n' + '* ``BlendDestinationOver``: ' + Qgis.BlendMode.DestinationOver.__doc__ + '\n' + '* ``BlendClear``: ' + Qgis.BlendMode.Clear.__doc__ + '\n' + '* ``BlendDestination``: ' + Qgis.BlendMode.Destination.__doc__ + '\n' + '* ``BlendSourceIn``: ' + Qgis.BlendMode.SourceIn.__doc__ + '\n' + '* ``BlendDestinationIn``: ' + Qgis.BlendMode.DestinationIn.__doc__ + '\n' + '* ``BlendSourceOut``: ' + Qgis.BlendMode.SourceOut.__doc__ + '\n' + '* ``BlendDestinationOut``: ' + Qgis.BlendMode.DestinationOut.__doc__ + '\n' + '* ``BlendSourceAtop``: ' + Qgis.BlendMode.SourceAtop.__doc__ + '\n' + '* ``BlendDestinationAtop``: ' + Qgis.BlendMode.DestinationAtop.__doc__ + '\n' + '* ``BlendXor``: ' + Qgis.BlendMode.Xor.__doc__
# --
Qgis.BlendMode.baseClass = Qgis
QgsUnitTypes.SystemOfMeasurement = Qgis.SystemOfMeasurement
# monkey patching scoped based enum
QgsUnitTypes.UnknownSystem = Qgis.SystemOfMeasurement.Unknown
QgsUnitTypes.SystemOfMeasurement.UnknownSystem = Qgis.SystemOfMeasurement.Unknown
QgsUnitTypes.UnknownSystem.is_monkey_patched = True
QgsUnitTypes.UnknownSystem.__doc__ = "Unknown system of measurement"
QgsUnitTypes.MetricSystem = Qgis.SystemOfMeasurement.Metric
QgsUnitTypes.SystemOfMeasurement.MetricSystem = Qgis.SystemOfMeasurement.Metric
QgsUnitTypes.MetricSystem.is_monkey_patched = True
QgsUnitTypes.MetricSystem.__doc__ = "International System of Units (SI)"
QgsUnitTypes.ImperialSystem = Qgis.SystemOfMeasurement.Imperial
QgsUnitTypes.SystemOfMeasurement.ImperialSystem = Qgis.SystemOfMeasurement.Imperial
QgsUnitTypes.ImperialSystem.is_monkey_patched = True
QgsUnitTypes.ImperialSystem.__doc__ = "British Imperial"
QgsUnitTypes.USCSSystem = Qgis.SystemOfMeasurement.USCS
QgsUnitTypes.SystemOfMeasurement.USCSSystem = Qgis.SystemOfMeasurement.USCS
QgsUnitTypes.USCSSystem.is_monkey_patched = True
QgsUnitTypes.USCSSystem.__doc__ = "United States customary system"
Qgis.SystemOfMeasurement.__doc__ = "Systems of unit measurement.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.SystemOfMeasurement.\n\n.. versionadded:: 3.30\n\n" + '* ``UnknownSystem``: ' + Qgis.SystemOfMeasurement.Unknown.__doc__ + '\n' + '* ``MetricSystem``: ' + Qgis.SystemOfMeasurement.Metric.__doc__ + '\n' + '* ``ImperialSystem``: ' + Qgis.SystemOfMeasurement.Imperial.__doc__ + '\n' + '* ``USCSSystem``: ' + Qgis.SystemOfMeasurement.USCS.__doc__
# --
Qgis.SystemOfMeasurement.baseClass = Qgis
QgsTolerance.UnitType = Qgis.MapToolUnit
# monkey patching scoped based enum
QgsTolerance.LayerUnits = Qgis.MapToolUnit.Layer
QgsTolerance.UnitType.LayerUnits = Qgis.MapToolUnit.Layer
QgsTolerance.LayerUnits.is_monkey_patched = True
QgsTolerance.LayerUnits.__doc__ = "Layer unit value"
QgsTolerance.Pixels = Qgis.MapToolUnit.Pixels
QgsTolerance.Pixels.is_monkey_patched = True
QgsTolerance.Pixels.__doc__ = "Pixels unit of tolerance"
QgsTolerance.ProjectUnits = Qgis.MapToolUnit.Project
QgsTolerance.UnitType.ProjectUnits = Qgis.MapToolUnit.Project
QgsTolerance.ProjectUnits.is_monkey_patched = True
QgsTolerance.ProjectUnits.__doc__ = "Map (project) units"
Qgis.MapToolUnit.__doc__ = "Type of unit of tolerance value from settings.\nFor map (project) units, use MapToolUnit.Project.\n\n.. versionadded:: 3.32\n\n" + '* ``LayerUnits``: ' + Qgis.MapToolUnit.Layer.__doc__ + '\n' + '* ``Pixels``: ' + Qgis.MapToolUnit.Pixels.__doc__ + '\n' + '* ``ProjectUnits``: ' + Qgis.MapToolUnit.Project.__doc__
# --
Qgis.MapToolUnit.baseClass = Qgis
QgsUnitTypes.UnitType = Qgis.UnitType
# monkey patching scoped based enum
QgsUnitTypes.TypeDistance = Qgis.UnitType.Distance
QgsUnitTypes.UnitType.TypeDistance = Qgis.UnitType.Distance
QgsUnitTypes.TypeDistance.is_monkey_patched = True
QgsUnitTypes.TypeDistance.__doc__ = "Distance unit"
QgsUnitTypes.TypeArea = Qgis.UnitType.Area
QgsUnitTypes.UnitType.TypeArea = Qgis.UnitType.Area
QgsUnitTypes.TypeArea.is_monkey_patched = True
QgsUnitTypes.TypeArea.__doc__ = "Area unit"
QgsUnitTypes.TypeVolume = Qgis.UnitType.Volume
QgsUnitTypes.UnitType.TypeVolume = Qgis.UnitType.Volume
QgsUnitTypes.TypeVolume.is_monkey_patched = True
QgsUnitTypes.TypeVolume.__doc__ = "Volume unit"
QgsUnitTypes.TypeUnknown = Qgis.UnitType.Unknown
QgsUnitTypes.UnitType.TypeUnknown = Qgis.UnitType.Unknown
QgsUnitTypes.TypeUnknown.is_monkey_patched = True
QgsUnitTypes.TypeUnknown.__doc__ = "Unknown unit type"
QgsUnitTypes.TypeTemporal = Qgis.UnitType.Temporal
QgsUnitTypes.UnitType.TypeTemporal = Qgis.UnitType.Temporal
QgsUnitTypes.TypeTemporal.is_monkey_patched = True
QgsUnitTypes.TypeTemporal.__doc__ = "Temporal unit"
Qgis.UnitType.__doc__ = "Unit types.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.UnitType.\n\n.. versionadded:: 3.30\n\n" + '* ``TypeDistance``: ' + Qgis.UnitType.Distance.__doc__ + '\n' + '* ``TypeArea``: ' + Qgis.UnitType.Area.__doc__ + '\n' + '* ``TypeVolume``: ' + Qgis.UnitType.Volume.__doc__ + '\n' + '* ``TypeUnknown``: ' + Qgis.UnitType.Unknown.__doc__ + '\n' + '* ``TypeTemporal``: ' + Qgis.UnitType.Temporal.__doc__
# --
Qgis.UnitType.baseClass = Qgis
QgsUnitTypes.DistanceUnit = Qgis.DistanceUnit
# monkey patching scoped based enum
QgsUnitTypes.DistanceMeters = Qgis.DistanceUnit.Meters
QgsUnitTypes.DistanceUnit.DistanceMeters = Qgis.DistanceUnit.Meters
QgsUnitTypes.DistanceMeters.is_monkey_patched = True
QgsUnitTypes.DistanceMeters.__doc__ = "Meters"
QgsUnitTypes.DistanceKilometers = Qgis.DistanceUnit.Kilometers
QgsUnitTypes.DistanceUnit.DistanceKilometers = Qgis.DistanceUnit.Kilometers
QgsUnitTypes.DistanceKilometers.is_monkey_patched = True
QgsUnitTypes.DistanceKilometers.__doc__ = "Kilometers"
QgsUnitTypes.DistanceFeet = Qgis.DistanceUnit.Feet
QgsUnitTypes.DistanceUnit.DistanceFeet = Qgis.DistanceUnit.Feet
QgsUnitTypes.DistanceFeet.is_monkey_patched = True
QgsUnitTypes.DistanceFeet.__doc__ = "Imperial feet"
QgsUnitTypes.DistanceNauticalMiles = Qgis.DistanceUnit.NauticalMiles
QgsUnitTypes.DistanceUnit.DistanceNauticalMiles = Qgis.DistanceUnit.NauticalMiles
QgsUnitTypes.DistanceNauticalMiles.is_monkey_patched = True
QgsUnitTypes.DistanceNauticalMiles.__doc__ = "Nautical miles"
QgsUnitTypes.DistanceYards = Qgis.DistanceUnit.Yards
QgsUnitTypes.DistanceUnit.DistanceYards = Qgis.DistanceUnit.Yards
QgsUnitTypes.DistanceYards.is_monkey_patched = True
QgsUnitTypes.DistanceYards.__doc__ = "Imperial yards"
QgsUnitTypes.DistanceMiles = Qgis.DistanceUnit.Miles
QgsUnitTypes.DistanceUnit.DistanceMiles = Qgis.DistanceUnit.Miles
QgsUnitTypes.DistanceMiles.is_monkey_patched = True
QgsUnitTypes.DistanceMiles.__doc__ = "Terrestrial miles"
QgsUnitTypes.DistanceDegrees = Qgis.DistanceUnit.Degrees
QgsUnitTypes.DistanceUnit.DistanceDegrees = Qgis.DistanceUnit.Degrees
QgsUnitTypes.DistanceDegrees.is_monkey_patched = True
QgsUnitTypes.DistanceDegrees.__doc__ = "Degrees, for planar geographic CRS distance measurements"
QgsUnitTypes.DistanceCentimeters = Qgis.DistanceUnit.Centimeters
QgsUnitTypes.DistanceUnit.DistanceCentimeters = Qgis.DistanceUnit.Centimeters
QgsUnitTypes.DistanceCentimeters.is_monkey_patched = True
QgsUnitTypes.DistanceCentimeters.__doc__ = "Centimeters"
QgsUnitTypes.DistanceMillimeters = Qgis.DistanceUnit.Millimeters
QgsUnitTypes.DistanceUnit.DistanceMillimeters = Qgis.DistanceUnit.Millimeters
QgsUnitTypes.DistanceMillimeters.is_monkey_patched = True
QgsUnitTypes.DistanceMillimeters.__doc__ = "Millimeters"
QgsUnitTypes.Inches = Qgis.DistanceUnit.Inches
QgsUnitTypes.Inches.is_monkey_patched = True
QgsUnitTypes.Inches.__doc__ = "Inches (since QGIS 3.32)"
QgsUnitTypes.DistanceUnknownUnit = Qgis.DistanceUnit.Unknown
QgsUnitTypes.DistanceUnit.DistanceUnknownUnit = Qgis.DistanceUnit.Unknown
QgsUnitTypes.DistanceUnknownUnit.is_monkey_patched = True
QgsUnitTypes.DistanceUnknownUnit.__doc__ = "Unknown distance unit"
Qgis.DistanceUnit.__doc__ = "Units of distance\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.DistanceUnit.\n\n.. versionadded:: 3.30\n\n" + '* ``DistanceMeters``: ' + Qgis.DistanceUnit.Meters.__doc__ + '\n' + '* ``DistanceKilometers``: ' + Qgis.DistanceUnit.Kilometers.__doc__ + '\n' + '* ``DistanceFeet``: ' + Qgis.DistanceUnit.Feet.__doc__ + '\n' + '* ``DistanceNauticalMiles``: ' + Qgis.DistanceUnit.NauticalMiles.__doc__ + '\n' + '* ``DistanceYards``: ' + Qgis.DistanceUnit.Yards.__doc__ + '\n' + '* ``DistanceMiles``: ' + Qgis.DistanceUnit.Miles.__doc__ + '\n' + '* ``DistanceDegrees``: ' + Qgis.DistanceUnit.Degrees.__doc__ + '\n' + '* ``DistanceCentimeters``: ' + Qgis.DistanceUnit.Centimeters.__doc__ + '\n' + '* ``DistanceMillimeters``: ' + Qgis.DistanceUnit.Millimeters.__doc__ + '\n' + '* ``Inches``: ' + Qgis.DistanceUnit.Inches.__doc__ + '\n' + '* ``DistanceUnknownUnit``: ' + Qgis.DistanceUnit.Unknown.__doc__
# --
Qgis.DistanceUnit.baseClass = Qgis
QgsUnitTypes.DistanceUnitType = Qgis.DistanceUnitType
# monkey patching scoped based enum
QgsUnitTypes.Standard = Qgis.DistanceUnitType.Standard
QgsUnitTypes.Standard.is_monkey_patched = True
QgsUnitTypes.Standard.__doc__ = "Unit is a standard measurement unit"
QgsUnitTypes.Geographic = Qgis.DistanceUnitType.Geographic
QgsUnitTypes.Geographic.is_monkey_patched = True
QgsUnitTypes.Geographic.__doc__ = "Unit is a geographic (e.g., degree based) unit"
QgsUnitTypes.UnknownType = Qgis.DistanceUnitType.Unknown
QgsUnitTypes.DistanceUnitType.UnknownType = Qgis.DistanceUnitType.Unknown
QgsUnitTypes.UnknownType.is_monkey_patched = True
QgsUnitTypes.UnknownType.__doc__ = "Unknown unit type"
Qgis.DistanceUnitType.__doc__ = "Types of distance units\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.DistanceUnitType.\n\n.. versionadded:: 3.30\n\n" + '* ``Standard``: ' + Qgis.DistanceUnitType.Standard.__doc__ + '\n' + '* ``Geographic``: ' + Qgis.DistanceUnitType.Geographic.__doc__ + '\n' + '* ``UnknownType``: ' + Qgis.DistanceUnitType.Unknown.__doc__
# --
Qgis.DistanceUnitType.baseClass = Qgis
QgsUnitTypes.AreaUnit = Qgis.AreaUnit
# monkey patching scoped based enum
QgsUnitTypes.AreaSquareMeters = Qgis.AreaUnit.SquareMeters
QgsUnitTypes.AreaUnit.AreaSquareMeters = Qgis.AreaUnit.SquareMeters
QgsUnitTypes.AreaSquareMeters.is_monkey_patched = True
QgsUnitTypes.AreaSquareMeters.__doc__ = "Square meters"
QgsUnitTypes.AreaSquareKilometers = Qgis.AreaUnit.SquareKilometers
QgsUnitTypes.AreaUnit.AreaSquareKilometers = Qgis.AreaUnit.SquareKilometers
QgsUnitTypes.AreaSquareKilometers.is_monkey_patched = True
QgsUnitTypes.AreaSquareKilometers.__doc__ = "Square kilometers"
QgsUnitTypes.AreaSquareFeet = Qgis.AreaUnit.SquareFeet
QgsUnitTypes.AreaUnit.AreaSquareFeet = Qgis.AreaUnit.SquareFeet
QgsUnitTypes.AreaSquareFeet.is_monkey_patched = True
QgsUnitTypes.AreaSquareFeet.__doc__ = "Square feet"
QgsUnitTypes.AreaSquareYards = Qgis.AreaUnit.SquareYards
QgsUnitTypes.AreaUnit.AreaSquareYards = Qgis.AreaUnit.SquareYards
QgsUnitTypes.AreaSquareYards.is_monkey_patched = True
QgsUnitTypes.AreaSquareYards.__doc__ = "Square yards"
QgsUnitTypes.AreaSquareMiles = Qgis.AreaUnit.SquareMiles
QgsUnitTypes.AreaUnit.AreaSquareMiles = Qgis.AreaUnit.SquareMiles
QgsUnitTypes.AreaSquareMiles.is_monkey_patched = True
QgsUnitTypes.AreaSquareMiles.__doc__ = "Square miles"
QgsUnitTypes.AreaHectares = Qgis.AreaUnit.Hectares
QgsUnitTypes.AreaUnit.AreaHectares = Qgis.AreaUnit.Hectares
QgsUnitTypes.AreaHectares.is_monkey_patched = True
QgsUnitTypes.AreaHectares.__doc__ = "Hectares"
QgsUnitTypes.AreaAcres = Qgis.AreaUnit.Acres
QgsUnitTypes.AreaUnit.AreaAcres = Qgis.AreaUnit.Acres
QgsUnitTypes.AreaAcres.is_monkey_patched = True
QgsUnitTypes.AreaAcres.__doc__ = "Acres"
QgsUnitTypes.AreaSquareNauticalMiles = Qgis.AreaUnit.SquareNauticalMiles
QgsUnitTypes.AreaUnit.AreaSquareNauticalMiles = Qgis.AreaUnit.SquareNauticalMiles
QgsUnitTypes.AreaSquareNauticalMiles.is_monkey_patched = True
QgsUnitTypes.AreaSquareNauticalMiles.__doc__ = "Square nautical miles"
QgsUnitTypes.AreaSquareDegrees = Qgis.AreaUnit.SquareDegrees
QgsUnitTypes.AreaUnit.AreaSquareDegrees = Qgis.AreaUnit.SquareDegrees
QgsUnitTypes.AreaSquareDegrees.is_monkey_patched = True
QgsUnitTypes.AreaSquareDegrees.__doc__ = "Square degrees, for planar geographic CRS area measurements"
QgsUnitTypes.AreaSquareCentimeters = Qgis.AreaUnit.SquareCentimeters
QgsUnitTypes.AreaUnit.AreaSquareCentimeters = Qgis.AreaUnit.SquareCentimeters
QgsUnitTypes.AreaSquareCentimeters.is_monkey_patched = True
QgsUnitTypes.AreaSquareCentimeters.__doc__ = "Square centimeters"
QgsUnitTypes.AreaSquareMillimeters = Qgis.AreaUnit.SquareMillimeters
QgsUnitTypes.AreaUnit.AreaSquareMillimeters = Qgis.AreaUnit.SquareMillimeters
QgsUnitTypes.AreaSquareMillimeters.is_monkey_patched = True
QgsUnitTypes.AreaSquareMillimeters.__doc__ = "Square millimeters"
QgsUnitTypes.SquareInches = Qgis.AreaUnit.SquareInches
QgsUnitTypes.SquareInches.is_monkey_patched = True
QgsUnitTypes.SquareInches.__doc__ = "Square inches (since QGIS 3.32)"
QgsUnitTypes.AreaUnknownUnit = Qgis.AreaUnit.Unknown
QgsUnitTypes.AreaUnit.AreaUnknownUnit = Qgis.AreaUnit.Unknown
QgsUnitTypes.AreaUnknownUnit.is_monkey_patched = True
QgsUnitTypes.AreaUnknownUnit.__doc__ = "Unknown areal unit"
Qgis.AreaUnit.__doc__ = "Units of area\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.AreaUnit.\n\n.. versionadded:: 3.30\n\n" + '* ``AreaSquareMeters``: ' + Qgis.AreaUnit.SquareMeters.__doc__ + '\n' + '* ``AreaSquareKilometers``: ' + Qgis.AreaUnit.SquareKilometers.__doc__ + '\n' + '* ``AreaSquareFeet``: ' + Qgis.AreaUnit.SquareFeet.__doc__ + '\n' + '* ``AreaSquareYards``: ' + Qgis.AreaUnit.SquareYards.__doc__ + '\n' + '* ``AreaSquareMiles``: ' + Qgis.AreaUnit.SquareMiles.__doc__ + '\n' + '* ``AreaHectares``: ' + Qgis.AreaUnit.Hectares.__doc__ + '\n' + '* ``AreaAcres``: ' + Qgis.AreaUnit.Acres.__doc__ + '\n' + '* ``AreaSquareNauticalMiles``: ' + Qgis.AreaUnit.SquareNauticalMiles.__doc__ + '\n' + '* ``AreaSquareDegrees``: ' + Qgis.AreaUnit.SquareDegrees.__doc__ + '\n' + '* ``AreaSquareCentimeters``: ' + Qgis.AreaUnit.SquareCentimeters.__doc__ + '\n' + '* ``AreaSquareMillimeters``: ' + Qgis.AreaUnit.SquareMillimeters.__doc__ + '\n' + '* ``SquareInches``: ' + Qgis.AreaUnit.SquareInches.__doc__ + '\n' + '* ``AreaUnknownUnit``: ' + Qgis.AreaUnit.Unknown.__doc__
# --
Qgis.AreaUnit.baseClass = Qgis
QgsUnitTypes.VolumeUnit = Qgis.VolumeUnit
# monkey patching scoped based enum
QgsUnitTypes.VolumeCubicMeters = Qgis.VolumeUnit.CubicMeters
QgsUnitTypes.VolumeUnit.VolumeCubicMeters = Qgis.VolumeUnit.CubicMeters
QgsUnitTypes.VolumeCubicMeters.is_monkey_patched = True
QgsUnitTypes.VolumeCubicMeters.__doc__ = "Cubic meters"
QgsUnitTypes.VolumeCubicFeet = Qgis.VolumeUnit.CubicFeet
QgsUnitTypes.VolumeUnit.VolumeCubicFeet = Qgis.VolumeUnit.CubicFeet
QgsUnitTypes.VolumeCubicFeet.is_monkey_patched = True
QgsUnitTypes.VolumeCubicFeet.__doc__ = "Cubic feet"
QgsUnitTypes.VolumeCubicYards = Qgis.VolumeUnit.CubicYards
QgsUnitTypes.VolumeUnit.VolumeCubicYards = Qgis.VolumeUnit.CubicYards
QgsUnitTypes.VolumeCubicYards.is_monkey_patched = True
QgsUnitTypes.VolumeCubicYards.__doc__ = "Cubic yards"
QgsUnitTypes.VolumeBarrel = Qgis.VolumeUnit.Barrel
QgsUnitTypes.VolumeUnit.VolumeBarrel = Qgis.VolumeUnit.Barrel
QgsUnitTypes.VolumeBarrel.is_monkey_patched = True
QgsUnitTypes.VolumeBarrel.__doc__ = "Barrels"
QgsUnitTypes.VolumeCubicDecimeter = Qgis.VolumeUnit.CubicDecimeter
QgsUnitTypes.VolumeUnit.VolumeCubicDecimeter = Qgis.VolumeUnit.CubicDecimeter
QgsUnitTypes.VolumeCubicDecimeter.is_monkey_patched = True
QgsUnitTypes.VolumeCubicDecimeter.__doc__ = "Cubic decimeters"
QgsUnitTypes.VolumeLiters = Qgis.VolumeUnit.Liters
QgsUnitTypes.VolumeUnit.VolumeLiters = Qgis.VolumeUnit.Liters
QgsUnitTypes.VolumeLiters.is_monkey_patched = True
QgsUnitTypes.VolumeLiters.__doc__ = "Litres"
QgsUnitTypes.VolumeGallonUS = Qgis.VolumeUnit.GallonUS
QgsUnitTypes.VolumeUnit.VolumeGallonUS = Qgis.VolumeUnit.GallonUS
QgsUnitTypes.VolumeGallonUS.is_monkey_patched = True
QgsUnitTypes.VolumeGallonUS.__doc__ = "US Gallons"
QgsUnitTypes.VolumeCubicInch = Qgis.VolumeUnit.CubicInch
QgsUnitTypes.VolumeUnit.VolumeCubicInch = Qgis.VolumeUnit.CubicInch
QgsUnitTypes.VolumeCubicInch.is_monkey_patched = True
QgsUnitTypes.VolumeCubicInch.__doc__ = "Cubic inches"
QgsUnitTypes.VolumeCubicCentimeter = Qgis.VolumeUnit.CubicCentimeter
QgsUnitTypes.VolumeUnit.VolumeCubicCentimeter = Qgis.VolumeUnit.CubicCentimeter
QgsUnitTypes.VolumeCubicCentimeter.is_monkey_patched = True
QgsUnitTypes.VolumeCubicCentimeter.__doc__ = "Cubic Centimeters"
QgsUnitTypes.VolumeCubicDegrees = Qgis.VolumeUnit.CubicDegrees
QgsUnitTypes.VolumeUnit.VolumeCubicDegrees = Qgis.VolumeUnit.CubicDegrees
QgsUnitTypes.VolumeCubicDegrees.is_monkey_patched = True
QgsUnitTypes.VolumeCubicDegrees.__doc__ = "Cubic degrees, for planar geographic CRS volume measurements"
QgsUnitTypes.VolumeUnknownUnit = Qgis.VolumeUnit.Unknown
QgsUnitTypes.VolumeUnit.VolumeUnknownUnit = Qgis.VolumeUnit.Unknown
QgsUnitTypes.VolumeUnknownUnit.is_monkey_patched = True
QgsUnitTypes.VolumeUnknownUnit.__doc__ = "Unknown volume unit"
Qgis.VolumeUnit.__doc__ = "Units of volume.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.VolumeUnit.\n\n.. versionadded:: 3.30\n\n" + '* ``VolumeCubicMeters``: ' + Qgis.VolumeUnit.CubicMeters.__doc__ + '\n' + '* ``VolumeCubicFeet``: ' + Qgis.VolumeUnit.CubicFeet.__doc__ + '\n' + '* ``VolumeCubicYards``: ' + Qgis.VolumeUnit.CubicYards.__doc__ + '\n' + '* ``VolumeBarrel``: ' + Qgis.VolumeUnit.Barrel.__doc__ + '\n' + '* ``VolumeCubicDecimeter``: ' + Qgis.VolumeUnit.CubicDecimeter.__doc__ + '\n' + '* ``VolumeLiters``: ' + Qgis.VolumeUnit.Liters.__doc__ + '\n' + '* ``VolumeGallonUS``: ' + Qgis.VolumeUnit.GallonUS.__doc__ + '\n' + '* ``VolumeCubicInch``: ' + Qgis.VolumeUnit.CubicInch.__doc__ + '\n' + '* ``VolumeCubicCentimeter``: ' + Qgis.VolumeUnit.CubicCentimeter.__doc__ + '\n' + '* ``VolumeCubicDegrees``: ' + Qgis.VolumeUnit.CubicDegrees.__doc__ + '\n' + '* ``VolumeUnknownUnit``: ' + Qgis.VolumeUnit.Unknown.__doc__
# --
Qgis.VolumeUnit.baseClass = Qgis
QgsUnitTypes.AngleUnit = Qgis.AngleUnit
# monkey patching scoped based enum
QgsUnitTypes.AngleDegrees = Qgis.AngleUnit.Degrees
QgsUnitTypes.AngleUnit.AngleDegrees = Qgis.AngleUnit.Degrees
QgsUnitTypes.AngleDegrees.is_monkey_patched = True
QgsUnitTypes.AngleDegrees.__doc__ = "Degrees"
QgsUnitTypes.AngleRadians = Qgis.AngleUnit.Radians
QgsUnitTypes.AngleUnit.AngleRadians = Qgis.AngleUnit.Radians
QgsUnitTypes.AngleRadians.is_monkey_patched = True
QgsUnitTypes.AngleRadians.__doc__ = "Square kilometers"
QgsUnitTypes.AngleGon = Qgis.AngleUnit.Gon
QgsUnitTypes.AngleUnit.AngleGon = Qgis.AngleUnit.Gon
QgsUnitTypes.AngleGon.is_monkey_patched = True
QgsUnitTypes.AngleGon.__doc__ = "Gon/gradian"
QgsUnitTypes.AngleMinutesOfArc = Qgis.AngleUnit.MinutesOfArc
QgsUnitTypes.AngleUnit.AngleMinutesOfArc = Qgis.AngleUnit.MinutesOfArc
QgsUnitTypes.AngleMinutesOfArc.is_monkey_patched = True
QgsUnitTypes.AngleMinutesOfArc.__doc__ = "Minutes of arc"
QgsUnitTypes.AngleSecondsOfArc = Qgis.AngleUnit.SecondsOfArc
QgsUnitTypes.AngleUnit.AngleSecondsOfArc = Qgis.AngleUnit.SecondsOfArc
QgsUnitTypes.AngleSecondsOfArc.is_monkey_patched = True
QgsUnitTypes.AngleSecondsOfArc.__doc__ = "Seconds of arc"
QgsUnitTypes.AngleTurn = Qgis.AngleUnit.Turn
QgsUnitTypes.AngleUnit.AngleTurn = Qgis.AngleUnit.Turn
QgsUnitTypes.AngleTurn.is_monkey_patched = True
QgsUnitTypes.AngleTurn.__doc__ = "Turn/revolutions"
QgsUnitTypes.AngleMilliradiansSI = Qgis.AngleUnit.MilliradiansSI
QgsUnitTypes.AngleUnit.AngleMilliradiansSI = Qgis.AngleUnit.MilliradiansSI
QgsUnitTypes.AngleMilliradiansSI.is_monkey_patched = True
QgsUnitTypes.AngleMilliradiansSI.__doc__ = "Angular milliradians (SI definition, 1/1000 of radian)"
QgsUnitTypes.AngleMilNATO = Qgis.AngleUnit.MilNATO
QgsUnitTypes.AngleUnit.AngleMilNATO = Qgis.AngleUnit.MilNATO
QgsUnitTypes.AngleMilNATO.is_monkey_patched = True
QgsUnitTypes.AngleMilNATO.__doc__ = "Angular mil (NATO definition, 6400 mil = 2PI radians)"
QgsUnitTypes.AngleUnknownUnit = Qgis.AngleUnit.Unknown
QgsUnitTypes.AngleUnit.AngleUnknownUnit = Qgis.AngleUnit.Unknown
QgsUnitTypes.AngleUnknownUnit.is_monkey_patched = True
QgsUnitTypes.AngleUnknownUnit.__doc__ = "Unknown angle unit"
Qgis.AngleUnit.__doc__ = "Units of angles.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.AngleUnit.\n\n.. versionadded:: 3.30\n\n" + '* ``AngleDegrees``: ' + Qgis.AngleUnit.Degrees.__doc__ + '\n' + '* ``AngleRadians``: ' + Qgis.AngleUnit.Radians.__doc__ + '\n' + '* ``AngleGon``: ' + Qgis.AngleUnit.Gon.__doc__ + '\n' + '* ``AngleMinutesOfArc``: ' + Qgis.AngleUnit.MinutesOfArc.__doc__ + '\n' + '* ``AngleSecondsOfArc``: ' + Qgis.AngleUnit.SecondsOfArc.__doc__ + '\n' + '* ``AngleTurn``: ' + Qgis.AngleUnit.Turn.__doc__ + '\n' + '* ``AngleMilliradiansSI``: ' + Qgis.AngleUnit.MilliradiansSI.__doc__ + '\n' + '* ``AngleMilNATO``: ' + Qgis.AngleUnit.MilNATO.__doc__ + '\n' + '* ``AngleUnknownUnit``: ' + Qgis.AngleUnit.Unknown.__doc__
# --
Qgis.AngleUnit.baseClass = Qgis
QgsUnitTypes.TemporalUnit = Qgis.TemporalUnit
# monkey patching scoped based enum
QgsUnitTypes.TemporalMilliseconds = Qgis.TemporalUnit.Milliseconds
QgsUnitTypes.TemporalUnit.TemporalMilliseconds = Qgis.TemporalUnit.Milliseconds
QgsUnitTypes.TemporalMilliseconds.is_monkey_patched = True
QgsUnitTypes.TemporalMilliseconds.__doc__ = "Milliseconds"
QgsUnitTypes.TemporalSeconds = Qgis.TemporalUnit.Seconds
QgsUnitTypes.TemporalUnit.TemporalSeconds = Qgis.TemporalUnit.Seconds
QgsUnitTypes.TemporalSeconds.is_monkey_patched = True
QgsUnitTypes.TemporalSeconds.__doc__ = "Seconds"
QgsUnitTypes.TemporalMinutes = Qgis.TemporalUnit.Minutes
QgsUnitTypes.TemporalUnit.TemporalMinutes = Qgis.TemporalUnit.Minutes
QgsUnitTypes.TemporalMinutes.is_monkey_patched = True
QgsUnitTypes.TemporalMinutes.__doc__ = "Minutes"
QgsUnitTypes.TemporalHours = Qgis.TemporalUnit.Hours
QgsUnitTypes.TemporalUnit.TemporalHours = Qgis.TemporalUnit.Hours
QgsUnitTypes.TemporalHours.is_monkey_patched = True
QgsUnitTypes.TemporalHours.__doc__ = "Hours"
QgsUnitTypes.TemporalDays = Qgis.TemporalUnit.Days
QgsUnitTypes.TemporalUnit.TemporalDays = Qgis.TemporalUnit.Days
QgsUnitTypes.TemporalDays.is_monkey_patched = True
QgsUnitTypes.TemporalDays.__doc__ = "Days"
QgsUnitTypes.TemporalWeeks = Qgis.TemporalUnit.Weeks
QgsUnitTypes.TemporalUnit.TemporalWeeks = Qgis.TemporalUnit.Weeks
QgsUnitTypes.TemporalWeeks.is_monkey_patched = True
QgsUnitTypes.TemporalWeeks.__doc__ = "Weeks"
QgsUnitTypes.TemporalMonths = Qgis.TemporalUnit.Months
QgsUnitTypes.TemporalUnit.TemporalMonths = Qgis.TemporalUnit.Months
QgsUnitTypes.TemporalMonths.is_monkey_patched = True
QgsUnitTypes.TemporalMonths.__doc__ = "Months"
QgsUnitTypes.TemporalYears = Qgis.TemporalUnit.Years
QgsUnitTypes.TemporalUnit.TemporalYears = Qgis.TemporalUnit.Years
QgsUnitTypes.TemporalYears.is_monkey_patched = True
QgsUnitTypes.TemporalYears.__doc__ = "Years"
QgsUnitTypes.TemporalDecades = Qgis.TemporalUnit.Decades
QgsUnitTypes.TemporalUnit.TemporalDecades = Qgis.TemporalUnit.Decades
QgsUnitTypes.TemporalDecades.is_monkey_patched = True
QgsUnitTypes.TemporalDecades.__doc__ = "Decades"
QgsUnitTypes.TemporalCenturies = Qgis.TemporalUnit.Centuries
QgsUnitTypes.TemporalUnit.TemporalCenturies = Qgis.TemporalUnit.Centuries
QgsUnitTypes.TemporalCenturies.is_monkey_patched = True
QgsUnitTypes.TemporalCenturies.__doc__ = "Centuries"
QgsUnitTypes.TemporalIrregularStep = Qgis.TemporalUnit.IrregularStep
QgsUnitTypes.TemporalUnit.TemporalIrregularStep = Qgis.TemporalUnit.IrregularStep
QgsUnitTypes.TemporalIrregularStep.is_monkey_patched = True
QgsUnitTypes.TemporalIrregularStep.__doc__ = "Special 'irregular step' time unit, used for temporal data which uses irregular, non-real-world unit steps (since QGIS 3.20)"
QgsUnitTypes.TemporalUnknownUnit = Qgis.TemporalUnit.Unknown
QgsUnitTypes.TemporalUnit.TemporalUnknownUnit = Qgis.TemporalUnit.Unknown
QgsUnitTypes.TemporalUnknownUnit.is_monkey_patched = True
QgsUnitTypes.TemporalUnknownUnit.__doc__ = "Unknown time unit"
Qgis.TemporalUnit.__doc__ = "Temporal units.\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.TemporalUnit.\n\n.. versionadded:: 3.30\n\n" + '* ``TemporalMilliseconds``: ' + Qgis.TemporalUnit.Milliseconds.__doc__ + '\n' + '* ``TemporalSeconds``: ' + Qgis.TemporalUnit.Seconds.__doc__ + '\n' + '* ``TemporalMinutes``: ' + Qgis.TemporalUnit.Minutes.__doc__ + '\n' + '* ``TemporalHours``: ' + Qgis.TemporalUnit.Hours.__doc__ + '\n' + '* ``TemporalDays``: ' + Qgis.TemporalUnit.Days.__doc__ + '\n' + '* ``TemporalWeeks``: ' + Qgis.TemporalUnit.Weeks.__doc__ + '\n' + '* ``TemporalMonths``: ' + Qgis.TemporalUnit.Months.__doc__ + '\n' + '* ``TemporalYears``: ' + Qgis.TemporalUnit.Years.__doc__ + '\n' + '* ``TemporalDecades``: ' + Qgis.TemporalUnit.Decades.__doc__ + '\n' + '* ``TemporalCenturies``: ' + Qgis.TemporalUnit.Centuries.__doc__ + '\n' + '* ``TemporalIrregularStep``: ' + Qgis.TemporalUnit.IrregularStep.__doc__ + '\n' + '* ``TemporalUnknownUnit``: ' + Qgis.TemporalUnit.Unknown.__doc__
# --
Qgis.TemporalUnit.baseClass = Qgis
QgsUnitTypes.RenderUnit = Qgis.RenderUnit
# monkey patching scoped based enum
QgsUnitTypes.RenderMillimeters = Qgis.RenderUnit.Millimeters
QgsUnitTypes.RenderUnit.RenderMillimeters = Qgis.RenderUnit.Millimeters
QgsUnitTypes.RenderMillimeters.is_monkey_patched = True
QgsUnitTypes.RenderMillimeters.__doc__ = "Millimeters"
QgsUnitTypes.RenderMapUnits = Qgis.RenderUnit.MapUnits
QgsUnitTypes.RenderUnit.RenderMapUnits = Qgis.RenderUnit.MapUnits
QgsUnitTypes.RenderMapUnits.is_monkey_patched = True
QgsUnitTypes.RenderMapUnits.__doc__ = "Map units"
QgsUnitTypes.RenderPixels = Qgis.RenderUnit.Pixels
QgsUnitTypes.RenderUnit.RenderPixels = Qgis.RenderUnit.Pixels
QgsUnitTypes.RenderPixels.is_monkey_patched = True
QgsUnitTypes.RenderPixels.__doc__ = "Pixels"
QgsUnitTypes.RenderPercentage = Qgis.RenderUnit.Percentage
QgsUnitTypes.RenderUnit.RenderPercentage = Qgis.RenderUnit.Percentage
QgsUnitTypes.RenderPercentage.is_monkey_patched = True
QgsUnitTypes.RenderPercentage.__doc__ = "Percentage of another measurement (e.g., canvas size, feature size)"
QgsUnitTypes.RenderPoints = Qgis.RenderUnit.Points
QgsUnitTypes.RenderUnit.RenderPoints = Qgis.RenderUnit.Points
QgsUnitTypes.RenderPoints.is_monkey_patched = True
QgsUnitTypes.RenderPoints.__doc__ = "Points (e.g., for font sizes)"
QgsUnitTypes.RenderInches = Qgis.RenderUnit.Inches
QgsUnitTypes.RenderUnit.RenderInches = Qgis.RenderUnit.Inches
QgsUnitTypes.RenderInches.is_monkey_patched = True
QgsUnitTypes.RenderInches.__doc__ = "Inches"
QgsUnitTypes.RenderUnknownUnit = Qgis.RenderUnit.Unknown
QgsUnitTypes.RenderUnit.RenderUnknownUnit = Qgis.RenderUnit.Unknown
QgsUnitTypes.RenderUnknownUnit.is_monkey_patched = True
QgsUnitTypes.RenderUnknownUnit.__doc__ = "Mixed or unknown units"
QgsUnitTypes.RenderMetersInMapUnits = Qgis.RenderUnit.MetersInMapUnits
QgsUnitTypes.RenderUnit.RenderMetersInMapUnits = Qgis.RenderUnit.MetersInMapUnits
QgsUnitTypes.RenderMetersInMapUnits.is_monkey_patched = True
QgsUnitTypes.RenderMetersInMapUnits.__doc__ = "Meters value as Map units"
Qgis.RenderUnit.__doc__ = "Rendering size units\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.RenderUnit.\n\n.. versionadded:: 3.30\n\n" + '* ``RenderMillimeters``: ' + Qgis.RenderUnit.Millimeters.__doc__ + '\n' + '* ``RenderMapUnits``: ' + Qgis.RenderUnit.MapUnits.__doc__ + '\n' + '* ``RenderPixels``: ' + Qgis.RenderUnit.Pixels.__doc__ + '\n' + '* ``RenderPercentage``: ' + Qgis.RenderUnit.Percentage.__doc__ + '\n' + '* ``RenderPoints``: ' + Qgis.RenderUnit.Points.__doc__ + '\n' + '* ``RenderInches``: ' + Qgis.RenderUnit.Inches.__doc__ + '\n' + '* ``RenderUnknownUnit``: ' + Qgis.RenderUnit.Unknown.__doc__ + '\n' + '* ``RenderMetersInMapUnits``: ' + Qgis.RenderUnit.MetersInMapUnits.__doc__
# --
Qgis.RenderUnit.baseClass = Qgis
QgsUnitTypes.LayoutUnit = Qgis.LayoutUnit
# monkey patching scoped based enum
QgsUnitTypes.LayoutMillimeters = Qgis.LayoutUnit.Millimeters
QgsUnitTypes.LayoutUnit.LayoutMillimeters = Qgis.LayoutUnit.Millimeters
QgsUnitTypes.LayoutMillimeters.is_monkey_patched = True
QgsUnitTypes.LayoutMillimeters.__doc__ = "Millimeters"
QgsUnitTypes.LayoutCentimeters = Qgis.LayoutUnit.Centimeters
QgsUnitTypes.LayoutUnit.LayoutCentimeters = Qgis.LayoutUnit.Centimeters
QgsUnitTypes.LayoutCentimeters.is_monkey_patched = True
QgsUnitTypes.LayoutCentimeters.__doc__ = "Centimeters"
QgsUnitTypes.LayoutMeters = Qgis.LayoutUnit.Meters
QgsUnitTypes.LayoutUnit.LayoutMeters = Qgis.LayoutUnit.Meters
QgsUnitTypes.LayoutMeters.is_monkey_patched = True
QgsUnitTypes.LayoutMeters.__doc__ = "Meters"
QgsUnitTypes.LayoutInches = Qgis.LayoutUnit.Inches
QgsUnitTypes.LayoutUnit.LayoutInches = Qgis.LayoutUnit.Inches
QgsUnitTypes.LayoutInches.is_monkey_patched = True
QgsUnitTypes.LayoutInches.__doc__ = "Inches"
QgsUnitTypes.LayoutFeet = Qgis.LayoutUnit.Feet
QgsUnitTypes.LayoutUnit.LayoutFeet = Qgis.LayoutUnit.Feet
QgsUnitTypes.LayoutFeet.is_monkey_patched = True
QgsUnitTypes.LayoutFeet.__doc__ = "Feet"
QgsUnitTypes.LayoutPoints = Qgis.LayoutUnit.Points
QgsUnitTypes.LayoutUnit.LayoutPoints = Qgis.LayoutUnit.Points
QgsUnitTypes.LayoutPoints.is_monkey_patched = True
QgsUnitTypes.LayoutPoints.__doc__ = "Typographic points"
QgsUnitTypes.LayoutPicas = Qgis.LayoutUnit.Picas
QgsUnitTypes.LayoutUnit.LayoutPicas = Qgis.LayoutUnit.Picas
QgsUnitTypes.LayoutPicas.is_monkey_patched = True
QgsUnitTypes.LayoutPicas.__doc__ = "Typographic picas"
QgsUnitTypes.LayoutPixels = Qgis.LayoutUnit.Pixels
QgsUnitTypes.LayoutUnit.LayoutPixels = Qgis.LayoutUnit.Pixels
QgsUnitTypes.LayoutPixels.is_monkey_patched = True
QgsUnitTypes.LayoutPixels.__doc__ = "Pixels"
Qgis.LayoutUnit.__doc__ = "Layout measurement units\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.LayoutUnit.\n\n.. versionadded:: 3.30\n\n" + '* ``LayoutMillimeters``: ' + Qgis.LayoutUnit.Millimeters.__doc__ + '\n' + '* ``LayoutCentimeters``: ' + Qgis.LayoutUnit.Centimeters.__doc__ + '\n' + '* ``LayoutMeters``: ' + Qgis.LayoutUnit.Meters.__doc__ + '\n' + '* ``LayoutInches``: ' + Qgis.LayoutUnit.Inches.__doc__ + '\n' + '* ``LayoutFeet``: ' + Qgis.LayoutUnit.Feet.__doc__ + '\n' + '* ``LayoutPoints``: ' + Qgis.LayoutUnit.Points.__doc__ + '\n' + '* ``LayoutPicas``: ' + Qgis.LayoutUnit.Picas.__doc__ + '\n' + '* ``LayoutPixels``: ' + Qgis.LayoutUnit.Pixels.__doc__
# --
Qgis.LayoutUnit.baseClass = Qgis
QgsUnitTypes.LayoutUnitType = Qgis.LayoutUnitType
# monkey patching scoped based enum
QgsUnitTypes.LayoutPaperUnits = Qgis.LayoutUnitType.PaperUnits
QgsUnitTypes.LayoutUnitType.LayoutPaperUnits = Qgis.LayoutUnitType.PaperUnits
QgsUnitTypes.LayoutPaperUnits.is_monkey_patched = True
QgsUnitTypes.LayoutPaperUnits.__doc__ = "Unit is a paper based measurement unit"
QgsUnitTypes.LayoutScreenUnits = Qgis.LayoutUnitType.ScreenUnits
QgsUnitTypes.LayoutUnitType.LayoutScreenUnits = Qgis.LayoutUnitType.ScreenUnits
QgsUnitTypes.LayoutScreenUnits.is_monkey_patched = True
QgsUnitTypes.LayoutScreenUnits.__doc__ = "Unit is a screen based measurement unit"
Qgis.LayoutUnitType.__doc__ = "Types of layout units\n\n.. note::\n\n   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.LayoutUnitType.\n\n.. versionadded:: 3.30\n\n" + '* ``LayoutPaperUnits``: ' + Qgis.LayoutUnitType.PaperUnits.__doc__ + '\n' + '* ``LayoutScreenUnits``: ' + Qgis.LayoutUnitType.ScreenUnits.__doc__
# --
Qgis.LayoutUnitType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.InputControllerType.Map2D.__doc__ = "2D map controller"
Qgis.InputControllerType.Map3D.__doc__ = "3D map controller"
Qgis.InputControllerType.__doc__ = "Input controller types.\n\n.. versionadded:: 3.34\n\n" + '* ``Map2D``: ' + Qgis.InputControllerType.Map2D.__doc__ + '\n' + '* ``Map3D``: ' + Qgis.InputControllerType.Map3D.__doc__
# --
Qgis.InputControllerType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.PostgresRelKind.NotSet.__doc__ = "Not set"
Qgis.PostgresRelKind.Unknown.__doc__ = "Unknown"
Qgis.PostgresRelKind.OrdinaryTable.__doc__ = "Ordinary table"
Qgis.PostgresRelKind.Index.__doc__ = "Index"
Qgis.PostgresRelKind.Sequence.__doc__ = "Sequence"
Qgis.PostgresRelKind.View.__doc__ = "View"
Qgis.PostgresRelKind.MaterializedView.__doc__ = "Materialized view"
Qgis.PostgresRelKind.CompositeType.__doc__ = "Composition type"
Qgis.PostgresRelKind.ToastTable.__doc__ = "TOAST table"
Qgis.PostgresRelKind.ForeignTable.__doc__ = "Foreign table"
Qgis.PostgresRelKind.PartitionedTable.__doc__ = "Partitioned table"
Qgis.PostgresRelKind.__doc__ = "Postgres database relkind options.\n\n.. versionadded:: 3.32\n\n" + '* ``NotSet``: ' + Qgis.PostgresRelKind.NotSet.__doc__ + '\n' + '* ``Unknown``: ' + Qgis.PostgresRelKind.Unknown.__doc__ + '\n' + '* ``OrdinaryTable``: ' + Qgis.PostgresRelKind.OrdinaryTable.__doc__ + '\n' + '* ``Index``: ' + Qgis.PostgresRelKind.Index.__doc__ + '\n' + '* ``Sequence``: ' + Qgis.PostgresRelKind.Sequence.__doc__ + '\n' + '* ``View``: ' + Qgis.PostgresRelKind.View.__doc__ + '\n' + '* ``MaterializedView``: ' + Qgis.PostgresRelKind.MaterializedView.__doc__ + '\n' + '* ``CompositeType``: ' + Qgis.PostgresRelKind.CompositeType.__doc__ + '\n' + '* ``ToastTable``: ' + Qgis.PostgresRelKind.ToastTable.__doc__ + '\n' + '* ``ForeignTable``: ' + Qgis.PostgresRelKind.ForeignTable.__doc__ + '\n' + '* ``PartitionedTable``: ' + Qgis.PostgresRelKind.PartitionedTable.__doc__
# --
Qgis.PostgresRelKind.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DatabaseProviderConnectionCapability2.SetFieldComment.__doc__ = "Can set comments for fields via setFieldComment()"
Qgis.DatabaseProviderConnectionCapability2.SetFieldAlias.__doc__ = "Can set aliases for fields via setFieldAlias()"
Qgis.DatabaseProviderConnectionCapability2.__doc__ = "The Capability enum represents the extended operations supported by the connection.\n\n.. versionadded:: 3.32\n\n" + '* ``SetFieldComment``: ' + Qgis.DatabaseProviderConnectionCapability2.SetFieldComment.__doc__ + '\n' + '* ``SetFieldAlias``: ' + Qgis.DatabaseProviderConnectionCapability2.SetFieldAlias.__doc__
# --
Qgis.DatabaseProviderConnectionCapability2.baseClass = Qgis
Qgis.DatabaseProviderConnectionCapabilities2 = lambda flags=0: Qgis.DatabaseProviderConnectionCapability2(flags)
Qgis.DatabaseProviderConnectionCapabilities2.baseClass = Qgis
DatabaseProviderConnectionCapabilities2 = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ProviderStyleStorageCapability.SaveToDatabase.__doc__ = ""
Qgis.ProviderStyleStorageCapability.LoadFromDatabase.__doc__ = ""
Qgis.ProviderStyleStorageCapability.DeleteFromDatabase.__doc__ = ""
Qgis.ProviderStyleStorageCapability.__doc__ = "The StorageCapability enum represents the style storage operations supported by the provider.\n\n.. versionadded:: 3.34\n\n" + '* ``SaveToDatabase``: ' + Qgis.ProviderStyleStorageCapability.SaveToDatabase.__doc__ + '\n' + '* ``LoadFromDatabase``: ' + Qgis.ProviderStyleStorageCapability.LoadFromDatabase.__doc__ + '\n' + '* ``DeleteFromDatabase``: ' + Qgis.ProviderStyleStorageCapability.DeleteFromDatabase.__doc__
# --
Qgis.ProviderStyleStorageCapability.baseClass = Qgis
Qgis.ProviderStyleStorageCapabilities = lambda flags=0: Qgis.ProviderStyleStorageCapability(flags)
Qgis.ProviderStyleStorageCapabilities.baseClass = Qgis
ProviderStyleStorageCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.UserProfileSelectionPolicy.LastProfile.__doc__ = "Open the last closed profile (only mode supported prior to QGIS 3.32)"
Qgis.UserProfileSelectionPolicy.DefaultProfile.__doc__ = "Open a specific profile"
Qgis.UserProfileSelectionPolicy.AskUser.__doc__ = "Let the user choose which profile to open"
Qgis.UserProfileSelectionPolicy.__doc__ = "User profile selection policy.\n\n.. versionadded:: 3.32\n\n" + '* ``LastProfile``: ' + Qgis.UserProfileSelectionPolicy.LastProfile.__doc__ + '\n' + '* ``DefaultProfile``: ' + Qgis.UserProfileSelectionPolicy.DefaultProfile.__doc__ + '\n' + '* ``AskUser``: ' + Qgis.UserProfileSelectionPolicy.AskUser.__doc__
# --
Qgis.UserProfileSelectionPolicy.baseClass = Qgis
QgsAttributeEditorElement.AttributeEditorType = Qgis.AttributeEditorType
# monkey patching scoped based enum
QgsAttributeEditorElement.AeTypeContainer = Qgis.AttributeEditorType.Container
QgsAttributeEditorElement.AttributeEditorType.AeTypeContainer = Qgis.AttributeEditorType.Container
QgsAttributeEditorElement.AeTypeContainer.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeContainer.__doc__ = "A container"
QgsAttributeEditorElement.AeTypeField = Qgis.AttributeEditorType.Field
QgsAttributeEditorElement.AttributeEditorType.AeTypeField = Qgis.AttributeEditorType.Field
QgsAttributeEditorElement.AeTypeField.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeField.__doc__ = "A field"
QgsAttributeEditorElement.AeTypeRelation = Qgis.AttributeEditorType.Relation
QgsAttributeEditorElement.AttributeEditorType.AeTypeRelation = Qgis.AttributeEditorType.Relation
QgsAttributeEditorElement.AeTypeRelation.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeRelation.__doc__ = "A relation"
QgsAttributeEditorElement.AeTypeQmlElement = Qgis.AttributeEditorType.QmlElement
QgsAttributeEditorElement.AttributeEditorType.AeTypeQmlElement = Qgis.AttributeEditorType.QmlElement
QgsAttributeEditorElement.AeTypeQmlElement.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeQmlElement.__doc__ = "A QML element"
QgsAttributeEditorElement.AeTypeHtmlElement = Qgis.AttributeEditorType.HtmlElement
QgsAttributeEditorElement.AttributeEditorType.AeTypeHtmlElement = Qgis.AttributeEditorType.HtmlElement
QgsAttributeEditorElement.AeTypeHtmlElement.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeHtmlElement.__doc__ = "A HTML element"
QgsAttributeEditorElement.AeTypeAction = Qgis.AttributeEditorType.Action
QgsAttributeEditorElement.AttributeEditorType.AeTypeAction = Qgis.AttributeEditorType.Action
QgsAttributeEditorElement.AeTypeAction.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeAction.__doc__ = "A layer action element (since QGIS 3.22)"
QgsAttributeEditorElement.AeTypeTextElement = Qgis.AttributeEditorType.TextElement
QgsAttributeEditorElement.AttributeEditorType.AeTypeTextElement = Qgis.AttributeEditorType.TextElement
QgsAttributeEditorElement.AeTypeTextElement.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeTextElement.__doc__ = "A text element (since QGIS 3.30)"
QgsAttributeEditorElement.AeTypeSpacerElement = Qgis.AttributeEditorType.SpacerElement
QgsAttributeEditorElement.AttributeEditorType.AeTypeSpacerElement = Qgis.AttributeEditorType.SpacerElement
QgsAttributeEditorElement.AeTypeSpacerElement.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeSpacerElement.__doc__ = "A spacer element (since QGIS 3.30)"
QgsAttributeEditorElement.AeTypeInvalid = Qgis.AttributeEditorType.Invalid
QgsAttributeEditorElement.AttributeEditorType.AeTypeInvalid = Qgis.AttributeEditorType.Invalid
QgsAttributeEditorElement.AeTypeInvalid.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeInvalid.__doc__ = "Invalid"
Qgis.AttributeEditorType.__doc__ = "Attribute editor types.\n\n.. note::\n\n   Prior to QGIS 3.32 this was available as :py:class:`QgsAttributeEditorElement`.AttributeEditorType.\n\n.. versionadded:: 3.32\n\n" + '* ``AeTypeContainer``: ' + Qgis.AttributeEditorType.Container.__doc__ + '\n' + '* ``AeTypeField``: ' + Qgis.AttributeEditorType.Field.__doc__ + '\n' + '* ``AeTypeRelation``: ' + Qgis.AttributeEditorType.Relation.__doc__ + '\n' + '* ``AeTypeQmlElement``: ' + Qgis.AttributeEditorType.QmlElement.__doc__ + '\n' + '* ``AeTypeHtmlElement``: ' + Qgis.AttributeEditorType.HtmlElement.__doc__ + '\n' + '* ``AeTypeAction``: ' + Qgis.AttributeEditorType.Action.__doc__ + '\n' + '* ``AeTypeTextElement``: ' + Qgis.AttributeEditorType.TextElement.__doc__ + '\n' + '* ``AeTypeSpacerElement``: ' + Qgis.AttributeEditorType.SpacerElement.__doc__ + '\n' + '* ``AeTypeInvalid``: ' + Qgis.AttributeEditorType.Invalid.__doc__
# --
Qgis.AttributeEditorType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AttributeEditorContainerType.GroupBox.__doc__ = "A group box"
Qgis.AttributeEditorContainerType.Tab.__doc__ = "A tab widget"
Qgis.AttributeEditorContainerType.Row.__doc__ = "A row of editors (horizontal layout)"
Qgis.AttributeEditorContainerType.__doc__ = "Attribute editor container types.\n\n.. versionadded:: 3.32\n\n" + '* ``GroupBox``: ' + Qgis.AttributeEditorContainerType.GroupBox.__doc__ + '\n' + '* ``Tab``: ' + Qgis.AttributeEditorContainerType.Tab.__doc__ + '\n' + '* ``Row``: ' + Qgis.AttributeEditorContainerType.Row.__doc__
# --
Qgis.AttributeEditorContainerType.baseClass = Qgis
QgsEditFormConfig.EditorLayout = Qgis.AttributeFormLayout
# monkey patching scoped based enum
QgsEditFormConfig.GeneratedLayout = Qgis.AttributeFormLayout.AutoGenerated
QgsEditFormConfig.EditorLayout.GeneratedLayout = Qgis.AttributeFormLayout.AutoGenerated
QgsEditFormConfig.GeneratedLayout.is_monkey_patched = True
QgsEditFormConfig.GeneratedLayout.__doc__ = "Autogenerate a simple tabular layout for the form"
QgsEditFormConfig.TabLayout = Qgis.AttributeFormLayout.DragAndDrop
QgsEditFormConfig.EditorLayout.TabLayout = Qgis.AttributeFormLayout.DragAndDrop
QgsEditFormConfig.TabLayout.is_monkey_patched = True
QgsEditFormConfig.TabLayout.__doc__ = "\"Drag and drop\" layout. Needs to be configured."
QgsEditFormConfig.UiFileLayout = Qgis.AttributeFormLayout.UiFile
QgsEditFormConfig.EditorLayout.UiFileLayout = Qgis.AttributeFormLayout.UiFile
QgsEditFormConfig.UiFileLayout.is_monkey_patched = True
QgsEditFormConfig.UiFileLayout.__doc__ = "Load a .ui file for the layout. Needs to be configured."
Qgis.AttributeFormLayout.__doc__ = "Available form types for layout of the attribute form editor.\n\n.. note::\n\n   Prior to QGIS 3.32 this was available as :py:class:`QgsEditFormConfig`.EditorLayout.\n\n.. versionadded:: 3.32\n\n" + '* ``GeneratedLayout``: ' + Qgis.AttributeFormLayout.AutoGenerated.__doc__ + '\n' + '* ``TabLayout``: ' + Qgis.AttributeFormLayout.DragAndDrop.__doc__ + '\n' + '* ``UiFileLayout``: ' + Qgis.AttributeFormLayout.UiFile.__doc__
# --
Qgis.AttributeFormLayout.baseClass = Qgis
QgsEditFormConfig.FeatureFormSuppress = Qgis.AttributeFormSuppression
# monkey patching scoped based enum
QgsEditFormConfig.SuppressDefault = Qgis.AttributeFormSuppression.Default
QgsEditFormConfig.FeatureFormSuppress.SuppressDefault = Qgis.AttributeFormSuppression.Default
QgsEditFormConfig.SuppressDefault.is_monkey_patched = True
QgsEditFormConfig.SuppressDefault.__doc__ = "Use the application-wide setting."
QgsEditFormConfig.SuppressOn = Qgis.AttributeFormSuppression.On
QgsEditFormConfig.FeatureFormSuppress.SuppressOn = Qgis.AttributeFormSuppression.On
QgsEditFormConfig.SuppressOn.is_monkey_patched = True
QgsEditFormConfig.SuppressOn.__doc__ = "Always suppress feature form."
QgsEditFormConfig.SuppressOff = Qgis.AttributeFormSuppression.Off
QgsEditFormConfig.FeatureFormSuppress.SuppressOff = Qgis.AttributeFormSuppression.Off
QgsEditFormConfig.SuppressOff.is_monkey_patched = True
QgsEditFormConfig.SuppressOff.__doc__ = "Never suppress feature form."
Qgis.AttributeFormSuppression.__doc__ = "Available form types for layout of the attribute form editor.\n\n.. note::\n\n   Prior to QGIS 3.32 this was available as :py:class:`QgsEditFormConfig`.FeatureFormSuppress.\n\n.. versionadded:: 3.32\n\n" + '* ``SuppressDefault``: ' + Qgis.AttributeFormSuppression.Default.__doc__ + '\n' + '* ``SuppressOn``: ' + Qgis.AttributeFormSuppression.On.__doc__ + '\n' + '* ``SuppressOff``: ' + Qgis.AttributeFormSuppression.Off.__doc__
# --
Qgis.AttributeFormSuppression.baseClass = Qgis
QgsEditFormConfig.PythonInitCodeSource = Qgis.AttributeFormPythonInitCodeSource
# monkey patching scoped based enum
QgsEditFormConfig.CodeSourceNone = Qgis.AttributeFormPythonInitCodeSource.NoSource
QgsEditFormConfig.PythonInitCodeSource.CodeSourceNone = Qgis.AttributeFormPythonInitCodeSource.NoSource
QgsEditFormConfig.CodeSourceNone.is_monkey_patched = True
QgsEditFormConfig.CodeSourceNone.__doc__ = "Do not use Python code at all"
QgsEditFormConfig.CodeSourceFile = Qgis.AttributeFormPythonInitCodeSource.File
QgsEditFormConfig.PythonInitCodeSource.CodeSourceFile = Qgis.AttributeFormPythonInitCodeSource.File
QgsEditFormConfig.CodeSourceFile.is_monkey_patched = True
QgsEditFormConfig.CodeSourceFile.__doc__ = "Load the Python code from an external file"
QgsEditFormConfig.CodeSourceDialog = Qgis.AttributeFormPythonInitCodeSource.Dialog
QgsEditFormConfig.PythonInitCodeSource.CodeSourceDialog = Qgis.AttributeFormPythonInitCodeSource.Dialog
QgsEditFormConfig.CodeSourceDialog.is_monkey_patched = True
QgsEditFormConfig.CodeSourceDialog.__doc__ = "Use the Python code provided in the dialog"
QgsEditFormConfig.CodeSourceEnvironment = Qgis.AttributeFormPythonInitCodeSource.Environment
QgsEditFormConfig.PythonInitCodeSource.CodeSourceEnvironment = Qgis.AttributeFormPythonInitCodeSource.Environment
QgsEditFormConfig.CodeSourceEnvironment.is_monkey_patched = True
QgsEditFormConfig.CodeSourceEnvironment.__doc__ = "Use the Python code available in the Python environment"
Qgis.AttributeFormPythonInitCodeSource.__doc__ = "The Python init code source for attribute forms.\n\n.. note::\n\n   Prior to QGIS 3.32 this was available as :py:class:`QgsEditFormConfig`.PythonInitCodeSource.\n\n.. versionadded:: 3.32\n\n" + '* ``CodeSourceNone``: ' + Qgis.AttributeFormPythonInitCodeSource.NoSource.__doc__ + '\n' + '* ``CodeSourceFile``: ' + Qgis.AttributeFormPythonInitCodeSource.File.__doc__ + '\n' + '* ``CodeSourceDialog``: ' + Qgis.AttributeFormPythonInitCodeSource.Dialog.__doc__ + '\n' + '* ``CodeSourceEnvironment``: ' + Qgis.AttributeFormPythonInitCodeSource.Environment.__doc__
# --
Qgis.AttributeFormPythonInitCodeSource.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ExpressionType.Qgis.__doc__ = "Native QGIS expression"
Qgis.ExpressionType.PointCloud.__doc__ = "Point cloud expression"
Qgis.ExpressionType.RasterCalculator.__doc__ = "Raster calculator expression (since QGIS 3.34)"
Qgis.ExpressionType.__doc__ = "Expression types\n\n.. versionadded:: 3.32\n\n" + '* ``Qgis``: ' + Qgis.ExpressionType.Qgis.__doc__ + '\n' + '* ``PointCloud``: ' + Qgis.ExpressionType.PointCloud.__doc__ + '\n' + '* ``RasterCalculator``: ' + Qgis.ExpressionType.RasterCalculator.__doc__
# --
Qgis.ExpressionType.baseClass = Qgis
QgsVectorFileWriter.SymbologyExport = Qgis.FeatureSymbologyExport
# monkey patching scoped based enum
QgsVectorFileWriter.NoSymbology = Qgis.FeatureSymbologyExport.NoSymbology
QgsVectorFileWriter.NoSymbology.is_monkey_patched = True
QgsVectorFileWriter.NoSymbology.__doc__ = "Export only data"
QgsVectorFileWriter.FeatureSymbology = Qgis.FeatureSymbologyExport.PerFeature
QgsVectorFileWriter.SymbologyExport.FeatureSymbology = Qgis.FeatureSymbologyExport.PerFeature
QgsVectorFileWriter.FeatureSymbology.is_monkey_patched = True
QgsVectorFileWriter.FeatureSymbology.__doc__ = "Keeps the number of features and export symbology per feature"
QgsVectorFileWriter.SymbolLayerSymbology = Qgis.FeatureSymbologyExport.PerSymbolLayer
QgsVectorFileWriter.SymbologyExport.SymbolLayerSymbology = Qgis.FeatureSymbologyExport.PerSymbolLayer
QgsVectorFileWriter.SymbolLayerSymbology.is_monkey_patched = True
QgsVectorFileWriter.SymbolLayerSymbology.__doc__ = "Exports one feature per symbol layer (considering symbol levels)"
Qgis.FeatureSymbologyExport.__doc__ = "Options for exporting features considering their symbology.\n\n.. note::\n\n   Prior to QGIS 3.32 this was available as :py:class:`QgsVectorFileWriter`.SymbologyExport.\n\n.. versionadded:: 3.32\n\n" + '* ``NoSymbology``: ' + Qgis.FeatureSymbologyExport.NoSymbology.__doc__ + '\n' + '* ``FeatureSymbology``: ' + Qgis.FeatureSymbologyExport.PerFeature.__doc__ + '\n' + '* ``SymbolLayerSymbology``: ' + Qgis.FeatureSymbologyExport.PerSymbolLayer.__doc__
# --
Qgis.FeatureSymbologyExport.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorTileProviderFlag.AlwaysUseTileMatrixSetFromProvider.__doc__ = "Vector tile layer must always use the tile matrix set from the data provider, and should never store, restore or override the definition of this matrix set."
Qgis.VectorTileProviderFlag.__doc__ = "Flags for vector tile data providers.\n\n.. versionadded:: 3.32\n\n" + '* ``AlwaysUseTileMatrixSetFromProvider``: ' + Qgis.VectorTileProviderFlag.AlwaysUseTileMatrixSetFromProvider.__doc__
# --
Qgis.VectorTileProviderFlag.baseClass = Qgis
Qgis.VectorTileProviderFlags = lambda flags=0: Qgis.VectorTileProviderFlag(flags)
Qgis.VectorTileProviderFlags.baseClass = Qgis
VectorTileProviderFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.VectorTileProviderCapability.ReadLayerMetadata.__doc__ = "Provider can read layer metadata from data store. See QgsDataProvider.layerMetadata()"
Qgis.VectorTileProviderCapability.__doc__ = "Enumeration with capabilities that vector tile data providers might implement.\n\n.. versionadded:: 3.32\n\n" + '* ``ReadLayerMetadata``: ' + Qgis.VectorTileProviderCapability.ReadLayerMetadata.__doc__
# --
Qgis.VectorTileProviderCapability.baseClass = Qgis
Qgis.VectorTileProviderCapabilities = lambda flags=0: Qgis.VectorTileProviderCapability(flags)
Qgis.VectorTileProviderCapabilities.baseClass = Qgis
VectorTileProviderCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.TileAvailability.Available.__doc__ = "Tile is available within the matrix"
Qgis.TileAvailability.NotAvailable.__doc__ = "Tile is not available within the matrix, e.g. there is no content for the tile"
Qgis.TileAvailability.AvailableNoChildren.__doc__ = "Tile is available within the matrix, and is known to have no children (ie no higher zoom level tiles exist covering this tile's region)"
Qgis.TileAvailability.UseLowerZoomLevelTile.__doc__ = "Tile is not available at the requested zoom level, it should be replaced by a tile from a lower zoom level instead182"
Qgis.TileAvailability.__doc__ = "Possible availability states for a tile within a tile matrix.\n\n.. versionadded:: 3.32\n\n" + '* ``Available``: ' + Qgis.TileAvailability.Available.__doc__ + '\n' + '* ``NotAvailable``: ' + Qgis.TileAvailability.NotAvailable.__doc__ + '\n' + '* ``AvailableNoChildren``: ' + Qgis.TileAvailability.AvailableNoChildren.__doc__ + '\n' + '* ``UseLowerZoomLevelTile``: ' + Qgis.TileAvailability.UseLowerZoomLevelTile.__doc__
# --
Qgis.TileAvailability.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TiledSceneProviderCapability.ReadLayerMetadata.__doc__ = "Provider can read layer metadata from data store. See QgsDataProvider.layerMetadata()"
Qgis.TiledSceneProviderCapability.__doc__ = "Tiled scene data provider capabilities.\n\n.. versionadded:: 3.34\n\n" + '* ``ReadLayerMetadata``: ' + Qgis.TiledSceneProviderCapability.ReadLayerMetadata.__doc__
# --
Qgis.TiledSceneProviderCapability.baseClass = Qgis
Qgis.TiledSceneProviderCapabilities = lambda flags=0: Qgis.TiledSceneProviderCapability(flags)
Qgis.TiledSceneProviderCapabilities.baseClass = Qgis
TiledSceneProviderCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.TiledSceneBoundingVolumeType.Region.__doc__ = "Region type"
Qgis.TiledSceneBoundingVolumeType.OrientedBox.__doc__ = "Oriented bounding box (rotated box)"
Qgis.TiledSceneBoundingVolumeType.Sphere.__doc__ = "Sphere"
Qgis.TiledSceneBoundingVolumeType.__doc__ = "Tiled scene bounding volume types.\n\n.. versionadded:: 3.34\n\n" + '* ``Region``: ' + Qgis.TiledSceneBoundingVolumeType.Region.__doc__ + '\n' + '* ``OrientedBox``: ' + Qgis.TiledSceneBoundingVolumeType.OrientedBox.__doc__ + '\n' + '* ``Sphere``: ' + Qgis.TiledSceneBoundingVolumeType.Sphere.__doc__
# --
Qgis.TiledSceneBoundingVolumeType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TileRefinementProcess.Replacement.__doc__ = "When tile is refined then its children should be used in place of itself."
Qgis.TileRefinementProcess.Additive.__doc__ = "When tile is refined its content should be used alongside its children simultaneously."
Qgis.TileRefinementProcess.__doc__ = "Tiled scene tile refinement processes.\n\nRefinement determines the process by which a lower resolution parent tile\nrenders when its higher resolution children are selected to be rendered.\n\n.. versionadded:: 3.34\n\n" + '* ``Replacement``: ' + Qgis.TileRefinementProcess.Replacement.__doc__ + '\n' + '* ``Additive``: ' + Qgis.TileRefinementProcess.Additive.__doc__
# --
Qgis.TileRefinementProcess.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TileChildrenAvailability.NoChildren.__doc__ = "Tile is known to have no children"
Qgis.TileChildrenAvailability.Available.__doc__ = "Tile children are already available"
Qgis.TileChildrenAvailability.NeedFetching.__doc__ = "Tile has children, but they are not yet available and must be fetched"
Qgis.TileChildrenAvailability.__doc__ = "Possible availability states for a tile's children.\n\n.. versionadded:: 3.34\n\n" + '* ``NoChildren``: ' + Qgis.TileChildrenAvailability.NoChildren.__doc__ + '\n' + '* ``Available``: ' + Qgis.TileChildrenAvailability.Available.__doc__ + '\n' + '* ``NeedFetching``: ' + Qgis.TileChildrenAvailability.NeedFetching.__doc__
# --
Qgis.TileChildrenAvailability.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TiledSceneRequestFlag.NoHierarchyFetch.__doc__ = "Do not allow hierarchy fetching when hierarchy is not currently available. Avoids network requests, but may result in an incomplete tile set. If set, then callers will need to manually perform hierarchy fetches as required."
Qgis.TiledSceneRequestFlag.__doc__ = "Flags which control how tiled scene requests behave.\n\n.. versionadded:: 3.34\n\n" + '* ``NoHierarchyFetch``: ' + Qgis.TiledSceneRequestFlag.NoHierarchyFetch.__doc__
# --
Qgis.TiledSceneRequestFlag.baseClass = Qgis
Qgis.TiledSceneRequestFlags = lambda flags=0: Qgis.TiledSceneRequestFlag(flags)
Qgis.TiledSceneRequestFlags.baseClass = Qgis
TiledSceneRequestFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.TiledSceneRendererFlag.RequiresTextures.__doc__ = "Renderer requires textures"
Qgis.TiledSceneRendererFlag.ForceRasterRender.__doc__ = "Layer should always be rendered as a raster image"
Qgis.TiledSceneRendererFlag.RendersTriangles.__doc__ = "Renderer can render triangle primitives"
Qgis.TiledSceneRendererFlag.RendersLines.__doc__ = "Renderer can render line primitives"
Qgis.TiledSceneRendererFlag.__doc__ = "Flags which control how tiled scene 2D renderers behave.\n\n.. versionadded:: 3.34\n\n" + '* ``RequiresTextures``: ' + Qgis.TiledSceneRendererFlag.RequiresTextures.__doc__ + '\n' + '* ``ForceRasterRender``: ' + Qgis.TiledSceneRendererFlag.ForceRasterRender.__doc__ + '\n' + '* ``RendersTriangles``: ' + Qgis.TiledSceneRendererFlag.RendersTriangles.__doc__ + '\n' + '* ``RendersLines``: ' + Qgis.TiledSceneRendererFlag.RendersLines.__doc__
# --
Qgis.TiledSceneRendererFlag.baseClass = Qgis
Qgis.TiledSceneRendererFlags = lambda flags=0: Qgis.TiledSceneRendererFlag(flags)
Qgis.TiledSceneRendererFlags.baseClass = Qgis
TiledSceneRendererFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.GdalResampleAlgorithm.RA_NearestNeighbour.__doc__ = "Nearest neighbour (select on one input pixel)"
Qgis.GdalResampleAlgorithm.RA_Bilinear.__doc__ = "Bilinear (2x2 kernel)"
Qgis.GdalResampleAlgorithm.RA_Cubic.__doc__ = "Cubic Convolution Approximation (4x4 kernel)"
Qgis.GdalResampleAlgorithm.RA_CubicSpline.__doc__ = "Cubic B-Spline Approximation (4x4 kernel)"
Qgis.GdalResampleAlgorithm.RA_Lanczos.__doc__ = "Lanczos windowed sinc interpolation (6x6 kernel)"
Qgis.GdalResampleAlgorithm.RA_Average.__doc__ = "Average (computes the average of all non-NODATA contributing pixels)"
Qgis.GdalResampleAlgorithm.RA_Mode.__doc__ = "Mode (selects the value which appears most often of all the sampled points)"
Qgis.GdalResampleAlgorithm.RA_Max.__doc__ = "Maximum (selects the maximum of all non-NODATA contributing pixels)"
Qgis.GdalResampleAlgorithm.RA_Min.__doc__ = "Minimum (selects the minimum of all non-NODATA contributing pixels)"
Qgis.GdalResampleAlgorithm.RA_Median.__doc__ = "Median (selects the median of all non-NODATA contributing pixels)"
Qgis.GdalResampleAlgorithm.RA_Q1.__doc__ = "First quartile (selects the first quartile of all non-NODATA contributing pixels)"
Qgis.GdalResampleAlgorithm.RA_Q3.__doc__ = "Third quartile (selects the third quartile of all non-NODATA contributing pixels)"
Qgis.GdalResampleAlgorithm.__doc__ = "Resampling algorithm to be used (equivalent to GDAL's enum GDALResampleAlg)\n\n.. note::\n\n   RA_Max, RA_Min, RA_Median, RA_Q1 and RA_Q3 are available on GDAL >= 2.0 builds only\n\n.. versionadded:: 3.34\n\n" + '* ``RA_NearestNeighbour``: ' + Qgis.GdalResampleAlgorithm.RA_NearestNeighbour.__doc__ + '\n' + '* ``RA_Bilinear``: ' + Qgis.GdalResampleAlgorithm.RA_Bilinear.__doc__ + '\n' + '* ``RA_Cubic``: ' + Qgis.GdalResampleAlgorithm.RA_Cubic.__doc__ + '\n' + '* ``RA_CubicSpline``: ' + Qgis.GdalResampleAlgorithm.RA_CubicSpline.__doc__ + '\n' + '* ``RA_Lanczos``: ' + Qgis.GdalResampleAlgorithm.RA_Lanczos.__doc__ + '\n' + '* ``RA_Average``: ' + Qgis.GdalResampleAlgorithm.RA_Average.__doc__ + '\n' + '* ``RA_Mode``: ' + Qgis.GdalResampleAlgorithm.RA_Mode.__doc__ + '\n' + '* ``RA_Max``: ' + Qgis.GdalResampleAlgorithm.RA_Max.__doc__ + '\n' + '* ``RA_Min``: ' + Qgis.GdalResampleAlgorithm.RA_Min.__doc__ + '\n' + '* ``RA_Median``: ' + Qgis.GdalResampleAlgorithm.RA_Median.__doc__ + '\n' + '* ``RA_Q1``: ' + Qgis.GdalResampleAlgorithm.RA_Q1.__doc__ + '\n' + '* ``RA_Q3``: ' + Qgis.GdalResampleAlgorithm.RA_Q3.__doc__
# --
Qgis.GdalResampleAlgorithm.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ZonalStatistic.Count.__doc__ = "Pixel count"
Qgis.ZonalStatistic.Sum.__doc__ = "Sum of pixel values"
Qgis.ZonalStatistic.Mean.__doc__ = "Mean of pixel values"
Qgis.ZonalStatistic.Median.__doc__ = "Median of pixel values"
Qgis.ZonalStatistic.StDev.__doc__ = "Standard deviation of pixel values"
Qgis.ZonalStatistic.Min.__doc__ = "Min of pixel values"
Qgis.ZonalStatistic.Max.__doc__ = "Max of pixel values"
Qgis.ZonalStatistic.Range.__doc__ = "Range of pixel values (max - min)"
Qgis.ZonalStatistic.Minority.__doc__ = "Minority of pixel values"
Qgis.ZonalStatistic.Majority.__doc__ = "Majority of pixel values"
Qgis.ZonalStatistic.Variety.__doc__ = "Variety (count of distinct) pixel values"
Qgis.ZonalStatistic.Variance.__doc__ = "Variance of pixel values"
Qgis.ZonalStatistic.All.__doc__ = "All statistics"
Qgis.ZonalStatistic.Default.__doc__ = "Default statistics"
Qgis.ZonalStatistic.__doc__ = "Statistics to be calculated during a zonal statistics operation.\n\n.. versionadded:: 3.36.\n\n" + '* ``Count``: ' + Qgis.ZonalStatistic.Count.__doc__ + '\n' + '* ``Sum``: ' + Qgis.ZonalStatistic.Sum.__doc__ + '\n' + '* ``Mean``: ' + Qgis.ZonalStatistic.Mean.__doc__ + '\n' + '* ``Median``: ' + Qgis.ZonalStatistic.Median.__doc__ + '\n' + '* ``StDev``: ' + Qgis.ZonalStatistic.StDev.__doc__ + '\n' + '* ``Min``: ' + Qgis.ZonalStatistic.Min.__doc__ + '\n' + '* ``Max``: ' + Qgis.ZonalStatistic.Max.__doc__ + '\n' + '* ``Range``: ' + Qgis.ZonalStatistic.Range.__doc__ + '\n' + '* ``Minority``: ' + Qgis.ZonalStatistic.Minority.__doc__ + '\n' + '* ``Majority``: ' + Qgis.ZonalStatistic.Majority.__doc__ + '\n' + '* ``Variety``: ' + Qgis.ZonalStatistic.Variety.__doc__ + '\n' + '* ``Variance``: ' + Qgis.ZonalStatistic.Variance.__doc__ + '\n' + '* ``All``: ' + Qgis.ZonalStatistic.All.__doc__ + '\n' + '* ``Default``: ' + Qgis.ZonalStatistic.Default.__doc__
# --
Qgis.ZonalStatistic.baseClass = Qgis
Qgis.ZonalStatistics = lambda flags=0: Qgis.ZonalStatistic(flags)
Qgis.ZonalStatistics.baseClass = Qgis
ZonalStatistics = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ZonalStatisticResult.Success.__doc__ = "Success"
Qgis.ZonalStatisticResult.LayerTypeWrong.__doc__ = "Layer is not a polygon layer"
Qgis.ZonalStatisticResult.LayerInvalid.__doc__ = "Layer is invalid"
Qgis.ZonalStatisticResult.RasterInvalid.__doc__ = "Raster layer is invalid"
Qgis.ZonalStatisticResult.RasterBandInvalid.__doc__ = "The raster band does not exist on the raster layer"
Qgis.ZonalStatisticResult.FailedToCreateField.__doc__ = "Output fields could not be created"
Qgis.ZonalStatisticResult.Canceled.__doc__ = "Algorithm was canceled"
Qgis.ZonalStatisticResult.__doc__ = "Zonal statistics result codes.\n\n.. versionadded:: 3.36.\n\n" + '* ``Success``: ' + Qgis.ZonalStatisticResult.Success.__doc__ + '\n' + '* ``LayerTypeWrong``: ' + Qgis.ZonalStatisticResult.LayerTypeWrong.__doc__ + '\n' + '* ``LayerInvalid``: ' + Qgis.ZonalStatisticResult.LayerInvalid.__doc__ + '\n' + '* ``RasterInvalid``: ' + Qgis.ZonalStatisticResult.RasterInvalid.__doc__ + '\n' + '* ``RasterBandInvalid``: ' + Qgis.ZonalStatisticResult.RasterBandInvalid.__doc__ + '\n' + '* ``FailedToCreateField``: ' + Qgis.ZonalStatisticResult.FailedToCreateField.__doc__ + '\n' + '* ``Canceled``: ' + Qgis.ZonalStatisticResult.Canceled.__doc__
# --
Qgis.ZonalStatisticResult.baseClass = Qgis
QgsAggregateCalculator.Aggregate = Qgis.Aggregate
# monkey patching scoped based enum
QgsAggregateCalculator.Count = Qgis.Aggregate.Count
QgsAggregateCalculator.Count.is_monkey_patched = True
QgsAggregateCalculator.Count.__doc__ = "Count"
QgsAggregateCalculator.CountDistinct = Qgis.Aggregate.CountDistinct
QgsAggregateCalculator.CountDistinct.is_monkey_patched = True
QgsAggregateCalculator.CountDistinct.__doc__ = "Number of distinct values"
QgsAggregateCalculator.CountMissing = Qgis.Aggregate.CountMissing
QgsAggregateCalculator.CountMissing.is_monkey_patched = True
QgsAggregateCalculator.CountMissing.__doc__ = "Number of missing (null) values"
QgsAggregateCalculator.Min = Qgis.Aggregate.Min
QgsAggregateCalculator.Min.is_monkey_patched = True
QgsAggregateCalculator.Min.__doc__ = "Min of values"
QgsAggregateCalculator.Max = Qgis.Aggregate.Max
QgsAggregateCalculator.Max.is_monkey_patched = True
QgsAggregateCalculator.Max.__doc__ = "Max of values"
QgsAggregateCalculator.Sum = Qgis.Aggregate.Sum
QgsAggregateCalculator.Sum.is_monkey_patched = True
QgsAggregateCalculator.Sum.__doc__ = "Sum of values"
QgsAggregateCalculator.Mean = Qgis.Aggregate.Mean
QgsAggregateCalculator.Mean.is_monkey_patched = True
QgsAggregateCalculator.Mean.__doc__ = "Mean of values (numeric fields only)"
QgsAggregateCalculator.Median = Qgis.Aggregate.Median
QgsAggregateCalculator.Median.is_monkey_patched = True
QgsAggregateCalculator.Median.__doc__ = "Median of values (numeric fields only)"
QgsAggregateCalculator.StDev = Qgis.Aggregate.StDev
QgsAggregateCalculator.StDev.is_monkey_patched = True
QgsAggregateCalculator.StDev.__doc__ = "Standard deviation of values (numeric fields only)"
QgsAggregateCalculator.StDevSample = Qgis.Aggregate.StDevSample
QgsAggregateCalculator.StDevSample.is_monkey_patched = True
QgsAggregateCalculator.StDevSample.__doc__ = "Sample standard deviation of values (numeric fields only)"
QgsAggregateCalculator.Range = Qgis.Aggregate.Range
QgsAggregateCalculator.Range.is_monkey_patched = True
QgsAggregateCalculator.Range.__doc__ = "Range of values (max - min) (numeric and datetime fields only)"
QgsAggregateCalculator.Minority = Qgis.Aggregate.Minority
QgsAggregateCalculator.Minority.is_monkey_patched = True
QgsAggregateCalculator.Minority.__doc__ = "Minority of values"
QgsAggregateCalculator.Majority = Qgis.Aggregate.Majority
QgsAggregateCalculator.Majority.is_monkey_patched = True
QgsAggregateCalculator.Majority.__doc__ = "Majority of values"
QgsAggregateCalculator.FirstQuartile = Qgis.Aggregate.FirstQuartile
QgsAggregateCalculator.FirstQuartile.is_monkey_patched = True
QgsAggregateCalculator.FirstQuartile.__doc__ = "First quartile (numeric fields only)"
QgsAggregateCalculator.ThirdQuartile = Qgis.Aggregate.ThirdQuartile
QgsAggregateCalculator.ThirdQuartile.is_monkey_patched = True
QgsAggregateCalculator.ThirdQuartile.__doc__ = "Third quartile (numeric fields only)"
QgsAggregateCalculator.InterQuartileRange = Qgis.Aggregate.InterQuartileRange
QgsAggregateCalculator.InterQuartileRange.is_monkey_patched = True
QgsAggregateCalculator.InterQuartileRange.__doc__ = "Inter quartile range (IQR) (numeric fields only)"
QgsAggregateCalculator.StringMinimumLength = Qgis.Aggregate.StringMinimumLength
QgsAggregateCalculator.StringMinimumLength.is_monkey_patched = True
QgsAggregateCalculator.StringMinimumLength.__doc__ = "Minimum length of string (string fields only)"
QgsAggregateCalculator.StringMaximumLength = Qgis.Aggregate.StringMaximumLength
QgsAggregateCalculator.StringMaximumLength.is_monkey_patched = True
QgsAggregateCalculator.StringMaximumLength.__doc__ = "Maximum length of string (string fields only)"
QgsAggregateCalculator.StringConcatenate = Qgis.Aggregate.StringConcatenate
QgsAggregateCalculator.StringConcatenate.is_monkey_patched = True
QgsAggregateCalculator.StringConcatenate.__doc__ = "Concatenate values with a joining string (string fields only). Specify the delimiter using setDelimiter()."
QgsAggregateCalculator.GeometryCollect = Qgis.Aggregate.GeometryCollect
QgsAggregateCalculator.GeometryCollect.is_monkey_patched = True
QgsAggregateCalculator.GeometryCollect.__doc__ = "Create a multipart geometry from aggregated geometries"
QgsAggregateCalculator.ArrayAggregate = Qgis.Aggregate.ArrayAggregate
QgsAggregateCalculator.ArrayAggregate.is_monkey_patched = True
QgsAggregateCalculator.ArrayAggregate.__doc__ = "Create an array of values"
QgsAggregateCalculator.StringConcatenateUnique = Qgis.Aggregate.StringConcatenateUnique
QgsAggregateCalculator.StringConcatenateUnique.is_monkey_patched = True
QgsAggregateCalculator.StringConcatenateUnique.__doc__ = "Concatenate unique values with a joining string (string fields only). Specify the delimiter using setDelimiter()."
Qgis.Aggregate.__doc__ = "Available aggregates to calculate. Not all aggregates are available for all field\ntypes.\n\n.. versionadded:: 3.36.\n\n" + '* ``Count``: ' + Qgis.Aggregate.Count.__doc__ + '\n' + '* ``CountDistinct``: ' + Qgis.Aggregate.CountDistinct.__doc__ + '\n' + '* ``CountMissing``: ' + Qgis.Aggregate.CountMissing.__doc__ + '\n' + '* ``Min``: ' + Qgis.Aggregate.Min.__doc__ + '\n' + '* ``Max``: ' + Qgis.Aggregate.Max.__doc__ + '\n' + '* ``Sum``: ' + Qgis.Aggregate.Sum.__doc__ + '\n' + '* ``Mean``: ' + Qgis.Aggregate.Mean.__doc__ + '\n' + '* ``Median``: ' + Qgis.Aggregate.Median.__doc__ + '\n' + '* ``StDev``: ' + Qgis.Aggregate.StDev.__doc__ + '\n' + '* ``StDevSample``: ' + Qgis.Aggregate.StDevSample.__doc__ + '\n' + '* ``Range``: ' + Qgis.Aggregate.Range.__doc__ + '\n' + '* ``Minority``: ' + Qgis.Aggregate.Minority.__doc__ + '\n' + '* ``Majority``: ' + Qgis.Aggregate.Majority.__doc__ + '\n' + '* ``FirstQuartile``: ' + Qgis.Aggregate.FirstQuartile.__doc__ + '\n' + '* ``ThirdQuartile``: ' + Qgis.Aggregate.ThirdQuartile.__doc__ + '\n' + '* ``InterQuartileRange``: ' + Qgis.Aggregate.InterQuartileRange.__doc__ + '\n' + '* ``StringMinimumLength``: ' + Qgis.Aggregate.StringMinimumLength.__doc__ + '\n' + '* ``StringMaximumLength``: ' + Qgis.Aggregate.StringMaximumLength.__doc__ + '\n' + '* ``StringConcatenate``: ' + Qgis.Aggregate.StringConcatenate.__doc__ + '\n' + '* ``GeometryCollect``: ' + Qgis.Aggregate.GeometryCollect.__doc__ + '\n' + '* ``ArrayAggregate``: ' + Qgis.Aggregate.ArrayAggregate.__doc__ + '\n' + '* ``StringConcatenateUnique``: ' + Qgis.Aggregate.StringConcatenateUnique.__doc__
# --
Qgis.Aggregate.baseClass = Qgis
QgsStatisticalSummary.Statistic = Qgis.Statistic
# monkey patching scoped based enum
QgsStatisticalSummary.Count = Qgis.Statistic.Count
QgsStatisticalSummary.Count.is_monkey_patched = True
QgsStatisticalSummary.Count.__doc__ = "Count"
QgsStatisticalSummary.CountMissing = Qgis.Statistic.CountMissing
QgsStatisticalSummary.CountMissing.is_monkey_patched = True
QgsStatisticalSummary.CountMissing.__doc__ = "Number of missing (null) values"
QgsStatisticalSummary.Sum = Qgis.Statistic.Sum
QgsStatisticalSummary.Sum.is_monkey_patched = True
QgsStatisticalSummary.Sum.__doc__ = "Sum of values"
QgsStatisticalSummary.Mean = Qgis.Statistic.Mean
QgsStatisticalSummary.Mean.is_monkey_patched = True
QgsStatisticalSummary.Mean.__doc__ = "Mean of values"
QgsStatisticalSummary.Median = Qgis.Statistic.Median
QgsStatisticalSummary.Median.is_monkey_patched = True
QgsStatisticalSummary.Median.__doc__ = "Median of values"
QgsStatisticalSummary.StDev = Qgis.Statistic.StDev
QgsStatisticalSummary.StDev.is_monkey_patched = True
QgsStatisticalSummary.StDev.__doc__ = "Standard deviation of values"
QgsStatisticalSummary.StDevSample = Qgis.Statistic.StDevSample
QgsStatisticalSummary.StDevSample.is_monkey_patched = True
QgsStatisticalSummary.StDevSample.__doc__ = "Sample standard deviation of values"
QgsStatisticalSummary.Min = Qgis.Statistic.Min
QgsStatisticalSummary.Min.is_monkey_patched = True
QgsStatisticalSummary.Min.__doc__ = "Min of values"
QgsStatisticalSummary.Max = Qgis.Statistic.Max
QgsStatisticalSummary.Max.is_monkey_patched = True
QgsStatisticalSummary.Max.__doc__ = "Max of values"
QgsStatisticalSummary.Range = Qgis.Statistic.Range
QgsStatisticalSummary.Range.is_monkey_patched = True
QgsStatisticalSummary.Range.__doc__ = "Range of values (max - min)"
QgsStatisticalSummary.Minority = Qgis.Statistic.Minority
QgsStatisticalSummary.Minority.is_monkey_patched = True
QgsStatisticalSummary.Minority.__doc__ = "Minority of values"
QgsStatisticalSummary.Majority = Qgis.Statistic.Majority
QgsStatisticalSummary.Majority.is_monkey_patched = True
QgsStatisticalSummary.Majority.__doc__ = "Majority of values"
QgsStatisticalSummary.Variety = Qgis.Statistic.Variety
QgsStatisticalSummary.Variety.is_monkey_patched = True
QgsStatisticalSummary.Variety.__doc__ = "Variety (count of distinct) values"
QgsStatisticalSummary.FirstQuartile = Qgis.Statistic.FirstQuartile
QgsStatisticalSummary.FirstQuartile.is_monkey_patched = True
QgsStatisticalSummary.FirstQuartile.__doc__ = "First quartile"
QgsStatisticalSummary.ThirdQuartile = Qgis.Statistic.ThirdQuartile
QgsStatisticalSummary.ThirdQuartile.is_monkey_patched = True
QgsStatisticalSummary.ThirdQuartile.__doc__ = "Third quartile"
QgsStatisticalSummary.InterQuartileRange = Qgis.Statistic.InterQuartileRange
QgsStatisticalSummary.InterQuartileRange.is_monkey_patched = True
QgsStatisticalSummary.InterQuartileRange.__doc__ = "Inter quartile range (IQR)"
QgsStatisticalSummary.First = Qgis.Statistic.First
QgsStatisticalSummary.First.is_monkey_patched = True
QgsStatisticalSummary.First.__doc__ = "First value (since QGIS 3.6)"
QgsStatisticalSummary.Last = Qgis.Statistic.Last
QgsStatisticalSummary.Last.is_monkey_patched = True
QgsStatisticalSummary.Last.__doc__ = "Last value (since QGIS 3.6)"
QgsStatisticalSummary.All = Qgis.Statistic.All
QgsStatisticalSummary.All.is_monkey_patched = True
QgsStatisticalSummary.All.__doc__ = "All statistics"
Qgis.Statistic.__doc__ = "Available generic statistics.\n\n.. versionadded:: 3.36.\n\n" + '* ``Count``: ' + Qgis.Statistic.Count.__doc__ + '\n' + '* ``CountMissing``: ' + Qgis.Statistic.CountMissing.__doc__ + '\n' + '* ``Sum``: ' + Qgis.Statistic.Sum.__doc__ + '\n' + '* ``Mean``: ' + Qgis.Statistic.Mean.__doc__ + '\n' + '* ``Median``: ' + Qgis.Statistic.Median.__doc__ + '\n' + '* ``StDev``: ' + Qgis.Statistic.StDev.__doc__ + '\n' + '* ``StDevSample``: ' + Qgis.Statistic.StDevSample.__doc__ + '\n' + '* ``Min``: ' + Qgis.Statistic.Min.__doc__ + '\n' + '* ``Max``: ' + Qgis.Statistic.Max.__doc__ + '\n' + '* ``Range``: ' + Qgis.Statistic.Range.__doc__ + '\n' + '* ``Minority``: ' + Qgis.Statistic.Minority.__doc__ + '\n' + '* ``Majority``: ' + Qgis.Statistic.Majority.__doc__ + '\n' + '* ``Variety``: ' + Qgis.Statistic.Variety.__doc__ + '\n' + '* ``FirstQuartile``: ' + Qgis.Statistic.FirstQuartile.__doc__ + '\n' + '* ``ThirdQuartile``: ' + Qgis.Statistic.ThirdQuartile.__doc__ + '\n' + '* ``InterQuartileRange``: ' + Qgis.Statistic.InterQuartileRange.__doc__ + '\n' + '* ``First``: ' + Qgis.Statistic.First.__doc__ + '\n' + '* ``Last``: ' + Qgis.Statistic.Last.__doc__ + '\n' + '* ``All``: ' + Qgis.Statistic.All.__doc__
# --
Qgis.Statistic.baseClass = Qgis
Qgis.Statistics = lambda flags=0: Qgis.Statistic(flags)
QgsStatisticalSummary.Statistics = Qgis.Statistics
Qgis.Statistics.baseClass = Qgis
Statistics = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsDateTimeStatisticalSummary.Statistic = Qgis.DateTimeStatistic
# monkey patching scoped based enum
QgsDateTimeStatisticalSummary.Count = Qgis.DateTimeStatistic.Count
QgsDateTimeStatisticalSummary.Count.is_monkey_patched = True
QgsDateTimeStatisticalSummary.Count.__doc__ = "Count"
QgsDateTimeStatisticalSummary.CountDistinct = Qgis.DateTimeStatistic.CountDistinct
QgsDateTimeStatisticalSummary.CountDistinct.is_monkey_patched = True
QgsDateTimeStatisticalSummary.CountDistinct.__doc__ = "Number of distinct datetime values"
QgsDateTimeStatisticalSummary.CountMissing = Qgis.DateTimeStatistic.CountMissing
QgsDateTimeStatisticalSummary.CountMissing.is_monkey_patched = True
QgsDateTimeStatisticalSummary.CountMissing.__doc__ = "Number of missing (null) values"
QgsDateTimeStatisticalSummary.Min = Qgis.DateTimeStatistic.Min
QgsDateTimeStatisticalSummary.Min.is_monkey_patched = True
QgsDateTimeStatisticalSummary.Min.__doc__ = "Minimum (earliest) datetime value"
QgsDateTimeStatisticalSummary.Max = Qgis.DateTimeStatistic.Max
QgsDateTimeStatisticalSummary.Max.is_monkey_patched = True
QgsDateTimeStatisticalSummary.Max.__doc__ = "Maximum (latest) datetime value"
QgsDateTimeStatisticalSummary.Range = Qgis.DateTimeStatistic.Range
QgsDateTimeStatisticalSummary.Range.is_monkey_patched = True
QgsDateTimeStatisticalSummary.Range.__doc__ = "Interval between earliest and latest datetime value"
QgsDateTimeStatisticalSummary.All = Qgis.DateTimeStatistic.All
QgsDateTimeStatisticalSummary.All.is_monkey_patched = True
QgsDateTimeStatisticalSummary.All.__doc__ = "All statistics"
Qgis.DateTimeStatistic.__doc__ = "Available date/time statistics.\n\n.. versionadded:: 3.36.\n\n" + '* ``Count``: ' + Qgis.DateTimeStatistic.Count.__doc__ + '\n' + '* ``CountDistinct``: ' + Qgis.DateTimeStatistic.CountDistinct.__doc__ + '\n' + '* ``CountMissing``: ' + Qgis.DateTimeStatistic.CountMissing.__doc__ + '\n' + '* ``Min``: ' + Qgis.DateTimeStatistic.Min.__doc__ + '\n' + '* ``Max``: ' + Qgis.DateTimeStatistic.Max.__doc__ + '\n' + '* ``Range``: ' + Qgis.DateTimeStatistic.Range.__doc__ + '\n' + '* ``All``: ' + Qgis.DateTimeStatistic.All.__doc__
# --
Qgis.DateTimeStatistic.baseClass = Qgis
Qgis.DateTimeStatistics = lambda flags=0: Qgis.DateTimeStatistic(flags)
QgsDateTimeStatisticalSummary.Statistics = Qgis.DateTimeStatistics
Qgis.DateTimeStatistics.baseClass = Qgis
DateTimeStatistics = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsStringStatisticalSummary.Statistic = Qgis.StringStatistic
# monkey patching scoped based enum
QgsStringStatisticalSummary.Count = Qgis.StringStatistic.Count
QgsStringStatisticalSummary.Count.is_monkey_patched = True
QgsStringStatisticalSummary.Count.__doc__ = "Count"
QgsStringStatisticalSummary.CountDistinct = Qgis.StringStatistic.CountDistinct
QgsStringStatisticalSummary.CountDistinct.is_monkey_patched = True
QgsStringStatisticalSummary.CountDistinct.__doc__ = "Number of distinct string values"
QgsStringStatisticalSummary.CountMissing = Qgis.StringStatistic.CountMissing
QgsStringStatisticalSummary.CountMissing.is_monkey_patched = True
QgsStringStatisticalSummary.CountMissing.__doc__ = "Number of missing (null) values"
QgsStringStatisticalSummary.Min = Qgis.StringStatistic.Min
QgsStringStatisticalSummary.Min.is_monkey_patched = True
QgsStringStatisticalSummary.Min.__doc__ = "Minimum string value"
QgsStringStatisticalSummary.Max = Qgis.StringStatistic.Max
QgsStringStatisticalSummary.Max.is_monkey_patched = True
QgsStringStatisticalSummary.Max.__doc__ = "Maximum string value"
QgsStringStatisticalSummary.MinimumLength = Qgis.StringStatistic.MinimumLength
QgsStringStatisticalSummary.MinimumLength.is_monkey_patched = True
QgsStringStatisticalSummary.MinimumLength.__doc__ = "Minimum length of string"
QgsStringStatisticalSummary.MaximumLength = Qgis.StringStatistic.MaximumLength
QgsStringStatisticalSummary.MaximumLength.is_monkey_patched = True
QgsStringStatisticalSummary.MaximumLength.__doc__ = "Maximum length of string"
QgsStringStatisticalSummary.MeanLength = Qgis.StringStatistic.MeanLength
QgsStringStatisticalSummary.MeanLength.is_monkey_patched = True
QgsStringStatisticalSummary.MeanLength.__doc__ = "Mean length of strings"
QgsStringStatisticalSummary.Minority = Qgis.StringStatistic.Minority
QgsStringStatisticalSummary.Minority.is_monkey_patched = True
QgsStringStatisticalSummary.Minority.__doc__ = "Minority of strings"
QgsStringStatisticalSummary.Majority = Qgis.StringStatistic.Majority
QgsStringStatisticalSummary.Majority.is_monkey_patched = True
QgsStringStatisticalSummary.Majority.__doc__ = "Majority of strings"
QgsStringStatisticalSummary.All = Qgis.StringStatistic.All
QgsStringStatisticalSummary.All.is_monkey_patched = True
QgsStringStatisticalSummary.All.__doc__ = "All statistics"
Qgis.StringStatistic.__doc__ = "Available string statistics.\n\n.. versionadded:: 3.36.\n\n" + '* ``Count``: ' + Qgis.StringStatistic.Count.__doc__ + '\n' + '* ``CountDistinct``: ' + Qgis.StringStatistic.CountDistinct.__doc__ + '\n' + '* ``CountMissing``: ' + Qgis.StringStatistic.CountMissing.__doc__ + '\n' + '* ``Min``: ' + Qgis.StringStatistic.Min.__doc__ + '\n' + '* ``Max``: ' + Qgis.StringStatistic.Max.__doc__ + '\n' + '* ``MinimumLength``: ' + Qgis.StringStatistic.MinimumLength.__doc__ + '\n' + '* ``MaximumLength``: ' + Qgis.StringStatistic.MaximumLength.__doc__ + '\n' + '* ``MeanLength``: ' + Qgis.StringStatistic.MeanLength.__doc__ + '\n' + '* ``Minority``: ' + Qgis.StringStatistic.Minority.__doc__ + '\n' + '* ``Majority``: ' + Qgis.StringStatistic.Majority.__doc__ + '\n' + '* ``All``: ' + Qgis.StringStatistic.All.__doc__
# --
Qgis.StringStatistic.baseClass = Qgis
Qgis.StringStatistics = lambda flags=0: Qgis.StringStatistic(flags)
QgsStringStatisticalSummary.Statistics = Qgis.StringStatistics
Qgis.StringStatistics.baseClass = Qgis
StringStatistics = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsRasterBandStats.Stats = Qgis.RasterBandStatistic
# monkey patching scoped based enum
QgsRasterBandStats.NoStatistic = Qgis.RasterBandStatistic.NoStatistic
QgsRasterBandStats.NoStatistic.is_monkey_patched = True
QgsRasterBandStats.NoStatistic.__doc__ = "No statistic"
QgsRasterBandStats.Min = Qgis.RasterBandStatistic.Min
QgsRasterBandStats.Min.is_monkey_patched = True
QgsRasterBandStats.Min.__doc__ = "Minimum"
QgsRasterBandStats.Max = Qgis.RasterBandStatistic.Max
QgsRasterBandStats.Max.is_monkey_patched = True
QgsRasterBandStats.Max.__doc__ = "Maximum"
QgsRasterBandStats.Range = Qgis.RasterBandStatistic.Range
QgsRasterBandStats.Range.is_monkey_patched = True
QgsRasterBandStats.Range.__doc__ = "Range"
QgsRasterBandStats.Sum = Qgis.RasterBandStatistic.Sum
QgsRasterBandStats.Sum.is_monkey_patched = True
QgsRasterBandStats.Sum.__doc__ = "Sum"
QgsRasterBandStats.Mean = Qgis.RasterBandStatistic.Mean
QgsRasterBandStats.Mean.is_monkey_patched = True
QgsRasterBandStats.Mean.__doc__ = "Mean"
QgsRasterBandStats.StdDev = Qgis.RasterBandStatistic.StdDev
QgsRasterBandStats.StdDev.is_monkey_patched = True
QgsRasterBandStats.StdDev.__doc__ = "Standard deviation"
QgsRasterBandStats.SumOfSquares = Qgis.RasterBandStatistic.SumOfSquares
QgsRasterBandStats.SumOfSquares.is_monkey_patched = True
QgsRasterBandStats.SumOfSquares.__doc__ = "Sum of squares"
QgsRasterBandStats.All = Qgis.RasterBandStatistic.All
QgsRasterBandStats.All.is_monkey_patched = True
QgsRasterBandStats.All.__doc__ = "All available statistics"
Qgis.RasterBandStatistic.__doc__ = "Available raster band statistics.\n\n.. versionadded:: 3.36.\n\n" + '* ``NoStatistic``: ' + Qgis.RasterBandStatistic.NoStatistic.__doc__ + '\n' + '* ``Min``: ' + Qgis.RasterBandStatistic.Min.__doc__ + '\n' + '* ``Max``: ' + Qgis.RasterBandStatistic.Max.__doc__ + '\n' + '* ``Range``: ' + Qgis.RasterBandStatistic.Range.__doc__ + '\n' + '* ``Sum``: ' + Qgis.RasterBandStatistic.Sum.__doc__ + '\n' + '* ``Mean``: ' + Qgis.RasterBandStatistic.Mean.__doc__ + '\n' + '* ``StdDev``: ' + Qgis.RasterBandStatistic.StdDev.__doc__ + '\n' + '* ``SumOfSquares``: ' + Qgis.RasterBandStatistic.SumOfSquares.__doc__ + '\n' + '* ``All``: ' + Qgis.RasterBandStatistic.All.__doc__
# --
Qgis.RasterBandStatistic.baseClass = Qgis
Qgis.RasterBandStatistics = lambda flags=0: Qgis.RasterBandStatistic(flags)
Qgis.RasterBandStatistics.baseClass = Qgis
RasterBandStatistics = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SensorThingsEntity.Invalid.__doc__ = "An invalid/unknown entity"
Qgis.SensorThingsEntity.Thing.__doc__ = "A Thing is an object of the physical world (physical things) or the information world (virtual things) that is capable of being identified and integrated into communication networks"
Qgis.SensorThingsEntity.Location.__doc__ = "A Location entity locates the Thing or the Things it associated with. A Thing’s Location entity is defined as the last known location of the Thing"
Qgis.SensorThingsEntity.HistoricalLocation.__doc__ = "A Thing’s HistoricalLocation entity set provides the times of the current (i.e., last known) and previous locations of the Thing"
Qgis.SensorThingsEntity.Datastream.__doc__ = "A Datastream groups a collection of Observations measuring the same ObservedProperty and produced by the same Sensor"
Qgis.SensorThingsEntity.Sensor.__doc__ = "A Sensor is an instrument that observes a property or phenomenon with the goal of producing an estimate of the value of the property"
Qgis.SensorThingsEntity.ObservedProperty.__doc__ = "An ObservedProperty specifies the phenomenon of an Observation"
Qgis.SensorThingsEntity.Observation.__doc__ = "An Observation is the act of measuring or otherwise determining the value of a property"
Qgis.SensorThingsEntity.FeatureOfInterest.__doc__ = "In the context of the Internet of Things, many Observations’ FeatureOfInterest can be the Location of the Thing. For example, the FeatureOfInterest of a wifi-connect thermostat can be the Location of the thermostat (i.e., the living room where the thermostat is located in). In the case of remote sensing, the FeatureOfInterest can be the geographical area or volume that is being sensed"
Qgis.SensorThingsEntity.MultiDatastream.__doc__ = "A MultiDatastream groups a collection of Observations and the Observations in a MultiDatastream have a complex result type. Implemented in the SensorThings version 1.1 \"MultiDatastream extension\". (Since QGIS 3.38)"
Qgis.SensorThingsEntity.__doc__ = "OGC SensorThings API entity types.\n\n.. versionadded:: 3.36\n\n" + '* ``Invalid``: ' + Qgis.SensorThingsEntity.Invalid.__doc__ + '\n' + '* ``Thing``: ' + Qgis.SensorThingsEntity.Thing.__doc__ + '\n' + '* ``Location``: ' + Qgis.SensorThingsEntity.Location.__doc__ + '\n' + '* ``HistoricalLocation``: ' + Qgis.SensorThingsEntity.HistoricalLocation.__doc__ + '\n' + '* ``Datastream``: ' + Qgis.SensorThingsEntity.Datastream.__doc__ + '\n' + '* ``Sensor``: ' + Qgis.SensorThingsEntity.Sensor.__doc__ + '\n' + '* ``ObservedProperty``: ' + Qgis.SensorThingsEntity.ObservedProperty.__doc__ + '\n' + '* ``Observation``: ' + Qgis.SensorThingsEntity.Observation.__doc__ + '\n' + '* ``FeatureOfInterest``: ' + Qgis.SensorThingsEntity.FeatureOfInterest.__doc__ + '\n' + '* ``MultiDatastream``: ' + Qgis.SensorThingsEntity.MultiDatastream.__doc__
# --
Qgis.SensorThingsEntity.baseClass = Qgis
