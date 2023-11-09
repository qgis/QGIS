<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ join_polys_to_lines_largest_unjoined.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>4</gml:X><gml:Y>4</gml:Y></gml:coord>
      <gml:coord><gml:X>6</gml:X><gml:Y>5</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                                
  <gml:featureMember>
    <ogr:join_polys_to_lines_largest_unjoined fid="polys.1">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>5,5 6,4 4,4 5,5</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>Aaaaa</ogr:name>
      <ogr:intval>-33</ogr:intval>
      <ogr:floatval>0</ogr:floatval>
    </ogr:join_polys_to_lines_largest_unjoined>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:join_polys_to_lines_largest_unjoined fid="polys.4">
      <ogr:name xsi:nil="true"/>
      <ogr:intval>120</ogr:intval>
      <ogr:floatval>-100291.43213</ogr:floatval>
    </ogr:join_polys_to_lines_largest_unjoined>
  </gml:featureMember>
</ogr:FeatureCollection>
