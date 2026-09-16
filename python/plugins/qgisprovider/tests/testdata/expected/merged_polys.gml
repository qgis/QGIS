<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ merged_polys.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>-1</gml:X><gml:Y>-3</gml:Y></gml:coord>
      <gml:coord><gml:X>10</gml:X><gml:Y>6</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>
                                                                                                                                                             
  <gml:featureMember>
    <ogr:merged_polys fid="multipolys.0">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>2,1 2,2 3,2 3,3 4,3 4,1 2,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>1</ogr:Bintval>
      <ogr:Bfloatval>0.123</ogr:Bfloatval>
      <ogr:layer>multipolys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/multipolys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="multipolys.1">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>7,-1 8,-1 8,3 7,3 7,-1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>7,6 7,5 7,4 8,4 9,5 9,6 7,6</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:layer>multipolys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/multipolys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="multipolys.2">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,0 0,1 1,1 1,0 0,0</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>2</ogr:Bintval>
      <ogr:Bfloatval>-0.123</ogr:Bfloatval>
      <ogr:layer>multipolys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/multipolys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="multipolys.3">
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>3</ogr:Bintval>
      <ogr:Bfloatval>0</ogr:Bfloatval>
      <ogr:layer>multipolys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/multipolys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="polys.0">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-1,-1 -1,3 3,3 3,2 2,2 2,-1 -1,-1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:name>aaaaa</ogr:name>
      <ogr:intval>33</ogr:intval>
      <ogr:floatval>44.123456</ogr:floatval>
      <ogr:layer>polys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/polys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="polys.1">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>5,5 6,4 4,4 5,5</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:name>Aaaaa</ogr:name>
      <ogr:intval>-33</ogr:intval>
      <ogr:floatval>0</ogr:floatval>
      <ogr:layer>polys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/polys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="polys.2">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>2,5 2,6 3,6 3,5 2,5</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:name>bbaaa</ogr:name>
      <ogr:floatval>0.123</ogr:floatval>
      <ogr:layer>polys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/polys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="polys.3">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>6,1 10,1 10,-3 6,-3 6,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs><gml:innerBoundaryIs><gml:LinearRing><gml:coordinates>7,0 7,-2 9,-2 9,0 7,0</gml:coordinates></gml:LinearRing></gml:innerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:name>ASDF</ogr:name>
      <ogr:intval>0</ogr:intval>
      <ogr:layer>polys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/polys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="polys.4">
      <ogr:intval>120</ogr:intval>
      <ogr:floatval>-100291.43213</ogr:floatval>
      <ogr:layer>polys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/polys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:merged_polys fid="polys.5">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>3,2 6,1 6,-3 2,-1 2,2 3,2</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:name>elim</ogr:name>
      <ogr:intval>2</ogr:intval>
      <ogr:floatval>3.33</ogr:floatval>
      <ogr:layer>polys.gml</ogr:layer>
      <ogr:path>/home/nyall/dev/QGIS/python/plugins/processing/tests/testdata/polys.gml</ogr:path>
    </ogr:merged_polys>
  </gml:featureMember>
</ogr:FeatureCollection>
