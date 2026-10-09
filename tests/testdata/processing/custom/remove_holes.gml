<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation=""
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>2</gml:X><gml:Y>-3</gml:Y></gml:coord>
      <gml:coord><gml:X>10</gml:X><gml:Y>6</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                              
  <gml:featureMember>
    <ogr:remove_holes fid="polys.4">
      <ogr:intval>120</ogr:intval>
      <ogr:floatval>-100291.43213</ogr:floatval>
    </ogr:remove_holes>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:remove_holes fid="polys.5">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>3,2 6,1 6,-3 2,-1 2,2 3,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>elim</ogr:name>
      <ogr:intval>2</ogr:intval>
      <ogr:floatval>3.33</ogr:floatval>
    </ogr:remove_holes>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:remove_holes fid="polys.3">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,1 10,1 10,-3 6,-3 6,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>7,0 7,-2 9,-2 9,0 7,0</gml:coordinates></gml:LinearRing></gml:innerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>ASDF</ogr:name>
      <ogr:intval>0</ogr:intval>
    </ogr:remove_holes>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:remove_holes fid="remove_holes.3">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>2,6 2,3 10,3 10,6 2,6</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>2.5,5.6 2.5,3.5 5.6,3.5 5.6,5.6 2.5,5.6</gml:coordinates></gml:LinearRing></gml:innerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>7.0,5.5 7,5 8,5 8.0,5.5 7.0,5.5</gml:coordinates></gml:LinearRing></gml:innerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>6.0,4.5 6.0,3.5 9.0,3.5 9.0,4.5 6.0,4.5</gml:coordinates></gml:LinearRing></gml:innerBoundaryIs></gml:Polygon></ogr:geometryProperty>
    </ogr:remove_holes>
  </gml:featureMember>
</ogr:FeatureCollection>
