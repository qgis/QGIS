<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ gridify_polys.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-2</gml:X><gml:Y>-4</gml:Y></gml:coord>
      <gml:coord><gml:X>10</gml:X><gml:Y>6</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                             
  <gml:featureMember>
    <ogr:gridify_polys fid="polys.0">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-2,-2 -2,4 4,4 4,2 2,2 2,-2 -2,-2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>aaaaa</ogr:name>
      <ogr:intval>33</ogr:intval>
      <ogr:floatval>44.123456</ogr:floatval>
    </ogr:gridify_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:gridify_polys fid="polys.1">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,6 6,4 4,4 6,6</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>Aaaaa</ogr:name>
      <ogr:intval>-33</ogr:intval>
      <ogr:floatval>0</ogr:floatval>
    </ogr:gridify_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:gridify_polys fid="polys.2">
      <ogr:name>bbaaa</ogr:name>
      <ogr:floatval>0.123</ogr:floatval>
    </ogr:gridify_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:gridify_polys fid="polys.3">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,2 10,2 10,-4 6,-4 6,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>8,0 8,-2 10,-2 10,0 8,0</gml:coordinates></gml:LinearRing></gml:innerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>ASDF</ogr:name>
      <ogr:intval>0</ogr:intval>
    </ogr:gridify_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:gridify_polys fid="polys.4">
      <ogr:intval>120</ogr:intval>
      <ogr:floatval>-100291.43213</ogr:floatval>
    </ogr:gridify_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:gridify_polys fid="polys.5">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>4,2 6,2 6,-4 2,-2 2,2 4,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>elim</ogr:name>
      <ogr:intval>2</ogr:intval>
      <ogr:floatval>3.33</ogr:floatval>
    </ogr:gridify_polys>
  </gml:featureMember>
</ogr:FeatureCollection>
