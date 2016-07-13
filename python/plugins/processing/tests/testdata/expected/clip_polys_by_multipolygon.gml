<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ clip_polys_by_multipolygon.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>0</gml:X><gml:Y>0</gml:Y></gml:coord>
      <gml:coord><gml:X>8</gml:X><gml:Y>2</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                                
  <gml:featureMember>
    <ogr:clip_polys_by_multipolygon fid="polys.0">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,0 0,1 1,1 1,0 0,0</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>aaaaa</ogr:name>
      <ogr:intval>33</ogr:intval>
      <ogr:floatval>44.123456</ogr:floatval>
    </ogr:clip_polys_by_multipolygon>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:clip_polys_by_multipolygon fid="polys.3">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>8,0 7,0 7,1 8,1 8,0</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>ASDF</ogr:name>
      <ogr:intval>0</ogr:intval>
    </ogr:clip_polys_by_multipolygon>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:clip_polys_by_multipolygon fid="polys.5">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>3,2 4.0,1.666666666666667 4,1 2,1 2,2 3,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>elim</ogr:name>
      <ogr:intval>2</ogr:intval>
      <ogr:floatval>3.33</ogr:floatval>
    </ogr:clip_polys_by_multipolygon>
  </gml:featureMember>
</ogr:FeatureCollection>
