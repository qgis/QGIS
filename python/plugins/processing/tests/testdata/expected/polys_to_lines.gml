<?xml version="1.0" encoding="utf-8" ?>
<ogr:FeatureCollection
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
     xsi:schemaLocation="http://ogr.maptools.org/ polys_to_lines.xsd"
     xmlns:ogr="http://ogr.maptools.org/"
     xmlns:gml="http://www.opengis.net/gml">
  <gml:boundedBy>
    <gml:Box>
      <gml:coord><gml:X>0</gml:X><gml:Y>-1</gml:Y></gml:coord>
      <gml:coord><gml:X>9</gml:X><gml:Y>6</gml:Y></gml:coord>
    </gml:Box>
  </gml:boundedBy>

  <gml:featureMember>
    <ogr:polys_to_lines fid="multipolys.0">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>2,1 2,2 3,2 3,3 4,3 4,1 2,1</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>1</ogr:Bintval>
      <ogr:Bfloatval>0.123</ogr:Bfloatval>
    </ogr:polys_to_lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:polys_to_lines fid="multipolys.1">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>7,-1 8,-1 8,3 7,3 7,-1</gml:coordinates></gml:LineString></ogr:geometryProperty>
    </ogr:polys_to_lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:polys_to_lines fid="multipolys.1">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>7,6 7,5 7,4 8,4 9,5 9,6 7,6</gml:coordinates></gml:LineString></ogr:geometryProperty>
    </ogr:polys_to_lines>
  </gml:featureMember>
  <gml:featureMember>
    <ogr:polys_to_lines fid="multipolys.2">
      <ogr:geometryProperty><gml:LineString srsName="EPSG:4326"><gml:coordinates>0,0 0,1 1,1 1,0 0,0</gml:coordinates></gml:LineString></ogr:geometryProperty>
      <ogr:Bname>Test</ogr:Bname>
      <ogr:Bintval>2</ogr:Bintval>
      <ogr:Bfloatval>-0.123</ogr:Bfloatval>
    </ogr:polys_to_lines>
  </gml:featureMember>
</ogr:FeatureCollection>
