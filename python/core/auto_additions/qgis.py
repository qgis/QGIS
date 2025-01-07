# The following has been generated automatically from src/core/qgis.h
# monkey patching scoped based enum
Qgis.AuthConfigurationStorageCapability.ClearStorage.__doc__ = "Can clear all configurations from storage"
Qgis.AuthConfigurationStorageCapability.ReadConfiguration.__doc__ = "Can read an authentication configuration"
Qgis.AuthConfigurationStorageCapability.UpdateConfiguration.__doc__ = "Can update an authentication configuration"
Qgis.AuthConfigurationStorageCapability.DeleteConfiguration.__doc__ = "Can deleet an authentication configuration"
Qgis.AuthConfigurationStorageCapability.CreateConfiguration.__doc__ = "Can create a new authentication configuration"
Qgis.AuthConfigurationStorageCapability.ReadCertificateIdentity.__doc__ = "Can read a certificate identity"
Qgis.AuthConfigurationStorageCapability.UpdateCertificateIdentity.__doc__ = "Can update a certificate identity"
Qgis.AuthConfigurationStorageCapability.DeleteCertificateIdentity.__doc__ = "Can delete a certificate identity"
Qgis.AuthConfigurationStorageCapability.CreateCertificateIdentity.__doc__ = "Can create a new certificate identity"
Qgis.AuthConfigurationStorageCapability.ReadSslCertificateCustomConfig.__doc__ = "Can read a SSL certificate custom config"
Qgis.AuthConfigurationStorageCapability.UpdateSslCertificateCustomConfig.__doc__ = "Can update a SSL certificate custom config"
Qgis.AuthConfigurationStorageCapability.DeleteSslCertificateCustomConfig.__doc__ = "Can delete a SSL certificate custom config"
Qgis.AuthConfigurationStorageCapability.CreateSslCertificateCustomConfig.__doc__ = "Can create a new SSL certificate custom config"
Qgis.AuthConfigurationStorageCapability.ReadCertificateAuthority.__doc__ = "Can read a certificate authority"
Qgis.AuthConfigurationStorageCapability.UpdateCertificateAuthority.__doc__ = "Can update a certificate authority"
Qgis.AuthConfigurationStorageCapability.DeleteCertificateAuthority.__doc__ = "Can delete a certificate authority"
Qgis.AuthConfigurationStorageCapability.CreateCertificateAuthority.__doc__ = "Can create a new certificate authority"
Qgis.AuthConfigurationStorageCapability.ReadCertificateTrustPolicy.__doc__ = "Can read a certificate trust policy"
Qgis.AuthConfigurationStorageCapability.UpdateCertificateTrustPolicy.__doc__ = "Can update a certificate trust policy"
Qgis.AuthConfigurationStorageCapability.DeleteCertificateTrustPolicy.__doc__ = "Can delete a certificate trust policy"
Qgis.AuthConfigurationStorageCapability.CreateCertificateTrustPolicy.__doc__ = "Can create a new certificate trust policy"
Qgis.AuthConfigurationStorageCapability.ReadMasterPassword.__doc__ = "Can read the master password"
Qgis.AuthConfigurationStorageCapability.UpdateMasterPassword.__doc__ = "Can update the master password"
Qgis.AuthConfigurationStorageCapability.DeleteMasterPassword.__doc__ = "Can delete the master password"
Qgis.AuthConfigurationStorageCapability.CreateMasterPassword.__doc__ = "Can create a new master password"
Qgis.AuthConfigurationStorageCapability.ReadSetting.__doc__ = "Can read the authentication settings"
Qgis.AuthConfigurationStorageCapability.UpdateSetting.__doc__ = "Can update the authentication setting"
Qgis.AuthConfigurationStorageCapability.DeleteSetting.__doc__ = "Can delete the authentication setting"
Qgis.AuthConfigurationStorageCapability.CreateSetting.__doc__ = "Can create a new authentication setting"
Qgis.AuthConfigurationStorageCapability.__doc__ = """Authentication configuration storage capabilities.

.. versionadded:: 3.40

* ``ClearStorage``: Can clear all configurations from storage
* ``ReadConfiguration``: Can read an authentication configuration
* ``UpdateConfiguration``: Can update an authentication configuration
* ``DeleteConfiguration``: Can deleet an authentication configuration
* ``CreateConfiguration``: Can create a new authentication configuration
* ``ReadCertificateIdentity``: Can read a certificate identity
* ``UpdateCertificateIdentity``: Can update a certificate identity
* ``DeleteCertificateIdentity``: Can delete a certificate identity
* ``CreateCertificateIdentity``: Can create a new certificate identity
* ``ReadSslCertificateCustomConfig``: Can read a SSL certificate custom config
* ``UpdateSslCertificateCustomConfig``: Can update a SSL certificate custom config
* ``DeleteSslCertificateCustomConfig``: Can delete a SSL certificate custom config
* ``CreateSslCertificateCustomConfig``: Can create a new SSL certificate custom config
* ``ReadCertificateAuthority``: Can read a certificate authority
* ``UpdateCertificateAuthority``: Can update a certificate authority
* ``DeleteCertificateAuthority``: Can delete a certificate authority
* ``CreateCertificateAuthority``: Can create a new certificate authority
* ``ReadCertificateTrustPolicy``: Can read a certificate trust policy
* ``UpdateCertificateTrustPolicy``: Can update a certificate trust policy
* ``DeleteCertificateTrustPolicy``: Can delete a certificate trust policy
* ``CreateCertificateTrustPolicy``: Can create a new certificate trust policy
* ``ReadMasterPassword``: Can read the master password
* ``UpdateMasterPassword``: Can update the master password
* ``DeleteMasterPassword``: Can delete the master password
* ``CreateMasterPassword``: Can create a new master password
* ``ReadSetting``: Can read the authentication settings
* ``UpdateSetting``: Can update the authentication setting
* ``DeleteSetting``: Can delete the authentication setting
* ``CreateSetting``: Can create a new authentication setting

"""
# --
Qgis.AuthConfigurationStorageCapability.baseClass = Qgis
Qgis.AuthConfigurationStorageCapabilities.baseClass = Qgis
AuthConfigurationStorageCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
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
Qgis.LayerType.__doc__ = """Types of layers that can be added to a map

.. versionadded:: 3.30.

* ``Vector``: Vector layer

  Available as ``QgsMapLayer.VectorLayer`` in older QGIS releases.

* ``Raster``: Raster layer

  Available as ``QgsMapLayer.RasterLayer`` in older QGIS releases.

* ``Plugin``: Plugin based layer

  Available as ``QgsMapLayer.PluginLayer`` in older QGIS releases.

* ``Mesh``: Mesh layer. Added in QGIS 3.2

  Available as ``QgsMapLayer.MeshLayer`` in older QGIS releases.

* ``VectorTile``: Vector tile layer. Added in QGIS 3.14

  Available as ``QgsMapLayer.VectorTileLayer`` in older QGIS releases.

* ``Annotation``: Contains freeform, georeferenced annotations. Added in QGIS 3.16

  Available as ``QgsMapLayer.AnnotationLayer`` in older QGIS releases.

* ``PointCloud``: Point cloud layer. Added in QGIS 3.18

  Available as ``QgsMapLayer.PointCloudLayer`` in older QGIS releases.

* ``Group``: Composite group layer. Added in QGIS 3.24

  Available as ``QgsMapLayer.GroupLayer`` in older QGIS releases.

* ``TiledScene``: Tiled scene layer. Added in QGIS 3.34

"""
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
QgsMapLayerProxyModel.All.__doc__ = "All layers"
QgsMapLayerProxyModel.SpatialLayer = Qgis.LayerFilter.SpatialLayer
QgsMapLayerProxyModel.SpatialLayer.is_monkey_patched = True
QgsMapLayerProxyModel.SpatialLayer.__doc__ = "All spatial layers. \n.. versionadded:: 3.24"
Qgis.LayerFilter.__doc__ = """Filter for layers

.. versionadded:: 3.34.

* ``RasterLayer``: 
* ``NoGeometry``: 
* ``PointLayer``: 
* ``LineLayer``: 
* ``PolygonLayer``: 
* ``HasGeometry``: 
* ``VectorLayer``: 
* ``PluginLayer``: 
* ``WritableLayer``: 
* ``MeshLayer``: QgsMeshLayer

  .. versionadded:: 3.6

* ``VectorTileLayer``: QgsVectorTileLayer

  .. versionadded:: 3.14

* ``PointCloudLayer``: QgsPointCloudLayer

  .. versionadded:: 3.18

* ``AnnotationLayer``: QgsAnnotationLayer

  .. versionadded:: 3.22

* ``TiledSceneLayer``: QgsTiledSceneLayer

  .. versionadded:: 3.34

* ``All``: All layers
* ``SpatialLayer``: All spatial layers.

  .. versionadded:: 3.24


"""
# --
Qgis.LayerFilters.baseClass = Qgis
LayerFilters = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.LoadStyleFlag.IgnoreMissingStyleErrors.__doc__ = "If the style is missing, then don't flag it as an error. This flag can be used when the caller is not certain that a style exists, and accordingly a failure to find the style does not indicate an issue with loading the style itself."
Qgis.LoadStyleFlag.__doc__ = """Flags for loading layer styles.

.. versionadded:: 3.38

* ``IgnoreMissingStyleErrors``: If the style is missing, then don't flag it as an error. This flag can be used when the caller is not certain that a style exists, and accordingly a failure to find the style does not indicate an issue with loading the style itself.

"""
# --
Qgis.LoadStyleFlag.baseClass = Qgis
Qgis.LoadStyleFlags.baseClass = Qgis
LoadStyleFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
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
QgsWkbTypes.PolyhedralSurface = Qgis.WkbType.PolyhedralSurface
QgsWkbTypes.PolyhedralSurface.is_monkey_patched = True
QgsWkbTypes.PolyhedralSurface.__doc__ = "PolyhedralSurface \n.. versionadded:: 3.40"
QgsWkbTypes.TIN = Qgis.WkbType.TIN
QgsWkbTypes.TIN.is_monkey_patched = True
QgsWkbTypes.TIN.__doc__ = "TIN \n.. versionadded:: 3.40"
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
QgsWkbTypes.PolyhedralSurfaceZ = Qgis.WkbType.PolyhedralSurfaceZ
QgsWkbTypes.PolyhedralSurfaceZ.is_monkey_patched = True
QgsWkbTypes.PolyhedralSurfaceZ.__doc__ = "PolyhedralSurfaceZ"
QgsWkbTypes.TINZ = Qgis.WkbType.TINZ
QgsWkbTypes.TINZ.is_monkey_patched = True
QgsWkbTypes.TINZ.__doc__ = "TINZ"
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
QgsWkbTypes.PolyhedralSurfaceM = Qgis.WkbType.PolyhedralSurfaceM
QgsWkbTypes.PolyhedralSurfaceM.is_monkey_patched = True
QgsWkbTypes.PolyhedralSurfaceM.__doc__ = "PolyhedralSurfaceM"
QgsWkbTypes.TINM = Qgis.WkbType.TINM
QgsWkbTypes.TINM.is_monkey_patched = True
QgsWkbTypes.TINM.__doc__ = "TINM"
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
QgsWkbTypes.PolyhedralSurfaceZM = Qgis.WkbType.PolyhedralSurfaceZM
QgsWkbTypes.PolyhedralSurfaceZM.is_monkey_patched = True
QgsWkbTypes.PolyhedralSurfaceZM.__doc__ = "PolyhedralSurfaceM"
QgsWkbTypes.TINZM = Qgis.WkbType.TINZM
QgsWkbTypes.TINZM.is_monkey_patched = True
QgsWkbTypes.TINZM.__doc__ = "TINZM"
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
Qgis.WkbType.__doc__ = """The WKB type describes the number of dimensions a geometry has

- Point
- LineString
- Polygon

as well as the number of dimensions for each individual vertex

- X (always)
- Y (always)
- Z (optional)
- M (measurement value, optional)

it also has values for multi types, collections, unknown geometry,
null geometry, no geometry and curve support.

These classes of geometry are often used for data sources to
communicate what kind of geometry should be expected for a given
geometry field. It is also used for tools or algorithms to decide
if they should be available for a given geometry type or act in
a different mode.

.. note::

   Prior to 3.30 this was available as :py:class:`QgsWkbTypes`.Type.

.. versionadded:: 3.30

* ``Unknown``: Unknown
* ``Point``: Point
* ``LineString``: LineString
* ``Polygon``: Polygon
* ``Triangle``: Triangle
* ``MultiPoint``: MultiPoint
* ``MultiLineString``: MultiLineString
* ``MultiPolygon``: MultiPolygon
* ``GeometryCollection``: GeometryCollection
* ``CircularString``: CircularString
* ``CompoundCurve``: CompoundCurve
* ``CurvePolygon``: CurvePolygon
* ``MultiCurve``: MultiCurve
* ``MultiSurface``: MultiSurface
* ``PolyhedralSurface``: PolyhedralSurface

  .. versionadded:: 3.40

* ``TIN``: TIN

  .. versionadded:: 3.40

* ``NoGeometry``: No geometry
* ``PointZ``: PointZ
* ``LineStringZ``: LineStringZ
* ``PolygonZ``: PolygonZ
* ``TriangleZ``: TriangleZ
* ``MultiPointZ``: MultiPointZ
* ``MultiLineStringZ``: MultiLineStringZ
* ``MultiPolygonZ``: MultiPolygonZ
* ``GeometryCollectionZ``: GeometryCollectionZ
* ``CircularStringZ``: CircularStringZ
* ``CompoundCurveZ``: CompoundCurveZ
* ``CurvePolygonZ``: CurvePolygonZ
* ``MultiCurveZ``: MultiCurveZ
* ``MultiSurfaceZ``: MultiSurfaceZ
* ``PolyhedralSurfaceZ``: PolyhedralSurfaceZ
* ``TINZ``: TINZ
* ``PointM``: PointM
* ``LineStringM``: LineStringM
* ``PolygonM``: PolygonM
* ``TriangleM``: TriangleM
* ``MultiPointM``: MultiPointM
* ``MultiLineStringM``: MultiLineStringM
* ``MultiPolygonM``: MultiPolygonM
* ``GeometryCollectionM``: GeometryCollectionM
* ``CircularStringM``: CircularStringM
* ``CompoundCurveM``: CompoundCurveM
* ``CurvePolygonM``: CurvePolygonM
* ``MultiCurveM``: MultiCurveM
* ``MultiSurfaceM``: MultiSurfaceM
* ``PolyhedralSurfaceM``: PolyhedralSurfaceM
* ``TINM``: TINM
* ``PointZM``: PointZM
* ``LineStringZM``: LineStringZM
* ``PolygonZM``: PolygonZM
* ``MultiPointZM``: MultiPointZM
* ``MultiLineStringZM``: MultiLineStringZM
* ``MultiPolygonZM``: MultiPolygonZM
* ``GeometryCollectionZM``: GeometryCollectionZM
* ``CircularStringZM``: CircularStringZM
* ``CompoundCurveZM``: CompoundCurveZM
* ``CurvePolygonZM``: CurvePolygonZM
* ``MultiCurveZM``: MultiCurveZM
* ``MultiSurfaceZM``: MultiSurfaceZM
* ``PolyhedralSurfaceZM``: PolyhedralSurfaceM
* ``TINZM``: TINZM
* ``TriangleZM``: TriangleZM
* ``Point25D``: Point25D
* ``LineString25D``: LineString25D
* ``Polygon25D``: Polygon25D
* ``MultiPoint25D``: MultiPoint25D
* ``MultiLineString25D``: MultiLineString25D
* ``MultiPolygon25D``: MultiPolygon25D

"""
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
Qgis.GeometryType.__doc__ = """The geometry types are used to group Qgis.WkbType in a
coarse way.

.. note::

   Prior to 3.30 this was available as :py:class:`QgsWkbTypes`.GeometryType.

.. versionadded:: 3.30

* ``Point``: Points

  Available as ``QgsWkbTypes.PointGeometry`` in older QGIS releases.

* ``Line``: Lines

  Available as ``QgsWkbTypes.LineGeometry`` in older QGIS releases.

* ``Polygon``: Polygons

  Available as ``QgsWkbTypes.PolygonGeometry`` in older QGIS releases.

* ``Unknown``: Unknown types

  Available as ``QgsWkbTypes.UnknownGeometry`` in older QGIS releases.

* ``Null``: No geometry

  Available as ``QgsWkbTypes.NullGeometry`` in older QGIS releases.


"""
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
Qgis.DataType.__doc__ = """Raster data types.
This is modified and extended copy of GDALDataType.

* ``UnknownDataType``: Unknown or unspecified type
* ``Byte``: Eight bit unsigned integer (quint8)
* ``Int8``: Eight bit signed integer (qint8) (added in QGIS 3.30)
* ``UInt16``: Sixteen bit unsigned integer (quint16)
* ``Int16``: Sixteen bit signed integer (qint16)
* ``UInt32``: Thirty two bit unsigned integer (quint32)
* ``Int32``: Thirty two bit signed integer (qint32)
* ``Float32``: Thirty two bit floating point (float)
* ``Float64``: Sixty four bit floating point (double)
* ``CInt16``: Complex Int16
* ``CInt32``: Complex Int32
* ``CFloat32``: Complex Float32
* ``CFloat64``: Complex Float64
* ``ARGB32``: Color, alpha, red, green, blue, 4 bytes the same as QImage.Format_ARGB32
* ``ARGB32_Premultiplied``: Color, alpha, red, green, blue, 4 bytes  the same as QImage.Format_ARGB32_Premultiplied

"""
# --
Qgis.DataType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CaptureTechnique.StraightSegments.__doc__ = "Default capture mode - capture occurs with straight line segments"
Qgis.CaptureTechnique.CircularString.__doc__ = "Capture in circular strings"
Qgis.CaptureTechnique.Streaming.__doc__ = "Streaming points digitizing mode (points are automatically added as the mouse cursor moves)."
Qgis.CaptureTechnique.Shape.__doc__ = "Digitize shapes."
Qgis.CaptureTechnique.__doc__ = """Capture technique.

.. versionadded:: 3.26

* ``StraightSegments``: Default capture mode - capture occurs with straight line segments
* ``CircularString``: Capture in circular strings
* ``Streaming``: Streaming points digitizing mode (points are automatically added as the mouse cursor moves).
* ``Shape``: Digitize shapes.

"""
# --
Qgis.CaptureTechnique.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorLayerTypeFlag.SqlQuery.__doc__ = "SQL query layer"
Qgis.VectorLayerTypeFlag.__doc__ = """Vector layer type flags.

.. versionadded:: 3.24

* ``SqlQuery``: SQL query layer

"""
# --
Qgis.VectorLayerTypeFlag.baseClass = Qgis
Qgis.VectorLayerTypeFlags.baseClass = Qgis
VectorLayerTypeFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
Qgis.PythonMacroMode = Qgis.PythonEmbeddedMode
# monkey patching scoped based enum
Qgis.Never = Qgis.PythonEmbeddedMode.Never
Qgis.Never.is_monkey_patched = True
Qgis.Never.__doc__ = "Python embedded never run"
Qgis.Ask = Qgis.PythonEmbeddedMode.Ask
Qgis.Ask.is_monkey_patched = True
Qgis.Ask.__doc__ = "User is prompt before running"
Qgis.SessionOnly = Qgis.PythonEmbeddedMode.SessionOnly
Qgis.SessionOnly.is_monkey_patched = True
Qgis.SessionOnly.__doc__ = "Only during this session"
Qgis.Always = Qgis.PythonEmbeddedMode.Always
Qgis.Always.is_monkey_patched = True
Qgis.Always.__doc__ = "Python embedded is always run"
Qgis.NotForThisSession = Qgis.PythonEmbeddedMode.NotForThisSession
Qgis.NotForThisSession.is_monkey_patched = True
Qgis.NotForThisSession.__doc__ = "Python embedded will not be run for this session"
Qgis.PythonEmbeddedMode.__doc__ = """Authorisation to run Python Embedded in projects

.. versionadded:: 3.40

* ``Never``: Python embedded never run
* ``Ask``: User is prompt before running
* ``SessionOnly``: Only during this session
* ``Always``: Python embedded is always run
* ``NotForThisSession``: Python embedded will not be run for this session

"""
# --
Qgis.PythonEmbeddedMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.PythonEmbeddedType.Macro.__doc__ = ""
Qgis.PythonEmbeddedType.ExpressionFunction.__doc__ = ""
Qgis.PythonEmbeddedType.__doc__ = """Type of Python Embedded in projects

.. versionadded:: 3.40

* ``Macro``: 
* ``ExpressionFunction``: 

"""
# --
Qgis.PythonEmbeddedType.baseClass = Qgis
QgsDataProvider.ReadFlag = Qgis.DataProviderReadFlag
# monkey patching scoped based enum
QgsDataProvider.FlagTrustDataSource = Qgis.DataProviderReadFlag.TrustDataSource
QgsDataProvider.ReadFlag.FlagTrustDataSource = Qgis.DataProviderReadFlag.TrustDataSource
QgsDataProvider.FlagTrustDataSource.is_monkey_patched = True
QgsDataProvider.FlagTrustDataSource.__doc__ = "Trust datasource config (primary key unicity, geometry type and srid, etc). Improves provider load time by skipping expensive checks like primary key unicity, geometry type and srid and by using estimated metadata on data load \n.. versionadded:: 3.16"
QgsDataProvider.SkipFeatureCount = Qgis.DataProviderReadFlag.SkipFeatureCount
QgsDataProvider.SkipFeatureCount.is_monkey_patched = True
QgsDataProvider.SkipFeatureCount.__doc__ = "Make featureCount() return -1 to indicate unknown, and subLayers() to return a unknown feature count as well. Since QGIS 3.18. Only implemented by OGR provider at time of writing."
QgsDataProvider.FlagLoadDefaultStyle = Qgis.DataProviderReadFlag.LoadDefaultStyle
QgsDataProvider.ReadFlag.FlagLoadDefaultStyle = Qgis.DataProviderReadFlag.LoadDefaultStyle
QgsDataProvider.FlagLoadDefaultStyle.is_monkey_patched = True
QgsDataProvider.FlagLoadDefaultStyle.__doc__ = "Reset the layer's style to the default for the datasource"
QgsDataProvider.SkipGetExtent = Qgis.DataProviderReadFlag.SkipGetExtent
QgsDataProvider.SkipGetExtent.is_monkey_patched = True
QgsDataProvider.SkipGetExtent.__doc__ = "Skip the extent from provider"
QgsDataProvider.SkipFullScan = Qgis.DataProviderReadFlag.SkipFullScan
QgsDataProvider.SkipFullScan.is_monkey_patched = True
QgsDataProvider.SkipFullScan.__doc__ = "Skip expensive full scan on files (i.e. on delimited text) \n.. versionadded:: 3.24"
QgsDataProvider.ForceReadOnly = Qgis.DataProviderReadFlag.ForceReadOnly
QgsDataProvider.ForceReadOnly.is_monkey_patched = True
QgsDataProvider.ForceReadOnly.__doc__ = "Open layer in a read-only mode \n.. versionadded:: 3.28"
QgsDataProvider.SkipCredentialsRequest = Qgis.DataProviderReadFlag.SkipCredentialsRequest
QgsDataProvider.SkipCredentialsRequest.is_monkey_patched = True
QgsDataProvider.SkipCredentialsRequest.__doc__ = "Skip credentials if the provided one are not valid, let the provider be invalid, avoiding to block the thread creating the provider if it is not the main thread \n.. versionadded:: 3.32"
QgsDataProvider.ParallelThreadLoading = Qgis.DataProviderReadFlag.ParallelThreadLoading
QgsDataProvider.ParallelThreadLoading.is_monkey_patched = True
QgsDataProvider.ParallelThreadLoading.__doc__ = "Provider is created in a parallel thread than the one where it will live \n.. versionadded:: 3.32.1"
Qgis.DataProviderReadFlag.__doc__ = """Flags which control data provider construction.

.. note::

   Prior to QGIS 3.40 this was available as :py:class:`QgsDataProvider`.ReadFlag

.. versionadded:: 3.40

* ``TrustDataSource``: Trust datasource config (primary key unicity, geometry type and srid, etc). Improves provider load time by skipping expensive checks like primary key unicity, geometry type and srid and by using estimated metadata on data load

  .. versionadded:: 3.16


  Available as ``QgsDataProvider.FlagTrustDataSource`` in older QGIS releases.

* ``SkipFeatureCount``: Make featureCount() return -1 to indicate unknown, and subLayers() to return a unknown feature count as well. Since QGIS 3.18. Only implemented by OGR provider at time of writing.
* ``LoadDefaultStyle``: Reset the layer's style to the default for the datasource

  Available as ``QgsDataProvider.FlagLoadDefaultStyle`` in older QGIS releases.

* ``SkipGetExtent``: Skip the extent from provider
* ``SkipFullScan``: Skip expensive full scan on files (i.e. on delimited text)

  .. versionadded:: 3.24

* ``ForceReadOnly``: Open layer in a read-only mode

  .. versionadded:: 3.28

* ``SkipCredentialsRequest``: Skip credentials if the provided one are not valid, let the provider be invalid, avoiding to block the thread creating the provider if it is not the main thread

  .. versionadded:: 3.32

* ``ParallelThreadLoading``: Provider is created in a parallel thread than the one where it will live

  .. versionadded:: 3.32.1


"""
# --
Qgis.DataProviderReadFlag.baseClass = Qgis
QgsDataProvider.ReadFlags = Qgis.DataProviderReadFlags
Qgis.DataProviderReadFlags.baseClass = Qgis
DataProviderReadFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsVectorDataProvider.Capability = Qgis.VectorProviderCapability
# monkey patching scoped based enum
QgsVectorDataProvider.NoCapabilities = Qgis.VectorProviderCapability.NoCapabilities
QgsVectorDataProvider.NoCapabilities.is_monkey_patched = True
QgsVectorDataProvider.NoCapabilities.__doc__ = "Provider has no capabilities"
QgsVectorDataProvider.AddFeatures = Qgis.VectorProviderCapability.AddFeatures
QgsVectorDataProvider.AddFeatures.is_monkey_patched = True
QgsVectorDataProvider.AddFeatures.__doc__ = "Allows adding features"
QgsVectorDataProvider.DeleteFeatures = Qgis.VectorProviderCapability.DeleteFeatures
QgsVectorDataProvider.DeleteFeatures.is_monkey_patched = True
QgsVectorDataProvider.DeleteFeatures.__doc__ = "Allows deletion of features"
QgsVectorDataProvider.ChangeAttributeValues = Qgis.VectorProviderCapability.ChangeAttributeValues
QgsVectorDataProvider.ChangeAttributeValues.is_monkey_patched = True
QgsVectorDataProvider.ChangeAttributeValues.__doc__ = "Allows modification of attribute values"
QgsVectorDataProvider.AddAttributes = Qgis.VectorProviderCapability.AddAttributes
QgsVectorDataProvider.AddAttributes.is_monkey_patched = True
QgsVectorDataProvider.AddAttributes.__doc__ = "Allows addition of new attributes (fields)"
QgsVectorDataProvider.DeleteAttributes = Qgis.VectorProviderCapability.DeleteAttributes
QgsVectorDataProvider.DeleteAttributes.is_monkey_patched = True
QgsVectorDataProvider.DeleteAttributes.__doc__ = "Allows deletion of attributes (fields)"
QgsVectorDataProvider.CreateSpatialIndex = Qgis.VectorProviderCapability.CreateSpatialIndex
QgsVectorDataProvider.CreateSpatialIndex.is_monkey_patched = True
QgsVectorDataProvider.CreateSpatialIndex.__doc__ = "Allows creation of spatial index"
QgsVectorDataProvider.SelectAtId = Qgis.VectorProviderCapability.SelectAtId
QgsVectorDataProvider.SelectAtId.is_monkey_patched = True
QgsVectorDataProvider.SelectAtId.__doc__ = "Fast access to features using their ID"
QgsVectorDataProvider.ChangeGeometries = Qgis.VectorProviderCapability.ChangeGeometries
QgsVectorDataProvider.ChangeGeometries.is_monkey_patched = True
QgsVectorDataProvider.ChangeGeometries.__doc__ = "Allows modifications of geometries"
QgsVectorDataProvider.SelectEncoding = Qgis.VectorProviderCapability.SelectEncoding
QgsVectorDataProvider.SelectEncoding.is_monkey_patched = True
QgsVectorDataProvider.SelectEncoding.__doc__ = "Allows user to select encoding"
QgsVectorDataProvider.CreateAttributeIndex = Qgis.VectorProviderCapability.CreateAttributeIndex
QgsVectorDataProvider.CreateAttributeIndex.is_monkey_patched = True
QgsVectorDataProvider.CreateAttributeIndex.__doc__ = "Can create indexes on provider's fields"
QgsVectorDataProvider.SimplifyGeometries = Qgis.VectorProviderCapability.SimplifyGeometries
QgsVectorDataProvider.SimplifyGeometries.is_monkey_patched = True
QgsVectorDataProvider.SimplifyGeometries.__doc__ = "Supports simplification of geometries on provider side according to a distance tolerance"
QgsVectorDataProvider.SimplifyGeometriesWithTopologicalValidation = Qgis.VectorProviderCapability.SimplifyGeometriesWithTopologicalValidation
QgsVectorDataProvider.SimplifyGeometriesWithTopologicalValidation.is_monkey_patched = True
QgsVectorDataProvider.SimplifyGeometriesWithTopologicalValidation.__doc__ = "Supports topological simplification of geometries on provider side according to a distance tolerance"
QgsVectorDataProvider.TransactionSupport = Qgis.VectorProviderCapability.TransactionSupport
QgsVectorDataProvider.TransactionSupport.is_monkey_patched = True
QgsVectorDataProvider.TransactionSupport.__doc__ = "Supports transactions"
QgsVectorDataProvider.CircularGeometries = Qgis.VectorProviderCapability.CircularGeometries
QgsVectorDataProvider.CircularGeometries.is_monkey_patched = True
QgsVectorDataProvider.CircularGeometries.__doc__ = "Supports circular geometry types (circularstring, compoundcurve, curvepolygon)"
QgsVectorDataProvider.ChangeFeatures = Qgis.VectorProviderCapability.ChangeFeatures
QgsVectorDataProvider.ChangeFeatures.is_monkey_patched = True
QgsVectorDataProvider.ChangeFeatures.__doc__ = "Supports joint updates for attributes and geometry. Providers supporting this should still define ChangeGeometries | ChangeAttributeValues."
QgsVectorDataProvider.RenameAttributes = Qgis.VectorProviderCapability.RenameAttributes
QgsVectorDataProvider.RenameAttributes.is_monkey_patched = True
QgsVectorDataProvider.RenameAttributes.__doc__ = "Supports renaming attributes (fields) \n.. versionadded:: 2.16"
QgsVectorDataProvider.FastTruncate = Qgis.VectorProviderCapability.FastTruncate
QgsVectorDataProvider.FastTruncate.is_monkey_patched = True
QgsVectorDataProvider.FastTruncate.__doc__ = "Supports fast truncation of the layer (removing all features) \n.. versionadded:: 3.0"
QgsVectorDataProvider.ReadLayerMetadata = Qgis.VectorProviderCapability.ReadLayerMetadata
QgsVectorDataProvider.ReadLayerMetadata.is_monkey_patched = True
QgsVectorDataProvider.ReadLayerMetadata.__doc__ = "Provider can read layer metadata from data store. Since QGIS 3.0. See QgsDataProvider.layerMetadata()"
QgsVectorDataProvider.WriteLayerMetadata = Qgis.VectorProviderCapability.WriteLayerMetadata
QgsVectorDataProvider.WriteLayerMetadata.is_monkey_patched = True
QgsVectorDataProvider.WriteLayerMetadata.__doc__ = "Provider can write layer metadata to the data store. Since QGIS 3.0. See QgsDataProvider.writeLayerMetadata()"
QgsVectorDataProvider.CancelSupport = Qgis.VectorProviderCapability.CancelSupport
QgsVectorDataProvider.CancelSupport.is_monkey_patched = True
QgsVectorDataProvider.CancelSupport.__doc__ = "Supports interruption of pending queries from a separated thread \n.. versionadded:: 3.2"
QgsVectorDataProvider.CreateRenderer = Qgis.VectorProviderCapability.CreateRenderer
QgsVectorDataProvider.CreateRenderer.is_monkey_patched = True
QgsVectorDataProvider.CreateRenderer.__doc__ = "Provider can create feature renderers using backend-specific formatting information. Since QGIS 3.2. See QgsVectorDataProvider.createRenderer()."
QgsVectorDataProvider.CreateLabeling = Qgis.VectorProviderCapability.CreateLabeling
QgsVectorDataProvider.CreateLabeling.is_monkey_patched = True
QgsVectorDataProvider.CreateLabeling.__doc__ = "Provider can set labeling settings using backend-specific formatting information. Since QGIS 3.6. See QgsVectorDataProvider.createLabeling()."
QgsVectorDataProvider.ReloadData = Qgis.VectorProviderCapability.ReloadData
QgsVectorDataProvider.ReloadData.is_monkey_patched = True
QgsVectorDataProvider.ReloadData.__doc__ = "Provider is able to force reload data"
QgsVectorDataProvider.FeatureSymbology = Qgis.VectorProviderCapability.FeatureSymbology
QgsVectorDataProvider.FeatureSymbology.is_monkey_patched = True
QgsVectorDataProvider.FeatureSymbology.__doc__ = "Provider is able retrieve embedded symbology associated with individual features \n.. versionadded:: 3.20"
QgsVectorDataProvider.EditingCapabilities = Qgis.VectorProviderCapability.EditingCapabilities
QgsVectorDataProvider.EditingCapabilities.is_monkey_patched = True
QgsVectorDataProvider.EditingCapabilities.__doc__ = "Bitmask of all editing capabilities"
Qgis.VectorProviderCapability.__doc__ = """Vector data provider capabilities.

.. note::

   Prior to QGIS 3.40 this was available as :py:class:`QgsVectorDataProvider`.Capability

.. versionadded:: 3.40

* ``NoCapabilities``: Provider has no capabilities
* ``AddFeatures``: Allows adding features
* ``DeleteFeatures``: Allows deletion of features
* ``ChangeAttributeValues``: Allows modification of attribute values
* ``AddAttributes``: Allows addition of new attributes (fields)
* ``DeleteAttributes``: Allows deletion of attributes (fields)
* ``CreateSpatialIndex``: Allows creation of spatial index
* ``SelectAtId``: Fast access to features using their ID
* ``ChangeGeometries``: Allows modifications of geometries
* ``SelectEncoding``: Allows user to select encoding
* ``CreateAttributeIndex``: Can create indexes on provider's fields
* ``SimplifyGeometries``: Supports simplification of geometries on provider side according to a distance tolerance
* ``SimplifyGeometriesWithTopologicalValidation``: Supports topological simplification of geometries on provider side according to a distance tolerance
* ``TransactionSupport``: Supports transactions
* ``CircularGeometries``: Supports circular geometry types (circularstring, compoundcurve, curvepolygon)
* ``ChangeFeatures``: Supports joint updates for attributes and geometry. Providers supporting this should still define ChangeGeometries | ChangeAttributeValues.
* ``RenameAttributes``: Supports renaming attributes (fields)

  .. versionadded:: 2.16

* ``FastTruncate``: Supports fast truncation of the layer (removing all features)

  .. versionadded:: 3.0

* ``ReadLayerMetadata``: Provider can read layer metadata from data store. Since QGIS 3.0. See QgsDataProvider.layerMetadata()
* ``WriteLayerMetadata``: Provider can write layer metadata to the data store. Since QGIS 3.0. See QgsDataProvider.writeLayerMetadata()
* ``CancelSupport``: Supports interruption of pending queries from a separated thread

  .. versionadded:: 3.2

* ``CreateRenderer``: Provider can create feature renderers using backend-specific formatting information. Since QGIS 3.2. See QgsVectorDataProvider.createRenderer().
* ``CreateLabeling``: Provider can set labeling settings using backend-specific formatting information. Since QGIS 3.6. See QgsVectorDataProvider.createLabeling().
* ``ReloadData``: Provider is able to force reload data
* ``FeatureSymbology``: Provider is able retrieve embedded symbology associated with individual features

  .. versionadded:: 3.20

* ``EditingCapabilities``: Bitmask of all editing capabilities

"""
# --
Qgis.VectorProviderCapability.baseClass = Qgis
QgsVectorDataProvider.Capabilities = Qgis.VectorProviderCapabilities
Qgis.VectorProviderCapabilities.baseClass = Qgis
VectorProviderCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsVectorDataProvider.FeatureCountState = Qgis.FeatureCountState
# monkey patching scoped based enum
QgsVectorDataProvider.Uncounted = Qgis.FeatureCountState.Uncounted
QgsVectorDataProvider.Uncounted.is_monkey_patched = True
QgsVectorDataProvider.Uncounted.__doc__ = "Feature count not yet computed"
QgsVectorDataProvider.UnknownCount = Qgis.FeatureCountState.UnknownCount
QgsVectorDataProvider.UnknownCount.is_monkey_patched = True
QgsVectorDataProvider.UnknownCount.__doc__ = "Provider returned an unknown feature count"
Qgis.FeatureCountState.__doc__ = """Enumeration of feature count states

.. versionadded:: 3.20

* ``Uncounted``: Feature count not yet computed
* ``UnknownCount``: Provider returned an unknown feature count

"""
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
Qgis.SpatialIndexPresence.__doc__ = """Enumeration of spatial index presence states.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureSource`.SpatialIndexPresence

.. versionadded:: 3.36

* ``Unknown``: Spatial index presence cannot be determined, index may or may not exist

  Available as ``QgsFeatureSource.SpatialIndexUnknown`` in older QGIS releases.

* ``NotPresent``: No spatial index exists for the source

  Available as ``QgsFeatureSource.SpatialIndexNotPresent`` in older QGIS releases.

* ``Present``: A valid spatial index exists for the source

  Available as ``QgsFeatureSource.SpatialIndexPresent`` in older QGIS releases.


"""
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
Qgis.FeatureAvailability.__doc__ = """Possible return value for :py:func:`QgsFeatureSource.hasFeatures()` to determine if a source is empty.

It is implemented as a three-value logic, so it can return if
there are features available for sure, if there are no features
available for sure or if there might be features available but
there is no guarantee for this.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureSource`.FeatureAvailability

.. versionadded:: 3.36

* ``NoFeaturesAvailable``: There are certainly no features available in this source
* ``FeaturesAvailable``: There is at least one feature available in this source
* ``FeaturesMaybeAvailable``: There may be features available in this source

"""
# --
Qgis.FeatureAvailability.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorDataProviderAttributeEditCapability.EditAlias.__doc__ = "Allows editing aliases"
Qgis.VectorDataProviderAttributeEditCapability.EditComment.__doc__ = "Allows editing comments"
Qgis.VectorDataProviderAttributeEditCapability.__doc__ = """Attribute editing capabilities which may be supported by vector data providers.

.. versionadded:: 3.32

* ``EditAlias``: Allows editing aliases
* ``EditComment``: Allows editing comments

"""
# --
Qgis.VectorDataProviderAttributeEditCapability.baseClass = Qgis
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
Qgis.SymbolType.__doc__ = """Symbol types

.. versionadded:: 3.20

* ``Marker``: Marker symbol
* ``Line``: Line symbol
* ``Fill``: Fill symbol
* ``Hybrid``: Hybrid symbol

"""
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
Qgis.ScaleMethod.__doc__ = """Scale methods

.. versionadded:: 3.20

* ``ScaleArea``: Calculate scale by the area
* ``ScaleDiameter``: Calculate scale by the diameter

"""
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
Qgis.SettingsType.__doc__ = """Types of settings entries

.. versionadded:: 3.26

* ``Custom``: Custom implementation
* ``Variant``: Generic variant
* ``String``: String
* ``StringList``: List of strings
* ``VariantMap``: Map of strings
* ``Bool``: Boolean
* ``Integer``: Integer
* ``Double``: Double precision number
* ``EnumFlag``: Enum or Flag
* ``Color``: Color

"""
# --
Qgis.SettingsType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SettingsTreeNodeType.Root.__doc__ = "Root Node"
Qgis.SettingsTreeNodeType.Standard.__doc__ = "Normal Node"
Qgis.SettingsTreeNodeType.NamedList.__doc__ = "Named List Node"
Qgis.SettingsTreeNodeType.__doc__ = """Type of tree node

.. versionadded:: 3.30

* ``Root``: Root Node
* ``Standard``: Normal Node
* ``NamedList``: Named List Node

"""
# --
Qgis.SettingsTreeNodeType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SettingsTreeNodeOption.NamedListSelectedItemSetting.__doc__ = "Creates a setting to store which is the current item"
Qgis.SettingsTreeNodeOption.__doc__ = """Options for named list nodes

.. versionadded:: 3.30

* ``NamedListSelectedItemSetting``: Creates a setting to store which is the current item

"""
# --
Qgis.SettingsTreeNodeOption.baseClass = Qgis
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
Qgis.PropertyType.__doc__ = """Property types

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProperty`.Type

.. versionadded:: 3.36

* ``Invalid``: Invalid (not set) property

  Available as ``QgsProperty.InvalidProperty`` in older QGIS releases.

* ``Static``: Static property

  Available as ``QgsProperty.StaticProperty`` in older QGIS releases.

* ``Field``: Field based property

  Available as ``QgsProperty.FieldBasedProperty`` in older QGIS releases.

* ``Expression``: Expression based property

  Available as ``QgsProperty.ExpressionBasedProperty`` in older QGIS releases.


"""
# --
Qgis.PropertyType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SldExportOption.NoOptions.__doc__ = "Default SLD export"
Qgis.SldExportOption.Svg.__doc__ = "Export complex styles to separate SVG files for better compatibility with OGC servers"
Qgis.SldExportOption.Png.__doc__ = "Export complex styles to separate PNG files for better compatibility with OGC servers"
Qgis.SldExportOption.__doc__ = """SLD export options

.. versionadded:: 3.30

* ``NoOptions``: Default SLD export
* ``Svg``: Export complex styles to separate SVG files for better compatibility with OGC servers
* ``Png``: Export complex styles to separate PNG files for better compatibility with OGC servers

"""
# --
Qgis.SldExportOption.baseClass = Qgis
Qgis.SldExportOptions.baseClass = Qgis
SldExportOptions = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SldExportVendorExtension.NoVendorExtension.__doc__ = "No vendor extensions"
Qgis.SldExportVendorExtension.GeoServerVendorExtension.__doc__ = "Use GeoServer vendor extensions when required"
Qgis.SldExportVendorExtension.DeegreeVendorExtension.__doc__ = "Use Deegree vendor extensions when required"
Qgis.SldExportVendorExtension.__doc__ = """SLD export vendor extensions, allow the use of vendor extensions when exporting to SLD.

.. versionadded:: 3.30

* ``NoVendorExtension``: No vendor extensions
* ``GeoServerVendorExtension``: Use GeoServer vendor extensions when required
* ``DeegreeVendorExtension``: Use Deegree vendor extensions when required

"""
# --
Qgis.SldExportVendorExtension.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SettingsOption.SaveFormerValue.__doc__ = "Save the former value of the settings"
Qgis.SettingsOption.SaveEnumFlagAsInt.__doc__ = "The enum/flag will be saved as an integer value instead of text"
Qgis.SettingsOption.__doc__ = """Settings options

.. versionadded:: 3.26

* ``SaveFormerValue``: Save the former value of the settings
* ``SaveEnumFlagAsInt``: The enum/flag will be saved as an integer value instead of text

"""
# --
Qgis.SettingsOption.baseClass = Qgis
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
Qgis.SnappingMode.__doc__ = """SnappingMode defines on which layer the snapping is performed

.. versionadded:: 3.26

* ``ActiveLayer``: On the active layer
* ``AllLayers``: On all vector layers
* ``AdvancedConfiguration``: On a per layer configuration basis

"""
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
QgsSnappingConfig.LineEndpointFlag.__doc__ = "Start or end points of lines, or first vertex in polygon rings only \n.. versionadded:: 3.20"
Qgis.SnappingType.__doc__ = """SnappingTypeFlag defines on what object the snapping is performed

.. versionadded:: 3.26

* ``NoSnap``: No snapping

  Available as ``QgsSnappingConfig.NoSnapFlag`` in older QGIS releases.

* ``Vertex``: On vertices

  Available as ``QgsSnappingConfig.VertexFlag`` in older QGIS releases.

* ``Segment``: On segments

  Available as ``QgsSnappingConfig.SegmentFlag`` in older QGIS releases.

* ``Area``: On Area

  Available as ``QgsSnappingConfig.AreaFlag`` in older QGIS releases.

* ``Centroid``: On centroid

  Available as ``QgsSnappingConfig.CentroidFlag`` in older QGIS releases.

* ``MiddleOfSegment``: On Middle segment

  Available as ``QgsSnappingConfig.MiddleOfSegmentFlag`` in older QGIS releases.

* ``LineEndpoint``: Start or end points of lines, or first vertex in polygon rings only

  .. versionadded:: 3.20


  Available as ``QgsSnappingConfig.LineEndpointFlag`` in older QGIS releases.


"""
# --
Qgis.SnappingType.baseClass = Qgis
QgsSnappingConfig.SnappingTypeFlag = Qgis.SnappingTypes
Qgis.SnappingTypes.baseClass = Qgis
SnappingTypes = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsSymbol.RenderHint = Qgis.SymbolRenderHint
# monkey patching scoped based enum
QgsSymbol.DynamicRotation = Qgis.SymbolRenderHint.DynamicRotation
QgsSymbol.DynamicRotation.is_monkey_patched = True
QgsSymbol.DynamicRotation.__doc__ = "Rotation of symbol may be changed during rendering and symbol should not be cached"
QgsSymbol.IsSymbolLayerSubSymbol = Qgis.SymbolRenderHint.IsSymbolLayerSubSymbol
QgsSymbol.IsSymbolLayerSubSymbol.is_monkey_patched = True
QgsSymbol.IsSymbolLayerSubSymbol.__doc__ = "Symbol is being rendered as a sub-symbol of a QgsSymbolLayer \n.. versionadded:: 3.38"
QgsSymbol.ForceVectorRendering = Qgis.SymbolRenderHint.ForceVectorRendering
QgsSymbol.ForceVectorRendering.is_monkey_patched = True
QgsSymbol.ForceVectorRendering.__doc__ = "Symbol must be rendered using vector methods, and optimisations like pre-rendered images must be disabled \n.. versionadded:: 3.40"
QgsSymbol.ExcludeSymbolBuffers = Qgis.SymbolRenderHint.ExcludeSymbolBuffers
QgsSymbol.ExcludeSymbolBuffers.is_monkey_patched = True
QgsSymbol.ExcludeSymbolBuffers.__doc__ = "Do not render symbol buffers. \n.. versionadded:: 3.40"
Qgis.SymbolRenderHint.__doc__ = """Flags controlling behavior of symbols during rendering

.. versionadded:: 3.20

* ``DynamicRotation``: Rotation of symbol may be changed during rendering and symbol should not be cached
* ``IsSymbolLayerSubSymbol``: Symbol is being rendered as a sub-symbol of a QgsSymbolLayer

  .. versionadded:: 3.38

* ``ForceVectorRendering``: Symbol must be rendered using vector methods, and optimisations like pre-rendered images must be disabled

  .. versionadded:: 3.40

* ``ExcludeSymbolBuffers``: Do not render symbol buffers.

  .. versionadded:: 3.40


"""
# --
Qgis.SymbolRenderHint.baseClass = Qgis
QgsSymbol.RenderHints = Qgis.SymbolRenderHints
Qgis.SymbolRenderHints.baseClass = Qgis
SymbolRenderHints = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SymbolRotationMode.RespectMapRotation.__doc__ = "Entity is rotated along with the map"
Qgis.SymbolRotationMode.IgnoreMapRotation.__doc__ = "Entity ignores map rotation"
Qgis.SymbolRotationMode.__doc__ = """Modes for handling how symbol and text entity rotation is handled when maps are rotated.

.. versionadded:: 3.32

* ``RespectMapRotation``: Entity is rotated along with the map
* ``IgnoreMapRotation``: Entity ignores map rotation

"""
# --
Qgis.SymbolRotationMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FeatureRendererFlag.AffectsLabeling.__doc__ = "If present, indicates that the renderer will participate in the map labeling problem"
Qgis.FeatureRendererFlag.__doc__ = """Flags controlling behavior of vector feature renderers.

.. versionadded:: 3.40

* ``AffectsLabeling``: If present, indicates that the renderer will participate in the map labeling problem

"""
# --
Qgis.FeatureRendererFlag.baseClass = Qgis
Qgis.FeatureRendererFlags.baseClass = Qgis
FeatureRendererFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SymbolFlag.RendererShouldUseSymbolLevels.__doc__ = "If present, indicates that a QgsFeatureRenderer using the symbol should use symbol levels for best results"
Qgis.SymbolFlag.AffectsLabeling.__doc__ = "If present, indicates that the symbol will participate in the map labeling problem \n.. versionadded:: 3.40"
Qgis.SymbolFlag.__doc__ = """Flags controlling behavior of symbols

.. versionadded:: 3.20

* ``RendererShouldUseSymbolLevels``: If present, indicates that a QgsFeatureRenderer using the symbol should use symbol levels for best results
* ``AffectsLabeling``: If present, indicates that the symbol will participate in the map labeling problem

  .. versionadded:: 3.40


"""
# --
Qgis.SymbolFlag.baseClass = Qgis
Qgis.SymbolFlags.baseClass = Qgis
SymbolFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsSymbol.PreviewFlag = Qgis.SymbolPreviewFlag
# monkey patching scoped based enum
QgsSymbol.FlagIncludeCrosshairsForMarkerSymbols = Qgis.SymbolPreviewFlag.FlagIncludeCrosshairsForMarkerSymbols
QgsSymbol.FlagIncludeCrosshairsForMarkerSymbols.is_monkey_patched = True
QgsSymbol.FlagIncludeCrosshairsForMarkerSymbols.__doc__ = "Include a crosshairs reference image in the background of marker symbol previews"
Qgis.SymbolPreviewFlag.__doc__ = """Flags for controlling how symbol preview images are generated.

.. versionadded:: 3.20

* ``FlagIncludeCrosshairsForMarkerSymbols``: Include a crosshairs reference image in the background of marker symbol previews

"""
# --
Qgis.SymbolPreviewFlag.baseClass = Qgis
QgsSymbol.SymbolPreviewFlags = Qgis.SymbolPreviewFlags
Qgis.SymbolPreviewFlags.baseClass = Qgis
SymbolPreviewFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SymbolLayerFlag.DisableFeatureClipping.__doc__ = "If present, indicates that features should never be clipped to the map extent during rendering"
Qgis.SymbolLayerFlag.CanCalculateMaskGeometryPerFeature.__doc__ = "If present, indicates that mask geometry can safely be calculated per feature for the symbol layer. This avoids using the entire symbol layer's mask geometry for every feature rendered, considerably simplifying vector exports and resulting in much smaller file sizes. \n.. versionadded:: 3.38"
Qgis.SymbolLayerFlag.AffectsLabeling.__doc__ = "If present, indicates that the symbol layer will participate in the map labeling problem \n.. versionadded:: 3.40"
Qgis.SymbolLayerFlag.__doc__ = """Flags controlling behavior of symbol layers

.. note::

   These differ from Qgis.SymbolLayerUserFlag in that Qgis.SymbolLayerFlag flags are used to reflect the inbuilt properties
   of a symbol layer type, whereas Qgis.SymbolLayerUserFlag are optional, user controlled flags which can be toggled
   for a symbol layer.

.. versionadded:: 3.22

* ``DisableFeatureClipping``: If present, indicates that features should never be clipped to the map extent during rendering
* ``CanCalculateMaskGeometryPerFeature``: If present, indicates that mask geometry can safely be calculated per feature for the symbol layer. This avoids using the entire symbol layer's mask geometry for every feature rendered, considerably simplifying vector exports and resulting in much smaller file sizes.

  .. versionadded:: 3.38

* ``AffectsLabeling``: If present, indicates that the symbol layer will participate in the map labeling problem

  .. versionadded:: 3.40


"""
# --
Qgis.SymbolLayerFlag.baseClass = Qgis
Qgis.SymbolLayerFlags.baseClass = Qgis
SymbolLayerFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SymbolLayerUserFlag.DisableSelectionRecoloring.__doc__ = "If present, indicates that the symbol layer should not be recolored when rendering selected features"
Qgis.SymbolLayerUserFlag.__doc__ = """User-specified flags controlling behavior of symbol layers.

.. note::

   These differ from Qgis.SymbolLayerFlag in that Qgis.SymbolLayerFlag flags are used to reflect the inbuilt properties
   of a symbol layer type, whereas Qgis.SymbolLayerUserFlag are optional, user controlled flags which can be toggled
   for a symbol layer.

.. versionadded:: 3.34

* ``DisableSelectionRecoloring``: If present, indicates that the symbol layer should not be recolored when rendering selected features

"""
# --
Qgis.SymbolLayerUserFlag.baseClass = Qgis
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
Qgis.BrowserItemType.__doc__ = """Browser item types.

.. versionadded:: 3.20

* ``Collection``: A collection of items
* ``Directory``: Represents a file directory
* ``Layer``: Represents a map layer
* ``Error``: Contains an error message
* ``Favorites``: Represents a favorite item
* ``Project``: Represents a QGIS project
* ``Custom``: Custom item type
* ``Fields``: Collection of fields
* ``Field``: Vector layer field

"""
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
Qgis.BrowserItemState.__doc__ = """Browser item states.

.. versionadded:: 3.20

* ``NotPopulated``: Children not yet created
* ``Populating``: Creating children in separate thread (populating or refreshing)
* ``Populated``: Children created

"""
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
QgsDataItem.ItemRepresentsFile.__doc__ = "Item's path() directly represents a file on disk \n.. versionadded:: 3.22"
QgsDataItem.RefreshChildrenWhenItemIsRefreshed = Qgis.BrowserItemCapability.RefreshChildrenWhenItemIsRefreshed
QgsDataItem.RefreshChildrenWhenItemIsRefreshed.is_monkey_patched = True
QgsDataItem.RefreshChildrenWhenItemIsRefreshed.__doc__ = "When the item is refreshed, all its populated children will also be refreshed in turn \n.. versionadded:: 3.26"
QgsDataItem.ReadOnly = Qgis.BrowserItemCapability.ReadOnly
QgsDataItem.ReadOnly.is_monkey_patched = True
QgsDataItem.ReadOnly.__doc__ = "Item is read only \n.. versionadded:: 3.40"
Qgis.BrowserItemCapability.__doc__ = """Browser item capabilities.

.. versionadded:: 3.20

* ``NoCapabilities``: Item has no capabilities
* ``SetCrs``: Can set CRS on layer or group of layers. deprecated since QGIS 3.6 -- no longer used by QGIS and will be removed in QGIS 4.0
* ``Fertile``: Can create children. Even items without this capability may have children, but cannot create them, it means that children are created by item ancestors.
* ``Fast``: CreateChildren() is fast enough to be run in main thread when refreshing items, most root items (wms,wfs,wcs,postgres...) are considered fast because they are reading data only from QgsSettings
* ``Collapse``: The collapse/expand status for this items children should be ignored in order to avoid undesired network connections (wms etc.)
* ``Rename``: Item can be renamed
* ``Delete``: Item can be deleted
* ``ItemRepresentsFile``: Item's path() directly represents a file on disk

  .. versionadded:: 3.22

* ``RefreshChildrenWhenItemIsRefreshed``: When the item is refreshed, all its populated children will also be refreshed in turn

  .. versionadded:: 3.26

* ``ReadOnly``: Item is read only

  .. versionadded:: 3.40


"""
# --
Qgis.BrowserItemCapability.baseClass = Qgis
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
Qgis.DataItemProviderCapability.__doc__ = """Capabilities for data item providers.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsDataProvider`.DataCapability

.. versionadded:: 3.36

* ``NoCapabilities``: No capabilities

  Available as ``QgsDataProvider.NoDataCapabilities`` in older QGIS releases.

* ``Files``: Can provides items which corresponds to files

  Available as ``QgsDataProvider.File`` in older QGIS releases.

* ``Directories``: Can provides items which corresponds to directories

  Available as ``QgsDataProvider.Dir`` in older QGIS releases.

* ``Databases``: Can provides items which corresponds to databases

  Available as ``QgsDataProvider.Database`` in older QGIS releases.

* ``NetworkSources``: Network/internet source

  Available as ``QgsDataProvider.Net`` in older QGIS releases.


"""
# --
Qgis.DataItemProviderCapability.baseClass = Qgis
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
QgsLayerItem.TiledScene.__doc__ = "Tiled scene layer \n.. versionadded:: 3.34"
Qgis.BrowserLayerType.__doc__ = """Browser item layer types

.. versionadded:: 3.20

* ``NoType``: No type
* ``Vector``: Generic vector layer
* ``Raster``: Raster layer
* ``Point``: Vector point layer
* ``Line``: Vector line layer
* ``Polygon``: Vector polygon layer
* ``TableLayer``: Vector non-spatial layer
* ``Database``: Database layer
* ``Table``: Database table
* ``Plugin``: Plugin based layer
* ``Mesh``: Mesh layer
* ``VectorTile``: Vector tile layer
* ``PointCloud``: Point cloud layer
* ``TiledScene``: Tiled scene layer

  .. versionadded:: 3.34


"""
# --
Qgis.BrowserLayerType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.BrowserDirectoryMonitoring.Default.__doc__ = "Use default logic to determine whether directory should be monitored"
Qgis.BrowserDirectoryMonitoring.NeverMonitor.__doc__ = "Never monitor the directory, regardless of the default logic"
Qgis.BrowserDirectoryMonitoring.AlwaysMonitor.__doc__ = "Always monitor the directory, regardless of the default logic"
Qgis.BrowserDirectoryMonitoring.__doc__ = """Browser directory item monitoring switches.

.. versionadded:: 3.20

* ``Default``: Use default logic to determine whether directory should be monitored
* ``NeverMonitor``: Never monitor the directory, regardless of the default logic
* ``AlwaysMonitor``: Always monitor the directory, regardless of the default logic

"""
# --
Qgis.BrowserDirectoryMonitoring.baseClass = Qgis
# monkey patching scoped based enum
Qgis.HttpMethod.Get.__doc__ = "GET method"
Qgis.HttpMethod.Post.__doc__ = "POST method"
Qgis.HttpMethod.__doc__ = """Different methods of HTTP requests

.. versionadded:: 3.22

* ``Get``: GET method
* ``Post``: POST method

"""
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
Qgis.VectorExportResult.__doc__ = """Vector layer export result codes.

.. versionadded:: 3.20

* ``Success``: No errors were encountered

  Available as ``QgsVectorLayerExporter.NoError`` in older QGIS releases.

* ``ErrorCreatingDataSource``: Could not create the destination data source

  Available as ``QgsVectorLayerExporter.ErrCreateDataSource`` in older QGIS releases.

* ``ErrorCreatingLayer``: Could not create destination layer

  Available as ``QgsVectorLayerExporter.ErrCreateLayer`` in older QGIS releases.

* ``ErrorAttributeTypeUnsupported``: Source layer has an attribute type which could not be handled by destination

  Available as ``QgsVectorLayerExporter.ErrAttributeTypeUnsupported`` in older QGIS releases.

* ``ErrorAttributeCreationFailed``: Destination provider was unable to create an attribute

  Available as ``QgsVectorLayerExporter.ErrAttributeCreationFailed`` in older QGIS releases.

* ``ErrorProjectingFeatures``: An error occurred while reprojecting features to destination CRS

  Available as ``QgsVectorLayerExporter.ErrProjection`` in older QGIS releases.

* ``ErrorFeatureWriteFailed``: An error occurred while writing a feature to the destination

  Available as ``QgsVectorLayerExporter.ErrFeatureWriteFailed`` in older QGIS releases.

* ``ErrorInvalidLayer``: Could not access newly created destination layer

  Available as ``QgsVectorLayerExporter.ErrInvalidLayer`` in older QGIS releases.

* ``ErrorInvalidProvider``: Could not find a matching provider key

  Available as ``QgsVectorLayerExporter.ErrInvalidProvider`` in older QGIS releases.

* ``ErrorProviderUnsupportedFeature``: Provider does not support creation of empty layers

  Available as ``QgsVectorLayerExporter.ErrProviderUnsupportedFeature`` in older QGIS releases.

* ``ErrorConnectionFailed``: Could not connect to destination

  Available as ``QgsVectorLayerExporter.ErrConnectionFailed`` in older QGIS releases.

* ``UserCanceled``: User canceled the export

  Available as ``QgsVectorLayerExporter.ErrUserCanceled`` in older QGIS releases.


"""
# --
Qgis.VectorExportResult.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorFileWriterCapability.FieldAliases.__doc__ = "Writer can support field aliases"
Qgis.VectorFileWriterCapability.FieldComments.__doc__ = "Writer can support field comments"
Qgis.VectorFileWriterCapability.__doc__ = """Capabilities supported by a :py:class:`QgsVectorFileWriter` object.

.. versionadded:: 3.32

* ``FieldAliases``: Writer can support field aliases
* ``FieldComments``: Writer can support field comments

"""
# --
Qgis.VectorFileWriterCapability.baseClass = Qgis
Qgis.VectorFileWriterCapabilities.baseClass = Qgis
VectorFileWriterCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SqlLayerDefinitionCapability.SubsetStringFilter.__doc__ = "SQL layer definition supports subset string filter"
Qgis.SqlLayerDefinitionCapability.GeometryColumn.__doc__ = "SQL layer definition supports geometry column"
Qgis.SqlLayerDefinitionCapability.PrimaryKeys.__doc__ = "SQL layer definition supports primary keys"
Qgis.SqlLayerDefinitionCapability.UnstableFeatureIds.__doc__ = "SQL layer definition supports disabling select at id"
Qgis.SqlLayerDefinitionCapability.__doc__ = """SqlLayerDefinitionCapability enum lists the arguments supported by the provider when creating SQL query layers.

.. versionadded:: 3.22

* ``SubsetStringFilter``: SQL layer definition supports subset string filter
* ``GeometryColumn``: SQL layer definition supports geometry column
* ``PrimaryKeys``: SQL layer definition supports primary keys
* ``UnstableFeatureIds``: SQL layer definition supports disabling select at id

"""
# --
Qgis.SqlLayerDefinitionCapability.baseClass = Qgis
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
Qgis.SqlKeywordCategory.__doc__ = """SqlKeywordCategory enum represents the categories of the SQL keywords used by the SQL query editor.

.. note::

   The category has currently no usage, but it was planned for future uses.

.. versionadded:: 3.22

* ``Keyword``: SQL keyword
* ``Constant``: SQL constant
* ``Function``: SQL generic function
* ``Geospatial``: SQL spatial function
* ``Operator``: SQL operator
* ``Math``: SQL math function
* ``Aggregate``: SQL aggregate function
* ``String``: SQL string function
* ``Identifier``: SQL identifier

"""
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
Qgis.DriveType.__doc__ = """Drive types

.. versionadded:: 3.20

* ``Unknown``: Unknown type
* ``Invalid``: Invalid path
* ``Removable``: Removable drive
* ``Fixed``: Fixed drive
* ``Remote``: Remote drive
* ``CdRom``: CD-ROM
* ``RamDisk``: RAM disk
* ``Cloud``: Cloud storage -- files may be remote or locally stored, depending on user configuration

"""
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
Qgis.ActionStart.__doc__ = """Enum to determine when an operation would begin

.. versionadded:: 3.22

* ``Deferred``: Do not start immediately the action

  Available as ``QgsNetworkContentFetcherRegistry.DownloadLater`` in older QGIS releases.

* ``Immediate``: Action will start immediately

  Available as ``QgsNetworkContentFetcherRegistry.DownloadImmediately`` in older QGIS releases.


"""
# --
Qgis.ActionStart.baseClass = Qgis
# monkey patching scoped based enum
Qgis.UnplacedLabelVisibility.FollowEngineSetting.__doc__ = "Respect the label engine setting"
Qgis.UnplacedLabelVisibility.NeverShow.__doc__ = "Never show unplaced labels, regardless of the engine setting"
Qgis.UnplacedLabelVisibility.__doc__ = """Unplaced label visibility.

.. versionadded:: 3.20

* ``FollowEngineSetting``: Respect the label engine setting
* ``NeverShow``: Never show unplaced labels, regardless of the engine setting

"""
# --
Qgis.UnplacedLabelVisibility.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LabelOverlapHandling.PreventOverlap.__doc__ = "Do not allow labels to overlap other labels"
Qgis.LabelOverlapHandling.AllowOverlapIfRequired.__doc__ = "Avoids overlapping labels when possible, but permit overlaps if labels for features cannot otherwise be placed"
Qgis.LabelOverlapHandling.AllowOverlapAtNoCost.__doc__ = "Labels may freely overlap other labels, at no cost"
Qgis.LabelOverlapHandling.__doc__ = """Label overlap handling.

.. versionadded:: 3.26

* ``PreventOverlap``: Do not allow labels to overlap other labels
* ``AllowOverlapIfRequired``: Avoids overlapping labels when possible, but permit overlaps if labels for features cannot otherwise be placed
* ``AllowOverlapAtNoCost``: Labels may freely overlap other labels, at no cost

"""
# --
Qgis.LabelOverlapHandling.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LabelPrioritization.PreferCloser.__doc__ = "Prefer closer labels, falling back to alternate positions before larger distances"
Qgis.LabelPrioritization.PreferPositionOrdering.__doc__ = "Prefer labels follow position ordering, falling back to more distance labels before alternate positions"
Qgis.LabelPrioritization.__doc__ = """Label prioritization.

.. versionadded:: 3.38

* ``PreferCloser``: Prefer closer labels, falling back to alternate positions before larger distances
* ``PreferPositionOrdering``: Prefer labels follow position ordering, falling back to more distance labels before alternate positions

"""
# --
Qgis.LabelPrioritization.baseClass = Qgis
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
QgsPalLayerSettings.OutsidePolygons.__doc__ = "Candidates are placed outside of polygon boundaries. Applies to polygon layers only \n.. versionadded:: 3.14"
Qgis.LabelPlacement.__doc__ = """Placement modes which determine how label candidates are generated for a feature.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.Placement

.. versionadded:: 3.26

* ``AroundPoint``: Arranges candidates in a circle around a point (or centroid of a polygon). Applies to point or polygon layers only.
* ``OverPoint``: Arranges candidates over a point (or centroid of a polygon), or at a preset offset from the point. Applies to point or polygon layers only.
* ``Line``: Arranges candidates parallel to a generalised line representing the feature or parallel to a polygon's perimeter. Applies to line or polygon layers only.
* ``Curved``: Arranges candidates following the curvature of a line feature. Applies to line layers only.
* ``Horizontal``: Arranges horizontal candidates scattered throughout a polygon feature. Applies to polygon layers only.
* ``Free``: Arranges candidates scattered throughout a polygon feature. Candidates are rotated to respect the polygon's orientation. Applies to polygon layers only.
* ``OrderedPositionsAroundPoint``: Candidates are placed in predefined positions around a point. Preference is given to positions with greatest cartographic appeal, e.g., top right, bottom right, etc. Applies to point layers only.
* ``PerimeterCurved``: Arranges candidates following the curvature of a polygon's boundary. Applies to polygon layers only.
* ``OutsidePolygons``: Candidates are placed outside of polygon boundaries. Applies to polygon layers only

  .. versionadded:: 3.14


"""
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
QgsPalLayerSettings.OverPoint = Qgis.LabelPredefinedPointPosition.OverPoint
QgsPalLayerSettings.OverPoint.is_monkey_patched = True
QgsPalLayerSettings.OverPoint.__doc__ = "Label directly centered over point \n.. versionadded:: 3.38"
Qgis.LabelPredefinedPointPosition.__doc__ = """Positions for labels when using the Qgis.LabelPlacement.OrderedPositionsAroundPoint placement mode.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.PredefinedPointPosition

.. versionadded:: 3.26

* ``TopLeft``: Label on top-left of point
* ``TopSlightlyLeft``: Label on top of point, slightly left of center
* ``TopMiddle``: Label directly above point
* ``TopSlightlyRight``: Label on top of point, slightly right of center
* ``TopRight``: Label on top-right of point
* ``MiddleLeft``: Label on left of point
* ``MiddleRight``: Label on right of point
* ``BottomLeft``: Label on bottom-left of point
* ``BottomSlightlyLeft``: Label below point, slightly left of center
* ``BottomMiddle``: Label directly below point
* ``BottomSlightlyRight``: Label below point, slightly right of center
* ``BottomRight``: Label on bottom right of point
* ``OverPoint``: Label directly centered over point

  .. versionadded:: 3.38


"""
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
Qgis.LabelOffsetType.__doc__ = """Behavior modifier for label offset and distance, only applies in some
label placement modes.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.OffsetType

.. versionadded:: 3.26

* ``FromPoint``: Offset distance applies from point geometry
* ``FromSymbolBounds``: Offset distance applies from rendered symbol bounds

"""
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
QgsPalLayerSettings.QuadrantBelowRight.__doc__ = "Below right"
Qgis.LabelQuadrantPosition.__doc__ = """Label quadrant positions

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.QuadrantPosition

.. versionadded:: 3.26

* ``AboveLeft``: Above left

  Available as ``QgsPalLayerSettings.QuadrantAboveLeft`` in older QGIS releases.

* ``Above``: Above center

  Available as ``QgsPalLayerSettings.QuadrantAbove`` in older QGIS releases.

* ``AboveRight``: Above right

  Available as ``QgsPalLayerSettings.QuadrantAboveRight`` in older QGIS releases.

* ``Left``: Left middle

  Available as ``QgsPalLayerSettings.QuadrantLeft`` in older QGIS releases.

* ``Over``: Center middle

  Available as ``QgsPalLayerSettings.QuadrantOver`` in older QGIS releases.

* ``Right``: Right middle

  Available as ``QgsPalLayerSettings.QuadrantRight`` in older QGIS releases.

* ``BelowLeft``: Below left

  Available as ``QgsPalLayerSettings.QuadrantBelowLeft`` in older QGIS releases.

* ``Below``: Below center

  Available as ``QgsPalLayerSettings.QuadrantBelow`` in older QGIS releases.

* ``BelowRight``: Below right

  Available as ``QgsPalLayerSettings.QuadrantBelowRight`` in older QGIS releases.


"""
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
Qgis.LabelLinePlacementFlag.__doc__ = """Line placement flags, which control how candidates are generated for a linear feature.

.. note::

   Prior to QGIS 3.32 this was available as :py:class:`QgsLabeling`.LinePlacementFlag

.. versionadded:: 3.32

* ``OnLine``: Labels can be placed directly over a line feature.
* ``AboveLine``: Labels can be placed above a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed below the line feature.
* ``BelowLine``: Labels can be placed below a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed above the line feature.
* ``MapOrientation``: Signifies that the AboveLine and BelowLine flags should respect the map's orientation rather than the feature's orientation. For example, AboveLine will always result in label's being placed above a line, regardless of the line's direction.

"""
# --
Qgis.LabelLinePlacementFlag.baseClass = Qgis
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
Qgis.LabelPolygonPlacementFlag.__doc__ = """Polygon placement flags, which control how candidates are generated for a polygon feature.

.. note::

   Prior to QGIS 3.32 this was available as :py:class:`QgsLabeling`.PolygonPlacementFlag

.. versionadded:: 3.32

* ``AllowPlacementOutsideOfPolygon``: Labels can be placed outside of a polygon feature
* ``AllowPlacementInsideOfPolygon``: Labels can be placed inside a polygon feature

"""
# --
Qgis.LabelPolygonPlacementFlag.baseClass = Qgis
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
Qgis.UpsideDownLabelHandling.__doc__ = """Handling techniques for upside down labels.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.UpsideDownLabels

.. versionadded:: 3.26

* ``FlipUpsideDownLabels``: Upside-down labels (90 <= angle < 270) are shown upright

  Available as ``QgsPalLayerSettings.Upright`` in older QGIS releases.

* ``AllowUpsideDownWhenRotationIsDefined``: Show upside down when rotation is layer- or data-defined

  Available as ``QgsPalLayerSettings.ShowDefined`` in older QGIS releases.

* ``AlwaysAllowUpsideDown``: Show upside down for all labels, including dynamic ones

  Available as ``QgsPalLayerSettings.ShowAll`` in older QGIS releases.


"""
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
Qgis.LabelMultiLineAlignment.__doc__ = """Text alignment for multi-line labels.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsPalLayerSettings`.MultiLineAlign

.. versionadded:: 3.26

* ``Left``: Left align

  Available as ``QgsPalLayerSettings.MultiLeft`` in older QGIS releases.

* ``Center``: Center align

  Available as ``QgsPalLayerSettings.MultiCenter`` in older QGIS releases.

* ``Right``: Right align

  Available as ``QgsPalLayerSettings.MultiRight`` in older QGIS releases.

* ``FollowPlacement``: Alignment follows placement of label, e.g., labels to the left of a feature will be drawn with right alignment

  Available as ``QgsPalLayerSettings.MultiFollowPlacement`` in older QGIS releases.

* ``Justify``: Justified

  Available as ``QgsPalLayerSettings.MultiJustify`` in older QGIS releases.


"""
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
QgsProviderMetadata.FilterPointCloud.__doc__ = "Point clouds \n.. versionadded:: 3.18"
QgsProviderMetadata.VectorTile = Qgis.FileFilterType.VectorTile
QgsProviderMetadata.VectorTile.is_monkey_patched = True
QgsProviderMetadata.VectorTile.__doc__ = "Vector tile layers \n.. versionadded:: 3.32"
QgsProviderMetadata.TiledScene = Qgis.FileFilterType.TiledScene
QgsProviderMetadata.TiledScene.is_monkey_patched = True
QgsProviderMetadata.TiledScene.__doc__ = "Tiled scene layers \n.. versionadded:: 3.34"
Qgis.FileFilterType.__doc__ = """Type of file filters

Prior to QGIS 3.32 this was available as :py:class:`QgsProviderMetadata`.FilterType

.. versionadded:: 3.32

* ``Vector``: Vector layers

  Available as ``QgsProviderMetadata.FilterVector`` in older QGIS releases.

* ``Raster``: Raster layers

  Available as ``QgsProviderMetadata.FilterRaster`` in older QGIS releases.

* ``Mesh``: Mesh layers

  Available as ``QgsProviderMetadata.FilterMesh`` in older QGIS releases.

* ``MeshDataset``: Mesh datasets

  Available as ``QgsProviderMetadata.FilterMeshDataset`` in older QGIS releases.

* ``PointCloud``: Point clouds

  .. versionadded:: 3.18


  Available as ``QgsProviderMetadata.FilterPointCloud`` in older QGIS releases.

* ``VectorTile``: Vector tile layers

  .. versionadded:: 3.32

* ``TiledScene``: Tiled scene layers

  .. versionadded:: 3.34


"""
# --
Qgis.FileFilterType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SublayerQueryFlag.FastScan.__doc__ = "Indicates that the provider must scan for sublayers using the fastest possible approach -- e.g. by first checking that a uri has an extension which is known to be readable by the provider"
Qgis.SublayerQueryFlag.ResolveGeometryType.__doc__ = "Attempt to resolve the geometry type for vector sublayers"
Qgis.SublayerQueryFlag.CountFeatures.__doc__ = "Count features in vector sublayers"
Qgis.SublayerQueryFlag.IncludeSystemTables.__doc__ = "Include system or internal tables (these are not included by default)"
Qgis.SublayerQueryFlag.__doc__ = """Flags which control how data providers will scan for sublayers in a dataset.

.. versionadded:: 3.22

* ``FastScan``: Indicates that the provider must scan for sublayers using the fastest possible approach -- e.g. by first checking that a uri has an extension which is known to be readable by the provider
* ``ResolveGeometryType``: Attempt to resolve the geometry type for vector sublayers
* ``CountFeatures``: Count features in vector sublayers
* ``IncludeSystemTables``: Include system or internal tables (these are not included by default)

"""
# --
Qgis.SublayerQueryFlag.baseClass = Qgis
Qgis.SublayerQueryFlags.baseClass = Qgis
SublayerQueryFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.SublayerFlag.SystemTable.__doc__ = "Sublayer is a system or internal table, which should be hidden by default"
Qgis.SublayerFlag.__doc__ = """Flags which reflect the properties of sublayers in a dataset.

.. versionadded:: 3.22

* ``SystemTable``: Sublayer is a system or internal table, which should be hidden by default

"""
# --
Qgis.SublayerFlag.baseClass = Qgis
Qgis.SublayerFlags.baseClass = Qgis
SublayerFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsColorRampShader.Type = Qgis.ShaderInterpolationMethod
# monkey patching scoped based enum
QgsColorRampShader.Interpolated = Qgis.ShaderInterpolationMethod.Linear
QgsColorRampShader.Type.Interpolated = Qgis.ShaderInterpolationMethod.Linear
QgsColorRampShader.Interpolated.is_monkey_patched = True
QgsColorRampShader.Interpolated.__doc__ = "Interpolates the color between two class breaks linearly"
QgsColorRampShader.Discrete = Qgis.ShaderInterpolationMethod.Discrete
QgsColorRampShader.Discrete.is_monkey_patched = True
QgsColorRampShader.Discrete.__doc__ = "Assigns the color of the higher class for every pixel between two class breaks"
QgsColorRampShader.Exact = Qgis.ShaderInterpolationMethod.Exact
QgsColorRampShader.Exact.is_monkey_patched = True
QgsColorRampShader.Exact.__doc__ = "Assigns the color of the exact matching value in the color ramp item list"
Qgis.ShaderInterpolationMethod.__doc__ = """Color ramp shader interpolation methods.

.. note::

   Prior to QGIS 3.38 this was available as :py:class:`QgsColorRampShader`.Type

.. versionadded:: 3.38

* ``Linear``: Interpolates the color between two class breaks linearly

  Available as ``QgsColorRampShader.Interpolated`` in older QGIS releases.

* ``Discrete``: Assigns the color of the higher class for every pixel between two class breaks
* ``Exact``: Assigns the color of the exact matching value in the color ramp item list

"""
# --
Qgis.ShaderInterpolationMethod.baseClass = Qgis
QgsColorRampShader.ClassificationMode = Qgis.ShaderClassificationMethod
# monkey patching scoped based enum
QgsColorRampShader.Continuous = Qgis.ShaderClassificationMethod.Continuous
QgsColorRampShader.Continuous.is_monkey_patched = True
QgsColorRampShader.Continuous.__doc__ = "Uses breaks from color palette"
QgsColorRampShader.EqualInterval = Qgis.ShaderClassificationMethod.EqualInterval
QgsColorRampShader.EqualInterval.is_monkey_patched = True
QgsColorRampShader.EqualInterval.__doc__ = "Uses equal interval"
QgsColorRampShader.Quantile = Qgis.ShaderClassificationMethod.Quantile
QgsColorRampShader.Quantile.is_monkey_patched = True
QgsColorRampShader.Quantile.__doc__ = "Uses quantile (i.e. equal pixel) count"
Qgis.ShaderClassificationMethod.__doc__ = """Color ramp shader classification methods.

.. note::

   Prior to QGIS 3.38 this was available as :py:class:`QgsColorRampShader`.ClassificationMode

.. versionadded:: 3.38

* ``Continuous``: Uses breaks from color palette
* ``EqualInterval``: Uses equal interval
* ``Quantile``: Uses quantile (i.e. equal pixel) count

"""
# --
Qgis.ShaderClassificationMethod.baseClass = Qgis
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
Qgis.RasterPipeInterfaceRole.__doc__ = """Raster pipe interface roles.

.. versionadded:: 3.22

* ``Unknown``: Unknown role

  Available as ``QgsRasterPipe.UnknownRole`` in older QGIS releases.

* ``Provider``: Data provider role

  Available as ``QgsRasterPipe.ProviderRole`` in older QGIS releases.

* ``Renderer``: Raster renderer role

  Available as ``QgsRasterPipe.RendererRole`` in older QGIS releases.

* ``Brightness``: Brightness filter role

  Available as ``QgsRasterPipe.BrightnessRole`` in older QGIS releases.

* ``Resampler``: Resampler role

  Available as ``QgsRasterPipe.ResamplerRole`` in older QGIS releases.

* ``Projector``: Projector role

  Available as ``QgsRasterPipe.ProjectorRole`` in older QGIS releases.

* ``Nuller``: Raster nuller role

  Available as ``QgsRasterPipe.NullerRole`` in older QGIS releases.

* ``HueSaturation``: Hue/saturation filter role (also applies grayscale/color inversion)

  Available as ``QgsRasterPipe.HueSaturationRole`` in older QGIS releases.


"""
# --
Qgis.RasterPipeInterfaceRole.baseClass = Qgis
QgsRasterPipe.ResamplingStage = Qgis.RasterResamplingStage
# monkey patching scoped based enum
QgsRasterPipe.ResampleFilter = Qgis.RasterResamplingStage.ResampleFilter
QgsRasterPipe.ResampleFilter.is_monkey_patched = True
QgsRasterPipe.ResampleFilter.__doc__ = "Resampling occurs in ResamplingFilter"
QgsRasterPipe.Provider = Qgis.RasterResamplingStage.Provider
QgsRasterPipe.Provider.is_monkey_patched = True
QgsRasterPipe.Provider.__doc__ = "Resampling occurs in Provider"
Qgis.RasterResamplingStage.__doc__ = """Stage at which raster resampling occurs.

.. versionadded:: 3.22

* ``ResampleFilter``: Resampling occurs in ResamplingFilter
* ``Provider``: Resampling occurs in Provider

"""
# --
Qgis.RasterResamplingStage.baseClass = Qgis
QgsRasterDataProvider.ResamplingMethod = Qgis.RasterResamplingMethod
# monkey patching scoped based enum
QgsRasterDataProvider.Nearest = Qgis.RasterResamplingMethod.Nearest
QgsRasterDataProvider.Nearest.is_monkey_patched = True
QgsRasterDataProvider.Nearest.__doc__ = "Nearest-neighbour resampling"
QgsRasterDataProvider.Bilinear = Qgis.RasterResamplingMethod.Bilinear
QgsRasterDataProvider.Bilinear.is_monkey_patched = True
QgsRasterDataProvider.Bilinear.__doc__ = "Bilinear (2x2 kernel) resampling"
QgsRasterDataProvider.Cubic = Qgis.RasterResamplingMethod.Cubic
QgsRasterDataProvider.Cubic.is_monkey_patched = True
QgsRasterDataProvider.Cubic.__doc__ = "Cubic Convolution Approximation (4x4 kernel) resampling"
QgsRasterDataProvider.CubicSpline = Qgis.RasterResamplingMethod.CubicSpline
QgsRasterDataProvider.CubicSpline.is_monkey_patched = True
QgsRasterDataProvider.CubicSpline.__doc__ = "Cubic B-Spline Approximation (4x4 kernel)"
QgsRasterDataProvider.Lanczos = Qgis.RasterResamplingMethod.Lanczos
QgsRasterDataProvider.Lanczos.is_monkey_patched = True
QgsRasterDataProvider.Lanczos.__doc__ = "Lanczos windowed sinc interpolation (6x6 kernel)"
QgsRasterDataProvider.Average = Qgis.RasterResamplingMethod.Average
QgsRasterDataProvider.Average.is_monkey_patched = True
QgsRasterDataProvider.Average.__doc__ = "Average resampling"
QgsRasterDataProvider.Mode = Qgis.RasterResamplingMethod.Mode
QgsRasterDataProvider.Mode.is_monkey_patched = True
QgsRasterDataProvider.Mode.__doc__ = "Mode (selects the value which appears most often of all the sampled points)"
QgsRasterDataProvider.Gauss = Qgis.RasterResamplingMethod.Gauss
QgsRasterDataProvider.Gauss.is_monkey_patched = True
QgsRasterDataProvider.Gauss.__doc__ = "Gauss blurring"
Qgis.RasterResamplingMethod.__doc__ = """Resampling method for raster provider-level resampling.

.. note::

   Prior to QGIS 3.42 this was available as :py:class:`QgsRasterDataProvider`.ResamplingMethod

.. versionadded:: 3.42

* ``Nearest``: Nearest-neighbour resampling
* ``Bilinear``: Bilinear (2x2 kernel) resampling
* ``Cubic``: Cubic Convolution Approximation (4x4 kernel) resampling
* ``CubicSpline``: Cubic B-Spline Approximation (4x4 kernel)
* ``Lanczos``: Lanczos windowed sinc interpolation (6x6 kernel)
* ``Average``: Average resampling
* ``Mode``: Mode (selects the value which appears most often of all the sampled points)
* ``Gauss``: Gauss blurring

"""
# --
Qgis.RasterResamplingMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RasterRendererFlag.InternalLayerOpacityHandling.__doc__ = "The renderer internally handles the raster layer's opacity, so the default layer level opacity handling should not be applied."
Qgis.RasterRendererFlag.UseNoDataForOutOfRangePixels.__doc__ = "Out of range pixels (eg those values outside of the rendered map's z range filter) should be set using additional nodata values instead of additional transparency values \n.. versionadded:: 3.38"
Qgis.RasterRendererFlag.__doc__ = """Flags which control behavior of raster renderers.

.. versionadded:: 3.28

* ``InternalLayerOpacityHandling``: The renderer internally handles the raster layer's opacity, so the default layer level opacity handling should not be applied.
* ``UseNoDataForOutOfRangePixels``: Out of range pixels (eg those values outside of the rendered map's z range filter) should be set using additional nodata values instead of additional transparency values

  .. versionadded:: 3.38


"""
# --
Qgis.RasterRendererFlag.baseClass = Qgis
Qgis.RasterRendererFlags.baseClass = Qgis
RasterRendererFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.RasterRendererCapability.UsesMultipleBands.__doc__ = "The renderer utilizes multiple raster bands for color data (note that alpha bands are not considered for this capability)"
Qgis.RasterRendererCapability.__doc__ = """Raster renderer capabilities.

.. versionadded:: 3.48

* ``UsesMultipleBands``: The renderer utilizes multiple raster bands for color data (note that alpha bands are not considered for this capability)

"""
# --
Qgis.RasterRendererCapability.baseClass = Qgis
Qgis.RasterRendererCapabilities.baseClass = Qgis
RasterRendererCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsRasterMinMaxOrigin.Limits = Qgis.RasterRangeLimit
# monkey patching scoped based enum
QgsRasterMinMaxOrigin.None_ = Qgis.RasterRangeLimit.NotSet
QgsRasterMinMaxOrigin.Limits.None_ = Qgis.RasterRangeLimit.NotSet
QgsRasterMinMaxOrigin.None_.is_monkey_patched = True
QgsRasterMinMaxOrigin.None_.__doc__ = "User defined"
QgsRasterMinMaxOrigin.MinMax = Qgis.RasterRangeLimit.MinimumMaximum
QgsRasterMinMaxOrigin.Limits.MinMax = Qgis.RasterRangeLimit.MinimumMaximum
QgsRasterMinMaxOrigin.MinMax.is_monkey_patched = True
QgsRasterMinMaxOrigin.MinMax.__doc__ = "Real min-max values"
QgsRasterMinMaxOrigin.StdDev = Qgis.RasterRangeLimit.StdDev
QgsRasterMinMaxOrigin.StdDev.is_monkey_patched = True
QgsRasterMinMaxOrigin.StdDev.__doc__ = "Range is [ mean - stdDevFactor() * stddev, mean + stdDevFactor() * stddev ]"
QgsRasterMinMaxOrigin.CumulativeCut = Qgis.RasterRangeLimit.CumulativeCut
QgsRasterMinMaxOrigin.CumulativeCut.is_monkey_patched = True
QgsRasterMinMaxOrigin.CumulativeCut.__doc__ = "Range is [ min + cumulativeCutLower() * (max - min), min + cumulativeCutUpper() * (max - min) ]"
Qgis.RasterRangeLimit.__doc__ = """Describes the limits used to compute raster ranges (min/max values).

.. note::

   Prior to QGIS 3.42 this was available as :py:class:`QgsRasterMinMaxOrigin`.Limits

.. versionadded:: 3.42

* ``NotSet``: User defined

  Available as ``QgsRasterMinMaxOrigin.None_`` in older QGIS releases.

* ``MinimumMaximum``: Real min-max values

  Available as ``QgsRasterMinMaxOrigin.MinMax`` in older QGIS releases.

* ``StdDev``: Range is [ mean - stdDevFactor() * stddev, mean + stdDevFactor() * stddev ]
* ``CumulativeCut``: Range is [ min + cumulativeCutLower() * (max - min), min + cumulativeCutUpper() * (max - min) ]

"""
# --
Qgis.RasterRangeLimit.baseClass = Qgis
QgsRasterMinMaxOrigin.Extent = Qgis.RasterRangeExtent
# monkey patching scoped based enum
QgsRasterMinMaxOrigin.None_ = Qgis.RasterRangeExtent.WholeRaster
QgsRasterMinMaxOrigin.Extent.None_ = Qgis.RasterRangeExtent.WholeRaster
QgsRasterMinMaxOrigin.None_.is_monkey_patched = True
QgsRasterMinMaxOrigin.None_.__doc__ = "Whole raster is used to compute statistics"
QgsRasterMinMaxOrigin.CurrentCanvas = Qgis.RasterRangeExtent.FixedCanvas
QgsRasterMinMaxOrigin.Extent.CurrentCanvas = Qgis.RasterRangeExtent.FixedCanvas
QgsRasterMinMaxOrigin.CurrentCanvas.is_monkey_patched = True
QgsRasterMinMaxOrigin.CurrentCanvas.__doc__ = "Current extent of the canvas (at the time of computation) is used to compute statistics"
QgsRasterMinMaxOrigin.UpdatedCanvas = Qgis.RasterRangeExtent.UpdatedCanvas
QgsRasterMinMaxOrigin.UpdatedCanvas.is_monkey_patched = True
QgsRasterMinMaxOrigin.UpdatedCanvas.__doc__ = "Constantly updated extent of the canvas is used to compute statistics"
Qgis.RasterRangeExtent.__doc__ = """Describes the extent used to compute raster ranges (min/max values).

.. note::

   Prior to QGIS 3.42 this was available as :py:class:`QgsRasterMinMaxOrigin`.Extent

.. versionadded:: 3.42

* ``WholeRaster``: Whole raster is used to compute statistics

  Available as ``QgsRasterMinMaxOrigin.None_`` in older QGIS releases.

* ``FixedCanvas``: Current extent of the canvas (at the time of computation) is used to compute statistics

  Available as ``QgsRasterMinMaxOrigin.CurrentCanvas`` in older QGIS releases.

* ``UpdatedCanvas``: Constantly updated extent of the canvas is used to compute statistics

"""
# --
Qgis.RasterRangeExtent.baseClass = Qgis
QgsRasterMinMaxOrigin.StatAccuracy = Qgis.RasterRangeAccuracy
# monkey patching scoped based enum
QgsRasterMinMaxOrigin.Exact = Qgis.RasterRangeAccuracy.Exact
QgsRasterMinMaxOrigin.Exact.is_monkey_patched = True
QgsRasterMinMaxOrigin.Exact.__doc__ = "Exact statistics"
QgsRasterMinMaxOrigin.Estimated = Qgis.RasterRangeAccuracy.Estimated
QgsRasterMinMaxOrigin.Estimated.is_monkey_patched = True
QgsRasterMinMaxOrigin.Estimated.__doc__ = "Approximated statistics"
Qgis.RasterRangeAccuracy.__doc__ = """Describes the accuracy used to compute raster ranges (min/max values).

.. note::

   Prior to QGIS 3.42 this was available as :py:class:`QgsRasterMinMaxOrigin`.StatAccuracy

.. versionadded:: 3.42

* ``Exact``: Exact statistics
* ``Estimated``: Approximated statistics

"""
# --
Qgis.RasterRangeAccuracy.baseClass = Qgis
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
Qgis.RasterAttributeTableFieldUsage.__doc__ = """The RasterAttributeTableFieldUsage enum represents the usage of a Raster Attribute Table field.

.. note::

   Directly mapped from GDALRATFieldUsage enum values.

.. versionadded:: 3.30

* ``Generic``: Field usage Generic
* ``PixelCount``: Field usage PixelCount
* ``Name``: Field usage Name
* ``Min``: Field usage Min
* ``Max``: Field usage Max
* ``MinMax``: Field usage MinMax
* ``Red``: Field usage Red
* ``Green``: Field usage Green
* ``Blue``: Field usage Blue
* ``Alpha``: Field usage Alpha
* ``RedMin``: Field usage RedMin
* ``GreenMin``: Field usage GreenMin
* ``BlueMin``: Field usage BlueMin
* ``AlphaMin``: Field usage AlphaMin
* ``RedMax``: Field usage RedMax
* ``GreenMax``: Field usage GreenMax
* ``BlueMax``: Field usage BlueMax
* ``AlphaMax``: Field usage AlphaMax
* ``MaxCount``: Not used by QGIS: GDAL Maximum GFU value (equals to GFU_AlphaMax+1 currently)

"""
# --
Qgis.RasterAttributeTableFieldUsage.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RasterAttributeTableType.Thematic.__doc__ = ""
Qgis.RasterAttributeTableType.Athematic.__doc__ = ""
Qgis.RasterAttributeTableType.__doc__ = """The RasterAttributeTableType enum represents the type of RAT.
note Directly mapped from GDALRATTableType enum values.

.. versionadded:: 3.30

* ``Thematic``: 
* ``Athematic``: 

"""
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
Qgis.RasterExportType.__doc__ = """Raster file export types.

Prior to QGIS 3.32 this was available as :py:class:`QgsRasterFileWriter`.Mode

.. versionadded:: 3.32

* ``Raw``: Raw data
* ``RenderedImage``: Rendered image

  Available as ``QgsRasterFileWriter.Image`` in older QGIS releases.


"""
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
Qgis.RasterFileWriterResult.__doc__ = """Raster file export results.

Prior to QGIS 3.32 this was available as :py:class:`QgsRasterFileWriter`.WriterError

.. versionadded:: 3.32

* ``Success``: Successful export

  Available as ``QgsRasterFileWriter.NoError`` in older QGIS releases.

* ``SourceProviderError``: Source data provider error
* ``DestinationProviderError``: Destination data provider error

  Available as ``QgsRasterFileWriter.DestProviderError`` in older QGIS releases.

* ``CreateDatasourceError``: Data source creation error
* ``WriteError``: Write error
* ``NoDataConflict``: Internal error if a value used for 'no data' was found in input
* ``Canceled``: Writing was manually canceled

  Available as ``QgsRasterFileWriter.WriteCanceled`` in older QGIS releases.


"""
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
Qgis.MeshEditingErrorType.__doc__ = """Type of error that can occur during mesh frame editing.

.. versionadded:: 3.22

* ``NoError``: No type
* ``InvalidFace``: An error occurs due to an invalid face (for example, vertex indexes are unordered)
* ``TooManyVerticesInFace``: A face has more vertices than the maximum number supported per face
* ``FlatFace``: A flat face is present
* ``UniqueSharedVertex``: A least two faces share only one vertices
* ``InvalidVertex``: An error occurs due to an invalid vertex (for example, vertex index is out of range the available vertex)
* ``ManifoldFace``: ManifoldFace

"""
# --
Qgis.MeshEditingErrorType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FilePathType.Absolute.__doc__ = "Absolute path"
Qgis.FilePathType.Relative.__doc__ = "Relative path"
Qgis.FilePathType.__doc__ = """File path types.

.. versionadded:: 3.22

* ``Absolute``: Absolute path
* ``Relative``: Relative path

"""
# --
Qgis.FilePathType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SublayerPromptMode.AlwaysAsk.__doc__ = "Always ask users to select from available sublayers, if sublayers are present"
Qgis.SublayerPromptMode.AskExcludingRasterBands.__doc__ = "Ask users to select from available sublayers, unless only raster bands are present"
Qgis.SublayerPromptMode.NeverAskSkip.__doc__ = "Never ask users to select sublayers, instead don't load anything"
Qgis.SublayerPromptMode.NeverAskLoadAll.__doc__ = "Never ask users to select sublayers, instead automatically load all available sublayers"
Qgis.SublayerPromptMode.__doc__ = """Specifies how to handle layer sources with multiple sublayers.

.. versionadded:: 3.22

* ``AlwaysAsk``: Always ask users to select from available sublayers, if sublayers are present
* ``AskExcludingRasterBands``: Ask users to select from available sublayers, unless only raster bands are present
* ``NeverAskSkip``: Never ask users to select sublayers, instead don't load anything
* ``NeverAskLoadAll``: Never ask users to select sublayers, instead automatically load all available sublayers

"""
# --
Qgis.SublayerPromptMode.baseClass = Qgis
QgsFields.FieldOrigin = Qgis.FieldOrigin
# monkey patching scoped based enum
QgsFields.OriginUnknown = Qgis.FieldOrigin.Unknown
QgsFields.FieldOrigin.OriginUnknown = Qgis.FieldOrigin.Unknown
QgsFields.OriginUnknown.is_monkey_patched = True
QgsFields.OriginUnknown.__doc__ = "The field origin has not been specified"
QgsFields.OriginProvider = Qgis.FieldOrigin.Provider
QgsFields.FieldOrigin.OriginProvider = Qgis.FieldOrigin.Provider
QgsFields.OriginProvider.is_monkey_patched = True
QgsFields.OriginProvider.__doc__ = "Field originates from the underlying data provider of the vector layer"
QgsFields.OriginJoin = Qgis.FieldOrigin.Join
QgsFields.FieldOrigin.OriginJoin = Qgis.FieldOrigin.Join
QgsFields.OriginJoin.is_monkey_patched = True
QgsFields.OriginJoin.__doc__ = "Field originates from a joined layer"
QgsFields.OriginEdit = Qgis.FieldOrigin.Edit
QgsFields.FieldOrigin.OriginEdit = Qgis.FieldOrigin.Edit
QgsFields.OriginEdit.is_monkey_patched = True
QgsFields.OriginEdit.__doc__ = "Field has been temporarily added in editing mode"
QgsFields.OriginExpression = Qgis.FieldOrigin.Expression
QgsFields.FieldOrigin.OriginExpression = Qgis.FieldOrigin.Expression
QgsFields.OriginExpression.is_monkey_patched = True
QgsFields.OriginExpression.__doc__ = "Field is calculated from an expression"
Qgis.FieldOrigin.__doc__ = """Field origin.

.. note::

   Prior to QGIS 3.38 this was available as :py:class:`QgsFields`.FieldOrigin

.. versionadded:: 3.38

* ``Unknown``: The field origin has not been specified

  Available as ``QgsFields.OriginUnknown`` in older QGIS releases.

* ``Provider``: Field originates from the underlying data provider of the vector layer

  Available as ``QgsFields.OriginProvider`` in older QGIS releases.

* ``Join``: Field originates from a joined layer

  Available as ``QgsFields.OriginJoin`` in older QGIS releases.

* ``Edit``: Field has been temporarily added in editing mode

  Available as ``QgsFields.OriginEdit`` in older QGIS releases.

* ``Expression``: Field is calculated from an expression

  Available as ``QgsFields.OriginExpression`` in older QGIS releases.


"""
# --
Qgis.FieldOrigin.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldConfigurationFlag.NoFlag.__doc__ = "No flag is defined"
Qgis.FieldConfigurationFlag.NotSearchable.__doc__ = "Defines if the field is searchable (used in the locator search for instance)"
Qgis.FieldConfigurationFlag.HideFromWms.__doc__ = "Field is not available if layer is served as WMS from QGIS server"
Qgis.FieldConfigurationFlag.HideFromWfs.__doc__ = "Field is not available if layer is served as WFS from QGIS server"
Qgis.FieldConfigurationFlag.__doc__ = """Configuration flags for fields
These flags are meant to be user-configurable
and are not describing any information from the data provider.

.. note::

   FieldConfigurationFlag are expressed in the negative forms so that default flags is NoFlag.

.. versionadded:: 3.34

* ``NoFlag``: No flag is defined
* ``NotSearchable``: Defines if the field is searchable (used in the locator search for instance)
* ``HideFromWms``: Field is not available if layer is served as WMS from QGIS server
* ``HideFromWfs``: Field is not available if layer is served as WFS from QGIS server

"""
# --
Qgis.FieldConfigurationFlag.baseClass = Qgis
Qgis.FieldConfigurationFlags.baseClass = Qgis
FieldConfigurationFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.FieldMetadataProperty.GeometryCrs.__doc__ = "Available for geometry field types with a specific associated coordinate reference system (as a QgsCoordinateReferenceSystem value)"
Qgis.FieldMetadataProperty.GeometryWkbType.__doc__ = "Available for geometry field types which accept geometries of a specific WKB type only (as a QgsWkbTypes.Type value)"
Qgis.FieldMetadataProperty.CustomProperty.__doc__ = "Starting point for custom user set properties"
Qgis.FieldMetadataProperty.__doc__ = """Standard field metadata values.

.. versionadded:: 3.30

* ``GeometryCrs``: Available for geometry field types with a specific associated coordinate reference system (as a QgsCoordinateReferenceSystem value)
* ``GeometryWkbType``: Available for geometry field types which accept geometries of a specific WKB type only (as a QgsWkbTypes.Type value)
* ``CustomProperty``: Starting point for custom user set properties

"""
# --
Qgis.FieldMetadataProperty.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SelectionRenderingMode.Default.__doc__ = "Use default symbol and selection colors"
Qgis.SelectionRenderingMode.CustomColor.__doc__ = "Use default symbol with a custom selection color"
Qgis.SelectionRenderingMode.CustomSymbol.__doc__ = "Use a custom symbol"
Qgis.SelectionRenderingMode.__doc__ = """Specifies how a selection should be rendered.

.. versionadded:: 3.34

* ``Default``: Use default symbol and selection colors
* ``CustomColor``: Use default symbol with a custom selection color
* ``CustomSymbol``: Use a custom symbol

"""
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
Qgis.SelectBehavior.__doc__ = """Specifies how a selection should be applied.

.. versionadded:: 3.22

* ``SetSelection``: Set selection, removing any existing selection
* ``AddToSelection``: Add selection to current selection
* ``IntersectSelection``: Modify current selection to include only select features which match
* ``RemoveFromSelection``: Remove from current selection

"""
# --
Qgis.SelectBehavior.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SelectGeometryRelationship.Intersect.__doc__ = "Select where features intersect the reference geometry"
Qgis.SelectGeometryRelationship.Within.__doc__ = "Select where features are within the reference geometry"
Qgis.SelectGeometryRelationship.__doc__ = """Geometry relationship test to apply for selecting features.

.. versionadded:: 3.28

* ``Intersect``: Select where features intersect the reference geometry
* ``Within``: Select where features are within the reference geometry

"""
# --
Qgis.SelectGeometryRelationship.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SelectionFlag.SingleFeatureSelection.__doc__ = "Select only a single feature, picking the \"best\" match for the selection geometry"
Qgis.SelectionFlag.ToggleSelection.__doc__ = "Enables a \"toggle\" selection mode, where previously selected matching features will be deselected and previously deselected features will be selected. This flag works only when the SingleFeatureSelection flag is also set."
Qgis.SelectionFlag.__doc__ = """Flags which control feature selection behavior.

.. versionadded:: 3.28

* ``SingleFeatureSelection``: Select only a single feature, picking the \"best\" match for the selection geometry
* ``ToggleSelection``: Enables a \"toggle\" selection mode, where previously selected matching features will be deselected and previously deselected features will be selected. This flag works only when the SingleFeatureSelection flag is also set.

"""
# --
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
Qgis.VectorEditResult.__doc__ = """Specifies the result of a vector layer edit operation

.. versionadded:: 3.22

* ``Success``: Edit operation was successful
* ``EmptyGeometry``: Edit operation resulted in an empty geometry
* ``EditFailed``: Edit operation failed
* ``FetchFeatureFailed``: Unable to fetch requested feature
* ``InvalidLayer``: Edit failed due to invalid layer

"""
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
Qgis.VertexMarkerType.__doc__ = """Editing vertex markers, used for showing vertices during a edit operation.

.. versionadded:: 3.22

* ``SemiTransparentCircle``: Semi-transparent circle marker
* ``Cross``: Cross marker
* ``NoMarker``: No marker

"""
# --
Qgis.VertexMarkerType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ContentStatus.NotStarted.__doc__ = "Content fetching/storing has not started yet"
Qgis.ContentStatus.Running.__doc__ = "Content fetching/storing is in progress"
Qgis.ContentStatus.Finished.__doc__ = "Content fetching/storing is finished and successful"
Qgis.ContentStatus.Failed.__doc__ = "Content fetching/storing has failed"
Qgis.ContentStatus.Canceled.__doc__ = "Content fetching/storing has been canceled"
Qgis.ContentStatus.__doc__ = """Status for fetched or stored content

.. versionadded:: 3.22

* ``NotStarted``: Content fetching/storing has not started yet
* ``Running``: Content fetching/storing is in progress
* ``Finished``: Content fetching/storing is finished and successful
* ``Failed``: Content fetching/storing has failed
* ``Canceled``: Content fetching/storing has been canceled

"""
# --
Qgis.ContentStatus.baseClass = Qgis
# monkey patching scoped based enum
Qgis.GpsConnectionType.Automatic.__doc__ = "Automatically detected GPS device connection"
Qgis.GpsConnectionType.Internal.__doc__ = "Internal GPS device"
Qgis.GpsConnectionType.Serial.__doc__ = "Serial port GPS device"
Qgis.GpsConnectionType.Gpsd.__doc__ = "GPSD device"
Qgis.GpsConnectionType.__doc__ = """GPS connection types.

.. versionadded:: 3.30

* ``Automatic``: Automatically detected GPS device connection
* ``Internal``: Internal GPS device
* ``Serial``: Serial port GPS device
* ``Gpsd``: GPSD device

"""
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
Qgis.DeviceConnectionStatus.__doc__ = """GPS connection status.

.. versionadded:: 3.30

* ``Disconnected``: Device is disconnected
* ``Connecting``: Device is connecting
* ``Connected``: Device is successfully connected

"""
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
Qgis.GpsFixStatus.__doc__ = """GPS fix status.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsGpsInformation`.FixStatus

.. versionadded:: 3.30

* ``NoData``: No fix data available
* ``NoFix``: GPS is not fixed
* ``Fix2D``: 2D fix
* ``Fix3D``: 3D fix

"""
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
Qgis.GnssConstellation.__doc__ = """GNSS constellation

.. versionadded:: 3.30

* ``Unknown``: Unknown/other system
* ``Gps``: Global Positioning System (GPS)
* ``Glonass``: Global Navigation Satellite System (GLONASS)
* ``Galileo``: Galileo
* ``BeiDou``: BeiDou
* ``Qzss``: Quasi Zenith Satellite System (QZSS)
* ``Navic``: Indian Regional Navigation Satellite System (IRNSS) / NAVIC
* ``Sbas``: SBAS

"""
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
Qgis.GpsQualityIndicator.__doc__ = """GPS signal quality indicator

.. versionadded:: 3.22.6

* ``Unknown``: Unknown
* ``Invalid``: Invalid
* ``GPS``: Standalone
* ``DGPS``: Differential GPS
* ``PPS``: PPS
* ``RTK``: Real-time-kynematic
* ``FloatRTK``: Float real-time-kynematic
* ``Estimated``: Estimated
* ``Manual``: Manual input mode
* ``Simulation``: Simulation mode

"""
# --
Qgis.GpsQualityIndicator.baseClass = Qgis
# monkey patching scoped based enum
Qgis.GpsNavigationStatus.NotValid.__doc__ = "Navigation status not valid"
Qgis.GpsNavigationStatus.Safe.__doc__ = "Safe"
Qgis.GpsNavigationStatus.Caution.__doc__ = "Caution"
Qgis.GpsNavigationStatus.Unsafe.__doc__ = "Unsafe"
Qgis.GpsNavigationStatus.__doc__ = """GPS navigation status.

.. versionadded:: 3.38

* ``NotValid``: Navigation status not valid
* ``Safe``: Safe
* ``Caution``: Caution
* ``Unsafe``: Unsafe

"""
# --
Qgis.GpsNavigationStatus.baseClass = Qgis
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
Qgis.GpsInformationComponent.__doc__ = """GPS information component.

.. versionadded:: 3.30

* ``Location``: 2D location (latitude/longitude), as a QgsPointXY value
* ``Altitude``: Altitude/elevation above or below the mean sea level
* ``GroundSpeed``: Ground speed
* ``Bearing``: Bearing measured in degrees clockwise from true north to the direction of travel
* ``TotalTrackLength``: Total distance of current GPS track (available from QgsGpsLogger class only)
* ``TrackDistanceFromStart``: Direct distance from first vertex in current GPS track to last vertex (available from QgsGpsLogger class only)
* ``Pdop``: Dilution of precision
* ``Hdop``: Horizontal dilution of precision
* ``Vdop``: Vertical dilution of precision
* ``HorizontalAccuracy``: Horizontal accuracy in meters
* ``VerticalAccuracy``: Vertical accuracy in meters
* ``HvAccuracy``: 3D RMS
* ``SatellitesUsed``: Count of satellites used in obtaining the fix
* ``Timestamp``: Timestamp
* ``TrackStartTime``: Timestamp at start of current track (available from QgsGpsLogger class only)
* ``TrackEndTime``: Timestamp at end (current point) of current track (available from QgsGpsLogger class only)
* ``TrackDistanceSinceLastPoint``: Distance since last recorded location (available from QgsGpsLogger class only)
* ``TrackTimeSinceLastPoint``: Time since last recorded location (available from QgsGpsLogger class only)
* ``GeoidalSeparation``: Geoidal separation, the difference between the WGS-84 Earth ellipsoid and mean-sea-level (geoid), \"-\" means mean-sea-level below ellipsoid
* ``EllipsoidAltitude``: Altitude/elevation above or below the WGS-84 Earth ellipsoid

"""
# --
Qgis.GpsInformationComponent.baseClass = Qgis
Qgis.GpsInformationComponents.baseClass = Qgis
GpsInformationComponents = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.BabelFormatCapability.Import.__doc__ = "Format supports importing"
Qgis.BabelFormatCapability.Export.__doc__ = "Format supports exporting"
Qgis.BabelFormatCapability.Waypoints.__doc__ = "Format supports waypoints"
Qgis.BabelFormatCapability.Routes.__doc__ = "Format supports routes"
Qgis.BabelFormatCapability.Tracks.__doc__ = "Format supports tracks"
Qgis.BabelFormatCapability.__doc__ = """Babel GPS format capabilities.

.. versionadded:: 3.22

* ``Import``: Format supports importing
* ``Export``: Format supports exporting
* ``Waypoints``: Format supports waypoints
* ``Routes``: Format supports routes
* ``Tracks``: Format supports tracks

"""
# --
Qgis.BabelFormatCapability.baseClass = Qgis
Qgis.BabelFormatCapabilities.baseClass = Qgis
BabelFormatCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.BabelCommandFlag.QuoteFilePaths.__doc__ = "File paths should be enclosed in quotations and escaped"
Qgis.BabelCommandFlag.__doc__ = """Babel command flags, which control how commands and arguments
are generated for executing GPSBabel processes.

.. versionadded:: 3.22

* ``QuoteFilePaths``: File paths should be enclosed in quotations and escaped

"""
# --
Qgis.BabelCommandFlag.baseClass = Qgis
Qgis.BabelCommandFlags.baseClass = Qgis
BabelCommandFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.GpsFeatureType.Waypoint.__doc__ = "Waypoint"
Qgis.GpsFeatureType.Route.__doc__ = "Route"
Qgis.GpsFeatureType.Track.__doc__ = "Track"
Qgis.GpsFeatureType.__doc__ = """GPS feature types.

.. versionadded:: 3.22

* ``Waypoint``: Waypoint
* ``Route``: Route
* ``Track``: Track

"""
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
Qgis.GeometryOperationResult.__doc__ = """Success or failure of a geometry operation.

This enum gives details about cause of failure.

.. versionadded:: 3.22

* ``Success``: Operation succeeded
* ``NothingHappened``: Nothing happened, without any error
* ``InvalidBaseGeometry``: The base geometry on which the operation is done is invalid or empty
* ``InvalidInputGeometryType``: The input geometry (ring, part, split line, etc.) has not the correct geometry type
* ``SelectionIsEmpty``: No features were selected
* ``SelectionIsGreaterThanOne``: More than one features were selected
* ``GeometryEngineError``: Geometry engine misses a method implemented or an error occurred in the geometry engine
* ``LayerNotEditable``: Cannot edit layer
* ``AddPartSelectedGeometryNotFound``: The selected geometry cannot be found
* ``AddPartNotMultiGeometry``: The source geometry is not multi
* ``AddRingNotClosed``: The input ring is not closed
* ``AddRingNotValid``: The input ring is not valid
* ``AddRingCrossesExistingRings``: The input ring crosses existing rings (it is not disjoint)
* ``AddRingNotInExistingFeature``: The input ring doesn't have any existing ring to fit into
* ``SplitCannotSplitPoint``: Cannot split points
* ``GeometryTypeHasChanged``: Operation has changed geometry type

"""
# --
Qgis.GeometryOperationResult.baseClass = Qgis
QgsGeometry.ValidityFlag = Qgis.GeometryValidityFlag
# monkey patching scoped based enum
QgsGeometry.FlagAllowSelfTouchingHoles = Qgis.GeometryValidityFlag.AllowSelfTouchingHoles
QgsGeometry.ValidityFlag.FlagAllowSelfTouchingHoles = Qgis.GeometryValidityFlag.AllowSelfTouchingHoles
QgsGeometry.FlagAllowSelfTouchingHoles.is_monkey_patched = True
QgsGeometry.FlagAllowSelfTouchingHoles.__doc__ = "Indicates that self-touching holes are permitted. OGC validity states that self-touching holes are NOT permitted, whilst other vendor validity checks (e.g. ESRI) permit self-touching holes."
Qgis.GeometryValidityFlag.__doc__ = """Geometry validity check flags.

.. versionadded:: 3.22

* ``AllowSelfTouchingHoles``: Indicates that self-touching holes are permitted. OGC validity states that self-touching holes are NOT permitted, whilst other vendor validity checks (e.g. ESRI) permit self-touching holes.

  Available as ``QgsGeometry.FlagAllowSelfTouchingHoles`` in older QGIS releases.


"""
# --
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
Qgis.GeometryValidationEngine.__doc__ = """Available engines for validating geometries.

.. versionadded:: 3.22

* ``QgisInternal``: Use internal QgsGeometryValidator method

  Available as ``QgsGeometry.ValidatorQgisInternal`` in older QGIS releases.

* ``Geos``: Use GEOS validation methods

  Available as ``QgsGeometry.ValidatorGeos`` in older QGIS releases.


"""
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
Qgis.BufferSide.__doc__ = """Side of line to buffer.

.. versionadded:: 3.22

* ``Left``: Buffer to left of line

  Available as ``QgsGeometry.SideLeft`` in older QGIS releases.

* ``Right``: Buffer to right of line

  Available as ``QgsGeometry.SideRight`` in older QGIS releases.


"""
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
Qgis.EndCapStyle.__doc__ = """End cap styles for buffers.

.. versionadded:: 3.22

* ``Round``: Round cap

  Available as ``QgsGeometry.CapRound`` in older QGIS releases.

* ``Flat``: Flat cap (in line with start/end of line)

  Available as ``QgsGeometry.CapFlat`` in older QGIS releases.

* ``Square``: Square cap (extends past start/end of line by buffer distance)

  Available as ``QgsGeometry.CapSquare`` in older QGIS releases.


"""
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
Qgis.JoinStyle.__doc__ = """Join styles for buffers.

.. versionadded:: 3.22

* ``Round``: Use rounded joins

  Available as ``QgsGeometry.JoinStyleRound`` in older QGIS releases.

* ``Miter``: Use mitered joins

  Available as ``QgsGeometry.JoinStyleMiter`` in older QGIS releases.

* ``Bevel``: Use beveled joins

  Available as ``QgsGeometry.JoinStyleBevel`` in older QGIS releases.


"""
# --
Qgis.JoinStyle.baseClass = Qgis
# monkey patching scoped based enum
Qgis.GeosCreationFlag.RejectOnInvalidSubGeometry.__doc__ = "Don't allow geometries with invalid sub-geometries to be created"
Qgis.GeosCreationFlag.SkipEmptyInteriorRings.__doc__ = "Skip any empty polygon interior ring"
Qgis.GeosCreationFlag.__doc__ = """Flags which control geos geometry creation behavior.

.. versionadded:: 3.40

* ``RejectOnInvalidSubGeometry``: Don't allow geometries with invalid sub-geometries to be created
* ``SkipEmptyInteriorRings``: Skip any empty polygon interior ring

"""
# --
Qgis.GeosCreationFlag.baseClass = Qgis
Qgis.GeosCreationFlags.baseClass = Qgis
GeosCreationFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.CoverageValidityResult.Invalid.__doc__ = "Coverage is invalid. Invalidity includes polygons that overlap, that have gaps smaller than the gap width, or non-polygonal entries in the input collection."
Qgis.CoverageValidityResult.Valid.__doc__ = "Coverage is valid"
Qgis.CoverageValidityResult.Error.__doc__ = "An exception occurred while determining validity"
Qgis.CoverageValidityResult.__doc__ = """Coverage validity results.

.. versionadded:: 3.36

* ``Invalid``: Coverage is invalid. Invalidity includes polygons that overlap, that have gaps smaller than the gap width, or non-polygonal entries in the input collection.
* ``Valid``: Coverage is valid
* ``Error``: An exception occurred while determining validity

"""
# --
Qgis.CoverageValidityResult.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MakeValidMethod.Linework.__doc__ = "Combines all rings into a set of noded lines and then extracts valid polygons from that linework."
Qgis.MakeValidMethod.Structure.__doc__ = "Structured method, first makes all rings valid and then merges shells and subtracts holes from shells to generate valid result. Assumes that holes and shells are correctly categorized. Requires GEOS 3.10+."
Qgis.MakeValidMethod.__doc__ = """Algorithms to use when repairing invalid geometries.

.. versionadded:: 3.28

* ``Linework``: Combines all rings into a set of noded lines and then extracts valid polygons from that linework.
* ``Structure``: Structured method, first makes all rings valid and then merges shells and subtracts holes from shells to generate valid result. Assumes that holes and shells are correctly categorized. Requires GEOS 3.10+.

"""
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
QgsFeatureRequest.IgnoreStaticNodesDuringExpressionCompilation.__doc__ = "If a feature request uses a filter expression which can be partially precalculated due to static nodes in the expression, setting this flag will prevent these precalculated values from being utilized during compilation of the filter for the backend provider. This flag significantly slows down feature requests and should be used for debugging purposes only. \n.. versionadded:: 3.18"
QgsFeatureRequest.EmbeddedSymbols = Qgis.FeatureRequestFlag.EmbeddedSymbols
QgsFeatureRequest.EmbeddedSymbols.is_monkey_patched = True
QgsFeatureRequest.EmbeddedSymbols.__doc__ = "Retrieve any embedded feature symbology \n.. versionadded:: 3.20"
Qgis.FeatureRequestFlag.__doc__ = """Flags for controlling feature requests.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureRequest`.Flag

.. versionadded:: 3.36

* ``NoFlags``: No flags are set
* ``NoGeometry``: Geometry is not required. It may still be returned if e.g. required for a filter condition.
* ``SubsetOfAttributes``: Fetch only a subset of attributes (setSubsetOfAttributes sets this flag)
* ``ExactIntersect``: Use exact geometry intersection (slower) instead of bounding boxes
* ``IgnoreStaticNodesDuringExpressionCompilation``: If a feature request uses a filter expression which can be partially precalculated due to static nodes in the expression, setting this flag will prevent these precalculated values from being utilized during compilation of the filter for the backend provider. This flag significantly slows down feature requests and should be used for debugging purposes only.

  .. versionadded:: 3.18

* ``EmbeddedSymbols``: Retrieve any embedded feature symbology

  .. versionadded:: 3.20


"""
# --
Qgis.FeatureRequestFlag.baseClass = Qgis
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
Qgis.FeatureRequestFilterType.__doc__ = """Types of feature request filters.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureRequest`.FilterType

.. versionadded:: 3.36

* ``NoFilter``: No filter is applied

  Available as ``QgsFeatureRequest.FilterNone`` in older QGIS releases.

* ``Fid``: Filter using feature ID

  Available as ``QgsFeatureRequest.FilterFid`` in older QGIS releases.

* ``Expression``: Filter using expression

  Available as ``QgsFeatureRequest.FilterExpression`` in older QGIS releases.

* ``Fids``: Filter using feature IDs

  Available as ``QgsFeatureRequest.FilterFids`` in older QGIS releases.


"""
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
Qgis.InvalidGeometryCheck.__doc__ = """Methods for handling of features with invalid geometries

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsFeatureRequest`.InvalidGeometryCheck

.. versionadded:: 3.36

* ``NoCheck``: No invalid geometry checking

  Available as ``QgsFeatureRequest.GeometryNoCheck`` in older QGIS releases.

* ``SkipInvalid``: Skip any features with invalid geometry. This requires a slow geometry validity check for every feature.

  Available as ``QgsFeatureRequest.GeometrySkipInvalid`` in older QGIS releases.

* ``AbortOnInvalid``: Close iterator on encountering any features with invalid geometry. This requires a slow geometry validity check for every feature.

  Available as ``QgsFeatureRequest.GeometryAbortOnInvalid`` in older QGIS releases.


"""
# --
Qgis.InvalidGeometryCheck.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SpatialFilterType.NoFilter.__doc__ = "No spatial filtering of features"
Qgis.SpatialFilterType.BoundingBox.__doc__ = "Filter using a bounding box"
Qgis.SpatialFilterType.DistanceWithin.__doc__ = "Filter by distance to reference geometry"
Qgis.SpatialFilterType.__doc__ = """Feature request spatial filter types.

.. versionadded:: 3.22

* ``NoFilter``: No spatial filtering of features
* ``BoundingBox``: Filter using a bounding box
* ``DistanceWithin``: Filter by distance to reference geometry

"""
# --
Qgis.SpatialFilterType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FileOperationFlag.IncludeMetadataFile.__doc__ = "Indicates that any associated .qmd metadata file should be included with the operation"
Qgis.FileOperationFlag.IncludeStyleFile.__doc__ = "Indicates that any associated .qml styling file should be included with the operation"
Qgis.FileOperationFlag.__doc__ = """File operation flags.

.. versionadded:: 3.22

* ``IncludeMetadataFile``: Indicates that any associated .qmd metadata file should be included with the operation
* ``IncludeStyleFile``: Indicates that any associated .qml styling file should be included with the operation

"""
# --
Qgis.FileOperationFlag.baseClass = Qgis
Qgis.FileOperationFlags.baseClass = Qgis
FileOperationFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.MapLayerProperty.UsersCannotToggleEditing.__doc__ = "Indicates that users are not allowed to toggle editing for this layer. Note that this does not imply that the layer is non-editable (see isEditable(), supportsEditing() ), rather that the editable status of the layer cannot be changed by users manually \n.. versionadded:: 3.22"
Qgis.MapLayerProperty.IsBasemapLayer.__doc__ = "Layer is considered a 'basemap' layer, and certain properties of the layer should be ignored when calculating project-level properties. For instance, the extent of basemap layers is ignored when calculating the extent of a project, as these layers are typically global and extend outside of a project's area of interest \n.. versionadded:: 3.26"
Qgis.MapLayerProperty.__doc__ = """Generic map layer properties.

.. versionadded:: 3.22

* ``UsersCannotToggleEditing``: Indicates that users are not allowed to toggle editing for this layer. Note that this does not imply that the layer is non-editable (see isEditable(), supportsEditing() ), rather that the editable status of the layer cannot be changed by users manually

  .. versionadded:: 3.22

* ``IsBasemapLayer``: Layer is considered a 'basemap' layer, and certain properties of the layer should be ignored when calculating project-level properties. For instance, the extent of basemap layers is ignored when calculating the extent of a project, as these layers are typically global and extend outside of a project's area of interest

  .. versionadded:: 3.26


"""
# --
Qgis.MapLayerProperty.baseClass = Qgis
Qgis.MapLayerProperties.baseClass = Qgis
MapLayerProperties = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.AutoRefreshMode.Disabled.__doc__ = "Automatic refreshing is disabled"
Qgis.AutoRefreshMode.ReloadData.__doc__ = "Reload data (and draw the new data)"
Qgis.AutoRefreshMode.RedrawOnly.__doc__ = "Redraw current data only"
Qgis.AutoRefreshMode.__doc__ = """Map layer automatic refresh modes.

.. versionadded:: 3.34

* ``Disabled``: Automatic refreshing is disabled
* ``ReloadData``: Reload data (and draw the new data)
* ``RedrawOnly``: Redraw current data only

"""
# --
Qgis.AutoRefreshMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DataProviderFlag.IsBasemapSource.__doc__ = "Associated source should be considered a 'basemap' layer. See Qgis.MapLayerProperty.IsBasemapLayer."
Qgis.DataProviderFlag.FastExtent2D.__doc__ = "Provider's 2D extent retrieval via QgsDataProvider.extent() is always guaranteed to be trivial/fast to calculate \n.. versionadded:: 3.38"
Qgis.DataProviderFlag.FastExtent3D.__doc__ = "Provider's 3D extent retrieval via QgsDataProvider.extent3D() is always guaranteed to be trivial/fast to calculate \n.. versionadded:: 3.38"
Qgis.DataProviderFlag.__doc__ = """Generic data provider flags.

.. versionadded:: 3.26

* ``IsBasemapSource``: Associated source should be considered a 'basemap' layer. See Qgis.MapLayerProperty.IsBasemapLayer.
* ``FastExtent2D``: Provider's 2D extent retrieval via QgsDataProvider.extent() is always guaranteed to be trivial/fast to calculate

  .. versionadded:: 3.38

* ``FastExtent3D``: Provider's 3D extent retrieval via QgsDataProvider.extent3D() is always guaranteed to be trivial/fast to calculate

  .. versionadded:: 3.38


"""
# --
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
Qgis.CrsType.__doc__ = """Coordinate reference system types.

Contains a subset of Proj's PJ_TYPE enum, specifically the types which relate to CRS types.

.. versionadded:: 3.34

* ``Unknown``: Unknown type
* ``Geodetic``: Geodetic CRS
* ``Geocentric``: Geocentric CRS
* ``Geographic2d``: 2D geographic CRS
* ``Geographic3d``: 3D geopraphic CRS
* ``Vertical``: Vertical CRS
* ``Projected``: Projected CRS
* ``Compound``: Compound (horizontal + vertical) CRS
* ``Temporal``: Temporal CRS
* ``Engineering``: Engineering CRS
* ``Bound``: Bound CRS
* ``Other``: Other type
* ``DerivedProjected``: Derived projected CRS

"""
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
Qgis.CrsAxisDirection.__doc__ = """Coordinate reference system axis directions.

From "Geographic information — Well-known text representation of coordinate reference systems", section 7.5.1.

.. versionadded:: 3.26

* ``North``: North
* ``NorthNorthEast``: North North East
* ``NorthEast``: North East
* ``EastNorthEast``: East North East
* ``East``: East
* ``EastSouthEast``: East South East
* ``SouthEast``: South East
* ``SouthSouthEast``: South South East
* ``South``: South
* ``SouthSouthWest``: South South West
* ``SouthWest``: South West
* ``WestSouthWest``: West South West
* ``West``: West
* ``WestNorthWest``: West North West
* ``NorthWest``: North West
* ``NorthNorthWest``: North North West
* ``GeocentricX``: Geocentric (X)
* ``GeocentricY``: Geocentric (Y)
* ``GeocentricZ``: Geocentric (Z)
* ``Up``: Up
* ``Down``: Down
* ``Forward``: Forward
* ``Aft``: Aft
* ``Port``: Port
* ``Starboard``: Starboard
* ``Clockwise``: Clockwise
* ``CounterClockwise``: Counter clockwise
* ``ColumnPositive``: Column positive
* ``ColumnNegative``: Column negative
* ``RowPositive``: Row positive
* ``RowNegative``: Row negative
* ``DisplayRight``: Display right
* ``DisplayLeft``: Display left
* ``DisplayUp``: Display up
* ``DisplayDown``: Display down
* ``Future``: Future
* ``Past``: Past
* ``Towards``: Towards
* ``AwayFrom``: Away from
* ``Unspecified``: Unspecified

"""
# --
Qgis.CrsAxisDirection.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CoordinateOrder.Default.__doc__ = "Respect the default axis ordering for the CRS, as defined in the CRS's parameters"
Qgis.CoordinateOrder.XY.__doc__ = "Easting/Northing (or Longitude/Latitude for geographic CRS)"
Qgis.CoordinateOrder.YX.__doc__ = "Northing/Easting (or Latitude/Longitude for geographic CRS)"
Qgis.CoordinateOrder.__doc__ = """Order of coordinates.

.. versionadded:: 3.26

* ``Default``: Respect the default axis ordering for the CRS, as defined in the CRS's parameters
* ``XY``: Easting/Northing (or Longitude/Latitude for geographic CRS)
* ``YX``: Northing/Easting (or Latitude/Longitude for geographic CRS)

"""
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
Qgis.CrsIdentifierType.__doc__ = """Available identifier string types for representing coordinate reference systems

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsCoordinateReferenceSystem`.IdentifierType

.. versionadded:: 3.36

* ``ShortString``: A heavily abbreviated string, for use when a compact representation is required
* ``MediumString``: A medium-length string, recommended for general purpose use
* ``FullString``: Full definition -- possibly a very lengthy string, e.g. with no truncation of custom WKT definitions

"""
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
Qgis.CrsWktVariant.__doc__ = """Coordinate reference system WKT formatting variants.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsCoordinateReferenceSystem`.WktVariant

.. versionadded:: 3.36

* ``Wkt1Gdal``: WKT1 as traditionally output by GDAL, deriving from OGC 01-009. A notable departure from WKT1_GDAL with respect to OGC 01-009 is that in WKT1_GDAL, the unit of the PRIMEM value is always degrees.

  Available as ``QgsCoordinateReferenceSystem.WKT1_GDAL`` in older QGIS releases.

* ``Wkt1Esri``: WKT1 as traditionally output by ESRI software, deriving from OGC 99-049.

  Available as ``QgsCoordinateReferenceSystem.WKT1_ESRI`` in older QGIS releases.

* ``Wkt2_2015``: Full WKT2 string, conforming to ISO 19162:2015(E) / OGC 12-063r5 with all possible nodes and new keyword names.

  Available as ``QgsCoordinateReferenceSystem.WKT2_2015`` in older QGIS releases.

* ``Wkt2_2015Simplified``: Same as WKT2_2015 with the following exceptions: UNIT keyword used. ID node only on top element. No ORDER element in AXIS element. PRIMEM node omitted if it is Greenwich.  ELLIPSOID.UNIT node omitted if it is UnitOfMeasure.METRE. PARAMETER.UNIT / PRIMEM.UNIT omitted if same as AXIS. AXIS.UNIT omitted and replaced by a common GEODCRS.UNIT if they are all the same on all axis.

  Available as ``QgsCoordinateReferenceSystem.WKT2_2015_SIMPLIFIED`` in older QGIS releases.

* ``Wkt2_2019``: Full WKT2 string, conforming to ISO 19162:2019 / OGC 18-010, with all possible nodes and new keyword names. Non-normative list of differences: WKT2_2019 uses GEOGCRS / BASEGEOGCRS keywords for GeographicCRS.

  Available as ``QgsCoordinateReferenceSystem.WKT2_2019`` in older QGIS releases.

* ``Wkt2_2019Simplified``: WKT2_2019 with the simplification rule of WKT2_SIMPLIFIED

  Available as ``QgsCoordinateReferenceSystem.WKT2_2019_SIMPLIFIED`` in older QGIS releases.

* ``Preferred``: Preferred format, matching the most recent WKT ISO standard. Currently an alias to WKT2_2019, but may change in future versions.

  Available as ``QgsCoordinateReferenceSystem.WKT_PREFERRED`` in older QGIS releases.

* ``PreferredSimplified``: Preferred simplified format, matching the most recent WKT ISO standard. Currently an alias to WKT2_2019_SIMPLIFIED, but may change in future versions.

  Available as ``QgsCoordinateReferenceSystem.WKT_PREFERRED_SIMPLIFIED`` in older QGIS releases.

* ``PreferredGdal``: Preferred format for conversion of CRS to WKT for use with the GDAL library.

  Available as ``QgsCoordinateReferenceSystem.WKT_PREFERRED_GDAL`` in older QGIS releases.


"""
# --
Qgis.CrsWktVariant.baseClass = Qgis
# monkey patching scoped based enum
Qgis.Axis.X.__doc__ = "X-axis"
Qgis.Axis.Y.__doc__ = "Y-axis"
Qgis.Axis.Z.__doc__ = "Z-axis"
Qgis.Axis.__doc__ = """Cartesian axes.

.. versionadded:: 3.34

* ``X``: X-axis
* ``Y``: Y-axis
* ``Z``: Z-axis

"""
# --
Qgis.Axis.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AnnotationItemFlag.ScaleDependentBoundingBox.__doc__ = "Item's bounding box will vary depending on map scale"
Qgis.AnnotationItemFlag.SupportsReferenceScale.__doc__ = "Item supports reference scale based rendering \n.. versionadded:: 3.40"
Qgis.AnnotationItemFlag.SupportsCallouts.__doc__ = "Item supports callouts \n.. versionadded:: 3.40"
Qgis.AnnotationItemFlag.__doc__ = """Flags for annotation items.

.. versionadded:: 3.22

* ``ScaleDependentBoundingBox``: Item's bounding box will vary depending on map scale
* ``SupportsReferenceScale``: Item supports reference scale based rendering

  .. versionadded:: 3.40

* ``SupportsCallouts``: Item supports callouts

  .. versionadded:: 3.40


"""
# --
Qgis.AnnotationItemFlag.baseClass = Qgis
Qgis.AnnotationItemFlags.baseClass = Qgis
AnnotationItemFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.AnnotationPlacementMode.SpatialBounds.__doc__ = "Item is rendered inside fixed spatial bounds, and size will depend on map scale"
Qgis.AnnotationPlacementMode.FixedSize.__doc__ = "Item is rendered at a fixed size, regardless of map scale. Item's location is georeferenced to a spatial location."
Qgis.AnnotationPlacementMode.RelativeToMapFrame.__doc__ = "Items size and placement is relative to the map's frame, and the item will always be rendered in the same visible location regardless of map extent or scale."
Qgis.AnnotationPlacementMode.__doc__ = """Annotation item placement modes.

.. versionadded:: 3.40

* ``SpatialBounds``: Item is rendered inside fixed spatial bounds, and size will depend on map scale
* ``FixedSize``: Item is rendered at a fixed size, regardless of map scale. Item's location is georeferenced to a spatial location.
* ``RelativeToMapFrame``: Items size and placement is relative to the map's frame, and the item will always be rendered in the same visible location regardless of map extent or scale.

"""
# --
Qgis.AnnotationPlacementMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AnnotationItemGuiFlag.FlagNoCreationTools.__doc__ = "Do not show item creation tools for the item type"
Qgis.AnnotationItemGuiFlag.__doc__ = """Flags for controlling how an annotation item behaves in the GUI.

.. versionadded:: 3.22

* ``FlagNoCreationTools``: Do not show item creation tools for the item type

"""
# --
Qgis.AnnotationItemGuiFlag.baseClass = Qgis
Qgis.AnnotationItemGuiFlags.baseClass = Qgis
AnnotationItemGuiFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.AnnotationItemNodeType.VertexHandle.__doc__ = "Node is a handle for manipulating vertices"
Qgis.AnnotationItemNodeType.CalloutHandle.__doc__ = "Node is a handle for manipulating callouts \n.. versionadded:: 3.40"
Qgis.AnnotationItemNodeType.__doc__ = """Annotation item node types.

.. versionadded:: 3.22

* ``VertexHandle``: Node is a handle for manipulating vertices
* ``CalloutHandle``: Node is a handle for manipulating callouts

  .. versionadded:: 3.40


"""
# --
Qgis.AnnotationItemNodeType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AnnotationItemEditOperationResult.Success.__doc__ = "Item was modified successfully"
Qgis.AnnotationItemEditOperationResult.Invalid.__doc__ = "Operation has invalid parameters for the item, no change occurred"
Qgis.AnnotationItemEditOperationResult.ItemCleared.__doc__ = "The operation results in the item being cleared, and the item should be removed from the layer as a result"
Qgis.AnnotationItemEditOperationResult.__doc__ = """Results from an edit operation on an annotation item.

.. versionadded:: 3.22

* ``Success``: Item was modified successfully
* ``Invalid``: Operation has invalid parameters for the item, no change occurred
* ``ItemCleared``: The operation results in the item being cleared, and the item should be removed from the layer as a result

"""
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
QgsTemporalNavigationObject.Movie.__doc__ = "Movie mode -- behaves like a video player, with a fixed frame duration and no temporal range \n.. versionadded:: 3.36"
Qgis.TemporalNavigationMode.__doc__ = """Temporal navigation modes.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsTemporalNavigationObject`.NavigationMode

.. versionadded:: 3.36

* ``Disabled``: Temporal navigation is disabled

  Available as ``QgsTemporalNavigationObject.NavigationOff`` in older QGIS releases.

* ``Animated``: Temporal navigation relies on frames within a datetime range
* ``FixedRange``: Temporal navigation relies on a fixed datetime range
* ``Movie``: Movie mode -- behaves like a video player, with a fixed frame duration and no temporal range

  .. versionadded:: 3.36


"""
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
Qgis.AnimationState.__doc__ = """Animation states.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsTemporalNavigationObject`.AnimationState

.. versionadded:: 3.36

* ``Forward``: Animation is playing forward.
* ``Reverse``: Animation is playing in reverse.
* ``Idle``: Animation is paused.

"""
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
Qgis.PlaybackOperation.__doc__ = """Media playback operations.

.. versionadded:: 3.36

* ``SkipToStart``: Jump to start of playback
* ``PreviousFrame``: Step to previous frame
* ``PlayReverse``: Play in reverse
* ``Pause``: Pause playback
* ``PlayForward``: Play forward
* ``NextFrame``: Step to next frame
* ``SkipToEnd``: Jump to end of playback

"""
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
Qgis.VectorTemporalMode.__doc__ = """Vector layer temporal feature modes

.. versionadded:: 3.22

* ``FixedTemporalRange``: Mode when temporal properties have fixed start and end datetimes.

  Available as ``QgsVectorLayerTemporalProperties.ModeFixedTemporalRange`` in older QGIS releases.

* ``FeatureDateTimeInstantFromField``: Mode when features have a datetime instant taken from a single field

  Available as ``QgsVectorLayerTemporalProperties.ModeFeatureDateTimeInstantFromField`` in older QGIS releases.

* ``FeatureDateTimeStartAndEndFromFields``: Mode when features have separate fields for start and end times

  Available as ``QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromFields`` in older QGIS releases.

* ``FeatureDateTimeStartAndDurationFromFields``: Mode when features have a field for start time and a field for event duration

  Available as ``QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndDurationFromFields`` in older QGIS releases.

* ``FeatureDateTimeStartAndEndFromExpressions``: Mode when features use expressions for start and end times

  Available as ``QgsVectorLayerTemporalProperties.ModeFeatureDateTimeStartAndEndFromExpressions`` in older QGIS releases.

* ``RedrawLayerOnly``: Redraw the layer when temporal range changes, but don't apply any filtering. Useful when symbology or rule based renderer expressions depend on the time range.

  Available as ``QgsVectorLayerTemporalProperties.ModeRedrawLayerOnly`` in older QGIS releases.


"""
# --
Qgis.VectorTemporalMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorTemporalLimitMode.IncludeBeginExcludeEnd.__doc__ = "Default mode: include the Begin limit, but exclude the End limit"
Qgis.VectorTemporalLimitMode.IncludeBeginIncludeEnd.__doc__ = "Mode to include both limits of the filtering timeframe"
Qgis.VectorTemporalLimitMode.__doc__ = """Mode for the handling of the limits of the filtering timeframe for vector features

.. versionadded:: 3.22

* ``IncludeBeginExcludeEnd``: Default mode: include the Begin limit, but exclude the End limit
* ``IncludeBeginIncludeEnd``: Mode to include both limits of the filtering timeframe

"""
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
Qgis.VectorDataProviderTemporalMode.__doc__ = """Vector data provider temporal handling modes.

.. versionadded:: 3.22

* ``HasFixedTemporalRange``: Entire dataset from provider has a fixed start and end datetime.

  Available as ``QgsVectorDataProviderTemporalCapabilities.ProviderHasFixedTemporalRange`` in older QGIS releases.

* ``StoresFeatureDateTimeInstantInField``: Dataset has feature datetime instants stored in a single field

  Available as ``QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeInstantInField`` in older QGIS releases.

* ``StoresFeatureDateTimeStartAndEndInSeparateFields``: Dataset stores feature start and end datetimes in separate fields

  Available as ``QgsVectorDataProviderTemporalCapabilities.ProviderStoresFeatureDateTimeStartAndEndInSeparateFields`` in older QGIS releases.


"""
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
QgsRasterLayerTemporalProperties.ModeRedrawLayerOnly.__doc__ = "Redraw the layer when temporal range changes, but don't apply any filtering. Useful when raster symbology expressions depend on the time range. \n.. versionadded:: 3.22"
QgsRasterLayerTemporalProperties.FixedRangePerBand = Qgis.RasterTemporalMode.FixedRangePerBand
QgsRasterLayerTemporalProperties.FixedRangePerBand.is_monkey_patched = True
QgsRasterLayerTemporalProperties.FixedRangePerBand.__doc__ = "Layer has a fixed temporal range per band \n.. versionadded:: 3.38"
QgsRasterLayerTemporalProperties.RepresentsTemporalValues = Qgis.RasterTemporalMode.RepresentsTemporalValues
QgsRasterLayerTemporalProperties.RepresentsTemporalValues.is_monkey_patched = True
QgsRasterLayerTemporalProperties.RepresentsTemporalValues.__doc__ = "Pixel values represent an datetime"
Qgis.RasterTemporalMode.__doc__ = """Raster layer temporal modes

.. versionadded:: 3.22

* ``FixedTemporalRange``: Mode when temporal properties have fixed start and end datetimes.

  Available as ``QgsRasterLayerTemporalProperties.ModeFixedTemporalRange`` in older QGIS releases.

* ``TemporalRangeFromDataProvider``: Mode when raster layer delegates temporal range handling to the dataprovider.

  Available as ``QgsRasterLayerTemporalProperties.ModeTemporalRangeFromDataProvider`` in older QGIS releases.

* ``RedrawLayerOnly``: Redraw the layer when temporal range changes, but don't apply any filtering. Useful when raster symbology expressions depend on the time range.

  .. versionadded:: 3.22


  Available as ``QgsRasterLayerTemporalProperties.ModeRedrawLayerOnly`` in older QGIS releases.

* ``FixedRangePerBand``: Layer has a fixed temporal range per band

  .. versionadded:: 3.38

* ``RepresentsTemporalValues``: Pixel values represent an datetime

"""
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
Qgis.TemporalIntervalMatchMethod.__doc__ = """Method to use when resolving a temporal range to a data provider layer or band.

.. versionadded:: 3.22

* ``MatchUsingWholeRange``: Use an exact match to the whole temporal range
* ``MatchExactUsingStartOfRange``: Match the start of the temporal range to a corresponding layer or band, and only use exact matching results
* ``MatchExactUsingEndOfRange``: Match the end of the temporal range to a corresponding layer or band, and only use exact matching results
* ``FindClosestMatchToStartOfRange``: Match the start of the temporal range to the least previous closest datetime.
* ``FindClosestMatchToEndOfRange``: Match the end of the temporal range to the least previous closest datetime.

"""
# --
Qgis.TemporalIntervalMatchMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RasterTemporalCapabilityFlag.RequestedTimesMustExactlyMatchAllAvailableTemporalRanges.__doc__ = "If present, indicates that the provider must only request temporal values which are exact matches for the values present in QgsRasterDataProviderTemporalCapabilities.allAvailableTemporalRanges()."
Qgis.RasterTemporalCapabilityFlag.__doc__ = """Flags for raster layer temporal capabilities.

.. versionadded:: 3.28

* ``RequestedTimesMustExactlyMatchAllAvailableTemporalRanges``: If present, indicates that the provider must only request temporal values which are exact matches for the values present in QgsRasterDataProviderTemporalCapabilities.allAvailableTemporalRanges().

"""
# --
Qgis.RasterTemporalCapabilityFlag.baseClass = Qgis
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
Qgis.TransformDirection.__doc__ = """Indicates the direction (forward or inverse) of a transform.

.. versionadded:: 3.22

* ``Forward``: Forward transform (from source to destination)

  Available as ``QgsCoordinateTransform.ForwardTransform`` in older QGIS releases.

* ``Reverse``: Reverse/inverse transform (from destination to source)

  Available as ``QgsCoordinateTransform.ReverseTransform`` in older QGIS releases.


"""
# --
Qgis.TransformDirection.baseClass = Qgis
# monkey patching scoped based enum
Qgis.CoordinateTransformationFlag.BallparkTransformsAreAppropriate.__doc__ = "Indicates that approximate \"ballpark\" results are appropriate for this coordinate transform. See QgsCoordinateTransform.setBallparkTransformsAreAppropriate() for further details."
Qgis.CoordinateTransformationFlag.IgnoreImpossibleTransformations.__doc__ = "Indicates that impossible transformations (such as those which attempt to transform between two different celestial bodies) should be silently handled and marked as invalid. See QgsCoordinateTransform.isTransformationPossible() and QgsCoordinateTransform.isValid()."
Qgis.CoordinateTransformationFlag.__doc__ = """Flags which adjust the coordinate transformations behave.

.. versionadded:: 3.26

* ``BallparkTransformsAreAppropriate``: Indicates that approximate \"ballpark\" results are appropriate for this coordinate transform. See QgsCoordinateTransform.setBallparkTransformsAreAppropriate() for further details.
* ``IgnoreImpossibleTransformations``: Indicates that impossible transformations (such as those which attempt to transform between two different celestial bodies) should be silently handled and marked as invalid. See QgsCoordinateTransform.isTransformationPossible() and QgsCoordinateTransform.isValid().

"""
# --
Qgis.CoordinateTransformationFlag.baseClass = Qgis
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
QgsMapSettings.HighQualityImageTransforms.__doc__ = "Enable high quality image transformations, which results in better appearance of scaled or rotated raster components of a map \n.. versionadded:: 3.24"
QgsMapSettings.SkipSymbolRendering = Qgis.MapSettingsFlag.SkipSymbolRendering
QgsMapSettings.SkipSymbolRendering.is_monkey_patched = True
QgsMapSettings.SkipSymbolRendering.__doc__ = "Disable symbol rendering while still drawing labels if enabled \n.. versionadded:: 3.24"
QgsMapSettings.ForceRasterMasks = Qgis.MapSettingsFlag.ForceRasterMasks
QgsMapSettings.ForceRasterMasks.is_monkey_patched = True
QgsMapSettings.ForceRasterMasks.__doc__ = "Force symbol masking to be applied using a raster method. This is considerably faster when compared to the vector method, but results in a inferior quality output. \n.. versionadded:: 3.26.1"
QgsMapSettings.RecordProfile = Qgis.MapSettingsFlag.RecordProfile
QgsMapSettings.RecordProfile.is_monkey_patched = True
QgsMapSettings.RecordProfile.__doc__ = "Enable run-time profiling while rendering \n.. versionadded:: 3.34"
QgsMapSettings.AlwaysUseGlobalMasks = Qgis.MapSettingsFlag.AlwaysUseGlobalMasks
QgsMapSettings.AlwaysUseGlobalMasks.is_monkey_patched = True
QgsMapSettings.AlwaysUseGlobalMasks.__doc__ = "When applying clipping paths for selective masking, always use global (\"entire map\") paths, instead of calculating local clipping paths per rendered feature. This results in considerably more complex vector exports in all current Qt versions. This flag only applies to vector map exports. \n.. versionadded:: 3.38"
Qgis.MapSettingsFlag.__doc__ = """Flags which adjust the way maps are rendered.

.. versionadded:: 3.22

* ``Antialiasing``: Enable anti-aliasing for map rendering
* ``DrawEditingInfo``: Enable drawing of vertex markers for layers in editing mode
* ``ForceVectorOutput``: Vector graphics should not be cached and drawn as raster images
* ``UseAdvancedEffects``: Enable layer opacity and blending effects
* ``DrawLabeling``: Enable drawing of labels on top of the map
* ``UseRenderingOptimization``: Enable vector simplification and other rendering optimizations
* ``DrawSelection``: Whether vector selections should be shown in the rendered map
* ``DrawSymbolBounds``: Draw bounds of symbols (for debugging/testing)
* ``RenderMapTile``: Draw map such that there are no problems between adjacent tiles
* ``RenderPartialOutput``: Whether to make extra effort to update map image with partially rendered layers (better for interactive map canvas). Added in QGIS 3.0
* ``RenderPreviewJob``: Render is a 'canvas preview' render, and shortcuts should be taken to ensure fast rendering
* ``RenderBlocking``: Render and load remote sources in the same thread to ensure rendering remote sources (svg and images). WARNING: this flag must NEVER be used from GUI based applications (like the main QGIS application) or crashes will result. Only for use in external scripts or QGIS server.
* ``LosslessImageRendering``: Render images losslessly whenever possible, instead of the default lossy jpeg rendering used for some destination devices (e.g. PDF). This flag only works with builds based on Qt 5.13 or later.
* ``Render3DMap``: Render is for a 3D map
* ``HighQualityImageTransforms``: Enable high quality image transformations, which results in better appearance of scaled or rotated raster components of a map

  .. versionadded:: 3.24

* ``SkipSymbolRendering``: Disable symbol rendering while still drawing labels if enabled

  .. versionadded:: 3.24

* ``ForceRasterMasks``: Force symbol masking to be applied using a raster method. This is considerably faster when compared to the vector method, but results in a inferior quality output.

  .. versionadded:: 3.26.1

* ``RecordProfile``: Enable run-time profiling while rendering

  .. versionadded:: 3.34

* ``AlwaysUseGlobalMasks``: When applying clipping paths for selective masking, always use global (\"entire map\") paths, instead of calculating local clipping paths per rendered feature. This results in considerably more complex vector exports in all current Qt versions. This flag only applies to vector map exports.

  .. versionadded:: 3.38


"""
# --
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
QgsRenderContext.RenderingSubSymbol.__doc__ = "Set whenever a sub-symbol of a parent symbol is currently being rendered. Can be used during symbol and symbol layer rendering to determine whether the symbol being rendered is a subsymbol. \n.. versionadded:: 3.24"
QgsRenderContext.HighQualityImageTransforms = Qgis.RenderContextFlag.HighQualityImageTransforms
QgsRenderContext.HighQualityImageTransforms.is_monkey_patched = True
QgsRenderContext.HighQualityImageTransforms.__doc__ = "Enable high quality image transformations, which results in better appearance of scaled or rotated raster components of a map \n.. versionadded:: 3.24"
QgsRenderContext.SkipSymbolRendering = Qgis.RenderContextFlag.SkipSymbolRendering
QgsRenderContext.SkipSymbolRendering.is_monkey_patched = True
QgsRenderContext.SkipSymbolRendering.__doc__ = "Disable symbol rendering while still drawing labels if enabled \n.. versionadded:: 3.24"
QgsRenderContext.RecordProfile = Qgis.RenderContextFlag.RecordProfile
QgsRenderContext.RecordProfile.is_monkey_patched = True
QgsRenderContext.RecordProfile.__doc__ = "Enable run-time profiling while rendering \n.. versionadded:: 3.34"
QgsRenderContext.AlwaysUseGlobalMasks = Qgis.RenderContextFlag.AlwaysUseGlobalMasks
QgsRenderContext.AlwaysUseGlobalMasks.is_monkey_patched = True
QgsRenderContext.AlwaysUseGlobalMasks.__doc__ = "When applying clipping paths for selective masking, always use global (\"entire map\") paths, instead of calculating local clipping paths per rendered feature. This results in considerably more complex vector exports in all current Qt versions. This flag only applies to vector map exports. \n.. versionadded:: 3.38"
QgsRenderContext.DisableSymbolClippingToExtent = Qgis.RenderContextFlag.DisableSymbolClippingToExtent
QgsRenderContext.DisableSymbolClippingToExtent.is_monkey_patched = True
QgsRenderContext.DisableSymbolClippingToExtent.__doc__ = "Force symbol clipping to map extent to be disabled in all situations. This will result in slower rendering, and should only be used in situations where the feature clipping is always undesirable. \n.. versionadded:: 3.40"
Qgis.RenderContextFlag.__doc__ = """Flags which affect rendering operations.

.. versionadded:: 3.22

* ``DrawEditingInfo``: Enable drawing of vertex markers for layers in editing mode
* ``ForceVectorOutput``: Vector graphics should not be cached and drawn as raster images
* ``UseAdvancedEffects``: Enable layer opacity and blending effects
* ``UseRenderingOptimization``: Enable vector simplification and other rendering optimizations
* ``DrawSelection``: Whether vector selections should be shown in the rendered map
* ``DrawSymbolBounds``: Draw bounds of symbols (for debugging/testing)
* ``RenderMapTile``: Draw map such that there are no problems between adjacent tiles
* ``Antialiasing``: Use antialiasing while drawing
* ``RenderPartialOutput``: Whether to make extra effort to update map image with partially rendered layers (better for interactive map canvas). Added in QGIS 3.0
* ``RenderPreviewJob``: Render is a 'canvas preview' render, and shortcuts should be taken to ensure fast rendering
* ``RenderBlocking``: Render and load remote sources in the same thread to ensure rendering remote sources (svg and images). WARNING: this flag must NEVER be used from GUI based applications (like the main QGIS application) or crashes will result. Only for use in external scripts or QGIS server.
* ``RenderSymbolPreview``: The render is for a symbol preview only and map based properties may not be available, so care should be taken to handle map unit based sizes in an appropriate way.
* ``LosslessImageRendering``: Render images losslessly whenever possible, instead of the default lossy jpeg rendering used for some destination devices (e.g. PDF). This flag only works with builds based on Qt 5.13 or later.
* ``ApplyScalingWorkaroundForTextRendering``: Whether a scaling workaround designed to stablise the rendering of small font sizes (or for painters scaled out by a large amount) when rendering text. Generally this is recommended, but it may incur some performance cost.
* ``Render3DMap``: Render is for a 3D map
* ``ApplyClipAfterReprojection``: Feature geometry clipping to mapExtent() must be performed after the geometries are transformed using coordinateTransform(). Usually feature geometry clipping occurs using the extent() in the layer's CRS prior to geometry transformation, but in some cases when extent() could not be accurately calculated it is necessary to clip geometries to mapExtent() AFTER transforming them using coordinateTransform().
* ``RenderingSubSymbol``: Set whenever a sub-symbol of a parent symbol is currently being rendered. Can be used during symbol and symbol layer rendering to determine whether the symbol being rendered is a subsymbol.

  .. versionadded:: 3.24

* ``HighQualityImageTransforms``: Enable high quality image transformations, which results in better appearance of scaled or rotated raster components of a map

  .. versionadded:: 3.24

* ``SkipSymbolRendering``: Disable symbol rendering while still drawing labels if enabled

  .. versionadded:: 3.24

* ``RecordProfile``: Enable run-time profiling while rendering

  .. versionadded:: 3.34

* ``AlwaysUseGlobalMasks``: When applying clipping paths for selective masking, always use global (\"entire map\") paths, instead of calculating local clipping paths per rendered feature. This results in considerably more complex vector exports in all current Qt versions. This flag only applies to vector map exports.

  .. versionadded:: 3.38

* ``DisableSymbolClippingToExtent``: Force symbol clipping to map extent to be disabled in all situations. This will result in slower rendering, and should only be used in situations where the feature clipping is always undesirable.

  .. versionadded:: 3.40


"""
# --
QgsRenderContext.Flags = Qgis.RenderContextFlags
Qgis.RenderContextFlag.baseClass = Qgis
Qgis.RenderContextFlags.baseClass = Qgis
RenderContextFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.MapLayerRendererFlag.RenderPartialOutputs.__doc__ = "The renderer benefits from rendering temporary in-progress preview renders. These are temporary results which will be used for the layer during rendering in-progress compositions, which will differ from the final layer render. They can be used for showing overlays or other information to users which help inform them about what is actually occurring during a slow layer render, but where these overlays and additional content is not wanted in the final layer renders. Another use case is rendering unsorted results as soon as they are available, before doing a final sorted render of the entire layer contents."
Qgis.MapLayerRendererFlag.RenderPartialOutputOverPreviousCachedImage.__doc__ = "When rendering temporary in-progress preview renders, these preview renders can be drawn over any previously cached layer render we have for the same region. This can allow eg a low-resolution zoomed in version of the last map render to be used as a base painting surface to overdraw with incremental preview render outputs. If not set, an empty image will be used as the starting point for the render preview image."
Qgis.MapLayerRendererFlag.AffectsLabeling.__doc__ = "The layer rendering will interact with the map labeling \n.. versionadded:: 3.40"
Qgis.MapLayerRendererFlag.__doc__ = """Flags which control how map layer renderers behave.

.. versionadded:: 3.34

* ``RenderPartialOutputs``: The renderer benefits from rendering temporary in-progress preview renders. These are temporary results which will be used for the layer during rendering in-progress compositions, which will differ from the final layer render. They can be used for showing overlays or other information to users which help inform them about what is actually occurring during a slow layer render, but where these overlays and additional content is not wanted in the final layer renders. Another use case is rendering unsorted results as soon as they are available, before doing a final sorted render of the entire layer contents.
* ``RenderPartialOutputOverPreviousCachedImage``: When rendering temporary in-progress preview renders, these preview renders can be drawn over any previously cached layer render we have for the same region. This can allow eg a low-resolution zoomed in version of the last map render to be used as a base painting surface to overdraw with incremental preview render outputs. If not set, an empty image will be used as the starting point for the render preview image.
* ``AffectsLabeling``: The layer rendering will interact with the map labeling

  .. versionadded:: 3.40


"""
# --
Qgis.MapLayerRendererFlag.baseClass = Qgis
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
QgsRenderContext.PreferText = Qgis.TextRenderFormat.PreferText
QgsRenderContext.PreferText.is_monkey_patched = True
QgsRenderContext.PreferText.__doc__ = "Render text as text objects, unless doing so results in rendering artifacts or poor quality rendering (depending on text format settings). When rendering using TextFormatAlwaysText to a vector based device (e.g. PDF or SVG), care must be taken to ensure that the required fonts are available to users when opening the created files, or default fallback fonts will be used to display the output instead. (Although PDF exports MAY automatically embed some fonts when possible, depending on the user's platform). \n.. versionadded:: 3.40"
Qgis.TextRenderFormat.__doc__ = """Options for rendering text.

.. versionadded:: 3.22

* ``AlwaysOutlines``: Always render text using path objects (AKA outlines/curves). This setting guarantees the best quality rendering, even when using a raster paint surface (where sub-pixel path based text rendering is superior to sub-pixel text-based rendering). The downside is that text is converted to paths only, so users cannot open created vector outputs for post-processing in other applications and retain text editability.  This setting also guarantees complete compatibility with the full range of formatting options available through QgsTextRenderer and QgsTextFormat, some of which may not be possible to reproduce when using a vector-based paint surface and TextFormatAlwaysText mode. A final benefit to this setting is that vector exports created using text as outlines do not require all users to have the original fonts installed in order to display the text in its original style.

  Available as ``QgsRenderContext.TextFormatAlwaysOutlines`` in older QGIS releases.

* ``AlwaysText``: Always render text as text objects. While this mode preserves text objects as text for post-processing in external vector editing applications, it can result in rendering artifacts or poor quality rendering, depending on the text format settings. Even with raster based paint devices, TextFormatAlwaysText can result in inferior rendering quality to TextFormatAlwaysOutlines. When rendering using TextFormatAlwaysText to a vector based device (e.g. PDF or SVG), care must be taken to ensure that the required fonts are available to users when opening the created files, or default fallback fonts will be used to display the output instead. (Although PDF exports MAY automatically embed some fonts when possible, depending on the user's platform).

  Available as ``QgsRenderContext.TextFormatAlwaysText`` in older QGIS releases.

* ``PreferText``: Render text as text objects, unless doing so results in rendering artifacts or poor quality rendering (depending on text format settings). When rendering using TextFormatAlwaysText to a vector based device (e.g. PDF or SVG), care must be taken to ensure that the required fonts are available to users when opening the created files, or default fallback fonts will be used to display the output instead. (Although PDF exports MAY automatically embed some fonts when possible, depending on the user's platform).

  .. versionadded:: 3.40


"""
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
QgsLabelingEngineSettings.CollectUnplacedLabels.__doc__ = "Whether unplaced labels should be collected in the labeling results (regardless of whether they are being rendered) \n.. versionadded:: 3.20"
QgsLabelingEngineSettings.DrawLabelMetrics = Qgis.LabelingFlag.DrawLabelMetrics
QgsLabelingEngineSettings.DrawLabelMetrics.is_monkey_patched = True
QgsLabelingEngineSettings.DrawLabelMetrics.__doc__ = "Whether to render label metric guides (for debugging) \n.. versionadded:: 3.30"
Qgis.LabelingFlag.__doc__ = """Various flags that affect drawing and placement of labels.

Prior to QGIS 3.30 this was available as :py:class:`QgsLabelingEngineSettings`.Flag

.. versionadded:: 3.30

* ``UseAllLabels``: Whether to draw all labels even if there would be collisions
* ``UsePartialCandidates``: Whether to use also label candidates that are partially outside of the map view
* ``RenderOutlineLabels``: Whether to render labels as text or outlines. Deprecated and of QGIS 3.4.3 - use defaultTextRenderFormat() instead.
* ``DrawLabelRectOnly``: Whether to only draw the label rect and not the actual label text (used for unit tests)
* ``DrawCandidates``: Whether to draw rectangles of generated candidates (good for debugging)
* ``DrawUnplacedLabels``: Whether to render unplaced labels as an indicator/warning for users
* ``CollectUnplacedLabels``: Whether unplaced labels should be collected in the labeling results (regardless of whether they are being rendered)

  .. versionadded:: 3.20

* ``DrawLabelMetrics``: Whether to render label metric guides (for debugging)

  .. versionadded:: 3.30


"""
# --
Qgis.LabelingFlag.baseClass = Qgis
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
Qgis.LabelPlacementEngineVersion.__doc__ = """Labeling placement engine version.

Prior to QGIS 3.30 this was available as :py:class:`QgsLabelingEngineSettings`.PlacementEngineVersion

.. versionadded:: 3.30

* ``Version1``: Version 1, matches placement from QGIS <= 3.10.1

  Available as ``QgsLabelingEngineSettings.PlacementEngineVersion1`` in older QGIS releases.

* ``Version2``: Version 2 (default for new projects since QGIS 3.12)

  Available as ``QgsLabelingEngineSettings.PlacementEngineVersion2`` in older QGIS releases.


"""
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
Qgis.TextOrientation.__doc__ = """Text orientations.

.. note::

   Prior to QGIS 3.28 this was available as :py:class:`QgsTextFormat`.TextOrientation

.. versionadded:: 3.28

* ``Horizontal``: Horizontally oriented text

  Available as ``QgsTextFormat.HorizontalOrientation`` in older QGIS releases.

* ``Vertical``: Vertically oriented text

  Available as ``QgsTextFormat.VerticalOrientation`` in older QGIS releases.

* ``RotationBased``: Horizontally or vertically oriented text based on rotation (only available for map labeling)

  Available as ``QgsTextFormat.RotationBasedOrientation`` in older QGIS releases.


"""
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
QgsTextRenderer.RectangleCapHeightBased.__doc__ = "Similar to Rectangle mode, but uses cap height only when calculating font heights for the first line of text, and cap height + descent for subsequent lines of text \n.. versionadded:: 3.30"
QgsTextRenderer.RectangleAscentBased = Qgis.TextLayoutMode.RectangleAscentBased
QgsTextRenderer.RectangleAscentBased.is_monkey_patched = True
QgsTextRenderer.RectangleAscentBased.__doc__ = "Similar to Rectangle mode, but uses ascents only when calculating font and line heights. \n.. versionadded:: 3.30"
Qgis.TextLayoutMode.__doc__ = """Text layout modes.

.. note::

   Prior to QGIS 3.28 this was available as :py:class:`QgsTextRenderer`.DrawMode

.. versionadded:: 3.28

* ``Rectangle``: Text within rectangle layout mode

  Available as ``QgsTextRenderer.Rect`` in older QGIS releases.

* ``Point``: Text at point of origin layout mode
* ``Labeling``: Labeling-specific layout mode

  Available as ``QgsTextRenderer.Label`` in older QGIS releases.

* ``RectangleCapHeightBased``: Similar to Rectangle mode, but uses cap height only when calculating font heights for the first line of text, and cap height + descent for subsequent lines of text

  .. versionadded:: 3.30

* ``RectangleAscentBased``: Similar to Rectangle mode, but uses ascents only when calculating font and line heights.

  .. versionadded:: 3.30


"""
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
Qgis.TextComponent.__doc__ = """Text components.

.. note::

   Prior to QGIS 3.28 this was available as :py:class:`QgsTextRenderer`.TextPart

.. versionadded:: 3.28

* ``Text``: Text component
* ``Buffer``: Buffer component
* ``Background``: Background shape
* ``Shadow``: Drop shadow

"""
# --
Qgis.TextComponent.baseClass = Qgis
Qgis.TextComponents.baseClass = Qgis
TextComponents = Qgis  # dirty hack since SIP seems to introduce the flags in module
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
Qgis.TextHorizontalAlignment.__doc__ = """Text horizontal alignment.

.. note::

   Prior to QGIS 3.28 this was available as :py:class:`QgsTextRenderer`.HAlignment

.. versionadded:: 3.28

* ``Left``: Left align

  Available as ``QgsTextRenderer.AlignLeft`` in older QGIS releases.

* ``Center``: Center align

  Available as ``QgsTextRenderer.AlignCenter`` in older QGIS releases.

* ``Right``: Right align

  Available as ``QgsTextRenderer.AlignRight`` in older QGIS releases.

* ``Justify``: Justify align

  Available as ``QgsTextRenderer.AlignJustify`` in older QGIS releases.


"""
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
Qgis.TextVerticalAlignment.__doc__ = """Text vertical alignment.

This enum controls vertical alignment of text in a predefined rectangular
bounding box. See also Qgis.TextCharacterVerticalAlignment.

.. note::

   Prior to QGIS 3.28 this was available as :py:class:`QgsTextRenderer`.VAlignment

.. versionadded:: 3.28

* ``Top``: Align to top

  Available as ``QgsTextRenderer.AlignTop`` in older QGIS releases.

* ``VerticalCenter``: Center align

  Available as ``QgsTextRenderer.AlignVCenter`` in older QGIS releases.

* ``Bottom``: Align to bottom

  Available as ``QgsTextRenderer.AlignBottom`` in older QGIS releases.


"""
# --
Qgis.TextVerticalAlignment.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TextCharacterVerticalAlignment.Normal.__doc__ = "Adjacent characters are positioned in the standard way for text in the writing system in use"
Qgis.TextCharacterVerticalAlignment.SuperScript.__doc__ = "Characters are placed above the base line for normal text."
Qgis.TextCharacterVerticalAlignment.SubScript.__doc__ = "Characters are placed below the base line for normal text."
Qgis.TextCharacterVerticalAlignment.__doc__ = """Text vertical alignment for characters.

This enum controls vertical alignment of individual characters within a block
of text.

.. versionadded:: 3.30

* ``Normal``: Adjacent characters are positioned in the standard way for text in the writing system in use
* ``SuperScript``: Characters are placed above the base line for normal text.
* ``SubScript``: Characters are placed below the base line for normal text.

"""
# --
Qgis.TextCharacterVerticalAlignment.baseClass = Qgis
QgsVectorSimplifyMethod.SimplifyAlgorithm = Qgis.VectorSimplificationAlgorithm
# monkey patching scoped based enum
QgsVectorSimplifyMethod.Distance = Qgis.VectorSimplificationAlgorithm.Distance
QgsVectorSimplifyMethod.Distance.is_monkey_patched = True
QgsVectorSimplifyMethod.Distance.__doc__ = "The simplification uses the distance between points to remove duplicate points"
QgsVectorSimplifyMethod.SnapToGrid = Qgis.VectorSimplificationAlgorithm.SnapToGrid
QgsVectorSimplifyMethod.SnapToGrid.is_monkey_patched = True
QgsVectorSimplifyMethod.SnapToGrid.__doc__ = "The simplification uses a grid (similar to ST_SnapToGrid) to remove duplicate points"
QgsVectorSimplifyMethod.Visvalingam = Qgis.VectorSimplificationAlgorithm.Visvalingam
QgsVectorSimplifyMethod.Visvalingam.is_monkey_patched = True
QgsVectorSimplifyMethod.Visvalingam.__doc__ = "The simplification gives each point in a line an importance weighting, so that least important points are removed first"
QgsVectorSimplifyMethod.SnappedToGridGlobal = Qgis.VectorSimplificationAlgorithm.SnappedToGridGlobal
QgsVectorSimplifyMethod.SnappedToGridGlobal.is_monkey_patched = True
QgsVectorSimplifyMethod.SnappedToGridGlobal.__doc__ = "Snap to a global grid based on the tolerance. Good for consistent results for incoming vertices, regardless of their feature"
Qgis.VectorSimplificationAlgorithm.__doc__ = """Simplification algorithms for vector features.

.. note::

   Prior to QGIS 3.38 this was available as :py:class:`QgsVectorSimplifyMethod`.SimplifyAlgorithm

.. versionadded:: 3.38

* ``Distance``: The simplification uses the distance between points to remove duplicate points
* ``SnapToGrid``: The simplification uses a grid (similar to ST_SnapToGrid) to remove duplicate points
* ``Visvalingam``: The simplification gives each point in a line an importance weighting, so that least important points are removed first
* ``SnappedToGridGlobal``: Snap to a global grid based on the tolerance. Good for consistent results for incoming vertices, regardless of their feature

"""
# --
Qgis.VectorSimplificationAlgorithm.baseClass = Qgis
QgsVectorSimplifyMethod.SimplifyHint = Qgis.VectorRenderingSimplificationFlag
# monkey patching scoped based enum
QgsVectorSimplifyMethod.NoSimplification = Qgis.VectorRenderingSimplificationFlag.NoSimplification
QgsVectorSimplifyMethod.NoSimplification.is_monkey_patched = True
QgsVectorSimplifyMethod.NoSimplification.__doc__ = "No simplification can be applied"
QgsVectorSimplifyMethod.GeometrySimplification = Qgis.VectorRenderingSimplificationFlag.GeometrySimplification
QgsVectorSimplifyMethod.GeometrySimplification.is_monkey_patched = True
QgsVectorSimplifyMethod.GeometrySimplification.__doc__ = "The geometries can be simplified using the current map2pixel context state"
QgsVectorSimplifyMethod.AntialiasingSimplification = Qgis.VectorRenderingSimplificationFlag.AntialiasingSimplification
QgsVectorSimplifyMethod.AntialiasingSimplification.is_monkey_patched = True
QgsVectorSimplifyMethod.AntialiasingSimplification.__doc__ = "The geometries can be rendered with 'AntiAliasing' disabled because of it is '1-pixel size'"
QgsVectorSimplifyMethod.FullSimplification = Qgis.VectorRenderingSimplificationFlag.FullSimplification
QgsVectorSimplifyMethod.FullSimplification.is_monkey_patched = True
QgsVectorSimplifyMethod.FullSimplification.__doc__ = "All simplification hints can be applied ( Geometry + AA-disabling )"
Qgis.VectorRenderingSimplificationFlag.__doc__ = """Simplification flags for vector feature rendering.

.. note::

   Prior to QGIS 3.38 this was available as :py:class:`QgsVectorSimplifyMethod`.SimplifyHint

.. versionadded:: 3.38

* ``NoSimplification``: No simplification can be applied
* ``GeometrySimplification``: The geometries can be simplified using the current map2pixel context state
* ``AntialiasingSimplification``: The geometries can be rendered with 'AntiAliasing' disabled because of it is '1-pixel size'
* ``FullSimplification``: All simplification hints can be applied ( Geometry + AA-disabling )

"""
# --
Qgis.VectorRenderingSimplificationFlag.baseClass = Qgis
QgsVectorSimplifyMethod.SimplifyHints = Qgis.VectorRenderingSimplificationFlags
Qgis.VectorRenderingSimplificationFlags.baseClass = Qgis
VectorRenderingSimplificationFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.RenderSubcomponentProperty.Generic.__doc__ = "Generic subcomponent property"
Qgis.RenderSubcomponentProperty.ShadowOffset.__doc__ = "Shadow offset"
Qgis.RenderSubcomponentProperty.BlurSize.__doc__ = "Blur size"
Qgis.RenderSubcomponentProperty.GlowSpread.__doc__ = "Glow spread size"
Qgis.RenderSubcomponentProperty.__doc__ = """Rendering subcomponent properties.

.. versionadded:: 3.22

* ``Generic``: Generic subcomponent property
* ``ShadowOffset``: Shadow offset
* ``BlurSize``: Blur size
* ``GlowSpread``: Glow spread size

"""
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
Qgis.VertexType.__doc__ = """Types of vertex.

.. versionadded:: 3.22

* ``Segment``: The actual start or end point of a segment

  Available as ``QgsVertexId.SegmentVertex`` in older QGIS releases.

* ``Curve``: An intermediate point on a segment defining the curvature of the segment

  Available as ``QgsVertexId.CurveVertex`` in older QGIS releases.


"""
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
QgsSimpleMarkerSymbolLayerBase.Octagon.__doc__ = "Octagon \n.. versionadded:: 3.18"
QgsSimpleMarkerSymbolLayerBase.SquareWithCorners = Qgis.MarkerShape.SquareWithCorners
QgsSimpleMarkerSymbolLayerBase.SquareWithCorners.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.SquareWithCorners.__doc__ = "A square with diagonal corners \n.. versionadded:: 3.18"
QgsSimpleMarkerSymbolLayerBase.AsteriskFill = Qgis.MarkerShape.AsteriskFill
QgsSimpleMarkerSymbolLayerBase.AsteriskFill.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.AsteriskFill.__doc__ = "A filled asterisk shape \n.. versionadded:: 3.18"
QgsSimpleMarkerSymbolLayerBase.HalfArc = Qgis.MarkerShape.HalfArc
QgsSimpleMarkerSymbolLayerBase.HalfArc.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.HalfArc.__doc__ = "A line-only half arc \n.. versionadded:: 3.20"
QgsSimpleMarkerSymbolLayerBase.ThirdArc = Qgis.MarkerShape.ThirdArc
QgsSimpleMarkerSymbolLayerBase.ThirdArc.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ThirdArc.__doc__ = "A line-only one third arc \n.. versionadded:: 3.20"
QgsSimpleMarkerSymbolLayerBase.QuarterArc = Qgis.MarkerShape.QuarterArc
QgsSimpleMarkerSymbolLayerBase.QuarterArc.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.QuarterArc.__doc__ = "A line-only one quarter arc \n.. versionadded:: 3.20"
QgsSimpleMarkerSymbolLayerBase.ParallelogramRight = Qgis.MarkerShape.ParallelogramRight
QgsSimpleMarkerSymbolLayerBase.ParallelogramRight.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ParallelogramRight.__doc__ = "Parallelogram that slants right \n.. versionadded:: 3.28"
QgsSimpleMarkerSymbolLayerBase.ParallelogramLeft = Qgis.MarkerShape.ParallelogramLeft
QgsSimpleMarkerSymbolLayerBase.ParallelogramLeft.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.ParallelogramLeft.__doc__ = "Parallelogram that slants left \n.. versionadded:: 3.28"
QgsSimpleMarkerSymbolLayerBase.Trapezoid = Qgis.MarkerShape.Trapezoid
QgsSimpleMarkerSymbolLayerBase.Trapezoid.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Trapezoid.__doc__ = "Trapezoid \n.. versionadded:: 3.28"
QgsSimpleMarkerSymbolLayerBase.Shield = Qgis.MarkerShape.Shield
QgsSimpleMarkerSymbolLayerBase.Shield.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Shield.__doc__ = "A shape consisting of a triangle attached to a rectangle \n.. versionadded:: 3.28"
QgsSimpleMarkerSymbolLayerBase.DiamondStar = Qgis.MarkerShape.DiamondStar
QgsSimpleMarkerSymbolLayerBase.DiamondStar.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.DiamondStar.__doc__ = "A 4-sided star \n.. versionadded:: 3.28"
QgsSimpleMarkerSymbolLayerBase.Heart = Qgis.MarkerShape.Heart
QgsSimpleMarkerSymbolLayerBase.Heart.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Heart.__doc__ = "Heart \n.. versionadded:: 3.28"
QgsSimpleMarkerSymbolLayerBase.Decagon = Qgis.MarkerShape.Decagon
QgsSimpleMarkerSymbolLayerBase.Decagon.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.Decagon.__doc__ = "Decagon \n.. versionadded:: 3.28"
QgsSimpleMarkerSymbolLayerBase.RoundedSquare = Qgis.MarkerShape.RoundedSquare
QgsSimpleMarkerSymbolLayerBase.RoundedSquare.is_monkey_patched = True
QgsSimpleMarkerSymbolLayerBase.RoundedSquare.__doc__ = "A square with rounded corners \n.. versionadded:: 3.28"
Qgis.MarkerShape.__doc__ = """Marker shapes.

.. note::

   Prior to QGIS 3.24 this was available as :py:class:`QgsSimpleMarkerSymbolLayerBase`.Shape

.. versionadded:: 3.24

* ``Square``: Square
* ``Diamond``: Diamond
* ``Pentagon``: Pentagon
* ``Hexagon``: Hexagon
* ``Triangle``: Triangle
* ``EquilateralTriangle``: Equilateral triangle
* ``Star``: Star
* ``Arrow``: Arrow
* ``Circle``: Circle
* ``Cross``: Cross (lines only)
* ``CrossFill``: Solid filled cross
* ``Cross2``: Rotated cross (lines only), 'x' shape
* ``Line``: Vertical line
* ``ArrowHead``: Right facing arrow head (unfilled, lines only)
* ``ArrowHeadFilled``: Right facing filled arrow head
* ``SemiCircle``: Semi circle (top half)
* ``ThirdCircle``: One third circle (top left third)
* ``QuarterCircle``: Quarter circle (top left quarter)
* ``QuarterSquare``: Quarter square (top left quarter)
* ``HalfSquare``: Half square (left half)
* ``DiagonalHalfSquare``: Diagonal half square (bottom left half)
* ``RightHalfTriangle``: Right half of triangle
* ``LeftHalfTriangle``: Left half of triangle
* ``Octagon``: Octagon

  .. versionadded:: 3.18

* ``SquareWithCorners``: A square with diagonal corners

  .. versionadded:: 3.18

* ``AsteriskFill``: A filled asterisk shape

  .. versionadded:: 3.18

* ``HalfArc``: A line-only half arc

  .. versionadded:: 3.20

* ``ThirdArc``: A line-only one third arc

  .. versionadded:: 3.20

* ``QuarterArc``: A line-only one quarter arc

  .. versionadded:: 3.20

* ``ParallelogramRight``: Parallelogram that slants right

  .. versionadded:: 3.28

* ``ParallelogramLeft``: Parallelogram that slants left

  .. versionadded:: 3.28

* ``Trapezoid``: Trapezoid

  .. versionadded:: 3.28

* ``Shield``: A shape consisting of a triangle attached to a rectangle

  .. versionadded:: 3.28

* ``DiamondStar``: A 4-sided star

  .. versionadded:: 3.28

* ``Heart``: Heart

  .. versionadded:: 3.28

* ``Decagon``: Decagon

  .. versionadded:: 3.28

* ``RoundedSquare``: A square with rounded corners

  .. versionadded:: 3.28


"""
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
QgsTemplatedLineSymbolLayerBase.InnerVertices.__doc__ = "Inner vertices (i.e. all vertices except the first and last vertex) \n.. versionadded:: 3.24"
Qgis.MarkerLinePlacement.__doc__ = """Defines how/where the symbols should be placed on a line.

.. note::

   Prior to QGIS 3.24 this was available as :py:class:`QgsTemplatedLineSymbolLayerBase`.Placement

.. versionadded:: 3.24

* ``Interval``: Place symbols at regular intervals
* ``Vertex``: Place symbols on every vertex in the line
* ``LastVertex``: Place symbols on the last vertex in the line
* ``FirstVertex``: Place symbols on the first vertex in the line
* ``CentralPoint``: Place symbols at the mid point of the line
* ``CurvePoint``: Place symbols at every virtual curve point in the line (used when rendering curved geometry types only)
* ``SegmentCenter``: Place symbols at the center of every line segment
* ``InnerVertices``: Inner vertices (i.e. all vertices except the first and last vertex)

  .. versionadded:: 3.24


"""
# --
Qgis.MarkerLinePlacement.baseClass = Qgis
Qgis.MarkerLinePlacements.baseClass = Qgis
MarkerLinePlacements = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.LinearReferencingPlacement.IntervalCartesian2D.__doc__ = "Place labels at regular intervals, using Cartesian distance calculations on a 2D plane"
Qgis.LinearReferencingPlacement.IntervalZ.__doc__ = "Place labels at regular intervals, linearly interpolated using Z values"
Qgis.LinearReferencingPlacement.IntervalM.__doc__ = "Place labels at regular intervals, linearly interpolated using M values"
Qgis.LinearReferencingPlacement.Vertex.__doc__ = "Place labels on every vertex in the line"
Qgis.LinearReferencingPlacement.__doc__ = """Defines how/where the labels should be placed in a linear referencing symbol layer.

.. versionadded:: 3.40

* ``IntervalCartesian2D``: Place labels at regular intervals, using Cartesian distance calculations on a 2D plane
* ``IntervalZ``: Place labels at regular intervals, linearly interpolated using Z values
* ``IntervalM``: Place labels at regular intervals, linearly interpolated using M values
* ``Vertex``: Place labels on every vertex in the line

"""
# --
Qgis.LinearReferencingPlacement.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LinearReferencingLabelSource.CartesianDistance2D.__doc__ = "Distance along line, calculated using Cartesian calculations on a 2D plane."
Qgis.LinearReferencingLabelSource.Z.__doc__ = "Z values"
Qgis.LinearReferencingLabelSource.M.__doc__ = "M values"
Qgis.LinearReferencingLabelSource.__doc__ = """Defines what quantity to use for the labels shown in a linear referencing symbol layer.

.. versionadded:: 3.40

* ``CartesianDistance2D``: Distance along line, calculated using Cartesian calculations on a 2D plane.
* ``Z``: Z values
* ``M``: M values

"""
# --
Qgis.LinearReferencingLabelSource.baseClass = Qgis
QgsGradientFillSymbolLayer.GradientColorType = Qgis.GradientColorSource
# monkey patching scoped based enum
QgsGradientFillSymbolLayer.SimpleTwoColor = Qgis.GradientColorSource.SimpleTwoColor
QgsGradientFillSymbolLayer.SimpleTwoColor.is_monkey_patched = True
QgsGradientFillSymbolLayer.SimpleTwoColor.__doc__ = "Simple two color gradient"
QgsGradientFillSymbolLayer.ColorRamp = Qgis.GradientColorSource.ColorRamp
QgsGradientFillSymbolLayer.ColorRamp.is_monkey_patched = True
QgsGradientFillSymbolLayer.ColorRamp.__doc__ = "Gradient color ramp"
Qgis.GradientColorSource.__doc__ = """Gradient color sources.

.. note::

   Prior to QGIS 3.24 this was available as :py:class:`QgsGradientFillSymbolLayer`.GradientColorType

.. versionadded:: 3.24

* ``SimpleTwoColor``: Simple two color gradient
* ``ColorRamp``: Gradient color ramp

"""
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
Qgis.GradientType.__doc__ = """Gradient types.

.. note::

   Prior to QGIS 3.24 this was available as :py:class:`QgsGradientFillSymbolLayer`.GradientType

.. versionadded:: 3.24

* ``Linear``: Linear gradient
* ``Radial``: Radial (circular) gradient
* ``Conical``: Conical (polar) gradient

"""
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
Qgis.SymbolCoordinateReference.__doc__ = """Symbol coordinate reference modes.

.. note::

   Prior to QGIS 3.24 this was available as :py:class:`QgsGradientFillSymbolLayer`.GradientCoordinateMode

.. versionadded:: 3.24

* ``Feature``: Relative to feature/shape being rendered
* ``Viewport``: Relative to the whole viewport/output device

"""
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
Qgis.GradientSpread.__doc__ = """Gradient spread options, which control how gradients are rendered outside of their
start and end points.

.. note::

   Prior to QGIS 3.24 this was available as :py:class:`QgsGradientFillSymbolLayer`.GradientSpread

.. versionadded:: 3.24

* ``Pad``: Pad out gradient using colors at endpoint of gradient
* ``Reflect``: Reflect gradient
* ``Repeat``: Repeat gradient

"""
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
Qgis.PointCountMethod.__doc__ = """Methods which define the number of points randomly filling a polygon.

.. note::

   Prior to QGIS 3.24 this was available as :py:class:`QgsRandomMarkerFillSymbolLayer`.CountMethod

.. versionadded:: 3.24

* ``Absolute``: The point count is used as an absolute count of markers

  Available as ``QgsRandomMarkerFillSymbolLayer.AbsoluteCount`` in older QGIS releases.

* ``DensityBased``: The point count is part of a marker density count

  Available as ``QgsRandomMarkerFillSymbolLayer.DensityBasedCount`` in older QGIS releases.


"""
# --
Qgis.PointCountMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MarkerClipMode.NoClipping.__doc__ = "No clipping, render complete markers"
Qgis.MarkerClipMode.Shape.__doc__ = "Clip to polygon shape"
Qgis.MarkerClipMode.CentroidWithin.__doc__ = "Render complete markers wherever their centroid falls within the polygon shape"
Qgis.MarkerClipMode.CompletelyWithin.__doc__ = "Render complete markers wherever the completely fall within the polygon shape"
Qgis.MarkerClipMode.__doc__ = """Marker clipping modes.

.. versionadded:: 3.24

* ``NoClipping``: No clipping, render complete markers
* ``Shape``: Clip to polygon shape
* ``CentroidWithin``: Render complete markers wherever their centroid falls within the polygon shape
* ``CompletelyWithin``: Render complete markers wherever the completely fall within the polygon shape

"""
# --
Qgis.MarkerClipMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LineClipMode.ClipPainterOnly.__doc__ = "Applying clipping on the painter only (i.e. line endpoints will coincide with polygon bounding box, but will not be part of the visible portion of the line)"
Qgis.LineClipMode.ClipToIntersection.__doc__ = "Clip lines to intersection with polygon shape (slower) (i.e. line endpoints will coincide with polygon exterior)"
Qgis.LineClipMode.NoClipping.__doc__ = "Lines are not clipped, will extend to shape's bounding box."
Qgis.LineClipMode.__doc__ = """Line clipping modes.

.. versionadded:: 3.24

* ``ClipPainterOnly``: Applying clipping on the painter only (i.e. line endpoints will coincide with polygon bounding box, but will not be part of the visible portion of the line)
* ``ClipToIntersection``: Clip lines to intersection with polygon shape (slower) (i.e. line endpoints will coincide with polygon exterior)
* ``NoClipping``: Lines are not clipped, will extend to shape's bounding box.

"""
# --
Qgis.LineClipMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DashPatternLineEndingRule.NoRule.__doc__ = "No special rule"
Qgis.DashPatternLineEndingRule.FullDash.__doc__ = "Start or finish the pattern with a full dash"
Qgis.DashPatternLineEndingRule.HalfDash.__doc__ = "Start or finish the pattern with a half length dash"
Qgis.DashPatternLineEndingRule.FullGap.__doc__ = "Start or finish the pattern with a full gap"
Qgis.DashPatternLineEndingRule.HalfGap.__doc__ = "Start or finish the pattern with a half length gap"
Qgis.DashPatternLineEndingRule.__doc__ = """Dash pattern line ending rules.

.. versionadded:: 3.24

* ``NoRule``: No special rule
* ``FullDash``: Start or finish the pattern with a full dash
* ``HalfDash``: Start or finish the pattern with a half length dash
* ``FullGap``: Start or finish the pattern with a full gap
* ``HalfGap``: Start or finish the pattern with a half length gap

"""
# --
Qgis.DashPatternLineEndingRule.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DashPatternSizeAdjustment.ScaleBothDashAndGap.__doc__ = "Both the dash and gap lengths are adjusted equally"
Qgis.DashPatternSizeAdjustment.ScaleDashOnly.__doc__ = "Only dash lengths are adjusted"
Qgis.DashPatternSizeAdjustment.ScaleGapOnly.__doc__ = "Only gap lengths are adjusted"
Qgis.DashPatternSizeAdjustment.__doc__ = """Dash pattern size adjustment options.

.. versionadded:: 3.24

* ``ScaleBothDashAndGap``: Both the dash and gap lengths are adjusted equally
* ``ScaleDashOnly``: Only dash lengths are adjusted
* ``ScaleGapOnly``: Only gap lengths are adjusted

"""
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
Qgis.GraduatedMethod.__doc__ = """Methods for modifying symbols by range in a graduated symbol renderer.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsGraduatedSymbolRenderer`.GraduatedMethod

.. versionadded:: 3.26

* ``Color``: Alter color of symbols

  Available as ``QgsGraduatedSymbolRenderer.GraduatedColor`` in older QGIS releases.

* ``Size``: Alter size of symbols

  Available as ``QgsGraduatedSymbolRenderer.GraduatedSize`` in older QGIS releases.


"""
# --
Qgis.GraduatedMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.PlotAxisSuffixPlacement.NoLabels.__doc__ = "Do not place suffixes"
Qgis.PlotAxisSuffixPlacement.EveryLabel.__doc__ = "Place suffix after every value label"
Qgis.PlotAxisSuffixPlacement.FirstLabel.__doc__ = "Place suffix after the first label value only"
Qgis.PlotAxisSuffixPlacement.LastLabel.__doc__ = "Place suffix after the last label value only"
Qgis.PlotAxisSuffixPlacement.FirstAndLastLabels.__doc__ = "Place suffix after the first and last label values only"
Qgis.PlotAxisSuffixPlacement.__doc__ = """Placement options for suffixes in the labels for axis of plots.

.. versionadded:: 3.32

* ``NoLabels``: Do not place suffixes
* ``EveryLabel``: Place suffix after every value label
* ``FirstLabel``: Place suffix after the first label value only
* ``LastLabel``: Place suffix after the last label value only
* ``FirstAndLastLabels``: Place suffix after the first and last label values only

"""
# --
Qgis.PlotAxisSuffixPlacement.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DpiMode.All.__doc__ = "All"
Qgis.DpiMode.Off.__doc__ = "Off"
Qgis.DpiMode.QGIS.__doc__ = "QGIS"
Qgis.DpiMode.UMN.__doc__ = "UMN"
Qgis.DpiMode.GeoServer.__doc__ = "GeoServer"
Qgis.DpiMode.__doc__ = """DpiMode enum

.. versionadded:: 3.26

* ``All``: All
* ``Off``: Off
* ``QGIS``: QGIS
* ``UMN``: UMN
* ``GeoServer``: GeoServer

"""
# --
Qgis.DpiMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TilePixelRatio.Undefined.__doc__ = "Undefined (not scale)"
Qgis.TilePixelRatio.StandardDpi.__doc__ = "Standard (96 DPI)"
Qgis.TilePixelRatio.HighDpi.__doc__ = "High (192 DPI)"
Qgis.TilePixelRatio.__doc__ = """DpiMode enum

.. versionadded:: 3.30

* ``Undefined``: Undefined (not scale)
* ``StandardDpi``: Standard (96 DPI)
* ``HighDpi``: High (192 DPI)

"""
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
QgsStringUtils.SmallCaps.__doc__ = "Mixed case small caps \n.. versionadded:: 3.24"
QgsStringUtils.TitleCase = Qgis.Capitalization.TitleCase
QgsStringUtils.TitleCase.is_monkey_patched = True
QgsStringUtils.TitleCase.__doc__ = "Simple title case conversion - does not fully grammatically parse the text and uses simple rules only. Note that this method does not convert any characters to lowercase, it only uppercases required letters. Callers must ensure that input strings are already lowercased."
QgsStringUtils.UpperCamelCase = Qgis.Capitalization.UpperCamelCase
QgsStringUtils.UpperCamelCase.is_monkey_patched = True
QgsStringUtils.UpperCamelCase.__doc__ = "Convert the string to upper camel case. Note that this method does not unaccent characters."
QgsStringUtils.AllSmallCaps = Qgis.Capitalization.AllSmallCaps
QgsStringUtils.AllSmallCaps.is_monkey_patched = True
QgsStringUtils.AllSmallCaps.__doc__ = "Force all characters to small caps \n.. versionadded:: 3.24"
Qgis.Capitalization.__doc__ = """String capitalization options.

.. note::

   Prior to QGIS 3.24 this was available as :py:class:`QgsStringUtils`.Capitalization

.. versionadded:: 3.24

* ``MixedCase``: Mixed case, ie no change
* ``AllUppercase``: Convert all characters to uppercase
* ``AllLowercase``: Convert all characters to lowercase
* ``ForceFirstLetterToCapital``: Convert just the first letter of each word to uppercase, leave the rest untouched
* ``SmallCaps``: Mixed case small caps

  .. versionadded:: 3.24

* ``TitleCase``: Simple title case conversion - does not fully grammatically parse the text and uses simple rules only. Note that this method does not convert any characters to lowercase, it only uppercases required letters. Callers must ensure that input strings are already lowercased.
* ``UpperCamelCase``: Convert the string to upper camel case. Note that this method does not unaccent characters.
* ``AllSmallCaps``: Force all characters to small caps

  .. versionadded:: 3.24


"""
# --
Qgis.Capitalization.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TextRendererFlag.WrapLines.__doc__ = "Automatically wrap long lines of text"
Qgis.TextRendererFlag.__doc__ = """Flags which control the behavior of rendering text.

.. versionadded:: 3.24

* ``WrapLines``: Automatically wrap long lines of text

"""
# --
Qgis.TextRendererFlag.baseClass = Qgis
Qgis.TextRendererFlags.baseClass = Qgis
TextRendererFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ScaleToTileZoomLevelMethod.MapBox.__doc__ = "Uses a scale doubling approach to account for hi-DPI tiles, and rounds to the nearest tile level for the map scale"
Qgis.ScaleToTileZoomLevelMethod.Esri.__doc__ = "No scale doubling, always rounds down when matching to available tile levels"
Qgis.ScaleToTileZoomLevelMethod.__doc__ = """Available methods for converting map scales to tile zoom levels.

.. versionadded:: 3.26

* ``MapBox``: Uses a scale doubling approach to account for hi-DPI tiles, and rounds to the nearest tile level for the map scale
* ``Esri``: No scale doubling, always rounds down when matching to available tile levels

"""
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
Qgis.AngularDirection.__doc__ = """Angular directions.

.. versionadded:: 3.24

* ``Clockwise``: Clockwise direction
* ``CounterClockwise``: Counter-clockwise direction
* ``NoOrientation``: Unknown orientation or sentinel value

"""
# --
Qgis.AngularDirection.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RendererUsage.View.__doc__ = "Renderer used for displaying on screen"
Qgis.RendererUsage.Export.__doc__ = "Renderer used for printing or exporting to a file"
Qgis.RendererUsage.Unknown.__doc__ = "Renderer used for unknown usage"
Qgis.RendererUsage.__doc__ = """Usage of the renderer.

.. versionadded:: 3.24

* ``View``: Renderer used for displaying on screen
* ``Export``: Renderer used for printing or exporting to a file
* ``Unknown``: Renderer used for unknown usage

"""
# --
Qgis.RendererUsage.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MapCanvasFlag.ShowMainAnnotationLayer.__doc__ = "The project's main annotation layer should be rendered in the canvas"
Qgis.MapCanvasFlag.__doc__ = """Flags controlling behavior of map canvases.

.. versionadded:: 3.40

* ``ShowMainAnnotationLayer``: The project's main annotation layer should be rendered in the canvas

"""
# --
Qgis.MapCanvasFlag.baseClass = Qgis
Qgis.MapCanvasFlags.baseClass = Qgis
MapCanvasFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ViewSyncModeFlag.Sync3DTo2D.__doc__ = "Synchronize 3D view camera to the main map canvas extent"
Qgis.ViewSyncModeFlag.Sync2DTo3D.__doc__ = "Update the 2D main canvas extent to include the viewed area from the 3D view"
Qgis.ViewSyncModeFlag.__doc__ = """Synchronization of 2D map canvas and 3D view

.. versionadded:: 3.26

* ``Sync3DTo2D``: Synchronize 3D view camera to the main map canvas extent
* ``Sync2DTo3D``: Update the 2D main canvas extent to include the viewed area from the 3D view

"""
# --
Qgis.ViewSyncModeFlag.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MapRecenteringMode.Always.__doc__ = "Always recenter map"
Qgis.MapRecenteringMode.WhenOutsideVisibleExtent.__doc__ = "Only recenter map when new center would be outside of current visible extent"
Qgis.MapRecenteringMode.Never.__doc__ = "Never recenter map"
Qgis.MapRecenteringMode.__doc__ = """Modes for recentering map canvases.

.. versionadded:: 3.30

* ``Always``: Always recenter map
* ``WhenOutsideVisibleExtent``: Only recenter map when new center would be outside of current visible extent
* ``Never``: Never recenter map

"""
# --
Qgis.MapRecenteringMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.HistoryProviderBackend.LocalProfile.__doc__ = "Local profile"
Qgis.HistoryProviderBackend.__doc__ = """History provider backends.

.. versionadded:: 3.24

* ``LocalProfile``: Local profile

"""
# --
Qgis.HistoryProviderBackend.baseClass = Qgis
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
Qgis.ProcessingSourceType.__doc__ = """Processing data source types.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessing`.SourceType

.. versionadded:: 3.36

* ``MapLayer``: Any map layer type (raster, vector, mesh, point cloud, annotation or plugin layer)

  Available as ``QgsProcessing.TypeMapLayer`` in older QGIS releases.

* ``VectorAnyGeometry``: Any vector layer with geometry

  Available as ``QgsProcessing.TypeVectorAnyGeometry`` in older QGIS releases.

* ``VectorPoint``: Vector point layers

  Available as ``QgsProcessing.TypeVectorPoint`` in older QGIS releases.

* ``VectorLine``: Vector line layers

  Available as ``QgsProcessing.TypeVectorLine`` in older QGIS releases.

* ``VectorPolygon``: Vector polygon layers

  Available as ``QgsProcessing.TypeVectorPolygon`` in older QGIS releases.

* ``Raster``: Raster layers

  Available as ``QgsProcessing.TypeRaster`` in older QGIS releases.

* ``File``: Files (i.e. non map layer sources, such as text files)

  Available as ``QgsProcessing.TypeFile`` in older QGIS releases.

* ``Vector``: Tables (i.e. vector layers with or without geometry). When used for a sink this indicates the sink has no geometry.

  Available as ``QgsProcessing.TypeVector`` in older QGIS releases.

* ``Mesh``: Mesh layers

  .. versionadded:: 3.6


  Available as ``QgsProcessing.TypeMesh`` in older QGIS releases.

* ``Plugin``: Plugin layers

  .. versionadded:: 3.22


  Available as ``QgsProcessing.TypePlugin`` in older QGIS releases.

* ``PointCloud``: Point cloud layers

  .. versionadded:: 3.22


  Available as ``QgsProcessing.TypePointCloud`` in older QGIS releases.

* ``Annotation``: Annotation layers

  .. versionadded:: 3.22


  Available as ``QgsProcessing.TypeAnnotation`` in older QGIS releases.

* ``VectorTile``: Vector tile layers

  .. versionadded:: 3.32


  Available as ``QgsProcessing.TypeVectorTile`` in older QGIS releases.


"""
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
QgsProcessingProvider.FlagCompatibleWithVirtualRaster.__doc__ = "The processing provider's algorithms can work with QGIS virtualraster data provider \n.. versionadded:: 3.36"
Qgis.ProcessingProviderFlag.__doc__ = """Flags indicating how and when an processing provider operates and should be exposed to users.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingProvider`.Flag

.. versionadded:: 3.36

* ``DeemphasiseSearchResults``: Algorithms should be de-emphasised in the search results when searching for algorithms. Use for low-priority providers or those with substantial known issues.

  Available as ``QgsProcessingProvider.FlagDeemphasiseSearchResults`` in older QGIS releases.

* ``CompatibleWithVirtualRaster``: The processing provider's algorithms can work with QGIS virtualraster data provider

  .. versionadded:: 3.36


  Available as ``QgsProcessingProvider.FlagCompatibleWithVirtualRaster`` in older QGIS releases.


"""
# --
Qgis.ProcessingProviderFlag.baseClass = Qgis
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
QgsProcessingAlgorithm.SecurityRisk = Qgis.ProcessingAlgorithmFlag.SecurityRisk
QgsProcessingAlgorithm.SecurityRisk.is_monkey_patched = True
QgsProcessingAlgorithm.SecurityRisk.__doc__ = "The algorithm represents a potential security risk if executed with untrusted inputs. \n.. versionadded:: 3.40"
QgsProcessingAlgorithm.FlagDeprecated = Qgis.ProcessingAlgorithmFlag.Deprecated
QgsProcessingAlgorithm.Flag.FlagDeprecated = Qgis.ProcessingAlgorithmFlag.Deprecated
QgsProcessingAlgorithm.FlagDeprecated.is_monkey_patched = True
QgsProcessingAlgorithm.FlagDeprecated.__doc__ = "Algorithm is deprecated"
Qgis.ProcessingAlgorithmFlag.__doc__ = """Flags indicating how and when an algorithm operates and should be exposed to users.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingAlgorithm`.Flag

.. versionadded:: 3.36

* ``HideFromToolbox``: Algorithm should be hidden from the toolbox

  Available as ``QgsProcessingAlgorithm.FlagHideFromToolbox`` in older QGIS releases.

* ``HideFromModeler``: Algorithm should be hidden from the modeler

  Available as ``QgsProcessingAlgorithm.FlagHideFromModeler`` in older QGIS releases.

* ``SupportsBatch``: Algorithm supports batch mode

  Available as ``QgsProcessingAlgorithm.FlagSupportsBatch`` in older QGIS releases.

* ``CanCancel``: Algorithm can be canceled

  Available as ``QgsProcessingAlgorithm.FlagCanCancel`` in older QGIS releases.

* ``RequiresMatchingCrs``: Algorithm requires that all input layers have matching coordinate reference systems

  Available as ``QgsProcessingAlgorithm.FlagRequiresMatchingCrs`` in older QGIS releases.

* ``NoThreading``: Algorithm is not thread safe and cannot be run in a background thread, e.g. for algorithms which manipulate the current project, layer selections, or with external dependencies which are not thread-safe.

  Available as ``QgsProcessingAlgorithm.FlagNoThreading`` in older QGIS releases.

* ``DisplayNameIsLiteral``: Algorithm's display name is a static literal string, and should not be translated or automatically formatted. For use with algorithms named after commands, e.g. GRASS 'v.in.ogr'.

  Available as ``QgsProcessingAlgorithm.FlagDisplayNameIsLiteral`` in older QGIS releases.

* ``SupportsInPlaceEdits``: Algorithm supports in-place editing

  Available as ``QgsProcessingAlgorithm.FlagSupportsInPlaceEdits`` in older QGIS releases.

* ``KnownIssues``: Algorithm has known issues

  Available as ``QgsProcessingAlgorithm.FlagKnownIssues`` in older QGIS releases.

* ``CustomException``: Algorithm raises custom exception notices, don't use the standard ones

  Available as ``QgsProcessingAlgorithm.FlagCustomException`` in older QGIS releases.

* ``PruneModelBranchesBasedOnAlgorithmResults``: Algorithm results will cause remaining model branches to be pruned based on the results of running the algorithm

  Available as ``QgsProcessingAlgorithm.FlagPruneModelBranchesBasedOnAlgorithmResults`` in older QGIS releases.

* ``SkipGenericModelLogging``: When running as part of a model, the generic algorithm setup and results logging should be skipped

  Available as ``QgsProcessingAlgorithm.FlagSkipGenericModelLogging`` in older QGIS releases.

* ``NotAvailableInStandaloneTool``: Algorithm should not be available from the standalone \"qgis_process\" tool. Used to flag algorithms which make no sense outside of the QGIS application, such as \"select by...\" style algorithms.

  Available as ``QgsProcessingAlgorithm.FlagNotAvailableInStandaloneTool`` in older QGIS releases.

* ``RequiresProject``: The algorithm requires that a valid QgsProject is available from the processing context in order to execute

  Available as ``QgsProcessingAlgorithm.FlagRequiresProject`` in older QGIS releases.

* ``SecurityRisk``: The algorithm represents a potential security risk if executed with untrusted inputs.

  .. versionadded:: 3.40

* ``Deprecated``: Algorithm is deprecated

  Available as ``QgsProcessingAlgorithm.FlagDeprecated`` in older QGIS releases.


"""
# --
Qgis.ProcessingAlgorithmFlag.baseClass = Qgis
QgsProcessingAlgorithm.Flags = Qgis.ProcessingAlgorithmFlags
Qgis.ProcessingAlgorithmFlags.baseClass = Qgis
ProcessingAlgorithmFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ProcessingAlgorithmDocumentationFlag.RegeneratesPrimaryKey.__doc__ = "Algorithm always drops any existing primary keys or FID values and regenerates them in outputs"
Qgis.ProcessingAlgorithmDocumentationFlag.RegeneratesPrimaryKeyInSomeScenarios.__doc__ = "Algorithm may drop the existing primary keys or FID values in some scenarios, depending on algorithm inputs and parameters"
Qgis.ProcessingAlgorithmDocumentationFlag.__doc__ = """Flags describing algorithm behavior for documentation purposes.

.. versionadded:: 3.40

* ``RegeneratesPrimaryKey``: Algorithm always drops any existing primary keys or FID values and regenerates them in outputs
* ``RegeneratesPrimaryKeyInSomeScenarios``: Algorithm may drop the existing primary keys or FID values in some scenarios, depending on algorithm inputs and parameters

"""
# --
Qgis.ProcessingAlgorithmDocumentationFlag.baseClass = Qgis
Qgis.ProcessingAlgorithmDocumentationFlags.baseClass = Qgis
ProcessingAlgorithmDocumentationFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingAlgorithm.PropertyAvailability = Qgis.ProcessingPropertyAvailability
# monkey patching scoped based enum
QgsProcessingAlgorithm.NotAvailable = Qgis.ProcessingPropertyAvailability.NotAvailable
QgsProcessingAlgorithm.NotAvailable.is_monkey_patched = True
QgsProcessingAlgorithm.NotAvailable.__doc__ = "Properties are not available"
QgsProcessingAlgorithm.Available = Qgis.ProcessingPropertyAvailability.Available
QgsProcessingAlgorithm.Available.is_monkey_patched = True
QgsProcessingAlgorithm.Available.__doc__ = "Properties are available"
Qgis.ProcessingPropertyAvailability.__doc__ = """Property availability, used for :py:class:`QgsProcessingAlgorithm`.VectorProperties
in order to determine if properties are available or not.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingAlgorithm`.PropertyAvailability

.. versionadded:: 3.36

* ``NotAvailable``: Properties are not available
* ``Available``: Properties are available

"""
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
QgsProcessingContext.ModelDebug.__doc__ = "Model debug level logging. Includes verbose logging and other outputs useful for debugging models \n.. versionadded:: 3.34"
Qgis.ProcessingLogLevel.__doc__ = """Logging level for algorithms to use when pushing feedback messages.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingContext`.LogLevel

.. versionadded:: 3.36

* ``DefaultLevel``: Default logging level
* ``Verbose``: Verbose logging
* ``ModelDebug``: Model debug level logging. Includes verbose logging and other outputs useful for debugging models

  .. versionadded:: 3.34


"""
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
Qgis.ProcessingFeatureSourceDefinitionFlag.__doc__ = """Flags which control behavior for a Processing feature source.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingFeatureSourceDefinition`.Flag

.. versionadded:: 3.36

* ``OverrideDefaultGeometryCheck``: If set, the default geometry check method (as dictated by QgsProcessingContext) will be overridden for this source

  Available as ``QgsProcessingFeatureSourceDefinition.FlagOverrideDefaultGeometryCheck`` in older QGIS releases.

* ``CreateIndividualOutputPerInputFeature``: If set, every feature processed from this source will be placed into its own individually created output destination. Support for this flag depends on how an algorithm is executed.

  Available as ``QgsProcessingFeatureSourceDefinition.FlagCreateIndividualOutputPerInputFeature`` in older QGIS releases.


"""
# --
Qgis.ProcessingFeatureSourceDefinitionFlag.baseClass = Qgis
QgsProcessingFeatureSourceDefinition.Flags = Qgis.ProcessingFeatureSourceDefinitionFlags
Qgis.ProcessingFeatureSourceDefinitionFlags.baseClass = Qgis
ProcessingFeatureSourceDefinitionFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingFeatureSource.Flag = Qgis.ProcessingFeatureSourceFlag
# monkey patching scoped based enum
QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks = Qgis.ProcessingFeatureSourceFlag.SkipGeometryValidityChecks
QgsProcessingFeatureSource.Flag.FlagSkipGeometryValidityChecks = Qgis.ProcessingFeatureSourceFlag.SkipGeometryValidityChecks
QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks.is_monkey_patched = True
QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks.__doc__ = "Invalid geometry checks should always be skipped. This flag can be useful for algorithms which always require invalid geometries, regardless of any user settings (e.g. \"repair geometry\" type algorithms)."
Qgis.ProcessingFeatureSourceFlag.__doc__ = """Flags which control how :py:class:`QgsProcessingFeatureSource` fetches features.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingFeatureSource`.Flag

.. versionadded:: 3.36

* ``SkipGeometryValidityChecks``: Invalid geometry checks should always be skipped. This flag can be useful for algorithms which always require invalid geometries, regardless of any user settings (e.g. \"repair geometry\" type algorithms).

  Available as ``QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks`` in older QGIS releases.


"""
# --
Qgis.ProcessingFeatureSourceFlag.baseClass = Qgis
QgsProcessingFeatureSource.Flags = Qgis.ProcessingFeatureSourceFlags
Qgis.ProcessingFeatureSourceFlags.baseClass = Qgis
ProcessingFeatureSourceFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsProcessingParameterType.ParameterFlag = Qgis.ProcessingParameterTypeFlag
# monkey patching scoped based enum
QgsProcessingParameterType.ExposeToModeler = Qgis.ProcessingParameterTypeFlag.ExposeToModeler
QgsProcessingParameterType.ExposeToModeler.is_monkey_patched = True
QgsProcessingParameterType.ExposeToModeler.__doc__ = "Is this parameter available in the modeler. Is set to on by default."
Qgis.ProcessingParameterTypeFlag.__doc__ = """Flags which dictate the behavior of Processing parameter types.

Each parameter type can offer a number of additional flags to fine tune its behavior
and capabilities.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterType`.ParameterFlag

.. versionadded:: 3.36

* ``ExposeToModeler``: Is this parameter available in the modeler. Is set to on by default.

"""
# --
Qgis.ProcessingParameterTypeFlag.baseClass = Qgis
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
Qgis.ProcessingParameterFlag.__doc__ = """Flags which dictate the behavior of Processing parameters.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterDefinition`.Flag

.. versionadded:: 3.36

* ``Advanced``: Parameter is an advanced parameter which should be hidden from users by default

  Available as ``QgsProcessingParameterDefinition.FlagAdvanced`` in older QGIS releases.

* ``Hidden``: Parameter is hidden and should not be shown to users

  Available as ``QgsProcessingParameterDefinition.FlagHidden`` in older QGIS releases.

* ``Optional``: Parameter is optional

  Available as ``QgsProcessingParameterDefinition.FlagOptional`` in older QGIS releases.

* ``IsModelOutput``: Destination parameter is final output. The parameter name will be used.

  Available as ``QgsProcessingParameterDefinition.FlagIsModelOutput`` in older QGIS releases.


"""
# --
Qgis.ProcessingParameterFlag.baseClass = Qgis
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
Qgis.ProcessingFileParameterBehavior.__doc__ = """Flags which dictate the behavior of :py:class:`QgsProcessingParameterFile`.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterFile`.Behavior

.. versionadded:: 3.36

* ``File``: Parameter is a single file
* ``Folder``: Parameter is a folder

"""
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
Qgis.ProcessingNumberParameterType.__doc__ = """Processing numeric parameter data types.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterNumber`.Type

.. versionadded:: 3.36

* ``Integer``: Integer values
* ``Double``: Double/float values

"""
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
Qgis.ProcessingFieldParameterDataType.__doc__ = """Processing field parameter data types.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterField`.DataType

.. versionadded:: 3.36

* ``Any``: Accepts any field
* ``Numeric``: Accepts numeric fields
* ``String``: Accepts string fields
* ``DateTime``: Accepts datetime fields
* ``Binary``: Accepts binary fields, since QGIS 3.34
* ``Boolean``: Accepts boolean fields, since QGIS 3.34

"""
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
Qgis.ProcessingDateTimeParameterDataType.__doc__ = """Processing date time parameter data types.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterDateTime`.Type

.. versionadded:: 3.36

* ``DateTime``: Datetime values
* ``Date``: Date values
* ``Time``: Time values

"""
# --
Qgis.ProcessingDateTimeParameterDataType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProcessingModelChildParameterSource.ModelParameter.__doc__ = "Parameter value is taken from a parent model parameter"
Qgis.ProcessingModelChildParameterSource.ChildOutput.__doc__ = "Parameter value is taken from an output generated by a child algorithm"
Qgis.ProcessingModelChildParameterSource.StaticValue.__doc__ = "Parameter value is a static value"
Qgis.ProcessingModelChildParameterSource.Expression.__doc__ = "Parameter value is taken from an expression, evaluated just before the algorithm runs"
Qgis.ProcessingModelChildParameterSource.ExpressionText.__doc__ = "Parameter value is taken from a text with expressions, evaluated just before the algorithm runs"
Qgis.ProcessingModelChildParameterSource.ModelOutput.__doc__ = "Parameter value is linked to an output parameter for the model"
Qgis.ProcessingModelChildParameterSource.__doc__ = """Processing model child parameter sources.

.. versionadded:: 3.34

* ``ModelParameter``: Parameter value is taken from a parent model parameter
* ``ChildOutput``: Parameter value is taken from an output generated by a child algorithm
* ``StaticValue``: Parameter value is a static value
* ``Expression``: Parameter value is taken from an expression, evaluated just before the algorithm runs
* ``ExpressionText``: Parameter value is taken from a text with expressions, evaluated just before the algorithm runs
* ``ModelOutput``: Parameter value is linked to an output parameter for the model

"""
# --
Qgis.ProcessingModelChildParameterSource.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProcessingModelChildAlgorithmExecutionStatus.NotExecuted.__doc__ = "Child has not been executed"
Qgis.ProcessingModelChildAlgorithmExecutionStatus.Success.__doc__ = "Child was successfully executed"
Qgis.ProcessingModelChildAlgorithmExecutionStatus.Failed.__doc__ = "Child encountered an error while executing"
Qgis.ProcessingModelChildAlgorithmExecutionStatus.__doc__ = """Reflects the status of a child algorithm in a Processing model.

.. versionadded:: 3.38

* ``NotExecuted``: Child has not been executed
* ``Success``: Child was successfully executed
* ``Failed``: Child encountered an error while executing

"""
# --
Qgis.ProcessingModelChildAlgorithmExecutionStatus.baseClass = Qgis
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
Qgis.ProcessingTinInputLayerType.__doc__ = """Defines the type of input layer for a Processing TIN input.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsProcessingParameterTinInputLayers`.Type

.. versionadded:: 3.36

* ``Vertices``: Input that adds only vertices
* ``StructureLines``: Input that adds add structure lines
* ``BreakLines``: Input that adds vertices and break lines

"""
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
Qgis.CrsDefinitionFormat.__doc__ = """CRS definition formats.

.. versionadded:: 3.24

* ``Wkt``: WKT format (always recommended over proj string format)

  Available as ``QgsCoordinateReferenceSystem.FormatWkt`` in older QGIS releases.

* ``Proj``: Proj string format

  Available as ``QgsCoordinateReferenceSystem.FormatProj`` in older QGIS releases.


"""
# --
Qgis.CrsDefinitionFormat.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldDomainSplitPolicy.DefaultValue.__doc__ = "Use default field value"
Qgis.FieldDomainSplitPolicy.Duplicate.__doc__ = "Duplicate original value"
Qgis.FieldDomainSplitPolicy.GeometryRatio.__doc__ = "New values are computed by the ratio of their area/length compared to the area/length of the original feature"
Qgis.FieldDomainSplitPolicy.UnsetField.__doc__ = "Clears the field value so that the data provider backend will populate using any backend triggers or similar logic \n.. versionadded:: 3.30"
Qgis.FieldDomainSplitPolicy.__doc__ = """Split policy for field domains.

When a feature is split into multiple parts, defines how the value of attributes
following the domain are computed.

.. versionadded:: 3.26

* ``DefaultValue``: Use default field value
* ``Duplicate``: Duplicate original value
* ``GeometryRatio``: New values are computed by the ratio of their area/length compared to the area/length of the original feature
* ``UnsetField``: Clears the field value so that the data provider backend will populate using any backend triggers or similar logic

  .. versionadded:: 3.30


"""
# --
Qgis.FieldDomainSplitPolicy.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldDomainMergePolicy.DefaultValue.__doc__ = "Use default field value"
Qgis.FieldDomainMergePolicy.Sum.__doc__ = "Sum of values"
Qgis.FieldDomainMergePolicy.GeometryWeighted.__doc__ = "New values are computed as the weighted average of the source values"
Qgis.FieldDomainMergePolicy.__doc__ = """Merge policy for field domains.

When a feature is built by merging multiple features, defines how the value of
attributes following the domain are computed.

.. versionadded:: 3.26

* ``DefaultValue``: Use default field value
* ``Sum``: Sum of values
* ``GeometryWeighted``: New values are computed as the weighted average of the source values

"""
# --
Qgis.FieldDomainMergePolicy.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldDuplicatePolicy.DefaultValue.__doc__ = "Use default field value"
Qgis.FieldDuplicatePolicy.Duplicate.__doc__ = "Duplicate original value"
Qgis.FieldDuplicatePolicy.UnsetField.__doc__ = "Clears the field value so that the data provider backend will populate using any backend triggers or similar logic \n.. versionadded:: 3.30"
Qgis.FieldDuplicatePolicy.__doc__ = """Duplicate policy for fields.

When a feature is duplicated, defines how the value of attributes are computed.

.. versionadded:: 3.38

* ``DefaultValue``: Use default field value
* ``Duplicate``: Duplicate original value
* ``UnsetField``: Clears the field value so that the data provider backend will populate using any backend triggers or similar logic

  .. versionadded:: 3.30


"""
# --
Qgis.FieldDuplicatePolicy.baseClass = Qgis
# monkey patching scoped based enum
Qgis.FieldDomainType.Coded.__doc__ = "Coded field domain"
Qgis.FieldDomainType.Range.__doc__ = "Numeric range field domain (min/max)"
Qgis.FieldDomainType.Glob.__doc__ = "Glob string pattern field domain"
Qgis.FieldDomainType.__doc__ = """Types of field domain

.. versionadded:: 3.26

* ``Coded``: Coded field domain
* ``Range``: Numeric range field domain (min/max)
* ``Glob``: Glob string pattern field domain

"""
# --
Qgis.FieldDomainType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TransactionMode.Disabled.__doc__ = "Edits are buffered locally and sent to the provider when toggling layer editing mode."
Qgis.TransactionMode.AutomaticGroups.__doc__ = "Automatic transactional editing means that on supported datasources (postgres and geopackage databases) the edit state of all tables that originate from the same database are synchronized and executed in a server side transaction."
Qgis.TransactionMode.BufferedGroups.__doc__ = "Buffered transactional editing means that all editable layers in the buffered transaction group are toggled synchronously and all edits are saved in a local edit buffer. Saving changes is executed within a single transaction on all layers (per provider)."
Qgis.TransactionMode.__doc__ = """Transaction mode.

.. versionadded:: 3.26

* ``Disabled``: Edits are buffered locally and sent to the provider when toggling layer editing mode.
* ``AutomaticGroups``: Automatic transactional editing means that on supported datasources (postgres and geopackage databases) the edit state of all tables that originate from the same database are synchronized and executed in a server side transaction.
* ``BufferedGroups``: Buffered transactional editing means that all editable layers in the buffered transaction group are toggled synchronously and all edits are saved in a local edit buffer. Saving changes is executed within a single transaction on all layers (per provider).

"""
# --
Qgis.TransactionMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AltitudeClamping.Absolute.__doc__ = "Elevation is taken directly from feature and is independent of terrain height (final elevation = feature elevation)"
Qgis.AltitudeClamping.Relative.__doc__ = "Elevation is relative to terrain height (final elevation = terrain elevation + feature elevation)"
Qgis.AltitudeClamping.Terrain.__doc__ = "Elevation is clamped to terrain (final elevation = terrain elevation)"
Qgis.AltitudeClamping.__doc__ = """Altitude clamping.

.. versionadded:: 3.26

* ``Absolute``: Elevation is taken directly from feature and is independent of terrain height (final elevation = feature elevation)
* ``Relative``: Elevation is relative to terrain height (final elevation = terrain elevation + feature elevation)
* ``Terrain``: Elevation is clamped to terrain (final elevation = terrain elevation)

"""
# --
Qgis.AltitudeClamping.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AltitudeBinding.Vertex.__doc__ = "Clamp every vertex of feature"
Qgis.AltitudeBinding.Centroid.__doc__ = "Clamp just centroid of feature"
Qgis.AltitudeBinding.__doc__ = """Altitude binding.

.. versionadded:: 3.26

* ``Vertex``: Clamp every vertex of feature
* ``Centroid``: Clamp just centroid of feature

"""
# --
Qgis.AltitudeBinding.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RangeLimits.IncludeBoth.__doc__ = "Both lower and upper values are included in the range"
Qgis.RangeLimits.IncludeLowerExcludeUpper.__doc__ = "Lower value is included in the range, upper value is excluded"
Qgis.RangeLimits.ExcludeLowerIncludeUpper.__doc__ = "Lower value is excluded from the range, upper value in inccluded"
Qgis.RangeLimits.ExcludeBoth.__doc__ = "Both lower and upper values are excluded from the range"
Qgis.RangeLimits.__doc__ = """Describes how the limits of a range are handled.

.. versionadded:: 3.38

* ``IncludeBoth``: Both lower and upper values are included in the range
* ``IncludeLowerExcludeUpper``: Lower value is included in the range, upper value is excluded
* ``ExcludeLowerIncludeUpper``: Lower value is excluded from the range, upper value in inccluded
* ``ExcludeBoth``: Both lower and upper values are excluded from the range

"""
# --
Qgis.RangeLimits.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RasterElevationMode.FixedElevationRange.__doc__ = "Layer has a fixed elevation range"
Qgis.RasterElevationMode.RepresentsElevationSurface.__doc__ = "Pixel values represent an elevation surface"
Qgis.RasterElevationMode.FixedRangePerBand.__doc__ = "Layer has a fixed (manually specified) elevation range per band"
Qgis.RasterElevationMode.DynamicRangePerBand.__doc__ = "Layer has a elevation range per band, calculated dynamically from an expression"
Qgis.RasterElevationMode.__doc__ = """Raster layer elevation modes.

.. versionadded:: 3.38

* ``FixedElevationRange``: Layer has a fixed elevation range
* ``RepresentsElevationSurface``: Pixel values represent an elevation surface
* ``FixedRangePerBand``: Layer has a fixed (manually specified) elevation range per band
* ``DynamicRangePerBand``: Layer has a elevation range per band, calculated dynamically from an expression

"""
# --
Qgis.RasterElevationMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MeshElevationMode.FixedElevationRange.__doc__ = "Layer has a fixed elevation range"
Qgis.MeshElevationMode.FromVertices.__doc__ = "Elevation should be taken from mesh vertices"
Qgis.MeshElevationMode.FixedRangePerGroup.__doc__ = "Layer has a fixed (manually specified) elevation range per group"
Qgis.MeshElevationMode.__doc__ = """Mesh layer elevation modes.

.. versionadded:: 3.38

* ``FixedElevationRange``: Layer has a fixed elevation range
* ``FromVertices``: Elevation should be taken from mesh vertices
* ``FixedRangePerGroup``: Layer has a fixed (manually specified) elevation range per group

"""
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
Qgis.BetweenLineConstraint.__doc__ = """Between line constraints which can be enabled

.. versionadded:: 3.26

* ``NoConstraint``: No additional constraint
* ``Perpendicular``: Perpendicular
* ``Parallel``: Parallel

"""
# --
Qgis.BetweenLineConstraint.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LineExtensionSide.BeforeVertex.__doc__ = "Lock to previous vertex"
Qgis.LineExtensionSide.AfterVertex.__doc__ = "Lock to next vertex"
Qgis.LineExtensionSide.NoVertex.__doc__ = "Don't lock to vertex"
Qgis.LineExtensionSide.__doc__ = """Designates whether the line extension constraint is currently soft locked
with the previous or next vertex of the locked one.

.. versionadded:: 3.26

* ``BeforeVertex``: Lock to previous vertex
* ``AfterVertex``: Lock to next vertex
* ``NoVertex``: Don't lock to vertex

"""
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
Qgis.CadConstraintType.__doc__ = """Advanced digitizing constraint type.

.. versionadded:: 3.32

* ``Generic``: Generic value
* ``Angle``: Angle value
* ``Distance``: Distance value
* ``XCoordinate``: X Coordinate value
* ``YCoordinate``: Y Coordinate value
* ``ZValue``: Z value
* ``MValue``: M value

"""
# --
Qgis.CadConstraintType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProjectFlag.EvaluateDefaultValuesOnProviderSide.__doc__ = "If set, default values for fields will be evaluated on the provider side when features from the project are created instead of when they are committed."
Qgis.ProjectFlag.TrustStoredLayerStatistics.__doc__ = "If set, then layer statistics (such as the layer extent) will be read from values stored in the project instead of requesting updated values from the data provider. Additionally, when this flag is set, primary key unicity is not checked for views and materialized views with Postgres provider."
Qgis.ProjectFlag.RememberLayerEditStatusBetweenSessions.__doc__ = "If set, then any layers set to be editable will be stored in the project and immediately made editable whenever that project is restored"
Qgis.ProjectFlag.RememberAttributeTableWindowsBetweenSessions.__doc__ = "If set, then any open attribute tables will be stored in the project and immediately reopened when the project is restored"
Qgis.ProjectFlag.__doc__ = """Flags which control the behavior of :py:class:`QgsProjects`.

.. versionadded:: 3.26

* ``EvaluateDefaultValuesOnProviderSide``: If set, default values for fields will be evaluated on the provider side when features from the project are created instead of when they are committed.
* ``TrustStoredLayerStatistics``: If set, then layer statistics (such as the layer extent) will be read from values stored in the project instead of requesting updated values from the data provider. Additionally, when this flag is set, primary key unicity is not checked for views and materialized views with Postgres provider.
* ``RememberLayerEditStatusBetweenSessions``: If set, then any layers set to be editable will be stored in the project and immediately made editable whenever that project is restored
* ``RememberAttributeTableWindowsBetweenSessions``: If set, then any open attribute tables will be stored in the project and immediately reopened when the project is restored

"""
# --
Qgis.ProjectFlag.baseClass = Qgis
Qgis.ProjectFlags.baseClass = Qgis
ProjectFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.PlotToolFlag.ShowContextMenu.__doc__ = "Show a context menu when right-clicking with the tool."
Qgis.PlotToolFlag.__doc__ = """Flags that control the way the :py:class:`QgsPlotTools` operate.

.. versionadded:: 3.26

* ``ShowContextMenu``: Show a context menu when right-clicking with the tool.

"""
# --
Qgis.PlotToolFlag.baseClass = Qgis
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
Qgis.Point3DShape.__doc__ = """3D point shape types.

.. note::

   Prior to QGIS 3.36 this was available as :py:class:`QgsPoint3DSymbol`.Shape

.. versionadded:: 3.36

* ``Cylinder``: Cylinder
* ``Sphere``: Sphere
* ``Cone``: Cone
* ``Cube``: Cube
* ``Torus``: Torus
* ``Plane``: Flat plane
* ``ExtrudedText``: Extruded text
* ``Model``: Model
* ``Billboard``: Billboard

"""
# --
Qgis.Point3DShape.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LightSourceType.Point.__doc__ = "Point light source"
Qgis.LightSourceType.Directional.__doc__ = "Directional light source"
Qgis.LightSourceType.__doc__ = """Light source types for 3D scenes.

.. versionadded:: 3.26

* ``Point``: Point light source
* ``Directional``: Directional light source

"""
# --
Qgis.LightSourceType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.NavigationMode.TerrainBased.__doc__ = "The default navigation based on the terrain"
Qgis.NavigationMode.Walk.__doc__ = "Uses WASD keys or arrows to navigate in walking (first person) manner"
Qgis.NavigationMode.__doc__ = """The navigation mode used by 3D cameras.

.. versionadded:: 3.30

* ``TerrainBased``: The default navigation based on the terrain
* ``Walk``: Uses WASD keys or arrows to navigate in walking (first person) manner

"""
# --
Qgis.NavigationMode.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VerticalAxisInversion.Never.__doc__ = "Never invert vertical axis movements"
Qgis.VerticalAxisInversion.WhenDragging.__doc__ = "Invert vertical axis movements when dragging in first person modes"
Qgis.VerticalAxisInversion.Always.__doc__ = "Always invert vertical axis movements"
Qgis.VerticalAxisInversion.__doc__ = """Vertical axis inversion options for 3D views.

.. versionadded:: 3.30

* ``Never``: Never invert vertical axis movements
* ``WhenDragging``: Invert vertical axis movements when dragging in first person modes
* ``Always``: Always invert vertical axis movements

"""
# --
Qgis.VerticalAxisInversion.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProfileSurfaceSymbology.Line.__doc__ = "The elevation surface will be rendered using a line symbol"
Qgis.ProfileSurfaceSymbology.FillBelow.__doc__ = "The elevation surface will be rendered using a fill symbol below the surface level"
Qgis.ProfileSurfaceSymbology.FillAbove.__doc__ = "The elevation surface will be rendered using a fill symbol above the surface level \n.. versionadded:: 3.32"
Qgis.ProfileSurfaceSymbology.__doc__ = """Surface symbology type for elevation profile plots.

.. versionadded:: 3.26

* ``Line``: The elevation surface will be rendered using a line symbol
* ``FillBelow``: The elevation surface will be rendered using a fill symbol below the surface level
* ``FillAbove``: The elevation surface will be rendered using a fill symbol above the surface level

  .. versionadded:: 3.32


"""
# --
Qgis.ProfileSurfaceSymbology.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorProfileType.IndividualFeatures.__doc__ = "Treat each feature as an individual object (eg buildings)"
Qgis.VectorProfileType.ContinuousSurface.__doc__ = "The features should be treated as representing values on a continuous surface (eg contour lines)"
Qgis.VectorProfileType.__doc__ = """Types of elevation profiles to generate for vector sources.

.. versionadded:: 3.26

* ``IndividualFeatures``: Treat each feature as an individual object (eg buildings)
* ``ContinuousSurface``: The features should be treated as representing values on a continuous surface (eg contour lines)

"""
# --
Qgis.VectorProfileType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ProfileGeneratorFlag.RespectsMaximumErrorMapUnit.__doc__ = "Generated profile respects the QgsProfileGenerationContext.maximumErrorMapUnits() property."
Qgis.ProfileGeneratorFlag.RespectsDistanceRange.__doc__ = "Generated profile respects the QgsProfileGenerationContext.distanceRange() property."
Qgis.ProfileGeneratorFlag.RespectsElevationRange.__doc__ = "Generated profile respects the QgsProfileGenerationContext.elevationRange() property."
Qgis.ProfileGeneratorFlag.__doc__ = """Flags that control the way the :py:class:`QgsAbstractProfileGenerator` operate.

.. versionadded:: 3.26

* ``RespectsMaximumErrorMapUnit``: Generated profile respects the QgsProfileGenerationContext.maximumErrorMapUnits() property.
* ``RespectsDistanceRange``: Generated profile respects the QgsProfileGenerationContext.distanceRange() property.
* ``RespectsElevationRange``: Generated profile respects the QgsProfileGenerationContext.elevationRange() property.

"""
# --
Qgis.ProfileGeneratorFlag.baseClass = Qgis
Qgis.ProfileGeneratorFlags.baseClass = Qgis
ProfileGeneratorFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ProfileExportType.Features3D.__doc__ = "Export profiles as 3D features, with elevation values stored in exported geometry Z values"
Qgis.ProfileExportType.Profile2D.__doc__ = "Export profiles as 2D profile lines, with elevation stored in exported geometry Y dimension and distance in X dimension"
Qgis.ProfileExportType.DistanceVsElevationTable.__doc__ = "Export profiles as a table of sampled distance vs elevation values"
Qgis.ProfileExportType.__doc__ = """Types of export for elevation profiles.

.. versionadded:: 3.32

* ``Features3D``: Export profiles as 3D features, with elevation values stored in exported geometry Z values
* ``Profile2D``: Export profiles as 2D profile lines, with elevation stored in exported geometry Y dimension and distance in X dimension
* ``DistanceVsElevationTable``: Export profiles as a table of sampled distance vs elevation values

"""
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
Qgis.PointCloudSymbol.__doc__ = """Rendering symbols for point cloud points.

.. versionadded:: 3.26

* ``Square``: Renders points as squares
* ``Circle``: Renders points as circles

"""
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
Qgis.PointCloudDrawOrder.__doc__ = """Pointcloud rendering order for 2d views

/since QGIS 3.26

* ``Default``: Draw points in the order they are stored
* ``BottomToTop``: Draw points with larger Z values last
* ``TopToBottom``: Draw points with larger Z values first

"""
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
Qgis.AvoidIntersectionsMode.__doc__ = """Flags which control how intersections of pre-existing feature are handled when digitizing new features.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsProject`.AvoidIntersectionsMode

.. versionadded:: 3.26

* ``AllowIntersections``: Overlap with any feature allowed when digitizing new features
* ``AvoidIntersectionsCurrentLayer``: Overlap with features from the active layer when digitizing new features not allowed
* ``AvoidIntersectionsLayers``: Overlap with features from a specified list of layers when digitizing new features not allowed

"""
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
Qgis.ProjectFileFormat.__doc__ = """Flags which control project read behavior.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsProject`.FileFormat

.. versionadded:: 3.26

* ``Qgz``: Archive file format, supports auxiliary data
* ``Qgs``: Project saved in a clear text, does not support auxiliary data

"""
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
QgsProject.FlagDontLoad3DViews.__doc__ = "Skip loading 3D views \n.. versionadded:: 3.26"
QgsProject.DontLoadProjectStyles = Qgis.ProjectReadFlag.DontLoadProjectStyles
QgsProject.DontLoadProjectStyles.is_monkey_patched = True
QgsProject.DontLoadProjectStyles.__doc__ = "Skip loading project style databases (deprecated -- use ProjectCapability.ProjectStyles flag instead)"
QgsProject.ForceReadOnlyLayers = Qgis.ProjectReadFlag.ForceReadOnlyLayers
QgsProject.ForceReadOnlyLayers.is_monkey_patched = True
QgsProject.ForceReadOnlyLayers.__doc__ = "Open layers in a read-only mode. \n.. versionadded:: 3.28"
QgsProject.DontUpgradeAnnotations = Qgis.ProjectReadFlag.DontUpgradeAnnotations
QgsProject.DontUpgradeAnnotations.is_monkey_patched = True
QgsProject.DontUpgradeAnnotations.__doc__ = "Don't upgrade old annotation items to QgsAnnotationItem \n.. versionadded:: 3.40"
Qgis.ProjectReadFlag.__doc__ = """Flags which control project read behavior.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsProject`.ReadFlag

.. versionadded:: 3.26

* ``DontResolveLayers``: Don't resolve layer paths (i.e. don't load any layer content). Dramatically improves project read time if the actual data from the layers is not required.

  Available as ``QgsProject.FlagDontResolveLayers`` in older QGIS releases.

* ``DontLoadLayouts``: Don't load print layouts. Improves project read time if layouts are not required, and allows projects to be safely read in background threads (since print layouts are not thread safe).

  Available as ``QgsProject.FlagDontLoadLayouts`` in older QGIS releases.

* ``TrustLayerMetadata``: Trust layer metadata. Improves project read time. Do not use it if layers' extent is not fixed during the project's use by QGIS and QGIS Server.

  Available as ``QgsProject.FlagTrustLayerMetadata`` in older QGIS releases.

* ``DontStoreOriginalStyles``: Skip the initial XML style storage for layers. Useful for minimising project load times in non-interactive contexts.

  Available as ``QgsProject.FlagDontStoreOriginalStyles`` in older QGIS releases.

* ``DontLoad3DViews``: Skip loading 3D views

  .. versionadded:: 3.26


  Available as ``QgsProject.FlagDontLoad3DViews`` in older QGIS releases.

* ``DontLoadProjectStyles``: Skip loading project style databases (deprecated -- use ProjectCapability.ProjectStyles flag instead)
* ``ForceReadOnlyLayers``: Open layers in a read-only mode.

  .. versionadded:: 3.28

* ``DontUpgradeAnnotations``: Don't upgrade old annotation items to QgsAnnotationItem

  .. versionadded:: 3.40


"""
# --
Qgis.ProjectReadFlag.baseClass = Qgis
QgsProject.ReadFlags = Qgis.ProjectReadFlags
Qgis.ProjectReadFlags.baseClass = Qgis
ProjectReadFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ProjectCapability.ProjectStyles.__doc__ = "Enable the project embedded style library. Enabling this flag can increase the time required to clear and load projects."
Qgis.ProjectCapability.__doc__ = """Flags which control project capabilities.

These flags are specific upfront on creation of a :py:class:`QgsProject` object, and can
be used to selectively enable potentially costly functionality for the project.

.. versionadded:: 3.26.1

* ``ProjectStyles``: Enable the project embedded style library. Enabling this flag can increase the time required to clear and load projects.

"""
# --
Qgis.ProjectCapability.baseClass = Qgis
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
Qgis.MapBoxGlStyleSourceType.__doc__ = """Available MapBox GL style source types.

.. versionadded:: 3.28

* ``Vector``: Vector source
* ``Raster``: Raster source
* ``RasterDem``: Raster DEM source
* ``GeoJson``: GeoJSON source
* ``Image``: Image source
* ``Video``: Video source
* ``Unknown``: Other/unknown source type

"""
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
Qgis.ArcGisRestServiceType.__doc__ = """Available ArcGIS REST service types.

.. note::

   Prior to QGIS 3.26 this was available as :py:class:`QgsArcGisPortalUtils`.ItemType.

.. versionadded:: 3.28

* ``FeatureServer``: FeatureServer

  Available as ``QgsArcGisPortalUtils.FeatureService`` in older QGIS releases.

* ``MapServer``: MapServer

  Available as ``QgsArcGisPortalUtils.MapService`` in older QGIS releases.

* ``ImageServer``: ImageServer

  Available as ``QgsArcGisPortalUtils.ImageService`` in older QGIS releases.

* ``GlobeServer``: GlobeServer
* ``GPServer``: GPServer
* ``GeocodeServer``: GeocodeServer
* ``Unknown``: Other unknown/unsupported type

"""
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
Qgis.RelationshipType.__doc__ = """Relationship types.

.. note::

   Prior to QGIS 3.28 this was available as :py:class:`QgsRelation`.RelationType.

.. versionadded:: 3.28

* ``Normal``: A normal relation
* ``Generated``: A generated relation is a child of a polymorphic relation

"""
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
Qgis.RelationshipStrength.__doc__ = """Relationship strength.

.. note::

   Prior to QGIS 3.28 this was available as :py:class:`QgsRelation`.RelationStrength.

.. versionadded:: 3.28

* ``Association``: Loose relation, related elements are not part of the parent and a parent copy will not copy any children.
* ``Composition``: Fix relation, related elements are part of the parent and a parent copy will copy any children or delete of parent will delete children

"""
# --
Qgis.RelationshipStrength.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RelationshipCardinality.OneToOne.__doc__ = "One to one relationship"
Qgis.RelationshipCardinality.OneToMany.__doc__ = "One to many relationship"
Qgis.RelationshipCardinality.ManyToOne.__doc__ = "Many to one relationship"
Qgis.RelationshipCardinality.ManyToMany.__doc__ = "Many to many relationship"
Qgis.RelationshipCardinality.__doc__ = """Relationship cardinality.

.. versionadded:: 3.28

* ``OneToOne``: One to one relationship
* ``OneToMany``: One to many relationship
* ``ManyToOne``: Many to one relationship
* ``ManyToMany``: Many to many relationship

"""
# --
Qgis.RelationshipCardinality.baseClass = Qgis
# monkey patching scoped based enum
Qgis.RelationshipCapability.MultipleFieldKeys.__doc__ = "Supports multiple field keys (as opposed to a singular field)"
Qgis.RelationshipCapability.ForwardPathLabel.__doc__ = "Supports forward path labels"
Qgis.RelationshipCapability.BackwardPathLabel.__doc__ = "Supports backward path labels"
Qgis.RelationshipCapability.__doc__ = """Relationship capabilities.

.. versionadded:: 3.30

* ``MultipleFieldKeys``: Supports multiple field keys (as opposed to a singular field)
* ``ForwardPathLabel``: Supports forward path labels
* ``BackwardPathLabel``: Supports backward path labels

"""
# --
Qgis.RelationshipCapability.baseClass = Qgis
Qgis.RelationshipCapabilities.baseClass = Qgis
RelationshipCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.CoordinateDisplayType.MapCrs.__doc__ = "Map CRS"
Qgis.CoordinateDisplayType.MapGeographic.__doc__ = "Map Geographic CRS equivalent (stays unchanged if the map CRS is geographic)"
Qgis.CoordinateDisplayType.CustomCrs.__doc__ = "Custom CRS"
Qgis.CoordinateDisplayType.__doc__ = """Formats for displaying coordinates

.. versionadded:: 3.28

* ``MapCrs``: Map CRS
* ``MapGeographic``: Map Geographic CRS equivalent (stays unchanged if the map CRS is geographic)
* ``CustomCrs``: Custom CRS

"""
# --
Qgis.CoordinateDisplayType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.SettingsOrigin.Any.__doc__ = "From any origin"
Qgis.SettingsOrigin.Global.__doc__ = "Global settings are stored in `qgis_global_settings.ini`"
Qgis.SettingsOrigin.Local.__doc__ = "Local settings are stored in the user profile"
Qgis.SettingsOrigin.__doc__ = """The setting origin describes where a setting is stored.

.. versionadded:: 3.30

* ``Any``: From any origin
* ``Global``: Global settings are stored in `qgis_global_settings.ini`
* ``Local``: Local settings are stored in the user profile

"""
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
Qgis.ScriptLanguage.__doc__ = """Scripting languages.

.. versionadded:: 3.30

* ``Css``: CSS
* ``QgisExpression``: QGIS expressions
* ``Html``: HTML
* ``JavaScript``: JavaScript
* ``Json``: JSON
* ``Python``: Python
* ``R``: R Stats
* ``Sql``: SQL
* ``Batch``: Windows batch files
* ``Bash``: Bash scripts
* ``Unknown``: Unknown/other language

"""
# --
Qgis.ScriptLanguage.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ScriptLanguageCapability.Reformat.__doc__ = "Language supports automatic code reformatting"
Qgis.ScriptLanguageCapability.CheckSyntax.__doc__ = "Language supports syntax checking"
Qgis.ScriptLanguageCapability.ToggleComment.__doc__ = "Language supports comment toggling"
Qgis.ScriptLanguageCapability.__doc__ = """Script language capabilities.

The flags reflect the support capabilities of a scripting language.

.. versionadded:: 3.32

* ``Reformat``: Language supports automatic code reformatting
* ``CheckSyntax``: Language supports syntax checking
* ``ToggleComment``: Language supports comment toggling

"""
# --
Qgis.ScriptLanguageCapability.baseClass = Qgis
Qgis.ScriptLanguageCapabilities.baseClass = Qgis
ScriptLanguageCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.LayerTreeInsertionMethod.AboveInsertionPoint.__doc__ = "Layers are added in the tree above the insertion point"
Qgis.LayerTreeInsertionMethod.TopOfTree.__doc__ = "Layers are added at the top of the layer tree"
Qgis.LayerTreeInsertionMethod.OptimalInInsertionGroup.__doc__ = "Layers are added at optimal locations across the insertion point's group"
Qgis.LayerTreeInsertionMethod.__doc__ = """Layer tree insertion methods

.. versionadded:: 3.30

* ``AboveInsertionPoint``: Layers are added in the tree above the insertion point
* ``TopOfTree``: Layers are added at the top of the layer tree
* ``OptimalInInsertionGroup``: Layers are added at optimal locations across the insertion point's group

"""
# --
Qgis.LayerTreeInsertionMethod.baseClass = Qgis
# monkey patching scoped based enum
Qgis.LayerTreeFilterFlag.SkipVisibilityCheck.__doc__ = "If set, the standard visibility check should be skipped"
Qgis.LayerTreeFilterFlag.__doc__ = """Layer tree filter flags.

.. versionadded:: 3.32

* ``SkipVisibilityCheck``: If set, the standard visibility check should be skipped

"""
# --
Qgis.LayerTreeFilterFlag.baseClass = Qgis
Qgis.LayerTreeFilterFlags.baseClass = Qgis
LayerTreeFilterFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.LegendJsonRenderFlag.ShowRuleDetails.__doc__ = "If set, the rule expression of a rule based renderer legend item will be added to the JSON"
Qgis.LegendJsonRenderFlag.__doc__ = """Legend JSON export flags.

Flags to control JSON attributes when exporting a legend in JSON format.

.. versionadded:: 3.36

* ``ShowRuleDetails``: If set, the rule expression of a rule based renderer legend item will be added to the JSON

"""
# --
Qgis.LegendJsonRenderFlag.baseClass = Qgis
Qgis.LegendJsonRenderFlags.baseClass = Qgis
LegendJsonRenderFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ActionType.Invalid.__doc__ = "Invalid"
Qgis.ActionType.MapLayerAction.__doc__ = "Standard actions (defined by core or plugins), corresponds to QgsMapLayerAction class."
Qgis.ActionType.AttributeAction.__doc__ = "Custom actions (manually defined in layer properties), corresponds to QgsAction class."
Qgis.ActionType.__doc__ = """Action types.

Prior to QGIS 3.30 this was available as :py:class:`QgsActionMenu`.ActionType

.. versionadded:: 3.30

* ``Invalid``: Invalid
* ``MapLayerAction``: Standard actions (defined by core or plugins), corresponds to QgsMapLayerAction class.
* ``AttributeAction``: Custom actions (manually defined in layer properties), corresponds to QgsAction class.

"""
# --
Qgis.ActionType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MapLayerActionTarget.Layer.__doc__ = "Action targets a complete layer"
Qgis.MapLayerActionTarget.SingleFeature.__doc__ = "Action targets a single feature from a layer"
Qgis.MapLayerActionTarget.MultipleFeatures.__doc__ = "Action targets multiple features from a layer"
Qgis.MapLayerActionTarget.AllActions.__doc__ = ""
Qgis.MapLayerActionTarget.__doc__ = """Map layer action targets.

Prior to QGIS 3.30 this was available as :py:class:`QgsMapLayerAction`.Target

.. versionadded:: 3.30

* ``Layer``: Action targets a complete layer
* ``SingleFeature``: Action targets a single feature from a layer
* ``MultipleFeatures``: Action targets multiple features from a layer
* ``AllActions``: 

"""
# --
Qgis.MapLayerActionTarget.baseClass = Qgis
Qgis.MapLayerActionTargets.baseClass = Qgis
MapLayerActionTargets = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.MapLayerActionFlag.EnabledOnlyWhenEditable.__doc__ = "Action should be shown only for editable layers"
Qgis.MapLayerActionFlag.__doc__ = """Map layer action flags.

Prior to QGIS 3.30 this was available as :py:class:`QgsMapLayerAction`.Flag

.. versionadded:: 3.30

* ``EnabledOnlyWhenEditable``: Action should be shown only for editable layers

"""
# --
Qgis.MapLayerActionFlag.baseClass = Qgis
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
Qgis.AttributeActionType.__doc__ = """Attribute action types.

Prior to QGIS 3.30 this was available as :py:class:`QgsAction`.ActionType

.. versionadded:: 3.30

* ``Generic``: Generic
* ``GenericPython``: Python
* ``Mac``: MacOS specific
* ``Windows``: Windows specific
* ``Unix``: Unix specific
* ``OpenUrl``: Open URL action
* ``SubmitUrlEncoded``: POST data to an URL, using \"application/x-www-form-urlencoded\" or \"application/json\" if the body is valid JSON

  .. versionadded:: 3.24

* ``SubmitUrlMultipart``: POST data to an URL using \"multipart/form-data\"

  .. versionadded:: 3.24


"""
# --
Qgis.AttributeActionType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MetadataDateType.Created.__doc__ = "Date created"
Qgis.MetadataDateType.Published.__doc__ = "Date published"
Qgis.MetadataDateType.Revised.__doc__ = "Date revised"
Qgis.MetadataDateType.Superseded.__doc__ = "Date superseded"
Qgis.MetadataDateType.__doc__ = """Date types for metadata.

.. versionadded:: 3.30

* ``Created``: Date created
* ``Published``: Date published
* ``Revised``: Date revised
* ``Superseded``: Date superseded

"""
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
QgsRaster.RedBand.__doc__ = "Red band of RGBA image, or red spectral band [0.62 - 0.69 um]"
QgsRaster.GreenBand = Qgis.RasterColorInterpretation.GreenBand
QgsRaster.GreenBand.is_monkey_patched = True
QgsRaster.GreenBand.__doc__ = "Green band of RGBA image, or green spectral band [0.51 - 0.60 um]"
QgsRaster.BlueBand = Qgis.RasterColorInterpretation.BlueBand
QgsRaster.BlueBand.is_monkey_patched = True
QgsRaster.BlueBand.__doc__ = "Blue band of RGBA image, or blue spectral band [0.45 - 0.53 um]"
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
QgsRaster.YellowBand.__doc__ = "Yellow band of CMYK image, or yellow spectral band [0.58 - 0.62 um]"
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
QgsRaster.PanBand = Qgis.RasterColorInterpretation.PanBand
QgsRaster.PanBand.is_monkey_patched = True
QgsRaster.PanBand.__doc__ = "Panchromatic band [0.40 - 1.00 um] \n.. versionadded:: 3.40"
QgsRaster.CoastalBand = Qgis.RasterColorInterpretation.CoastalBand
QgsRaster.CoastalBand.is_monkey_patched = True
QgsRaster.CoastalBand.__doc__ = "Coastal band [0.40 - 0.45 um] \n.. versionadded:: 3.40"
QgsRaster.RedEdgeBand = Qgis.RasterColorInterpretation.RedEdgeBand
QgsRaster.RedEdgeBand.is_monkey_patched = True
QgsRaster.RedEdgeBand.__doc__ = "Red-edge band [0.69 - 0.79 um] \n.. versionadded:: 3.40"
QgsRaster.NIRBand = Qgis.RasterColorInterpretation.NIRBand
QgsRaster.NIRBand.is_monkey_patched = True
QgsRaster.NIRBand.__doc__ = "Near-InfraRed (NIR) band [0.75 - 1.40 um] \n.. versionadded:: 3.40"
QgsRaster.SWIRBand = Qgis.RasterColorInterpretation.SWIRBand
QgsRaster.SWIRBand.is_monkey_patched = True
QgsRaster.SWIRBand.__doc__ = "Short-Wavelength InfraRed (SWIR) band [1.40 - 3.00 um] \n.. versionadded:: 3.40"
QgsRaster.MWIRBand = Qgis.RasterColorInterpretation.MWIRBand
QgsRaster.MWIRBand.is_monkey_patched = True
QgsRaster.MWIRBand.__doc__ = "Mid-Wavelength InfraRed (MWIR) band [3.00 - 8.00 um] \n.. versionadded:: 3.40"
QgsRaster.LWIRBand = Qgis.RasterColorInterpretation.LWIRBand
QgsRaster.LWIRBand.is_monkey_patched = True
QgsRaster.LWIRBand.__doc__ = "Long-Wavelength InfraRed (LWIR) band [8.00 - 15 um] \n.. versionadded:: 3.40"
QgsRaster.TIRBand = Qgis.RasterColorInterpretation.TIRBand
QgsRaster.TIRBand.is_monkey_patched = True
QgsRaster.TIRBand.__doc__ = "Thermal InfraRed (TIR) band (MWIR or LWIR) [3 - 15 um] \n.. versionadded:: 3.40"
QgsRaster.OtherIRBand = Qgis.RasterColorInterpretation.OtherIRBand
QgsRaster.OtherIRBand.is_monkey_patched = True
QgsRaster.OtherIRBand.__doc__ = "Other infrared band [0.75 - 1000 um] \n.. versionadded:: 3.40"
QgsRaster.SAR_Ka_Band = Qgis.RasterColorInterpretation.SAR_Ka_Band
QgsRaster.SAR_Ka_Band.is_monkey_patched = True
QgsRaster.SAR_Ka_Band.__doc__ = "Synthetic Aperture Radar (SAR) Ka band [0.8 - 1.1 cm / 27 - 40 GHz] \n.. versionadded:: 3.40"
QgsRaster.SAR_K_Band = Qgis.RasterColorInterpretation.SAR_K_Band
QgsRaster.SAR_K_Band.is_monkey_patched = True
QgsRaster.SAR_K_Band.__doc__ = "Synthetic Aperture Radar (SAR) K band [1.1 - 1.7 cm / 18 - 27 GHz] \n.. versionadded:: 3.40"
QgsRaster.SAR_Ku_Band = Qgis.RasterColorInterpretation.SAR_Ku_Band
QgsRaster.SAR_Ku_Band.is_monkey_patched = True
QgsRaster.SAR_Ku_Band.__doc__ = "Synthetic Aperture Radar (SAR) Ku band [1.7 - 2.4 cm / 12 - 18 GHz] \n.. versionadded:: 3.40"
QgsRaster.SAR_X_Band = Qgis.RasterColorInterpretation.SAR_X_Band
QgsRaster.SAR_X_Band.is_monkey_patched = True
QgsRaster.SAR_X_Band.__doc__ = "Synthetic Aperture Radar (SAR) X band [2.4 - 3.8 cm / 8 - 12 GHz] \n.. versionadded:: 3.40"
QgsRaster.SAR_C_Band = Qgis.RasterColorInterpretation.SAR_C_Band
QgsRaster.SAR_C_Band.is_monkey_patched = True
QgsRaster.SAR_C_Band.__doc__ = "Synthetic Aperture Radar (SAR) C band [3.8 - 7.5 cm / 4 - 8 GHz] \n.. versionadded:: 3.40"
QgsRaster.SAR_S_Band = Qgis.RasterColorInterpretation.SAR_S_Band
QgsRaster.SAR_S_Band.is_monkey_patched = True
QgsRaster.SAR_S_Band.__doc__ = "Synthetic Aperture Radar (SAR) S band [7.5 - 15 cm / 2 - 4 GHz] \n.. versionadded:: 3.40"
QgsRaster.SAR_L_Band = Qgis.RasterColorInterpretation.SAR_L_Band
QgsRaster.SAR_L_Band.is_monkey_patched = True
QgsRaster.SAR_L_Band.__doc__ = "Synthetic Aperture Radar (SAR) L band [15 - 30 cm / 1 - 2 GHz] \n.. versionadded:: 3.40"
QgsRaster.SAR_P_Band = Qgis.RasterColorInterpretation.SAR_P_Band
QgsRaster.SAR_P_Band.is_monkey_patched = True
QgsRaster.SAR_P_Band.__doc__ = "Synthetic Aperture Radar (SAR) P band [30 - 100 cm / 0.3 - 1 GHz] \n.. versionadded:: 3.40"
Qgis.RasterColorInterpretation.__doc__ = """Raster color interpretation.

This is a modified copy of the GDAL GDALColorInterp enum.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.ColorInterpretation

.. versionadded:: 3.30

* ``Undefined``: Undefined

  Available as ``QgsRaster.UndefinedColorInterpretation`` in older QGIS releases.

* ``GrayIndex``: Grayscale
* ``PaletteIndex``: Paletted (see associated color table)
* ``RedBand``: Red band of RGBA image, or red spectral band [0.62 - 0.69 um]
* ``GreenBand``: Green band of RGBA image, or green spectral band [0.51 - 0.60 um]
* ``BlueBand``: Blue band of RGBA image, or blue spectral band [0.45 - 0.53 um]
* ``AlphaBand``: Alpha (0=transparent, 255=opaque)
* ``HueBand``: Hue band of HLS image
* ``SaturationBand``: Saturation band of HLS image
* ``LightnessBand``: Lightness band of HLS image
* ``CyanBand``: Cyan band of CMYK image
* ``MagentaBand``: Magenta band of CMYK image
* ``YellowBand``: Yellow band of CMYK image, or yellow spectral band [0.58 - 0.62 um]
* ``BlackBand``: Black band of CMLY image
* ``YCbCr_YBand``: Y Luminance
* ``YCbCr_CbBand``: Cb Chroma
* ``YCbCr_CrBand``: Cr Chroma
* ``ContinuousPalette``: Continuous palette, QGIS addition, GRASS
* ``PanBand``: Panchromatic band [0.40 - 1.00 um]

  .. versionadded:: 3.40

* ``CoastalBand``: Coastal band [0.40 - 0.45 um]

  .. versionadded:: 3.40

* ``RedEdgeBand``: Red-edge band [0.69 - 0.79 um]

  .. versionadded:: 3.40

* ``NIRBand``: Near-InfraRed (NIR) band [0.75 - 1.40 um]

  .. versionadded:: 3.40

* ``SWIRBand``: Short-Wavelength InfraRed (SWIR) band [1.40 - 3.00 um]

  .. versionadded:: 3.40

* ``MWIRBand``: Mid-Wavelength InfraRed (MWIR) band [3.00 - 8.00 um]

  .. versionadded:: 3.40

* ``LWIRBand``: Long-Wavelength InfraRed (LWIR) band [8.00 - 15 um]

  .. versionadded:: 3.40

* ``TIRBand``: Thermal InfraRed (TIR) band (MWIR or LWIR) [3 - 15 um]

  .. versionadded:: 3.40

* ``OtherIRBand``: Other infrared band [0.75 - 1000 um]

  .. versionadded:: 3.40

* ``SAR_Ka_Band``: Synthetic Aperture Radar (SAR) Ka band [0.8 - 1.1 cm / 27 - 40 GHz]

  .. versionadded:: 3.40

* ``SAR_K_Band``: Synthetic Aperture Radar (SAR) K band [1.1 - 1.7 cm / 18 - 27 GHz]

  .. versionadded:: 3.40

* ``SAR_Ku_Band``: Synthetic Aperture Radar (SAR) Ku band [1.7 - 2.4 cm / 12 - 18 GHz]

  .. versionadded:: 3.40

* ``SAR_X_Band``: Synthetic Aperture Radar (SAR) X band [2.4 - 3.8 cm / 8 - 12 GHz]

  .. versionadded:: 3.40

* ``SAR_C_Band``: Synthetic Aperture Radar (SAR) C band [3.8 - 7.5 cm / 4 - 8 GHz]

  .. versionadded:: 3.40

* ``SAR_S_Band``: Synthetic Aperture Radar (SAR) S band [7.5 - 15 cm / 2 - 4 GHz]

  .. versionadded:: 3.40

* ``SAR_L_Band``: Synthetic Aperture Radar (SAR) L band [15 - 30 cm / 1 - 2 GHz]

  .. versionadded:: 3.40

* ``SAR_P_Band``: Synthetic Aperture Radar (SAR) P band [30 - 100 cm / 0.3 - 1 GHz]

  .. versionadded:: 3.40


"""
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
Qgis.RasterLayerType.__doc__ = """Raster layer types.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsRasterLayer`.LayerType

.. versionadded:: 3.30

* ``GrayOrUndefined``: Gray or undefined
* ``Palette``: Palette
* ``MultiBand``: Multi band

  Available as ``QgsRasterLayer.Multiband`` in older QGIS releases.

* ``SingleBandColorData``: Single band containing color data

  Available as ``QgsRasterLayer.ColorLayer`` in older QGIS releases.


"""
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
Qgis.RasterDrawingStyle.__doc__ = """Raster drawing styles.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.DrawingStyle

.. versionadded:: 3.30

* ``Undefined``: Undefined

  Available as ``QgsRaster.UndefinedDrawingStyle`` in older QGIS releases.

* ``SingleBandGray``: A single band image drawn as a range of gray colors
* ``SingleBandPseudoColor``: A single band image drawn using a pseudocolor algorithm
* ``PalettedColor``: A \"Palette\" image drawn using color table
* ``PalettedSingleBandGray``: A \"Palette\" layer drawn in gray scale
* ``PalettedSingleBandPseudoColor``: A \"Palette\" layerdrawn using a pseudocolor algorithm
* ``PalettedMultiBandColor``: Currently not supported
* ``MultiBandSingleBandGray``: A layer containing 2 or more bands, but a single band drawn as a range of gray colors
* ``MultiBandSingleBandPseudoColor``: A layer containing 2 or more bands, but a single band drawn using a pseudocolor algorithm
* ``MultiBandColor``: A layer containing 2 or more bands, mapped to RGB color space. In the case of a multiband with only two bands, one band will be mapped to more than one color.
* ``SingleBandColorData``: ARGB values rendered directly

  Available as ``QgsRaster.SingleBandColorDataStyle`` in older QGIS releases.


"""
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
Qgis.RasterPyramidFormat.__doc__ = """Raster pyramid formats.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.RasterPyramidsFormat

.. versionadded:: 3.30

* ``GeoTiff``: Geotiff .ovr (external)

  Available as ``QgsRaster.PyramidsGTiff`` in older QGIS releases.

* ``Internal``: Internal

  Available as ``QgsRaster.PyramidsInternal`` in older QGIS releases.

* ``Erdas``: Erdas Image .aux (external)

  Available as ``QgsRaster.PyramidsErdas`` in older QGIS releases.


"""
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
Qgis.RasterBuildPyramidOption.__doc__ = """Raster pyramid building options.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.RasterBuildPyramids

.. versionadded:: 3.30

* ``No``: Never

  Available as ``QgsRaster.PyramidsFlagNo`` in older QGIS releases.

* ``Yes``: Yes

  Available as ``QgsRaster.PyramidsFlagYes`` in older QGIS releases.

* ``CopyExisting``: Copy existing

  Available as ``QgsRaster.PyramidsCopyExisting`` in older QGIS releases.


"""
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
Qgis.RasterIdentifyFormat.__doc__ = """Raster identify formats.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsRaster`.IdentifyFormat

.. versionadded:: 3.30

* ``Undefined``: Undefined

  Available as ``QgsRaster.IdentifyFormatUndefined`` in older QGIS releases.

* ``Value``: Numerical pixel value

  Available as ``QgsRaster.IdentifyFormatValue`` in older QGIS releases.

* ``Text``: WMS text

  Available as ``QgsRaster.IdentifyFormatText`` in older QGIS releases.

* ``Html``: WMS HTML

  Available as ``QgsRaster.IdentifyFormatHtml`` in older QGIS releases.

* ``Feature``: WMS GML/JSON -> feature

  Available as ``QgsRaster.IdentifyFormatFeature`` in older QGIS releases.


"""
# --
Qgis.RasterIdentifyFormat.baseClass = Qgis
QgsRasterInterface.Capability = Qgis.RasterInterfaceCapability
# monkey patching scoped based enum
QgsRasterInterface.NoCapabilities = Qgis.RasterInterfaceCapability.NoCapabilities
QgsRasterInterface.NoCapabilities.is_monkey_patched = True
QgsRasterInterface.NoCapabilities.__doc__ = "No capabilities"
QgsRasterInterface.Size = Qgis.RasterInterfaceCapability.Size
QgsRasterInterface.Size.is_monkey_patched = True
QgsRasterInterface.Size.__doc__ = "Original data source size (and thus resolution) is known, it is not always available, for example for WMS"
QgsRasterInterface.Create = Qgis.RasterInterfaceCapability.Create
QgsRasterInterface.Create.is_monkey_patched = True
QgsRasterInterface.Create.__doc__ = "Create new datasets (Unused and deprecated -- will be removed in QGIS 4)"
QgsRasterInterface.Remove = Qgis.RasterInterfaceCapability.Remove
QgsRasterInterface.Remove.is_monkey_patched = True
QgsRasterInterface.Remove.__doc__ = "Delete datasets (Unused and deprecated -- will be removed in QGIS 4)"
QgsRasterInterface.BuildPyramids = Qgis.RasterInterfaceCapability.BuildPyramids
QgsRasterInterface.BuildPyramids.is_monkey_patched = True
QgsRasterInterface.BuildPyramids.__doc__ = "Supports building of pyramids (overviews) (Deprecated since QGIS 3.38 -- use RasterProviderCapability.BuildPyramids instead)"
QgsRasterInterface.Identify = Qgis.RasterInterfaceCapability.Identify
QgsRasterInterface.Identify.is_monkey_patched = True
QgsRasterInterface.Identify.__doc__ = "At least one identify format supported"
QgsRasterInterface.IdentifyValue = Qgis.RasterInterfaceCapability.IdentifyValue
QgsRasterInterface.IdentifyValue.is_monkey_patched = True
QgsRasterInterface.IdentifyValue.__doc__ = "Numerical values"
QgsRasterInterface.IdentifyText = Qgis.RasterInterfaceCapability.IdentifyText
QgsRasterInterface.IdentifyText.is_monkey_patched = True
QgsRasterInterface.IdentifyText.__doc__ = "WMS text"
QgsRasterInterface.IdentifyHtml = Qgis.RasterInterfaceCapability.IdentifyHtml
QgsRasterInterface.IdentifyHtml.is_monkey_patched = True
QgsRasterInterface.IdentifyHtml.__doc__ = "WMS HTML"
QgsRasterInterface.IdentifyFeature = Qgis.RasterInterfaceCapability.IdentifyFeature
QgsRasterInterface.IdentifyFeature.is_monkey_patched = True
QgsRasterInterface.IdentifyFeature.__doc__ = "WMS GML -> feature"
QgsRasterInterface.Prefetch = Qgis.RasterInterfaceCapability.Prefetch
QgsRasterInterface.Prefetch.is_monkey_patched = True
QgsRasterInterface.Prefetch.__doc__ = "Allow prefetching of out-of-view images"
Qgis.RasterInterfaceCapability.__doc__ = """Raster interface capabilities.

.. note::

   Prior to QGIS 3.38 this was available as :py:class:`QgsRasterInterface`.Capability

.. versionadded:: 3.38

* ``NoCapabilities``: No capabilities
* ``Size``: Original data source size (and thus resolution) is known, it is not always available, for example for WMS
* ``Create``: Create new datasets (Unused and deprecated -- will be removed in QGIS 4)
* ``Remove``: Delete datasets (Unused and deprecated -- will be removed in QGIS 4)
* ``BuildPyramids``: Supports building of pyramids (overviews) (Deprecated since QGIS 3.38 -- use RasterProviderCapability.BuildPyramids instead)
* ``Identify``: At least one identify format supported
* ``IdentifyValue``: Numerical values
* ``IdentifyText``: WMS text
* ``IdentifyHtml``: WMS HTML
* ``IdentifyFeature``: WMS GML -> feature
* ``Prefetch``: Allow prefetching of out-of-view images

"""
# --
Qgis.RasterInterfaceCapability.baseClass = Qgis
Qgis.RasterInterfaceCapabilities.baseClass = Qgis
RasterInterfaceCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
QgsRasterDataProvider.ProviderCapability = Qgis.RasterProviderCapability
# monkey patching scoped based enum
QgsRasterDataProvider.NoProviderCapabilities = Qgis.RasterProviderCapability.NoProviderCapabilities
QgsRasterDataProvider.NoProviderCapabilities.is_monkey_patched = True
QgsRasterDataProvider.NoProviderCapabilities.__doc__ = "Provider has no capabilities"
QgsRasterDataProvider.ReadLayerMetadata = Qgis.RasterProviderCapability.ReadLayerMetadata
QgsRasterDataProvider.ReadLayerMetadata.is_monkey_patched = True
QgsRasterDataProvider.ReadLayerMetadata.__doc__ = "Provider can read layer metadata from data store. Since QGIS 3.0. See QgsDataProvider.layerMetadata()"
QgsRasterDataProvider.WriteLayerMetadata = Qgis.RasterProviderCapability.WriteLayerMetadata
QgsRasterDataProvider.WriteLayerMetadata.is_monkey_patched = True
QgsRasterDataProvider.WriteLayerMetadata.__doc__ = "Provider can write layer metadata to the data store. Since QGIS 3.0. See QgsDataProvider.writeLayerMetadata()"
QgsRasterDataProvider.ProviderHintBenefitsFromResampling = Qgis.RasterProviderCapability.ProviderHintBenefitsFromResampling
QgsRasterDataProvider.ProviderHintBenefitsFromResampling.is_monkey_patched = True
QgsRasterDataProvider.ProviderHintBenefitsFromResampling.__doc__ = "Provider benefits from resampling and should apply user default resampling settings \n.. versionadded:: 3.10"
QgsRasterDataProvider.ProviderHintCanPerformProviderResampling = Qgis.RasterProviderCapability.ProviderHintCanPerformProviderResampling
QgsRasterDataProvider.ProviderHintCanPerformProviderResampling.is_monkey_patched = True
QgsRasterDataProvider.ProviderHintCanPerformProviderResampling.__doc__ = "Provider can perform resampling (to be opposed to post rendering resampling) \n.. versionadded:: 3.16"
QgsRasterDataProvider.ReloadData = Qgis.RasterProviderCapability.ReloadData
QgsRasterDataProvider.ReloadData.is_monkey_patched = True
QgsRasterDataProvider.ReloadData.__doc__ = "Is able to force reload data / clear local caches. Since QGIS 3.18, see QgsDataProvider.reloadProviderData()"
QgsRasterDataProvider.DpiDependentData = Qgis.RasterProviderCapability.DpiDependentData
QgsRasterDataProvider.DpiDependentData.is_monkey_patched = True
QgsRasterDataProvider.DpiDependentData.__doc__ = "Provider's rendering is dependent on requested pixel size of the viewport \n.. versionadded:: 3.20"
QgsRasterDataProvider.NativeRasterAttributeTable = Qgis.RasterProviderCapability.NativeRasterAttributeTable
QgsRasterDataProvider.NativeRasterAttributeTable.is_monkey_patched = True
QgsRasterDataProvider.NativeRasterAttributeTable.__doc__ = "Indicates that the provider supports native raster attribute table \n.. versionadded:: 3.30"
QgsRasterDataProvider.BuildPyramids = Qgis.RasterProviderCapability.BuildPyramids
QgsRasterDataProvider.BuildPyramids.is_monkey_patched = True
QgsRasterDataProvider.BuildPyramids.__doc__ = "Supports building of pyramids (overviews) (since QGIS 3.38 -- this is a replacement for RasterInterfaceCapability.BuildPyramids)"
Qgis.RasterProviderCapability.__doc__ = """Raster data provider capabilities.

.. note::

   Prior to QGIS 3.38 this was available as :py:class:`QgsRasterDataProvider`.ProviderCapability

.. versionadded:: 3.38

* ``NoProviderCapabilities``: Provider has no capabilities
* ``ReadLayerMetadata``: Provider can read layer metadata from data store. Since QGIS 3.0. See QgsDataProvider.layerMetadata()
* ``WriteLayerMetadata``: Provider can write layer metadata to the data store. Since QGIS 3.0. See QgsDataProvider.writeLayerMetadata()
* ``ProviderHintBenefitsFromResampling``: Provider benefits from resampling and should apply user default resampling settings

  .. versionadded:: 3.10

* ``ProviderHintCanPerformProviderResampling``: Provider can perform resampling (to be opposed to post rendering resampling)

  .. versionadded:: 3.16

* ``ReloadData``: Is able to force reload data / clear local caches. Since QGIS 3.18, see QgsDataProvider.reloadProviderData()
* ``DpiDependentData``: Provider's rendering is dependent on requested pixel size of the viewport

  .. versionadded:: 3.20

* ``NativeRasterAttributeTable``: Indicates that the provider supports native raster attribute table

  .. versionadded:: 3.30

* ``BuildPyramids``: Supports building of pyramids (overviews) (since QGIS 3.38 -- this is a replacement for RasterInterfaceCapability.BuildPyramids)

"""
# --
Qgis.RasterProviderCapability.baseClass = Qgis
QgsRasterDataProvider.ProviderCapabilities = Qgis.RasterProviderCapabilities
Qgis.RasterProviderCapabilities.baseClass = Qgis
RasterProviderCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ElevationMapCombineMethod.HighestElevation.__doc__ = "Keep the highest elevation if it is not null"
Qgis.ElevationMapCombineMethod.NewerElevation.__doc__ = "Keep the new elevation regardless of its value if it is not null"
Qgis.ElevationMapCombineMethod.__doc__ = """Methods used to select the elevation when two elevation maps are combined

.. versionadded:: 3.30

* ``HighestElevation``: Keep the highest elevation if it is not null
* ``NewerElevation``: Keep the new elevation regardless of its value if it is not null

"""
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
Qgis.BlendMode.__doc__ = """Blending modes defining the available composition modes that can
be used when painting.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsPainting`.BlendMode.

.. versionadded:: 3.30

* ``Normal``: Normal

  Available as ``QgsPainting.BlendNormal`` in older QGIS releases.

* ``Lighten``: Lighten

  Available as ``QgsPainting.BlendLighten`` in older QGIS releases.

* ``Screen``: Screen

  Available as ``QgsPainting.BlendScreen`` in older QGIS releases.

* ``Dodge``: Dodge

  Available as ``QgsPainting.BlendDodge`` in older QGIS releases.

* ``Addition``: Addition

  Available as ``QgsPainting.BlendAddition`` in older QGIS releases.

* ``Darken``: Darken

  Available as ``QgsPainting.BlendDarken`` in older QGIS releases.

* ``Multiply``: Multiple

  Available as ``QgsPainting.BlendMultiply`` in older QGIS releases.

* ``Burn``: Burn

  Available as ``QgsPainting.BlendBurn`` in older QGIS releases.

* ``Overlay``: Overlay

  Available as ``QgsPainting.BlendOverlay`` in older QGIS releases.

* ``SoftLight``: Soft light

  Available as ``QgsPainting.BlendSoftLight`` in older QGIS releases.

* ``HardLight``: Hard light

  Available as ``QgsPainting.BlendHardLight`` in older QGIS releases.

* ``Difference``: Difference

  Available as ``QgsPainting.BlendDifference`` in older QGIS releases.

* ``Subtract``: Subtract

  Available as ``QgsPainting.BlendSubtract`` in older QGIS releases.

* ``Source``: Source

  Available as ``QgsPainting.BlendSource`` in older QGIS releases.

* ``DestinationOver``: Destination over

  Available as ``QgsPainting.BlendDestinationOver`` in older QGIS releases.

* ``Clear``: Clear

  Available as ``QgsPainting.BlendClear`` in older QGIS releases.

* ``Destination``: Destination

  Available as ``QgsPainting.BlendDestination`` in older QGIS releases.

* ``SourceIn``: Source in

  Available as ``QgsPainting.BlendSourceIn`` in older QGIS releases.

* ``DestinationIn``: Destination in

  Available as ``QgsPainting.BlendDestinationIn`` in older QGIS releases.

* ``SourceOut``: Source out

  Available as ``QgsPainting.BlendSourceOut`` in older QGIS releases.

* ``DestinationOut``: Destination out

  Available as ``QgsPainting.BlendDestinationOut`` in older QGIS releases.

* ``SourceAtop``: Source atop

  Available as ``QgsPainting.BlendSourceAtop`` in older QGIS releases.

* ``DestinationAtop``: Destination atop

  Available as ``QgsPainting.BlendDestinationAtop`` in older QGIS releases.

* ``Xor``: XOR

  Available as ``QgsPainting.BlendXor`` in older QGIS releases.


"""
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
Qgis.SystemOfMeasurement.__doc__ = """Systems of unit measurement.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.SystemOfMeasurement.

.. versionadded:: 3.30

* ``Unknown``: Unknown system of measurement

  Available as ``QgsUnitTypes.UnknownSystem`` in older QGIS releases.

* ``Metric``: International System of Units (SI)

  Available as ``QgsUnitTypes.MetricSystem`` in older QGIS releases.

* ``Imperial``: British Imperial

  Available as ``QgsUnitTypes.ImperialSystem`` in older QGIS releases.

* ``USCS``: United States customary system

  Available as ``QgsUnitTypes.USCSSystem`` in older QGIS releases.


"""
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
Qgis.MapToolUnit.__doc__ = """Type of unit of tolerance value from settings.
For map (project) units, use MapToolUnit.Project.

.. versionadded:: 3.32

* ``Layer``: Layer unit value

  Available as ``QgsTolerance.LayerUnits`` in older QGIS releases.

* ``Pixels``: Pixels unit of tolerance
* ``Project``: Map (project) units

  Available as ``QgsTolerance.ProjectUnits`` in older QGIS releases.


"""
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
Qgis.UnitType.__doc__ = """Unit types.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.UnitType.

.. versionadded:: 3.30

* ``Distance``: Distance unit

  Available as ``QgsUnitTypes.TypeDistance`` in older QGIS releases.

* ``Area``: Area unit

  Available as ``QgsUnitTypes.TypeArea`` in older QGIS releases.

* ``Volume``: Volume unit

  Available as ``QgsUnitTypes.TypeVolume`` in older QGIS releases.

* ``Unknown``: Unknown unit type

  Available as ``QgsUnitTypes.TypeUnknown`` in older QGIS releases.

* ``Temporal``: Temporal unit

  Available as ``QgsUnitTypes.TypeTemporal`` in older QGIS releases.


"""
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
QgsUnitTypes.Inches.__doc__ = "Inches \n.. versionadded:: 3.32"
QgsUnitTypes.ChainsInternational = Qgis.DistanceUnit.ChainsInternational
QgsUnitTypes.ChainsInternational.is_monkey_patched = True
QgsUnitTypes.ChainsInternational.__doc__ = "International chains \n.. versionadded:: 3.40"
QgsUnitTypes.ChainsBritishBenoit1895A = Qgis.DistanceUnit.ChainsBritishBenoit1895A
QgsUnitTypes.ChainsBritishBenoit1895A.is_monkey_patched = True
QgsUnitTypes.ChainsBritishBenoit1895A.__doc__ = "British chains (Benoit 1895 A) \n.. versionadded:: 3.40"
QgsUnitTypes.ChainsBritishBenoit1895B = Qgis.DistanceUnit.ChainsBritishBenoit1895B
QgsUnitTypes.ChainsBritishBenoit1895B.is_monkey_patched = True
QgsUnitTypes.ChainsBritishBenoit1895B.__doc__ = "British chains (Benoit 1895 B) \n.. versionadded:: 3.40"
QgsUnitTypes.ChainsBritishSears1922Truncated = Qgis.DistanceUnit.ChainsBritishSears1922Truncated
QgsUnitTypes.ChainsBritishSears1922Truncated.is_monkey_patched = True
QgsUnitTypes.ChainsBritishSears1922Truncated.__doc__ = "British chains (Sears 1922 truncated) \n.. versionadded:: 3.40"
QgsUnitTypes.ChainsBritishSears1922 = Qgis.DistanceUnit.ChainsBritishSears1922
QgsUnitTypes.ChainsBritishSears1922.is_monkey_patched = True
QgsUnitTypes.ChainsBritishSears1922.__doc__ = "British chains (Sears 1922) \n.. versionadded:: 3.40"
QgsUnitTypes.ChainsClarkes = Qgis.DistanceUnit.ChainsClarkes
QgsUnitTypes.ChainsClarkes.is_monkey_patched = True
QgsUnitTypes.ChainsClarkes.__doc__ = "Clarke's chains \n.. versionadded:: 3.40"
QgsUnitTypes.ChainsUSSurvey = Qgis.DistanceUnit.ChainsUSSurvey
QgsUnitTypes.ChainsUSSurvey.is_monkey_patched = True
QgsUnitTypes.ChainsUSSurvey.__doc__ = "US Survey chains \n.. versionadded:: 3.40"
QgsUnitTypes.FeetBritish1865 = Qgis.DistanceUnit.FeetBritish1865
QgsUnitTypes.FeetBritish1865.is_monkey_patched = True
QgsUnitTypes.FeetBritish1865.__doc__ = "British feet (1865) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetBritish1936 = Qgis.DistanceUnit.FeetBritish1936
QgsUnitTypes.FeetBritish1936.is_monkey_patched = True
QgsUnitTypes.FeetBritish1936.__doc__ = "British feet (1936) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetBritishBenoit1895A = Qgis.DistanceUnit.FeetBritishBenoit1895A
QgsUnitTypes.FeetBritishBenoit1895A.is_monkey_patched = True
QgsUnitTypes.FeetBritishBenoit1895A.__doc__ = "British feet (Benoit 1895 A) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetBritishBenoit1895B = Qgis.DistanceUnit.FeetBritishBenoit1895B
QgsUnitTypes.FeetBritishBenoit1895B.is_monkey_patched = True
QgsUnitTypes.FeetBritishBenoit1895B.__doc__ = "British feet (Benoit 1895 B) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetBritishSears1922Truncated = Qgis.DistanceUnit.FeetBritishSears1922Truncated
QgsUnitTypes.FeetBritishSears1922Truncated.is_monkey_patched = True
QgsUnitTypes.FeetBritishSears1922Truncated.__doc__ = "British feet (Sears 1922 truncated) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetBritishSears1922 = Qgis.DistanceUnit.FeetBritishSears1922
QgsUnitTypes.FeetBritishSears1922.is_monkey_patched = True
QgsUnitTypes.FeetBritishSears1922.__doc__ = "British feet (Sears 1922) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetClarkes = Qgis.DistanceUnit.FeetClarkes
QgsUnitTypes.FeetClarkes.is_monkey_patched = True
QgsUnitTypes.FeetClarkes.__doc__ = "Clarke's feet \n.. versionadded:: 3.40"
QgsUnitTypes.FeetGoldCoast = Qgis.DistanceUnit.FeetGoldCoast
QgsUnitTypes.FeetGoldCoast.is_monkey_patched = True
QgsUnitTypes.FeetGoldCoast.__doc__ = "Gold Coast feet \n.. versionadded:: 3.40"
QgsUnitTypes.FeetIndian = Qgis.DistanceUnit.FeetIndian
QgsUnitTypes.FeetIndian.is_monkey_patched = True
QgsUnitTypes.FeetIndian.__doc__ = "Indian (geodetic) feet \n.. versionadded:: 3.40"
QgsUnitTypes.FeetIndian1937 = Qgis.DistanceUnit.FeetIndian1937
QgsUnitTypes.FeetIndian1937.is_monkey_patched = True
QgsUnitTypes.FeetIndian1937.__doc__ = "Indian feet (1937) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetIndian1962 = Qgis.DistanceUnit.FeetIndian1962
QgsUnitTypes.FeetIndian1962.is_monkey_patched = True
QgsUnitTypes.FeetIndian1962.__doc__ = "Indian feet (1962) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetIndian1975 = Qgis.DistanceUnit.FeetIndian1975
QgsUnitTypes.FeetIndian1975.is_monkey_patched = True
QgsUnitTypes.FeetIndian1975.__doc__ = "Indian feet (1975) \n.. versionadded:: 3.40"
QgsUnitTypes.FeetUSSurvey = Qgis.DistanceUnit.FeetUSSurvey
QgsUnitTypes.FeetUSSurvey.is_monkey_patched = True
QgsUnitTypes.FeetUSSurvey.__doc__ = "US Survey feet \n.. versionadded:: 3.40"
QgsUnitTypes.LinksInternational = Qgis.DistanceUnit.LinksInternational
QgsUnitTypes.LinksInternational.is_monkey_patched = True
QgsUnitTypes.LinksInternational.__doc__ = "International links \n.. versionadded:: 3.40"
QgsUnitTypes.LinksBritishBenoit1895A = Qgis.DistanceUnit.LinksBritishBenoit1895A
QgsUnitTypes.LinksBritishBenoit1895A.is_monkey_patched = True
QgsUnitTypes.LinksBritishBenoit1895A.__doc__ = "British links (Benoit 1895 A) \n.. versionadded:: 3.40"
QgsUnitTypes.LinksBritishBenoit1895B = Qgis.DistanceUnit.LinksBritishBenoit1895B
QgsUnitTypes.LinksBritishBenoit1895B.is_monkey_patched = True
QgsUnitTypes.LinksBritishBenoit1895B.__doc__ = "British links (Benoit 1895 B) \n.. versionadded:: 3.40"
QgsUnitTypes.LinksBritishSears1922Truncated = Qgis.DistanceUnit.LinksBritishSears1922Truncated
QgsUnitTypes.LinksBritishSears1922Truncated.is_monkey_patched = True
QgsUnitTypes.LinksBritishSears1922Truncated.__doc__ = "British links (Sears 1922 truncated) \n.. versionadded:: 3.40"
QgsUnitTypes.LinksBritishSears1922 = Qgis.DistanceUnit.LinksBritishSears1922
QgsUnitTypes.LinksBritishSears1922.is_monkey_patched = True
QgsUnitTypes.LinksBritishSears1922.__doc__ = "British links (Sears 1922) \n.. versionadded:: 3.40"
QgsUnitTypes.LinksClarkes = Qgis.DistanceUnit.LinksClarkes
QgsUnitTypes.LinksClarkes.is_monkey_patched = True
QgsUnitTypes.LinksClarkes.__doc__ = "Clarke's links \n.. versionadded:: 3.40"
QgsUnitTypes.LinksUSSurvey = Qgis.DistanceUnit.LinksUSSurvey
QgsUnitTypes.LinksUSSurvey.is_monkey_patched = True
QgsUnitTypes.LinksUSSurvey.__doc__ = "US Survey links \n.. versionadded:: 3.40"
QgsUnitTypes.YardsBritishBenoit1895A = Qgis.DistanceUnit.YardsBritishBenoit1895A
QgsUnitTypes.YardsBritishBenoit1895A.is_monkey_patched = True
QgsUnitTypes.YardsBritishBenoit1895A.__doc__ = "British yards (Benoit 1895 A) \n.. versionadded:: 3.40"
QgsUnitTypes.YardsBritishBenoit1895B = Qgis.DistanceUnit.YardsBritishBenoit1895B
QgsUnitTypes.YardsBritishBenoit1895B.is_monkey_patched = True
QgsUnitTypes.YardsBritishBenoit1895B.__doc__ = "British yards (Benoit 1895 B) \n.. versionadded:: 3.40"
QgsUnitTypes.YardsBritishSears1922Truncated = Qgis.DistanceUnit.YardsBritishSears1922Truncated
QgsUnitTypes.YardsBritishSears1922Truncated.is_monkey_patched = True
QgsUnitTypes.YardsBritishSears1922Truncated.__doc__ = "British yards (Sears 1922 truncated) \n.. versionadded:: 3.40"
QgsUnitTypes.YardsBritishSears1922 = Qgis.DistanceUnit.YardsBritishSears1922
QgsUnitTypes.YardsBritishSears1922.is_monkey_patched = True
QgsUnitTypes.YardsBritishSears1922.__doc__ = "British yards (Sears 1922) \n.. versionadded:: 3.40"
QgsUnitTypes.YardsClarkes = Qgis.DistanceUnit.YardsClarkes
QgsUnitTypes.YardsClarkes.is_monkey_patched = True
QgsUnitTypes.YardsClarkes.__doc__ = "Clarke's yards \n.. versionadded:: 3.40"
QgsUnitTypes.YardsIndian = Qgis.DistanceUnit.YardsIndian
QgsUnitTypes.YardsIndian.is_monkey_patched = True
QgsUnitTypes.YardsIndian.__doc__ = "Indian yards \n.. versionadded:: 3.40"
QgsUnitTypes.YardsIndian1937 = Qgis.DistanceUnit.YardsIndian1937
QgsUnitTypes.YardsIndian1937.is_monkey_patched = True
QgsUnitTypes.YardsIndian1937.__doc__ = "Indian yards (1937) \n.. versionadded:: 3.40"
QgsUnitTypes.YardsIndian1962 = Qgis.DistanceUnit.YardsIndian1962
QgsUnitTypes.YardsIndian1962.is_monkey_patched = True
QgsUnitTypes.YardsIndian1962.__doc__ = "Indian yards (1962) \n.. versionadded:: 3.40"
QgsUnitTypes.YardsIndian1975 = Qgis.DistanceUnit.YardsIndian1975
QgsUnitTypes.YardsIndian1975.is_monkey_patched = True
QgsUnitTypes.YardsIndian1975.__doc__ = "Indian yards (1975) \n.. versionadded:: 3.40"
QgsUnitTypes.MilesUSSurvey = Qgis.DistanceUnit.MilesUSSurvey
QgsUnitTypes.MilesUSSurvey.is_monkey_patched = True
QgsUnitTypes.MilesUSSurvey.__doc__ = "US Survey miles \n.. versionadded:: 3.40"
QgsUnitTypes.Fathoms = Qgis.DistanceUnit.Fathoms
QgsUnitTypes.Fathoms.is_monkey_patched = True
QgsUnitTypes.Fathoms.__doc__ = "Fathoms \n.. versionadded:: 3.40"
QgsUnitTypes.MetersGermanLegal = Qgis.DistanceUnit.MetersGermanLegal
QgsUnitTypes.MetersGermanLegal.is_monkey_patched = True
QgsUnitTypes.MetersGermanLegal.__doc__ = "German legal meter \n.. versionadded:: 3.40"
QgsUnitTypes.DistanceUnknownUnit = Qgis.DistanceUnit.Unknown
QgsUnitTypes.DistanceUnit.DistanceUnknownUnit = Qgis.DistanceUnit.Unknown
QgsUnitTypes.DistanceUnknownUnit.is_monkey_patched = True
QgsUnitTypes.DistanceUnknownUnit.__doc__ = "Unknown distance unit"
Qgis.DistanceUnit.__doc__ = """Units of distance

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.DistanceUnit.

.. versionadded:: 3.30

* ``Meters``: Meters

  Available as ``QgsUnitTypes.DistanceMeters`` in older QGIS releases.

* ``Kilometers``: Kilometers

  Available as ``QgsUnitTypes.DistanceKilometers`` in older QGIS releases.

* ``Feet``: Imperial feet

  Available as ``QgsUnitTypes.DistanceFeet`` in older QGIS releases.

* ``NauticalMiles``: Nautical miles

  Available as ``QgsUnitTypes.DistanceNauticalMiles`` in older QGIS releases.

* ``Yards``: Imperial yards

  Available as ``QgsUnitTypes.DistanceYards`` in older QGIS releases.

* ``Miles``: Terrestrial miles

  Available as ``QgsUnitTypes.DistanceMiles`` in older QGIS releases.

* ``Degrees``: Degrees, for planar geographic CRS distance measurements

  Available as ``QgsUnitTypes.DistanceDegrees`` in older QGIS releases.

* ``Centimeters``: Centimeters

  Available as ``QgsUnitTypes.DistanceCentimeters`` in older QGIS releases.

* ``Millimeters``: Millimeters

  Available as ``QgsUnitTypes.DistanceMillimeters`` in older QGIS releases.

* ``Inches``: Inches

  .. versionadded:: 3.32

* ``ChainsInternational``: International chains

  .. versionadded:: 3.40

* ``ChainsBritishBenoit1895A``: British chains (Benoit 1895 A)

  .. versionadded:: 3.40

* ``ChainsBritishBenoit1895B``: British chains (Benoit 1895 B)

  .. versionadded:: 3.40

* ``ChainsBritishSears1922Truncated``: British chains (Sears 1922 truncated)

  .. versionadded:: 3.40

* ``ChainsBritishSears1922``: British chains (Sears 1922)

  .. versionadded:: 3.40

* ``ChainsClarkes``: Clarke's chains

  .. versionadded:: 3.40

* ``ChainsUSSurvey``: US Survey chains

  .. versionadded:: 3.40

* ``FeetBritish1865``: British feet (1865)

  .. versionadded:: 3.40

* ``FeetBritish1936``: British feet (1936)

  .. versionadded:: 3.40

* ``FeetBritishBenoit1895A``: British feet (Benoit 1895 A)

  .. versionadded:: 3.40

* ``FeetBritishBenoit1895B``: British feet (Benoit 1895 B)

  .. versionadded:: 3.40

* ``FeetBritishSears1922Truncated``: British feet (Sears 1922 truncated)

  .. versionadded:: 3.40

* ``FeetBritishSears1922``: British feet (Sears 1922)

  .. versionadded:: 3.40

* ``FeetClarkes``: Clarke's feet

  .. versionadded:: 3.40

* ``FeetGoldCoast``: Gold Coast feet

  .. versionadded:: 3.40

* ``FeetIndian``: Indian (geodetic) feet

  .. versionadded:: 3.40

* ``FeetIndian1937``: Indian feet (1937)

  .. versionadded:: 3.40

* ``FeetIndian1962``: Indian feet (1962)

  .. versionadded:: 3.40

* ``FeetIndian1975``: Indian feet (1975)

  .. versionadded:: 3.40

* ``FeetUSSurvey``: US Survey feet

  .. versionadded:: 3.40

* ``LinksInternational``: International links

  .. versionadded:: 3.40

* ``LinksBritishBenoit1895A``: British links (Benoit 1895 A)

  .. versionadded:: 3.40

* ``LinksBritishBenoit1895B``: British links (Benoit 1895 B)

  .. versionadded:: 3.40

* ``LinksBritishSears1922Truncated``: British links (Sears 1922 truncated)

  .. versionadded:: 3.40

* ``LinksBritishSears1922``: British links (Sears 1922)

  .. versionadded:: 3.40

* ``LinksClarkes``: Clarke's links

  .. versionadded:: 3.40

* ``LinksUSSurvey``: US Survey links

  .. versionadded:: 3.40

* ``YardsBritishBenoit1895A``: British yards (Benoit 1895 A)

  .. versionadded:: 3.40

* ``YardsBritishBenoit1895B``: British yards (Benoit 1895 B)

  .. versionadded:: 3.40

* ``YardsBritishSears1922Truncated``: British yards (Sears 1922 truncated)

  .. versionadded:: 3.40

* ``YardsBritishSears1922``: British yards (Sears 1922)

  .. versionadded:: 3.40

* ``YardsClarkes``: Clarke's yards

  .. versionadded:: 3.40

* ``YardsIndian``: Indian yards

  .. versionadded:: 3.40

* ``YardsIndian1937``: Indian yards (1937)

  .. versionadded:: 3.40

* ``YardsIndian1962``: Indian yards (1962)

  .. versionadded:: 3.40

* ``YardsIndian1975``: Indian yards (1975)

  .. versionadded:: 3.40

* ``MilesUSSurvey``: US Survey miles

  .. versionadded:: 3.40

* ``Fathoms``: Fathoms

  .. versionadded:: 3.40

* ``MetersGermanLegal``: German legal meter

  .. versionadded:: 3.40

* ``Unknown``: Unknown distance unit

  Available as ``QgsUnitTypes.DistanceUnknownUnit`` in older QGIS releases.


"""
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
Qgis.DistanceUnitType.__doc__ = """Types of distance units

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.DistanceUnitType.

.. versionadded:: 3.30

* ``Standard``: Unit is a standard measurement unit
* ``Geographic``: Unit is a geographic (e.g., degree based) unit
* ``Unknown``: Unknown unit type

  Available as ``QgsUnitTypes.UnknownType`` in older QGIS releases.


"""
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
QgsUnitTypes.SquareInches.__doc__ = "Square inches \n.. versionadded:: 3.32"
QgsUnitTypes.AreaUnknownUnit = Qgis.AreaUnit.Unknown
QgsUnitTypes.AreaUnit.AreaUnknownUnit = Qgis.AreaUnit.Unknown
QgsUnitTypes.AreaUnknownUnit.is_monkey_patched = True
QgsUnitTypes.AreaUnknownUnit.__doc__ = "Unknown areal unit"
Qgis.AreaUnit.__doc__ = """Units of area

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.AreaUnit.

.. versionadded:: 3.30

* ``SquareMeters``: Square meters

  Available as ``QgsUnitTypes.AreaSquareMeters`` in older QGIS releases.

* ``SquareKilometers``: Square kilometers

  Available as ``QgsUnitTypes.AreaSquareKilometers`` in older QGIS releases.

* ``SquareFeet``: Square feet

  Available as ``QgsUnitTypes.AreaSquareFeet`` in older QGIS releases.

* ``SquareYards``: Square yards

  Available as ``QgsUnitTypes.AreaSquareYards`` in older QGIS releases.

* ``SquareMiles``: Square miles

  Available as ``QgsUnitTypes.AreaSquareMiles`` in older QGIS releases.

* ``Hectares``: Hectares

  Available as ``QgsUnitTypes.AreaHectares`` in older QGIS releases.

* ``Acres``: Acres

  Available as ``QgsUnitTypes.AreaAcres`` in older QGIS releases.

* ``SquareNauticalMiles``: Square nautical miles

  Available as ``QgsUnitTypes.AreaSquareNauticalMiles`` in older QGIS releases.

* ``SquareDegrees``: Square degrees, for planar geographic CRS area measurements

  Available as ``QgsUnitTypes.AreaSquareDegrees`` in older QGIS releases.

* ``SquareCentimeters``: Square centimeters

  Available as ``QgsUnitTypes.AreaSquareCentimeters`` in older QGIS releases.

* ``SquareMillimeters``: Square millimeters

  Available as ``QgsUnitTypes.AreaSquareMillimeters`` in older QGIS releases.

* ``SquareInches``: Square inches

  .. versionadded:: 3.32

* ``Unknown``: Unknown areal unit

  Available as ``QgsUnitTypes.AreaUnknownUnit`` in older QGIS releases.


"""
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
Qgis.VolumeUnit.__doc__ = """Units of volume.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.VolumeUnit.

.. versionadded:: 3.30

* ``CubicMeters``: Cubic meters

  Available as ``QgsUnitTypes.VolumeCubicMeters`` in older QGIS releases.

* ``CubicFeet``: Cubic feet

  Available as ``QgsUnitTypes.VolumeCubicFeet`` in older QGIS releases.

* ``CubicYards``: Cubic yards

  Available as ``QgsUnitTypes.VolumeCubicYards`` in older QGIS releases.

* ``Barrel``: Barrels

  Available as ``QgsUnitTypes.VolumeBarrel`` in older QGIS releases.

* ``CubicDecimeter``: Cubic decimeters

  Available as ``QgsUnitTypes.VolumeCubicDecimeter`` in older QGIS releases.

* ``Liters``: Litres

  Available as ``QgsUnitTypes.VolumeLiters`` in older QGIS releases.

* ``GallonUS``: US Gallons

  Available as ``QgsUnitTypes.VolumeGallonUS`` in older QGIS releases.

* ``CubicInch``: Cubic inches

  Available as ``QgsUnitTypes.VolumeCubicInch`` in older QGIS releases.

* ``CubicCentimeter``: Cubic Centimeters

  Available as ``QgsUnitTypes.VolumeCubicCentimeter`` in older QGIS releases.

* ``CubicDegrees``: Cubic degrees, for planar geographic CRS volume measurements

  Available as ``QgsUnitTypes.VolumeCubicDegrees`` in older QGIS releases.

* ``Unknown``: Unknown volume unit

  Available as ``QgsUnitTypes.VolumeUnknownUnit`` in older QGIS releases.


"""
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
Qgis.AngleUnit.__doc__ = """Units of angles.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.AngleUnit.

.. versionadded:: 3.30

* ``Degrees``: Degrees

  Available as ``QgsUnitTypes.AngleDegrees`` in older QGIS releases.

* ``Radians``: Square kilometers

  Available as ``QgsUnitTypes.AngleRadians`` in older QGIS releases.

* ``Gon``: Gon/gradian

  Available as ``QgsUnitTypes.AngleGon`` in older QGIS releases.

* ``MinutesOfArc``: Minutes of arc

  Available as ``QgsUnitTypes.AngleMinutesOfArc`` in older QGIS releases.

* ``SecondsOfArc``: Seconds of arc

  Available as ``QgsUnitTypes.AngleSecondsOfArc`` in older QGIS releases.

* ``Turn``: Turn/revolutions

  Available as ``QgsUnitTypes.AngleTurn`` in older QGIS releases.

* ``MilliradiansSI``: Angular milliradians (SI definition, 1/1000 of radian)

  Available as ``QgsUnitTypes.AngleMilliradiansSI`` in older QGIS releases.

* ``MilNATO``: Angular mil (NATO definition, 6400 mil = 2PI radians)

  Available as ``QgsUnitTypes.AngleMilNATO`` in older QGIS releases.

* ``Unknown``: Unknown angle unit

  Available as ``QgsUnitTypes.AngleUnknownUnit`` in older QGIS releases.


"""
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
QgsUnitTypes.TemporalIrregularStep.__doc__ = "Special 'irregular step' time unit, used for temporal data which uses irregular, non-real-world unit steps \n.. versionadded:: 3.20"
QgsUnitTypes.TemporalUnknownUnit = Qgis.TemporalUnit.Unknown
QgsUnitTypes.TemporalUnit.TemporalUnknownUnit = Qgis.TemporalUnit.Unknown
QgsUnitTypes.TemporalUnknownUnit.is_monkey_patched = True
QgsUnitTypes.TemporalUnknownUnit.__doc__ = "Unknown time unit"
Qgis.TemporalUnit.__doc__ = """Temporal units.

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.TemporalUnit.

.. versionadded:: 3.30

* ``Milliseconds``: Milliseconds

  Available as ``QgsUnitTypes.TemporalMilliseconds`` in older QGIS releases.

* ``Seconds``: Seconds

  Available as ``QgsUnitTypes.TemporalSeconds`` in older QGIS releases.

* ``Minutes``: Minutes

  Available as ``QgsUnitTypes.TemporalMinutes`` in older QGIS releases.

* ``Hours``: Hours

  Available as ``QgsUnitTypes.TemporalHours`` in older QGIS releases.

* ``Days``: Days

  Available as ``QgsUnitTypes.TemporalDays`` in older QGIS releases.

* ``Weeks``: Weeks

  Available as ``QgsUnitTypes.TemporalWeeks`` in older QGIS releases.

* ``Months``: Months

  Available as ``QgsUnitTypes.TemporalMonths`` in older QGIS releases.

* ``Years``: Years

  Available as ``QgsUnitTypes.TemporalYears`` in older QGIS releases.

* ``Decades``: Decades

  Available as ``QgsUnitTypes.TemporalDecades`` in older QGIS releases.

* ``Centuries``: Centuries

  Available as ``QgsUnitTypes.TemporalCenturies`` in older QGIS releases.

* ``IrregularStep``: Special 'irregular step' time unit, used for temporal data which uses irregular, non-real-world unit steps

  .. versionadded:: 3.20


  Available as ``QgsUnitTypes.TemporalIrregularStep`` in older QGIS releases.

* ``Unknown``: Unknown time unit

  Available as ``QgsUnitTypes.TemporalUnknownUnit`` in older QGIS releases.


"""
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
Qgis.RenderUnit.__doc__ = """Rendering size units

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.RenderUnit.

.. versionadded:: 3.30

* ``Millimeters``: Millimeters

  Available as ``QgsUnitTypes.RenderMillimeters`` in older QGIS releases.

* ``MapUnits``: Map units

  Available as ``QgsUnitTypes.RenderMapUnits`` in older QGIS releases.

* ``Pixels``: Pixels

  Available as ``QgsUnitTypes.RenderPixels`` in older QGIS releases.

* ``Percentage``: Percentage of another measurement (e.g., canvas size, feature size)

  Available as ``QgsUnitTypes.RenderPercentage`` in older QGIS releases.

* ``Points``: Points (e.g., for font sizes)

  Available as ``QgsUnitTypes.RenderPoints`` in older QGIS releases.

* ``Inches``: Inches

  Available as ``QgsUnitTypes.RenderInches`` in older QGIS releases.

* ``Unknown``: Mixed or unknown units

  Available as ``QgsUnitTypes.RenderUnknownUnit`` in older QGIS releases.

* ``MetersInMapUnits``: Meters value as Map units

  Available as ``QgsUnitTypes.RenderMetersInMapUnits`` in older QGIS releases.


"""
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
Qgis.LayoutUnit.__doc__ = """Layout measurement units

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.LayoutUnit.

.. versionadded:: 3.30

* ``Millimeters``: Millimeters

  Available as ``QgsUnitTypes.LayoutMillimeters`` in older QGIS releases.

* ``Centimeters``: Centimeters

  Available as ``QgsUnitTypes.LayoutCentimeters`` in older QGIS releases.

* ``Meters``: Meters

  Available as ``QgsUnitTypes.LayoutMeters`` in older QGIS releases.

* ``Inches``: Inches

  Available as ``QgsUnitTypes.LayoutInches`` in older QGIS releases.

* ``Feet``: Feet

  Available as ``QgsUnitTypes.LayoutFeet`` in older QGIS releases.

* ``Points``: Typographic points

  Available as ``QgsUnitTypes.LayoutPoints`` in older QGIS releases.

* ``Picas``: Typographic picas

  Available as ``QgsUnitTypes.LayoutPicas`` in older QGIS releases.

* ``Pixels``: Pixels

  Available as ``QgsUnitTypes.LayoutPixels`` in older QGIS releases.


"""
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
Qgis.LayoutUnitType.__doc__ = """Types of layout units

.. note::

   Prior to QGIS 3.30 this was available as :py:class:`QgsUnitTypes`.LayoutUnitType.

.. versionadded:: 3.30

* ``PaperUnits``: Unit is a paper based measurement unit

  Available as ``QgsUnitTypes.LayoutPaperUnits`` in older QGIS releases.

* ``ScreenUnits``: Unit is a screen based measurement unit

  Available as ``QgsUnitTypes.LayoutScreenUnits`` in older QGIS releases.


"""
# --
Qgis.LayoutUnitType.baseClass = Qgis
QgsLayoutItemPicture.Format = Qgis.PictureFormat
# monkey patching scoped based enum
QgsLayoutItemPicture.FormatSVG = Qgis.PictureFormat.SVG
QgsLayoutItemPicture.Format.FormatSVG = Qgis.PictureFormat.SVG
QgsLayoutItemPicture.FormatSVG.is_monkey_patched = True
QgsLayoutItemPicture.FormatSVG.__doc__ = "SVG image"
QgsLayoutItemPicture.FormatRaster = Qgis.PictureFormat.Raster
QgsLayoutItemPicture.Format.FormatRaster = Qgis.PictureFormat.Raster
QgsLayoutItemPicture.FormatRaster.is_monkey_patched = True
QgsLayoutItemPicture.FormatRaster.__doc__ = "Raster image"
QgsLayoutItemPicture.FormatUnknown = Qgis.PictureFormat.Unknown
QgsLayoutItemPicture.Format.FormatUnknown = Qgis.PictureFormat.Unknown
QgsLayoutItemPicture.FormatUnknown.is_monkey_patched = True
QgsLayoutItemPicture.FormatUnknown.__doc__ = "Invalid or unknown image type"
Qgis.PictureFormat.__doc__ = """Picture formats.

.. note::

   Prior to QGIS 3.40 this was available as :py:class:`QgsLayoutItemPicture`.Format.

.. versionadded:: 3.40

* ``SVG``: SVG image

  Available as ``QgsLayoutItemPicture.FormatSVG`` in older QGIS releases.

* ``Raster``: Raster image

  Available as ``QgsLayoutItemPicture.FormatRaster`` in older QGIS releases.

* ``Unknown``: Invalid or unknown image type

  Available as ``QgsLayoutItemPicture.FormatUnknown`` in older QGIS releases.


"""
# --
Qgis.PictureFormat.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ScaleCalculationMethod.HorizontalTop.__doc__ = "Calculate horizontally, across top of map"
Qgis.ScaleCalculationMethod.HorizontalMiddle.__doc__ = "Calculate horizontally, across midle of map"
Qgis.ScaleCalculationMethod.HorizontalBottom.__doc__ = "Calculate horizontally, across bottom of map"
Qgis.ScaleCalculationMethod.HorizontalAverage.__doc__ = "Calculate horizontally, using the average of the top, middle and bottom scales"
Qgis.ScaleCalculationMethod.__doc__ = """Scale calculation logic.

.. versionadded:: 3.40

* ``HorizontalTop``: Calculate horizontally, across top of map
* ``HorizontalMiddle``: Calculate horizontally, across midle of map
* ``HorizontalBottom``: Calculate horizontally, across bottom of map
* ``HorizontalAverage``: Calculate horizontally, using the average of the top, middle and bottom scales

"""
# --
Qgis.ScaleCalculationMethod.baseClass = Qgis
QgsScaleBarSettings.Alignment = Qgis.ScaleBarAlignment
# monkey patching scoped based enum
QgsScaleBarSettings.AlignLeft = Qgis.ScaleBarAlignment.Left
QgsScaleBarSettings.Alignment.AlignLeft = Qgis.ScaleBarAlignment.Left
QgsScaleBarSettings.AlignLeft.is_monkey_patched = True
QgsScaleBarSettings.AlignLeft.__doc__ = "Left aligned"
QgsScaleBarSettings.AlignMiddle = Qgis.ScaleBarAlignment.Middle
QgsScaleBarSettings.Alignment.AlignMiddle = Qgis.ScaleBarAlignment.Middle
QgsScaleBarSettings.AlignMiddle.is_monkey_patched = True
QgsScaleBarSettings.AlignMiddle.__doc__ = "Center aligned"
QgsScaleBarSettings.AlignRight = Qgis.ScaleBarAlignment.Right
QgsScaleBarSettings.Alignment.AlignRight = Qgis.ScaleBarAlignment.Right
QgsScaleBarSettings.AlignRight.is_monkey_patched = True
QgsScaleBarSettings.AlignRight.__doc__ = "Right aligned"
Qgis.ScaleBarAlignment.__doc__ = """Scalebar alignment.

.. note::

   Prior to QGIS 3.40 this was available as :py:class:`QgsScaleBarSettings`.Alignment.

.. versionadded:: 3.40

* ``Left``: Left aligned

  Available as ``QgsScaleBarSettings.AlignLeft`` in older QGIS releases.

* ``Middle``: Center aligned

  Available as ``QgsScaleBarSettings.AlignMiddle`` in older QGIS releases.

* ``Right``: Right aligned

  Available as ``QgsScaleBarSettings.AlignRight`` in older QGIS releases.


"""
# --
Qgis.ScaleBarAlignment.baseClass = Qgis
QgsScaleBarSettings.SegmentSizeMode = Qgis.ScaleBarSegmentSizeMode
# monkey patching scoped based enum
QgsScaleBarSettings.SegmentSizeFixed = Qgis.ScaleBarSegmentSizeMode.Fixed
QgsScaleBarSettings.SegmentSizeMode.SegmentSizeFixed = Qgis.ScaleBarSegmentSizeMode.Fixed
QgsScaleBarSettings.SegmentSizeFixed.is_monkey_patched = True
QgsScaleBarSettings.SegmentSizeFixed.__doc__ = "Scale bar segment size is fixed to a map unit"
QgsScaleBarSettings.SegmentSizeFitWidth = Qgis.ScaleBarSegmentSizeMode.FitWidth
QgsScaleBarSettings.SegmentSizeMode.SegmentSizeFitWidth = Qgis.ScaleBarSegmentSizeMode.FitWidth
QgsScaleBarSettings.SegmentSizeFitWidth.is_monkey_patched = True
QgsScaleBarSettings.SegmentSizeFitWidth.__doc__ = "Scale bar segment size is calculated to fit a size range"
Qgis.ScaleBarSegmentSizeMode.__doc__ = """Modes for setting size for scale bar segments.

.. note::

   Prior to QGIS 3.40 this was available as :py:class:`QgsScaleBarSettings`.SegmentSizeMode.

.. versionadded:: 3.40

* ``Fixed``: Scale bar segment size is fixed to a map unit

  Available as ``QgsScaleBarSettings.SegmentSizeFixed`` in older QGIS releases.

* ``FitWidth``: Scale bar segment size is calculated to fit a size range

  Available as ``QgsScaleBarSettings.SegmentSizeFitWidth`` in older QGIS releases.


"""
# --
Qgis.ScaleBarSegmentSizeMode.baseClass = Qgis
QgsScaleBarSettings.LabelVerticalPlacement = Qgis.ScaleBarDistanceLabelVerticalPlacement
# monkey patching scoped based enum
QgsScaleBarSettings.LabelAboveSegment = Qgis.ScaleBarDistanceLabelVerticalPlacement.AboveSegment
QgsScaleBarSettings.LabelVerticalPlacement.LabelAboveSegment = Qgis.ScaleBarDistanceLabelVerticalPlacement.AboveSegment
QgsScaleBarSettings.LabelAboveSegment.is_monkey_patched = True
QgsScaleBarSettings.LabelAboveSegment.__doc__ = "Labels are drawn above the scalebar"
QgsScaleBarSettings.LabelBelowSegment = Qgis.ScaleBarDistanceLabelVerticalPlacement.BelowSegment
QgsScaleBarSettings.LabelVerticalPlacement.LabelBelowSegment = Qgis.ScaleBarDistanceLabelVerticalPlacement.BelowSegment
QgsScaleBarSettings.LabelBelowSegment.is_monkey_patched = True
QgsScaleBarSettings.LabelBelowSegment.__doc__ = "Labels are drawn below the scalebar"
Qgis.ScaleBarDistanceLabelVerticalPlacement.__doc__ = """Scale bar distance label vertical placement.

.. note::

   Prior to QGIS 3.40 this was available as :py:class:`QgsScaleBarSettings`.LabelVerticalPlacement.

.. versionadded:: 3.40

* ``AboveSegment``: Labels are drawn above the scalebar

  Available as ``QgsScaleBarSettings.LabelAboveSegment`` in older QGIS releases.

* ``BelowSegment``: Labels are drawn below the scalebar

  Available as ``QgsScaleBarSettings.LabelBelowSegment`` in older QGIS releases.


"""
# --
Qgis.ScaleBarDistanceLabelVerticalPlacement.baseClass = Qgis
QgsScaleBarSettings.LabelHorizontalPlacement = Qgis.ScaleBarDistanceLabelHorizontalPlacement
# monkey patching scoped based enum
QgsScaleBarSettings.LabelCenteredEdge = Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredEdge
QgsScaleBarSettings.LabelHorizontalPlacement.LabelCenteredEdge = Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredEdge
QgsScaleBarSettings.LabelCenteredEdge.is_monkey_patched = True
QgsScaleBarSettings.LabelCenteredEdge.__doc__ = "Labels are drawn centered relative to segment's edge"
QgsScaleBarSettings.LabelCenteredSegment = Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
QgsScaleBarSettings.LabelHorizontalPlacement.LabelCenteredSegment = Qgis.ScaleBarDistanceLabelHorizontalPlacement.CenteredSegment
QgsScaleBarSettings.LabelCenteredSegment.is_monkey_patched = True
QgsScaleBarSettings.LabelCenteredSegment.__doc__ = "Labels are drawn centered relative to segment"
Qgis.ScaleBarDistanceLabelHorizontalPlacement.__doc__ = """Scale bar distance label horizontal placement.

.. note::

   Prior to QGIS 3.40 this was available as :py:class:`QgsScaleBarSettings`.LabelHorizontalPlacement.

.. versionadded:: 3.40

* ``CenteredEdge``: Labels are drawn centered relative to segment's edge

  Available as ``QgsScaleBarSettings.LabelCenteredEdge`` in older QGIS releases.

* ``CenteredSegment``: Labels are drawn centered relative to segment

  Available as ``QgsScaleBarSettings.LabelCenteredSegment`` in older QGIS releases.


"""
# --
Qgis.ScaleBarDistanceLabelHorizontalPlacement.baseClass = Qgis
# monkey patching scoped based enum
Qgis.InputControllerType.Map2D.__doc__ = "2D map controller"
Qgis.InputControllerType.Map3D.__doc__ = "3D map controller"
Qgis.InputControllerType.__doc__ = """Input controller types.

.. versionadded:: 3.34

* ``Map2D``: 2D map controller
* ``Map3D``: 3D map controller

"""
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
Qgis.PostgresRelKind.__doc__ = """Postgres database relkind options.

.. versionadded:: 3.32

* ``NotSet``: Not set
* ``Unknown``: Unknown
* ``OrdinaryTable``: Ordinary table
* ``Index``: Index
* ``Sequence``: Sequence
* ``View``: View
* ``MaterializedView``: Materialized view
* ``CompositeType``: Composition type
* ``ToastTable``: TOAST table
* ``ForeignTable``: Foreign table
* ``PartitionedTable``: Partitioned table

"""
# --
Qgis.PostgresRelKind.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DatabaseProviderConnectionCapability2.SetFieldComment.__doc__ = "Can set comments for fields via setFieldComment()"
Qgis.DatabaseProviderConnectionCapability2.SetFieldAlias.__doc__ = "Can set aliases for fields via setFieldAlias()"
Qgis.DatabaseProviderConnectionCapability2.__doc__ = """The Capability enum represents the extended operations supported by the connection.

.. versionadded:: 3.32

* ``SetFieldComment``: Can set comments for fields via setFieldComment()
* ``SetFieldAlias``: Can set aliases for fields via setFieldAlias()

"""
# --
Qgis.DatabaseProviderConnectionCapability2.baseClass = Qgis
Qgis.DatabaseProviderConnectionCapabilities2.baseClass = Qgis
DatabaseProviderConnectionCapabilities2 = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.ProviderStyleStorageCapability.SaveToDatabase.__doc__ = ""
Qgis.ProviderStyleStorageCapability.LoadFromDatabase.__doc__ = ""
Qgis.ProviderStyleStorageCapability.DeleteFromDatabase.__doc__ = ""
Qgis.ProviderStyleStorageCapability.__doc__ = """The StorageCapability enum represents the style storage operations supported by the provider.

.. versionadded:: 3.34

* ``SaveToDatabase``: 
* ``LoadFromDatabase``: 
* ``DeleteFromDatabase``: 

"""
# --
Qgis.ProviderStyleStorageCapability.baseClass = Qgis
Qgis.ProviderStyleStorageCapabilities.baseClass = Qgis
ProviderStyleStorageCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.UserProfileSelectionPolicy.LastProfile.__doc__ = "Open the last closed profile (only mode supported prior to QGIS 3.32)"
Qgis.UserProfileSelectionPolicy.DefaultProfile.__doc__ = "Open a specific profile"
Qgis.UserProfileSelectionPolicy.AskUser.__doc__ = "Let the user choose which profile to open"
Qgis.UserProfileSelectionPolicy.__doc__ = """User profile selection policy.

.. versionadded:: 3.32

* ``LastProfile``: Open the last closed profile (only mode supported prior to QGIS 3.32)
* ``DefaultProfile``: Open a specific profile
* ``AskUser``: Let the user choose which profile to open

"""
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
QgsAttributeEditorElement.AeTypeAction.__doc__ = "A layer action element \n.. versionadded:: 3.22"
QgsAttributeEditorElement.AeTypeTextElement = Qgis.AttributeEditorType.TextElement
QgsAttributeEditorElement.AttributeEditorType.AeTypeTextElement = Qgis.AttributeEditorType.TextElement
QgsAttributeEditorElement.AeTypeTextElement.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeTextElement.__doc__ = "A text element \n.. versionadded:: 3.30"
QgsAttributeEditorElement.AeTypeSpacerElement = Qgis.AttributeEditorType.SpacerElement
QgsAttributeEditorElement.AttributeEditorType.AeTypeSpacerElement = Qgis.AttributeEditorType.SpacerElement
QgsAttributeEditorElement.AeTypeSpacerElement.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeSpacerElement.__doc__ = "A spacer element \n.. versionadded:: 3.30"
QgsAttributeEditorElement.AeTypeInvalid = Qgis.AttributeEditorType.Invalid
QgsAttributeEditorElement.AttributeEditorType.AeTypeInvalid = Qgis.AttributeEditorType.Invalid
QgsAttributeEditorElement.AeTypeInvalid.is_monkey_patched = True
QgsAttributeEditorElement.AeTypeInvalid.__doc__ = "Invalid"
Qgis.AttributeEditorType.__doc__ = """Attribute editor types.

.. note::

   Prior to QGIS 3.32 this was available as :py:class:`QgsAttributeEditorElement`.AttributeEditorType.

.. versionadded:: 3.32

* ``Container``: A container

  Available as ``QgsAttributeEditorElement.AeTypeContainer`` in older QGIS releases.

* ``Field``: A field

  Available as ``QgsAttributeEditorElement.AeTypeField`` in older QGIS releases.

* ``Relation``: A relation

  Available as ``QgsAttributeEditorElement.AeTypeRelation`` in older QGIS releases.

* ``QmlElement``: A QML element

  Available as ``QgsAttributeEditorElement.AeTypeQmlElement`` in older QGIS releases.

* ``HtmlElement``: A HTML element

  Available as ``QgsAttributeEditorElement.AeTypeHtmlElement`` in older QGIS releases.

* ``Action``: A layer action element

  .. versionadded:: 3.22


  Available as ``QgsAttributeEditorElement.AeTypeAction`` in older QGIS releases.

* ``TextElement``: A text element

  .. versionadded:: 3.30


  Available as ``QgsAttributeEditorElement.AeTypeTextElement`` in older QGIS releases.

* ``SpacerElement``: A spacer element

  .. versionadded:: 3.30


  Available as ``QgsAttributeEditorElement.AeTypeSpacerElement`` in older QGIS releases.

* ``Invalid``: Invalid

  Available as ``QgsAttributeEditorElement.AeTypeInvalid`` in older QGIS releases.


"""
# --
Qgis.AttributeEditorType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.AttributeEditorContainerType.GroupBox.__doc__ = "A group box"
Qgis.AttributeEditorContainerType.Tab.__doc__ = "A tab widget"
Qgis.AttributeEditorContainerType.Row.__doc__ = "A row of editors (horizontal layout)"
Qgis.AttributeEditorContainerType.__doc__ = """Attribute editor container types.

.. versionadded:: 3.32

* ``GroupBox``: A group box
* ``Tab``: A tab widget
* ``Row``: A row of editors (horizontal layout)

"""
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
Qgis.AttributeFormLayout.__doc__ = """Available form types for layout of the attribute form editor.

.. note::

   Prior to QGIS 3.32 this was available as :py:class:`QgsEditFormConfig`.EditorLayout.

.. versionadded:: 3.32

* ``AutoGenerated``: Autogenerate a simple tabular layout for the form

  Available as ``QgsEditFormConfig.GeneratedLayout`` in older QGIS releases.

* ``DragAndDrop``: \"Drag and drop\" layout. Needs to be configured.

  Available as ``QgsEditFormConfig.TabLayout`` in older QGIS releases.

* ``UiFile``: Load a .ui file for the layout. Needs to be configured.

  Available as ``QgsEditFormConfig.UiFileLayout`` in older QGIS releases.


"""
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
Qgis.AttributeFormSuppression.__doc__ = """Available form types for layout of the attribute form editor.

.. note::

   Prior to QGIS 3.32 this was available as :py:class:`QgsEditFormConfig`.FeatureFormSuppress.

.. versionadded:: 3.32

* ``Default``: Use the application-wide setting.

  Available as ``QgsEditFormConfig.SuppressDefault`` in older QGIS releases.

* ``On``: Always suppress feature form.

  Available as ``QgsEditFormConfig.SuppressOn`` in older QGIS releases.

* ``Off``: Never suppress feature form.

  Available as ``QgsEditFormConfig.SuppressOff`` in older QGIS releases.


"""
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
Qgis.AttributeFormPythonInitCodeSource.__doc__ = """The Python init code source for attribute forms.

.. note::

   Prior to QGIS 3.32 this was available as :py:class:`QgsEditFormConfig`.PythonInitCodeSource.

.. versionadded:: 3.32

* ``NoSource``: Do not use Python code at all

  Available as ``QgsEditFormConfig.CodeSourceNone`` in older QGIS releases.

* ``File``: Load the Python code from an external file

  Available as ``QgsEditFormConfig.CodeSourceFile`` in older QGIS releases.

* ``Dialog``: Use the Python code provided in the dialog

  Available as ``QgsEditFormConfig.CodeSourceDialog`` in older QGIS releases.

* ``Environment``: Use the Python code available in the Python environment

  Available as ``QgsEditFormConfig.CodeSourceEnvironment`` in older QGIS releases.


"""
# --
Qgis.AttributeFormPythonInitCodeSource.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ExpressionType.Qgis.__doc__ = "Native QGIS expression"
Qgis.ExpressionType.PointCloud.__doc__ = "Point cloud expression"
Qgis.ExpressionType.RasterCalculator.__doc__ = "Raster calculator expression \n.. versionadded:: 3.34"
Qgis.ExpressionType.__doc__ = """Expression types

.. versionadded:: 3.32

* ``Qgis``: Native QGIS expression
* ``PointCloud``: Point cloud expression
* ``RasterCalculator``: Raster calculator expression

  .. versionadded:: 3.34


"""
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
Qgis.FeatureSymbologyExport.__doc__ = """Options for exporting features considering their symbology.

.. note::

   Prior to QGIS 3.32 this was available as :py:class:`QgsVectorFileWriter`.SymbologyExport.

.. versionadded:: 3.32

* ``NoSymbology``: Export only data
* ``PerFeature``: Keeps the number of features and export symbology per feature

  Available as ``QgsVectorFileWriter.FeatureSymbology`` in older QGIS releases.

* ``PerSymbolLayer``: Exports one feature per symbol layer (considering symbol levels)

  Available as ``QgsVectorFileWriter.SymbolLayerSymbology`` in older QGIS releases.


"""
# --
Qgis.FeatureSymbologyExport.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VectorTileProviderFlag.AlwaysUseTileMatrixSetFromProvider.__doc__ = "Vector tile layer must always use the tile matrix set from the data provider, and should never store, restore or override the definition of this matrix set."
Qgis.VectorTileProviderFlag.__doc__ = """Flags for vector tile data providers.

.. versionadded:: 3.32

* ``AlwaysUseTileMatrixSetFromProvider``: Vector tile layer must always use the tile matrix set from the data provider, and should never store, restore or override the definition of this matrix set.

"""
# --
Qgis.VectorTileProviderFlag.baseClass = Qgis
Qgis.VectorTileProviderFlags.baseClass = Qgis
VectorTileProviderFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.VectorTileProviderCapability.ReadLayerMetadata.__doc__ = "Provider can read layer metadata from data store. See QgsDataProvider.layerMetadata()"
Qgis.VectorTileProviderCapability.__doc__ = """Enumeration with capabilities that vector tile data providers might implement.

.. versionadded:: 3.32

* ``ReadLayerMetadata``: Provider can read layer metadata from data store. See QgsDataProvider.layerMetadata()

"""
# --
Qgis.VectorTileProviderCapability.baseClass = Qgis
Qgis.VectorTileProviderCapabilities.baseClass = Qgis
VectorTileProviderCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.TileAvailability.Available.__doc__ = "Tile is available within the matrix"
Qgis.TileAvailability.NotAvailable.__doc__ = "Tile is not available within the matrix, e.g. there is no content for the tile"
Qgis.TileAvailability.AvailableNoChildren.__doc__ = "Tile is available within the matrix, and is known to have no children (ie no higher zoom level tiles exist covering this tile's region)"
Qgis.TileAvailability.UseLowerZoomLevelTile.__doc__ = "Tile is not available at the requested zoom level, it should be replaced by a tile from a lower zoom level instead182"
Qgis.TileAvailability.__doc__ = """Possible availability states for a tile within a tile matrix.

.. versionadded:: 3.32

* ``Available``: Tile is available within the matrix
* ``NotAvailable``: Tile is not available within the matrix, e.g. there is no content for the tile
* ``AvailableNoChildren``: Tile is available within the matrix, and is known to have no children (ie no higher zoom level tiles exist covering this tile's region)
* ``UseLowerZoomLevelTile``: Tile is not available at the requested zoom level, it should be replaced by a tile from a lower zoom level instead182

"""
# --
Qgis.TileAvailability.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TiledSceneProviderCapability.ReadLayerMetadata.__doc__ = "Provider can read layer metadata from data store. See QgsDataProvider.layerMetadata()"
Qgis.TiledSceneProviderCapability.__doc__ = """Tiled scene data provider capabilities.

.. versionadded:: 3.34

* ``ReadLayerMetadata``: Provider can read layer metadata from data store. See QgsDataProvider.layerMetadata()

"""
# --
Qgis.TiledSceneProviderCapability.baseClass = Qgis
Qgis.TiledSceneProviderCapabilities.baseClass = Qgis
TiledSceneProviderCapabilities = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.TiledSceneBoundingVolumeType.Region.__doc__ = "Region type"
Qgis.TiledSceneBoundingVolumeType.OrientedBox.__doc__ = "Oriented bounding box (rotated box)"
Qgis.TiledSceneBoundingVolumeType.Sphere.__doc__ = "Sphere"
Qgis.TiledSceneBoundingVolumeType.__doc__ = """Tiled scene bounding volume types.

.. versionadded:: 3.34

* ``Region``: Region type
* ``OrientedBox``: Oriented bounding box (rotated box)
* ``Sphere``: Sphere

"""
# --
Qgis.TiledSceneBoundingVolumeType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TileRefinementProcess.Replacement.__doc__ = "When tile is refined then its children should be used in place of itself."
Qgis.TileRefinementProcess.Additive.__doc__ = "When tile is refined its content should be used alongside its children simultaneously."
Qgis.TileRefinementProcess.__doc__ = """Tiled scene tile refinement processes.

Refinement determines the process by which a lower resolution parent tile
renders when its higher resolution children are selected to be rendered.

.. versionadded:: 3.34

* ``Replacement``: When tile is refined then its children should be used in place of itself.
* ``Additive``: When tile is refined its content should be used alongside its children simultaneously.

"""
# --
Qgis.TileRefinementProcess.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TileChildrenAvailability.NoChildren.__doc__ = "Tile is known to have no children"
Qgis.TileChildrenAvailability.Available.__doc__ = "Tile children are already available"
Qgis.TileChildrenAvailability.NeedFetching.__doc__ = "Tile has children, but they are not yet available and must be fetched"
Qgis.TileChildrenAvailability.__doc__ = """Possible availability states for a tile's children.

.. versionadded:: 3.34

* ``NoChildren``: Tile is known to have no children
* ``Available``: Tile children are already available
* ``NeedFetching``: Tile has children, but they are not yet available and must be fetched

"""
# --
Qgis.TileChildrenAvailability.baseClass = Qgis
# monkey patching scoped based enum
Qgis.TiledSceneRequestFlag.NoHierarchyFetch.__doc__ = "Do not allow hierarchy fetching when hierarchy is not currently available. Avoids network requests, but may result in an incomplete tile set. If set, then callers will need to manually perform hierarchy fetches as required."
Qgis.TiledSceneRequestFlag.__doc__ = """Flags which control how tiled scene requests behave.

.. versionadded:: 3.34

* ``NoHierarchyFetch``: Do not allow hierarchy fetching when hierarchy is not currently available. Avoids network requests, but may result in an incomplete tile set. If set, then callers will need to manually perform hierarchy fetches as required.

"""
# --
Qgis.TiledSceneRequestFlag.baseClass = Qgis
Qgis.TiledSceneRequestFlags.baseClass = Qgis
TiledSceneRequestFlags = Qgis  # dirty hack since SIP seems to introduce the flags in module
# monkey patching scoped based enum
Qgis.TiledSceneRendererFlag.RequiresTextures.__doc__ = "Renderer requires textures"
Qgis.TiledSceneRendererFlag.ForceRasterRender.__doc__ = "Layer should always be rendered as a raster image"
Qgis.TiledSceneRendererFlag.RendersTriangles.__doc__ = "Renderer can render triangle primitives"
Qgis.TiledSceneRendererFlag.RendersLines.__doc__ = "Renderer can render line primitives"
Qgis.TiledSceneRendererFlag.__doc__ = """Flags which control how tiled scene 2D renderers behave.

.. versionadded:: 3.34

* ``RequiresTextures``: Renderer requires textures
* ``ForceRasterRender``: Layer should always be rendered as a raster image
* ``RendersTriangles``: Renderer can render triangle primitives
* ``RendersLines``: Renderer can render line primitives

"""
# --
Qgis.TiledSceneRendererFlag.baseClass = Qgis
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
Qgis.GdalResampleAlgorithm.__doc__ = """Resampling algorithm to be used (equivalent to GDAL's enum GDALResampleAlg)

.. note::

   RA_Max, RA_Min, RA_Median, RA_Q1 and RA_Q3 are available on GDAL >= 2.0 builds only

.. versionadded:: 3.34

* ``RA_NearestNeighbour``: Nearest neighbour (select on one input pixel)
* ``RA_Bilinear``: Bilinear (2x2 kernel)
* ``RA_Cubic``: Cubic Convolution Approximation (4x4 kernel)
* ``RA_CubicSpline``: Cubic B-Spline Approximation (4x4 kernel)
* ``RA_Lanczos``: Lanczos windowed sinc interpolation (6x6 kernel)
* ``RA_Average``: Average (computes the average of all non-NODATA contributing pixels)
* ``RA_Mode``: Mode (selects the value which appears most often of all the sampled points)
* ``RA_Max``: Maximum (selects the maximum of all non-NODATA contributing pixels)
* ``RA_Min``: Minimum (selects the minimum of all non-NODATA contributing pixels)
* ``RA_Median``: Median (selects the median of all non-NODATA contributing pixels)
* ``RA_Q1``: First quartile (selects the first quartile of all non-NODATA contributing pixels)
* ``RA_Q3``: Third quartile (selects the third quartile of all non-NODATA contributing pixels)

"""
# --
Qgis.GdalResampleAlgorithm.baseClass = Qgis
# monkey patching scoped based enum
Qgis.VsiHandlerType.Invalid.__doc__ = "Invalid type, i.e. not a valid VSI handler"
Qgis.VsiHandlerType.Archive.__doc__ = "File archive type (e.g. vsizip)"
Qgis.VsiHandlerType.Network.__doc__ = "Generic network types (e.g. vsicurl)"
Qgis.VsiHandlerType.Cloud.__doc__ = "Specific cloud provider types (e.g. vsis3)"
Qgis.VsiHandlerType.Memory.__doc__ = "In-memory types (e.g. vsimem)"
Qgis.VsiHandlerType.Other.__doc__ = "All other types"
Qgis.VsiHandlerType.__doc__ = """GDAL VSI handler types.

.. versionadded:: 3.40

* ``Invalid``: Invalid type, i.e. not a valid VSI handler
* ``Archive``: File archive type (e.g. vsizip)
* ``Network``: Generic network types (e.g. vsicurl)
* ``Cloud``: Specific cloud provider types (e.g. vsis3)
* ``Memory``: In-memory types (e.g. vsimem)
* ``Other``: All other types

"""
# --
Qgis.VsiHandlerType.baseClass = Qgis
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
Qgis.ZonalStatistic.MinimumPoint.__doc__ = "Pixel centroid for minimum pixel value \n.. versionadded:: 3.42"
Qgis.ZonalStatistic.MaximumPoint.__doc__ = "Pixel centroid for maximum pixel value \n.. versionadded:: 3.42"
Qgis.ZonalStatistic.All.__doc__ = "All statistics. For QGIS 3.x this includes ONLY numeric statistics, but for 4.0 this will be extended to included non-numeric statistics. Consider using AllNumeric instead."
Qgis.ZonalStatistic.AllNumeric.__doc__ = "All numeric statistics \n.. versionadded:: 3.42"
Qgis.ZonalStatistic.Default.__doc__ = "Default statistics"
Qgis.ZonalStatistic.__doc__ = """Statistics to be calculated during a zonal statistics operation.

.. versionadded:: 3.36.

* ``Count``: Pixel count
* ``Sum``: Sum of pixel values
* ``Mean``: Mean of pixel values
* ``Median``: Median of pixel values
* ``StDev``: Standard deviation of pixel values
* ``Min``: Min of pixel values
* ``Max``: Max of pixel values
* ``Range``: Range of pixel values (max - min)
* ``Minority``: Minority of pixel values
* ``Majority``: Majority of pixel values
* ``Variety``: Variety (count of distinct) pixel values
* ``Variance``: Variance of pixel values
* ``MinimumPoint``: Pixel centroid for minimum pixel value

  .. versionadded:: 3.42

* ``MaximumPoint``: Pixel centroid for maximum pixel value

  .. versionadded:: 3.42

* ``All``: All statistics. For QGIS 3.x this includes ONLY numeric statistics, but for 4.0 this will be extended to included non-numeric statistics. Consider using AllNumeric instead.
* ``AllNumeric``: All numeric statistics

  .. versionadded:: 3.42

* ``Default``: Default statistics

"""
# --
Qgis.ZonalStatistic.baseClass = Qgis
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
Qgis.ZonalStatisticResult.__doc__ = """Zonal statistics result codes.

.. versionadded:: 3.36.

* ``Success``: Success
* ``LayerTypeWrong``: Layer is not a polygon layer
* ``LayerInvalid``: Layer is invalid
* ``RasterInvalid``: Raster layer is invalid
* ``RasterBandInvalid``: The raster band does not exist on the raster layer
* ``FailedToCreateField``: Output fields could not be created
* ``Canceled``: Algorithm was canceled

"""
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
Qgis.Aggregate.__doc__ = """Available aggregates to calculate. Not all aggregates are available for all field
types.

.. versionadded:: 3.36.

* ``Count``: Count
* ``CountDistinct``: Number of distinct values
* ``CountMissing``: Number of missing (null) values
* ``Min``: Min of values
* ``Max``: Max of values
* ``Sum``: Sum of values
* ``Mean``: Mean of values (numeric fields only)
* ``Median``: Median of values (numeric fields only)
* ``StDev``: Standard deviation of values (numeric fields only)
* ``StDevSample``: Sample standard deviation of values (numeric fields only)
* ``Range``: Range of values (max - min) (numeric and datetime fields only)
* ``Minority``: Minority of values
* ``Majority``: Majority of values
* ``FirstQuartile``: First quartile (numeric fields only)
* ``ThirdQuartile``: Third quartile (numeric fields only)
* ``InterQuartileRange``: Inter quartile range (IQR) (numeric fields only)
* ``StringMinimumLength``: Minimum length of string (string fields only)
* ``StringMaximumLength``: Maximum length of string (string fields only)
* ``StringConcatenate``: Concatenate values with a joining string (string fields only). Specify the delimiter using setDelimiter().
* ``GeometryCollect``: Create a multipart geometry from aggregated geometries
* ``ArrayAggregate``: Create an array of values
* ``StringConcatenateUnique``: Concatenate unique values with a joining string (string fields only). Specify the delimiter using setDelimiter().

"""
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
QgsStatisticalSummary.First.__doc__ = "First value \n.. versionadded:: 3.6"
QgsStatisticalSummary.Last = Qgis.Statistic.Last
QgsStatisticalSummary.Last.is_monkey_patched = True
QgsStatisticalSummary.Last.__doc__ = "Last value \n.. versionadded:: 3.6"
QgsStatisticalSummary.All = Qgis.Statistic.All
QgsStatisticalSummary.All.is_monkey_patched = True
QgsStatisticalSummary.All.__doc__ = "All statistics"
Qgis.Statistic.__doc__ = """Available generic statistics.

.. versionadded:: 3.36.

* ``Count``: Count
* ``CountMissing``: Number of missing (null) values
* ``Sum``: Sum of values
* ``Mean``: Mean of values
* ``Median``: Median of values
* ``StDev``: Standard deviation of values
* ``StDevSample``: Sample standard deviation of values
* ``Min``: Min of values
* ``Max``: Max of values
* ``Range``: Range of values (max - min)
* ``Minority``: Minority of values
* ``Majority``: Majority of values
* ``Variety``: Variety (count of distinct) values
* ``FirstQuartile``: First quartile
* ``ThirdQuartile``: Third quartile
* ``InterQuartileRange``: Inter quartile range (IQR)
* ``First``: First value

  .. versionadded:: 3.6

* ``Last``: Last value

  .. versionadded:: 3.6

* ``All``: All statistics

"""
# --
Qgis.Statistic.baseClass = Qgis
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
Qgis.DateTimeStatistic.__doc__ = """Available date/time statistics.

.. versionadded:: 3.36.

* ``Count``: Count
* ``CountDistinct``: Number of distinct datetime values
* ``CountMissing``: Number of missing (null) values
* ``Min``: Minimum (earliest) datetime value
* ``Max``: Maximum (latest) datetime value
* ``Range``: Interval between earliest and latest datetime value
* ``All``: All statistics

"""
# --
Qgis.DateTimeStatistic.baseClass = Qgis
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
Qgis.StringStatistic.__doc__ = """Available string statistics.

.. versionadded:: 3.36.

* ``Count``: Count
* ``CountDistinct``: Number of distinct string values
* ``CountMissing``: Number of missing (null) values
* ``Min``: Minimum string value
* ``Max``: Maximum string value
* ``MinimumLength``: Minimum length of string
* ``MaximumLength``: Maximum length of string
* ``MeanLength``: Mean length of strings
* ``Minority``: Minority of strings
* ``Majority``: Majority of strings
* ``All``: All statistics

"""
# --
Qgis.StringStatistic.baseClass = Qgis
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
Qgis.RasterBandStatistic.__doc__ = """Available raster band statistics.

.. versionadded:: 3.36.

* ``NoStatistic``: No statistic
* ``Min``: Minimum
* ``Max``: Maximum
* ``Range``: Range
* ``Sum``: Sum
* ``Mean``: Mean
* ``StdDev``: Standard deviation
* ``SumOfSquares``: Sum of squares
* ``All``: All available statistics

"""
# --
Qgis.RasterBandStatistic.baseClass = Qgis
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
Qgis.SensorThingsEntity.MultiDatastream.__doc__ = "A MultiDatastream groups a collection of Observations and the Observations in a MultiDatastream have a complex result type. Implemented in the SensorThings version 1.1 \"MultiDatastream extension\". \n.. versionadded:: 3.38"
Qgis.SensorThingsEntity.__doc__ = """OGC SensorThings API entity types.

.. versionadded:: 3.36

* ``Invalid``: An invalid/unknown entity
* ``Thing``: A Thing is an object of the physical world (physical things) or the information world (virtual things) that is capable of being identified and integrated into communication networks
* ``Location``: A Location entity locates the Thing or the Things it associated with. A Thing’s Location entity is defined as the last known location of the Thing
* ``HistoricalLocation``: A Thing’s HistoricalLocation entity set provides the times of the current (i.e., last known) and previous locations of the Thing
* ``Datastream``: A Datastream groups a collection of Observations measuring the same ObservedProperty and produced by the same Sensor
* ``Sensor``: A Sensor is an instrument that observes a property or phenomenon with the goal of producing an estimate of the value of the property
* ``ObservedProperty``: An ObservedProperty specifies the phenomenon of an Observation
* ``Observation``: An Observation is the act of measuring or otherwise determining the value of a property
* ``FeatureOfInterest``: In the context of the Internet of Things, many Observations’ FeatureOfInterest can be the Location of the Thing. For example, the FeatureOfInterest of a wifi-connect thermostat can be the Location of the thermostat (i.e., the living room where the thermostat is located in). In the case of remote sensing, the FeatureOfInterest can be the geographical area or volume that is being sensed
* ``MultiDatastream``: A MultiDatastream groups a collection of Observations and the Observations in a MultiDatastream have a complex result type. Implemented in the SensorThings version 1.1 \"MultiDatastream extension\".

  .. versionadded:: 3.38


"""
# --
Qgis.SensorThingsEntity.baseClass = Qgis
# monkey patching scoped based enum
Qgis.ColorModel.Rgb.__doc__ = "RGB color model"
Qgis.ColorModel.Cmyk.__doc__ = "CMYK color model"
Qgis.ColorModel.__doc__ = """Color model types

.. versionadded:: 3.40

* ``Rgb``: RGB color model
* ``Cmyk``: CMYK color model

"""
# --
Qgis.ColorModel.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DocumentationApi.PyQgis.__doc__ = "PyQgis API documentation"
Qgis.DocumentationApi.PyQgisSearch.__doc__ = "Search in PyQgis API documentation"
Qgis.DocumentationApi.CppQgis.__doc__ = "C++ QGIS API documentation"
Qgis.DocumentationApi.Qt.__doc__ = "Qt API documentation"
Qgis.DocumentationApi.__doc__ = """Documentation API

.. versionadded:: 3.42

* ``PyQgis``: PyQgis API documentation
* ``PyQgisSearch``: Search in PyQgis API documentation
* ``CppQgis``: C++ QGIS API documentation
* ``Qt``: Qt API documentation

"""
# --
Qgis.DocumentationApi.baseClass = Qgis
# monkey patching scoped based enum
Qgis.DocumentationBrowser.DeveloperToolsPanel.__doc__ = "Embedded webview in the DevTools panel"
Qgis.DocumentationBrowser.SystemWebBrowser.__doc__ = "Default system web browser"
Qgis.DocumentationBrowser.__doc__ = """Documentation API browser

.. versionadded:: 3.42

* ``DeveloperToolsPanel``: Embedded webview in the DevTools panel
* ``SystemWebBrowser``: Default system web browser

"""
# --
Qgis.DocumentationBrowser.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MouseHandlesAction.MoveItem.__doc__ = "Move item"
Qgis.MouseHandlesAction.ResizeUp.__doc__ = "Resize up (Top handle)"
Qgis.MouseHandlesAction.ResizeDown.__doc__ = "Resize down (Bottom handle)"
Qgis.MouseHandlesAction.ResizeLeft.__doc__ = "Resize left (Left handle)"
Qgis.MouseHandlesAction.ResizeRight.__doc__ = "Resize right (Right handle)"
Qgis.MouseHandlesAction.ResizeLeftUp.__doc__ = "Resize left up (Top left handle)"
Qgis.MouseHandlesAction.ResizeRightUp.__doc__ = "Resize right up (Top right handle)"
Qgis.MouseHandlesAction.ResizeLeftDown.__doc__ = "Resize left down (Bottom left handle)"
Qgis.MouseHandlesAction.ResizeRightDown.__doc__ = "Resize right down (Bottom right handle)"
Qgis.MouseHandlesAction.SelectItem.__doc__ = "Select item"
Qgis.MouseHandlesAction.NoAction.__doc__ = "No action"
Qgis.MouseHandlesAction.__doc__ = """Action to be performed by the mouse handles

.. versionadded:: 3.42

* ``MoveItem``: Move item
* ``ResizeUp``: Resize up (Top handle)
* ``ResizeDown``: Resize down (Bottom handle)
* ``ResizeLeft``: Resize left (Left handle)
* ``ResizeRight``: Resize right (Right handle)
* ``ResizeLeftUp``: Resize left up (Top left handle)
* ``ResizeRightUp``: Resize right up (Top right handle)
* ``ResizeLeftDown``: Resize left down (Bottom left handle)
* ``ResizeRightDown``: Resize right down (Bottom right handle)
* ``SelectItem``: Select item
* ``NoAction``: No action

"""
# --
Qgis.MouseHandlesAction.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MeshRangeLimit.NotSet.__doc__ = "User defined"
Qgis.MeshRangeLimit.MinimumMaximum.__doc__ = "Real min-max values"
Qgis.MeshRangeLimit.__doc__ = """Describes the limits used to compute mesh ranges (min/max values).

.. versionadded:: 3.42

* ``NotSet``: User defined
* ``MinimumMaximum``: Real min-max values

"""
# --
Qgis.MeshRangeLimit.baseClass = Qgis
# monkey patching scoped based enum
Qgis.MeshRangeExtent.WholeMesh.__doc__ = "Whole mesh is used to compute statistics"
Qgis.MeshRangeExtent.FixedCanvas.__doc__ = "Current extent of the canvas (at the time of computation) is used to compute statistics"
Qgis.MeshRangeExtent.UpdatedCanvas.__doc__ = "Constantly updated extent of the canvas is used to compute statistics"
Qgis.MeshRangeExtent.__doc__ = """Describes the extent used to compute mesh ranges (min/max values).

.. versionadded:: 3.42

* ``WholeMesh``: Whole mesh is used to compute statistics
* ``FixedCanvas``: Current extent of the canvas (at the time of computation) is used to compute statistics
* ``UpdatedCanvas``: Constantly updated extent of the canvas is used to compute statistics

"""
# --
Qgis.MeshRangeExtent.baseClass = Qgis
# monkey patching scoped based enum
Qgis.PointCloudAccessType.Local.__doc__ = "Local means the source is a local file on the machine"
Qgis.PointCloudAccessType.Remote.__doc__ = "Remote means it's loaded through a protocol like HTTP"
Qgis.PointCloudAccessType.__doc__ = """The access type of the data, local is for local files and remote for remote files (over HTTP).

.. seealso:: :py:class:`QgsPointCloudIndex`

.. versionadded:: 3.42

* ``Local``: Local means the source is a local file on the machine
* ``Remote``: Remote means it's loaded through a protocol like HTTP

"""
# --
Qgis.PointCloudAccessType.baseClass = Qgis
# monkey patching scoped based enum
Qgis.PointCloudZoomOutRenderBehavior.RenderExtents.__doc__ = "Render only point cloud extents when zoomed out"
Qgis.PointCloudZoomOutRenderBehavior.RenderOverview.__doc__ = "Render overview point cloud when zoomed out"
Qgis.PointCloudZoomOutRenderBehavior.RenderOverviewAndExtents.__doc__ = "Render point cloud extents over overview point cloud"
Qgis.PointCloudZoomOutRenderBehavior.__doc__ = """Point cloud zoom out options

.. versionadded:: 3.42

* ``RenderExtents``: Render only point cloud extents when zoomed out
* ``RenderOverview``: Render overview point cloud when zoomed out
* ``RenderOverviewAndExtents``: Render point cloud extents over overview point cloud

"""
# --
Qgis.PointCloudZoomOutRenderBehavior.baseClass = Qgis
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


Qgis.BrowserItemCapability.__or__ = lambda flag1, flag2: Qgis.BrowserItemCapabilities(_force_int(flag1) | _force_int(flag2))
Qgis.GeometryValidityFlag.__or__ = lambda flag1, flag2: Qgis.GeometryValidityFlags(_force_int(flag1) | _force_int(flag2))
Qgis.LabelingFlag.__or__ = lambda flag1, flag2: Qgis.LabelingFlags(_force_int(flag1) | _force_int(flag2))
Qgis.LabelLinePlacementFlag.__or__ = lambda flag1, flag2: Qgis.LabelLinePlacementFlags(_force_int(flag1) | _force_int(flag2))
Qgis.MapSettingsFlag.__or__ = lambda flag1, flag2: Qgis.MapSettingsFlags(_force_int(flag1) | _force_int(flag2))
Qgis.ProjectReadFlag.__or__ = lambda flag1, flag2: Qgis.ProjectReadFlags(_force_int(flag1) | _force_int(flag2))
Qgis.RenderContextFlag.__or__ = lambda flag1, flag2: Qgis.RenderContextFlags(_force_int(flag1) | _force_int(flag2))
Qgis.SnappingType.__or__ = lambda flag1, flag2: Qgis.SnappingTypes(_force_int(flag1) | _force_int(flag2))
Qgis.SymbolPreviewFlag.__or__ = lambda flag1, flag2: Qgis.SymbolPreviewFlags(_force_int(flag1) | _force_int(flag2))
Qgis.SymbolRenderHint.__or__ = lambda flag1, flag2: Qgis.SymbolRenderHints(_force_int(flag1) | _force_int(flag2))
Qgis.FeatureRequestFlag.__or__ = lambda flag1, flag2: Qgis.FeatureRequestFlags(_force_int(flag1) | _force_int(flag2))
Qgis.ProcessingFeatureSourceDefinitionFlag.__or__ = lambda flag1, flag2: Qgis.ProcessingFeatureSourceDefinitionFlags(_force_int(flag1) | _force_int(flag2))
Qgis.ZonalStatistic.__or__ = lambda flag1, flag2: Qgis.ZonalStatistics(_force_int(flag1) | _force_int(flag2))
Qgis.Statistic.__or__ = lambda flag1, flag2: Qgis.Statistics(_force_int(flag1) | _force_int(flag2))
Qgis.DateTimeStatistic.__or__ = lambda flag1, flag2: Qgis.DateTimeStatistics(_force_int(flag1) | _force_int(flag2))
Qgis.StringStatistic.__or__ = lambda flag1, flag2: Qgis.StringStatistics(_force_int(flag1) | _force_int(flag2))
Qgis.RasterBandStatistic.__or__ = lambda flag1, flag2: Qgis.RasterBandStatistics(_force_int(flag1) | _force_int(flag2))
Qgis.RasterProviderCapability.__or__ = lambda flag1, flag2: Qgis.RasterProviderCapabilities(_force_int(flag1) | _force_int(flag2))
Qgis.ProcessingProviderFlag.__or__ = lambda flag1, flag2: Qgis.ProcessingProviderFlags(_force_int(flag1) | _force_int(flag2))
Qgis.ProcessingAlgorithmFlag.__or__ = lambda flag1, flag2: Qgis.ProcessingAlgorithmFlags(_force_int(flag1) | _force_int(flag2))
Qgis.ProcessingFeatureSourceFlag.__or__ = lambda flag1, flag2: Qgis.ProcessingFeatureSourceFlags(_force_int(flag1) | _force_int(flag2))
Qgis.ProcessingParameterTypeFlag.__or__ = lambda flag1, flag2: Qgis.ProcessingParameterTypeFlags(_force_int(flag1) | _force_int(flag2))
Qgis.ProcessingParameterFlag.__or__ = lambda flag1, flag2: Qgis.ProcessingParameterFlags(_force_int(flag1) | _force_int(flag2))
Qgis.DataItemProviderCapability.__or__ = lambda flag1, flag2: Qgis.DataItemProviderCapabilities(_force_int(flag1) | _force_int(flag2))
Qgis.VectorRenderingSimplificationFlag.__or__ = lambda flag1, flag2: Qgis.VectorRenderingSimplificationFlags(_force_int(flag1) | _force_int(flag2))
Qgis.DataProviderReadFlag.__or__ = lambda flag1, flag2: Qgis.DataProviderReadFlags(_force_int(flag1) | _force_int(flag2))
Qgis.VectorProviderCapability.__or__ = lambda flag1, flag2: Qgis.VectorProviderCapabilities(_force_int(flag1) | _force_int(flag2))
try:
    Qgis.__attribute_docs__ = {'QGIS_DEV_VERSION': 'The development version', 'DEFAULT_SEARCH_RADIUS_MM': 'Identify search radius in mm', 'DEFAULT_MAPTOPIXEL_THRESHOLD': 'Default threshold between map coordinates and device coordinates for map2pixel simplification', 'DEFAULT_HIGHLIGHT_COLOR': 'Default highlight color.  The transparency is expected to only be applied to polygon\nfill. Lines and outlines are rendered opaque.', 'DEFAULT_HIGHLIGHT_BUFFER_MM': 'Default highlight buffer in mm.', 'DEFAULT_HIGHLIGHT_MIN_WIDTH_MM': 'Default highlight line/stroke minimum width in mm.', 'SCALE_PRECISION': 'Fudge factor used to compare two scales. The code is often going from scale to scale\ndenominator. So it looses precision and, when a limit is inclusive, can lead to errors.\nTo avoid that, use this factor instead of using <= or >=.\n\n.. deprecated:: 3.40\n\n   No longer used by QGIS and will be removed in QGIS 4.0.', 'DEFAULT_Z_COORDINATE': 'Default Z coordinate value.\nThis value have to be assigned to the Z coordinate for the vertex.', 'DEFAULT_M_COORDINATE': 'Default M coordinate value.\nThis value have to be assigned to the M coordinate for the vertex.\n\n.. versionadded:: 3.20', 'UI_SCALE_FACTOR': 'UI scaling factor. This should be applied to all widget sizes obtained from font metrics,\nto account for differences in the default font sizes across different platforms.', 'DEFAULT_SNAP_TOLERANCE': 'Default snapping distance tolerance.', 'DEFAULT_SNAP_UNITS': 'Default snapping distance units.'}
    Qgis.version = staticmethod(Qgis.version)
    Qgis.versionInt = staticmethod(Qgis.versionInt)
    Qgis.releaseName = staticmethod(Qgis.releaseName)
    Qgis.devVersion = staticmethod(Qgis.devVersion)
    Qgis.defaultProjectScales = staticmethod(Qgis.defaultProjectScales)
    Qgis.geosVersionInt = staticmethod(Qgis.geosVersionInt)
    Qgis.geosVersionMajor = staticmethod(Qgis.geosVersionMajor)
    Qgis.geosVersionMinor = staticmethod(Qgis.geosVersionMinor)
    Qgis.geosVersionPatch = staticmethod(Qgis.geosVersionPatch)
    Qgis.geosVersion = staticmethod(Qgis.geosVersion)
except (NameError, AttributeError):
    pass
