<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation=""
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-1</gml:X><gml:Y>-2.3875</gml:Y></gml:coord>
      <gml:coord><gml:X>4.775</gml:X><gml:Y>3</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                     
  <gml:featureMember>
    <ogr:extract_by_extent_clip fid="polys.0">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-1,-1 -1,3 3,3 3,2 2,2 2,-1 -1,-1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:name>aaaaa</ogr:name>
      <ogr:intval>33</ogr:intval>
      <ogr:floatval>44.123456</ogr:floatval>
    </ogr:extract_by_extent_clip>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:extract_by_extent_clip fid="polys.5">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>3,2 4.775,1.40833333333333 4.775,-2.3875 2,-1 2,2 3,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:name>elim</ogr:name>
      <ogr:intval>2</ogr:intval>
      <ogr:floatval>3.33</ogr:floatval>
    </ogr:extract_by_extent_clip>
  </gml:featureMember>
</ogr:FeatureCollection>
