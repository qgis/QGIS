<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation=""
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-1</gml:X><gml:Y>-3</gml:Y></gml:coord>
      <gml:coord><gml:X>6</gml:X><gml:Y>3</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                              
  <gml:featureMember>
    <ogr:extract_by_location_contains fid="polys.0">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-1,-1 -1,3 3,3 3,2 2,2 2,-1 -1,-1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>aaaaa</ogr:name>
      <ogr:intval>33</ogr:intval>
      <ogr:floatval>44.123456</ogr:floatval>
    </ogr:extract_by_location_contains>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:extract_by_location_contains fid="polys.5">
      <ogr:geometryProperty><gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>3,2 6,1 6,-3 2,-1 2,2 3,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></ogr:geometryProperty>
      <ogr:name>elim</ogr:name>
      <ogr:intval>2</ogr:intval>
      <ogr:floatval>3.33</ogr:floatval>
    </ogr:extract_by_location_contains>
  </gml:featureMember>
</ogr:FeatureCollection>
