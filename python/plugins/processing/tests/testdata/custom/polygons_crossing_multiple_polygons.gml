<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ polygons_crossing_multiple_polygons.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-1</gml:X><gml:Y>-2</gml:Y></gml:coord>
      <gml:coord><gml:X>13</gml:X><gml:Y>4</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                             
  <gml:featureMember>
    <ogr:polygons_crossing_multiple_polygons fid="polygons_crossing_multiple_polygons.0">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>1,4 1,0 3,0 3,4 1,4</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>poly1</ogr:name>
      <ogr:val>3</ogr:val>
    </ogr:polygons_crossing_multiple_polygons>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:polygons_crossing_multiple_polygons fid="polygons_crossing_multiple_polygons.1">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>11,2 13,2 13,4 11,4 11,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>poly2</ogr:name>
      <ogr:val>54</ogr:val>
    </ogr:polygons_crossing_multiple_polygons>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:polygons_crossing_multiple_polygons fid="polygons_crossing_multiple_polygons.2">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,2 -1,2 -1,1 0,1 0,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>poly3</ogr:name>
      <ogr:val>7</ogr:val>
    </ogr:polygons_crossing_multiple_polygons>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:polygons_crossing_multiple_polygons fid="polygons_crossing_multiple_polygons.3">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>1,-2 1,0 10,0 10,-2 1,-2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>poly4</ogr:name>
      <ogr:val>6</ogr:val>
    </ogr:polygons_crossing_multiple_polygons>
  </gml:featureMember>
</ogr:FeatureCollection>
