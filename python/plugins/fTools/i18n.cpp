/*
 This is NOT a proper c++ source code. This file is only designed to be caught
 by qmake and included in lupdate. It contains all translateable strings copied
 from the python file(s):
   fTools.py

 Please keep the python file(s) and this file synchronized. Borys hopes we'll find a
 more automated and reliable way to either include the python files in lupdate
 or reconcile both lupdate and pylupdate in one .ts file some day.
*/


/*---------------------
file: fTools.py
---------------------*/

translate( "fTools", "Quantum GIS version detected: " )
translate( "fTools", "This version of fTools requires at least QGIS version 1.0.0" )
translate( "fTools", "Plugin will not be enabled." )
translate( "fTools", "&Tools" )
translate( "fTools", "&Analysis Tools" )
translate( "fTools", "Distance matrix" )
translate( "fTools", "Sum line lengths" )
translate( "fTools", "Points in polygon" )
translate( "fTools", "Basic statistics" )
translate( "fTools", "List unique values" )
translate( "fTools", "Nearest neighbour analysis" )
translate( "fTools", "Mean coordinate(s)" )
translate( "fTools", "Line intersections" )
translate( "fTools", "&Sampling Tools" )
translate( "fTools", "Random selection" )
translate( "fTools", "Random selection within subsets" )
translate( "fTools", "Random points" )
translate( "fTools", "Regular points" )
translate( "fTools", "Vector grid" )
translate( "fTools", "Select by location" )
translate( "fTools", "&Geoprocessing Tools" )
translate( "fTools", "Convex hull(s)" )
translate( "fTools", "Buffer(s)" )
translate( "fTools", "Intersect" )
translate( "fTools", "Union" )
translate( "fTools", "Symetrical difference" )
translate( "fTools", "Clip" )
translate( "fTools", "Dissolve" )
translate( "fTools", "Difference" )
translate( "fTools", "G&eometry Tools" )
translate( "fTools", "Export/Add geometry columns" )
translate( "fTools", "Check geometry validity" )
translate( "fTools", "Polygon centroids" )
translate( "fTools", "Extract nodes" )
translate( "fTools", "Simplify geometries" )
translate( "fTools", "Multipart to singleparts" )
translate( "fTools", "Singleparts to multipart" )
translate( "fTools", "Polygons to lines" )
translate( "fTools", "&Data Management Tools" )
translate( "fTools", "Export to new projection" )
translate( "fTools", "Define current projection" )
translate( "fTools", "Join attributes" )
translate( "fTools", "Join attributes by location" )
translate( "fTools", "Split vector layer" )
translate( "fTools", "About fTools" )
translate( "fTools", "&Research Tools" )
translate( "fTools", "Delaunay triangulation" )
translate( "fTools", "Polygon from layer extent" )
translate( "fTools", "Input layer" )
translate( "fTools", "Input point vector layer" )
translate( "fTools", "Output polygon shapefile" )

/*---------------------
file: doGeoprocessing.py
---------------------*/

GeoprocessingDialog::foo()
{
  tr( "Dissolve all" )
  tr( "Please specify an input layer" )
  tr( "Please specify a difference/intersect/union layer" )
  tr( "Please specify valid buffer value" )
  tr( "Please specify dissolve field" )
  tr( "Please specify output shapefile" )
  tr( "Unable to create geoprocessing result." )
  tr( "Created output shapefile" )
  tr( "Would you like to add the new layer to the TOC?" )
  tr( "Buffer(s)" )
  tr( "Create single minimum convex hull" )
  tr( "Create convex hulls based on input field" )
  tr( "Convex hull(s)" )
  tr( "Dissolve" )
  tr( "Erase layer" )
  tr( "Difference" )
  tr( "Intersect layer" )
  tr( "Intersect" )
  tr( "Difference layer" )
  tr( "Symetrical difference" )
  tr( "Clip layer" )
  tr( "Clip" )
  tr( "Union layer" )
  tr( "Union" )
}

/*---------------------
file: doGeometry.py
---------------------*/

GeometryDialog::foo()
{
  tr( "Merge all" )
  tr( "Please specify input vector layer" )
  tr( "Please specify output shapefile" )
  tr( "Please specify valid tolerance value" )
  tr( "Please specify valid UID field" )
  tr( "Created output shapefile" )
  tr( "Would you like to add the new layer to the TOC?" )
  tr( "Singleparts to multipart" )
  tr( "Output shapefile" )
  tr( "Multipart to singleparts" )
  tr( "Output shapefile" )
  tr( "Extract nodes" )
  tr( "Polygons to lines" )
  tr( "Output shapefile" )
  tr( "Input polygon vector layer" )
  tr( "Export/Add geometry columns" )
  tr( "Output shapefile" )
  tr( "Input vector layer" )
  tr( "Simplify geometries" )
  tr( "Output shapefile" )
  tr( "Polygon centroids" )
  tr( "Output point shapefile" )
  tr( "Input polygon vector layer" )
  tr( "Error processing specified tolerance!" )
  tr( "Please choose larger tolerance..." )
  tr( "Function not found" )
  tr( "Error writing output shapefile" )
  tr( "Unable to delete existing layer..." )
}

/*---------------------
file: doVisual.py
---------------------*/

VisualDialog::foo()
{
  tr( "Please specify input vector layer" )
  tr( "Please specify input field" )
  tr( "Check geometry validity" )
  tr( "Geometry errors" )
  tr( "Total encountered errors" )
  tr( "List unique values" )
  tr( "Unique values:" )
  tr( "Total unique values:" )
  tr( "Basics statistics" )
  tr( "Statistics output" )
  tr( "Nearest neighbour analysis" )
  tr( "Nearest neighbour statistics" )
  tr( "Observed mean distance : " )
  tr( "Expected mean distance : " )
  tr( "Nearest neighbour index : " )
  tr( "Feature %1 contains an unnested hole" )
  tr( "Feature %1 is not closed" )
  tr( "Feature %1 is self intersecting" )
  tr( "Feature %1 has incorrect node ordering" )
}
