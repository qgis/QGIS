<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation=""
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>0</gml:X><gml:Y>-1</gml:Y></gml:coord>
      <gml:coord><gml:X>9</gml:X><gml:Y>6</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>

  <gml:featureMember>
    <ogr:multipolys fid="multipolys.0">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>2,1 2,2 3,2 3,3 4,3 4,1 2,1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>1</ogr:Bintval>
      <ogr:Bfloatval>0.123</ogr:Bfloatval>
    </ogr:multipolys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:multipolys fid="multipolys.1">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>7,-1 8,-1 8,3 7,3 7,-1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>7,6 7,5 7,4 8,4 9,5 9,6 7,6</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
    </ogr:multipolys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:multipolys fid="multipolys.2">
      <ogr:geometryProperty><gml:MultiPolygon srsName="EPSG:4326"><gml:polygonMember><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>0,0 0,1 1,1 1,0 0,0</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></gml:polygonMember></gml:MultiPolygon></ogr:geometryProperty>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>2</ogr:Bintval>
      <ogr:Bfloatval>-0.123</ogr:Bfloatval>
    </ogr:multipolys>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:multipolys fid="multipolys.3">
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>3</ogr:Bintval>
      <ogr:Bfloatval>0</ogr:Bfloatval>
    </ogr:multipolys>
  </gml:featureMember>
</ogr:FeatureCollection>
