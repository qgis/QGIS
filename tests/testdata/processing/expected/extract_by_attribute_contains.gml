<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation=""
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-1</gml:X><gml:Y>-1</gml:Y></gml:coord>
      <gml:coord><gml:X>6</gml:X><gml:Y>6</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                              
  <gml:featureMember>
    <ogr:extract_by_attribute_contains fid="polys.0">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-1,-1 -1,3 3,3 3,2 2,2 2,-1 -1,-1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>aaaaa</ogr:name>
      <ogr:intval>33</ogr:intval>
      <ogr:floatval>44.123456</ogr:floatval>
    </ogr:extract_by_attribute_contains>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:extract_by_attribute_contains fid="polys.1">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>5,5 6,4 4,4 5,5</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>Aaaaa</ogr:name>
      <ogr:intval>-33</ogr:intval>
      <ogr:floatval>0</ogr:floatval>
    </ogr:extract_by_attribute_contains>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:extract_by_attribute_contains fid="polys.2">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>2,5 2,6 3,6 3,5 2,5</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>bbaaa</ogr:name>
      <ogr:floatval>0.123</ogr:floatval>
    </ogr:extract_by_attribute_contains>
  </gml:featureMember>
</ogr:FeatureCollection>
