<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ execute_sql.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>2</gml:X><gml:Y>-3</gml:Y></gml:coord>
      <gml:coord><gml:X>6</gml:X><gml:Y>2</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                               
  <gml:featureMember>
    <ogr:execute_sql fid="polys.5">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>3,2 6,1 6,-3 2,-1 2,2 3,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:name>elim</ogr:name>
      <ogr:intval>2</ogr:intval>
      <ogr:floatval>3.330000000000000</ogr:floatval>
    </ogr:execute_sql>
  </gml:featureMember>
</ogr:FeatureCollection>
