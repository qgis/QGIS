--- python/core/auto_generated/providers/qgsprovidermetadata.sip.in	2023-03-31 12:46:07.392212852 +1000
+++ python/core/auto_generated/providers/qgsprovidermetadata.sip.in.c4aab9a2c1257d44136dd689911d88fb01647b99.prepare	2023-03-31 12:48:47.815670333 +1000
@@ -275,7 +275,7 @@
 .. versionadded:: 3.10
 %End
 
-    virtual QString filters( FilterType type );
+    virtual QString filters( Qgis::FileFilterType type );
 %Docstring
 Builds the list of file filter strings (supported formats)
 
--- python/core/auto_generated/qgis.sip.in	2023-03-31 12:46:07.497213806 +1000
+++ python/core/auto_generated/qgis.sip.in.c4aab9a2c1257d44136dd689911d88fb01647b99.prepare	2023-03-31 12:48:47.968671723 +1000
@@ -530,6 +530,7 @@
       Mesh,
       MeshDataset,
       PointCloud,
+      VectorTile,
     };
 
     enum class SublayerQueryFlag
